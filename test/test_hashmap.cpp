#include <gtest/gtest.h>
#include "HashMap.h"

TEST(HashMapTest, ListBucketOperations) {
    HashMap<int, int> map(5);
    int cap = map.capacity();
    int idx = 0;

    EXPECT_TRUE(map.put(0, 1));
    EXPECT_TRUE(map.put(cap, 2));
    EXPECT_TRUE(map.put(2 * cap, 3));
    EXPECT_FALSE(map.bucketIsTree(idx));
    EXPECT_EQ(map.bucketSize(idx), 3);

    int* val = map.get(cap);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 2);

    EXPECT_FALSE(map.put(cap, 99));
    EXPECT_TRUE(map.remove(cap));
    EXPECT_EQ(map.get(cap), nullptr);
    EXPECT_FALSE(map.remove(cap));
}

TEST(HashMapTest, TreeifyAndTreeOperations) {
    HashMap<int, int> map(17);
    int cap = map.capacity();
    int idx = 0;

    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(map.put(i * cap, i + 10));
    }
    EXPECT_TRUE(map.bucketIsTree(idx));
    EXPECT_EQ(map.bucketSize(idx), 8);

    int* val = map.get(3 * cap);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 13);

    EXPECT_FALSE(map.put(3 * cap, 99));
    EXPECT_TRUE(map.remove(3 * cap));
    EXPECT_EQ(map.get(3 * cap), nullptr);
    EXPECT_FALSE(map.remove(3 * cap));

    EXPECT_TRUE(map.put(3 * cap, 33));
    val = map.get(3 * cap);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 33);
}

TEST(HashMapTest, RehashMovesListAndTreeBuckets) {
    HashMap<int, int> map(11);
    int cap = map.capacity();

    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(map.put(i * cap, 100 + i));
    }
    EXPECT_TRUE(map.bucketIsTree(0));

    int extraNeeded = cap * 2 + 1 - 8;
    int inserted = 0;
    for (int i = 1; inserted < extraNeeded; ++i) {
        if (i % cap == 0) {
            continue;
        }
        EXPECT_TRUE(map.put(i, 200 + i));
        ++inserted;
    }

    EXPECT_GT(map.capacity(), cap);
    EXPECT_EQ(map.size(), cap * 2 + 1);

    for (int i = 0; i < 8; ++i) {
        int key = i * cap;
        int* val = map.get(key);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, 100 + i);
    }

    inserted = 0;
    for (int i = 1; inserted < extraNeeded; ++i) {
        if (i % cap == 0) {
            continue;
        }
        int* val = map.get(i);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, 200 + i);
        ++inserted;
    }
}
