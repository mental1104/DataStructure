#ifndef __DSA_GRAPH_MATRIX
#define __DSA_GRAPH_MATRIX

#include <fstream>
#include <stdexcept>
#include <vector>

#include "utils.h"
#include "Vector.h"
#include "Graph.h"
#include "PU.h"

using std::ifstream;

/// 教学邻接矩阵图。
///
/// 继续保留 Vector<Vector<Edge*>> 的可视化表示，但显式实现深拷贝和资源清理，
/// 避免编译器生成的浅拷贝导致双重释放。工业实现位于 dsa/container/graph。
template<typename Tv, typename Te>
class GraphMatrix : public Graph<Tv, Te> {
private:
    Vector<Vertex<Tv> > _V;
    Vector<Vector<Edge<Te>*> > _E;
    EType missing_edge_type_{EType::UNDETERMINED};

    /// 释放当前矩阵拥有的全部边对象，但不删除顶点。
    void clearEdges() noexcept;

    /// 从 other 深拷贝顶点与边；失败时清理已创建边并保持合法空边状态。
    void copyFrom(const GraphMatrix& other);

    void checkVertex(int vertex_index) const;
    void checkEdge(int from, int to);

    /// 批量结构变化后统一重建边数、度数和边端点编号。
    void recomputeMetadata() noexcept;

public:
    GraphMatrix() = default;
    GraphMatrix(ifstream& alg4, GType type);
    GraphMatrix(const GraphMatrix& other);
    GraphMatrix& operator=(const GraphMatrix& other);
    ~GraphMatrix() override;

    const Vector<Vertex<Tv> >& V() const {
        return _V;
    }

    const Vector<Vector<Edge<Te>*> >& E() const {
        return _E;
    }

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
        int candidate = j > this->n ? this->n : j;
        while (--candidate >= 0) {
            if (exists(i, candidate))
                return candidate;
        }
        return -1;
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
        return i >= 0 && i < this->n && j >= 0 && j < this->n && _E[i][j] != nullptr;
    }

    EType& type(int i, int j) override {
        if (!exists(i, j)) {
            missing_edge_type_ = EType::UNDETERMINED;
            return missing_edge_type_;
        }
        return _E[i][j]->type;
    }

    Te& edge(int i, int j) override {
        checkEdge(i, j);
        return _E[i][j]->data;
    }

    double& weight(int i, int j) override {
        checkEdge(i, j);
        return _E[i][j]->weight;
    }

    // 边动态操作
    void insert(Te const& value, int i, int j, double w = 0.0) override;
    Te remove(int i, int j) override;

    /// 反转全部有向边，保留边值、权重和数量。
    void reverse() override;
};

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::clearEdges() noexcept {
    for (int row = 0; row < _E.size(); ++row) {
        for (int column = 0; column < _E[row].size(); ++column) {
            delete _E[row][column];
            _E[row][column] = nullptr;
        }
    }
    this->e = 0;
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::copyFrom(const GraphMatrix& other) {
    _V = other._V;
    _E = Vector<Vector<Edge<Te>*> >();
    this->observer = other.observer;
    this->n = other.n;
    this->e = 0;

    try {
        for (int row = 0; row < this->n; ++row)
            _E.insert(Vector<Edge<Te>*>(this->n, this->n, nullptr));

        for (int row = 0; row < this->n; ++row) {
            for (int column = 0; column < this->n; ++column) {
                if (!other._E[row][column])
                    continue;
                _E[row][column] = new Edge<Te>(*other._E[row][column]);
                ++this->e;
            }
        }
    } catch (...) {
        clearEdges();
        _E = Vector<Vector<Edge<Te>*> >();
        _V = Vector<Vertex<Tv> >();
        this->n = 0;
        throw;
    }
}

template<typename Tv, typename Te>
GraphMatrix<Tv, Te>::GraphMatrix(const GraphMatrix& other) {
    copyFrom(other);
}

template<typename Tv, typename Te>
GraphMatrix<Tv, Te>& GraphMatrix<Tv, Te>::operator=(const GraphMatrix& other) {
    if (this == &other)
        return *this;

    clearEdges();
    _E = Vector<Vector<Edge<Te>*> >();
    _V = Vector<Vertex<Tv> >();
    this->n = 0;
    copyFrom(other);
    return *this;
}

template<typename Tv, typename Te>
GraphMatrix<Tv, Te>::~GraphMatrix() {
    clearEdges();
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::checkVertex(int vertex_index) const {
    if (vertex_index < 0 || vertex_index >= this->n)
        throw std::out_of_range("graph vertex index out of range");
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::checkEdge(int from, int to) {
    if (!exists(from, to))
        throw std::out_of_range("graph edge does not exist");
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::recomputeMetadata() noexcept {
    this->e = 0;
    for (int vertex_index = 0; vertex_index < this->n; ++vertex_index) {
        _V[vertex_index].rank = vertex_index;
        _V[vertex_index].inDegree = 0;
        _V[vertex_index].outDegree = 0;
    }

    for (int from = 0; from < this->n; ++from) {
        for (int to = 0; to < this->n; ++to) {
            Edge<Te>* const current = _E[from][to];
            if (!current)
                continue;
            current->x = from;
            current->y = to;
            ++this->e;
            ++_V[from].outDegree;
            ++_V[to].inDegree;
        }
    }
}

template<typename Tv, typename Te>
GraphMatrix<Tv, Te>::GraphMatrix(ifstream& alg4, GType graph_type) {
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
int GraphMatrix<Tv, Te>::insert(Tv const& value) {
    for (int row = 0; row < this->n; ++row)
        _E[row].insert(nullptr);

    const int new_count = this->n + 1;
    _E.insert(Vector<Edge<Te>*>(new_count, new_count, nullptr));
    const int index = _V.insert(Vertex<Tv>(this->n, value));
    this->n = new_count;
    return index;
}

template<typename Tv, typename Te>
Tv GraphMatrix<Tv, Te>::remove(int vertex_index) {
    checkVertex(vertex_index);
    Tv removed = _V[vertex_index].data;

    for (int column = 0; column < this->n; ++column) {
        delete _E[vertex_index][column];
        _E[vertex_index][column] = nullptr;
    }
    _E.remove(vertex_index);

    for (int row = 0; row < this->n - 1; ++row) {
        Edge<Te>* incoming = _E[row].remove(vertex_index);
        delete incoming;
    }

    _V.remove(vertex_index);
    --this->n;
    recomputeMetadata();
    return removed;
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::insert(Te const& value, int from, int to, double edge_weight) {
    checkVertex(from);
    checkVertex(to);
    if (exists(from, to))
        return;

    Edge<Te>* const pending = new Edge<Te>(value, edge_weight, from, to);
    _E[from][to] = pending;
    ++this->e;
    ++_V[from].outDegree;
    ++_V[to].inDegree;
}

template<typename Tv, typename Te>
Te GraphMatrix<Tv, Te>::remove(int from, int to) {
    checkEdge(from, to);
    Te removed = _E[from][to]->data;
    delete _E[from][to];
    _E[from][to] = nullptr;
    --this->e;
    --_V[from].outDegree;
    --_V[to].inDegree;
    return removed;
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::reverse() {
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
    for (int from = 0; from < this->n; ++from) {
        for (int to = 0; to < this->n; ++to) {
            if (_E[from][to])
                snapshots.push_back(Snapshot(_E[from][to]->data, _E[from][to]->weight, from, to));
        }
    }

    clearEdges();
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
