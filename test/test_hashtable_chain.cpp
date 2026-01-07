#include <gtest/gtest.h>
#include "HashtableChain.h"

TEST(HashtableChainTest, BasicPutGetRemove) {
    HashtableChain<int, int> table(3);
    EXPECT_EQ(table.size(), 0);

    EXPECT_TRUE(table.put(1, 10));
    EXPECT_TRUE(table.put(4, 40)); // collide with 1 when M == 3
    EXPECT_EQ(table.size(), 2);

    EXPECT_FALSE(table.put(1, 99));
    int* val = table.get(1);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 10);

    EXPECT_EQ(table.get(7), nullptr);

    EXPECT_TRUE(table.remove(1));
    EXPECT_EQ(table.get(1), nullptr);
    EXPECT_EQ(table.size(), 1);
    EXPECT_FALSE(table.remove(1));
}

TEST(HashtableChainTest, RehashPreservesEntries) {
    HashtableChain<int, int> table(3);
    int oldCap = table.capacity();
    int target = oldCap * 2 + 1;

    for (int i = 0; i < target; ++i) {
        EXPECT_TRUE(table.put(i, i + 100));
    }

    EXPECT_GT(table.capacity(), oldCap);
    EXPECT_EQ(table.size(), target);

    for (int i = 0; i < target; ++i) {
        int* val = table.get(i);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, i + 100);
    }
}
