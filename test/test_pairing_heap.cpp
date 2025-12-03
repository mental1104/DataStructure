#include <gtest/gtest.h>
#include <stdexcept>
#include "PairingHeap.h"

// 插入元素
TEST(PairingHeapTest, InsertTest) {
    PairingHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.size(), 3);
    EXPECT_EQ(heap.getMax(), 20);
}

// 删除最大值
TEST(PairingHeapTest, DeleteMaxTest) {
    PairingHeap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    EXPECT_EQ(heap.delMax(), 20);
    EXPECT_EQ(heap.size(), 2);
    EXPECT_EQ(heap.getMax(), 10);
}

// 合并两个配对堆
TEST(PairingHeapTest, MergeTest) {
    PairingHeap<int> heap1, heap2;
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
TEST(PairingHeapTest, EmptyHeapTest) {
    PairingHeap<int> heap;
    EXPECT_THROW(heap.getMax(), std::runtime_error);
    EXPECT_THROW(heap.delMax(), std::runtime_error);
}

// 小顶堆
TEST(PairingHeapTest, MinHeapTest) {
    PairingHeap<int, false> heap;
    heap.insert(10);
    heap.insert(2);
    heap.insert(8);
    EXPECT_EQ(heap.getMax(), 2);
    EXPECT_EQ(heap.delMax(), 2);
    EXPECT_EQ(heap.getMax(), 8);
}
