#pragma once 
#include "../def.hpp"

class QuickFind : public UnionFind{
public:
    QuickFind() = delete;
    QuickFind(int N);
    QuickFind(std::ifstream& is);
    ~QuickFind(){   delete[] _id;    }

    virtual int find(int p)    {   return _id[p];   }//quick find
    virtual void unite(int p, int q);
};

QuickFind::QuickFind(int N){
    _N = N;
    _count = _N;
    _id = new int[N];
    for(int i = 0; i < _N; i++){
        _id[i] = i;
    }
}

QuickFind::QuickFind(std::ifstream& is){
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

void QuickFind::unite(int p, int q){
    int pID = find(p);
    int qID = find(q);

    if(pID == qID) return;

    for(int i = 0; i < _N; i++)
        if(_id[i] == pID)
            _id[i] = qID;
    --_count;
}
    