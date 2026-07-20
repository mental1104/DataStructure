#include "gtest/gtest.h"

#include <dsa/container/tree/AVL.h>
#include <dsa/container/tree/BST.h>
#include <dsa/container/tree/RedBlack.h>

#include <random>
#include <set>
#include <vector>

namespace {

template<typename Tree>
std::vector<int> values(const Tree& tree) {
    std::vector<int> result;
    for (typename Tree::iterator iterator = tree.begin(); iterator != tree.end(); ++iterator)
        result.push_back(*iterator);
    return result;
}

template<typename Tree>
void expectMatches(const Tree& tree, const std::set<int>& expected) {
    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.size(), expected.size());
    EXPECT_EQ(values(tree), std::vector<int>(expected.begin(), expected.end()));
}

template<typename Tree>
void runRandomizedDifferential() {
    Tree tree;
    std::set<int> expected;
    std::mt19937 generator(20260720u);
    std::uniform_int_distribution<int> key_distribution(0, 200);
    std::uniform_int_distribution<int> operation_distribution(0, 2);

    for (int step = 0; step < 2000; ++step) {
        const int key = key_distribution(generator);
        const int operation = operation_distribution(generator);
        if (operation == 0) {
            EXPECT_EQ(tree.insert(key).second, expected.insert(key).second);
        } else if (operation == 1) {
            EXPECT_EQ(tree.erase(key), expected.erase(key));
        } else {
            EXPECT_EQ(tree.contains(key), expected.count(key) != 0);
        }
        expectMatches(tree, expected);
    }
}

template<typename Tree>
void runValueSemantics() {
    Tree tree;
    for (int value : std::vector<int>{50, 20, 80, 10, 30, 70, 90})
        tree.insert(value);

    Tree copy(tree);
    EXPECT_TRUE(copy.validate());
    EXPECT_EQ(values(copy), values(tree));
    copy.erase(20);
    EXPECT_TRUE(tree.contains(20));

    Tree moved(std::move(copy));
    EXPECT_TRUE(copy.empty());
    EXPECT_TRUE(moved.validate());

    Tree assigned;
    assigned = tree;
    EXPECT_EQ(values(assigned), values(tree));

    Tree move_assigned;
    move_assigned = std::move(assigned);
    EXPECT_TRUE(assigned.empty());
    EXPECT_TRUE(move_assigned.validate());
}

} // namespace

TEST(ContainerBSTTest, RandomizedDifferential) {
    runRandomizedDifferential<dsa::container::BST<int> >();
}

TEST(ContainerAVLTest, RandomizedDifferential) {
    runRandomizedDifferential<dsa::container::AVL<int> >();
}

TEST(ContainerRedBlackTest, RandomizedDifferential) {
    runRandomizedDifferential<dsa::container::RedBlack<int> >();
}

TEST(ContainerSearchTreeTest, ValueSemanticsAndBounds) {
    runValueSemantics<dsa::container::BST<int> >();
    runValueSemantics<dsa::container::AVL<int> >();
    runValueSemantics<dsa::container::RedBlack<int> >();

    dsa::container::RedBlack<int> tree;
    for (int value : std::vector<int>{1, 3, 5, 7, 9})
        tree.insert(value);
    ASSERT_NE(tree.lower_bound(4), tree.end());
    EXPECT_EQ(*tree.lower_bound(4), 5);
    ASSERT_NE(tree.upper_bound(5), tree.end());
    EXPECT_EQ(*tree.upper_bound(5), 7);
    EXPECT_EQ(tree.range_aggregate(3, 7, 0, [](int result, int value) {
        return result + value;
    }), 15);
}
