#include <gtest/gtest.h>
#include <stdexcept>
#include "SkewHeap.h"

// 插入元素
TEST(SkewHeapTest, InsertTest) {
    SkewHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.size(), 3);
    EXPECT_EQ(heap.getMax(), 20);
}

// 删除最大值
TEST(SkewHeapTest, DeleteMaxTest) {
    SkewHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.delMax(), 20);
    EXPECT_EQ(heap.size(), 2);
    EXPECT_EQ(heap.getMax(), 10);
}

// 合并两个斜堆
TEST(SkewHeapTest, MergeTest) {
    SkewHeap<int> heap1, heap2;
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
TEST(SkewHeapTest, EmptyHeapTest) {
    SkewHeap<int> heap;
    EXPECT_THROW(heap.getMax(), std::runtime_error);
    EXPECT_THROW(heap.delMax(), std::runtime_error);
}

// 小顶堆
TEST(SkewHeapTest, MinHeapTest) {
    SkewHeap<int, false> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(3);
    EXPECT_EQ(heap.getMax(), 3);
    EXPECT_EQ(heap.delMax(), 3);
    EXPECT_EQ(heap.getMax(), 10);
}
