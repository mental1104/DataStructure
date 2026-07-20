#ifndef DSA_ALGORITHM_SEQUENCE_H
#define DSA_ALGORITHM_SEQUENCE_H

#include <algorithm>
#include <iterator>
#include <random>
#include <utility>

#include "Search.h"

namespace dsa {
namespace algorithm {

template<typename RandomIt, typename UniformRandomBitGenerator>
void shuffle(
    RandomIt first,
    RandomIt last,
    UniformRandomBitGenerator& generator
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    for (Difference count = last - first; count > 1; --count) {
        std::uniform_int_distribution<Difference> distribution(0, count - 1);
        std::iter_swap(first + (count - 1), first + distribution(generator));
    }
}

template<typename InputIt, typename Visitor>
void forEach(InputIt first, InputIt last, Visitor&& visitor) {
    while (first != last) {
        visitor(*first);
        ++first;
    }
}

template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt deduplicate(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    ForwardIt write = first;

    for (ForwardIt read = first; read != last; ++read) {
        bool exists = false;
        for (ForwardIt current = first; current != write; ++current) {
            if (equal(*current, *read)) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            if (write != read)
                *write = std::move(*read);
            ++write;
        }
    }

    return write;
}

template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt uniqueAdjacent(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    if (first == last)
        return last;

    ForwardIt result = first;
    while (++first != last) {
        if (!equal(*result, *first)) {
            ++result;
            if (result != first)
                *result = std::move(*first);
        }
    }

    return ++result;
}

template<typename ForwardIt, typename Compare = DefaultLess>
typename std::iterator_traits<ForwardIt>::difference_type disorderCount(
    ForwardIt first,
    ForwardIt last,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<ForwardIt>::difference_type Difference;

    if (first == last)
        return Difference(0);

    Difference count = 0;
    ForwardIt previous = first;
    ForwardIt current = first;
    ++current;

    while (current != last) {
        if (compare(*current, *previous))
            ++count;
        previous = current;
        ++current;
    }

    return count;
}

template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt majorityCandidate(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    ForwardIt candidate = last;
    int count = 0;

    for (ForwardIt current = first; current != last; ++current) {
        if (count == 0) {
            candidate = current;
            count = 1;
        } else if (equal(*candidate, *current)) {
            ++count;
        } else {
            --count;
        }
    }

    return candidate;
}

} // namespace algorithm
} // namespace dsa

#endif
