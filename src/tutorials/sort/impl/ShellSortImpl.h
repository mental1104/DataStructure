#pragma once

template <typename T>
inline void VectorSortImpl::shellSort(Vector<T> &container, Rank lo, Rank hi)
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
