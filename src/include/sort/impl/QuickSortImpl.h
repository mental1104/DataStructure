#pragma once

template <typename T>
inline Rank VectorSortImpl::partition(Vector<T> &container, Rank lo, Rank hi)
{
    Rank i = lo, j = hi+1;
    T v = container[lo];
    while(true){
        while(container[++i] < v)   if(i == hi) break;
        while(v < container[--j])   if(j == lo) break;
        if(i >= j) break;
        swap(container[i], container[j]);
    }
    swap(container[lo], container[j]);
    return j;
}

template <typename T>
inline void VectorSortImpl::quick(Vector<T> &container, Rank lo, Rank hi)
{
    if (lo >= hi) return;
    Rank j = partition(container, lo, hi);
    quick(container, lo, j-1);
    quick(container, j+1, hi);
}

template <typename T>
inline void VectorSortImpl::quickSort(Vector<T> &container, Rank lo, Rank hi)
{
    quick(container, lo, hi-1);
}

template <typename T>
inline int VectorSortImpl::partitionB(Vector<T>& container, int lo, int hi) {
    T pivot = container[lo];         // 选择第一个元素作为 pivot
    int i = lo + 1;                  // i 从 lo+1 开始
    int j = hi - 1;                  // j 指向最后一个有效元素

    while (true) {
        // 向右扫描，直到遇到不小于 pivot 的元素
        while (i < hi && container[i] < pivot) {
            ++i;
        }
        // 向左扫描，直到遇到不大于 pivot 的元素
        while (j >= lo && container[j] > pivot) {
            --j;
        }
        if (i >= j) break;
        // 交换 i 和 j 处的元素
        swap(container[i], container[j]);
        ++i; --j;
    }
    // 将 pivot 放到最终位置 j
    swap(container[lo], container[j]);
    return j;
}

template <typename T>
inline void VectorSortImpl::quickB(Vector<T>& container, Rank lo, Rank hi) {
    // 当区间内元素数目较小时，使用插入排序
    if (hi - lo < 16) {
        insertionSort(container, lo, hi);
        return;
    }
    // 使用 partitionB 分区
    int j = partitionB(container, lo, hi);
    // 递归排序左侧区间 [lo, j)
    quickB(container, lo, j);
    // 递归排序右侧区间 [j+1, hi)
    quickB(container, j + 1, hi);
}

template <typename T>
inline void VectorSortImpl::quickSortB(Vector<T> &container, Rank lo, Rank hi)
{
    quickB(container, lo, hi);
}

template <typename T>
inline void VectorSortImpl::quick3way(Vector<T> &container, Rank lo, Rank hi)
{
    if(hi - lo < 2) return;  // 区间内少于 2 个元素直接返回
    Rank lt = lo, i = lo+1, gt = hi - 1;
    T v = container[lo];
    while(i <= gt){
        if (container[i] < v) {
            swap(container[lt++], container[i++]);
        }
        else if (container[i] > v) {
            swap(container[i], container[gt--]);
        }
        else {
            i++;
        }
    }
    quick3way(container, lo, lt);       // 左区间：[lo, lt)
    quick3way(container, gt+1, hi);       // 右区间：[gt+1, hi)
}
