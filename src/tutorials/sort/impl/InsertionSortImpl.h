#pragma once

template <typename T>
inline void VectorSortImpl::insertionSort(Vector<T> &container, Rank lo, Rank hi)
{
    for(int i = lo+1; i < hi; i++){
        for(int j = i; j > 0 && container[j] < container[j-1]; j--)
            swap(container[j], container[j-1]);
    }
}

template <typename T>
inline void ListSortImpl::insertionSort(List<T>& container)
{
    ListNode<T> * p = container.first();
    int n = container.size();
    for(int r = 0; r < n; r++){
        container.insertA(container.search(p->data, r, p), p->data);
        p = p->succ;
        container.remove(p->pred);
    }
}
