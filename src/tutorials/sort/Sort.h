#ifndef __DSA_SORT
#define __DSA_SORT

#include <stdexcept>

#include <dsa/algorithm/Sort.h>
#include <dsa/container/list/List.h>
#include <dsa/container/vector/Vector.h>

#include "List.h"
#include "SortImpl.h"
#include "Vector.h"

/// iterator-first 公共入口，将区间排序直接转发到独立算法层。
template<typename Iterator, typename Compare>
void Sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy,
    Compare compare
) {
    dsa::algorithm::sort(first, last, strategy, compare);
}

/// 使用元素默认小于关系的 iterator-first 公共入口。
template<typename Iterator>
void Sort(
    Iterator first,
    Iterator last,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    dsa::algorithm::sort(first, last, strategy);
}

/// 教学 Vector facade 只负责把容器转换为随机访问迭代器区间。
template<typename T>
void Sort(Vector<T>& container, SortStrategy strategy = SortStrategy::QuickSort) {
    switch (strategy) {
    case SortStrategy::BubbleSort:
    case SortStrategy::SelectionSort:
    case SortStrategy::InsertionSort:
    case SortStrategy::ShellSort:
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
    case SortStrategy::QuickSort:
    case SortStrategy::Quick3way:
    case SortStrategy::QuickSortB:
    case SortStrategy::HeapSort:
        Sort(container.begin(), container.end(), strategy);
        return;
    default:
        // 教学版原先对 RadixSort 和非法枚举执行提示后空操作，继续保留该兼容语义。
        VectorSortImpl::Sort(container, 0, container.size(), strategy);
        return;
    }
}

/// 教学 List 仅把原有通用策略转发到迭代器算法，节点归并与基数排序保留专用实现。
template<typename T>
void Sort(List<T>& container, SortStrategy strategy = SortStrategy::MergeSort) {
    switch (strategy) {
    case SortStrategy::SelectionSort:
    case SortStrategy::InsertionSort:
        Sort(container.begin(), container.end(), strategy);
        return;
    case SortStrategy::MergeSort:
        ListSortImpl::Sort(container, strategy);
        return;
    case SortStrategy::RadixSort:
        ListSortImpl::Sort(container, strategy);
        return;
    default:
        // 其余策略和非法枚举仍按旧教学 facade 的提示后空操作处理。
        ListSortImpl::Sort(container, strategy);
        return;
    }
}

/// 工业 Vector facade 不再选择或实现算法，只转发完整随机访问区间。
template<typename T, typename Allocator>
void Sort(
    dsa::container::Vector<T, Allocator>& container,
    SortStrategy strategy = SortStrategy::QuickSort
) {
    Sort(container.begin(), container.end(), strategy);
}

/// 工业 List 对节点归并使用容器特化，其余可用策略转发到 iterator-first 算法。
template<typename T, typename Allocator>
void Sort(
    dsa::container::List<T, Allocator>& container,
    SortStrategy strategy = SortStrategy::MergeSort
) {
    switch (strategy) {
    case SortStrategy::MergeSort:
    case SortStrategy::MergeSortB:
        container.sort();
        return;
    case SortStrategy::RadixSort:
        throw std::invalid_argument(
            "RadixSort is only available for the teaching List"
        );
    default:
        Sort(container.begin(), container.end(), strategy);
        return;
    }
}

#endif
