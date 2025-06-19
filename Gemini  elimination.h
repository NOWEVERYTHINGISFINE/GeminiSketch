#ifndef GEMINI_ELIMINATION_H
#define GEMINI_ELIMINATION_H

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
    int ec; // 边计数
    int CF; // 标记
    std::vector<Edge> list; // 边列表
    int GT; // 时间戳
    Bucket* bqp; // 桶队列指针
    Bucket() : ec(0), CF(0), GT(0), bqp(nullptr) {}
};

// Define the working matrix structure
struct WorkingMatrix {
    std::vector<std::vector<Bucket>> G; // 矩阵
    int WS; // 工作状态
    Bucket* HP; // 头指针
    Bucket* MP; // 中间指针
    Bucket* TP; // 尾指针
    WorkingMatrix(int size) : WS(0), HP(nullptr), MP(nullptr), TP(nullptr) {
        G.resize(size, std::vector<Bucket>(size));
    }
};

int H(int x);

// Insertion operation
void insertion(WorkingMatrix& matrix, Edge e);

// Rolling-out elimination strategy
void rollingOutElimination(WorkingMatrix& matrix, int Te);

// Full scan elimination strategy
void fullScanElimination(WorkingMatrix& matrix, int Te);

// Lazy elimination strategy
void lazyElimination(WorkingMatrix& matrix, Edge e, int Te);

// Temporal graph edge query algorithm
bool temporalEdgeQuery(WorkingMatrix& matrix, std::pair<int, int> edge, int start_time, int end_time);

// New: Subgraph query algorithm
int subgraphQuery(const WorkingMatrix& matrix, const std::vector<Edge>& subgraph, int t_b, int t_e);

// New: Reachability query algorithm
bool reachabilityQuery(const WorkingMatrix& matrix, std::pair<int, int> startEndPair, int t_b, int t_e);

#endif