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
    int WS; // working status
    Bucket* HP; // head pointer
    Bucket* MP; // middle pointer
    Bucket* TP; // tail pointer
    WorkingMatrix(int size) : WS(0), HP(nullptr), MP(nullptr), TP(nullptr) {
        G.resize(size, std::vector<Bucket>(size));
    }
};

int H(int x) {
    return XXH32(&x, sizeof(x), 0) % 10;
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

// Rolling-out elimination strategy
void rollingOutElimination(WorkingMatrix& matrix, int Te) {
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

// Full scan elimination strategy
void fullScanElimination(WorkingMatrix& matrix, int Te) {
    for (auto& row : matrix.G) {
        for (auto& bucket : row) {
            while (!bucket.list.empty() && bucket.list.front().time <= Te) {
                bucket.ec -= 1;
                if (bucket.ec == 0) {
                    bucket.vx = std::make_pair(0, 0);
                    bucket.CF = 0;
                    bucket.GT = 0;
                    bucket.bqp = nullptr;
                }
                bucket.list.erase(bucket.list.begin());
            }
        }
    }
}

// Lazy elimination strategy
void lazyElimination(WorkingMatrix& matrix, Edge e, int Te) {
    int i = H(e.sd.first);
    int j = H(e.sd.second);
    Bucket& bucket = matrix.G[i][j];
    while (!bucket.list.empty() && bucket.list.front().time <= Te) {
        bucket.ec -= 1;
        if (bucket.ec == 0) {
            bucket.vx = std::make_pair(0, 0);
            bucket.CF = 0;
            bucket.GT = 0;
            bucket.bqp = nullptr;
        }
        bucket.list.erase(bucket.list.begin());
    }
}

// Temporal graph edge query algorithm
// Define the value of g, which can be adjusted according to actual conditions
const int g = 1;

// Modify the vertex query function to include the chain hashing compensation mechanism (assuming there is a similar function in the file, taking temporalEdgeQuery as an example)
bool temporalEdgeQuery(WorkingMatrix& matrix, std::pair<int, int> edge, int start_time, int end_time) {
    int i = H(edge.first);
    for (int offset = 0; offset <= g; ++offset) {
        int adjusted_i = (i + offset) % matrix.G.size();
        int j = H(edge.second);
        Bucket& bucket = matrix.G[adjusted_i][j];

        if (bucket.vx == edge) {
            for (const auto& e : bucket.list) {
                if (e.sd == edge && e.time >= start_time && e.time <= end_time) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Other query functions can be modified similarly

// New: Subgraph query algorithm
int subgraphQuery(const WorkingMatrix& matrix, const std::vector<Edge>& subgraph, int t_b, int t_e) {
    std::vector<Edge> activeEdges = findActiveEdges(matrix, t_b, t_e);
    int totalWeight = 0;

    for (const auto& subEdge : subgraph) {
        bool found = false;
        for (const auto& edge : activeEdges) {
            if (subEdge.sd == edge.sd && edge.time >= t_b && edge.time <= t_e) {
                found = true;
                totalWeight += edge.weight;
                break;
            }
        }
        if (!found) {
            return -1; 
        }
    }

    return totalWeight;
}

// New: Reachability query algorithm
bool reachabilityQuery(const WorkingMatrix& matrix, std::pair<int, int> startEndPair, int t_b, int t_e) {
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


int main() {
    WorkingMatrix matrix(10);
    Edge e(std::make_pair(1, 2), 10, 1);
    insertion(matrix, e);

    rollingOutElimination(matrix, 2);
    fullScanElimination(matrix, 2);
    lazyElimination(matrix, e, 2);

    return 0;

}
