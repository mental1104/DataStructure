#ifndef __DSA_GRAPH_LIST
#define __DSA_GRAPH_LIST

#include <fstream>
#include "Graph.h"
#include "List.h"

// Adjacency-list implementation of Graph
template<typename Tv, typename Te>
class GraphList : public Graph<Tv, Te> {
private:
    Vector<Vertex<Tv>> _V;              // vertex set
    Vector<List<Edge<Te>*>> _adj;       // adjacency lists

    Edge<Te>* findEdge(int i, int j, ListNode<Edge<Te>*>*& pos) {
        pos = nullptr;
        if (i < 0 || i >= this->n) return nullptr;
        for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ) {
            if (p->data->y == j) { pos = p; return p->data; }
        }
        return nullptr;
    }

public:
    GraphList() { this->n = 0; this->e = 0; }
    GraphList(std::ifstream& alg4, GType type) { this->n = 0; this->e = 0;
        int vNum = 0, eNum = 0;
        alg4 >> vNum >> eNum;
        for (int i = 0; i < vNum; i++) insert(Tv(i));
        int src = 0, det = 0; double weg = 0.0;
        switch (type) {
            case GType::DIGRAPH:
                for (int k = 0; k < eNum; k++) { alg4 >> src >> det; insert(Te(k), src, det); }
                break;
            case GType::UNDIGRAPH:
                for (int k = 0; k < eNum; k++) { alg4 >> src >> det; insert(Te(k), src, det); insert(Te(k), det, src); }
                break;
            case GType::WEIGHTEDDIGRAPH:
                for (int k = 0; k < eNum; k++) { alg4 >> src >> det >> weg; insert(Te(weg), src, det, weg); }
                break;
            case GType::WEIGHTEDUNDIGRAPH:
                for (int k = 0; k < eNum; k++) { alg4 >> src >> det >> weg; insert(Te(weg), src, det, weg); insert(Te(weg), det, src, weg); }
                break;
        }
    }
    ~GraphList() {
        for (int i = 0; i < this->n; ++i)
            for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ)
                delete p->data;
    }

    // vertex getters
    virtual Tv& vertex(int i) { return _V[i].data; }
    virtual int inDegree(int i) { return _V[i].inDegree; }
    virtual int outDegree(int i) { return _V[i].outDegree; }
    virtual int firstNbr(int i) {
        if (i < 0 || i >= this->n) return -1;
        auto p = _adj[i].first();
        return _adj[i].valid(p) ? p->data->y : -1;
    }
    virtual int nextNbr(int i, int j) {
        if (i < 0 || i >= this->n) return -1;
        int candidate = -1;
        for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ) {
            int nbr = p->data->y;
            if (nbr < j && nbr > candidate) candidate = nbr; // follow matrix-style descending search
        }
        return candidate;
    }
    virtual VStatus& status(int i) { return _V[i].status; }
    virtual int& dTime(int i) { return _V[i].dTime; }
    virtual int& fTime(int i) { return _V[i].fTime; }
    virtual int& hca(int i) { return fTime(i); }
    virtual int& parent(int i) { return _V[i].parent; }
    virtual double& priority(int i) { return _V[i].priority; }

    // vertex dynamic ops
    virtual int insert(Tv const& vertex) {
        _adj.insert(List<Edge<Te>*>());
        this->n++;
        return _V.insert(Vertex<Tv>(this->n - 1, vertex));
    }

    virtual Tv remove(int i) {
        if (i < 0 || i >= this->n) return Tv();
        // remove outgoing
        for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ) {
            Edge<Te>* edge_ptr = p->data;
            _V[edge_ptr->y].inDegree--;
            delete edge_ptr;
            this->e--;
        }
        _adj.remove(i);
        Tv vBak = vertex(i);
        _V.remove(i);
        this->n--;

        // remove incoming and fix indices greater than i
        for (int u = 0; u < this->n; ++u) {
            for (auto p = _adj[u].first(); _adj[u].valid(p); ) {
                auto curr = p;
                p = p->succ;
                Edge<Te>* edge_ptr = curr->data;
                if (edge_ptr->y == i) {
                    _adj[u].remove(curr);
                    _V[u].outDegree--;
                    this->e--;
                    delete edge_ptr;
                } else {
                    if (edge_ptr->y > i) edge_ptr->y--;
                    if (edge_ptr->x > i) edge_ptr->x--;
                }
            }
        }
        return vBak;
    }

    // edge queries
    virtual bool exists(int i, int j) {
        ListNode<Edge<Te>*>* pos = nullptr;
        return findEdge(i, j, pos) != nullptr;
    }

    virtual EType& type(int i, int j) {
        static EType dummy = EType::UNDETERMINED;
        ListNode<Edge<Te>*>* pos = nullptr;
        Edge<Te>* edge = findEdge(i, j, pos);
        return edge ? edge->type : dummy;
    }
    virtual Te& edge(int i, int j) {
        ListNode<Edge<Te>*>* pos = nullptr;
        return findEdge(i, j, pos)->data;
    }
    virtual double& weight(int i, int j) {
        ListNode<Edge<Te>*>* pos = nullptr;
        return findEdge(i, j, pos)->weight;
    }

    // edge dynamic ops
    virtual void insert(Te const& edge, int i, int j, double w = 0.0) {
        if (exists(i, j)) return;
        Edge<Te>* edge_ptr = new Edge<Te>(edge, w, i, j);
        _adj[i].insertAsFirst(edge_ptr);
        this->e++;
        _V[i].outDegree++;
        _V[j].inDegree++;
    }

    virtual Te remove(int i, int j) {
        ListNode<Edge<Te>*>* pos = nullptr;
        Edge<Te>* edge_ptr = findEdge(i, j, pos);
        if (!edge_ptr) return Te();
        Te data = edge_ptr->data;
        _adj[i].remove(pos);
        this->e--;
        _V[i].outDegree--;
        _V[j].inDegree--;
        delete edge_ptr;
        return data;
    }

    virtual void reverse() {
        Vector<List<Edge<Te>*>> newAdj(this->n, this->n, List<Edge<Te>*>());
        for (int i = 0; i < this->n; ++i) {
            for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ) {
                Edge<Te>* edge_ptr = p->data;
                Edge<Te>* reversed_edge = new Edge<Te>(edge_ptr->data, edge_ptr->weight, edge_ptr->y, edge_ptr->x);
                newAdj[edge_ptr->y].insertAsFirst(reversed_edge);
            }
        }
        // clear old edges
        for (int i = 0; i < this->n; ++i) {
            for (auto p = _adj[i].first(); _adj[i].valid(p); p = p->succ) {
                delete p->data;
            }
        }
        _adj = newAdj;
        // swap in/out degrees
        for (int i = 0; i < this->n; ++i) {
            std::swap(_V[i].inDegree, _V[i].outDegree);
        }
    }
};

#endif
