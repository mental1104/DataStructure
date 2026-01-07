/*
 * @Date: 2025-01-26 16:35:34
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-26 16:35:37
 */
#include <gtest/gtest.h>
#include <vector>
#include "BinTree.h" // 假设 BinNode 定义在该头文件中

namespace {
struct IntCollector {
    std::vector<int>* out;
    void operator()(int& v) { out->push_back(v); }
};
}

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

TEST(BinNodeTest, ReplaceChildrenSuccAndTraversals) {
    BinTree<int> tree;
    tree.insertAsRoot(10);
    BinNode<int>* root = tree.root();

    root->insertAsLC(5);
    root->insertAsLC(3); // replace left child
    root->insertAsRC(15);
    root->insertAsRC(20); // replace right child
    root->lc->insertAsLC(1);
    root->rc->insertAsLC(12);

    ASSERT_NE(root->lc, nullptr);
    ASSERT_NE(root->rc, nullptr);
    EXPECT_EQ(root->lc->data, 3);
    EXPECT_EQ(root->rc->data, 20);

    ASSERT_NE(root->succ(), nullptr);
    EXPECT_EQ(root->succ()->data, 12);
    EXPECT_EQ(root->rc->succ(), nullptr);
    ASSERT_NE(root->lc->succ(), nullptr);
    EXPECT_EQ(root->lc->succ()->data, 10);

    std::vector<int> pre;
    IntCollector preVis{&pre};
    root->travPre(preVis);
    EXPECT_EQ(pre, std::vector<int>({10, 3, 1, 20, 12}));

    std::vector<int> in;
    IntCollector inVis{&in};
    root->travIn(inVis);
    EXPECT_EQ(in, std::vector<int>({1, 3, 10, 12, 20}));

    std::vector<int> post;
    IntCollector postVis{&post};
    root->travPost(postVis);
    EXPECT_EQ(post, std::vector<int>({1, 3, 12, 20, 10}));

    std::vector<int> level;
    IntCollector levelVis{&level};
    root->travLevel(levelVis);
    EXPECT_EQ(level, std::vector<int>({10, 3, 20, 1, 12}));
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

TEST(BinNodeTest, TallerChildTieBreaksBySide) {
    BinNode<int> parent(10);
    BinNode<int> left(5, &parent);
    BinNode<int> right(15, &parent);
    parent.lc = &left;
    parent.rc = &right;

    BinNode<int> left_l(2, &left);
    BinNode<int> left_r(7, &left);
    left.lc = &left_l;
    left.rc = &left_r;
    left_l.height = 0;
    left_r.height = 0;
    left.height = 1;

    BinNode<int> right_l(12, &right);
    BinNode<int> right_r(18, &right);
    right.lc = &right_l;
    right.rc = &right_r;
    right_l.height = 0;
    right_r.height = 0;
    right.height = 1;

    EXPECT_EQ(tallerChild(&left), left.lc);
    EXPECT_EQ(tallerChild(&right), right.rc);
}
