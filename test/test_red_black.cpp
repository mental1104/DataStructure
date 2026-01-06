// redblack_test.cpp
#include "gtest/gtest.h"
#include "RedBlack.h"   // 包含 RedBlack 以及 BST、BinTree、BinNode 等定义
#include <cstdio>
#include <cmath>

// ================= 辅助函数 =================

// 1. 利用红黑树（继承自 BinTree）的 travIn 方法，将中序遍历结果依次存入 C 数组 arr 中。
//    capacity 为数组容量，返回实际存入的元素个数。
template<typename T>
int getInOrder(RedBlack<T>& tree, T arr[], int capacity) {
    int count = 0;
    tree.travIn([&](const T &value) {
        if (count < capacity)
            arr[count++] = value;
    });
    return count;
}

// 2. 使用冒泡排序对 C 数组进行升序排序
void sortArray(int arr[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (arr[i] > arr[j]) {
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

// 3. 比较两个 C 数组是否相等。若长度不同或对应位置元素不等，则返回 false。
bool arraysEqual(const int arr1[], int count1, const int arr2[], int count2) {
    if (count1 != count2)
        return false;
    for (int i = 0; i < count1; i++) {
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

// 4. 从 C 数组中删除第一次出现的值等于 value 的元素，删除后将后续元素左移，并将 count 减 1。
//    如果删除成功返回 true，否则返回 false。
bool removeElement(int arr[], int &count, int value) {
    for (int i = 0; i < count; i++) {
        if (arr[i] == value) {
            for (int j = i; j < count - 1; j++) {
                arr[j] = arr[j+1];
            }
            count--;
            return true;
        }
    }
    return false;
}

// 5. 递归检查以 node 为根的子树是否满足红黑树性质，并返回该子树的黑高度（定义：空结点黑高度为 0）。
//    检查的内容包括：若结点为红，则其左右子均必须为黑；左右子树的黑高度必须相等。
int checkRB(BinNode<int>* node, bool &valid) {
    if (!node)
        return 0;
    // 红结点不能有红子
    if (IsRed(node)) {
        if (!IsBlack(node->lc) || !IsBlack(node->rc)) {
            valid = false;
        }
    }
    int leftBH = checkRB(node->lc, valid);
    int rightBH = checkRB(node->rc, valid);
    if (leftBH != rightBH) {
        valid = false;
    }
    return IsBlack(node) ? leftBH + 1 : leftBH;
}

// 6. 检查整个红黑树是否满足红黑性质：
//    (1) 根必须为黑；
//    (2) 每个红结点的子结点必须为黑；
//    (3) 从任一结点到其所有后代空结点的路径具有相同的黑高度。
bool verifyRedBlackTree(BinNode<int>* root) {
    if (!root)
        return true;
    if (!IsBlack(root))
        return false;
    bool valid = true;
    checkRB(root, valid);
    return valid;
}

class RedBlackHarness : public RedBlack<int> {
public:
    using RedBlack<int>::_root;
    using RedBlack<int>::_size;
    using RedBlack<int>::_hot;
    using RedBlack<int>::solveDoubleBlack;
};

static BinNode<int>* makeNode(int value, RBColor color, int height = 0) {
    return new BinNode<int>(value, nullptr, nullptr, nullptr, height, 1, color);
}

static void linkLeft(BinNode<int>* parent, BinNode<int>* child) {
    parent->lc = child;
    if (child) child->parent = parent;
}

static void linkRight(BinNode<int>* parent, BinNode<int>* child) {
    parent->rc = child;
    if (child) child->parent = parent;
}

// ================= 单元测试 =================

/*
 * 测试1：插入操作后，红黑树的中序遍历结果应始终有序，
 *       同时树的大小正确，并且整个树满足红黑树性质。
 */
TEST(RBTreeTest, InsertionMaintainsInOrderAndRBProperty) {
    RedBlack<int> rb;
    int expected[100];      // 保存已插入的所有元素（预期中序序列）
    int expectedCount = 0;
    int inOrder[100];       // 实际中序遍历结果

    // 选择一组数据，部分插入会引起红黑树的调整
    int nums[] = {30, 20, 40, 10, 25, 35, 50, 5, 15, 27};
    int nCount = sizeof(nums) / sizeof(nums[0]);

    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        rb.insert(num);
        expected[expectedCount++] = num;
        sortArray(expected, expectedCount);

        int inCount = getInOrder(rb, inOrder, 100);
        EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
            << "插入 " << num << " 后，中序遍历结果不符合预期。";
        EXPECT_EQ(rb.size(), expectedCount)
            << "插入 " << num << " 后，红黑树的节点数应为 " << expectedCount;
        EXPECT_TRUE(verifyRedBlackTree(rb.root()))
            << "插入 " << num << " 后，红黑树的性质被破坏。";
    }
}

TEST(RBTreeTest, InsertTriggersZigZigRotation) {
    RedBlack<int> rb;
    rb.insert(10);
    rb.insert(5);
    rb.insert(1);
    ASSERT_TRUE(rb.root() != nullptr);
    EXPECT_EQ(rb.root()->data, 5);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}

TEST(RBTreeTest, InsertTriggersZigZagRotation) {
    RedBlack<int> rb;
    rb.insert(10);
    rb.insert(5);
    rb.insert(7);
    ASSERT_TRUE(rb.root() != nullptr);
    EXPECT_EQ(rb.root()->data, 7);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}

/*
 * 测试2：重复插入同一元素时，不应增加节点数，
 *       同时树的中序序列与红黑性质均应保持正确。
 */
TEST(RBTreeTest, DuplicateInsertion) {
    RedBlack<int> rb;
    rb.insert(15);
    rb.insert(10);
    rb.insert(20);
    int oldSize = rb.size();

    // 重复插入 15 应返回已有结点，且树的结构不变
    BinNode<int>*& nodePtr = rb.search(15);
    nodePtr = rb.insert(15);
    EXPECT_EQ(rb.size(), oldSize)
        << "重复插入元素不应增加红黑树的节点数。";
    ASSERT_TRUE(nodePtr != nullptr);
    EXPECT_EQ(nodePtr->data, 15);

    // 检查中序遍历结果是否为 {10, 15, 20}
    int expected[] = {10, 15, 20};
    int expectedCount = 3;
    int inOrder[100];
    int inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}

/*
 * 测试3：删除操作后（包括删除叶结点、只有一个子结点、以及双子结点），
 *       红黑树的中序遍历结果、节点数以及红黑性质均应保持正确。
 */
TEST(RBTreeTest, RemoveMaintainsInOrderAndRBProperty) {
    RedBlack<int> rb;
    int expected[100];
    int expectedCount = 0;
    int inOrder[100];

    int nums[] = {30, 20, 40, 10, 25, 35, 50, 5, 15, 27};
    int nCount = sizeof(nums) / sizeof(nums[0]);
    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        rb.insert(num);
        expected[expectedCount++] = num;
    }
    sortArray(expected, expectedCount);
    int inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(rb.size(), expectedCount);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));

    // 删除叶结点：删除 5
    EXPECT_TRUE(rb.remove(5));
    EXPECT_TRUE(removeElement(expected, expectedCount, 5));
    inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(rb.size(), expectedCount);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));

    // 删除只有一个子结点的结点：删除 40（具体结构可能因调整略有不同）
    EXPECT_TRUE(rb.remove(40));
    EXPECT_TRUE(removeElement(expected, expectedCount, 40));
    inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(rb.size(), expectedCount);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));

    // 删除双子结点的结点：删除 20
    EXPECT_TRUE(rb.remove(20));
    EXPECT_TRUE(removeElement(expected, expectedCount, 20));
    inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(rb.size(), expectedCount);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));

    // 尝试删除不存在的元素：删除 100 应返回 false，树结构不变
    EXPECT_FALSE(rb.remove(100));
    inCount = getInOrder(rb, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(rb.size(), expectedCount);
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}

TEST(RBTreeTest, RemoveRootWithSingleChild) {
    RedBlack<int> rb;
    rb.insert(10);
    rb.insert(5);
    EXPECT_TRUE(rb.remove(10));
    ASSERT_TRUE(rb.root() != nullptr);
    EXPECT_EQ(rb.root()->data, 5);
    EXPECT_TRUE(IsBlack(rb.root()));
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}

TEST(RBTreeTest, RemoveTriggersDoubleBlackFixup) {
    RedBlackHarness rb;
    BinNode<int>* root = makeNode(10, RBColor::BLACK);
    BinNode<int>* left = makeNode(5, RBColor::BLACK);
    BinNode<int>* right = makeNode(15, RBColor::BLACK);
    BinNode<int>* left_left = makeNode(2, RBColor::BLACK);
    BinNode<int>* left_right = makeNode(7, RBColor::BLACK);

    linkLeft(root, left);
    linkRight(root, right);
    linkLeft(left, left_left);
    linkRight(left, left_right);

    rb._root = root;
    rb._size = 5;

    EXPECT_TRUE(rb.remove(2));
    EXPECT_EQ(rb.size(), 4);
    BinNode<int>*& res = rb.search(2);
    EXPECT_EQ(res, nullptr);
}

TEST(RBTreeTest, SolveDoubleBlackBlackSiblingWithRedChild) {
    RedBlackHarness rb;
    BinNode<int>* root = makeNode(10, RBColor::BLACK);
    BinNode<int>* r = makeNode(5, RBColor::BLACK);
    BinNode<int>* s = makeNode(15, RBColor::BLACK);
    BinNode<int>* t = makeNode(20, RBColor::RED);

    linkLeft(root, r);
    linkRight(root, s);
    linkRight(s, t);

    rb._root = root;
    rb._size = 4;

    rb.solveDoubleBlack(r);
    ASSERT_TRUE(rb.root() != nullptr);
    EXPECT_EQ(rb.root()->data, 15);
    ASSERT_TRUE(rb.root()->lc != nullptr);
    ASSERT_TRUE(rb.root()->rc != nullptr);
    EXPECT_EQ(rb.root()->lc->color, RBColor::BLACK);
    EXPECT_EQ(rb.root()->rc->color, RBColor::BLACK);
}

TEST(RBTreeTest, SolveDoubleBlackBlackSiblingNoRedChildParentRed) {
    RedBlackHarness rb;
    BinNode<int>* root = makeNode(10, RBColor::RED);
    BinNode<int>* r = makeNode(5, RBColor::BLACK);
    BinNode<int>* s = makeNode(15, RBColor::BLACK);

    linkLeft(root, r);
    linkRight(root, s);

    rb._root = root;
    rb._size = 3;

    rb.solveDoubleBlack(r);
    EXPECT_EQ(root->color, RBColor::BLACK);
    EXPECT_EQ(s->color, RBColor::RED);
}

TEST(RBTreeTest, SolveDoubleBlackBlackSiblingNoRedChildParentBlack) {
    RedBlackHarness rb;
    BinNode<int>* root = makeNode(10, RBColor::BLACK, 2);
    BinNode<int>* r = makeNode(5, RBColor::BLACK);
    BinNode<int>* s = makeNode(15, RBColor::BLACK);

    linkLeft(root, r);
    linkRight(root, s);

    rb._root = root;
    rb._size = 3;

    rb.solveDoubleBlack(r);
    EXPECT_EQ(root->color, RBColor::BLACK);
    EXPECT_EQ(s->color, RBColor::RED);
    EXPECT_EQ(root->height, 1);
}

TEST(RBTreeTest, SolveDoubleBlackRedSibling) {
    RedBlackHarness rb;
    BinNode<int>* root = makeNode(10, RBColor::BLACK);
    BinNode<int>* r = makeNode(5, RBColor::BLACK);
    BinNode<int>* s = makeNode(15, RBColor::RED);
    BinNode<int>* s_left = makeNode(13, RBColor::BLACK);
    BinNode<int>* s_right = makeNode(20, RBColor::BLACK);

    linkLeft(root, r);
    linkRight(root, s);
    linkLeft(s, s_left);
    linkRight(s, s_right);

    rb._root = root;
    rb._size = 5;

    rb.solveDoubleBlack(r);
    ASSERT_TRUE(rb.root() != nullptr);
    EXPECT_EQ(rb.root()->data, 15);
    EXPECT_EQ(rb.root()->color, RBColor::BLACK);
}

/*
 * 测试4：删除操作后，被删除的元素应在红黑树中查找不到。
 */
TEST(RBTreeTest, SearchAfterRemoval) {
    RedBlack<int> rb;
    rb.insert(50);
    rb.insert(30);
    rb.insert(70);
    EXPECT_TRUE(rb.remove(30));

    // search 返回 BinNode<T>* 的引用，删除后应返回 nullptr
    BinNode<int>*& result = rb.search(30);
    EXPECT_EQ(result, nullptr)
        << "删除后的元素在红黑树中应查找不到。";
    EXPECT_TRUE(verifyRedBlackTree(rb.root()));
}
