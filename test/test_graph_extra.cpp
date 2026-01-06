#include <gtest/gtest.h>

#define private public
#include "Graph.h"
#undef private

#include "GraphList.h"
#include "GraphMatrix.h"
#include "GraphObserver.h"
#include "PU.h"

namespace {
template <typename Tv, typename Te>
struct RecordingObserver : GraphObserver<Tv, Te> {
    int scc_calls = 0;
    int kruskal_edge_calls = 0;
    bool kruskal_done = false;
    int kruskal_edges = 0;
    double total_weight = 0.0;

    void onSCCComponent(const Vector<int>& nodes) override {
        (void)nodes;
        ++scc_calls;
    }

    void onKruskalEdge(int /*u*/, int /*v*/, double /*w*/) override {
        ++kruskal_edge_calls;
    }

    void onKruskalDone(double total, const Vector<KruskalEdgeSummary<Tv, Te>>& edges) override {
        kruskal_done = true;
        total_weight = total;
        kruskal_edges = edges.size();
    }
};

class GraphMatrixGhost : public GraphMatrix<int, int> {
public:
    int firstNbr(int i) override { return (i == 0) ? 1 : -1; }
    int nextNbr(int /*i*/, int /*j*/) override { return -1; }
    bool exists(int /*i*/, int /*j*/) override { return false; }
};
}

TEST(GraphCoreTest, EdgeComparison) {
    Edge<int> e1(1, 2.0, 0, 1);
    Edge<int> e2(2, 1.0, 1, 2);
    Edge<int> e3;
    EXPECT_TRUE(e1 < e2);
    EXPECT_FALSE(e2 < e1);
    (void)e3;
}

TEST(GraphAlgorithmTest, DFSBackAndCrossEdges) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(30, 1, 0, 1.0);
    graph.insert(40, 1, 2, 1.0);

    graph.dfs(0);

    EXPECT_EQ(graph.type(1, 0), EType::BACKWARD);
    EXPECT_EQ(graph.type(1, 2), EType::CROSS);
}

TEST(GraphAlgorithmTest, TSortDetectsBackEdge) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 2; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 0, 1.0);

    Stack<int>* order = new Stack<int>;
    int clock = 0;
    graph.TSort(0, clock, order);

    EXPECT_EQ(graph.type(1, 0), EType::BACKWARD);
    delete order;
}

TEST(GraphAlgorithmTest, DirectedCycleEarlyReturn) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 2, 1.0);
    graph.insert(20, 2, 0, 1.0);
    graph.insert(30, 0, 1, 1.0);

    Stack<int>* topo = graph.tSort(0);
    EXPECT_TRUE(topo->empty());
    delete topo;
}

TEST(GraphAlgorithmTest, ConnectedComponentsObserver) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 2, 3, 1.0);
    graph.insert(20, 3, 2, 1.0);

    RecordingObserver<int, int> observer;
    graph.setObserver(&observer);
    int count = graph.connectedComponents(true);

    EXPECT_EQ(count, 2);
    EXPECT_EQ(observer.scc_calls, 2);
}

TEST(GraphAlgorithmTest, KosarajuObserver) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 0, 1.0);

    RecordingObserver<int, int> observer;
    graph.setObserver(&observer);
    int count = graph.kosarajuSCC(true);

    EXPECT_EQ(count, 2);
    EXPECT_EQ(observer.scc_calls, 2);
}

TEST(GraphAlgorithmTest, KruskalObserver) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 1, 2, 2.0);
    graph.insert(20, 2, 1, 2.0);
    graph.insert(30, 0, 2, 3.0);
    graph.insert(30, 2, 0, 3.0);

    RecordingObserver<int, int> observer;
    graph.setObserver(&observer);
    graph.kruskal(true);

    EXPECT_TRUE(observer.kruskal_edge_calls >= 2);
    EXPECT_TRUE(observer.kruskal_done);
    EXPECT_TRUE(observer.total_weight > 0.0);
    EXPECT_EQ(observer.kruskal_edges, observer.kruskal_edge_calls);
    EXPECT_EQ(graph.status(0), VStatus::VISITED);
    EXPECT_EQ(graph.status(1), VStatus::VISITED);
    EXPECT_EQ(graph.status(2), VStatus::VISITED);
    EXPECT_TRUE(graph.type(0, 1) == EType::TREE || graph.type(1, 0) == EType::TREE);
}

TEST(GraphAlgorithmTest, PUStrategiesUpdate) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 2; i++) graph.insert(i);
    graph.insert(10, 0, 1, 5.0);

    graph.priority(0) = 0.0;
    graph.priority(1) = 100.0;
    graph.status(0) = VStatus::VISITED;
    graph.status(1) = VStatus::UNDISCOVERED;

    DijkstraPU<int, int> dijkstra;
    dijkstra(&graph, 0, 1);
    EXPECT_EQ(graph.parent(1), 0);
    EXPECT_EQ(graph.priority(1), 5.0);

    graph.priority(1) = 100.0;
    graph.parent(1) = -1;

    PrimPU<int, int> prim;
    prim(&graph, 0, 1);
    EXPECT_EQ(graph.parent(1), 0);
    EXPECT_EQ(graph.priority(1), 5.0);
}

TEST(GraphAlgorithmTest, BCCSkipsMissingEdges) {
    GraphMatrixGhost graph;
    graph.insert(0);
    graph.insert(1);

    graph.bcc(0);
    SUCCEED();
}
