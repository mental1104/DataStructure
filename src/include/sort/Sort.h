#ifndef __DSA_SORT
#define __DSA_SORT

/*
 * @Date: 2023-05-15 22:16:54
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-15 23:34:30
 */
#include "Vector.h"
#include "List.h"
#include "SortImpl.h"


template<typename T>
void Sort(Vector<T>& container, SortStrategy strategy = SortStrategy::QuickSort){
    VectorSortImpl::Sort(container, 0, container.size(), strategy);
}

template<typename T>
void Sort(List<T>& container, SortStrategy strategy = SortStrategy::MergeSort){
    ListSortImpl::Sort(container, strategy);
}

#endif