#ifndef __DSA_PQ
#define __DSA_PQ

template<typename T>
struct PQ {
    virtual void insert(T) = 0;
    virtual T getMax() = 0;
    virtual T delMax() = 0;
};

#endif