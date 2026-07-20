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

/// 调用原有 VectorSortImpl 对教学版 Vector 执行指定排序策略。
template<typename T>
void Sort(Vector<T>& container, SortStrategy strategy = SortStrategy::QuickSort) {
    VectorSortImpl::Sort(container, 0, container.size(), strategy);
}

/// 调用原有 ListSortImpl 对教学版 List 执行指定排序策略。
template<typename T>
void Sort(List<T>& container, SortStrategy strategy = SortStrategy::MergeSort) {
    ListSortImpl::Sort(container, strategy);
}

namespace dsa {
namespace sort_detail {

/// 使用相邻元素比较和交换实现冒泡排序。
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

/// 每轮选择未排序区间最小元素并放到当前起点。
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

/// 通过不断向前交换，将当前元素插入前方已排序区间。
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

/// 使用 Knuth 间隔序列实现希尔排序。
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

/// 根据 SortStrategy 为随机访问迭代器区间选择排序实现。
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

/// 对工业版 allocator-aware Vector 执行指定排序策略。
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
