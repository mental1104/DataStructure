#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "dsa/container/graph/GraphList.h"
#include "dsa/container/graph/GraphMatrix.h"

namespace {

template<typename GraphType>
void buildDag(GraphType& graph) {
    for (int i = 0; i < 5; ++i)
        graph.addVertex(std::string(1, static_cast<char>('A' + i)));
    EXPECT_TRUE(graph.addEdge(0, 1, std::string("01"), 1.0));
    EXPECT_TRUE(graph.addEdge(0, 2, std::string("02"), 4.0));
    EXPECT_TRUE(graph.addEdge(1, 2, std::string("12"), 2.0));
    EXPECT_TRUE(graph.addEdge(1, 3, std::string("13"), 5.0));
    EXPECT_TRUE(graph.addEdge(2, 3, std::string("23"), 1.0));
    EXPECT_FALSE(graph.addEdge(0, 1, std::string("duplicate"), 9.0));
}

template<typename GraphType>
void verifySharedAlgorithms(GraphType& graph) {
    const dsa::core::graph::TraversalResult bfs = graph.breadthFirst(0);
    const dsa::core::graph::TraversalResult dfs = graph.depthFirst(0);

    EXPECT_EQ(bfs.order.size(), 5U);
    EXPECT_EQ(dfs.order.size(), 5U);
    EXPECT_EQ(bfs.state.parent[1], 0);
    EXPECT_EQ(bfs.state.status[0], dsa::core::graph::VertexStatus::SOURCE);

    const dsa::core::graph::TopologicalResult topological = graph.topologicalSort(0);
    ASSERT_TRUE(topological.acyclic);
    ASSERT_EQ(topological.order.size(), 5U);
    std::vector<int> position(5, -1);
    for (std::size_t index = 0; index < topological.order.size(); ++index)
        position[static_cast<std::size_t>(topological.order[index])] = static_cast<int>(index);
    EXPECT_LT(position[0], position[1]);
    EXPECT_LT(position[0], position[2]);
    EXPECT_LT(position[1], position[3]);
    EXPECT_LT(position[2], position[3]);

    const dsa::core::graph::PathResult paths = graph.dijkstra(0);
    EXPECT_NEAR(paths.state.priority[3], 4.0, 1e-9);
    EXPECT_EQ(paths.state.parent[3], 2);
    EXPECT_EQ(paths.state.priority[4], std::numeric_limits<double>::infinity());

    const dsa::core::graph::ComponentResult components = graph.stronglyConnectedComponents();
    EXPECT_EQ(components.count, 5);
    EXPECT_FALSE(graph.hasDirectedCycle());

    // 结果状态外置：运行多个算法不会改变图中的永久顶点或边值。
    EXPECT_EQ(graph.vertex(0), "A");
    EXPECT_EQ(graph.edge(0, 1), "01");
}

template<typename GraphType>
void verifyCopyReverseAndRemoval(GraphType& graph) {
    GraphType copied(graph);
    copied.vertex(0) = "changed";
    copied.edge(0, 1) = "changed-edge";
    EXPECT_EQ(graph.vertex(0), "A");
    EXPECT_EQ(graph.edge(0, 1), "01");

    GraphType assigned;
    assigned = graph;
    assigned.vertex(0) = "assigned";
    EXPECT_EQ(graph.vertex(0), "A");

    graph.reverseEdges();
    ASSERT_TRUE(graph.containsEdge(1, 0));
    EXPECT_EQ(graph.edge(1, 0), "01");
    EXPECT_NEAR(graph.edgeWeight(1, 0), 1.0, 1e-9);
    graph.reverseEdges();

    EXPECT_EQ(graph.removeVertex(1), "B");
    EXPECT_EQ(graph.vertexCount(), 4U);
    ASSERT_TRUE(graph.containsEdge(0, 1));
    EXPECT_EQ(graph.edge(0, 1), "02");
    EXPECT_EQ(graph.outDegree(0), 1U);
    EXPECT_EQ(graph.inDegree(1), 1U);
    EXPECT_THROW(graph.edge(3, 0), std::out_of_range);
}

template<typename GraphType>
void verifyMst() {
    GraphType graph;
    for (int vertex = 0; vertex < 4; ++vertex)
        graph.addVertex(vertex);

    const int edges[][3] = {{0, 1, 1}, {1, 2, 2}, {0, 2, 4}, {2, 3, 1}};
    for (std::size_t index = 0; index < 4; ++index) {
        graph.addEdge(edges[index][0], edges[index][1], edges[index][2], edges[index][2]);
        graph.addEdge(edges[index][1], edges[index][0], edges[index][2], edges[index][2]);
    }

    EXPECT_TRUE(graph.hasUndirectedCycle());
    const dsa::core::graph::PathResult prim = graph.prim(0);
    EXPECT_EQ(prim.state.parent[1], 0);

    const dsa::core::graph::SpanningForestResult kruskal = graph.kruskal();
    EXPECT_EQ(kruskal.edges.size(), 3U);
    EXPECT_NEAR(kruskal.total_weight, 4.0, 1e-9);
}

template<typename GraphType>
void verifyNegativeWeightRejected() {
    GraphType graph;
    graph.addVertex(0);
    graph.addVertex(1);
    graph.addEdge(0, 1, 1, -1.0);
    EXPECT_THROW(graph.dijkstra(0), std::domain_error);
}

template<typename GraphType>
void verifyMoveOnlyValues() {
    GraphType graph;
    graph.addVertex(std::unique_ptr<int>(new int(7)));
    graph.addVertex(std::unique_ptr<int>(new int(8)));
    graph.addEdge(0, 1, std::unique_ptr<int>(new int(9)), 3.0);

    EXPECT_EQ(*graph.vertex(0), 7);
    EXPECT_EQ(*graph.edge(0, 1), 9);

    GraphType moved(std::move(graph));
    EXPECT_EQ(moved.vertexCount(), 2U);
    EXPECT_EQ(*moved.edge(0, 1), 9);
}

} // namespace

TEST(ContainerGraphMatrixTest, SharedAlgorithmsAndExternalState) {
    dsa::container::GraphMatrix<std::string, std::string> graph;
    buildDag(graph);
    verifySharedAlgorithms(graph);
}

TEST(ContainerGraphListTest, SharedAlgorithmsAndExternalState) {
    dsa::container::GraphList<std::string, std::string> graph;
    buildDag(graph);
    verifySharedAlgorithms(graph);
}

TEST(ContainerGraphMatrixTest, CopyReverseAndVertexRemoval) {
    dsa::container::GraphMatrix<std::string, std::string> graph;
    buildDag(graph);
    verifyCopyReverseAndRemoval(graph);
}

TEST(ContainerGraphListTest, CopyReverseAndVertexRemoval) {
    dsa::container::GraphList<std::string, std::string> graph;
    buildDag(graph);
    verifyCopyReverseAndRemoval(graph);
}

TEST(ContainerGraphTest, MatrixAndListShareMstSemantics) {
    verifyMst<dsa::container::GraphMatrix<int, int> >();
    verifyMst<dsa::container::GraphList<int, int> >();
}

TEST(ContainerGraphTest, DijkstraRejectsReachableNegativeEdges) {
    verifyNegativeWeightRejected<dsa::container::GraphMatrix<int, int> >();
    verifyNegativeWeightRejected<dsa::container::GraphList<int, int> >();
}

TEST(ContainerGraphTest, SupportsMoveOnlyVertexAndEdgeValues) {
    verifyMoveOnlyValues<
        dsa::container::GraphMatrix<std::unique_ptr<int>, std::unique_ptr<int> >
    >();
    verifyMoveOnlyValues<
        dsa::container::GraphList<std::unique_ptr<int>, std::unique_ptr<int> >
    >();
}
