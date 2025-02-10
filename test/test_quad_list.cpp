#include <gtest/gtest.h>
#include "Quadlist.h"  // 假设 Quadlist 与 QuadlistNode 的实现已在此头文件中

// 测试 QuadlistNode 的 insertAsSuccAbove 方法
TEST(QuadlistNodeTest, InsertAsSuccAbove) {
    // 手动构造两个节点并建立初始连接：node1 -> node2
    QuadlistNode<int> node1(10);
    QuadlistNode<int> node2(20);
    node1.succ = &node2;
    node2.pred = &node1;

    // 在 node1 后面插入一个新节点，设置 node2 作为上层节点 b
    QuadlistNode<int>* newNode = node1.insertAsSuccAbove(15, &node2);
    
    // 检查新节点中的数据及指针更新是否正确
    EXPECT_EQ(newNode->entry, 15);
    EXPECT_EQ(newNode->pred, &node1);
    EXPECT_EQ(newNode->succ, &node2);
    EXPECT_EQ(node1.succ, newNode);
    EXPECT_EQ(node2.pred, newNode);
    // 如果 b 被提供，则 b 的 above 指针应指向新节点
    EXPECT_EQ(node2.above, newNode);
}

// 测试 Quadlist 初始状态
TEST(QuadlistTest, InitialEmpty) {
    Quadlist<int> list;
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());
    // 空表时，first() 返回 header->succ，即 trailer，而 trailer 不是有效数据节点
    EXPECT_FALSE(list.valid(list.first()));
    EXPECT_FALSE(list.valid(list.last()));
}

// 测试 Quadlist 的插入与删除操作
TEST(QuadlistTest, InsertionAndRemoval) {
    Quadlist<int> list;
    
    // 利用空表时 trailer->pred 即 header，插入第一个节点
    QuadlistNode<int>* firstNode = list.insertAfterAbove(100, list.first()->pred);
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list.valid(firstNode));
    EXPECT_EQ(firstNode->entry, 100);
    
    // 在第一个节点后再插入一个节点
    QuadlistNode<int>* secondNode = list.insertAfterAbove(200, firstNode);
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list.valid(secondNode));
    EXPECT_EQ(secondNode->entry, 200);
    
    // 检查链表首尾节点
    EXPECT_EQ(list.first(), firstNode);
    EXPECT_EQ(list.last(), secondNode);
    
    // 删除第一个节点
    int removedValue = list.remove(firstNode);
    EXPECT_EQ(removedValue, 100);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.first(), secondNode);
    EXPECT_EQ(list.last(), secondNode);
    
    // 删除剩余节点后，链表应为空
    removedValue = list.remove(secondNode);
    EXPECT_EQ(removedValue, 200);
    EXPECT_EQ(list.size(), 0);
    EXPECT_FALSE(list.valid(list.first()));
    EXPECT_FALSE(list.valid(list.last()));
}

// 测试 Quadlist 的遍历功能
TEST(QuadlistTest, TraverseTest) {
    Quadlist<int> list;
    // 利用 trailer->pred 获取 header，从而插入第一个节点
    QuadlistNode<int>* node1 = list.insertAfterAbove(10, list.first()->pred);
    QuadlistNode<int>* node2 = list.insertAfterAbove(20, node1);
    QuadlistNode<int>* node3 = list.insertAfterAbove(30, node2);
    // 此时链表中顺序应为：10, 20, 30

    std::vector<int> values;
    // 利用模板版本的 traverse 遍历链表，将各节点的数值存入 values
    list.traverse([&values](int &val) {
        values.push_back(val);
    });
    
    std::vector<int> expected = {10, 20, 30};
    EXPECT_EQ(values, expected);
}
