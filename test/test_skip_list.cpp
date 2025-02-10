#include <gtest/gtest.h>
#include "Skiplist.h"  // 假设 Skiplist 模板类及其所有依赖已在此头文件中定义

TEST(SkiplistTest, BasicPutGetRemove) {
    Skiplist<int, int> skiplist;
    // 初始状态下查询应返回空指针
    EXPECT_EQ(skiplist.get(10), nullptr);
    // 删除不存在的键应返回 false
    EXPECT_FALSE(skiplist.remove(10));

    // 插入操作
    EXPECT_TRUE(skiplist.put(10, 100));
    EXPECT_TRUE(skiplist.put(20, 200));
    EXPECT_TRUE(skiplist.put(30, 300));
    
    // 检查跳表大小（底层链表的元素数）应为 3
    EXPECT_EQ(skiplist.size(), 3);

    // 验证各键对应的值
    int* val = skiplist.get(10);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 100);

    val = skiplist.get(20);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 200);

    val = skiplist.get(30);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 300);

    // 删除一个键后再检查
    EXPECT_TRUE(skiplist.remove(20));
    EXPECT_EQ(skiplist.get(20), nullptr);
    EXPECT_EQ(skiplist.size(), 2);

    // 删除剩余键，最终跳表应为空
    EXPECT_TRUE(skiplist.remove(10));
    EXPECT_EQ(skiplist.get(10), nullptr);
    EXPECT_EQ(skiplist.size(), 1);

    EXPECT_TRUE(skiplist.remove(30));
    EXPECT_EQ(skiplist.get(30), nullptr);
    EXPECT_EQ(skiplist.size(), 0);
}

TEST(SkiplistTest, DuplicateInsertionTest) {
    Skiplist<int, int> skiplist;
    // 插入相同键两次
    EXPECT_TRUE(skiplist.put(50, 500));
    EXPECT_TRUE(skiplist.put(50, 550));

    // 根据实现，get 应返回第二次插入的值
    int* val = skiplist.get(50);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 550);

    // 底层链表可能保留重复项，故大小可能为 2
    EXPECT_EQ(skiplist.size(), 2);
}

TEST(SkiplistTest, RemoveNonexistentTest) {
    Skiplist<int, int> skiplist;
    // 删除不存在的键应返回 false
    EXPECT_FALSE(skiplist.remove(100));

    // 插入部分键，再尝试删除一个不存在的键
    EXPECT_TRUE(skiplist.put(1, 10));
    EXPECT_TRUE(skiplist.put(2, 20));
    EXPECT_FALSE(skiplist.remove(3));
    EXPECT_EQ(skiplist.size(), 2);
}

TEST(SkiplistTest, LevelTest) {
    Skiplist<int, int> skiplist;
    // 插入多个键观察跳表层高（随机行为可能导致层高不同，但至少应为 1）
    for (int i = 0; i < 100; i++) {
        EXPECT_TRUE(skiplist.put(i, i * 10));
    }
    EXPECT_GE(skiplist.level(), 1);
}
