#ifndef DSA_CONTAINER_GRAPH_GRAPH_LIST_H
#define DSA_CONTAINER_GRAPH_GRAPH_LIST_H

#include <cstddef>
#include <stdexcept>
#include <utility>

#include <dsa/container/vector/Vector.h>

#include "Graph.h"

namespace dsa {
namespace container {

/// 基于仓库 Vector 的有序邻接表工业图。
/// 顶点 ID 是连续下标，删除顶点会重编号后续顶点；每个邻接表按目标 ID 升序保存。
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
            : data(value), in_degree(0), out_degree(0) {}
        explicit VertexRecord(vertex_type&& value)
            : data(std::move(value)), in_degree(0), out_degree(0) {}
    };

    struct EdgeRecord {
        edge_type data;
        double weight;

        EdgeRecord(const edge_type& value, double edgeWeight)
            : data(value), weight(edgeWeight) {}
        EdgeRecord(edge_type&& value, double edgeWeight)
            : data(std::move(value)), weight(edgeWeight) {}
    };

    struct AdjacencyEntry {
        vertex_id target;
        EdgeRecord edge;

        AdjacencyEntry(vertex_id targetVertex, const edge_type& value, double weight)
            : target(targetVertex), edge(value, weight) {}
        AdjacencyEntry(vertex_id targetVertex, edge_type&& value, double weight)
            : target(targetVertex), edge(std::move(value), weight) {}
    };

    typedef dsa::container::Vector<AdjacencyEntry> adjacency_list;

public:
    GraphList() : edge_count_(0) {}
    GraphList(const GraphList&) = default;
    GraphList& operator=(const GraphList&) = default;
    GraphList(GraphList&&) noexcept = default;
    GraphList& operator=(GraphList&&) noexcept = default;
    ~GraphList() = default;

    void swap(GraphList& other) noexcept {
        vertices_.swap(other.vertices_);
        adjacency_.swap(other.adjacency_);
        using std::swap;
        swap(edge_count_, other.edge_count_);
    }

    size_type vertexCount() const noexcept { return vertices_.size(); }
    size_type edgeCount() const noexcept { return edge_count_; }
    bool empty() const noexcept { return vertices_.empty(); }

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

    vertex_id addVertex(const vertex_type& value) { return emplaceVertex(value); }
    vertex_id addVertex(vertex_type&& value) { return emplaceVertex(std::move(value)); }

    vertex_type removeVertex(vertex_id vertex) {
        checkVertex(vertex);
        const size_type index = static_cast<size_type>(vertex);
        vertex_type removed(std::move(vertices_[index].data));
        adjacency_.erase(adjacency_.begin() + static_cast<typename adjacency_container::difference_type>(index));
        vertices_.erase(vertices_.begin() + static_cast<typename vertex_container::difference_type>(index));

        for (size_type source = 0; source < adjacency_.size(); ++source) {
            adjacency_list& edges = adjacency_[source];
            size_type position = 0;
            while (position < edges.size()) {
                if (edges[position].target == vertex) {
                    edges.erase(edges.begin() + static_cast<typename adjacency_list::difference_type>(position));
                    continue;
                }
                if (edges[position].target > vertex)
                    --edges[position].target;
                ++position;
            }
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
        const adjacency_list& edges = adjacency_[static_cast<size_type>(from)];
        const size_type position = lowerBound(edges, to);
        return position < edges.size() && edges[position].target == to;
    }

    template<typename... Args>
    bool emplaceEdge(vertex_id from, vertex_id to, double weight, Args&&... args) {
        checkVertex(from);
        checkVertex(to);
        adjacency_list& edges = adjacency_[static_cast<size_type>(from)];
        const size_type position = lowerBound(edges, to);
        if (position < edges.size() && edges[position].target == to)
            return false;

        edge_type value(std::forward<Args>(args)...);
        edges.insert(
            edges.begin() + static_cast<typename adjacency_list::difference_type>(position),
            AdjacencyEntry(to, std::move(value), weight)
        );
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

    bool removeEdge(vertex_id from, vertex_id to) {
        checkVertex(from);
        checkVertex(to);
        adjacency_list& edges = adjacency_[static_cast<size_type>(from)];
        const size_type position = lowerBound(edges, to);
        if (position >= edges.size() || edges[position].target != to)
            return false;
        edges.erase(edges.begin() + static_cast<typename adjacency_list::difference_type>(position));
        --edge_count_;
        --vertices_[static_cast<size_type>(from)].out_degree;
        --vertices_[static_cast<size_type>(to)].in_degree;
        return true;
    }

    edge_type& edge(vertex_id from, vertex_id to) { return edgeRecord(from, to).data; }
    const edge_type& edge(vertex_id from, vertex_id to) const { return edgeRecord(from, to).data; }
    double edgeWeight(vertex_id from, vertex_id to) const { return edgeRecord(from, to).weight; }
    void setEdgeWeight(vertex_id from, vertex_id to, double weight) { edgeRecord(from, to).weight = weight; }

    int firstNeighbor(int vertex) const {
        checkVertex(vertex);
        const adjacency_list& edges = adjacency_[static_cast<size_type>(vertex)];
        return edges.empty() ? -1 : edges.back().target;
    }

    int nextNeighbor(int vertex, int current) const {
        checkVertex(vertex);
        const adjacency_list& edges = adjacency_[static_cast<size_type>(vertex)];
        const size_type position = lowerBound(edges, current);
        return position == 0 ? -1 : edges[position - 1].target;
    }

    void reverseEdges() {
        adjacency_container reversed(vertices_.size());
        for (size_type from = 0; from < adjacency_.size(); ++from) {
            const adjacency_list& edges = adjacency_[from];
            for (typename adjacency_list::const_iterator it = edges.begin(); it != edges.end(); ++it) {
                adjacency_list& target = reversed[static_cast<size_type>(it->target)];
                const size_type position = lowerBound(target, static_cast<vertex_id>(from));
                target.insert(
                    target.begin() + static_cast<typename adjacency_list::difference_type>(position),
                    AdjacencyEntry(static_cast<vertex_id>(from), it->edge.data, it->edge.weight)
                );
            }
        }
        adjacency_.swap(reversed);
        recomputeMetadata();
    }

    void clear() noexcept {
        adjacency_.clear();
        vertices_.clear();
        edge_count_ = 0;
    }

private:
    typedef dsa::container::Vector<VertexRecord> vertex_container;
    typedef dsa::container::Vector<adjacency_list> adjacency_container;

    vertex_container vertices_;
    adjacency_container adjacency_;
    size_type edge_count_;

    static size_type lowerBound(const adjacency_list& edges, vertex_id target) {
        size_type first = 0;
        size_type count = edges.size();
        while (count != 0) {
            const size_type step = count / 2;
            const size_type middle = first + step;
            if (edges[middle].target < target) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

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
        adjacency_list& edges = adjacency_[static_cast<size_type>(from)];
        const size_type position = lowerBound(edges, to);
        if (position >= edges.size() || edges[position].target != to)
            throw std::out_of_range("graph edge does not exist");
        return edges[position].edge;
    }
    const EdgeRecord& edgeRecord(vertex_id from, vertex_id to) const {
        checkVertex(from);
        checkVertex(to);
        const adjacency_list& edges = adjacency_[static_cast<size_type>(from)];
        const size_type position = lowerBound(edges, to);
        if (position >= edges.size() || edges[position].target != to)
            throw std::out_of_range("graph edge does not exist");
        return edges[position].edge;
    }

    void recomputeMetadata() noexcept {
        edge_count_ = 0;
        for (size_type vertexIndex = 0; vertexIndex < vertices_.size(); ++vertexIndex) {
            vertices_[vertexIndex].in_degree = 0;
            vertices_[vertexIndex].out_degree = 0;
        }
        for (size_type from = 0; from < adjacency_.size(); ++from) {
            const adjacency_list& edges = adjacency_[from];
            for (typename adjacency_list::const_iterator it = edges.begin(); it != edges.end(); ++it) {
                ++edge_count_;
                ++vertices_[from].out_degree;
                ++vertices_[static_cast<size_type>(it->target)].in_degree;
            }
        }
    }
};

template<typename Tv, typename Te>
void swap(GraphList<Tv, Te>& left, GraphList<Tv, Te>& right) noexcept {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
