#ifndef __DSA_SORT
#define __DSA_SORT

#include "Vector.h"
#include "List.h"
#include "SortImpl.h"
#include <dsa/container/vector/Vector.h>
#include <dsa/container/list/List.h>
#include <dsa/algorithm/Sort.h>
#include <stdexcept>

// 教学 Vector 保留原门面，真实排序流程由 iterator-first 算法层统一实现。
template<typename T>
void Sort(Vector<T>& container, SortStrategy strategy = SortStrategy::QuickSort) {
    VectorSortImpl::Sort(container, 0, container.size(), strategy);
}

// 教学 List 保留原门面，链式区间通过临时缓冲复用同一算法实现。
template<typename T>
void Sort(List<T>& container, SortStrategy strategy = SortStrategy::MergeSort) {
    ListSortImpl::Sort(container, strategy);
}

// 工业 Vector 直接使用随机访问迭代器。
template<typename T, typename Allocator>
void Sort(
    dsa::container::Vector<T, Allocator>& container,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    if (strategy == SortStrategy::RadixSort)
        throw std::invalid_argument(
            "RadixSort is only available for the teaching List"
        );
    if (!dsa::algorithm::sort(container.begin(), container.end(), strategy))
        throw std::invalid_argument("unsupported sort strategy");
}

// 工业 List 保留历史策略约束，但支持的策略复用 iterator-first 算法。
template<typename T, typename Allocator>
void Sort(
    dsa::container::List<T, Allocator>& container,
    SortStrategy strategy = SortStrategy::MergeSort
) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
    case SortStrategy::SelectionSort:
    case SortStrategy::InsertionSort:
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
        if (!dsa::algorithm::sort(container.begin(), container.end(), strategy))
            throw std::invalid_argument("unsupported sort strategy");
        return;
    case SortStrategy::ShellSort:
    case SortStrategy::QuickSort:
    case SortStrategy::Quick3way:
    case SortStrategy::QuickSortB:
    case SortStrategy::HeapSort:
        throw std::invalid_argument(
            "The selected strategy requires random-access iterators"
        );
    case SortStrategy::RadixSort:
        throw std::invalid_argument(
            "RadixSort is only available for the teaching List"
        );
    }
    throw std::invalid_argument("unknown sort strategy");
}

#endif
