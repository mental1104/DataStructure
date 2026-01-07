#include <gtest/gtest.h>
#include "LeftHeap.h" // 假设 LeftHeap 的实现文件
#include "Vector.h"

// 测试插入元素
TEST(LeftHeapTest, InsertTest) {
    LeftHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.getMax(), 20);
}

// 测试删除最大值
TEST(LeftHeapTest, DeleteMaxTest) {
    LeftHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    heap.delMax();
    EXPECT_EQ(heap.getMax(), 10);
}

// 测试合并两个左式堆
TEST(LeftHeapTest, MergeTest) {
    LeftHeap<int> heap1, heap2;
    heap1.insert(10);
    heap1.insert(30);
    heap2.insert(20);
    heap2.insert(40);
    
    heap1.merge(heap2);
    EXPECT_EQ(heap1.getMax(), 40);
}

// 测试空堆操作
TEST(LeftHeapTest, EmptyHeapTest) {
    LeftHeap<int> heap;
    EXPECT_THROW(heap.getMax(), std::runtime_error);
    EXPECT_THROW(heap.delMax(), std::runtime_error);
}

// 小顶堆
TEST(LeftHeapTest, MinHeapTest) {
    LeftHeap<int, false> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.getMax(), 5);
    EXPECT_EQ(heap.delMax(), 5);
    EXPECT_EQ(heap.getMax(), 10);
}

TEST(LeftHeapTest, ConstructFromVector) {
    Vector<int> vec;
    vec.insert(4);
    vec.insert(12);
    vec.insert(7);
    LeftHeap<int> heap(vec);
    EXPECT_EQ(heap.getMax(), 12);
}
