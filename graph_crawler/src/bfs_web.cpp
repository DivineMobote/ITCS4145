
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cstdio>
#include <chrono>

#include <curl/curl.h>
#include <rapidjson/document.h>

using namespace std;

// Base URL for the API
static const string BASE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";


// libcurl callback: writes response body into a std::string
static size_t write_to_string(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t bytes = size * nmemb;
    string* out = static_cast<string*>(userdata);
    out->append(static_cast<char*>(ptr), bytes);
    return bytes;
}


// URL-encode a node 
static string url_encode(CURL* curl, const string& s) {
    char* enc = curl_easy_escape(curl, s.c_str(), (int)s.size());
    if (!enc) return s; // fallback (shouldn’t happen)
    string out(enc);
    curl_free(enc);
    return out;
}


// Get neighbors for a node from the API. 
// Accepts RAW node and encodes it internally.
// Returns true on success, and fills 'out' with the neighbors.
static bool get_neighbors(CURL* curl, const string& node_raw, vector<string>& out) {
    out.clear();

    // Encode every time the API is hit (to handle special chars)
    string encoded = url_encode(curl, node_raw);
    string url = BASE_URL + encoded;
    string body;

    // Basic curl set for a GET request
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "bfs-web-student/0.1");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    // Make the request    
    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        cerr << "[curl] " << curl_easy_strerror(rc) << " for " << url << "\n";
        return false;
    }

    // Check HTTP status code
    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200) {
        cerr << "[http] status " << code << " for " << url << "\n";
        return false;
    }

    // Parse JSON
    rapidjson::Document d;
    d.Parse(body.c_str());
    if (d.HasParseError() || !d.IsObject() || !d.HasMember("neighbors") || !d["neighbors"].IsArray()) {
        cerr << "[json] unexpected response for " << node_raw << "\n";
        return false;
    }

    // Copy neighbors into output vector
    for (auto& v : d["neighbors"].GetArray()) {
        if (v.IsString()) out.push_back(v.GetString());
    }
    return true;
}


// Helps for bad CLI
static void usage(const char* prog) {
    cerr << "Usage: " << prog << " \"<start node>\" <depth> [-o output.txt]\n";
    cerr << "Example: " << prog << " \"Tom Hanks\" 2 -o tom2.txt\n";
}


// Main program: BFS from start node up to max depth, using the web API
int main(int argc, char** argv) {
    if (argc < 3) { usage(argv[0]); return 1; }

    string start_raw = argv[1];
    int max_depth = -1;
    try {
        max_depth = stoi(argv[2]);
    } catch (...) {
        cerr << "Depth must be an integer.\n";
        return 1;
    }
    if (max_depth < 0) {
        cerr << "Depth must be >= 0.\n";
        return 1;
    }

    // Output file for writing full results (optional)
    string out_path;
    for (int i = 3; i < argc; ++i) {
        string a = argv[i];
        if (a == "-o" && i + 1 < argc) out_path = argv[++i];
    }

    // Init CURL, once for the whole run then reuse it for all requests
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to init CURL\n";
        return 2;
    }

    // BFS
    auto t0 = chrono::high_resolution_clock::now();

    // queue of (node, depth)
    string start = start_raw;
    queue<pair<string,int>> q;
    unordered_set<string> seen;
    unordered_map<string,int> dist;

    q.push({start, 0});
    seen.insert(start);
    dist[start] = 0;

    // BFS loop
    vector<string> nbrs;
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        const string& u = cur.first;
        int d = cur.second;

        if (d == max_depth) continue;

        if (!get_neighbors(curl, u, nbrs)) {
            
            continue;
        }

        for (const auto& v : nbrs) {
            if (seen.insert(v).second) {
                dist[v] = d + 1;
                q.push({v, d + 1});
            }
        }
    }

    auto t1 = chrono::high_resolution_clock::now();
    double ms = chrono::duration<double, std::milli>(t1 - t0).count();

    curl_easy_cleanup(curl);

    // sort results by distance then name 
    vector<pair<string,int>> rows(dist.begin(), dist.end());
    sort(rows.begin(), rows.end(), [](const auto& a, const auto& b){
        if (a.second != b.second) return a.second < b.second;
        return a.first < b.first;
    });

    // write file if requested
    if (!out_path.empty()) {
        FILE* f = fopen(out_path.c_str(), "w");
        if (!f) {
            cerr << "Could not open " << out_path << " for writing.\n";
            return 1;
        }
        for (auto& p : rows) {
            
            fprintf(f, "%s\t%d\n", p.first.c_str(), p.second);
        }
        fclose(f);
    }

    // brief summary + small preview
    cout << "Start: \"" << start_raw << "\"  depth: " << max_depth
         << "  reached: " << rows.size()
         << "  time: " << ms << " ms\n";
    cout << "node\tdist\n";
    size_t show = min<size_t>(rows.size(), 20);
    for (size_t i = 0; i < show; ++i) {
        cout << rows[i].first << "\t" << rows[i].second << "\n";
    }
    if (rows.size() > show) cout << "... (" << (rows.size() - show) << " more)\n";

    return 0;
}

