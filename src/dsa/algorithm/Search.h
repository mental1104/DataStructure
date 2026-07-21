#ifndef DSA_ALGORITHM_SEARCH_H
#define DSA_ALGORITHM_SEARCH_H

#include <iterator>

namespace dsa {
namespace algorithm {

struct DefaultLess {
    template<typename Left, typename Right>
    bool operator()(const Left& left, const Right& right) const {
        return left < right;
    }
};

struct DefaultEqual {
    template<typename Left, typename Right>
    bool operator()(const Left& left, const Right& right) const {
        return left == right;
    }
};

template<
    typename BidirectionalIt,
    typename Value,
    typename Equal = DefaultEqual
>
BidirectionalIt findLast(
    BidirectionalIt first,
    BidirectionalIt last,
    const Value& value,
    Equal equal = Equal()
) {
    BidirectionalIt current = last;
    while (current != first) {
        --current;
        if (equal(*current, value))
            return current;
    }
    return last;
}

template<
    typename RandomIt,
    typename Value,
    typename Compare = DefaultLess
>
RandomIt upperBound(
    RandomIt first,
    RandomIt last,
    const Value& value,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    while (first != last) {
        const Difference count = last - first;
        RandomIt middle = first + (count >> 1);
        if (compare(value, *middle))
            last = middle;
        else
            first = middle + 1;
    }
    return first;
}

template<
    typename RandomIt,
    typename Value,
    typename Compare = DefaultLess
>
RandomIt fibonacciUpperBound(
    RandomIt first,
    RandomIt last,
    const Value& value,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference size = last - first;
    if (size <= 0)
        return first;

    Difference previousPrevious = 0;
    Difference previous = 1;
    Difference current = previousPrevious + previous;
    while (current < size) {
        previousPrevious = previous;
        previous = current;
        current = previousPrevious + previous;
    }

    Difference offset = -1;
    while (current > 1) {
        Difference index = offset + previousPrevious;
        if (index >= size)
            index = size - 1;

        if (!compare(value, *(first + index))) {
            current = previous;
            previous = previousPrevious;
            previousPrevious = current - previous;
            offset = index;
        } else {
            current = previousPrevious;
            previous = previous - previousPrevious;
            previousPrevious = current - previous;
        }
    }

    if (
        previous &&
        offset + 1 < size &&
        !compare(value, *(first + offset + 1))
    ) {
        ++offset;
    }

    return first + (offset + 1);
}

} // namespace algorithm
} // namespace dsa

#endif
