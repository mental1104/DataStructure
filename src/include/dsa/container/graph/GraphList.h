#ifndef DSA_CONTAINER_GRAPH_GRAPH_LIST_H
#define DSA_CONTAINER_GRAPH_GRAPH_LIST_H

#include <cstddef>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Graph.h"

namespace dsa {
namespace container {

/// 基于有序邻接表的工业图。
///
/// 顶点 ID 是连续下标，删除顶点会重编号后续顶点。邻接表按目标 ID 有序，
/// 因而矩阵版和表版能提供一致、确定的降序邻居遍历结果。
template<typename Tv, typename Te>
class GraphList : public Graph<Tv, Te, GraphList<Tv, Te> > {
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

        EdgeRecord(const EdgeRecord&) = default;
        EdgeRecord(EdgeRecord&&) = default;
        EdgeRecord& operator=(const EdgeRecord&) = default;
        EdgeRecord& operator=(EdgeRecord&&) = default;
    };

    typedef std::map<vertex_id, EdgeRecord> adjacency_map;

public:
    GraphList()
        : edge_count_(0) {
    }

    GraphList(const GraphList&) = default;
    GraphList& operator=(const GraphList&) = default;
    GraphList(GraphList&&) noexcept = default;
    GraphList& operator=(GraphList&&) noexcept = default;
    ~GraphList() = default;

    /// 交换两张图的全部所有权；所有顶点、边和迭代器引用失效。
    void swap(GraphList& other) noexcept {
        vertices_.swap(other.vertices_);
        adjacency_.swap(other.adjacency_);
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

    /// 直接构造顶点。插入可能使顶点引用失效，但既有 ID 不变。
    template<typename... Args>
    vertex_id emplaceVertex(Args&&... args) {
        vertex_type value(std::forward<Args>(args)...);
        vertices_.push_back(VertexRecord(std::move(value)));
        try {
            adjacency_.emplace_back();
        } catch (...) {
            vertices_.pop_back();
            throw;
        }
        return static_cast<vertex_id>(vertices_.size() - 1);
    }

    vertex_id addVertex(const vertex_type& value) {
        return emplaceVertex(value);
    }

    vertex_id addVertex(vertex_type&& value) {
        return emplaceVertex(std::move(value));
    }

    /// 删除顶点及全部关联边；后续顶点 ID 减一。
    /// 对 move-only 边值提供 basic guarantee；可复制边值的常规路径不会泄漏资源。
    vertex_type removeVertex(vertex_id vertex) {
        checkVertex(vertex);
        const size_type index = static_cast<size_type>(vertex);
        vertex_type removed(std::move(vertices_[index].data));

        adjacency_.erase(adjacency_.begin() + vertex);
        vertices_.erase(vertices_.begin() + vertex);

        for (size_type source = 0; source < adjacency_.size(); ++source) {
            adjacency_map rebuilt;
            adjacency_map& current = adjacency_[source];
            for (typename adjacency_map::iterator it = current.begin(); it != current.end(); ++it) {
                if (it->first == vertex)
                    continue;
                const vertex_id adjusted = it->first > vertex ? it->first - 1 : it->first;
                rebuilt.emplace(adjusted, std::move(it->second));
            }
            current.swap(rebuilt);
        }
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
        const adjacency_map& edges = adjacency_[static_cast<size_type>(from)];
        return edges.find(to) != edges.end();
    }

    /// 直接构造边；重复边不覆盖原值并返回 false。
    template<typename... Args>
    bool emplaceEdge(vertex_id from, vertex_id to, double weight, Args&&... args) {
        checkVertex(from);
        checkVertex(to);
        adjacency_map& edges = adjacency_[static_cast<size_type>(from)];
        if (edges.find(to) != edges.end())
            return false;

        edge_type value(std::forward<Args>(args)...);
        const std::pair<typename adjacency_map::iterator, bool> inserted =
            edges.emplace(to, EdgeRecord(std::move(value), weight));
        if (!inserted.second)
            return false;
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

    /// 删除边并立即析构边值；不存在时返回 false。
    bool removeEdge(vertex_id from, vertex_id to) {
        checkVertex(from);
        checkVertex(to);
        adjacency_map& edges = adjacency_[static_cast<size_type>(from)];
        const typename adjacency_map::iterator found = edges.find(to);
        if (found == edges.end())
            return false;
        edges.erase(found);
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

    /// 返回编号最大的邻接顶点；不存在时返回 -1。
    int firstNeighbor(int vertex) const {
        checkVertex(vertex);
        const adjacency_map& edges = adjacency_[static_cast<size_type>(vertex)];
        return edges.empty() ? -1 : edges.rbegin()->first;
    }

    /// 返回严格小于 current 的最大邻接顶点；不存在时返回 -1。
    int nextNeighbor(int vertex, int current) const {
        checkVertex(vertex);
        const adjacency_map& edges = adjacency_[static_cast<size_type>(vertex)];
        typename adjacency_map::const_iterator it = edges.lower_bound(current);
        if (it == edges.begin())
            return -1;
        --it;
        return it->first;
    }

    /// 原地反转全部边并保留边值、权重；边值需可复制，完成后边引用失效。
    void reverseEdges() {
        std::vector<adjacency_map> reversed(vertices_.size());
        for (size_type from = 0; from < adjacency_.size(); ++from) {
            for (typename adjacency_map::const_iterator it = adjacency_[from].begin();
                 it != adjacency_[from].end(); ++it) {
                reversed[static_cast<size_type>(it->first)].emplace(
                    static_cast<vertex_id>(from),
                    EdgeRecord(it->second.data, it->second.weight)
                );
            }
        }
        adjacency_.swap(reversed);
        recomputeMetadata();
    }

    /// 删除所有顶点和边并立即释放资源。
    void clear() noexcept {
        adjacency_.clear();
        vertices_.clear();
        edge_count_ = 0;
    }

private:
    std::vector<VertexRecord> vertices_;
    std::vector<adjacency_map> adjacency_;
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
        adjacency_map& edges = adjacency_[static_cast<size_type>(from)];
        const typename adjacency_map::iterator found = edges.find(to);
        if (found == edges.end())
            throw std::out_of_range("graph edge does not exist");
        return found->second;
    }

    const EdgeRecord& edgeRecord(vertex_id from, vertex_id to) const {
        checkVertex(from);
        checkVertex(to);
        const adjacency_map& edges = adjacency_[static_cast<size_type>(from)];
        const typename adjacency_map::const_iterator found = edges.find(to);
        if (found == edges.end())
            throw std::out_of_range("graph edge does not exist");
        return found->second;
    }

    /// 在删除顶点或反转边后统一重建边数和度数。
    void recomputeMetadata() noexcept {
        edge_count_ = 0;
        for (size_type vertex = 0; vertex < vertices_.size(); ++vertex) {
            vertices_[vertex].in_degree = 0;
            vertices_[vertex].out_degree = 0;
        }
        for (size_type from = 0; from < adjacency_.size(); ++from) {
            for (typename adjacency_map::const_iterator it = adjacency_[from].begin();
                 it != adjacency_[from].end(); ++it) {
                ++edge_count_;
                ++vertices_[from].out_degree;
                ++vertices_[static_cast<size_type>(it->first)].in_degree;
            }
        }
    }
};

/// ADL swap，复杂度 O(1)。
template<typename Tv, typename Te>
void swap(GraphList<Tv, Te>& lhs, GraphList<Tv, Te>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace container
} // namespace dsa

#endif
