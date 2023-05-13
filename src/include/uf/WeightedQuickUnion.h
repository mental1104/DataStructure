#pragma once

#include "utils.h"
#include "UnionFind.h"

class WeightedQuickUnion : public UnionFind {
protected:
    int* _sz;
public:
    WeightedQuickUnion() = delete;
    WeightedQuickUnion(int N);
    WeightedQuickUnion(std::ifstream& is);
    virtual ~WeightedQuickUnion(){   delete[] _id;  delete[] _sz;  }

    virtual int find(int p);
    virtual void unite(int p, int q);
};

WeightedQuickUnion::WeightedQuickUnion(int N){
    _N = N;
    _count = _N;
    _id = new int[N];
    for(int i = 0; i < _N; i++){
        _id[i] = i;
    }

    _sz = new int[N];
    for(int i = 0; i < _N; i++){
        _sz[i] = 1;
    }
}

WeightedQuickUnion::WeightedQuickUnion(std::ifstream& is){
    is >> _N;
    _count = _N;
    _id = new int[_N];
    //int temp;
    for(int i = 0; i < _N; i++){
        _id[i] = i;
    }
    _sz = new int[_N];
    for(int i = 0; i < _N; i++){
        _sz[i] = 1;
    }

    while(is){
        int p, q;
        is >> p >> q;
        if(connected(p, q)) continue;
        unite(p, q);
        //printf("%d %d\n", p, q);
    }
    printf("%d components\n", count());
}

int WeightedQuickUnion::find(int p){
    while(p != _id[p])
        p = _id[p];
    return p;
}

void WeightedQuickUnion::unite(int p, int q){
    int i = find(p);
    int j = find(q);

    if(i == j) return;

    //Make smaller root point to larger one
    if(_sz[i] < _sz[j]) {   _id[i] = j; _sz[j] += _sz[i]; }
    else                {   _id[j] = i; _sz[i] += _sz[j]; }
    --_count;
}