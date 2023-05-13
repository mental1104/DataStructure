#pragma once

#include "utils.h"
#include "UnionFind.h"


class QuickUnion : public UnionFind {
public:
    QuickUnion() = delete;
    QuickUnion(int N);
    QuickUnion(std::ifstream& is);
    ~QuickUnion(){   delete[] _id;    }

    virtual int find(int p);
    virtual void unite(int p, int q);
};

QuickUnion::QuickUnion(int N){
    _N = N;
    _count = _N;
    _id = new int[N];
    for(int i = 0; i < _N; i++){
        _id[i] = i;
    }
}

QuickUnion::QuickUnion(std::ifstream& is){
    is >> _N;
    _count = _N;
    _id = new int[_N];
    //int temp;
    for(int i = 0; i < _N; i++){
        _id[i] = i;
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

int QuickUnion::find(int p){
    while(p != _id[p])
        p = _id[p];
    return p;
}

void QuickUnion::unite(int p, int q){
    int pRoot = find(p);
    int qRoot = find(q);

    if(pRoot == qRoot) return;

    _id[pRoot] = qRoot;

    --_count;
}
    