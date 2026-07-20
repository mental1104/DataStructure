#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "GraphList.h"
#include "GraphMatrix.h"

namespace {

template<typename GraphType>
void verifyTeachingOwnershipAndMetadata() {
    GraphType graph;
    for (int vertex = 0; vertex < 4; ++vertex)
        graph.insert(vertex);
    graph.insert(10, 0, 1, 1.5);
    graph.insert(20, 0, 2, 2.5);
    graph.insert(30, 2, 3, 3.5);

    GraphType copied(graph);
    copied.vertex(0) = 100;
    copied.edge(0, 1) = 101;
    EXPECT_EQ(graph.vertex(0), 0);
    EXPECT_EQ(graph.edge(0, 1), 10);

    GraphType assigned;
    assigned.insert(-1);
    assigned = graph;
    assigned.vertex(0) = 200;
    EXPECT_EQ(graph.vertex(0), 0);
    EXPECT_EQ(assigned.e, graph.e);

    graph.reverse();
    ASSERT_TRUE(graph.exists(1, 0));
    ASSERT_TRUE(graph.exists(2, 0));
    EXPECT_EQ(graph.edge(1, 0), 10);
    EXPECT_NEAR(graph.weight(1, 0), 1.5, 1e-9);
    EXPECT_EQ(graph.inDegree(0), 2);
    EXPECT_EQ(graph.outDegree(0), 0);

    graph.reverse();
    EXPECT_EQ(graph.remove(1), 1);
    EXPECT_EQ(graph.n, 3);
    ASSERT_TRUE(graph.exists(0, 1));
    EXPECT_EQ(graph.edge(0, 1), 20);
    EXPECT_EQ(graph.outDegree(0), 1);
    EXPECT_EQ(graph.inDegree(1), 1);
    EXPECT_THROW(graph.edge(2, 0), std::out_of_range);
}

} // namespace

TEST(GraphMatrixLifecycleTest, DeepCopyReverseAndReindex) {
    verifyTeachingOwnershipAndMetadata<GraphMatrix<int, int> >();
}

TEST(GraphListLifecycleTest, DeepCopyReverseAndReindex) {
    verifyTeachingOwnershipAndMetadata<GraphList<int, int> >();
}
