#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <dsa/algorithm/Sort.h>
#include <dsa/container/segment/SegmentTree.h>
#include <dsa/container/tree/Splay.h>

namespace {
struct MoveOnly {
    explicit MoveOnly(int input) : value(input) {}
    MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = -1; }
    MoveOnly& operator=(MoveOnly&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    int value;
};

struct MoveOnlyLess {
    bool operator()(const MoveOnly& left, const MoveOnly& right) const {
        return left.value < right.value;
    }
};

struct Concat {
    std::string operator()(const std::string& left, const std::string& right) const {
        return left + right;
    }
};
}

TEST(IndustrialSplayTreeTest, SplaysHotNodeAndMaintainsOrdering) {
    dsa::container::SplayTree<int> tree;
    for (int value : std::vector<int>{50, 20, 70, 10, 30, 60, 80})
        EXPECT_TRUE(tree.insert(value).second);
    EXPECT_TRUE(tree.validate());
    EXPECT_NE(tree.find(30), tree.end());
    ASSERT_NE(tree.root_value(), nullptr);
    EXPECT_EQ(*tree.root_value(), 30);
    EXPECT_EQ(tree.range_aggregate(20, 60, 0, [](int sum, int value) { return sum + value; }), 160);
    EXPECT_EQ(tree.erase(50), 1u);
    EXPECT_TRUE(tree.validate());
}

TEST(IndustrialSplayTreeTest, SupportsMoveOnlyValues) {
    dsa::container::SplayTree<MoveOnly, MoveOnlyLess> tree;
    tree.insert(MoveOnly(3));
    tree.insert(MoveOnly(1));
    tree.insert(MoveOnly(2));
    EXPECT_TRUE(tree.validate());
    EXPECT_EQ(tree.find(MoveOnly(2))->value, 2);
}

TEST(IndustrialSegmentTreeTest, PreservesNonCommutativeQueryOrder) {
    const std::vector<std::string> values{"a", "b", "c", "d"};
    dsa::container::SegmentTree<std::string, Concat> tree(
        values.begin(), values.end(), Concat(), std::string()
    );
    EXPECT_EQ(tree.query(1, 4), "bcd");
    tree.update(2, "X");
    EXPECT_EQ(tree.query(0, 4), "abXd");
    EXPECT_THROW(tree.query(4, 5), std::out_of_range);
}

TEST(IteratorSortAlgorithmTest, SortsStandardAndMoveOnlyRanges) {
    std::vector<int> values{5, 1, 4, 2, 3};
    EXPECT_TRUE(dsa::algorithm::sort(
        values.begin(), values.end(), dsa::algorithm::SortStrategy::Quick3way
    ));
    EXPECT_TRUE(std::is_sorted(values.begin(), values.end()));

    std::vector<MoveOnly> moveOnly;
    moveOnly.emplace_back(4);
    moveOnly.emplace_back(1);
    moveOnly.emplace_back(3);
    moveOnly.emplace_back(2);
    EXPECT_TRUE(dsa::algorithm::sort(
        moveOnly.begin(), moveOnly.end(),
        dsa::algorithm::SortStrategy::QuickSort, MoveOnlyLess()
    ));
    for (std::size_t i = 1; i < moveOnly.size(); ++i)
        EXPECT_LT(moveOnly[i - 1].value, moveOnly[i].value);
}

TEST(IndustrialSplayTreeTest, MovedFromTreeRemainsReusable) {
    dsa::container::SplayTree<int> source;
    source.insert(2);
    source.insert(1);
    source.insert(3);

    dsa::container::SplayTree<int> moved(std::move(source));
    EXPECT_EQ(moved.size(), 3u);
    EXPECT_TRUE(source.empty());
    EXPECT_TRUE(source.insert(4).second);
    EXPECT_TRUE(source.validate());

    dsa::container::SplayTree<int> assigned;
    assigned.insert(9);
    assigned = std::move(moved);
    EXPECT_EQ(assigned.size(), 3u);
    EXPECT_TRUE(moved.empty());
    EXPECT_TRUE(moved.insert(5).second);
    EXPECT_TRUE(moved.validate());
}


TEST(IndustrialSplayTreeTest, DifferentialOperationsMatchStdSet) {
    dsa::container::SplayTree<int> actual;
    std::set<int> expected;
    for (int step = 0; step < 1500; ++step) {
        const int value = (step * 73) % 257;
        if (step % 4 == 0) {
            EXPECT_EQ(actual.erase(value), expected.erase(value));
        } else {
            EXPECT_EQ(actual.insert(value).second, expected.insert(value).second);
        }
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_TRUE(actual.validate());
        for (int probe = 0; probe < 257; probe += 31) {
            EXPECT_EQ(actual.contains(probe), expected.count(probe) != 0);
        }
    }
    std::vector<int> values;
    for (auto it = actual.begin(); it != actual.end(); ++it)
        values.push_back(*it);
    EXPECT_TRUE(std::equal(values.begin(), values.end(), expected.begin(), expected.end()));
}
