#ifndef __DSA_PQ
#define __DSA_PQ

template<typename T, bool MAX = true>
struct Priority {
    static bool higher(const T& a, const T& b) { //a 是否比 b 优先
        return MAX ? a > b : a < b;
    }
};

template<typename T, bool MAX = true>
struct PQ {
    virtual void insert(T) = 0;
    virtual T getMax() = 0;
    virtual T delMax() = 0;
};

#endif
