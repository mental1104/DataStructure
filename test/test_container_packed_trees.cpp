#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

#include <dsa/container/tree/BPlusTree.h>
#include <dsa/container/tree/BStarTree.h>

TEST(IndustrialBPlusTreeTest, MaintainsLeafOrderAndMapping) {
    dsa::container::BPlusTree<int, std::string> tree(5);
    for (int i = 199; i >= 0; --i)
        EXPECT_TRUE(tree.insert(i, std::to_string(i)));
    EXPECT_FALSE(tree.insert(10, "duplicate"));
    ASSERT_NE(tree.get(10), nullptr);
    EXPECT_EQ(*tree.get(10), "10");
    EXPECT_TRUE(tree.validate());

    int expected = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it, ++expected) {
        EXPECT_EQ(it->first, expected);
        EXPECT_EQ(it->second, std::to_string(expected));
    }
    EXPECT_EQ(expected, 200);
    EXPECT_EQ(tree.range_aggregate(5, 9, 0, [](int count, const std::string&) { return count + 1; }), 5);

    for (int i = 0; i < 200; i += 2)
        EXPECT_EQ(tree.erase(i), 1u);
    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.size(), 100u);
}

TEST(IndustrialBPlusTreeTest, CopyAndMoveOwnValues) {
    dsa::container::BPlusTree<int, std::string> source;
    source.insert_or_assign(1, "one");
    source.insert_or_assign(2, "two");
    dsa::container::BPlusTree<int, std::string> copy(source);
    copy.insert_or_assign(1, "changed");
    EXPECT_EQ(source.at(1), "one");
    EXPECT_EQ(copy.at(1), "changed");

    dsa::container::BPlusTree<int, std::string> moved(std::move(copy));
    EXPECT_EQ(moved.size(), 2u);
    EXPECT_TRUE(copy.empty());
    EXPECT_TRUE(copy.insert(3, "three"));
    EXPECT_EQ(copy.at(3), "three");
}

TEST(IndustrialBStarTreeTest, UsesTwoThirdsPackedMultiwayStructure) {
    dsa::container::BStarTree<int> tree(6);
    for (int i = 0; i < 300; ++i)
        EXPECT_TRUE(tree.insert(i));
    EXPECT_TRUE(tree.validate());
    EXPECT_TRUE(tree.contains(150));
    EXPECT_EQ(tree.range_aggregate(10, 19, 0, [](int sum, int value) { return sum + value; }), 145);
    for (int i = 0; i < 300; i += 3)
        EXPECT_TRUE(tree.remove(i));
    EXPECT_TRUE(tree.validate());
    EXPECT_FALSE(tree.contains(150));
}


TEST(IndustrialBPlusTreeTest, DifferentialOperationsMatchStdMap) {
    dsa::container::BPlusTree<int, int> actual(7);
    std::map<int, int> expected;
    for (int step = 0; step < 1200; ++step) {
        const int key = (step * 89) % 317;
        if (step % 7 == 0) {
            EXPECT_EQ(actual.erase(key), expected.erase(key));
        } else {
            actual.insert_or_assign(key, step);
            expected[key] = step;
        }
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_TRUE(actual.validate());
    }
    auto actualIt = actual.begin();
    auto expectedIt = expected.begin();
    while (actualIt != actual.end() && expectedIt != expected.end()) {
        EXPECT_EQ(actualIt->first, expectedIt->first);
        EXPECT_EQ(actualIt->second, expectedIt->second);
        ++actualIt;
        ++expectedIt;
    }
    EXPECT_EQ(actualIt, actual.end());
    EXPECT_EQ(expectedIt, expected.end());
}
