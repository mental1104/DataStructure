#ifndef __DSA_GRAPH
#define __DSA_GRAPH

#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "utils.h"
#include "Vector.h"
#include "Stack.h"
#include "GraphObserver.h"
#include "dsa/core/graph/GraphAlgorithm.h"

using VStatus = dsa::core::graph::VertexStatus;
using EType = dsa::core::graph::EdgeType;

enum class GType {
    UNDIGRAPH,
    DIGRAPH,
    WEIGHTEDUNDIGRAPH,
    WEIGHTEDDIGRAPH
};

/// 教学图顶点：结构数据与历史遍历元数据保持兼容。
template<typename Tv>
struct Vertex {
    Tv data;
    int inDegree{0};
    int outDegree{0};
    VStatus status{VStatus::UNDISCOVERED};
    int dTime{-1};
    int fTime{-1};
    int parent{-1};
    double priority{std::numeric_limits<double>::infinity()};
    int rank{-1};

    Vertex() = default;

    /// 保留仅传 rank 的历史构造方式；该重载要求 Tv 可默认构造。
    explicit Vertex(int vertex_rank)
        : data(), rank(vertex_rank) {
    }

    /// 使用显式数据构造顶点，不再要求 Tv 能由整数 0 隐式构造。
    Vertex(int vertex_rank, Tv const& value)
        : data(value), rank(vertex_rank) {
    }

    /// 教学 Heap 约定“小权重具有更高优先级”。
    bool operator<(const Vertex<Tv>& rhs) const {
        return priority > rhs.priority;
    }
};

/// 教学图边；具体所有权仍由 GraphMatrix/GraphList 负责。
template<typename Te>
struct Edge {
    Te data;
    double weight{0.0};
    EType type{EType::UNDETERMINED};
    int x{-1};
    int y{-1};

    Edge() = default;

    Edge(Te const& value, double edge_weight, int from, int to)
        : data(value), weight(edge_weight), x(from), y(to) {
    }

    /// 教学 Heap 约定“小权重具有更高优先级”。
    bool operator<(const Edge<Te>& rhs) const {
        return weight > rhs.weight;
    }
};

/// 教学图抽象门面。
///
/// 公开接口和顶点内遍历字段保持原样；实际 BFS/DFS/拓扑排序/SCC/最短路/MST
/// 已统一委托给 dsa::core::graph::GraphAlgorithm。共享算法只看到邻接语义，
/// GraphMatrix 与 GraphList 不再各自复制算法。
template<typename Tv, typename Te>
class Graph {
private:
    /// 把历史虚接口适配为共享 GraphAlgorithm 的最小只读 View。
    class AlgorithmView {
    public:
        explicit AlgorithmView(Graph& graph)
            : graph_(graph) {
        }

        std::size_t vertexCount() const {
            return graph_.n < 0 ? 0U : static_cast<std::size_t>(graph_.n);
        }

        int firstNeighbor(int vertex) const {
            return graph_.firstNbr(vertex);
        }

        int nextNeighbor(int vertex, int current) const {
            return graph_.nextNbr(vertex, current);
        }

        bool containsEdge(int from, int to) const {
            return graph_.exists(from, to);
        }

        double edgeWeight(int from, int to) const {
            return graph_.weight(from, to);
        }

    private:
        Graph& graph_;
    };

    typedef dsa::core::graph::GraphAlgorithm<AlgorithmView> SharedAlgorithm;

    /// 清空教学对象中的历史遍历状态和边分类。
    void reset();

    /// 将共享算法的外置状态同步回历史教学字段，维持原观察方式。
    void applyState(const dsa::core::graph::TraversalState& state);

    /// 无向图环检测的兼容入口。
    bool cycle();

    /// 有向图环检测的兼容入口；flag 仅保留历史签名。
    bool directedCycle(bool flag = false);

    /// 保留旧测试和教学材料使用的单入口拓扑递归 helper 签名。
    void TSort(int vertex, int& clock, Stack<Tv>* stack);

    /// 教学版 BCC 仍保留栈式演示流程；它依赖历史 hca/fTime 复用约定。
    void BCC(int vertex, int& clock, Stack<int>& stack);

    /// 历史 PFS updater 直接操作 Graph 指针，暂保留在教学门面中。
    template<typename PU>
    void PFS(int vertex, PU priority_updater);

public:
    GraphObserver<Tv, Te>* observer;
    int n;
    int e;

    Graph()
        : observer(nullptr), n(0), e(0) {
    }

    virtual ~Graph() = default;

    void setObserver(GraphObserver<Tv, Te>* graph_observer) {
        observer = graph_observer;
    }

    // 顶点接口
    virtual int insert(Tv const&) = 0;
    virtual Tv remove(int) = 0;
    virtual Tv& vertex(int) = 0;
    virtual int inDegree(int) = 0;
    virtual int outDegree(int) = 0;
    virtual int firstNbr(int) = 0;
    virtual int nextNbr(int, int) = 0;
    virtual VStatus& status(int) = 0;
    virtual int& dTime(int) = 0;
    virtual int& fTime(int) = 0;
    virtual int& hca(int) = 0;
    virtual int& parent(int) = 0;
    virtual double& priority(int) = 0;

    // 边接口
    virtual bool exists(int, int) = 0;
    virtual void insert(Te const& edge, int, int, double) = 0;
    virtual Te remove(int, int) = 0;
    virtual EType& type(int, int) = 0;
    virtual Te& edge(int, int) = 0;
    virtual double& weight(int, int) = 0;
    virtual void reverse() = 0;

    // 教学算法 facade
    void bfs(int start);
    void dfs(int start);
    void bcc(int start);
    Stack<Tv>* tSort(int start);
    void prim(int start);
    void dijkstra(int start);

    template<typename PU>
    void pfs(int start, PU priority_updater);

    void kruskal(bool flag = false);
    int connectedComponents(bool flag = false);
    bool connectedComponents(int lhs, int rhs);
    void reachableComponents(int source);
    int kosarajuSCC(bool flag = false);
};

template<typename Tv, typename Te>
void Graph<Tv, Te>::reset() {
    for (int vertex_index = 0; vertex_index < n; ++vertex_index) {
        status(vertex_index) = VStatus::UNDISCOVERED;
        dTime(vertex_index) = -1;
        fTime(vertex_index) = -1;
        parent(vertex_index) = -1;
        priority(vertex_index) = std::numeric_limits<double>::infinity();
        for (int neighbor = firstNbr(vertex_index); neighbor >= 0;
             neighbor = nextNbr(vertex_index, neighbor)) {
            if (exists(vertex_index, neighbor))
                type(vertex_index, neighbor) = EType::UNDETERMINED;
        }
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::applyState(const dsa::core::graph::TraversalState& state) {
    reset();
    const int count = std::min(n, static_cast<int>(state.status.size()));
    for (int vertex_index = 0; vertex_index < count; ++vertex_index) {
        status(vertex_index) = state.status[static_cast<std::size_t>(vertex_index)];
        dTime(vertex_index) = state.discovery_time[static_cast<std::size_t>(vertex_index)];
        fTime(vertex_index) = state.finish_time[static_cast<std::size_t>(vertex_index)];
        parent(vertex_index) = state.parent[static_cast<std::size_t>(vertex_index)];
        priority(vertex_index) = state.priority[static_cast<std::size_t>(vertex_index)];
    }

    for (typename dsa::container::Vector<
             dsa::core::graph::TraversalState::EdgeClassification
         >::const_iterator it = state.edge_type.begin();
         it != state.edge_type.end(); ++it) {
        if (exists(it->from, it->to))
            type(it->from, it->to) = it->type;
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::bfs(int start) {
    AlgorithmView view(*this);
    const dsa::core::graph::TraversalResult result = SharedAlgorithm::breadthFirst(view, start);
    applyState(result.state);
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::dfs(int start) {
    AlgorithmView view(*this);
    const dsa::core::graph::TraversalResult result = SharedAlgorithm::depthFirst(view, start);
    applyState(result.state);
}

template<typename Tv, typename Te>
Stack<Tv>* Graph<Tv, Te>::tSort(int start) {
    Stack<Tv>* stack = new Stack<Tv>;
    AlgorithmView view(*this);
    const dsa::core::graph::TopologicalResult result = SharedAlgorithm::topologicalSort(view, start);
    applyState(result.state);
    if (!result.acyclic)
        return stack;

    for (typename dsa::container::Vector<int>::const_reverse_iterator it = result.order.rbegin();
         it != result.order.rend(); ++it) {
        stack->push(vertex(*it));
    }
    return stack;
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::cycle() {
    AlgorithmView view(*this);
    return SharedAlgorithm::hasUndirectedCycle(view);
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::directedCycle(bool flag) {
    (void)flag;
    AlgorithmView view(*this);
    return SharedAlgorithm::hasDirectedCycle(view);
}

template<typename Tv, typename Te>
int Graph<Tv, Te>::connectedComponents(bool flag) {
    AlgorithmView view(*this);
    const dsa::core::graph::ComponentResult result = SharedAlgorithm::connectedComponents(view);
    if (flag && observer) {
        for (std::size_t component = 0; component < result.components.size(); ++component) {
            Vector<int> nodes;
            for (std::size_t index = 0; index < result.components[component].size(); ++index)
                nodes.insert(result.components[component][index]);
            observer->onSCCComponent(nodes);
        }
    }
    return result.count;
}

template<typename Tv, typename Te>
bool Graph<Tv, Te>::connectedComponents(int lhs, int rhs) {
    AlgorithmView view(*this);
    const dsa::core::graph::ComponentResult result = SharedAlgorithm::connectedComponents(view);
    if (lhs < 0 || rhs < 0 || lhs >= n || rhs >= n)
        return false;
    return result.component_of[static_cast<std::size_t>(lhs)] ==
           result.component_of[static_cast<std::size_t>(rhs)];
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::reachableComponents(int source) {
    AlgorithmView view(*this);
    const dsa::container::Vector<bool> marked = SharedAlgorithm::reachable(view, source);
    if (!observer)
        return;

    Vector<int> reachable_vertices;
    for (std::size_t vertex_index = 0; vertex_index < marked.size(); ++vertex_index) {
        if (marked[vertex_index])
            reachable_vertices.insert(static_cast<int>(vertex_index));
    }
    observer->onSCCComponent(reachable_vertices);
}

template<typename Tv, typename Te>
int Graph<Tv, Te>::kosarajuSCC(bool flag) {
    AlgorithmView view(*this);

    // 保留历史教学契约：无环图返回 0；工业 Graph 则返回数学意义上的单点 SCC。
    if (!SharedAlgorithm::hasDirectedCycle(view))
        return 0;

    const dsa::core::graph::ComponentResult result =
        SharedAlgorithm::stronglyConnectedComponents(view);
    if (flag && observer) {
        for (std::size_t component = 0; component < result.components.size(); ++component) {
            Vector<int> nodes;
            for (std::size_t index = 0; index < result.components[component].size(); ++index)
                nodes.insert(result.components[component][index]);
            observer->onSCCComponent(nodes);
        }
    }
    return result.count;
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::dijkstra(int start) {
    AlgorithmView view(*this);
    const dsa::core::graph::PathResult result = SharedAlgorithm::dijkstra(view, start);
    applyState(result.state);
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::prim(int start) {
    AlgorithmView view(*this);
    const dsa::core::graph::PathResult result = SharedAlgorithm::prim(view, start);
    applyState(result.state);
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::kruskal(bool flag) {
    AlgorithmView view(*this);
    const dsa::core::graph::SpanningForestResult result = SharedAlgorithm::kruskal(view);
    applyState(result.state);

    if (flag && observer) {
        Vector<KruskalEdgeSummary<Tv, Te> > summaries;
        for (std::size_t index = 0; index < result.edges.size(); ++index) {
            const dsa::core::graph::SpanningEdge& selected = result.edges[index];
            observer->onKruskalEdge(selected.from, selected.to, selected.weight);
            KruskalEdgeSummary<Tv, Te> summary{selected.from, selected.to};
            summaries.insert(summary);
        }
        observer->onKruskalDone(result.total_weight, summaries);
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::TSort(int vertex, int& clock, Stack<Tv>* stack) {
    if (!stack)
        throw std::invalid_argument("topological stack must not be null");
    AlgorithmView view(*this);
    const dsa::core::graph::TopologicalResult result =
        SharedAlgorithm::topologicalSort(view, vertex);
    applyState(result.state);
    clock = 0;
    for (std::size_t index = 0; index < result.state.finish_time.size(); ++index)
        clock = std::max(clock, result.state.finish_time[index]);
    if (!result.acyclic)
        return;
    for (typename dsa::container::Vector<int>::const_reverse_iterator it = result.order.rbegin();
         it != result.order.rend(); ++it) {
        stack->push(this->vertex(*it));
    }
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::bcc(int start) {
    reset();
    if (n <= 0)
        return;
    if (start < 0 || start >= n)
        throw std::out_of_range("graph vertex index out of range");

    int clock = 0;
    int vertex_index = start;
    Stack<int> stack;
    do {
        if (status(vertex_index) == VStatus::UNDISCOVERED) {
            BCC(vertex_index, clock, stack);
            if (!stack.empty())
                stack.pop();
        }
        vertex_index = (vertex_index + 1) % n;
    } while (vertex_index != start);
}

template<typename Tv, typename Te>
void Graph<Tv, Te>::BCC(int vertex_index, int& clock, Stack<int>& stack) {
    hca(vertex_index) = dTime(vertex_index) = ++clock;
    status(vertex_index) = VStatus::DISCOVERED;
    stack.push(vertex_index);

    for (int neighbor = firstNbr(vertex_index); neighbor >= 0;
         neighbor = nextNbr(vertex_index, neighbor)) {
        if (!exists(vertex_index, neighbor))
            continue;
        switch (status(neighbor)) {
            case VStatus::UNDISCOVERED:
                parent(neighbor) = vertex_index;
                type(vertex_index, neighbor) = EType::TREE;
                BCC(neighbor, clock, stack);
                if (hca(neighbor) < dTime(vertex_index)) {
                    hca(vertex_index) = std::min(hca(vertex_index), hca(neighbor));
                } else {
                    while (!stack.empty()) {
                        const int popped = stack.pop();
                        if (popped == neighbor)
                            break;
                    }
                }
                break;
            case VStatus::DISCOVERED:
            case VStatus::SOURCE:
                type(vertex_index, neighbor) = EType::BACKWARD;
                if (neighbor != parent(vertex_index))
                    hca(vertex_index) = std::min(hca(vertex_index), dTime(neighbor));
                break;
            case VStatus::VISITED:
                type(vertex_index, neighbor) =
                    dTime(vertex_index) < dTime(neighbor) ? EType::FORWARD : EType::CROSS;
                break;
        }
    }
    status(vertex_index) = VStatus::VISITED;
}

template<typename Tv, typename Te>
template<typename PU>
void Graph<Tv, Te>::pfs(int start, PU priority_updater) {
    reset();
    if (n <= 0)
        return;
    if (start < 0 || start >= n)
        throw std::out_of_range("graph vertex index out of range");

    for (int offset = 0; offset < n; ++offset) {
        const int vertex_index = (start + offset) % n;
        if (status(vertex_index) == VStatus::UNDISCOVERED)
            PFS(vertex_index, priority_updater);
    }
}

template<typename Tv, typename Te>
template<typename PU>
void Graph<Tv, Te>::PFS(int vertex_index, PU priority_updater) {
    priority(vertex_index) = 0.0;
    status(vertex_index) = VStatus::VISITED;

    for (int visited = 1; visited < n; ++visited) {
        for (int neighbor = firstNbr(vertex_index); neighbor >= 0;
             neighbor = nextNbr(vertex_index, neighbor)) {
            if (exists(vertex_index, neighbor))
                priority_updater(this, vertex_index, neighbor);
        }

        double shortest = std::numeric_limits<double>::infinity();
        int next = -1;
        for (int candidate = 0; candidate < n; ++candidate) {
            if (status(candidate) == VStatus::UNDISCOVERED && priority(candidate) < shortest) {
                shortest = priority(candidate);
                next = candidate;
            }
        }
        if (next < 0)
            break;
        vertex_index = next;
        status(vertex_index) = VStatus::VISITED;
        if (parent(vertex_index) >= 0)
            type(parent(vertex_index), vertex_index) = EType::TREE;
    }
}

#endif
