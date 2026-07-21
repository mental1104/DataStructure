#ifndef __DSA_FIB
#define __DSA_FIB

#include <dsa/algorithm/Fibonacci.h>

// 教学门面保留原 int API，状态推进统一复用通用 Fibonacci 游标。
class Fib {
public:
    explicit Fib(int n) : cursor_(n) {}
    int get() const { return cursor_.get(); }
    int next() { return cursor_.next(); }
    int prev() { return cursor_.prev(); }

private:
    dsa::algorithm::FibonacciCursor<int> cursor_;
};

#endif
