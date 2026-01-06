// bstar_tree_test.cpp
#include "gtest/gtest.h"
#include "BStarTree.h"
#include <vector>

static bool node_has_key(BTNode<int>* node, int key) {
    if (!node) return false;
    Rank r = node->key.search(key);
    return r >= 0 && r < node->key.size() && node->key[r] == key;
}

class BStarTreeHarness : public BStarTree<int> {
public:
    explicit BStarTreeHarness(int order)
        : BStarTree<int>(order) {}

    using BStarTree<int>::_order;
    using BStarTree<int>::_root;
    using BStarTree<int>::solveUnderflow;
};

namespace {
BTNode<int>* makeNodeWithKeys(const std::vector<int>& keys, int childCount) {
    BTNode<int>* node = new BTNode<int>();
    node->child.remove(0);
    for (size_t i = 0; i < keys.size(); ++i) {
        node->key.insert(node->key.size(), keys[i]);
    }
    for (int i = 0; i < childCount; ++i) {
        node->child.insert(node->child.size(), static_cast<BTNode<int>*>(nullptr));
    }
    return node;
}
}

TEST(BStarTreeTest, InsertAndSearch) {
    BStarTree<int> tree(5);
    std::vector<int> keys;
    for (int i = 0; i < 200; ++i) keys.push_back(i * 3 + 1);

    for (int k : keys) {
        EXPECT_TRUE(tree.insert(k, true));
    }
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    for (int k : keys) {
        EXPECT_TRUE(node_has_key(tree.search(k, 0), k));
    }
    EXPECT_FALSE(node_has_key(tree.search(99999, 0), 99999));
}

TEST(BStarTreeTest, DuplicateRejected) {
    BStarTree<int> tree(6);
    EXPECT_TRUE(tree.insert(10, true));
    EXPECT_FALSE(tree.insert(10, true));
    EXPECT_EQ(tree.size(), 1);
}

TEST(BStarTreeTest, RemoveAll) {
    BStarTree<int> tree(5);
    std::vector<int> keys{5, 1, 9, 3, 7, 2, 6, 8, 4};
    for (int k : keys) tree.insert(k, true);
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    for (int k : keys) {
        EXPECT_TRUE(tree.remove(k));
        EXPECT_FALSE(node_has_key(tree.search(k, 0), k));
    }
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_FALSE(tree.remove(42));
}

TEST(BStarTreeTest, BorrowFromLeftSiblingOnUnderflow) {
    BStarTreeHarness tree(5);
    BTNode<int>* parent = makeNodeWithKeys(std::vector<int>{50}, 2);
    BTNode<int>* left = makeNodeWithKeys(std::vector<int>{10, 20, 30}, 4);
    BTNode<int>* node = makeNodeWithKeys(std::vector<int>{5}, 2);

    parent->child[0] = left;
    parent->child[1] = node;
    left->parent = parent;
    node->parent = parent;
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(node->key[0], 50);
    EXPECT_EQ(parent->key[0], 30);
    EXPECT_EQ(left->key.size(), 2);
}

TEST(BStarTreeTest, BorrowFromRightSiblingOnUnderflow) {
    BStarTreeHarness tree(5);
    BTNode<int>* parent = makeNodeWithKeys(std::vector<int>{55}, 2);
    BTNode<int>* node = makeNodeWithKeys(std::vector<int>{5}, 2);
    BTNode<int>* right = makeNodeWithKeys(std::vector<int>{60, 70, 80}, 4);

    parent->child[0] = node;
    parent->child[1] = right;
    node->parent = parent;
    right->parent = parent;
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(node->key.size(), 2);
    EXPECT_EQ(node->key[1], 55);
    EXPECT_EQ(parent->key[0], 60);
    EXPECT_EQ(right->key.size(), 2);
}

TEST(BStarTreeTest, MergeWithRightSiblingOnUnderflow) {
    BStarTreeHarness tree(5);
    BTNode<int>* parent = makeNodeWithKeys(std::vector<int>{50}, 2);
    BTNode<int>* node = makeNodeWithKeys(std::vector<int>{5}, 2);
    BTNode<int>* right = makeNodeWithKeys(std::vector<int>{70, 80}, 3);

    parent->child[0] = node;
    parent->child[1] = right;
    node->parent = parent;
    right->parent = parent;
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(tree.root(), right);
    EXPECT_EQ(right->parent, nullptr);
}
