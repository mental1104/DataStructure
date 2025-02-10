/*
 * @Date: 2023-05-15 22:20:37
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-25 21:31:21
 */
#ifndef __DSA_SORTIMPL
#define __DSA_SORTIMPL

#include <iostream>
#include "Vector.h"
#include "List.h"
#include "common.h"

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


class VectorSortImpl {
public:
    template<typename T>
    static void Sort(Vector<T>& container, Rank lo, Rank hi, SortStrategy strategy);
private:
    // bubbleSort
    template<typename T> static bool bubble(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void bubbleSort(Vector<T>& container, Rank lo, Rank hi);
    // selectionSort
    template<typename T> static void selectionSort(Vector<T>& container, Rank lo, Rank hi);
    // insertionSort
    template<typename T> static void insertionSort(Vector<T>& container, Rank lo, Rank hi);
    // shellSort
    template<typename T> static void shellSort(Vector<T>& container, Rank lo, Rank hi);
    // mergeSort
    template<typename T> static void topDown(Vector<T>& container, T* aux, Rank lo, Rank hi);
    template<typename T> static void merge(Vector<T>& container, T* aux, Rank lo, Rank mid, Rank hi);
    template<typename T> static void mergeSortA(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void mergeSortB(Vector<T>& container, Rank lo, Rank hi);
    // quickSort
    template<typename T> static Rank partition(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static int partitionB(Vector<T>& container, int lo, int hi);
    template<typename T> static void quick(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void quickSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void quickB(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void quickSortB(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void quick3way(Vector<T>& container, Rank lo, Rank hi);
    // heapSort
    template<typename T> static void sink(Vector<T>& container, Rank k, int heap);
    template<typename T> static void heapSort(Vector<T>& container, Rank lo, Rank hi);
};

template <typename T>
inline void VectorSortImpl::Sort(Vector<T> &container, Rank lo, Rank hi, SortStrategy strategy)
{
    switch(strategy){
    case SortStrategy::BubbleSort:
        bubbleSort(container, lo, hi);
        break;
    case SortStrategy::SelectionSort:
        selectionSort(container, lo, hi);
        break;
    case SortStrategy::InsertionSort:
        insertionSort(container, lo, hi);
        break;
    case SortStrategy::ShellSort:
        shellSort(container, lo, hi);
        break;
    case SortStrategy::MergeSort:
        mergeSortA(container, lo, hi);
        break;
    case SortStrategy::MergeSortB:
        mergeSortB(container, lo, hi);
        break;
    case SortStrategy::QuickSort:
        quickSort(container, lo, hi);
        break;
    case SortStrategy::Quick3way:
        quick3way(container, lo, hi);
        break;
    case SortStrategy::QuickSortB:
        quickSortB(container, lo, hi);
        break;
    case SortStrategy::HeapSort:
        // it's broken
        heapSort(container, lo, hi);
        break;
    default:
        std::cout << "Not supported yet! " << std::endl;
        break;
    }
}

template <typename T>
bool VectorSortImpl::bubble(Vector<T> &container, Rank lo, Rank hi)
{
    bool sorted = true;
    while(++lo < hi)
        if(container[lo - 1] > container[lo]){
            sorted = false;
            swap(container[lo-1], container[lo]);
        }
    return sorted;
}

template <typename T>
void VectorSortImpl::bubbleSort(Vector<T> &container, Rank lo, Rank hi)
{
    while(!bubble(container, lo, hi--));
}

template <typename T>
void VectorSortImpl::selectionSort(Vector<T> &container, Rank lo, Rank hi)
{
    for(int i = lo; i < hi; i++){
        int min = i;
        for(int j = i+1; j < hi; j++)
            if(container[j] < container[min]) 
                min = j;
            swap(container[i], container[min]);
    }
    return;
}

template <typename T>
void VectorSortImpl::insertionSort(Vector<T> &container, Rank lo, Rank hi)
{
    for(int i = lo+1; i < hi; i++){
        for(int j = i; j > 0 && container[j] < container[j-1]; j--)
            swap(container[j], container[j-1]);
    }
}

template <typename T>
void VectorSortImpl::shellSort(Vector<T> &container, Rank lo, Rank hi)
{
    int N = hi - lo;
    int h = 1;
    while(h < N/3) h = 3*h + 1;//1, 4, 13, 40, 121, 364, 1093, ...
    while(h >= 1){
        for(int i = lo+h; i < hi; i++){
            for(int j = i; j >= lo+h && container[j] < container[j-h]; j-=h)
                swap(container[j], container[j-h]);
        }
        h = h/3;
    }
}

template <typename T>
void VectorSortImpl::topDown(Vector<T> &container, T *aux, Rank lo, Rank hi)
{
    if(lo >= hi) return;
    Rank mid = lo + (hi-lo)/2;
    topDown(container, aux, lo, mid);
    topDown(container, aux, mid+1, hi);
    merge(container, aux, lo, mid, hi);
}

template <typename T>
void VectorSortImpl::merge(Vector<T> &container, T *aux, Rank lo, Rank mid, Rank hi)
{
    Rank i = lo, j = mid + 1;

    for(Rank k = lo; k <= hi; k++)
        aux[k] = container[k];
    
    for(Rank k = lo; k <= hi; k++)
        if      (i > mid)           container[k] = aux[j++];//当左数组耗尽时
        else if (j > hi)            container[k] = aux[i++];//当右数组耗尽时
        else if (aux[j] < aux[i])   container[k] = aux[j++];//比较两个数组队列头元素，取小者
        else                        container[k] = aux[i++];  
}

template <typename T>
void VectorSortImpl::mergeSortA(Vector<T> &container, Rank lo, Rank hi)
{
    T* aux = new T[hi-lo];
    topDown(container, aux, lo, hi-1);
    delete[] aux;
}

template <typename T>
void VectorSortImpl::mergeSortB(Vector<T> &container, Rank lo, Rank hi)
{
    int N = hi - lo;
    T* aux = new T[N];
    for(int sz = 1; sz < N; sz = sz+sz)
        for(int lo = 0; lo < N-sz; lo += sz+sz){
            merge(container, aux, lo, lo+sz-1, min(lo+sz+sz-1, N-1));
        }
    delete[] aux;
}

template <typename T>
Rank VectorSortImpl::partition(Vector<T> &container, Rank lo, Rank hi)
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
void VectorSortImpl::quick(Vector<T> &container, Rank lo, Rank hi)
{
    if (lo >= hi) return;
    Rank j = partition(container, lo, hi);
    quick(container, lo, j-1);
    quick(container, j+1, hi);
}

template <typename T>
void VectorSortImpl::quickSort(Vector<T> &container, Rank lo, Rank hi)
{
    quick(container, lo, hi-1);

}

template <typename T>
int VectorSortImpl::partitionB(Vector<T>& container, int lo, int hi) {
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
void VectorSortImpl::quickB(Vector<T>& container, int lo, int hi) {
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
void VectorSortImpl::quickSortB(Vector<T> &container, Rank lo, Rank hi)
{
    quickB(container, lo, hi);
}

template <typename T>
void VectorSortImpl::quick3way(Vector<T> &container, Rank lo, Rank hi)
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


template <typename T>
void VectorSortImpl::sink(Vector<T> &container, Rank k, int heap)
{
    while(2*k <= heap){
        int j = 2*k;
        if(j < heap && container[j] < container[j+1]) j++;
        if(!(container[k] < container[j])) break;
        swap(container[k], container[j]);
        k = j;
    }
}

template <typename T>
void VectorSortImpl::heapSort(Vector<T> &container, Rank lo, Rank hi)
{
    int heap = hi - lo - 1;
    for(int k = heap/2; k >= 1; k--)
        sink(container, k, heap);
    while(heap > 1){
        swap(container[1], container[heap--]);
        sink(container, 1, heap);
    }
}


class ListSortImpl {
public:
    template<typename T>
    static void Sort(List<T>& container, SortStrategy strategy);
private:
    // selectionSort
    template<typename T> static ListNode<T>* selectMax(ListNode<T>* p, int n);
    template<typename T> static void selectionSort(List<T>& container);
    // insertionSort
    template<typename T> static void insertionSort(List<T>& container);
    // mergeSort
    template<typename T> static void merge(ListNode<T>*& p, int n, List<T>& L, ListNode<T>* q, int m);
    template<typename T> static void mergeSort(List<T> &container, ListNode<T>* p, int n);
    // radixSort
    template<typename T> static void radixSort(List<T> &container);
};

template <typename T>
void ListSortImpl::Sort(List<T> &container, SortStrategy strategy)
{
    switch(strategy){
    case SortStrategy::SelectionSort:
        selectionSort(container);
        break;
    case SortStrategy::InsertionSort:
        insertionSort(container);
        break;
    case SortStrategy::MergeSort:
        // it's broken
        mergeSort(container, container.first(), container.size());
        break;
    case SortStrategy::RadixSort:
        radixSort(container);
        break;
    default:
        std::cout << "Not supported yet! " << std::endl;
        break;
    }
}

template <typename T>
ListNode<T> *ListSortImpl::selectMax(ListNode<T> *p, int n)
{
    ListNode<T>* max = p;
    for(ListNode<T>* cur = p; 1 < n; n--){
        cur = cur->succ;
        if(!(cur->data < max->data))
            max = cur;
    }
    return max;
}

template <typename T>
void ListSortImpl::selectionSort(List<T>& container)
{
    ListNode<T> *p = container.first();
    int n = container.size();
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++)
        tail = tail->succ;
    while(1 < n){
        ListNode<T>* max = selectMax(head->succ, n);
        container.insertB(tail, container.remove(max));
        tail = tail->pred;
        n--;
    }
}

template <typename T>
void ListSortImpl::insertionSort(List<T>& container)
{
    ListNode<T> * p = container.first();
    int n = container.size();
    for(int r = 0; r < n; r++){
        container.insertA(container.search(p->data, r, p), p->data);
        p = p->succ;
        container.remove(p->pred);
    }
}

template <typename T>
void ListSortImpl::merge(ListNode<T> *&p, int n, List<T> &L, ListNode<T> *q, int m) {
    ListNode<T>* pp = p->pred;  // 前驱作为哨兵节点
    while (m > 0) {
        // 避免 q 访问无效节点
        if (q == nullptr) break;

        if (n > 0 && p != nullptr && p->data <= q->data) {
            p = p->succ;
            n--;

            // 额外检查 p 是否变成尾哨兵
            if (p == q) break;
        } else {
            // 先保存 q->succ，确保 q 不会变成无效指针
            ListNode<T>* next_q = q->succ;

            // 确保 q 的 pred 是有效的
            if (q->pred != nullptr) {
                L.insertB(p, L.remove(q->pred));
            }

            q = next_q;
            m--;

            // 避免 q 变成无效指针后被访问
            if (q == nullptr) break;
        }
    }
    p = (pp != nullptr) ? pp->succ : nullptr;
}


template <typename T>
void ListSortImpl::mergeSort(List<T> &container, ListNode<T> *p, int n) {
    if (n < 2) return;

    int m = n / 2;
    ListNode<T>* q = p;

    // 避免 q 变成 nullptr
    for (int i = 0; i < m && q != nullptr; i++) {
        q = q->succ;
    }

    // 递归排序前要确保 q 仍然是有效指针
    if (q != nullptr) {
        mergeSort(container, p, m);
        mergeSort(container, q, n - m);
        merge(p, m, container, q, n - m);
    }
}


template <typename T>
inline void ListSortImpl::radixSort(List<T> &container)
{
    ListNode<T>* p = container.first();
    int n = container.size();
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++) tail = tail->succ;
    for(U radixBit = 0x1; radixBit && (p = head); radixBit <<= 1)
        for(int i = 0; i < n; i++){
            if(radixBit & U(p->succ->data))
                tail->insertAsPred(container.remove(p->succ));
            else
                p = p->succ;
        }
    return;
}


#endif