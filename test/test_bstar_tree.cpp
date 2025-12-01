// bstar_tree_test.cpp
#include "gtest/gtest.h"
#include "BStarTree.h"

static bool node_has_key(BTNode<int>* node, int key) {
    if (!node) return false;
    Rank r = node->key.search(key);
    return r >= 0 && r < node->key.size() && node->key[r] == key;
}

TEST(BStarTreeTest, InsertAndSearch) {
    BStarTree<int> tree(5);
    std::vector<int> keys;
    for (int i = 0; i < 200; ++i) keys.push_back(i * 3 + 1);

    for (int k : keys) {
        EXPECT_TRUE(tree.insert(k));
    }
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    for (int k : keys) {
        EXPECT_TRUE(node_has_key(tree.search(k), k));
    }
    EXPECT_FALSE(node_has_key(tree.search(99999), 99999));
}

TEST(BStarTreeTest, DuplicateRejected) {
    BStarTree<int> tree(6);
    EXPECT_TRUE(tree.insert(10));
    EXPECT_FALSE(tree.insert(10));
    EXPECT_EQ(tree.size(), 1);
}

TEST(BStarTreeTest, RemoveAll) {
    BStarTree<int> tree(5);
    std::vector<int> keys{5, 1, 9, 3, 7, 2, 6, 8, 4};
    for (int k : keys) tree.insert(k);
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    for (int k : keys) {
        EXPECT_TRUE(tree.remove(k));
        EXPECT_FALSE(node_has_key(tree.search(k), k));
    }
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_FALSE(tree.remove(42));
}
