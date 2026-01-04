#include <gtest/gtest.h>
#include "GraphObserver.h"

TEST(GraphObserverTest, DefaultCallbacks) {
    GraphObserver<int, int> observer;
    Vector<int> nodes;
    nodes.insert(1);
    nodes.insert(2);
    observer.onSCCComponent(nodes);
    observer.onKruskalEdge(1, 2, 3.5);

    Vector<KruskalEdgeSummary<int, int>> edges;
    KruskalEdgeSummary<int, int> summary{1, 2};
    edges.insert(summary);
    observer.onKruskalDone(3.5, edges);
}
