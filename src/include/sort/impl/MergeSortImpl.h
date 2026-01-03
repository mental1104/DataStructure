#pragma once

template <typename T>
inline void VectorSortImpl::topDown(Vector<T> &container, T *aux, Rank lo, Rank hi)
{
    if(lo >= hi) return;
    Rank mid = lo + (hi-lo)/2;
    topDown(container, aux, lo, mid);
    topDown(container, aux, mid+1, hi);
    merge(container, aux, lo, mid, hi);
}

template <typename T>
inline void VectorSortImpl::merge(Vector<T> &container, T *aux, Rank lo, Rank mid, Rank hi)
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
inline void VectorSortImpl::mergeSortA(Vector<T> &container, Rank lo, Rank hi)
{
    T* aux = new T[hi-lo];
    topDown(container, aux, lo, hi-1);
    delete[] aux;
}

template <typename T>
inline void VectorSortImpl::mergeSortB(Vector<T> &container, Rank lo, Rank hi)
{
    int N = hi - lo;
    T* aux = new T[N];
    for(int sz = 1; sz < N; sz = sz+sz)
        for(int start = 0; start < N-sz; start += sz+sz){
            merge(container, aux, start, start+sz-1, min(start+sz+sz-1, N-1));
        }
    delete[] aux;
}

template <typename T>
inline void ListSortImpl::mergeSort(List<T>& container) {
    if (container.size() < 2) return;
    ListNode<T>* firstNode = container.first();
    mergeSortHelper(firstNode, container.size(), container);
}

template <typename T>
inline void ListSortImpl::mergeSortHelper(ListNode<T>*& p, Rank n, List<T>& container) {
    if (n < 2) return;
    Rank m = n >> 1;
    ListNode<T>* q = p;
    for (Rank i = 0; i < m; i++) q = q->succ;
    mergeSortHelper(p, m, container);
    mergeSortHelper(q, n - m, container);
    p = merge(p, m, container, q, n - m);
}

template <typename T>
inline ListNode<T>* ListSortImpl::merge(ListNode<T>* p, Rank n, List<T>& container, ListNode<T>* q, Rank m) {
    ListNode<T>* pp = p->pred;
    while ((0 < m) && (q != p)) {
        if ((0 < n) && (p->data <= q->data)) {
            p = p->succ;
            n--;
        } else {
            container.insertB(p, container.remove((q = q->succ)->pred));
            m--;
        }
    }
    return pp->succ;
}
