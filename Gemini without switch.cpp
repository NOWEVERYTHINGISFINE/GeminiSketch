#include <iostream>
#include <vector>
#include <utility>
#include <xxhash.h>

// Define the edge structure
struct Edge {
    std::pair<int, int> sd; // <s, d>
    int weight;
    int time;
    Edge(std::pair<int, int> sd, int weight, int time) : sd(sd), weight(weight), time(time) {}
};

// Define the bucket structure
struct Bucket {
    std::pair<int, int> vx; // <s, d>
    int ec; // edge count
    int CF; // flag
    std::vector<Edge> list; // edge list
    int GT; // timestamp
    Bucket* bqp; // bucket queue pointer
    Bucket() : ec(0), CF(0), GT(0), bqp(nullptr) {}
};

// Define the working matrix structure
struct WorkingMatrix {
    std::vector<std::vector<Bucket>> G; // matrix
    Bucket* HP; // head pointer
    Bucket* MP; // middle pointer
    Bucket* TP; // tail pointer
    WorkingMatrix(int size) : HP(nullptr), MP(nullptr), TP(nullptr) {
        G.resize(size, std::vector<Bucket>(size));
    }
};

// Hash function example
// Replace with xxhash's hash function

int H(int x) {
    return XXH32(&x, sizeof(x), 0) % 10;
}

// Vertex query algorithm
// Define the value of g, which can be adjusted according to actual conditions
const int g = 1;

// Modify the vertex query function to include the chain hashing compensation mechanism
bool vertexQuery(const WorkingMatrix& matrix, int v, int t_b, int t_e) {
    int r = H(v);
    for (int offset = 0; offset <= g; ++offset) {
        int adjusted_r = (r + offset) % matrix.G.size();
        for (const auto& bucket : matrix.G[adjusted_r]) {
            if (bucket.vx.first == v || bucket.vx.second == v) {
                for (const auto& edge : bucket.list) {
                    if (edge.time >= t_b && edge.time <= t_e) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Other query functions can be modified similarly

// Calculate the total outgoing edge weight of vertex v within [t_b, t_e]
int totalOutgoingWeight(const WorkingMatrix& matrix, int v, int t_b, int t_e) {
    int totalWeight = 0;
    int r = H(v);
    for (const auto& bucket : matrix.G[r]) {
        if (bucket.vx.first == v) {
            for (const auto& edge : bucket.list) {
                if (edge.time >= t_b && edge.time <= t_e) {
                    totalWeight += edge.weight;
                }
            }
        }
    }
    return totalWeight;
}

// Calculate the number of outgoing edges of vertex v within [t_b, t_e]
int outgoingEdgeCount(const WorkingMatrix& matrix, int v, int t_b, int t_e) {
    int count = 0;
    int r = H(v);
    for (const auto& bucket : matrix.G[r]) {
        if (bucket.vx.first == v) {
            for (const auto& edge : bucket.list) {
                if (edge.time >= t_b && edge.time <= t_e) {
                    count++;
                }
            }
        }
    }
    return count;
}

// Insertion operation
void insertion(WorkingMatrix& matrix, Edge e) {
    int i = H(e.sd.first);
    int j = H(e.sd.second);

    for (; ; ) {
        if (matrix.G[i][j].vx == e.sd || matrix.G[i][j].vx == std::make_pair(0, 0)) {
            break;
        } else {
            matrix.G[i][j].CF = 0;
        }
    }

    if (matrix.G[i][j].vx == std::make_pair(0, 0)) {
        matrix.G[i][j].ec += 1;
        matrix.G[i][j].vx = e.sd;
        matrix.G[i][j].CF = 1;
        matrix.G[i][j].list.push_back(e);
        matrix.G[i][j].GT = e.time;

        if (matrix.TP == nullptr) {
            matrix.HP = &matrix.G[i][j];
            matrix.MP = &matrix.G[i][j];
            matrix.TP = &matrix.G[i][j];
        } else {
            matrix.TP->bqp = &matrix.G[i][j];
            matrix.TP = &matrix.G[i][j];
        }
    } else {
        matrix.G[i][j].ec += 1;
        // Other processing...
    }
}

// Eliminate expired edges operation
void eliminateExpiredEdges(WorkingMatrix& matrix, int Te) {
    if (matrix.MP != nullptr && matrix.MP->list.front().time <= Te) {
        Bucket* WP = matrix.HP;
        while (WP != matrix.MP) {
            while (!WP->list.empty() && WP->list.front().time <= Te) {
                WP->ec -= 1;
                if (WP->ec == 0) {
                    Bucket* NB = WP;
                    WP = WP->bqp;
                    // Remove NB from virtual bucket queue...
                } else {
                    WP->list.erase(WP->list.begin());
                }
            }
            WP = WP->bqp;
        }

        while (!matrix.MP->list.empty() && matrix.MP->list.front().time <= Te) {
            Bucket* S = matrix.MP;
            // Process S...
        }
    }
}

// Time-related query: Find all active edges within [t_b, t_e]
std::vector<Edge> findActiveEdges(const WorkingMatrix& matrix, int t_b, int t_e) {
    std::vector<Edge> activeEdges;
    for (const auto& row : matrix.G) {
        for (const auto& bucket : row) {
            for (const auto& edge : bucket.list) {
                if (edge.time >= t_b && edge.time <= t_e) {
                    activeEdges.push_back(edge);
                }
            }
        }
    }
    return activeEdges;
}

// Time-related query: Check the relationship between vertices within [t_b, t_e]
bool checkVertexRelationship(const WorkingMatrix& matrix, std::pair<int, int> vertexPair, int t_b, int t_e) {
    for (const auto& row : matrix.G) {
        for (const auto& bucket : row) {
            for (const auto& edge : bucket.list) {
                if (edge.sd == vertexPair && edge.time >= t_b && edge.time <= t_e) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Reachability query
bool reachabilityQuery(const WorkingMatrix& matrix, std::pair<int, int> startEndPair, int t_b, int t_e) {
    // Simple implementation, can be optimized according to actual requirements
    std::vector<Edge> activeEdges = findActiveEdges(matrix, t_b, t_e);
    std::vector<int> visited;
    std::vector<int> queue;
    queue.push_back(startEndPair.first);

    while (!queue.empty()) {
        int current = queue.back();
        queue.pop_back();
        visited.push_back(current);

        for (const auto& edge : activeEdges) {
            if (edge.sd.first == current && std::find(visited.begin(), visited.end(), edge.sd.second) == visited.end()) {
                if (edge.sd.second == startEndPair.second) {
                    return true;
                }
                queue.push_back(edge.sd.second);
            }
        }
    }
    return false;
}

class GeminiSketch {
private:
    int num_buckets;
    std::vector<int> counters;

public:
    GeminiSketch(int buckets) : num_buckets(buckets), counters(buckets, 0) {}

    void insertEdge(int edge_id) {
        int hash_value = std::hash<int>()(edge_id) % num_buckets;
        counters[hash_value]++;
    }

    int queryEdge(int edge_id) {
        int hash_value = std::hash<int>()(edge_id) % num_buckets;
        return counters[hash_value];
    }
};