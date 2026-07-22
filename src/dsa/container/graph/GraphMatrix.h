#ifndef DSA_CONTAINER_GRAPH_GRAPH_MATRIX_H
#define DSA_CONTAINER_GRAPH_GRAPH_MATRIX_H

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include <dsa/container/vector/Vector.h>

#include "Graph.h"

namespace dsa {
namespace container {

/// 由仓库 Vector 管理连续表格、由 std::unique_ptr 管理边对象的邻接矩阵图。
///
/// 顶点 ID 等于当前连续下标；删除顶点会使其后的 ID 减一。该行为被明确暴露，
/// 避免把“稳定 ID”伪装成未实现的保证。边对象独占所有权，复制时深拷贝，移动为 O(1)。
template<typename Tv, typename Te>
class GraphMatrix : public Graph<Tv, Te, GraphMatrix<Tv, Te> > {
public:
    typedef Tv vertex_type;
    typedef Te edge_type;
    typedef std::size_t size_type;
    typedef int vertex_id;

private:
    struct VertexRecord {
        vertex_type data;
        size_type in_degree;
        size_type out_degree;

        explicit VertexRecord(const vertex_type& value)
            : data(value), in_degree(0), out_degree(0) {
        }

        explicit VertexRecord(vertex_type&& value)
            : data(std::move(value)), in_degree(0), out_degree(0) {
        }

        VertexRecord(const VertexRecord&) = default;
        VertexRecord(VertexRecord&&) = default;
        VertexRecord& operator=(const VertexRecord&) = default;
        VertexRecord& operator=(VertexRecord&&) = default;
    };

    struct EdgeRecord {
        edge_type data;
        double weight;

        EdgeRecord(const edge_type& value, double edge_weight)
            : data(value), weight(edge_weight) {
        }

        EdgeRecord(edge_type&& value, double edge_weight)
            : data(std::move(value)), weight(edge_weight) {
        }
    };

    typedef std::unique_ptr<EdgeRecord> edge_pointer;
    typedef dsa::container::Vector<edge_pointer> edge_row;

public:
    GraphMatrix()
        : edge_count_(0) {
    }

    /// 深拷贝全部顶点和边，复制结果与源图拥有独立边对象。
    GraphMatrix(const GraphMatrix& other)
        : vertices_(other.vertices_), edges_(other.vertices_.size()), edge_count_(other.edge_count_) {
        for (size_type row = 0; row < other.edges_.size(); ++row) {
            edges_[row].resize(other.edges_[row].size());
            for (size_type column = 0; column < other.edges_[row].size(); ++column) {
                if (other.edges_[row][column]) {
                    edges_[row][column].reset(new EdgeRecord(
                        other.edges_[row][column]->data,
                        other.edges_[row][column]->weight
                    ));
                }
            }
        }
    }

    /// 通过 copy-and-swap 提供强异常保证。
    GraphMatrix& operator=(const GraphMatrix& other) {
        if (this != &other) {
            GraphMatrix copy(other);
            swap(copy);
        }
        return *this;
    }

    GraphMatrix(GraphMatrix&&) noexcept = default;
    GraphMatrix& operator=(GraphMatrix&&) noexcept = default;
    ~GraphMatrix() = default;

    /// 交换两张图的全部所有权；所有顶点/边引用和指针均失效。
    void swap(GraphMatrix& other) noexcept {
        vertices_.swap(other.vertices_);
        edges_.swap(other.edges_);
        std::swap(edge_count_, other.edge_count_);
    }

    size_type vertexCount() const noexcept {
        return vertices_.size();
    }

    size_type edgeCount() const noexcept {
        return edge_count_;
    }

    bool empty() const noexcept {
        return vertices_.empty();
    }

    /// 直接在图中构造顶点。插入可能使全部顶点引用失效，但既有顶点 ID 不变。
    template<typename... Args>
    vertex_id emplaceVertex(Args&&... args) {
        const size_type old_count = vertices_.size();
        edge_row new_row(old_count + 1);
        size_type expanded_rows = 0;
        bool vertex_added = false;

        try {
            for (; expanded_rows < edges_.size(); ++expanded_rows)
                edges_[expanded_rows].push_back(edge_pointer());
            vertex_type value(std::forward<Args>(args)...);
            vertices_.push_back(VertexRecord(std::move(value)));
            vertex_added = true;
            edges_.push_back(std::move(new_row));
        } catch (...) {
            if (vertex_added)
                vertices_.pop_back();
            while (expanded_rows > 0) {
                --expanded_rows;
                edges_[expanded_rows].pop_back();
            }
            throw;
        }
        return static_cast<vertex_id>(old_count);
    }

    vertex_id addVertex(const vertex_type& value) {
        return emplaceVertex(value);
    }

    vertex_id addVertex(vertex_type&& value) {
        return emplaceVertex(std::move(value));
    }

    /// 删除顶点及其所有关联边；其后顶点 ID 减一，所有顶点/边引用和指针失效。
    vertex_type removeVertex(vertex_id vertex) {
        checkVertex(vertex);
        const size_type index = static_cast<size_type>(vertex);
        vertex_type removed(std::move(vertices_[index].data));

        edges_.erase(edges_.begin() + vertex);
        for (size_type row = 0; row < edges_.size(); ++row)
            edges_[row].erase(edges_[row].begin() + vertex);
        vertices_.erase(vertices_.begin() + vertex);
        recomputeMetadata();
        return removed;
    }

    vertex_type& vertex(vertex_id id) {
        checkVertex(id);
        return vertices_[static_cast<size_type>(id)].data;
    }

    const vertex_type& vertex(vertex_id id) const {
        checkVertex(id);
        return vertices_[static_cast<size_type>(id)].data;
    }

    size_type inDegree(vertex_id id) const {
        checkVertex(id);
        return vertices_[static_cast<size_type>(id)].in_degree;
    }

    size_type outDegree(vertex_id id) const {
        checkVertex(id);
        return vertices_[static_cast<size_type>(id)].out_degree;
    }

    bool containsEdge(vertex_id from, vertex_id to) const noexcept {
        if (!validVertex(from) || !validVertex(to))
            return false;
        return static_cast<bool>(edges_[static_cast<size_type>(from)][static_cast<size_type>(to)]);
    }

    /// 直接构造一条边；重复边不覆盖原值并返回 false。
    template<typename... Args>
    bool emplaceEdge(vertex_id from, vertex_id to, double weight, Args&&... args) {
        checkVertex(from);
        checkVertex(to);
        edge_pointer& slot = edges_[static_cast<size_type>(from)][static_cast<size_type>(to)];
        if (slot)
            return false;

        edge_type value(std::forward<Args>(args)...);
        edge_pointer pending(new EdgeRecord(std::move(value), weight));
        slot = std::move(pending);
        ++edge_count_;
        ++vertices_[static_cast<size_type>(from)].out_degree;
        ++vertices_[static_cast<size_type>(to)].in_degree;
        return true;
    }

    bool addEdge(vertex_id from, vertex_id to, const edge_type& value, double weight = 0.0) {
        return emplaceEdge(from, to, weight, value);
    }

    bool addEdge(vertex_id from, vertex_id to, edge_type&& value, double weight = 0.0) {
        return emplaceEdge(from, to, weight, std::move(value));
    }

    /// 删除边并立即析构其值；不存在时返回 false。
    bool removeEdge(vertex_id from, vertex_id to) {
        checkVertex(from);
        checkVertex(to);
        edge_pointer& slot = edges_[static_cast<size_type>(from)][static_cast<size_type>(to)];
        if (!slot)
            return false;
        slot.reset();
        --edge_count_;
        --vertices_[static_cast<size_type>(from)].out_degree;
        --vertices_[static_cast<size_type>(to)].in_degree;
        return true;
    }

    edge_type& edge(vertex_id from, vertex_id to) {
        return edgeRecord(from, to).data;
    }

    const edge_type& edge(vertex_id from, vertex_id to) const {
        return edgeRecord(from, to).data;
    }

    double edgeWeight(vertex_id from, vertex_id to) const {
        return edgeRecord(from, to).weight;
    }

    void setEdgeWeight(vertex_id from, vertex_id to, double weight) {
        edgeRecord(from, to).weight = weight;
    }

    /// 按降序返回首个邻接顶点，保持与教学 GraphMatrix 一致的遍历顺序。
    int firstNeighbor(int vertex) const {
        checkVertex(vertex);
        return nextNeighbor(vertex, static_cast<int>(vertices_.size()));
    }

    /// 返回严格小于 current 的最大邻接顶点；不存在时返回 -1。
    int nextNeighbor(int vertex, int current) const {
        checkVertex(vertex);
        int candidate = current;
        const int count = static_cast<int>(vertices_.size());
        if (candidate > count)
            candidate = count;
        while (--candidate >= 0) {
            if (containsEdge(vertex, candidate))
                return candidate;
        }
        return -1;
    }

    /// 原地反转全部有向边，保留边值和权重；完成后所有边引用和指针失效。
    void reverseEdges() {
        GraphMatrix reversed;
        reversed.vertices_ = vertices_;
        reversed.edges_.resize(vertices_.size());
        for (size_type row = 0; row < vertices_.size(); ++row)
            reversed.edges_[row].resize(vertices_.size());

        for (size_type from = 0; from < vertices_.size(); ++from) {
            for (size_type to = 0; to < vertices_.size(); ++to) {
                if (!edges_[from][to])
                    continue;
                reversed.edges_[to][from].reset(new EdgeRecord(
                    edges_[from][to]->data,
                    edges_[from][to]->weight
                ));
            }
        }
        reversed.edge_count_ = edge_count_;
        reversed.recomputeMetadata();
        swap(reversed);
    }

    /// 删除全部顶点和边，立即释放资源。
    void clear() noexcept {
        edges_.clear();
        vertices_.clear();
        edge_count_ = 0;
    }

private:
    dsa::container::Vector<VertexRecord> vertices_;
    dsa::container::Vector<edge_row> edges_;
    size_type edge_count_;

    bool validVertex(vertex_id vertex) const noexcept {
        return vertex >= 0 && static_cast<size_type>(vertex) < vertices_.size();
    }

    void checkVertex(vertex_id vertex) const {
        if (!validVertex(vertex))
            throw std::out_of_range("graph vertex index out of range");
    }

    EdgeRecord& edgeRecord(vertex_id from, vertex_id to) {
        checkVertex(from);
        checkVertex(to);
        edge_pointer& pointer = edges_[static_cast<size_type>(from)][static_cast<size_type>(to)];
        if (!pointer)
            throw std::out_of_range("graph edge does not exist");
        return *pointer;
    }

    const EdgeRecord& edgeRecord(vertex_id from, vertex_id to) const {
        checkVertex(from);
        checkVertex(to);
        const edge_pointer& pointer = edges_[static_cast<size_type>(from)][static_cast<size_type>(to)];
        if (!pointer)
            throw std::out_of_range("graph edge does not exist");
        return *pointer;
    }

    /// 在批量结构变化后一次性重建边数和度数，避免多处分支各自维护计数。
    void recomputeMetadata() noexcept {
        edge_count_ = 0;
        for (size_type vertex = 0; vertex < vertices_.size(); ++vertex) {
            vertices_[vertex].in_degree = 0;
            vertices_[vertex].out_degree = 0;
        }
        for (size_type from = 0; from < edges_.size(); ++from) {
            for (size_type to = 0; to < edges_[from].size(); ++to) {
                if (!edges_[from][to])
                    continue;
                ++edge_count_;
                ++vertices_[from].out_degree;
                ++vertices_[to].in_degree;
            }
        }
    }
};

/// ADL swap，复杂度 O(1)。
template<typename Tv, typename Te>
void swap(GraphMatrix<Tv, Te>& lhs, GraphMatrix<Tv, Te>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace container
} // namespace dsa

#endif
