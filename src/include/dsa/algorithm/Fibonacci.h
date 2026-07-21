#ifndef DSA_ALGORITHM_FIBONACCI_H
#define DSA_ALGORITHM_FIBONACCI_H

#include <limits>
#include <stdexcept>
#include <type_traits>

namespace dsa {
namespace algorithm {

// 返回第 n 个 Fibonacci 数（F(0)=0），溢出时抛出 overflow_error。
template<typename Integer>
Integer fibonacci(unsigned int n) {
    static_assert(std::is_integral<Integer>::value, "fibonacci requires integral result type");
    Integer previous = 0;
    Integer current = 1;
    if (n == 0)
        return previous;
    for (unsigned int i = 1; i < n; ++i) {
        if (current > std::numeric_limits<Integer>::max() - previous)
            throw std::overflow_error("fibonacci overflow");
        const Integer next = previous + current;
        previous = current;
        current = next;
    }
    return current;
}

// 可前后移动的 Fibonacci 游标；保持 previous/current 为相邻项。
template<typename Integer>
class FibonacciCursor {
public:
    explicit FibonacciCursor(Integer lowerBound = 0)
        : previous_(1), current_(0) {
        if (lowerBound < 0)
            throw std::invalid_argument("Fibonacci lower bound must be non-negative");
        while (current_ < lowerBound)
            next();
    }

    Integer get() const { return current_; }

    Integer next() {
        if (previous_ > std::numeric_limits<Integer>::max() - current_)
            throw std::overflow_error("Fibonacci cursor overflow");
        const Integer nextValue = previous_ + current_;
        previous_ = current_;
        current_ = nextValue;
        return current_;
    }

    Integer prev() {
        const Integer oldPrevious = previous_;
        previous_ = current_ - oldPrevious;
        current_ = oldPrevious;
        return current_;
    }

private:
    Integer previous_;
    Integer current_;
};

} // namespace algorithm
} // namespace dsa

#endif
