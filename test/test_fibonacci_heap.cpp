#include <gtest/gtest.h>
#include <stdexcept>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#include "FibonacciHeap.h"
#undef private
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class FibonacciHeapHarness : public FibonacciHeap<int> {
public:
    using BinTree<int>::_root;
    using BinTree<int>::_size;
};

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

// 小顶堆
TEST(FibonacciHeapTest, MinHeapTest) {
    FibonacciHeap<int, false> heap;
    heap.insert(7);
    heap.insert(3);
    heap.insert(9);
    EXPECT_EQ(heap.getMax(), 3);
    EXPECT_EQ(heap.delMax(), 3);
    EXPECT_EQ(heap.getMax(), 7);
}

TEST(FibonacciHeapTest, MergeIntoEmptyHeap) {
    FibonacciHeap<int> left;
    FibonacciHeap<int> right;
    right.insert(5);
    right.insert(8);

    left.merge(right);
    EXPECT_EQ(left.getMax(), 8);
    EXPECT_EQ(left.size(), 2);
    EXPECT_TRUE(right.empty());
}

TEST(FibonacciHeapTest, MergeSetsBestRootWhenMissing) {
    FibonacciHeap<int> left;
    left.insertAsRoot(1);
    FibonacciHeap<int> right;
    right.insert(5);

    left.merge(right);
    EXPECT_EQ(left.getMax(), 5);
}

TEST(FibonacciHeapTest, DeleteMaxPromotesChildren) {
    FibonacciHeapHarness heap;
    BinNode<int>* root = new BinNode<int>(10, nullptr);
    BinNode<int>* child = new BinNode<int>(5, root);
    root->lc = child;
    heap._root = root;
    heap._bestRoot = root;
    heap._size = 2;

    EXPECT_EQ(heap.delMax(), 10);
    EXPECT_EQ(heap.size(), 1);
    EXPECT_EQ(heap.getMax(), 5);
}
