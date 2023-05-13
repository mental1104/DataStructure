#pragma once

class UnionFind{
protected:
    int* _id;
    int _count;
    int _N;
public:
    virtual int count() {   return _count;  }
    virtual bool connected(int p, int q) {    return find(p) == find(q);    }
    virtual int find(int p) = 0;
    virtual void unite(int p, int q) = 0;

    void traverse();
};

void UnionFind::traverse(){
    return;
    // for(int i = 0; i < _N; i++)
    //     printf("%d ", _id[i]);
    // printf("\n");
}