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
    template<typename T> static bool bubble(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void bubbleSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void selectionSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void insertionSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void shellSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void topDown(Vector<T>& container, T* aux, Rank lo, Rank hi);
    template<typename T> static void merge(Vector<T>& container, T* aux, Rank lo, Rank mid, Rank hi);
    template<typename T> static void mergeSortA(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void mergeSortB(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static Rank partition(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static int partitionB(Vector<T>& container, int lo, int hi);
    template<typename T> static void quick(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void quickSort(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void quickB(Vector<T>& container, Rank lo, Rank hi);
    template<typename T> static void quickSortB(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void quick3way(Vector<T>& container, Rank lo, Rank hi);

    template<typename T> static void sink(Vector<T>& container, Rank k, int heap);
    template<typename T> static void heapSort(Vector<T>& container, Rank lo, Rank hi);
};

class ListSortImpl {
public:
    template<typename T>
    static void Sort(List<T>& container, SortStrategy strategy);
private:
    template<typename T> static ListNode<T>* selectMax(ListNode<T>* p, int n);
    template<typename T> static void selectionSort(List<T>& container);

    template<typename T> static void insertionSort(List<T>& container);

    template<typename T> static void mergeSort(List<T>& container);
    template<typename T> static void mergeSortHelper(ListNode<T>*& p, Rank n, List<T>& container);
    template<typename T> static ListNode<T>* merge(ListNode<T>* p, Rank n, List<T>& container, ListNode<T>* q, Rank m);

    template<typename T> static void radixSort(List<T> &container);
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
inline void ListSortImpl::Sort(List<T> &container, SortStrategy strategy)
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
        mergeSort(container);
        break;
    case SortStrategy::RadixSort:
        radixSort(container);
        break;
    default:
        std::cout << "Not supported yet! " << std::endl;
        break;
    }
}

#include "impl/BubbleSortImpl.h"
#include "impl/SelectionSortImpl.h"
#include "impl/InsertionSortImpl.h"
#include "impl/ShellSortImpl.h"
#include "impl/MergeSortImpl.h"
#include "impl/QuickSortImpl.h"
#include "impl/HeapSortImpl.h"

#endif
