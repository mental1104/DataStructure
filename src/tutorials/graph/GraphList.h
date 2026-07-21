#ifndef __DSA_GRAPH_LIST
#define __DSA_GRAPH_LIST

#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Graph.h"
#include "List.h"

/// 教学邻接表图。
///
/// 继续保留 Vector<List<Edge*>> 的节点可视化表示；边对象由 GraphList 独占，
/// 复制时逐边深拷贝，删除顶点后统一重建端点编号和度数。工业实现位于
/// dsa/container/graph/GraphList.h，不继承教学存储。
template<typename Tv, typename Te>
class GraphList : public Graph<Tv, Te> {
private:
    Vector<Vertex<Tv> > _V;
    Vector<List<Edge<Te>*> > _adj;
    EType missing_edge_type_{EType::UNDETERMINED};

    /// 释放当前邻接表拥有的全部边对象，List 节点由 List 自身析构。
    void clearEdges() noexcept;

    /// 从 other 深拷贝顶点和边；仅在当前对象为空时调用。
    void copyFrom(const GraphList& other);

    void checkVertex(int vertex_index) const;
    Edge<Te>* findEdge(int from, int to, ListNode<Edge<Te>*>*& position) const;
    Edge<Te>* checkedEdge(int from, int to) const;

    /// 删除顶点或反转边后统一重建边数、度数和端点编号。
    void recomputeMetadata() noexcept;

public:
    GraphList() = default;
    GraphList(std::ifstream& alg4, GType type);
    GraphList(const GraphList& other);
    GraphList& operator=(const GraphList& other);
    ~GraphList() override;

    // 顶点查询
    Tv& vertex(int i) override {
        checkVertex(i);
        return _V[i].data;
    }

    int inDegree(int i) override {
        checkVertex(i);
        return _V[i].inDegree;
    }

    int outDegree(int i) override {
        checkVertex(i);
        return _V[i].outDegree;
    }

    int firstNbr(int i) override {
        checkVertex(i);
        return nextNbr(i, this->n);
    }

    int nextNbr(int i, int j) override {
        checkVertex(i);
        int candidate = -1;
        for (ListNode<Edge<Te>*>* node = _adj[i].first(); _adj[i].valid(node);
             node = node->succ) {
            const int neighbor = node->data->y;
            if (neighbor < j && neighbor > candidate)
                candidate = neighbor;
        }
        return candidate;
    }

    VStatus& status(int i) override {
        checkVertex(i);
        return _V[i].status;
    }

    int& dTime(int i) override {
        checkVertex(i);
        return _V[i].dTime;
    }

    int& fTime(int i) override {
        checkVertex(i);
        return _V[i].fTime;
    }

    int& hca(int i) override {
        return fTime(i);
    }

    int& parent(int i) override {
        checkVertex(i);
        return _V[i].parent;
    }

    double& priority(int i) override {
        checkVertex(i);
        return _V[i].priority;
    }

    // 顶点动态操作
    int insert(Tv const& value) override;
    Tv remove(int i) override;

    // 边查询
    bool exists(int i, int j) override {
        ListNode<Edge<Te>*>* position = nullptr;
        return findEdge(i, j, position) != nullptr;
    }

    EType& type(int i, int j) override {
        ListNode<Edge<Te>*>* position = nullptr;
        Edge<Te>* const found = findEdge(i, j, position);
        if (!found) {
            missing_edge_type_ = EType::UNDETERMINED;
            return missing_edge_type_;
        }
        return found->type;
    }

    Te& edge(int i, int j) override {
        return checkedEdge(i, j)->data;
    }

    double& weight(int i, int j) override {
        return checkedEdge(i, j)->weight;
    }

    // 边动态操作
    void insert(Te const& value, int i, int j, double w = 0.0) override;
    Te remove(int i, int j) override;

    /// 反转全部有向边并保留边值、权重和数量。
    void reverse() override;
};

template<typename Tv, typename Te>
void GraphList<Tv, Te>::clearEdges() noexcept {
    for (int source = 0; source < _adj.size(); ++source) {
        for (ListNode<Edge<Te>*>* node = _adj[source].first(); _adj[source].valid(node);
             node = node->succ) {
            delete node->data;
            node->data = nullptr;
        }
    }
    this->e = 0;
}

template<typename Tv, typename Te>
void GraphList<Tv, Te>::copyFrom(const GraphList& other) {
    this->observer = other.observer;
    this->n = 0;
    this->e = 0;
    try {
        for (int vertex_index = 0; vertex_index < other.n; ++vertex_index)
            insert(other._V[vertex_index].data);

        // 保留完整教学顶点元数据，而边对象必须重新分配。
        _V = other._V;
        for (int source = 0; source < other.n; ++source) {
            for (ListNode<Edge<Te>*>* node = other._adj[source].first();
                 other._adj[source].valid(node); node = node->succ) {
                Edge<Te>* const cloned = new Edge<Te>(*node->data);
                try {
                    _adj[source].insertAsLast(cloned);
                } catch (...) {
                    delete cloned;
                    throw;
                }
                ++this->e;
            }
        }
    } catch (...) {
        clearEdges();
        _adj = Vector<List<Edge<Te>*> >();
        _V = Vector<Vertex<Tv> >();
        this->n = 0;
        throw;
    }
}

template<typename Tv, typename Te>
GraphList<Tv, Te>::GraphList(const GraphList& other) {
    copyFrom(other);
}

template<typename Tv, typename Te>
GraphList<Tv, Te>& GraphList<Tv, Te>::operator=(const GraphList& other) {
    if (this == &other)
        return *this;

    clearEdges();
    _adj = Vector<List<Edge<Te>*> >();
    _V = Vector<Vertex<Tv> >();
    this->n = 0;
    copyFrom(other);
    return *this;
}

template<typename Tv, typename Te>
GraphList<Tv, Te>::~GraphList() {
    clearEdges();
}

template<typename Tv, typename Te>
void GraphList<Tv, Te>::checkVertex(int vertex_index) const {
    if (vertex_index < 0 || vertex_index >= this->n)
        throw std::out_of_range("graph vertex index out of range");
}

template<typename Tv, typename Te>
Edge<Te>* GraphList<Tv, Te>::findEdge(
    int from,
    int to,
    ListNode<Edge<Te>*>*& position
) const {
    position = nullptr;
    if (from < 0 || from >= this->n || to < 0 || to >= this->n)
        return nullptr;

    for (ListNode<Edge<Te>*>* node = _adj[from].first(); _adj[from].valid(node);
         node = node->succ) {
        if (node->data->y == to) {
            position = node;
            return node->data;
        }
    }
    return nullptr;
}

template<typename Tv, typename Te>
Edge<Te>* GraphList<Tv, Te>::checkedEdge(int from, int to) const {
    ListNode<Edge<Te>*>* position = nullptr;
    Edge<Te>* const found = findEdge(from, to, position);
    if (!found)
        throw std::out_of_range("graph edge does not exist");
    return found;
}

template<typename Tv, typename Te>
void GraphList<Tv, Te>::recomputeMetadata() noexcept {
    this->e = 0;
    for (int vertex_index = 0; vertex_index < this->n; ++vertex_index) {
        _V[vertex_index].rank = vertex_index;
        _V[vertex_index].inDegree = 0;
        _V[vertex_index].outDegree = 0;
    }

    for (int source = 0; source < this->n; ++source) {
        for (ListNode<Edge<Te>*>* node = _adj[source].first(); _adj[source].valid(node);
             node = node->succ) {
            node->data->x = source;
            ++this->e;
            ++_V[source].outDegree;
            ++_V[node->data->y].inDegree;
        }
    }
}

template<typename Tv, typename Te>
GraphList<Tv, Te>::GraphList(std::ifstream& alg4, GType graph_type) {
    int vertex_count = 0;
    int edge_count = 0;
    alg4 >> vertex_count >> edge_count;
    for (int vertex_index = 0; vertex_index < vertex_count; ++vertex_index)
        insert(Tv(vertex_index));

    int source = 0;
    int target = 0;
    double edge_weight = 0.0;
    for (int edge_index = 0; edge_index < edge_count; ++edge_index) {
        switch (graph_type) {
            case GType::DIGRAPH:
                alg4 >> source >> target;
                insert(Te(edge_index), source, target);
                break;
            case GType::UNDIGRAPH:
                alg4 >> source >> target;
                insert(Te(edge_index), source, target);
                insert(Te(edge_index), target, source);
                break;
            case GType::WEIGHTEDDIGRAPH:
                alg4 >> source >> target >> edge_weight;
                insert(Te(edge_weight), source, target, edge_weight);
                break;
            case GType::WEIGHTEDUNDIGRAPH:
                alg4 >> source >> target >> edge_weight;
                insert(Te(edge_weight), source, target, edge_weight);
                insert(Te(edge_weight), target, source, edge_weight);
                break;
        }
    }
}

template<typename Tv, typename Te>
int GraphList<Tv, Te>::insert(Tv const& value) {
    _adj.insert(List<Edge<Te>*>());
    try {
        const int index = _V.insert(Vertex<Tv>(this->n, value));
        ++this->n;
        return index;
    } catch (...) {
        _adj.remove(_adj.size() - 1);
        throw;
    }
}

template<typename Tv, typename Te>
Tv GraphList<Tv, Te>::remove(int vertex_index) {
    checkVertex(vertex_index);
    Tv removed = _V[vertex_index].data;

    // 先释放出边对象，再由 Vector 删除其 List 容器。
    for (ListNode<Edge<Te>*>* node = _adj[vertex_index].first();
         _adj[vertex_index].valid(node); node = node->succ) {
        delete node->data;
        node->data = nullptr;
    }
    _adj.remove(vertex_index);
    _V.remove(vertex_index);
    --this->n;

    // 删除所有入边，并把受删除下标影响的端点整体左移。
    for (int source = 0; source < this->n; ++source) {
        for (ListNode<Edge<Te>*>* node = _adj[source].first(); _adj[source].valid(node);) {
            ListNode<Edge<Te>*>* const current = node;
            node = node->succ;
            Edge<Te>* const current_edge = current->data;
            if (current_edge->y == vertex_index) {
                _adj[source].remove(current);
                delete current_edge;
            } else if (current_edge->y > vertex_index) {
                --current_edge->y;
            }
        }
    }

    recomputeMetadata();
    return removed;
}

template<typename Tv, typename Te>
void GraphList<Tv, Te>::insert(Te const& value, int from, int to, double edge_weight) {
    checkVertex(from);
    checkVertex(to);
    if (exists(from, to))
        return;

    Edge<Te>* const pending = new Edge<Te>(value, edge_weight, from, to);
    try {
        _adj[from].insertAsFirst(pending);
    } catch (...) {
        delete pending;
        throw;
    }
    ++this->e;
    ++_V[from].outDegree;
    ++_V[to].inDegree;
}

template<typename Tv, typename Te>
Te GraphList<Tv, Te>::remove(int from, int to) {
    ListNode<Edge<Te>*>* position = nullptr;
    Edge<Te>* const found = findEdge(from, to, position);
    if (!found)
        throw std::out_of_range("graph edge does not exist");

    Te removed = found->data;
    _adj[from].remove(position);
    delete found;
    --this->e;
    --_V[from].outDegree;
    --_V[to].inDegree;
    return removed;
}

template<typename Tv, typename Te>
void GraphList<Tv, Te>::reverse() {
    struct Snapshot {
        Te data;
        double weight;
        int from;
        int to;

        Snapshot(Te const& value, double edge_weight, int source, int target)
            : data(value), weight(edge_weight), from(source), to(target) {
        }
    };

    std::vector<Snapshot> snapshots;
    snapshots.reserve(static_cast<std::size_t>(this->e));
    for (int source = 0; source < this->n; ++source) {
        for (ListNode<Edge<Te>*>* node = _adj[source].first(); _adj[source].valid(node);
             node = node->succ) {
            snapshots.push_back(Snapshot(node->data->data, node->data->weight, source, node->data->y));
        }
    }

    clearEdges();
    _adj = Vector<List<Edge<Te>*> >();
    for (int vertex_index = 0; vertex_index < this->n; ++vertex_index)
        _adj.insert(List<Edge<Te>*>());
    for (int vertex_index = 0; vertex_index < this->n; ++vertex_index) {
        _V[vertex_index].inDegree = 0;
        _V[vertex_index].outDegree = 0;
    }

    for (typename std::vector<Snapshot>::const_iterator it = snapshots.begin();
         it != snapshots.end(); ++it) {
        insert(it->data, it->to, it->from, it->weight);
    }
}

#endif
