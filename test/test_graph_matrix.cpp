#include "gtest/gtest.h"
#include "GraphMatrix.h"
#include <fstream>
#include <sstream>
#include <cstdio>  // for remove()

// 测试顶点和边的插入与删除
TEST(GraphMatrixTest, InsertAndRemoveVertexEdge) {
    GraphMatrix<int, int> graph;
    // 插入 5 个顶点
    for (int i = 0; i < 5; i++) {
        graph.insert(i);
    }
    EXPECT_EQ(graph.n, 5);
    // 插入边
    graph.insert(100, 0, 1, 1.0);
    graph.insert(200, 1, 2, 2.0);
    graph.insert(300, 2, 3, 3.0);
    EXPECT_TRUE(graph.exists(0, 1));
    EXPECT_TRUE(graph.exists(1, 2));
    EXPECT_TRUE(graph.exists(2, 3));
    // 删除边 (1,2)
    int removedEdge = graph.remove(1, 2);
    EXPECT_EQ(removedEdge, 200);
    EXPECT_FALSE(graph.exists(1, 2));
    // 删除顶点 3
    int vertexData = graph.remove(3);
    EXPECT_EQ(vertexData, 3);
    EXPECT_EQ(graph.n, 4);
}

// 测试 BFS 算法（基类方法）
TEST(GraphMatrixTest, BFSTraversal) {
    // 构造无向图：顶点 0-1, 0-2, 1-3
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 4; i++) {
        graph.insert(i);
    }
    // 无向图边：插入双向边
    graph.insert(10, 0, 1, 1.0);
    graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(20, 2, 0, 1.0);
    graph.insert(30, 1, 3, 1.0);
    graph.insert(30, 3, 1, 1.0);
    graph.bfs(0);
    // 检查所有顶点都已被访问（dTime 不再为初始值 -1）
    for (int i = 0; i < 4; i++) {
        EXPECT_NE(graph.dTime(i), -1);
    }
}

// 测试 DFS 算法（基类方法）
TEST(GraphMatrixTest, DFSTraversal) {
    // 构造有向图：0->1, 0->2, 1->3
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 4; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(30, 1, 3, 1.0);
    graph.dfs(0);
    // 检查各顶点的发现和结束时间
    for (int i = 0; i < 4; i++) {
        EXPECT_NE(graph.dTime(i), -1);
        EXPECT_NE(graph.fTime(i), -1);
    }
}

// 测试基于 DFS 的拓扑排序（基类方法）
// 构造 DAG: 0->1, 0->2, 1->3, 2->3
TEST(GraphMatrixTest, TopologicalSort) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 4; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 0, 2, 1.0);
    graph.insert(30, 1, 3, 1.0);
    graph.insert(40, 2, 3, 1.0);
    // tSort 返回一个栈，栈顶为拓扑排序中的“最后”一个（即源点应出现在最前面）
    Stack<int>* topo = graph.tSort(0);
    std::vector<int> order;
    while (!topo->empty()) {
        order.push_back(topo->pop());
    }
    // 对于这个 DAG，有效拓扑序可能是 {0,1,2,3} 或 {0,2,1,3}，因此检查 0 在最前、3 在最后
    EXPECT_EQ(order.front(), 0);
    EXPECT_EQ(order.back(), 3);
    delete topo;
}

// 测试无向图连通分量
TEST(GraphMatrixTest, ConnectedComponents) {
    // 构造无向图：5 个顶点，0-1-2 连通，3-4 连通
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 5; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 1, 2, 1.0);
    graph.insert(20, 2, 1, 1.0);
    graph.insert(30, 3, 4, 1.0);
    graph.insert(30, 4, 3, 1.0);
    int count = graph.connectedComponents(false);
    EXPECT_EQ(count, 2);
}

// 测试有向图反转
TEST(GraphMatrixTest, ReverseGraph) {
    // 构造有向图：0->1, 1->2
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 2, 1.0);
    // 反转图
    graph.reverse();
    // 现在应存在 1->0 和 2->1，且原来的边不存在
    EXPECT_FALSE(graph.exists(0, 1));
    EXPECT_FALSE(graph.exists(1, 2));
    EXPECT_TRUE(graph.exists(1, 0));
    EXPECT_TRUE(graph.exists(2, 1));
}

// 测试有向图的环检测和 Kosaraju 强连通分量
TEST(GraphMatrixTest, DirectedCycleAndSCC) {
    // 构造有向图：0->1, 1->2, 2->0 形成环，再加一个孤立顶点 3
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 4; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(20, 1, 2, 1.0);
    graph.insert(30, 2, 0, 1.0);
    // 不直接调用私有的 directedCycle，而是调用 kosarajuSCC（内部会先检测环）
    int sccCount = graph.kosarajuSCC(false);
    // 对于图中存在环的情况，强连通分量应为 2：{0,1,2} 和 {3}
    EXPECT_EQ(sccCount, 2);
}


// 测试双连通分量（BCC），这里只是运行一下以保证不会崩溃
TEST(GraphMatrixTest, BCCTest) {
    // 构造无向三角形：0-1, 1-2, 2-0
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 3; i++) {
        graph.insert(i);
    }
    graph.insert(10, 0, 1, 1.0);
    graph.insert(10, 1, 0, 1.0);
    graph.insert(20, 1, 2, 1.0);
    graph.insert(20, 2, 1, 1.0);
    graph.insert(30, 2, 0, 1.0);
    graph.insert(30, 0, 2, 1.0);
    // 调用 bcc 算法（输出信息通过 printf 打印，这里只检测执行过程）
    graph.bcc(0);
    SUCCEED();
}

// 测试 pfs（优先级搜索）方法
TEST(GraphMatrixTest, PFSTest) {
    GraphMatrix<int, int> graph;
    for (int i = 0; i < 4; i++) {
        graph.insert(i);
    }
    // 为测试构造简单图，加几条边
    graph.insert(10, 0, 1, 5.0);
    graph.insert(20, 0, 2, 2.0);
    graph.insert(30, 1, 2, 1.0);
    graph.insert(40, 1, 3, 3.0);
    graph.insert(50, 2, 3, 1.0);
    // 定义一个简单的更新器：如果边权比当前优先级小就更新
    auto updater = [](Graph<int, int>* g, int s, int w) {
        if (g->exists(s, w) && (g->weight(s, w) < g->priority(w))) {
            g->priority(w) = g->weight(s, w);
            g->parent(w) = s;
        }
    };
    graph.pfs(0, updater);
    // 检查顶点 3 的父节点应该来自 1 或 2
    int p = graph.parent(3);
    EXPECT_TRUE(p == 1 || p == 2);
}

// 测试从文件流构造 GraphMatrix
TEST(GraphMatrixTest, ConstructFromFile) {
    // 先写入临时文件
    std::string filename = "temp_graph.txt";
    std::ofstream ofs(filename);
    // 文件格式：顶点数 边数，然后每行一条边（这里构造 DIGRAPH）
    ofs << "3 2\n0 1\n1 2\n";
    ofs.close();
    std::ifstream ifs(filename);
    GraphMatrix<int, int> graph(ifs, GType::DIGRAPH);
    ifs.close();
    std::remove(filename.c_str());
    EXPECT_EQ(graph.n, 3);
    EXPECT_TRUE(graph.exists(0, 1));
    EXPECT_TRUE(graph.exists(1, 2));
    EXPECT_FALSE(graph.exists(2, 0));
}
