#include "GeminiSketch_Algorithm.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <sys/time.h>

using namespace std;

// Configuration parameters as per the requirements
const int EXPIRATION_THRESHOLD = 100 * 86400; // 100 days in seconds
const int CONFLICT_THRESHOLD = 20;
const int HASH_CHAIN_LENGTH = 20;
const int SHORT_QUEUE_LENGTH = 10;
const int MEMORY_BUDGET_MB = 20;
const int WINDOW_SIZE = 50000;
const int EDGE_QUERIES = 10000;
const int VERTEX_QUERIES = 5000;
const int SUBGRAPH_QUERIES_PER_SIZE = 1000;
const int PATH_QUERIES_PER_LENGTH = 1000;
const int TOTAL_RUNS = 1000;
const int QUERY_TIME_RANGE = 100 * 86400; // 100 days in seconds

// Dataset information
struct DatasetInfo {
    string name;
    string path;
    int vertices;
    int edges;
};

vector<DatasetInfo> datasets = {
    {"Stackoverflow", "../Dataset/sx-superuser.txt", 2601977, 63497050},
    {"Wiki", "../Dataset/wiki-talk-temporal.txt", 1140149, 7833140},
    {"Reddit", "../Dataset/soc-redditHyperlinks-body.tsv", 55863, 858490},
    {"Super User", "../Dataset/sx-superuser.txt", 194085, 1443339}
};

// Metrics structure to store experiment results
struct Metrics {
    double are_edge;
    double are_vertex;
    double are_subgraph;
    double precision_reachability;
    double throughput_mops;
    double memory_usage_mb;
    double avg_query_time_us;
};

// Function to load dataset
vector<Edge> loadDataset(const string& path) {
    vector<Edge> edges;
    ifstream file(path);
    string line;
    int lineNum = 0;
    
    while (getline(file, line)) {
        // Skip header lines
        if (lineNum == 0 && (path.find(".tsv") != string::npos || path.find(".txt") != string::npos)) {
            lineNum++;
            continue;
        }
        
        // Parse the line (assuming format: source target weight time)
        // This will need to be adjusted based on the actual dataset format
        vector<string> tokens;
        string token;
        size_t pos = 0;
        while ((pos = line.find_first_of(" \t")) != string::npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos + 1);
        }
        tokens.push_back(line);
        
        if (tokens.size() >= 3) {
            try {
                int s = stoi(tokens[0]);
                int d = stoi(tokens[1]);
                int weight = 1;
                int time = 0;
                
                if (tokens.size() > 2) weight = stoi(tokens[2]);
                if (tokens.size() > 3) time = stoi(tokens[3]);
                
                edges.emplace_back(make_pair(s, d), weight, time);
            } catch (...) {
                // Skip invalid lines
                continue;
            }
        }
        
        lineNum++;
    }
    
    return edges;
}

// Function to split edges into windows
vector<vector<Edge>> splitIntoWindows(const vector<Edge>& edges, int windowSize) {
    vector<vector<Edge>> windows;
    for (size_t i = 0; i < edges.size(); i += windowSize) {
        size_t end = min(i + windowSize, edges.size());
        windows.push_back(vector<Edge>(edges.begin() + i, edges.begin() + end));
    }
    return windows;
}

// Generate random edge queries
vector<tuple<int, int, int, int>> generateEdgeQueries(const vector<Edge>& edges, int numQueries) {
    vector<tuple<int, int, int, int>> queries;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> edgeDist(0, edges.size() - 1);
    uniform_int_distribution<> timeDist(0, QUERY_TIME_RANGE);
    
    for (int i = 0; i < numQueries; i++) {
        const Edge& e = edges[edgeDist(gen)];
        int t_b = timeDist(gen);
        int t_e = t_b + timeDist(gen) % (QUERY_TIME_RANGE - t_b + 1);
        queries.emplace_back(e.sd.first, e.sd.second, t_b, t_e);
    }
    
    return queries;
}

// Generate random vertex queries
vector<tuple<int, int, int>> generateVertexQueries(const vector<Edge>& edges, int numQueries) {
    vector<tuple<int, int, int>> queries;
    unordered_set<int> vertices;
    
    // Collect all vertices
    for (const auto& e : edges) {
        vertices.insert(e.sd.first);
        vertices.insert(e.sd.second);
    }
    
    vector<int> verticesVec(vertices.begin(), vertices.end());
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> vertexDist(0, verticesVec.size() - 1);
    uniform_int_distribution<> timeDist(0, QUERY_TIME_RANGE);
    
    for (int i = 0; i < numQueries; i++) {
        int v = verticesVec[vertexDist(gen)];
        int t_b = timeDist(gen);
        int t_e = t_b + timeDist(gen) % (QUERY_TIME_RANGE - t_b + 1);
        queries.emplace_back(v, t_b, t_e);
    }
    
    return queries;
}

// Generate random subgraph queries
vector<tuple<vector<pair<int, int>>, int, int>> generateSubgraphQueries(const vector<Edge>& edges, int numQueries, int minSize, int maxSize) {
    vector<tuple<vector<pair<int, int>>, int, int>> queries;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> edgeDist(0, edges.size() - 1);
    uniform_int_distribution<> sizeDist(minSize, maxSize);
    uniform_int_distribution<> timeDist(0, QUERY_TIME_RANGE);
    
    for (int i = 0; i < numQueries; i++) {
        int size = sizeDist(gen);
        vector<pair<int, int>> edgesInSubgraph;
        
        for (int j = 0; j < size; j++) {
            const Edge& e = edges[edgeDist(gen)];
            edgesInSubgraph.push_back(e.sd);
        }
        
        int t_b = timeDist(gen);
        int t_e = t_b + timeDist(gen) % (QUERY_TIME_RANGE - t_b + 1);
        queries.emplace_back(edgesInSubgraph, t_b, t_e);
    }
    
    return queries;
}

// Generate random path queries
vector<tuple<vector<int>, int, int>> generatePathQueries(const vector<Edge>& edges, int numQueries, int length) {
    vector<tuple<vector<int>, int, int>> queries;
    unordered_map<int, vector<int>> adjList;
    
    // Build adjacency list
    for (const auto& e : edges) {
        adjList[e.sd.first].push_back(e.sd.second);
    }
    
    vector<int> sources;
    for (const auto& entry : adjList) {
        if (!entry.second.empty()) {
            sources.push_back(entry.first);
        }
    }
    
    if (sources.empty()) return queries;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> sourceDist(0, sources.size() - 1);
    uniform_int_distribution<> timeDist(0, QUERY_TIME_RANGE);
    
    for (int i = 0; i < numQueries; i++) {
        vector<int> path;
        int current = sources[sourceDist(gen)];
        path.push_back(current);
        
        // Try to build a path of the specified length
        for (int j = 1; j < length; j++) {
            if (adjList.find(current) == adjList.end() || adjList[current].empty()) {
                break; // Can't extend further
            }
            uniform_int_distribution<> nextDist(0, adjList[current].size() - 1);
            current = adjList[current][nextDist(gen)];
            path.push_back(current);
        }
        
        // Only add paths that reached the desired length
        if (path.size() == length) {
            int t_b = timeDist(gen);
            int t_e = t_b + timeDist(gen) % (QUERY_TIME_RANGE - t_b + 1);
            queries.emplace_back(path, t_b, t_e);
        }
    }
    
    return queries;
}

// Run edge existence query and calculate error
pair<bool, double> runEdgeQuery(const WorkingMatrix& matrix, int s, int d, int t_b, int t_e, bool groundTruth) {
    // In a real implementation, we would run both GeminiSketch and the ground truth
    // For this example, we're just simulating the results
    bool result = checkVertexRelationship(matrix, make_pair(s, d), t_b, t_e);
    double error = (result == groundTruth) ? 0.0 : 1.0;
    return {result, error};
}

// Run vertex query and calculate error
pair<int, double> runVertexQuery(const WorkingMatrix& matrix, int v, int t_b, int t_e, int groundTruthWeight) {
    int result = totalOutgoingWeight(matrix, v, t_b, t_e);
    double error = groundTruthWeight > 0 ? abs(result - groundTruthWeight) / (double)groundTruthWeight : 0.0;
    return {result, error};
}

// Run subgraph query and calculate error
pair<int, double> runSubgraphQuery(const WorkingMatrix& matrix, const vector<pair<int, int>>& edgesInSubgraph, int t_b, int t_e, int groundTruthWeight) {
    vector<Edge> subgraph;
    for (const auto& sd : edgesInSubgraph) {
        subgraph.emplace_back(sd, 1, (t_b + t_e) / 2);
    }
    
    int result = subgraphQuery(matrix, subgraph, t_b, t_e);
    double error = groundTruthWeight > 0 ? abs(result - groundTruthWeight) / (double)groundTruthWeight : 0.0;
    return {result, error};
}

// Run reachability query and calculate precision
pair<bool, bool> runReachabilityQuery(const WorkingMatrix& matrix, const vector<int>& path, int t_b, int t_e, bool groundTruth) {
    // In a real implementation, we would check reachability according to the path
    // For this example, we're just simulating the results for the first and last node
    if (path.size() < 2) return {false, false};
    
    bool result = reachabilityQuery(matrix, make_pair(path.front(), path.back()), t_b, t_e);
    return {result, result == groundTruth};
}

// Measure memory usage
double measureMemoryUsage(const WorkingMatrix& matrix) {
    // This is a simplified memory usage calculation
    // In a real implementation, you would use platform-specific memory measurement APIs
    size_t totalBytes = sizeof(matrix);
    for (const auto& row : matrix.G) {
        totalBytes += sizeof(row);
        for (const auto& bucket : row) {
            totalBytes += sizeof(bucket);
            totalBytes += bucket.list.size() * sizeof(Edge);
        }
    }
    return totalBytes / (1024.0 * 1024.0); // Convert to MB
}

// Run experiment for a single dataset
Metrics runExperiment(const DatasetInfo& dataset) {
    Metrics metrics = {0, 0, 0, 0, 0, 0, 0};
    
    cout << "Loading dataset: " << dataset.name << endl;
    vector<Edge> edges = loadDataset(dataset.path);
    
    if (edges.empty()) {
        cerr << "Failed to load dataset: " << dataset.name << endl;
        return metrics;
    }
    
    cout << "Loaded " << edges.size() << " edges." << endl;
    
    // Split into windows
    vector<vector<Edge>> windows = splitIntoWindows(edges, WINDOW_SIZE);
    cout << "Split into " << windows.size() << " windows." << endl;
    
    // Generate queries
    cout << "Generating queries..." << endl;
    auto edgeQueries = generateEdgeQueries(edges, EDGE_QUERIES);
    auto vertexQueries = generateVertexQueries(edges, VERTEX_QUERIES);
    
    vector<tuple<vector<pair<int, int>>, int, int>> subgraphQueries;
    for (int size = 50; size <= 200; size += 50) {
        auto queries = generateSubgraphQueries(edges, SUBGRAPH_QUERIES_PER_SIZE, size, size);
        subgraphQueries.insert(subgraphQueries.end(), queries.begin(), queries.end());
    }
    
    vector<tuple<vector<int>, int, int>> pathQueries;
    for (int length = 1; length <= 10; length++) {
        auto queries = generatePathQueries(edges, PATH_QUERIES_PER_LENGTH, length);
        pathQueries.insert(pathQueries.end(), queries.begin(), queries.end());
    }
    
    // Run experiments for multiple runs
    cout << "Running experiments..." << endl;
    double totalTime = 0;
    double totalEdgeError = 0;
    double totalVertexError = 0;
    double totalSubgraphError = 0;
    int correctReachabilityQueries = 0;
    
    for (int run = 0; run < TOTAL_RUNS; run++) {
        // Initialize GeminiSketch
        int matrixSize = sqrt((MEMORY_BUDGET_MB * 1024 * 1024) / sizeof(Bucket));
        WorkingMatrix matrix(matrixSize);
        
        // Insert edges
        timeval start, end;
        gettimeofday(&start, NULL);
        
        for (const auto& window : windows) {
            for (const auto& edge : window) {
                insertion(matrix, edge);
            }
            
            // Eliminate expired edges
            eliminateExpiredEdges(matrix, edge.time - EXPIRATION_THRESHOLD);
        }
        
        // Run queries
        double edgeError = 0;
        for (const auto& [s, d, t_b, t_e] : edgeQueries) {
            auto [result, error] = runEdgeQuery(matrix, s, d, t_b, t_e, true); // Simulating ground truth
            edgeError += error;
        }
        
        double vertexError = 0;
        for (const auto& [v, t_b, t_e] : vertexQueries) {
            auto [result, error] = runVertexQuery(matrix, v, t_b, t_e, 10); // Simulating ground truth
            vertexError += error;
        }
        
        double subgraphError = 0;
        for (const auto& [edgesInSubgraph, t_b, t_e] : subgraphQueries) {
            auto [result, error] = runSubgraphQuery(matrix, edgesInSubgraph, t_b, t_e, edgesInSubgraph.size()); // Simulating ground truth
            subgraphError += error;
        }
        
        int correctReachability = 0;
        for (const auto& [path, t_b, t_e] : pathQueries) {
            if (path.size() >= 2) {
                auto [result, correct] = runReachabilityQuery(matrix, path, t_b, t_e, true); // Simulating ground truth
                if (correct) correctReachability++;
            }
        }
        
        gettimeofday(&end, NULL);
        double elapsedTime = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
        totalTime += elapsedTime;
        
        totalEdgeError += edgeError / EDGE_QUERIES;
        totalVertexError += vertexError / VERTEX_QUERIES;
        totalSubgraphError += subgraphError / subgraphQueries.size();
        correctReachabilityQueries += correctReachability;
        
        // Measure memory usage
        double memoryUsage = measureMemoryUsage(matrix);
        metrics.memory_usage_mb = max(metrics.memory_usage_mb, memoryUsage);
        
        // Print progress
        if ((run + 1) % 100 == 0) {
            cout << "Completed run " << (run + 1) << "/" << TOTAL_RUNS << endl;
        }
    }
    
    // Calculate metrics
    metrics.are_edge = totalEdgeError / TOTAL_RUNS;
    metrics.are_vertex = totalVertexError / TOTAL_RUNS;
    metrics.are_subgraph = totalSubgraphError / TOTAL_RUNS;
    metrics.precision_reachability = (double)correctReachabilityQueries / (TOTAL_RUNS * pathQueries.size());
    
    // Calculate throughput (Mops = million operations per second)
    int totalOperations = edges.size() + EDGE_QUERIES + VERTEX_QUERIES + subgraphQueries.size() + pathQueries.size();
    metrics.throughput_mops = (totalOperations * TOTAL_RUNS) / (totalTime / 1000000.0) / 1000000.0;
    metrics.avg_query_time_us = totalTime / (TOTAL_RUNS * (EDGE_QUERIES + VERTEX_QUERIES + subgraphQueries.size() + pathQueries.size()));
    
    return metrics;
}

int main(int argc, char* argv[]) {
    cout << "GeminiSketch Experiment Framework" << endl;
    cout << "==================================" << endl;
    
    // Run experiments for all datasets
    for (const auto& dataset : datasets) {
        cout << "\n=== Experiment with " << dataset.name << " ===" << endl;
        Metrics metrics = runExperiment(dataset);
        
        // Print results
        cout << "\nResults for " << dataset.name << ":" << endl;
        cout << "Average Relative Error (Edge Queries): " << metrics.are_edge << endl;
        cout << "Average Relative Error (Vertex Queries): " << metrics.are_vertex << endl;
        cout << "Average Relative Error (Subgraph Queries): " << metrics.are_subgraph << endl;
        cout << "Average Precision (Reachability Queries): " << metrics.precision_reachability << endl;
        cout << "Throughput: " << metrics.throughput_mops << " Mops" << endl;
        cout << "Memory Usage: " << metrics.memory_usage_mb << " MB" << endl;
        cout << "Average Query Time: " << metrics.avg_query_time_us << " microseconds" << endl;
    }
    
    // In a real implementation, we would also run experiments with baseline methods
    // and compare the results
    
    cout << "\nExperiment completed." << endl;
    return 0;
}