#include <gtest/gtest.h>
#include "HashtableB.h"  // 假设 QuadraticHT 模板类及其依赖已在此头文件中定义

TEST(QuadraticHTTest, BasicPutGetRemove) {
    QuadraticHT<int, int> table(5);
    EXPECT_EQ(table.size(), 0);

    EXPECT_TRUE(table.put(1, 100));
    EXPECT_EQ(table.size(), 1);
    EXPECT_TRUE(table.put(2, 200));
    EXPECT_EQ(table.size(), 2);
    // 重复键插入应返回 false
    EXPECT_FALSE(table.put(1, 300));
    int* val = table.get(1);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 100);

    // 查询不存在的键应返回 nullptr
    EXPECT_EQ(table.get(3), nullptr);

    // 删除测试
    EXPECT_TRUE(table.remove(1));
    EXPECT_EQ(table.get(1), nullptr);
    EXPECT_EQ(table.size(), 1);
    EXPECT_FALSE(table.remove(1));
}

TEST(QuadraticHTTest, RehashingTest) {
    QuadraticHT<int, int> table(5);
    int initialCapacity = table._M();

    // 插入足够数量的元素触发 rehash (条件： N >= M)
    for (int i = 0; i < initialCapacity; i++) {
        EXPECT_TRUE(table.put(i, i * 10));
    }

    int newCapacity = table._M();
    EXPECT_GE(newCapacity, initialCapacity);

    // 检查所有插入元素是否能正确访问
    for (int i = 0; i < initialCapacity; i++) {
        int* val = table.get(i);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, i * 10);
    }
}

TEST(QuadraticHTTest, MultipleRehashesTest) {
    QuadraticHT<int, int> table(5);
    const int numEntries = 50;  // 足够的元素以触发多次 rehash
    for (int i = 0; i < numEntries; i++) {
        EXPECT_TRUE(table.put(i, i + 100));
    }
    EXPECT_EQ(table.size(), numEntries);
    for (int i = 0; i < numEntries; i++) {
        int* val = table.get(i);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, i + 100);
    }
}

TEST(QuadraticHTTest, RemovalAndReinsertionTest) {
    QuadraticHT<int, int> table(5);
    // 插入一些数据
    for (int i = 0; i < 10; i++) {
        EXPECT_TRUE(table.put(i, i * 10));
    }
    // 删除部分键
    EXPECT_TRUE(table.remove(3));
    EXPECT_TRUE(table.remove(7));
    EXPECT_EQ(table.get(3), nullptr);
    EXPECT_EQ(table.get(7), nullptr);

    // 重新插入被删除的键
    EXPECT_TRUE(table.put(3, 333));
    EXPECT_TRUE(table.put(7, 777));

    int* val = table.get(3);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 333);

    val = table.get(7);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 777);
}

TEST(QuadraticHTTest, IntrospectionAndQuadraticProbe) {
    QuadraticHT<int, int> table(5);
    int cap = table._M();
    EXPECT_EQ(table._N(), 0);
    EXPECT_NE(table._lazyRemoval(), nullptr);
    EXPECT_EQ(table._ht(0), nullptr);

    EXPECT_TRUE(table.put(0, 1));
    EXPECT_TRUE(table.put(cap, 2));
    EXPECT_TRUE(table.put(2 * cap, 3));
    EXPECT_EQ(table.size(), 3);
    EXPECT_EQ(*table.get(cap), 2);
}
