// bplus_tree_test.cpp
#include "gtest/gtest.h"
#include <functional>
#include <utility>
#include <vector>
#include "BST.h"
#include "Vector.h"
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#include "BPlusTree.h"
#undef private
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace {
template <typename T>
void pushVector(Vector<T>& vec, const T& value) {
    vec.insert(vec.size(), value);
}

BPlusTree<int, int>::Node* makeLeaf(const std::vector<std::pair<int, int>>& items) {
    BPlusTree<int, int>::Node* node = new BPlusTree<int, int>::Node(true);
    for (size_t i = 0; i < items.size(); ++i) {
        pushVector(node->key, items[i].first);
        pushVector(node->value, items[i].second);
    }
    return node;
}

BPlusTree<int, int>::Node* makeInternal(const std::vector<int>& keys, int childCount) {
    BPlusTree<int, int>::Node* node = new BPlusTree<int, int>::Node(false);
    for (size_t i = 0; i < keys.size(); ++i) {
        pushVector(node->key, keys[i]);
    }
    for (int i = 0; i < childCount; ++i) {
        pushVector(node->child, static_cast<BPlusTree<int, int>::Node*>(nullptr));
    }
    return node;
}

void appendChild(BPlusTree<int, int>::Node* parent, BPlusTree<int, int>::Node* child) {
    pushVector(parent->child, child);
    if (child) child->parent = parent;
}
}

TEST(BPlusTreeTest, InsertAndSearch) {
    BPlusTree<int, int> tree(3);
    std::vector<int> keys{10, 20, 5, 6, 12, 30, 7, 17};

    for (int k : keys) {
        EXPECT_TRUE(tree.insert(k, k * 10));
    }
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    const BPlusTree<int, int>& ctree = tree;
    for (int k : keys) {
        const int* v = ctree.search(k);
        ASSERT_NE(v, nullptr);
        EXPECT_EQ(*v, k * 10);
    }
    EXPECT_EQ(ctree.search(100), nullptr);
}

TEST(BPlusTreeTest, RejectDuplicate) {
    BPlusTree<int, int> tree(4);
    EXPECT_TRUE(tree.insert(1, 11));
    EXPECT_FALSE(tree.insert(1, 22));
    EXPECT_EQ(tree.size(), 1);
    const BPlusTree<int, int>& ctree = tree;
    const int* v = ctree.search(1);
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(*v, 11);
}

TEST(BPlusTreeTest, RangeQueryReturnsSortedValues) {
    BPlusTree<int, int> tree(4);
    std::vector<int> keys;
    for (int i = 1; i <= 15; i++) {
        keys.push_back(i);
    }
    for (int k : keys) {
        EXPECT_TRUE(tree.insert(k, k));
    }

    Vector<int> range = tree.rangeQuery(4, 11);
    ASSERT_EQ(range.size(), 8);
    for (int i = 0; i < range.size(); i++) {
        EXPECT_EQ(range[i], i + 4);
    }
}

TEST(BPlusTreeTest, RemoveAndCollapse) {
    BPlusTree<int, int> tree(3);
    for (int i = 1; i <= 10; i++) {
        EXPECT_TRUE(tree.insert(i, i));
    }

    std::vector<int> toRemove{2, 3, 4, 5, 6};
    for (int k : toRemove) {
        EXPECT_TRUE(tree.remove(k));
    }
    EXPECT_FALSE(tree.remove(100)); // not exist
    EXPECT_EQ(tree.size(), 5);

    for (int k : toRemove) {
        const BPlusTree<int, int>& ctree = tree;
        EXPECT_EQ(ctree.search(k), nullptr);
    }
    Vector<int> remaining = tree.rangeQuery(1, 10);
    ASSERT_EQ(remaining.size(), 5);
    for (int i = 0; i < remaining.size(); i++) {
        int expected = (i < 1) ? 1 : (i + 6); // keys left: 1,7,8,9,10
        EXPECT_EQ(remaining[i], expected);
    }

    for (int k : std::vector<int>{1, 7, 8, 9, 10}) {
        EXPECT_TRUE(tree.remove(k));
    }
    EXPECT_TRUE(tree.empty());
    const BPlusTree<int, int>& ctree = tree;
    EXPECT_EQ(ctree.search(1), nullptr);
}

TEST(BPlusTreeTest, ChildIndexNotFound) {
    BPlusTree<int, int> tree(3);
    BPlusTree<int, int>::Node* parent = makeInternal(std::vector<int>{10, 20}, 3);
    BPlusTree<int, int>::Node* child = new BPlusTree<int, int>::Node(true);

    EXPECT_EQ(tree.childIndex(parent, child), -1);

    delete child;
    delete parent;
}

TEST(BPlusTreeTest, BorrowFromLeftLeafOnUnderflow) {
    BPlusTree<int, int> tree(4);
    BPlusTree<int, int>::Node* parent = makeInternal(std::vector<int>{4}, 0);
    BPlusTree<int, int>::Node* left = makeLeaf({{1, 10}, {2, 20}, {3, 30}});
    BPlusTree<int, int>::Node* right = makeLeaf({{4, 40}});

    appendChild(parent, left);
    appendChild(parent, right);
    tree._root = parent;

    tree.handleUnderflow(right);

    EXPECT_EQ(left->key.size(), 2);
    EXPECT_EQ(right->key.size(), 2);
    EXPECT_EQ(right->key[0], 3);
    EXPECT_EQ(parent->key[0], 3);
}

TEST(BPlusTreeTest, BorrowFromLeftInternalOnUnderflow) {
    BPlusTree<int, int> tree(4);
    BPlusTree<int, int>::Node* parent = makeInternal(std::vector<int>{50}, 0);
    BPlusTree<int, int>::Node* left = makeInternal(std::vector<int>{10, 20}, 3);
    BPlusTree<int, int>::Node* node = makeInternal(std::vector<int>(), 1);

    appendChild(parent, left);
    appendChild(parent, node);
    tree._root = parent;

    tree.handleUnderflow(node);

    EXPECT_EQ(node->key.size(), 1);
    EXPECT_EQ(node->key[0], 50);
    EXPECT_EQ(parent->key[0], 20);
    EXPECT_EQ(left->key.size(), 1);
}
