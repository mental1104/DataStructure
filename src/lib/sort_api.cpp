#include <chrono>
#include <random>
#include "Vector.h"
#include "SortImpl.h"
#include "Sort.h"

extern "C" {

// C接口，供Python调用
double sort_random_vector(int size, int algorithm) {
    if (size <= 0) return -1.0;

    // 生成随机数
    Vector<int> vec;
    std::mt19937 rng(std::random_device{}()); // 随机种子
    std::uniform_int_distribution<int> dist(0, 1000000);
    
    for (int i = 0; i < size; ++i) {
        vec.insert(dist(rng));
    }

    // 选择排序策略
    SortStrategy strategy = SortStrategy::BubbleSort;
    switch (algorithm) {
        case 0: strategy = SortStrategy::BubbleSort; break;
        case 1: strategy = SortStrategy::SelectionSort; break;
        case 2: strategy = SortStrategy::InsertionSort; break;
        case 3: strategy = SortStrategy::ShellSort; break;
        case 4: strategy = SortStrategy::MergeSort; break;
        case 5: strategy = SortStrategy::MergeSortB; break;
        case 6: strategy = SortStrategy::QuickSort; break;
        case 7: strategy = SortStrategy::Quick3way; break;
        case 8: strategy = SortStrategy::QuickSortB; break;
        case 9: strategy = SortStrategy::HeapSort; break;
        default: strategy = SortStrategy::BubbleSort; break;
    }

    // 计时开始
    auto start = std::chrono::high_resolution_clock::now();
    Sort(vec, strategy);
    auto end = std::chrono::high_resolution_clock::now();

    // 计算耗时（秒）
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

}
