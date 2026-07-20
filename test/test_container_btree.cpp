#include "gtest/gtest.h"

#include <dsa/container/tree/BTree.h>

#include <random>
#include <set>
#include <vector>

namespace {

std::vector<int> values(const dsa::container::BTree<int>& tree) {
    std::vector<int> result;
    for (dsa::container::BTree<int>::iterator iterator = tree.begin();
         iterator != tree.end(); ++iterator)
        result.push_back(*iterator);
    return result;
}

void expectMatches(const dsa::container::BTree<int>& tree, const std::set<int>& expected) {
    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.size(), expected.size());
    EXPECT_EQ(values(tree), std::vector<int>(expected.begin(), expected.end()));
}

} // namespace

TEST(ContainerBTreeTest, RandomizedDifferentialAcrossOrders) {
    for (std::size_t order = 3; order <= 8; ++order) {
        dsa::container::BTree<int> tree(order);
        std::set<int> expected;
        std::mt19937 generator(static_cast<unsigned>(order * 20260720u));
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
}

TEST(ContainerBTreeTest, ValueSemanticsBoundsAndRange) {
    dsa::container::BTree<int> tree(5);
    for (int value : std::vector<int>{50, 20, 80, 10, 30, 70, 90, 25, 35})
        tree.insert(value);

    dsa::container::BTree<int> copy(tree);
    EXPECT_EQ(values(copy), values(tree));
    copy.erase(20);
    EXPECT_TRUE(tree.contains(20));

    dsa::container::BTree<int> moved(std::move(copy));
    EXPECT_TRUE(copy.empty());
    EXPECT_TRUE(moved.validate());

    ASSERT_NE(tree.lower_bound(26), tree.end());
    EXPECT_EQ(*tree.lower_bound(26), 30);
    ASSERT_NE(tree.upper_bound(70), tree.end());
    EXPECT_EQ(*tree.upper_bound(70), 80);
    EXPECT_EQ(tree.range_aggregate(25, 70, 0, [](int result, int value) {
        return result + value;
    }), 210);
}

TEST(ContainerBTreeTest, RejectsInvalidOrder) {
    EXPECT_THROW(dsa::container::BTree<int>(2), std::invalid_argument);
}
