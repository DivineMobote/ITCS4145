#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <curl/curl.h>
// RapidJSON error plumbing 
#include <stdexcept>
#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

// Wrapper for RapidJson parse errors (convert to C++ exceptions)
struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset)
        : std::runtime_error(msg), rapidjson::ParseResult(code, offset) {}
};
#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) \
    throw ParseException(code, #code, offset)

using namespace std;
using namespace rapidjson;

// ---- Web API base ----
static const string BASE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";
static bool DEBUG_LOG = false;

// ----------- curl helpers -----------
static size_t write_cb(void* ptr, size_t sz, size_t nm, string* out) {
    size_t bytes = sz * nm;
    out->append(static_cast<char*>(ptr), bytes);
    return bytes;
}

static string url_encode(CURL* curl, const string& s) {
    char* enc = curl_easy_escape(curl, s.c_str(), (int)s.size());
    string r = enc ? enc : "";
    if (enc) curl_free(enc);
    return r;
}

static string fetch_neighbors(CURL* curl, const string& node) {
    string url = BASE_URL + url_encode(curl, node);
    string resp;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "C++-Client/1.0");
    // small safety timeouts so bad nodes don’t stall everything 
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    if (DEBUG_LOG) cerr << "[GET] " << url << "\n";
    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        if (DEBUG_LOG) cerr << "curl error: " << curl_easy_strerror(rc) << "\n";
        return "{}";
    }
    return resp;
}

static vector<string> parse_neighbors(const string& json) {
    vector<string> out;
    Document d;
    d.Parse(json.c_str());
    if (d.HasParseError()) {
        RAPIDJSON_PARSE_ERROR_NORETURN(d.GetParseError(), d.GetErrorOffset());
    }
    if (d.HasMember("neighbors") && d["neighbors"].IsArray()) {
        for (auto& v : d["neighbors"].GetArray()) {
            if (v.IsString()) out.push_back(v.GetString());
        }
    }
    return out;
}

// ------------- Blocking queue ----------------
struct Work { string node; int level; };

class BlockingQueue {
public:
    void push(const Work& w) {
        {
            lock_guard<mutex> lk(m_);
            if (closed_) return;      
            q_.push(w);
        }
        cv_.notify_one();
    }
    // returns false only when queue is closed AND empty
    bool pop(Work& w) {
        unique_lock<mutex> lk(m_);
        cv_.wait(lk, [&]{ return closed_ || !q_.empty(); });
        if (q_.empty()) return false; // closed & empty
        w = q_.front();
        q_.pop();
        return true;
    }
    // Wakes up all sleepers so they can exit
    void close() {
        {
            lock_guard<mutex> lk(m_);
            closed_ = true;
        }
        cv_.notify_all();
    }
    bool empty() const {
        lock_guard<mutex> lk(m_);
        return q_.empty();
    }
private:
    mutable mutex m_;
    condition_variable cv_;
    queue<Work> q_;
    bool closed_ = false;
};

// ----------- parallel BFS ------------
static vector<string> bfs_parallel(const string& start, int max_depth, int thread_count) {
    BlockingQueue q;
    unordered_set<string> visited;
    vector<string> order;

    mutex visited_mtx;
    mutex order_mtx;

    // tracks how many workers are processing items right now
    atomic<int> active{0};

    // seed the queue
    {
        lock_guard<mutex> lk(visited_mtx);
        visited.insert(start);
    }
    q.push({start, 0});

    // Worker thread procedure
    auto worker = [&](int tid) {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        Work w;
        while (true) {
            if (!q.pop(w)) break; 

            active.fetch_add(1);

            // record the node in output order(print later in main)
            {
                lock_guard<mutex> lk(order_mtx);
                order.push_back(w.node);
            }

            // Only expand neighbors if we're not in the max depth yet
            if (w.level < max_depth) {
                try {
                    auto body = fetch_neighbors(curl, w.node);
                    auto nbrs = parse_neighbors(body);

                    for (const auto& nb : nbrs) {
                        bool first_time = false;
                        // Check visited under a lock (avoid races)
                        {
                            lock_guard<mutex> lk(visited_mtx);
                            if (!visited.count(nb)) {
                                visited.insert(nb);
                                first_time = true;
                            }
                        }
                        if (first_time) {
                            q.push({nb, w.level + 1});
                        }
                    }
                } catch (const ParseException&) {
                    if (DEBUG_LOG) cerr << "[tid " << tid << "] bad JSON @ " << w.node << "\n";
                }
            }

            // this worker finished one item
            int now = active.fetch_sub(1) - 1;

            // If no work in flight AND queue currently empty then close queue to wake sleepers
            if (now == 0 && q.empty()) {
                q.close();
            }
        }

        curl_easy_cleanup(curl);
    };

    // launch pool
    vector<thread> pool;
    pool.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) pool.emplace_back(worker, i);
    for (auto& t : pool) t.join();

    return order;
}


int main(int argc, char** argv) {
    // Agrs: program <start_node> <depth> [threads=8] [debug=0|1]
    if (argc < 3 || argc > 5) {
        cerr << "Usage: " << argv[0] << " <start_node> <depth> [threads=8] [debug=0|1]\n";
        return 1;
    }
    string start_node = argv[1];   
    int depth = 0;
    try { depth = stoi(argv[2]); } catch (...) {
        cerr << "Depth must be an integer.\n"; return 1;
    }
    int threads = (argc >= 4) ? max(1, stoi(argv[3])) : 8;
    DEBUG_LOG = (argc == 5) ? (stoi(argv[4]) != 0) : false;

    // curl global init once
    curl_global_init(CURL_GLOBAL_ALL);

    auto t0 = chrono::steady_clock::now();
    auto visited_order = bfs_parallel(start_node, depth, threads);
    auto t1 = chrono::steady_clock::now();

    // text output (one node per line)
    for (const auto& n : visited_order) {
        cout << "- " << n << "\n";
    }
    cout << "Visited: " << visited_order.size() << " nodes\n";
    chrono::duration<double> secs = t1 - t0;
    cout << "Time to crawl: " << secs.count() << "s\n";

    curl_global_cleanup();
    return 0;
}
