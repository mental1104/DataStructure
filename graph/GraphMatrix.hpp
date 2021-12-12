#pragma once

#include "../vector/Vector.hpp"  
#include "Graph.hpp" 
#include <fstream>

using std::ifstream;

template<typename Tv>
struct Vertex {
    Tv data;
    int inDegree{0};
    int outDegree{0};
    VStatus status{VStatus::UNDISCOVERED};
    int dTime{-1};
    int fTime{-1};
    int parent{-1};
    int priority{INT_MAX};
    Vertex(Tv const& d = Tv(0)):data{d}{}
};

template<typename Te>
struct Edge {
    Te data;
    double weight;
    EType type{EType::UNDETERMINED};
    Edge(Te const& d, double w):data{d}, weight{w} {}
};

template<typename Tv, typename Te>
class GraphMatrix: public Graph<Tv, Te> {
private:
    Vector<Vertex<Tv>> _V{1000};//顶点集
    Vector<Vector<Edge<Te>*>> _E{1000};//边集
public:
    GraphMatrix() { 
        this->n = 0;
        this->e = 0; 
    }
    GraphMatrix(ifstream& alg4, GType type);
    ~GraphMatrix() {
        for(int j = 0; j < this->n; j++)
            for(int k = 0; k < this->n; k++)
                delete _E[j][k];
    }
    const Vector<Vertex<Tv>>& V() const {  return _V;  }
    const Vector<Vector<Edge<Te>*>>& E() const {  return _E;}
    //基本操作
    virtual Tv& vertex(int i) { return _V[i].data; }
    virtual int inDegree(int i) {   return _V[i].inDegree; }
    virtual int outDegree(int i) {  return _V[i].outDegree; }
    virtual int firstNbr(int i) {   return nextNbr(i, this->n); }
    virtual int nextNbr(int i, int j){//相对于顶点j的下一邻接顶点
        while((-1 < j) && (!exists(i, --j)))
            ; 
        return j;
    }
    virtual VStatus& status(int i) {    return _V[i].status; }
    virtual int& dTime(int i) { return _V[i].dTime; }
    virtual int& fTime(int i) { return _V[i].fTime; }
    virtual int& parent(int i) {    return _V[i].parent; }
    virtual int& priority(int i) {   return _V[i].priority; }
    //顶点动态操作
    virtual int insert(Tv const& vertex);
    virtual Tv remove(int i);
    //边的确认操作
    virtual bool exists(int i, int j);
    //边的基本操作
    virtual EType& type(int i, int j) { return _E[i][j]->type; }
    virtual Te& edge(int i, int j) {  return _E[i][j]->data; }
    virtual double& weight(int i, int j) { return _E[i][j]->weight; }
    //边的动态操作
    virtual void insert(Te const& edge, int i, int j, int w = 0);
    virtual Te remove(int i, int j);
};


template<typename Tv, typename Te>
GraphMatrix<Tv, Te>::GraphMatrix(ifstream& alg4, GType type){
    int vNum = 0;
    int eNum = 0;
    alg4 >> vNum >> eNum;
    for(int i = 0; i < vNum; i++)
        this->insert(Tv());
    
    int src = 0, det = 0;
    double weg = 0.0;

    switch(type){
        case GType::GRAPH:
            for(int i = 0; i < eNum; i++){
                alg4 >> src >> det;
                this->insert(Te(), src, det, weg);
            }
            break;
        
        case GType::WEIGHTEDGRAPH:
            for(int i = 0; i < eNum; i++){
                alg4 >> src >> det >> weg;
                this->insert(Te(), src, det, weg);
            }
    }
}

template<typename Tv, typename Te>
int GraphMatrix<Tv, Te>::insert(Tv const& vertex){
    for(int j = 0; j < this->n; j++){
        _E[j].insert(nullptr);
    }
        
    this->n++;
    _E.insert(Vector<Edge<Te>*>(this->n, this->n, nullptr));
    return _V.insert(Vertex<Tv>(vertex));
}

template<typename Tv, typename Te>
Tv GraphMatrix<Tv, Te>::remove(int i){
    for(int j = 0; j < this->n; j++)
        if(exists(i, j)){
            delete _E[i][j];
            _V[j].inDegree--;
        }
    _E.remove(i);
    this->n--;
    Tv vBak = vertex(i);
    _V.remove(i);
    for(int j = 0; j < this->n; j++)
        if(Edge<Te>* e = _E[j].remove(i)){
            delete e;
            _V[j].outDegree--;
        }
    return vBak;
}
template<typename Tv, typename Te>
bool GraphMatrix<Tv, Te>::exists(int i, int j){
    return (0 <= i) && (i < this->n) && (0 <= j) && (j < this->n) && _E[i][j] != nullptr; 
}

template<typename Tv, typename Te>
void GraphMatrix<Tv, Te>::insert(Te const& edge, int i, int j, int w){
    if(exists(i, j)) 
        return;
    _E[i][j] = new Edge<Te>(edge, w);
    this->e++;
    _V[i].outDegree++;
    _V[j].inDegree++;
}

template<typename Tv, typename Te>
Te GraphMatrix<Tv, Te>::remove(int i, int j){
    Te eBak = edge(i, j);
    delete _E[i][j];
    _E[i][j] = nullptr;
    this->e--;
    _V[i].outDegree--;
    _V[j].inDegree--;
    return eBak;  
}




