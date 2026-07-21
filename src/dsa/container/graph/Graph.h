#ifndef DSA_CONTAINER_GRAPH_GRAPH_H
#define DSA_CONTAINER_GRAPH_GRAPH_H

#include <cstddef>
#include <vector>

#include "../../core/graph/GraphAlgorithm.h"

namespace dsa {
namespace container {

/// 工业图的无存储 CRTP 门面。
///
/// Derived 只负责顶点、边和邻接关系的所有权；全部遍历状态由返回结果持有，
/// 因而连续调用不同算法不会污染图本身，也无需 reset 图中的永久数据。
template<typename Tv, typename Te, typename Derived>
class Graph {
public:
    typedef Tv vertex_type;
    typedef Te edge_type;
    typedef std::size_t size_type;
    typedef int vertex_id;
    typedef dsa::core::graph::TraversalResult traversal_result;
    typedef dsa::core::graph::TopologicalResult topological_result;
    typedef dsa::core::graph::ComponentResult component_result;
    typedef dsa::core::graph::PathResult path_result;
    typedef dsa::core::graph::SpanningForestResult spanning_forest_result;

    /// 返回覆盖所有连通域的 BFS 森林及访问顺序。
    traversal_result breadthFirst(vertex_id start = 0) const {
        const View view(derived());
        return Algorithm::breadthFirst(view, start);
    }

    /// 返回覆盖所有连通域的 DFS 森林及访问顺序。
    traversal_result depthFirst(vertex_id start = 0) const {
        const View view(derived());
        return Algorithm::depthFirst(view, start);
    }

    /// 返回拓扑排序；有向环存在时 acyclic=false。
    topological_result topologicalSort(vertex_id start = 0) const {
        const View view(derived());
        return Algorithm::topologicalSort(view, start);
    }

    /// 判断无向视角下是否存在环。
    bool hasUndirectedCycle() const {
        const View view(derived());
        return Algorithm::hasUndirectedCycle(view);
    }

    /// 判断有向图是否存在环。
    bool hasDirectedCycle() const {
        const View view(derived());
        return Algorithm::hasDirectedCycle(view);
    }

    /// 计算按出边可达关系划分的连通分量。
    component_result connectedComponents() const {
        const View view(derived());
        return Algorithm::connectedComponents(view);
    }

    /// 返回从 source 通过出边可达的顶点集合标记。
    std::vector<bool> reachableFrom(vertex_id source) const {
        const View view(derived());
        return Algorithm::reachable(view, source);
    }

    /// 计算所有强连通分量；无环图中的单顶点分量也会被返回。
    component_result stronglyConnectedComponents() const {
        const View view(derived());
        return Algorithm::stronglyConnectedComponents(view);
    }

    /// Dijkstra 单源最短路；负权边会触发 domain_error。
    path_result dijkstra(vertex_id source) const {
        const View view(derived());
        return Algorithm::dijkstra(view, source);
    }

    /// Prim 最小生成树；只覆盖 source 所在连通分量。
    path_result prim(vertex_id source = 0) const {
        const View view(derived());
        return Algorithm::prim(view, source);
    }

    /// Kruskal 最小生成森林。
    spanning_forest_result kruskal() const {
        const View view(derived());
        return Algorithm::kruskal(view);
    }

protected:
    Graph() {
    }

    ~Graph() {
    }

private:
    /// 将 Derived 的公开结构查询适配为共享 GraphAlgorithm 的最小 View contract。
    class View {
    public:
        explicit View(const Derived& graph)
            : graph_(graph) {
        }

        std::size_t vertexCount() const {
            return graph_.vertexCount();
        }

        int firstNeighbor(int vertex) const {
            return graph_.firstNeighbor(vertex);
        }

        int nextNeighbor(int vertex, int current) const {
            return graph_.nextNeighbor(vertex, current);
        }

        bool containsEdge(int from, int to) const {
            return graph_.containsEdge(from, to);
        }

        double edgeWeight(int from, int to) const {
            return graph_.edgeWeight(from, to);
        }

    private:
        const Derived& graph_;
    };

    typedef dsa::core::graph::GraphAlgorithm<View> Algorithm;

    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
};

} // namespace container
} // namespace dsa

#endif
