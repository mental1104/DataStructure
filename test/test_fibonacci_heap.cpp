#include <gtest/gtest.h>
#include <stdexcept>
#include "FibonacciHeap.h"

// 插入元素
TEST(FibonacciHeapTest, InsertTest) {
    FibonacciHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.size(), 3);
    EXPECT_EQ(heap.getMax(), 20);
}

// 删除最大值
TEST(FibonacciHeapTest, DeleteMaxTest) {
    FibonacciHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.delMax(), 20);
    EXPECT_EQ(heap.size(), 2);
    EXPECT_EQ(heap.getMax(), 10);
}

// 合并两个斐波那契堆
TEST(FibonacciHeapTest, MergeTest) {
    FibonacciHeap<int> heap1, heap2;
    heap1.insert(10);
    heap1.insert(30);
    heap2.insert(20);
    heap2.insert(40);

    heap1.merge(heap2);
    EXPECT_EQ(heap1.getMax(), 40);
    EXPECT_EQ(heap1.size(), 4);
    EXPECT_TRUE(heap2.empty());
}

// 空堆操作
TEST(FibonacciHeapTest, EmptyHeapTest) {
    FibonacciHeap<int> heap;
    EXPECT_THROW(heap.getMax(), std::runtime_error);
    EXPECT_THROW(heap.delMax(), std::runtime_error);
}
