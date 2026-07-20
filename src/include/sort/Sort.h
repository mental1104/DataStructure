#ifndef __DSA_SORT
#define __DSA_SORT

/*
 * @Date: 2023-05-15 22:16:54
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 23:34:30
 */
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "Vector.h"
#include "List.h"
#include "SortImpl.h"
#include "../dsa/container/vector/Vector.h"


template<typename T>
void Sort(Vector<T>& container, SortStrategy strategy = SortStrategy::QuickSort){
    VectorSortImpl::Sort(container, 0, container.size(), strategy);
}

template<typename T>
void Sort(List<T>& container, SortStrategy strategy = SortStrategy::MergeSort){
    ListSortImpl::Sort(container, strategy);
}

namespace dsa {
namespace sort_detail {

template<typename RandomIt>
void bubbleSort(RandomIt first, RandomIt last) {
    while (first != last) {
        bool sorted = true;
        RandomIt current = first;
        if (current == last)
            return;
        RandomIt next = current;
        ++next;
        while (next != last) {
            if (*next < *current) {
                std::iter_swap(current, next);
                sorted = false;
            }
            ++current;
            ++next;
        }
        if (sorted)
            return;
        --last;
    }
}

template<typename RandomIt>
void selectionSort(RandomIt first, RandomIt last) {
    for (RandomIt current = first; current != last; ++current) {
        RandomIt minimum = current;
        for (RandomIt candidate = current + 1; candidate != last; ++candidate) {
            if (*candidate < *minimum)
                minimum = candidate;
        }
        if (minimum != current)
            std::iter_swap(current, minimum);
    }
}

template<typename RandomIt>
void insertionSort(RandomIt first, RandomIt last) {
    if (first == last)
        return;
    for (RandomIt current = first + 1; current != last; ++current) {
        RandomIt moving = current;
        while (moving != first && *moving < *(moving - 1)) {
            std::iter_swap(moving, moving - 1);
            --moving;
        }
    }
}

template<typename RandomIt>
void shellSort(RandomIt first, RandomIt last) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;
    const Difference count = last - first;
    Difference gap = 1;
    while (gap < count / 3)
        gap = 3 * gap + 1;

    while (gap >= 1) {
        for (Difference i = gap; i < count; ++i) {
            for (
                Difference j = i;
                j >= gap && *(first + j) < *(first + j - gap);
                j -= gap
            ) {
                std::iter_swap(first + j, first + j - gap);
            }
        }
        gap /= 3;
    }
}

template<typename RandomIt>
void sortRandomAccess(
    RandomIt first,
    RandomIt last,
    SortStrategy strategy
) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
        bubbleSort(first, last);
        break;
    case SortStrategy::SelectionSort:
        selectionSort(first, last);
        break;
    case SortStrategy::InsertionSort:
        insertionSort(first, last);
        break;
    case SortStrategy::ShellSort:
        shellSort(first, last);
        break;
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
        std::stable_sort(first, last);
        break;
    case SortStrategy::QuickSort:
    case SortStrategy::Quick3way:
    case SortStrategy::QuickSortB:
        std::sort(first, last);
        break;
    case SortStrategy::HeapSort:
        std::make_heap(first, last);
        std::sort_heap(first, last);
        break;
    case SortStrategy::RadixSort:
        throw std::invalid_argument(
            "RadixSort is only available for the teaching List"
        );
    }
}

} // namespace sort_detail
} // namespace dsa

template<typename T, typename Allocator>
void Sort(
    dsa::container::Vector<T, Allocator>& container,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    dsa::sort_detail::sortRandomAccess(
        container.begin(),
        container.end(),
        strategy
    );
}

#endif
