#ifndef GEMINI_WITHOUT_SWITCH_H
#define GEMINI_WITHOUT_SWITCH_H

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

int H(int x);

// Vertex query algorithm
bool vertexQuery(const WorkingMatrix& matrix, int v, int t_b, int t_e);

// Calculate the total outgoing edge weight of vertex v within [t_b, t_e]
int totalOutgoingWeight(const WorkingMatrix& matrix, int v, int t_b, int t_e);

// Calculate the number of outgoing edges of vertex v within [t_b, t_e]
int outgoingEdgeCount(const WorkingMatrix& matrix, int v, int t_b, int t_e);

// Insertion operation
void insertion(WorkingMatrix& matrix, Edge e);

// Eliminate expired edges operation
void eliminateExpiredEdges(WorkingMatrix& matrix, int Te);

// Time-related query: Find all active edges within [t_b, t_e]
std::vector<Edge> findActiveEdges(const WorkingMatrix& matrix, int t_b, int t_e);

// Time-related query: Check the relationship between vertices within [t_b, t_e]
bool checkVertexRelationship(const WorkingMatrix& matrix, std::pair<int, int> vertexPair, int t_b, int t_e);

// Reachability query
bool reachabilityQuery(const WorkingMatrix& matrix, std::pair<int, int> startEndPair, int t_b, int t_e);

#endif