#ifndef __DSA_PQ
#define __DSA_PQ

#ifndef DSA_NOINLINE
#if defined(_MSC_VER)
#define DSA_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define DSA_NOINLINE __attribute__((noinline))
#else
#define DSA_NOINLINE
#endif
#endif

template<typename T, bool MAX = true>
struct Priority {
    static bool higher(const T& a, const T& b) { //a 是否比 b 优先
        return MAX ? b < a : a < b; //仅依赖operator<，减少自定义类型要求
    }
};

template<typename T, bool MAX = true>
struct PQ {
    virtual void insert(T) = 0;
    virtual T getMax() = 0;
    virtual T delMax() = 0;
};

#endif
