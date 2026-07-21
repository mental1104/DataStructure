#ifndef DSA_ALGORITHM_SORT_H
#define DSA_ALGORITHM_SORT_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace dsa {
namespace algorithm {

// 统一排序策略；未知枚举值返回 false，不修改已知范围之外的状态。
enum class SortStrategy {
    BubbleSort,
    SelectionSort,
    InsertionSort,
    ShellSort,
    MergeSort,
    MergeSortB,
    QuickSort,
    Quick3way,
    QuickSortB,
    HeapSort,
    RadixSort
};

namespace detail {

template<typename RandomIt, typename Compare>
void bubbleSort(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::difference_type difference_type;
    difference_type count = last - first;
    while (count > 1) {
        bool sorted = true;
        for (difference_type i = 1; i < count; ++i) {
            if (compare(*(first + i), *(first + i - 1))) {
                std::iter_swap(first + i, first + i - 1);
                sorted = false;
            }
        }
        if (sorted)
            return;
        --count;
    }
}

template<typename RandomIt, typename Compare>
void selectionSort(RandomIt first, RandomIt last, Compare compare) {
    for (RandomIt current = first; current != last; ++current) {
        RandomIt selected = current;
        for (RandomIt candidate = current + 1; candidate != last; ++candidate) {
            if (compare(*candidate, *selected))
                selected = candidate;
        }
        if (selected != current)
            std::iter_swap(current, selected);
    }
}

template<typename RandomIt, typename Compare>
void insertionSort(RandomIt first, RandomIt last, Compare compare) {
    if (first == last)
        return;
    for (RandomIt current = first + 1; current != last; ++current) {
        RandomIt moving = current;
        while (moving != first && compare(*moving, *(moving - 1))) {
            std::iter_swap(moving, moving - 1);
            --moving;
        }
    }
}

template<typename RandomIt, typename Compare>
void shellSort(RandomIt first, RandomIt last, Compare compare) {
    typedef typename std::iterator_traits<RandomIt>::difference_type difference_type;
    const difference_type count = last - first;
    difference_type gap = 1;
    while (gap < count / 3)
        gap = gap * 3 + 1;
    while (gap > 0) {
        for (difference_type i = gap; i < count; ++i) {
            difference_type j = i;
            while (j >= gap && compare(*(first + j), *(first + j - gap))) {
                std::iter_swap(first + j, first + j - gap);
                j -= gap;
            }
        }
        gap /= 3;
    }
}

template<typename RandomIt, typename Compare>
void quick3way(RandomIt first, RandomIt last, Compare compare) {
    if (last - first < 2)
        return;
    typedef typename std::iterator_traits<RandomIt>::value_type value_type;
    value_type pivot(*(first + (last - first) / 2));
    RandomIt lower = first;
    RandomIt current = first;
    RandomIt upper = last;
    while (current != upper) {
        if (compare(*current, pivot)) {
            std::iter_swap(lower, current);
            ++lower;
            ++current;
        } else if (compare(pivot, *current)) {
            --upper;
            std::iter_swap(current, upper);
        } else {
            ++current;
        }
    }
    quick3way(first, lower, compare);
    quick3way(upper, last, compare);
}

template<typename Unsigned>
Unsigned signAdjusted(Unsigned value, std::true_type) {
    return value ^ (Unsigned(1) << (sizeof(Unsigned) * 8 - 1));
}

template<typename Unsigned>
Unsigned signAdjusted(Unsigned value, std::false_type) {
    return value;
}

template<typename RandomIt>
bool radixSortIntegral(RandomIt first, RandomIt last, std::true_type) {
    typedef typename std::iterator_traits<RandomIt>::value_type value_type;
    typedef typename std::make_unsigned<value_type>::type unsigned_type;
    typedef typename std::iterator_traits<RandomIt>::difference_type difference_type;
    const difference_type count = last - first;
    if (count < 2)
        return true;

    std::vector<value_type> buffer(static_cast<std::size_t>(count));
    for (std::size_t byte = 0; byte < sizeof(value_type); ++byte) {
        std::size_t frequencies[256] = {};
        for (RandomIt it = first; it != last; ++it) {
            unsigned_type raw = static_cast<unsigned_type>(*it);
            raw = signAdjusted(raw, typename std::is_signed<value_type>::type());
            ++frequencies[(raw >> (byte * 8)) & 0xffu];
        }
        std::size_t offsets[256];
        offsets[0] = 0;
        for (std::size_t i = 1; i < 256; ++i)
            offsets[i] = offsets[i - 1] + frequencies[i - 1];
        for (RandomIt it = first; it != last; ++it) {
            unsigned_type raw = static_cast<unsigned_type>(*it);
            raw = signAdjusted(raw, typename std::is_signed<value_type>::type());
            buffer[offsets[(raw >> (byte * 8)) & 0xffu]++] = std::move(*it);
        }
        for (difference_type i = 0; i < count; ++i)
            *(first + i) = std::move(buffer[static_cast<std::size_t>(i)]);
    }
    return true;
}

template<typename RandomIt>
bool radixSortIntegral(RandomIt, RandomIt, std::false_type) {
    return false;
}

template<typename RandomIt, typename Compare>
bool sortRandomAccess(RandomIt first, RandomIt last, SortStrategy strategy, Compare compare) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
        bubbleSort(first, last, compare);
        return true;
    case SortStrategy::SelectionSort:
        selectionSort(first, last, compare);
        return true;
    case SortStrategy::InsertionSort:
        insertionSort(first, last, compare);
        return true;
    case SortStrategy::ShellSort:
        shellSort(first, last, compare);
        return true;
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
        std::stable_sort(first, last, compare);
        return true;
    case SortStrategy::QuickSort:
    case SortStrategy::QuickSortB:
        std::sort(first, last, compare);
        return true;
    case SortStrategy::Quick3way:
        std::sort(first, last, compare);
        return true;
    case SortStrategy::HeapSort:
        std::make_heap(first, last, compare);
        std::sort_heap(first, last, compare);
        return true;
    case SortStrategy::RadixSort:
        return radixSortIntegral(
            first,
            last,
            typename std::is_integral<
                typename std::iterator_traits<RandomIt>::value_type
            >::type()
        );
    }
    return false;
}

template<typename Iterator, typename Compare>
bool sortDispatch(
    Iterator first,
    Iterator last,
    SortStrategy strategy,
    Compare compare,
    std::random_access_iterator_tag
) {
    return sortRandomAccess(first, last, strategy, compare);
}

template<typename Iterator, typename Compare, typename Category>
bool sortDispatch(
    Iterator first,
    Iterator last,
    SortStrategy strategy,
    Compare compare,
    Category
) {
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    std::vector<value_type> values;
    for (Iterator it = first; it != last; ++it)
        values.push_back(*it);
    if (!sortRandomAccess(values.begin(), values.end(), strategy, compare))
        return false;
    typename std::vector<value_type>::iterator source = values.begin();
    for (Iterator target = first; target != last; ++target, ++source)
        *target = std::move(*source);
    return true;
}

} // namespace detail

// 对任意标准迭代器区间排序。随机访问迭代器原地执行；其他迭代器通过临时连续缓冲复用同一算法。
template<typename Iterator, typename Compare>
bool sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy,
    Compare compare
) {
    typedef typename std::iterator_traits<Iterator>::iterator_category category;
    return detail::sortDispatch(first, last, strategy, compare, category());
}

template<typename Iterator>
bool sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    return dsa::algorithm::sort(first, last, strategy, std::less<value_type>());
}

} // namespace algorithm
} // namespace dsa

#endif
