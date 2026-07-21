#ifndef DSA_CORE_GRAPH_GRAPH_ALGORITHM_H
#define DSA_CORE_GRAPH_GRAPH_ALGORITHM_H

#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

#include <dsa/algorithm/Sort.h>
#include <dsa/container/heap/Heap.h>
#include <dsa/container/queue/Queue.h>
#include <dsa/container/vector/Vector.h>

namespace dsa {
namespace core {
namespace graph {

/// 顶点在一次算法运行中的访问状态；工业图把它保存在结果对象中，教学图再同步回顶点元数据。
enum class VertexStatus {
    SOURCE,
    UNDISCOVERED,
    DISCOVERED,
    VISITED
};

/// 边在一次遍历中的分类；不属于图的永久结构状态。
enum class EdgeType {
    UNDETERMINED,
    TREE,
    CROSS,
    FORWARD,
    BACKWARD
};

/// 一次图算法的外置状态，避免工业图把遍历时间、父节点和优先级永久塞进顶点对象。
struct TraversalState {
    dsa::container::Vector<VertexStatus> status;
    dsa::container::Vector<int> discovery_time;
    dsa::container::Vector<int> finish_time;
    dsa::container::Vector<int> parent;
    dsa::container::Vector<double> priority;

    struct EdgeClassification {
        int from;
        int to;
        EdgeType type;

        EdgeClassification(int source, int target, EdgeType edgeType)
            : from(source), to(target), type(edgeType) {}
    };

    dsa::container::Vector<EdgeClassification> edge_type;

    explicit TraversalState(std::size_t count = 0) {
        reset(count);
    }

    /// 重置为 count 个未发现顶点，并清空所有临时边分类。
    void reset(std::size_t count) {
        status.assign(count, VertexStatus::UNDISCOVERED);
        discovery_time.assign(count, -1);
        finish_time.assign(count, -1);
        parent.assign(count, -1);
        priority.assign(count, std::numeric_limits<double>::infinity());
        edge_type.clear();
    }

    /// 记录一条真实边在本次算法中的分类。
    void setEdgeType(int from, int to, EdgeType type) {
        for (dsa::container::Vector<EdgeClassification>::iterator it = edge_type.begin();
             it != edge_type.end(); ++it) {
            if (it->from == from && it->to == to) {
                it->type = type;
                return;
            }
        }
        edge_type.push_back(EdgeClassification(from, to, type));
    }

    /// 查询边分类；未触达的边保持 UNDETERMINED。
    EdgeType edgeType(int from, int to) const {
        for (dsa::container::Vector<EdgeClassification>::const_iterator it = edge_type.begin();
             it != edge_type.end(); ++it) {
            if (it->from == from && it->to == to)
                return it->type;
        }
        return EdgeType::UNDETERMINED;
    }
};

/// BFS/DFS 的访问顺序和完整外置状态。
struct TraversalResult {
    TraversalState state;
    dsa::container::Vector<int> order;

    explicit TraversalResult(std::size_t count = 0)
        : state(count) {
    }
};

/// 拓扑排序结果；存在有向环时 acyclic 为 false 且 order 为空。
struct TopologicalResult {
    TraversalState state;
    dsa::container::Vector<int> order;
    bool acyclic;

    explicit TopologicalResult(std::size_t count = 0)
        : state(count), acyclic(true) {
    }
};

/// 连通分量或强连通分量结果，component_of[v] 给出顶点所属分量编号。
struct ComponentResult {
    int count;
    dsa::container::Vector<int> component_of;
    dsa::container::Vector<dsa::container::Vector<int> > components;

    explicit ComponentResult(std::size_t vertex_count = 0)
        : count(0), component_of(vertex_count, -1) {
    }
};

/// 单源路径或 Prim 的结果。
struct PathResult {
    TraversalState state;

    explicit PathResult(std::size_t count = 0)
        : state(count) {
    }
};

/// 生成树/森林中的一条边。
struct SpanningEdge {
    int from;
    int to;
    double weight;

    SpanningEdge(int source, int target, double edge_weight)
        : from(source), to(target), weight(edge_weight) {
    }
};

/// Kruskal 结果；state 仅记录被选边和涉及顶点的访问状态。
struct SpanningForestResult {
    TraversalState state;
    dsa::container::Vector<SpanningEdge> edges;
    double total_weight;

    explicit SpanningForestResult(std::size_t count = 0)
        : state(count), total_weight(0.0) {
    }
};

/// 图算法共享实现。
///
/// View contract:
///   std::size_t vertexCount() const
///   int firstNeighbor(int vertex) const
///   int nextNeighbor(int vertex, int current) const
///   bool containsEdge(int from, int to) const
///   double edgeWeight(int from, int to) const
///
/// View 只暴露图的结构语义，不暴露矩阵、邻接表、allocator 或节点所有权。
template<typename View>
class GraphAlgorithm {
public:
    /// 从 start 开始并按顶点编号轮转，生成覆盖所有连通域的 BFS 森林。
    static TraversalResult breadthFirst(const View& view, int start) {
        const int count = checkedCount(view);
        TraversalResult result(static_cast<std::size_t>(count));
        if (count == 0)
            return result;
        checkVertex(start, count);

        int clock = 0;
        int vertex = start;
        do {
            if (result.state.status[vertex] == VertexStatus::UNDISCOVERED) {
                breadthFirstComponent(view, vertex, result, clock);
                result.state.status[vertex] = VertexStatus::SOURCE;
            }
            vertex = (vertex + 1) % count;
        } while (vertex != start);
        return result;
    }

    /// 从 start 开始并按顶点编号轮转，生成覆盖所有连通域的 DFS 森林。
    static TraversalResult depthFirst(const View& view, int start) {
        const int count = checkedCount(view);
        TraversalResult result(static_cast<std::size_t>(count));
        if (count == 0)
            return result;
        checkVertex(start, count);

        int clock = 0;
        int vertex = start;
        do {
            if (result.state.status[vertex] == VertexStatus::UNDISCOVERED) {
                depthFirstVisit(view, vertex, result, clock);
                result.state.status[vertex] = VertexStatus::SOURCE;
            }
            vertex = (vertex + 1) % count;
        } while (vertex != start);
        return result;
    }

    /// 基于 DFS 生成拓扑序；发现后向边时返回 acyclic=false。
    static TopologicalResult topologicalSort(const View& view, int start) {
        const int count = checkedCount(view);
        TopologicalResult result(static_cast<std::size_t>(count));
        if (count == 0)
            return result;
        checkVertex(start, count);

        int clock = 0;
        dsa::container::Vector<int> postorder;
        int vertex = start;
        do {
            if (result.state.status[vertex] == VertexStatus::UNDISCOVERED &&
                !topologicalVisit(view, vertex, result, postorder, clock)) {
                result.acyclic = false;
                result.order.clear();
                return result;
            }
            vertex = (vertex + 1) % count;
        } while (vertex != start);

        result.order.assign(postorder.rbegin(), postorder.rend());
        return result;
    }

    /// 判断无向视角下是否存在环；调用方负责保证每条无向边的表示语义一致。
    static bool hasUndirectedCycle(const View& view) {
        const int count = checkedCount(view);
        dsa::container::Vector<bool> marked(static_cast<std::size_t>(count), false);
        for (int vertex = 0; vertex < count; ++vertex) {
            if (!marked[vertex] && undirectedCycleVisit(view, vertex, -1, marked))
                return true;
        }
        return false;
    }

    /// 判断有向图是否存在环。
    static bool hasDirectedCycle(const View& view) {
        const int count = checkedCount(view);
        dsa::container::Vector<unsigned char> color(static_cast<std::size_t>(count), 0);
        for (int vertex = 0; vertex < count; ++vertex) {
            if (color[vertex] == 0 && directedCycleVisit(view, vertex, color))
                return true;
        }
        return false;
    }

    /// 按出边可达关系计算连通分量；无向图需要以双向边表示。
    static ComponentResult connectedComponents(const View& view) {
        const int count = checkedCount(view);
        ComponentResult result(static_cast<std::size_t>(count));
        for (int vertex = 0; vertex < count; ++vertex) {
            if (result.component_of[vertex] != -1)
                continue;
            result.components.push_back(dsa::container::Vector<int>());
            componentVisit(view, vertex, result.count, result);
            ++result.count;
        }
        return result;
    }

    /// 返回从 source 通过出边可达的顶点标记。
    static dsa::container::Vector<bool> reachable(const View& view, int source) {
        const int count = checkedCount(view);
        if (count == 0)
            return dsa::container::Vector<bool>();
        checkVertex(source, count);

        dsa::container::Vector<bool> marked(static_cast<std::size_t>(count), false);
        reachableVisit(view, source, marked);
        return marked;
    }

    /// 使用 Kosaraju 算法计算全部强连通分量；DAG 也会得到每个顶点各自的 SCC。
    static ComponentResult stronglyConnectedComponents(const View& view) {
        const int count = checkedCount(view);
        ComponentResult result(static_cast<std::size_t>(count));
        dsa::container::Vector<dsa::container::Vector<int> > reverse(static_cast<std::size_t>(count));

        for (int from = 0; from < count; ++from) {
            for (int to = view.firstNeighbor(from); to >= 0;
                 to = view.nextNeighbor(from, to)) {
                if (view.containsEdge(from, to))
                    reverse[static_cast<std::size_t>(to)].push_back(from);
            }
        }

        dsa::container::Vector<bool> marked(static_cast<std::size_t>(count), false);
        dsa::container::Vector<int> postorder;
        for (int vertex = 0; vertex < count; ++vertex) {
            if (!marked[vertex])
                reversePostVisit(reverse, vertex, marked, postorder);
        }

        for (dsa::container::Vector<int>::reverse_iterator it = postorder.rbegin();
             it != postorder.rend(); ++it) {
            const int vertex = *it;
            if (result.component_of[vertex] != -1)
                continue;
            result.components.push_back(dsa::container::Vector<int>());
            componentVisit(view, vertex, result.count, result);
            ++result.count;
        }
        return result;
    }

    /// Dijkstra 单源最短路；检测到负权边时抛出 domain_error。
    static PathResult dijkstra(const View& view, int source) {
        const int count = checkedCount(view);
        PathResult result(static_cast<std::size_t>(count));
        if (count == 0)
            return result;
        checkVertex(source, count);

        typedef std::pair<double, int> QueueEntry;
        dsa::container::BinaryHeap<QueueEntry, std::greater<QueueEntry> > queue;
        result.state.priority[source] = 0.0;
        queue.push(QueueEntry(0.0, source));

        while (!queue.empty()) {
            const QueueEntry current = queue.top();
            queue.pop();
            const int vertex = current.second;
            if (current.first != result.state.priority[vertex] ||
                result.state.status[vertex] == VertexStatus::VISITED)
                continue;

            result.state.status[vertex] = VertexStatus::VISITED;
            for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
                 neighbor = view.nextNeighbor(vertex, neighbor)) {
                if (!view.containsEdge(vertex, neighbor))
                    continue;
                const double weight = view.edgeWeight(vertex, neighbor);
                if (weight < 0.0)
                    throw std::domain_error("dijkstra requires non-negative edge weights");
                const double candidate = result.state.priority[vertex] + weight;
                if (candidate < result.state.priority[neighbor]) {
                    result.state.priority[neighbor] = candidate;
                    result.state.parent[neighbor] = vertex;
                    queue.push(QueueEntry(candidate, neighbor));
                }
            }
        }

        markParentEdges(result.state);
        result.state.status[source] = VertexStatus::SOURCE;
        return result;
    }

    /// Prim 最小生成树；只覆盖 source 所在连通分量，非连通顶点保持未发现。
    static PathResult prim(const View& view, int source) {
        const int count = checkedCount(view);
        PathResult result(static_cast<std::size_t>(count));
        if (count == 0)
            return result;
        checkVertex(source, count);

        typedef std::pair<double, int> QueueEntry;
        dsa::container::BinaryHeap<QueueEntry, std::greater<QueueEntry> > queue;
        result.state.priority[source] = 0.0;
        queue.push(QueueEntry(0.0, source));

        while (!queue.empty()) {
            const QueueEntry current = queue.top();
            queue.pop();
            const int vertex = current.second;
            if (result.state.status[vertex] == VertexStatus::VISITED)
                continue;
            result.state.status[vertex] = VertexStatus::VISITED;

            for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
                 neighbor = view.nextNeighbor(vertex, neighbor)) {
                if (!view.containsEdge(vertex, neighbor) ||
                    result.state.status[neighbor] == VertexStatus::VISITED)
                    continue;
                const double weight = view.edgeWeight(vertex, neighbor);
                if (weight < result.state.priority[neighbor]) {
                    result.state.priority[neighbor] = weight;
                    result.state.parent[neighbor] = vertex;
                    queue.push(QueueEntry(weight, neighbor));
                }
            }
        }

        markParentEdges(result.state);
        result.state.status[source] = VertexStatus::SOURCE;
        return result;
    }

    /// Kruskal 最小生成森林；把输入边按无向候选处理，反向重复边由并查集自然跳过。
    static SpanningForestResult kruskal(const View& view) {
        const int count = checkedCount(view);
        SpanningForestResult result(static_cast<std::size_t>(count));
        dsa::container::Vector<SpanningEdge> candidates;
        for (int from = 0; from < count; ++from) {
            for (int to = view.firstNeighbor(from); to >= 0;
                 to = view.nextNeighbor(from, to)) {
                if (view.containsEdge(from, to))
                    candidates.push_back(SpanningEdge(from, to, view.edgeWeight(from, to)));
            }
        }

        dsa::algorithm::sort(
            candidates.begin(), candidates.end(),
            dsa::algorithm::SortStrategy::QuickSort, spanningEdgeLess
        );
        DisjointSet sets(count);
        for (typename dsa::container::Vector<SpanningEdge>::const_iterator it = candidates.begin();
             it != candidates.end(); ++it) {
            if (it->from == it->to || sets.connected(it->from, it->to))
                continue;
            sets.unite(it->from, it->to);
            result.edges.push_back(*it);
            result.total_weight += it->weight;
            result.state.setEdgeType(it->from, it->to, EdgeType::TREE);
            result.state.status[it->from] = VertexStatus::VISITED;
            result.state.status[it->to] = VertexStatus::VISITED;
        }
        return result;
    }

private:
    /// 轻量并查集，只服务 Kruskal 的共享流程。
    class DisjointSet {
    public:
        explicit DisjointSet(int count)
            : parent_(static_cast<std::size_t>(count)), rank_(static_cast<std::size_t>(count), 0) {
            for (int i = 0; i < count; ++i)
                parent_[static_cast<std::size_t>(i)] = i;
        }

        int find(int value) {
            int& parent = parent_[static_cast<std::size_t>(value)];
            if (parent != value)
                parent = find(parent);
            return parent;
        }

        bool connected(int lhs, int rhs) {
            return find(lhs) == find(rhs);
        }

        void unite(int lhs, int rhs) {
            int lhs_root = find(lhs);
            int rhs_root = find(rhs);
            if (lhs_root == rhs_root)
                return;
            int& lhs_rank = rank_[static_cast<std::size_t>(lhs_root)];
            int& rhs_rank = rank_[static_cast<std::size_t>(rhs_root)];
            if (lhs_rank < rhs_rank)
                parent_[static_cast<std::size_t>(lhs_root)] = rhs_root;
            else if (rhs_rank < lhs_rank)
                parent_[static_cast<std::size_t>(rhs_root)] = lhs_root;
            else {
                parent_[static_cast<std::size_t>(rhs_root)] = lhs_root;
                ++lhs_rank;
            }
        }

    private:
        dsa::container::Vector<int> parent_;
        dsa::container::Vector<int> rank_;
    };

    static int checkedCount(const View& view) {
        const std::size_t count = view.vertexCount();
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("graph vertex count exceeds supported index range");
        return static_cast<int>(count);
    }

    static void checkVertex(int vertex, int count) {
        if (vertex < 0 || vertex >= count)
            throw std::out_of_range("graph vertex index out of range");
    }

    static void breadthFirstComponent(
        const View& view,
        int source,
        TraversalResult& result,
        int& clock
    ) {
        dsa::container::Queue<int> queue;
        result.state.status[source] = VertexStatus::DISCOVERED;
        queue.push(source);

        while (!queue.empty()) {
            const int vertex = queue.front();
            queue.pop();
            result.state.discovery_time[vertex] = ++clock;
            result.order.push_back(vertex);

            for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
                 neighbor = view.nextNeighbor(vertex, neighbor)) {
                if (!view.containsEdge(vertex, neighbor))
                    continue;
                if (result.state.status[neighbor] == VertexStatus::UNDISCOVERED) {
                    result.state.status[neighbor] = VertexStatus::DISCOVERED;
                    result.state.parent[neighbor] = vertex;
                    result.state.setEdgeType(vertex, neighbor, EdgeType::TREE);
                    queue.push(neighbor);
                } else {
                    result.state.setEdgeType(vertex, neighbor, EdgeType::CROSS);
                }
            }
            result.state.status[vertex] = VertexStatus::VISITED;
        }
    }

    static void depthFirstVisit(
        const View& view,
        int vertex,
        TraversalResult& result,
        int& clock
    ) {
        result.state.discovery_time[vertex] = ++clock;
        result.state.status[vertex] = VertexStatus::DISCOVERED;
        result.order.push_back(vertex);

        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (!view.containsEdge(vertex, neighbor))
                continue;
            switch (result.state.status[neighbor]) {
                case VertexStatus::UNDISCOVERED:
                    result.state.parent[neighbor] = vertex;
                    result.state.setEdgeType(vertex, neighbor, EdgeType::TREE);
                    depthFirstVisit(view, neighbor, result, clock);
                    break;
                case VertexStatus::DISCOVERED:
                case VertexStatus::SOURCE:
                    result.state.setEdgeType(vertex, neighbor, EdgeType::BACKWARD);
                    break;
                case VertexStatus::VISITED:
                    result.state.setEdgeType(
                        vertex,
                        neighbor,
                        result.state.discovery_time[vertex] < result.state.discovery_time[neighbor]
                            ? EdgeType::FORWARD
                            : EdgeType::CROSS
                    );
                    break;
            }
        }
        result.state.status[vertex] = VertexStatus::VISITED;
        result.state.finish_time[vertex] = ++clock;
    }

    static bool topologicalVisit(
        const View& view,
        int vertex,
        TopologicalResult& result,
        dsa::container::Vector<int>& postorder,
        int& clock
    ) {
        result.state.discovery_time[vertex] = ++clock;
        result.state.status[vertex] = VertexStatus::DISCOVERED;

        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (!view.containsEdge(vertex, neighbor))
                continue;
            if (result.state.status[neighbor] == VertexStatus::UNDISCOVERED) {
                result.state.parent[neighbor] = vertex;
                result.state.setEdgeType(vertex, neighbor, EdgeType::TREE);
                if (!topologicalVisit(view, neighbor, result, postorder, clock))
                    return false;
            } else if (result.state.status[neighbor] == VertexStatus::DISCOVERED) {
                result.state.setEdgeType(vertex, neighbor, EdgeType::BACKWARD);
                return false;
            } else {
                result.state.setEdgeType(
                    vertex,
                    neighbor,
                    result.state.discovery_time[vertex] < result.state.discovery_time[neighbor]
                        ? EdgeType::FORWARD
                        : EdgeType::CROSS
                );
            }
        }

        result.state.status[vertex] = VertexStatus::VISITED;
        result.state.finish_time[vertex] = ++clock;
        postorder.push_back(vertex);
        return true;
    }

    static bool undirectedCycleVisit(
        const View& view,
        int vertex,
        int parent,
        dsa::container::Vector<bool>& marked
    ) {
        marked[vertex] = true;
        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (!view.containsEdge(vertex, neighbor))
                continue;
            if (!marked[neighbor]) {
                if (undirectedCycleVisit(view, neighbor, vertex, marked))
                    return true;
            } else if (neighbor != parent) {
                return true;
            }
        }
        return false;
    }

    static bool directedCycleVisit(
        const View& view,
        int vertex,
        dsa::container::Vector<unsigned char>& color
    ) {
        color[vertex] = 1;
        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (!view.containsEdge(vertex, neighbor))
                continue;
            if (color[neighbor] == 1)
                return true;
            if (color[neighbor] == 0 && directedCycleVisit(view, neighbor, color))
                return true;
        }
        color[vertex] = 2;
        return false;
    }

    static void componentVisit(
        const View& view,
        int vertex,
        int component,
        ComponentResult& result
    ) {
        result.component_of[vertex] = component;
        result.components[static_cast<std::size_t>(component)].push_back(vertex);
        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (view.containsEdge(vertex, neighbor) && result.component_of[neighbor] == -1)
                componentVisit(view, neighbor, component, result);
        }
    }

    static void reachableVisit(const View& view, int vertex, dsa::container::Vector<bool>& marked) {
        marked[vertex] = true;
        for (int neighbor = view.firstNeighbor(vertex); neighbor >= 0;
             neighbor = view.nextNeighbor(vertex, neighbor)) {
            if (view.containsEdge(vertex, neighbor) && !marked[neighbor])
                reachableVisit(view, neighbor, marked);
        }
    }

    static void reversePostVisit(
        const dsa::container::Vector<dsa::container::Vector<int> >& reverse,
        int vertex,
        dsa::container::Vector<bool>& marked,
        dsa::container::Vector<int>& postorder
    ) {
        marked[vertex] = true;
        const dsa::container::Vector<int>& neighbors = reverse[static_cast<std::size_t>(vertex)];
        for (dsa::container::Vector<int>::const_iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
            if (!marked[*it])
                reversePostVisit(reverse, *it, marked, postorder);
        }
        postorder.push_back(vertex);
    }

    static void markParentEdges(TraversalState& state) {
        for (std::size_t vertex = 0; vertex < state.parent.size(); ++vertex) {
            if (state.parent[vertex] >= 0)
                state.setEdgeType(state.parent[vertex], static_cast<int>(vertex), EdgeType::TREE);
        }
    }

    static bool spanningEdgeLess(const SpanningEdge& lhs, const SpanningEdge& rhs) {
        if (lhs.weight != rhs.weight)
            return lhs.weight < rhs.weight;
        if (lhs.from != rhs.from)
            return lhs.from < rhs.from;
        return lhs.to < rhs.to;
    }
};

} // namespace graph
} // namespace core
} // namespace dsa

#endif
