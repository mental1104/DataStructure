#ifndef __DSA_SORTIMPL
#define __DSA_SORTIMPL

#include <dsa/algorithm/Sort.h>
#include "Vector.h"
#include "List.h"
#include "common.h"

// 保留历史全局枚举名，实际定义统一位于 dsa::algorithm。
typedef dsa::algorithm::SortStrategy SortStrategy;

class VectorSortImpl {
public:
    template<typename T>
    static void Sort(Vector<T>& container, Rank lo, Rank hi, SortStrategy strategy) {
        if (lo < 0 || hi < lo || hi > container.size())
            return;
        if (strategy == SortStrategy::RadixSort)
            return;
        dsa::algorithm::sort(container.begin() + lo, container.begin() + hi, strategy);
    }
};

class ListSortImpl {
public:
    template<typename T>
    static void Sort(List<T>& container, SortStrategy strategy) {
        switch (strategy) {
        case SortStrategy::SelectionSort:
        case SortStrategy::InsertionSort:
        case SortStrategy::MergeSort:
        case SortStrategy::MergeSortB:
        case SortStrategy::RadixSort:
            dsa::algorithm::sort(container.begin(), container.end(), strategy);
            return;
        default:
            return;
        }
    }
};

#endif
