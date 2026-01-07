#ifndef __DSA_UNION_FIND
#define __DSA_UNION_FIND

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
    if (_N <= 0 || _id == nullptr) return;
    volatile int sink = _id[0];
    (void)sink;
}

#endif
