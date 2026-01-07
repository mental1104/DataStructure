#include <gtest/gtest.h>
#include <vector>
#include "BinTree.h" // 假设 BinTree 定义在该头文件中

namespace {
struct IntCollector {
    std::vector<int>* out;
    void operator()(int& v) { out->push_back(v); }
};
}

// 测试 BinTree 初始化
TEST(BinTreeTest, Initialization) {
    BinTree<int> tree;

    // 验证树初始化时为空
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.root(), nullptr);
}

// 测试插入根节点
TEST(BinTreeTest, InsertRoot) {
    BinTree<int> tree;
    BinNode<int>* root = tree.insertAsRoot(10);

    // 验证根节点是否插入成功
    ASSERT_NE(tree.root(), nullptr);
    EXPECT_EQ(root, tree.root());
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(tree.root()->data, 10);
    EXPECT_EQ(tree.root()->parent, nullptr);
    EXPECT_EQ(tree.root()->lc, nullptr);
    EXPECT_EQ(tree.root()->rc, nullptr);
}

// 测试插入左孩子和右孩子
TEST(BinTreeTest, InsertChildren) {
    BinTree<int> tree;
    tree.insertAsRoot(10);

    // 插入左孩子和右孩子
    BinNode<int>* root = tree.root();
    tree.insertAsLC(root, 5);
    tree.insertAsRC(root, 15);

    // 验证左孩子
    ASSERT_NE(root->lc, nullptr);
    EXPECT_EQ(root->lc->data, 5);
    EXPECT_EQ(root->lc->parent, root);

    // 验证右孩子
    ASSERT_NE(root->rc, nullptr);
    EXPECT_EQ(root->rc->data, 15);
    EXPECT_EQ(root->rc->parent, root);

    // 验证树大小
    EXPECT_EQ(tree.size(), 3);
}

// 测试树的高度
TEST(BinTreeTest, TreeHeight) {
    BinTree<int> tree;
    tree.insertAsRoot(10);
    BinNode<int>* root = tree.root();

    // 插入子节点
    tree.insertAsLC(root, 5);
    BinNode<int>* leftChild = root->lc;
    tree.insertAsLC(leftChild, 2); // 左孩子的左孩子

    // 验证高度
    EXPECT_EQ(root->height, 2);  // 根节点高度
    EXPECT_EQ(leftChild->height, 1); // 左孩子高度
    EXPECT_EQ(leftChild->lc->height, 0); // 左孩子的左孩子高度
}

// 测试树的节点数量更新
TEST(BinTreeTest, NodeCount) {
    BinTree<int> tree;
    tree.insertAsRoot(20);

    // 插入多个节点
    BinNode<int>* root = tree.root();
    tree.insertAsLC(root, 10);
    tree.insertAsRC(root, 30);
    tree.insertAsLC(root->lc, 5);  // 左孩子的左孩子
    tree.insertAsRC(root->rc, 35); // 右孩子的右孩子

    // 验证节点数量
    EXPECT_EQ(tree.size(), 5);
}

// 测试树的删除节点
TEST(BinTreeTest, RemoveSubTree) {
    BinTree<int> tree;
    tree.insertAsRoot(50);
    BinNode<int>* root = tree.root();

    // 插入左孩子和右孩子
    tree.insertAsLC(root, 25);
    tree.insertAsRC(root, 75);

    // 删除左子树
    tree.remove(root->lc);

    // 验证左子树被删除
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(root->lc, nullptr);
}

TEST(BinTreeTest, TraversalWrappersAndIterator) {
    BinTree<int> tree;
    tree.insertAsRoot(10);
    BinNode<int>* root = tree.root();
    tree.insertAsLC(root, 5);
    tree.insertAsRC(root, 15);
    tree.insertAsLC(root->lc, 2);
    tree.insertAsRC(root->lc, 7);

    std::vector<int> pre;
    IntCollector preVis{&pre};
    tree.travPre(preVis);
    EXPECT_EQ(pre, std::vector<int>({10, 5, 2, 7, 15}));

    std::vector<int> in;
    IntCollector inVis{&in};
    tree.travIn(inVis);
    EXPECT_EQ(in, std::vector<int>({2, 5, 7, 10, 15}));

    std::vector<int> post;
    IntCollector postVis{&post};
    tree.travPost(postVis);
    EXPECT_EQ(post, std::vector<int>({2, 7, 5, 15, 10}));

    std::vector<int> level;
    IntCollector levelVis{&level};
    tree.travLevel(levelVis);
    EXPECT_EQ(level, std::vector<int>({10, 5, 15, 2, 7}));

    BinTree<int>::iterator it = tree.begin();
    EXPECT_TRUE(it != tree.end());
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 7);
    ++it;
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(*it, 15);
    ++it;
    EXPECT_FALSE(it != tree.end());
}
