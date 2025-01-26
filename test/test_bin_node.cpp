/*
 * @Date: 2025-01-26 16:35:34
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-26 16:35:37
 */
#include <gtest/gtest.h>
#include "BinTree.h" // 假设 BinNode 定义在该头文件中

// 测试 BinNode 的 insertAsLC
TEST(BinNodeTest, InsertAsLC) {
    BinNode<int> node(10); // 根节点
    node.insertAsLC(5);    // 插入左孩子

    // 验证左孩子是否存在且值正确
    ASSERT_NE(node.lc, nullptr);
    EXPECT_EQ(node.lc->data, 5);

    // 验证左孩子的父节点是否正确
    EXPECT_EQ(node.lc->parent, &node);
}

// 测试 BinNode 的 insertAsRC
TEST(BinNodeTest, InsertAsRC) {
    BinNode<int> node(20); // 根节点
    node.insertAsRC(30);   // 插入右孩子

    // 验证右孩子是否存在且值正确
    ASSERT_NE(node.rc, nullptr);
    EXPECT_EQ(node.rc->data, 30);

    // 验证右孩子的父节点是否正确
    EXPECT_EQ(node.rc->parent, &node);
}

// 测试 BinNode 同时插入左右孩子
TEST(BinNodeTest, InsertBothChildren) {
    BinNode<int> node(40); // 根节点

    // 插入左孩子和右孩子
    node.insertAsLC(15);
    node.insertAsRC(25);

    // 验证左孩子和右孩子是否正确
    ASSERT_NE(node.lc, nullptr);
    ASSERT_NE(node.rc, nullptr);

    EXPECT_EQ(node.lc->data, 15);
    EXPECT_EQ(node.rc->data, 25);

    // 验证父节点是否正确
    EXPECT_EQ(node.lc->parent, &node);
    EXPECT_EQ(node.rc->parent, &node);
}