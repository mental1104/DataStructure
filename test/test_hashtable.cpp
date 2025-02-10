#include <gtest/gtest.h>
#include "Hashtable.h"  // 假设 Hashtable 模板类及依赖已在此头文件中定义

TEST(HashtableTest, BasicPutGetRemove) {
    Hashtable<int, int> table(3);
    EXPECT_EQ(table.size(), 0);

    // 插入操作测试
    EXPECT_TRUE(table.put(10, 100));
    EXPECT_EQ(table.size(), 1);
    EXPECT_TRUE(table.put(20, 200));
    EXPECT_EQ(table.size(), 2);

    // 重复键插入应返回 false
    EXPECT_FALSE(table.put(10, 300));
    int* val = table.get(10);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 100);

    // 查询不存在的键应返回 nullptr
    EXPECT_EQ(table.get(30), nullptr);

    // 删除测试
    EXPECT_TRUE(table.remove(10));
    EXPECT_EQ(table.get(10), nullptr);
    EXPECT_EQ(table.size(), 1);
    EXPECT_FALSE(table.remove(10));
}

TEST(HashtableTest, RehashingTest) {
    Hashtable<int, int> table(3);
    int initialCapacity = table.capacity();

    // 插入足够数量的元素触发 rehash（条件：N*2 > M）
    for (int i = 0; i < initialCapacity; i++) {
        EXPECT_TRUE(table.put(i, i * 10));
    }

    int newCapacity = table.capacity();
    EXPECT_GE(newCapacity, initialCapacity);

    // 检查所有插入元素是否都能正确访问
    for (int i = 0; i < initialCapacity; i++) {
        int* val = table.get(i);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, i * 10);
    }
}

TEST(HashtableTest, MultipleRehashesTest) {
    Hashtable<int, int> table(3);
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

TEST(HashtableTest, RemovalAndReinsertionTest) {
    Hashtable<int, int> table(3);
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
