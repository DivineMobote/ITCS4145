#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

#include <curl/curl.h>

#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

// RapidJSON error plumbing that is similar to the sequential code
struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset)
        : std::runtime_error(msg), rapidjson::ParseResult(code, offset) {}
};
#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) throw ParseException(code, #code, offset)

// --------- Config-----------
static bool DEBUG_LOG = false;
static const std::string SERVICE_URL =
    "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// Default max thread count
static int default_max_threads() {
    if (const char* env = std::getenv("MAX_THREADS")) {
        int t = std::max(1, std::atoi(env));
        return t;
    }
    return 8;
}

// -------curl helpers ---------- 
// URL-encode a node name for the REST call. 
static std::string url_encode(CURL* curl, const std::string& input) {
    char* out = curl_easy_escape(curl, input.c_str(), (int)input.size());
    std::string s = out ? out : "";
    if (out) curl_free(out);
    return s;
}

// Collect HTTP response bytes into a std::string.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

// Each thread should use its own CURL easy handle
static std::string fetch_neighbors(CURL* curl, const std::string& node) {
    std::string url = SERVICE_URL + url_encode(curl, node);
    std::string response;

    if (DEBUG_LOG) std::cerr << "GET " << url << "\n";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        if (DEBUG_LOG) std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        response = "{}"; // fall back to empty
    }

    curl_slist_free_all(headers);
    return response;
}

//  Parse {"neighbors": ["...","..."]} into a vector<string>
static std::vector<std::string> parse_neighbors(const std::string& json_str) {
    std::vector<std::string> neighbors;
    try {
        rapidjson::Document doc;
        doc.Parse(json_str.c_str());
        if (doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
            for (const auto& n : doc["neighbors"].GetArray()) {
                if (n.IsString()) neighbors.emplace_back(n.GetString());
            }
        }
    } catch (const ParseException&) {
        std::cerr << "JSON parse error on: " << json_str << "\n";
        throw;
    }
    return neighbors;
}


// ------ Parallel level-by-level BFS ------
// Returns levels[0..depth], where levels[0] = {start}
static std::vector<std::vector<std::string>> bfs_parallel(
    const std::string& start, int depth, int max_threads)
{
    std::vector<std::vector<std::string>> levels;
    levels.reserve(std::max(1, depth + 1));
    levels.push_back({start});

    std::unordered_set<std::string> visited;
    visited.reserve(4096);
    visited.insert(start);

    std::mutex visited_mtx; 
    for (int d = 0; d < depth; ++d) {
        const auto& curr = levels[d];
        if (curr.empty()) { 
            levels.emplace_back();
            continue;
        }

        // decide threads for this level
        const int T = std::max(1, std::min<int>(max_threads, (int)curr.size()));

        // partition [0..curr.size()) into T ranges
        std::vector<std::pair<size_t, size_t>> ranges;
        ranges.reserve(T);
        const size_t N = curr.size();
        const size_t base = N / T;
        size_t rem = N % T;
        size_t lo = 0;
        for (int t = 0; t < T; ++t) {
            size_t sz = base + (rem ? 1 : 0);
            if (rem) --rem;
            ranges.emplace_back(lo, lo + sz);
            lo += sz;
        }

        // thread-local buffers to reduce lock confusions
        std::vector<std::vector<std::string>> discovered(T);
        std::vector<std::thread> threads;
        threads.reserve(T);

        for (int t = 0; t < T; ++t) {
            auto [L, R] = ranges[t];
            threads.emplace_back([&, t, L, R]() {
                CURL* curl = curl_easy_init();
                if (!curl) return;

                std::vector<std::string> local;
                local.reserve((R > L) ? (R - L) * 8 : 8);

                for (size_t i = L; i < R; ++i) {
                    const std::string& node = curr[i];
                    std::string json = fetch_neighbors(curl, node);

                    std::vector<std::string> nbrs;
                    try {
                        nbrs = parse_neighbors(json);
                    } catch (...) {
                        // Skip node on parse failure and keep going
                        continue;
                    }

                    // Check-then-insert must be atomic for mutex guard
                    for (const auto& nb : nbrs) {
                        bool add = false;
                        {   
                            std::scoped_lock lk(visited_mtx);
                            auto [it, inserted] = visited.insert(nb);
                            if (inserted) add = true;
                        }
                        if (add) local.emplace_back(nb);
                    }
                }

                curl_easy_cleanup(curl);
                discovered[t] = std::move(local);
            });
        }

        // Wait for all threads on this level
        for (auto& th : threads) th.join();

        // Merge thread-local results into next level
        std::vector<std::string> next;
        size_t total = 0;
        for (const auto& v : discovered) total += v.size();
        next.reserve(total);
        for (int t = 0; t < T; ++t) {
            const auto& v = discovered[t];
            next.insert(next.end(), v.begin(), v.end());
        }

        levels.emplace_back(std::move(next));
    }

    return levels;
}


// -------- Main ------------
int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <node_name> <depth> [max_threads]\n";
        return 1;
    }
    
    // If DEBUG env var is needed to get extra curl logs
    if (const char* dbg = std::getenv("DEBUG")) {
        DEBUG_LOG = (std::atoi(dbg) != 0);
    }

    // Parse CLI args
    std::string start_node = argv[1];
    int depth = 0;
    try {
        depth = std::stoi(argv[2]);
        if (depth < 0) throw std::invalid_argument("neg");
    } catch (...) {
        std::cerr << "Error: depth must be a non-negative integer.\n";
        return 1;
    }

    int max_threads = (argc == 4) ? std::max(1, std::atoi(argv[3])) : default_max_threads();

    // One-time curl init for this process
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        std::cerr << "Failed to curl_global_init()\n";
        return 2;
    }

    // Run the parrel, level-by-level BFS
    const auto t0 = std::chrono::steady_clock::now();
    auto levels = bfs_parallel(start_node, depth, max_threads);
    const auto t1 = std::chrono::steady_clock::now();

    // Match the sequential client's output format
    for (const auto& lvl : levels) {
        for (const auto& node : lvl) {
            std::cout << "- " << node << "\n";
        }
        std::cout << lvl.size() << "\n";
    }

    std::chrono::duration<double> elapsed = t1 - t0;
    std::cout << "Time to crawl: " << elapsed.count() << "s\n";

    curl_global_cleanup();
    return 0;
}
