// bplus_tree_test.cpp
#include "gtest/gtest.h"
#include "BPlusTree.h"
#include <vector>

TEST(BPlusTreeTest, InsertAndSearch) {
    BPlusTree<int, int> tree(3);
    std::vector<int> keys{10, 20, 5, 6, 12, 30, 7, 17};

    for (int k : keys) {
        EXPECT_TRUE(tree.insert(k, k * 10));
    }
    EXPECT_EQ(tree.size(), static_cast<int>(keys.size()));

    for (int k : keys) {
        const int* v = tree.search(k);
        ASSERT_NE(v, nullptr);
        EXPECT_EQ(*v, k * 10);
    }
    EXPECT_EQ(tree.search(100), nullptr);
}

TEST(BPlusTreeTest, RejectDuplicate) {
    BPlusTree<int, int> tree(4);
    EXPECT_TRUE(tree.insert(1, 11));
    EXPECT_FALSE(tree.insert(1, 22));
    EXPECT_EQ(tree.size(), 1);
    const int* v = tree.search(1);
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
        EXPECT_EQ(tree.search(k), nullptr);
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
    EXPECT_EQ(tree.search(1), nullptr);
}
