#include "Gemini  elimination.h"
#include "Gemini without switch.h"
#include "GeminiSketch_Algorithm.h"
#include <sys/time.h>

int main() {
    struct timeval start, end;
    // 记录开始时间
    gettimeofday(&start, NULL);

    // Initialize the working matrix
    WorkingMatrix matrix(10);

    // Create an edge
    Edge e(std::make_pair(1, 2), 10, 1);

    // Insert an edge
    insertion(matrix, e);

    // Vertex query example
    bool result = vertexQuery(matrix, 1, 0, 2);
    if (result) {
        std::cout << "Vertex query successful!" << std::endl;
    } else {
        std::cout << "Vertex query failed!" << std::endl;
    }

    // Total outgoing edge weight query example
    int totalWeight = totalOutgoingWeight(matrix, 1, 0, 2);
    std::cout << "Total outgoing edge weight: " << totalWeight << std::endl;

    // Outgoing edge count query example
    int edgeCount = outgoingEdgeCount(matrix, 1, 0, 2);
    std::cout << "Number of outgoing edges: " << edgeCount << std::endl;

    // Reachability query example
    bool reachable = reachabilityQuery(matrix, std::make_pair(1, 2), 0, 2);
    if (reachable) {
        std::cout << "Reachability query successful!" << std::endl;
    } else {
        std::cout << "Reachability query failed!" << std::endl;
    }

    // 示例：创建子图
    std::vector<Edge> subgraph;
    subgraph.push_back(Edge(std::make_pair(1, 2), 10, 1));

    // Subgraph query example
    int subgraphResult = subgraphQuery(matrix, subgraph, 0, 2);
    if (subgraphResult != -1) {
        std::cout << "Subgraph query successful, total weight: " << subgraphResult << std::endl;
    } else {
        std::cout << "Subgraph query failed!" << std::endl;
    }

    // 记录结束时间
    gettimeofday(&end, NULL);
    // 计算平均哈希链长度
float avgChainLength = averageHashChainLength(matrix);
std::cout << "平均哈希链长度: " << avgChainLength << std::endl;

// 计算耗时（微秒）
    long long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    std::cout << "Query time: " << elapsed_time << " microseconds" << std::endl;
    std::cout << "Throughput: " << 1.0 / (elapsed_time / 1000000.0) << " queries per second" << std::endl;

    return 0;
}