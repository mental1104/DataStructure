#include "gtest/gtest.h"
#include "List.h"
#include "ListNode.h"

// 测试初始化与基本操作
TEST(ListTest, InitializationAndBasicOperations) {
    List<int> list;

    EXPECT_EQ(list.size(), 0);          // 检查初始化大小是否为0
    EXPECT_TRUE(list.empty());         // 检查列表是否为空

    list.insertAsFirst(10);
    EXPECT_EQ(list.size(), 1);         // 插入后大小是否为1
    EXPECT_FALSE(list.empty());        // 检查列表是否不为空
    EXPECT_EQ(list[0], 10);            // 检查插入的值是否正确
}

// 测试插入与删除操作
TEST(ListTest, InsertAndRemove) {
    List<int> list;

    list.insertAsFirst(10);
    list.insertAsLast(20);
    list.insertAsLast(30);

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list[0], 10);
    EXPECT_EQ(list[1], 20);
    EXPECT_EQ(list[2], 30);

    auto node = list.first()->succ;  // 指向第二个节点
    EXPECT_EQ(list.remove(node), 20); // 删除第二个节点
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[1], 30);
}

// 测试查找功能
TEST(ListTest, Find) {
    List<int> list;

    list.insertAsLast(10);
    list.insertAsLast(20);
    list.insertAsLast(30);

    ListNode<int>* node = list.find(20);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->data, 20);

    EXPECT_EQ(list.find(40), nullptr); // 查找不存在的值
}

// 测试唯一化功能
TEST(ListTest, Uniquify) {
    List<int> list;

    list.insertAsLast(10);
    list.insertAsLast(10);
    list.insertAsLast(20);
    list.insertAsLast(20);
    list.insertAsLast(30);

    EXPECT_EQ(list.uniquify(), 2); // 删除了两个重复元素
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list[0], 10);
    EXPECT_EQ(list[1], 20);
    EXPECT_EQ(list[2], 30);
}

// 测试反转功能
TEST(ListTest, Reverse) {
    List<int> list;

    list.insertAsLast(10);
    list.insertAsLast(20);
    list.insertAsLast(30);

    list.reverse();

    EXPECT_EQ(list[0], 30);
    EXPECT_EQ(list[1], 20);
    EXPECT_EQ(list[2], 10);
}

// 测试迭代器
TEST(ListTest, Iterator) {
    List<int> list;

    list.insertAsLast(10);
    list.insertAsLast(20);
    list.insertAsLast(30);

    int sum = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        sum += *it;
    }

    EXPECT_EQ(sum, 60); // 验证迭代器功能
}
