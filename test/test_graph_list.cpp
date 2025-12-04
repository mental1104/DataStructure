#include "gtest/gtest.h"
#include "GraphList.h"
#include <fstream>
#include <sstream>
#include <cstdio>  // for remove()

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
