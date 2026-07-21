#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

#include <dsa/container/skiplist/QuadList.h>
#include <dsa/container/skiplist/SkipList.h>

TEST(IndustrialSkipListTest, OrderedMapOperationsAndRangeAggregation) {
    dsa::container::SkipList<int, int> list(std::less<int>(), std::allocator<std::pair<const int, int> >(), 16, 0.5, 7);
    for (int i = 99; i >= 0; --i)
        EXPECT_TRUE(list.insert_or_assign(i, i * 2).second);
    EXPECT_FALSE(list.insert_or_assign(10, 999).second);
    ASSERT_NE(list.get(10), nullptr);
    EXPECT_EQ(*list.get(10), 999);
    EXPECT_TRUE(list.validate());

    int previous = -1;
    for (auto it = list.begin(); it != list.end(); ++it) {
        EXPECT_GT(it->first, previous);
        previous = it->first;
    }
    EXPECT_EQ(list.range_aggregate(3, 5, 0, [](int sum, int value) { return sum + value; }), 24);
    EXPECT_EQ(list.erase(10), 1u);
    EXPECT_EQ(list.erase(10), 0u);
    EXPECT_TRUE(list.validate());
}

TEST(IndustrialSkipListTest, CopyAndMoveOwnNodesIndependently) {
    dsa::container::SkipList<int, std::string> source;
    source.insert_or_assign(1, "one");
    source.insert_or_assign(2, "two");
    dsa::container::SkipList<int, std::string> copy(source);
    copy.insert_or_assign(1, "changed");
    EXPECT_EQ(*source.get(1), "one");
    EXPECT_EQ(*copy.get(1), "changed");

    dsa::container::SkipList<int, std::string> moved(std::move(copy));
    EXPECT_EQ(moved.size(), 2u);
    EXPECT_TRUE(copy.empty());
    EXPECT_TRUE(copy.insert_or_assign(3, "three").second);
    EXPECT_EQ(*copy.get(3), "three");

    dsa::container::SkipList<int, std::string> assigned;
    assigned.insert_or_assign(9, "old");
    assigned = std::move(moved);
    EXPECT_EQ(assigned.size(), 2u);
    EXPECT_TRUE(moved.empty());
    EXPECT_TRUE(moved.insert_or_assign(4, "four").second);
}

TEST(IndustrialQuadListTest, LinksHorizontalAndVerticalNodes) {
    dsa::container::QuadList<int> lower;
    auto* lowerNode = lower.insert_after(nullptr, 10);
    auto* lowerSecond = lower.insert_after(lowerNode, 20);
    dsa::container::QuadList<int> upper;
    auto* upperNode = upper.insert_after(nullptr, 10, lowerNode);
    EXPECT_TRUE(lower.validate());
    EXPECT_TRUE(upper.validate());
    EXPECT_EQ(lowerNode->above, upperNode);
    EXPECT_EQ(upperNode->below, lowerNode);
    EXPECT_EQ(lower.erase(lowerSecond), 20);
    EXPECT_TRUE(lower.validate());
}


TEST(IndustrialSkipListTest, DifferentialOperationsMatchStdMap) {
    dsa::container::SkipList<int, int> actual(
        std::less<int>(), std::allocator<std::pair<const int, int> >(), 20, 0.5, 17
    );
    std::map<int, int> expected;
    for (int step = 0; step < 1500; ++step) {
        const int key = (step * 61) % 293;
        if (step % 6 == 0) {
            EXPECT_EQ(actual.erase(key), expected.erase(key));
        } else {
            actual.insert_or_assign(key, step);
            expected[key] = step;
        }
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_TRUE(actual.validate());
        for (int probe = 0; probe < 293; probe += 37) {
            auto found = expected.find(probe);
            const int* value = actual.get(probe);
            EXPECT_EQ(value != nullptr, found != expected.end());
            if (value) {
                EXPECT_EQ(*value, found->second);
            }
        }
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
