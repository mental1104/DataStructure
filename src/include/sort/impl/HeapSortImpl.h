#pragma once

template <typename T>
inline void VectorSortImpl::sink(Vector<T> &container, Rank k, int heap) {
    while (2 * k + 1 <= heap) { // 0-based：左子节点是 2*k+1
        int j = 2 * k + 1;
        if (j + 1 <= heap && container[j] < container[j + 1]) j++; // 右子节点更大
        if (!(container[k] < container[j])) break;
        swap(container[k], container[j]);
        k = j;
    }
}

template <typename T>
inline void VectorSortImpl::heapSort(Vector<T> &container, Rank lo, Rank hi) {
    int heap = hi - lo - 1;
    
    // 从 heap/2 - 1 开始构建大顶堆（0-based）
    for (int k = heap / 2; k >= 0; k--)
        sink(container, k, heap);
    
    while (heap > 0) {
        swap(container[0], container[heap--]);  // 交换根节点（最大值）到末尾
        sink(container, 0, heap);  // 重新调整堆，从根开始下沉
    }
}
