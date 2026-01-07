// test_sort.cpp
#include "gtest/gtest.h"
#include "Sort.h"      // 包含 Sort(Vector<T>&, SortStrategy) 与 Sort(List<T>&, SortStrategy)
#include "Vector.h"    // 自定义 Vector 类（测试时用于存放 int 数据）
#include "List.h"      // 自定义 List 类（测试时用于存放 int 数据）
#include <vector>
#include <iostream>

// 辅助函数：检查容器（支持 begin()/end() 迭代器）是否已排序
template <typename Container>
bool isSorted(const Container &container) {
    if (container.size() < 2)
        return true;
    auto it = container.begin();
    auto next = it;
    ++next;
    while (next != container.end()) {
        if (*it > *next)
            return false;
        ++it;
        ++next;
    }
    return true;
}

// ======================== Vector 排序单元测试 =========================

// 测试不同排序策略下 Vector 排序的正确性（HeapSort 策略被注释为“有问题”，因此不测试）
TEST(VectorSortTest, VariousStrategies) {
    // 针对 Vector 的排序策略
    std::vector<SortStrategy> strategies = {
        SortStrategy::BubbleSort,
        SortStrategy::SelectionSort,
        SortStrategy::InsertionSort,
        SortStrategy::ShellSort,
        SortStrategy::MergeSort,
        SortStrategy::MergeSortB,
        SortStrategy::QuickSort,
        SortStrategy::Quick3way,
        SortStrategy::QuickSortB,
        SortStrategy::HeapSort
    };

    // 对每种策略构造同样的测试数据
    for (auto strat : strategies) {
        Vector<int> vec;
        // 注意：这里使用 insert(e) 函数，默认插入到末尾
        for(int i = 0; i < 20; i++) {
            vec.insert(5);
            vec.insert(3);
            vec.insert(8);
            vec.insert(1);
            vec.insert(9);
            vec.insert(2);
        }
        // 调用排序函数
        Sort(vec, strat);

        // 检查排序结果是否正确
        EXPECT_TRUE(isSorted(vec)) 
            << "Vector 排序失败, 策略: " << static_cast<int>(strat);
    }
}

// 针对空 Vector 与单元素 Vector 的边界测试
TEST(VectorSortTest, EdgeCases) {
    { // 空向量测试
        Vector<int> vecEmpty;
        Sort(vecEmpty, SortStrategy::QuickSort);
        EXPECT_TRUE(isSorted(vecEmpty));
    }
    { // 单元素向量测试
        Vector<int> vecSingle;
        vecSingle.insert(42);
        Sort(vecSingle, SortStrategy::QuickSort);
        EXPECT_TRUE(isSorted(vecSingle));
        EXPECT_EQ(vecSingle[0], 42);
    }
}

// ======================== List 排序单元测试 =========================

// 测试不同排序策略下 List 排序的正确性
TEST(ListSortTest, VariousStrategies) {
    // 针对 List 的排序策略
    std::vector<SortStrategy> strategies = {
        SortStrategy::SelectionSort,
        SortStrategy::InsertionSort,
        SortStrategy::MergeSort,
        SortStrategy::RadixSort
    };

    for (auto strat : strategies) {
        List<int> lst;
        // 使用 List 的 insert 函数默认插入到末尾
        lst.insert(5);
        lst.insert(3);
        lst.insert(8);
        lst.insert(1);
        lst.insert(9);
        lst.insert(2);

        Sort(lst, strat);

        // 遍历 List 检查是否已排序
        auto it = lst.begin();
        auto next = it;
        ++next;
        bool sorted = true;
        while (next != lst.end()) {
            if (*it > *next) {
                sorted = false;
                break;
            }
            ++it;
            ++next;
        }
        EXPECT_TRUE(sorted) 
            << "List 排序失败, 策略: " << static_cast<int>(strat);
    }
}

// 针对空 List 与单元素 List 的边界测试
TEST(ListSortTest, EdgeCases) {
    { // 空链表测试
        List<int> lstEmpty;
        Sort(lstEmpty, SortStrategy::MergeSort);
        EXPECT_EQ(lstEmpty.begin(), lstEmpty.end());
    }
    { // 单元素链表测试
        List<int> lstSingle;
        lstSingle.insert(100);
        Sort(lstSingle, SortStrategy::MergeSort);
        auto it = lstSingle.begin();
        EXPECT_NE(it, lstSingle.end());
        EXPECT_EQ(*it, 100);
    }
}

TEST(SortImplTest, UnsupportedStrategies) {
    Vector<int> vec;
    vec.insert(3);
    vec.insert(1);
    Sort(vec, static_cast<SortStrategy>(999));
    EXPECT_EQ(vec.size(), 2);

    List<int> lst;
    lst.insert(2);
    lst.insert(1);
    Sort(lst, static_cast<SortStrategy>(999));
    EXPECT_EQ(lst.size(), 2);
}
