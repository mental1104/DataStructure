#include "gtest/gtest.h"
#include "GraphList.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>  // for remove()

namespace {
std::string writeTempFile(const std::string& name, const std::string& content) {
    std::ofstream ofs(name);
    ofs << content;
    return name;
}
}

// 直接复用 GraphMatrix 的测试逻辑，只是类型替换为 GraphList

TEST(GraphListTest, InsertAndRemoveVertexEdge) {
    GraphList<int, int> graph;
    for (int i = 0; i < 5; i++) graph.insert(i);
    EXPECT_EQ(graph.n, 5);
    graph.insert(100, 0, 1, 1.0);
    graph.insert(200, 1, 2, 2.0);
    graph.insert(300, 2, 3, 3.0);
    EXPECT_TRUE(graph.exists(0, 1));
    EXPECT_TRUE(graph.exists(1, 2));
    EXPECT_TRUE(graph.exists(2, 3));
    int removedEdge = graph.remove(1, 2);
    EXPECT_EQ(removedEdge, 200);
    EXPECT_FALSE(graph.exists(1, 2));
    int vertexData = graph.remove(3);
    EXPECT_EQ(vertexData, 3);
    EXPECT_EQ(graph.n, 4);
}

TEST(GraphListTest, InsertReturnValue) {
    GraphList<int, int> graph;
    int idx = graph.insert(42);
    EXPECT_EQ(idx, 0);
}

TEST(GraphListTest, AccessorsAndEdgeData) {
    GraphList<int, int> graph;
    for (int i = 0; i < 2; i++) graph.insert(i);
    graph.insert(42, 0, 1, 3.0);

    EXPECT_EQ(graph.outDegree(0), 1);
    EXPECT_EQ(graph.inDegree(1), 1);
    EXPECT_EQ(graph.edge(0, 1), 42);
}

TEST(GraphListTest, RemoveVertexUpdatesDegrees) {
    GraphList<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);

    graph.remove(0);
    EXPECT_EQ(graph.n, 2);
    EXPECT_EQ(graph.inDegree(0), 0);
    EXPECT_EQ(graph.inDegree(1), 0);
}

TEST(GraphListTest, BFSTraversal) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0); graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 0, 2, 1.0); graph.insert(20, 2, 0, 1.0);
    graph.insert(30, 1, 3, 1.0); graph.insert(30, 3, 1, 1.0);
    graph.bfs(0);
    for (int i = 0; i < 4; i++) EXPECT_NE(graph.dTime(i), -1);
}

TEST(GraphListTest, DFSTraversal) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(30, 1, 3, 1.0);
    graph.dfs(0);
    for (int i = 0; i < 4; i++) {
        EXPECT_NE(graph.dTime(i), -1);
        EXPECT_NE(graph.fTime(i), -1);
    }
}

TEST(GraphListTest, TopologicalSort) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(30, 1, 3, 1.0);
    graph.insert(40, 2, 3, 1.0);
    Stack<int>* topo = graph.tSort(0);
    std::vector<int> order;
    while (!topo->empty()) order.push_back(topo->pop());
    EXPECT_EQ(order.front(), 0);
    EXPECT_EQ(order.back(), 3);
    delete topo;
}

TEST(GraphListTest, ConnectedComponents) {
    GraphList<int, int> graph;
    for (int i = 0; i < 5; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0); graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 1, 2, 1.0); graph.insert(20, 2, 1, 1.0);
    graph.insert(30, 3, 4, 1.0); graph.insert(30, 4, 3, 1.0);
    int count = graph.connectedComponents(false);
    EXPECT_EQ(count, 2);
}

TEST(GraphListTest, ReverseGraph) {
    GraphList<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 2, 1.0);
    graph.reverse();
    EXPECT_FALSE(graph.exists(0, 1));
    EXPECT_FALSE(graph.exists(1, 2));
    EXPECT_TRUE(graph.exists(1, 0));
    EXPECT_TRUE(graph.exists(2, 1));
}

TEST(GraphListTest, DirectedCycleAndSCC) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 2, 1.0);
    graph.insert(30, 2, 0, 1.0);
    int sccCount = graph.kosarajuSCC(false);
    EXPECT_EQ(sccCount, 2); // {0,1,2} and {3}
}

TEST(GraphListTest, BCCTest) {
    GraphList<int, int> graph;
    for (int i = 0; i < 3; i++) graph.insert(i);
    graph.insert(10, 0, 1, 1.0); graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 1, 2, 1.0); graph.insert(20, 2, 1, 1.0);
    graph.insert(30, 2, 0, 1.0); graph.insert(30, 0, 2, 1.0);
    graph.bcc(0);
    SUCCEED();
}

TEST(GraphListTest, PFSTest) {
    GraphList<int, int> graph;
    for (int i = 0; i < 4; i++) graph.insert(i);
    graph.insert(10, 0, 1, 5.0);
    graph.insert(20, 0, 2, 2.0);
    graph.insert(30, 1, 2, 1.0);
    graph.insert(40, 1, 3, 3.0);
    graph.insert(50, 2, 3, 1.0);
    auto updater = [](Graph<int, int>* g, int s, int w) {
        if (g->exists(s, w) && (g->weight(s, w) < g->priority(w))) {
            g->priority(w) = g->weight(s, w);
            g->parent(w) = s;
        }
    };
    graph.pfs(0, updater);
    int p = graph.parent(3);
    EXPECT_TRUE(p == 1 || p == 2);
}

TEST(GraphListTest, ConstructFromFile) {
    std::string filename = "temp_graph.txt";
    std::ofstream ofs(filename);
    ofs << "3 2\n0 1\n1 2\n";
    ofs.close();
    std::ifstream ifs(filename);
    GraphList<int, int> graph(ifs, GType::DIGRAPH);
    ifs.close();
    std::remove(filename.c_str());
    EXPECT_EQ(graph.n, 3);
    EXPECT_TRUE(graph.exists(0, 1));
    EXPECT_TRUE(graph.exists(1, 2));
    EXPECT_FALSE(graph.exists(2, 0));
}

TEST(GraphListTest, ConstructFromFileVariants) {
    const std::string unweighted = writeTempFile("temp_graph_unweighted.txt", "3 2\n0 1\n1 2\n");
    const std::string weighted = writeTempFile("temp_graph_weighted.txt", "3 2\n0 1 1.5\n1 2 2.5\n");

    {
        std::ifstream ifs(unweighted);
        GraphList<int, int> graph(ifs, GType::UNDIGRAPH);
        EXPECT_TRUE(graph.exists(0, 1));
        EXPECT_TRUE(graph.exists(1, 0));
        EXPECT_TRUE(graph.exists(1, 2));
        EXPECT_TRUE(graph.exists(2, 1));
    }

    {
        std::ifstream ifs(weighted);
        GraphList<int, int> graph(ifs, GType::WEIGHTEDDIGRAPH);
        EXPECT_TRUE(graph.exists(0, 1));
        EXPECT_TRUE(graph.exists(1, 2));
        EXPECT_FALSE(graph.exists(1, 0));
    }

    {
        std::ifstream ifs(weighted);
        GraphList<int, int> graph(ifs, GType::WEIGHTEDUNDIGRAPH);
        EXPECT_TRUE(graph.exists(0, 1));
        EXPECT_TRUE(graph.exists(1, 0));
        EXPECT_TRUE(graph.exists(1, 2));
        EXPECT_TRUE(graph.exists(2, 1));
    }

    std::remove(unweighted.c_str());
    std::remove(weighted.c_str());
}
