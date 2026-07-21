#pragma once

template <typename T>
inline bool VectorSortImpl::bubble(Vector<T> &container, Rank lo, Rank hi)
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
inline void VectorSortImpl::bubbleSort(Vector<T> &container, Rank lo, Rank hi)
{
    while(!bubble(container, lo, hi--));
}
