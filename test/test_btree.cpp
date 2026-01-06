#include <gtest/gtest.h>
#include <vector>
#include "BTree.h"

class BTreeHarness : public BTree<int> {
public:
    explicit BTreeHarness(int order) : BTree<int>(order) {}
    using BTree<int>::_root;
    using BTree<int>::solveUnderflow;
};

namespace {
BTNode<int>* makeNode(const std::vector<int>& keys, int childCount) {
    BTNode<int>* node = new BTNode<int>();
    if (node->child.size() > 0) {
        node->child.remove(0);
    }
    for (size_t i = 0; i < keys.size(); ++i) {
        node->key.insert(node->key.size(), keys[i]);
    }
    for (int i = 0; i < childCount; ++i) {
        node->child.insert(node->child.size(), static_cast<BTNode<int>*>(nullptr));
    }
    return node;
}

void attachChild(BTNode<int>* parent, int idx, BTNode<int>* child) {
    parent->child[idx] = child;
    if (child) child->parent = parent;
}
}

TEST(BTreeTest, InsertSearchRemoveRangeAggregate) {
    BTree<int> tree(3);
    EXPECT_TRUE(tree.empty());
    for (int i = 1; i <= 20; ++i) {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_FALSE(tree.insert(10));
    EXPECT_EQ(tree.size(), 20);

    EXPECT_NE(tree.search(7), nullptr);
    EXPECT_EQ(tree.search(100), nullptr);

    BTNode<int>* root = tree.root();
    ASSERT_NE(root, nullptr);
    ASSERT_TRUE(root->child.size() > 0);
    ASSERT_TRUE(root->child[0] != nullptr);

    int rootKey = root->key[0];
    EXPECT_TRUE(tree.remove(rootKey));
    EXPECT_EQ(tree.search(rootKey), nullptr);
    EXPECT_EQ(tree.size(), 19);

    int sum = tree.rangeAggregate(5, 12, 0, [](int acc, int val) { return acc + val; });
    int expected = 0;
    for (int i = 5; i <= 12; ++i) {
        if (i != rootKey) expected += i;
    }
    EXPECT_EQ(sum, expected);
}

TEST(BTreeTest, SolveUnderflowBorrowLeft) {
    BTreeHarness tree(5);
    release(tree._root);

    BTNode<int>* parent = makeNode(std::vector<int>{50}, 2);
    BTNode<int>* left = makeNode(std::vector<int>{10, 20, 30}, 4);
    BTNode<int>* node = makeNode(std::vector<int>{60}, 2);

    attachChild(parent, 0, left);
    attachChild(parent, 1, node);
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(parent->key[0], 30);
    EXPECT_EQ(left->key.size(), 2);
    EXPECT_EQ(node->key.size(), 2);
    EXPECT_EQ(node->key[0], 50);
}

TEST(BTreeTest, SolveUnderflowBorrowRight) {
    BTreeHarness tree(5);
    release(tree._root);

    BTNode<int>* parent = makeNode(std::vector<int>{50}, 2);
    BTNode<int>* node = makeNode(std::vector<int>{10}, 2);
    BTNode<int>* right = makeNode(std::vector<int>{60, 70, 80}, 4);

    attachChild(parent, 0, node);
    attachChild(parent, 1, right);
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(parent->key[0], 60);
    EXPECT_EQ(node->key.size(), 2);
    EXPECT_EQ(node->key[1], 50);
    EXPECT_EQ(right->key.size(), 2);
}

TEST(BTreeTest, SolveUnderflowMergeLeft) {
    BTreeHarness tree(4);
    release(tree._root);

    BTNode<int>* parent = makeNode(std::vector<int>{40, 80}, 3);
    BTNode<int>* left = makeNode(std::vector<int>{10}, 2);
    BTNode<int>* node = makeNode(std::vector<int>(), 1);
    BTNode<int>* right = makeNode(std::vector<int>{90}, 2);

    attachChild(parent, 0, left);
    attachChild(parent, 1, node);
    attachChild(parent, 2, right);
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(parent->key.size(), 1);
    EXPECT_EQ(parent->child.size(), 2);
    EXPECT_EQ(left->key.size(), 2);
    EXPECT_EQ(left->key[1], 40);
}

TEST(BTreeTest, SolveUnderflowMergeRightAndCollapseRoot) {
    BTreeHarness tree(4);
    release(tree._root);

    BTNode<int>* parent = makeNode(std::vector<int>{50}, 2);
    BTNode<int>* node = makeNode(std::vector<int>(), 1);
    BTNode<int>* right = makeNode(std::vector<int>{70}, 2);

    attachChild(parent, 0, node);
    attachChild(parent, 1, right);
    tree._root = parent;

    tree.solveUnderflow(node);

    EXPECT_EQ(tree.root(), right);
    EXPECT_EQ(right->parent, nullptr);
    EXPECT_EQ(right->key.size(), 2);
    EXPECT_EQ(right->key[0], 50);
}
