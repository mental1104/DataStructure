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

// 教学优先级策略：MAX=true 时较大值优先，否则较小值优先。
template<typename T, bool MAX = true>
struct Priority {
    // 判断 a 的优先级是否高于 b，仅要求 T 提供 operator<。
    static bool higher(const T& a, const T& b) {
        return MAX ? b < a : a < b;
    }

    // 作为共享堆算法的函数对象入口。
    bool operator()(const T& a, const T& b) const {
        return higher(a, b);
    }
};

// 教学优先队列抽象接口；具体结构继续保留 insert/getMax/delMax 历史命名。
template<typename T, bool MAX = true>
struct PQ {
    virtual ~PQ() {}
    virtual void insert(T) = 0;
    virtual T getMax() = 0;
    virtual T delMax() = 0;
};

#endif
