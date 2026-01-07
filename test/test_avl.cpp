// avl_test.cpp
#include "gtest/gtest.h"
#include "AVL.h"  // 包含 AVL 及其依赖的头文件
#include <cstdio>
#include <cmath>

// =============== 辅助函数 ===============

// 1. 利用 AVL（继承自 BST/BinTree）的 travIn 方法获取中序遍历结果，结果存入 C 数组 arr 中。
//    capacity 表示数组容量，返回实际存入的元素个数。
template<typename T>
int getInOrder(AVL<T>& tree, T arr[], int capacity) {
    int count = 0;
    tree.travIn([&](const T& value) {
        if (count < capacity) {
            arr[count++] = value;
        }
    });
    return count;
}

// 2. 实现一个简单的冒泡排序，对 C 数组进行升序排序
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

// 3. 比较两个 C 数组是否相等。若长度不同或对应元素不相等，则返回 false。
bool arraysEqual(const int arr1[], int count1, const int arr2[], int count2) {
    if (count1 != count2)
        return false;
    for (int i = 0; i < count1; i++) {
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

// 4. 从 C 数组中删除第一个值等于 value 的元素，删除后将后续元素左移，同时 count 减 1。
//    若删除成功返回 true，否则返回 false。
bool removeElement(int arr[], int &count, int value) {
    for (int i = 0; i < count; i++) {
        if (arr[i] == value) {
            for (int j = i; j < count - 1; j++) {
                arr[j] = arr[j + 1];
            }
            count--;
            return true;
        }
    }
    return false;
}

// 5. 检查 AVL 平衡性：递归计算以 node 为根的子树高度，同时检测任一结点左右子树高度差是否超过 1。
//    若超过，则通过 bool 引用参数 balanced 置为 false。
template<typename T>
int verifyAVL(BinNode<T>* node, bool &balanced) {
    if (!node) return 0;
    int leftHeight = verifyAVL(node->lc, balanced);
    int rightHeight = verifyAVL(node->rc, balanced);
    if (std::abs(leftHeight - rightHeight) > 1)
        balanced = false;
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// 6. 通过根结点检查整个 AVL 树是否满足平衡性
template<typename T>
bool isAVL(BinNode<T>* root) {
    bool balanced = true;
    verifyAVL(root, balanced);
    return balanced;
}

// =============== 单元测试 ===============

// 测试 1：插入操作后，AVL 树不仅中序遍历结果有序，而且整体满足 AVL 平衡性
TEST(AVLTest, InsertionMaintainsInOrderAndAVLProperty) {
    AVL<int> avl;
    int expected[100];      // 用于保存所有已插入元素（预期中序序列）
    int expectedCount = 0;
    int inOrder[100];       // 实际中序遍历结果

    // 选择一组数据，注意部分数据插入后会触发 AVL 树的旋转
    int nums[] = {10, 20, 30, 40, 50, 25};
    int nCount = sizeof(nums) / sizeof(nums[0]);

    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        avl.insert(num);
        expected[expectedCount++] = num;
        sortArray(expected, expectedCount);

        int inCount = getInOrder(avl, inOrder, 100);
        EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
            << "插入 " << num << " 后，中序遍历结果不符合预期。";
        EXPECT_EQ(avl.size(), expectedCount)
            << "插入 " << num << " 后，AVL 树的节点数应为 " << expectedCount;

        // 检查 AVL 平衡性：从根结点出发检测整棵树是否满足高度平衡条件
        EXPECT_TRUE(isAVL(avl.root()))
            << "插入 " << num << " 后，AVL 平衡性被破坏。";
    }
}

// 测试 2：重复插入同一元素时，不应增加节点数，并且中序遍历与 AVL 平衡性保持正确
TEST(AVLTest, DuplicateInsertion) {
    AVL<int> avl;
    avl.insert(15);
    avl.insert(10);
    avl.insert(20);
    int oldSize = avl.size();

    // 对元素 15 重复插入，insert 应返回已有结点指针，且树结构不变
    BinNode<int>*& nodePtr = avl.search(15);
    nodePtr = avl.insert(15);
    EXPECT_EQ(avl.size(), oldSize)
        << "重复插入元素不应增加 AVL 树的节点数。";
    ASSERT_TRUE(nodePtr != nullptr);
    EXPECT_EQ(nodePtr->data, 15);

    // 检查中序遍历结果
    int expected[] = {10, 15, 20};
    int expectedCount = 3;
    int inOrder[100];
    int inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    // 同时检查 AVL 平衡性
    EXPECT_TRUE(isAVL(avl.root()));
}

// 测试 3：删除操作后，无论删除叶结点、单子结点或双子结点，都应保持中序遍历有序和 AVL 平衡性
TEST(AVLTest, RemoveMaintainsInOrderAndAVLProperty) {
    AVL<int> avl;
    int expected[100];
    int expectedCount = 0;
    int inOrder[100];

    int nums[] = {20, 10, 30, 5, 15, 25, 35};
    int nCount = sizeof(nums) / sizeof(nums[0]);
    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        avl.insert(num);
        expected[expectedCount++] = num;
    }
    sortArray(expected, expectedCount);
    int inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(avl.size(), expectedCount);
    EXPECT_TRUE(isAVL(avl.root()));

    // 删除叶结点：删除 5
    EXPECT_TRUE(avl.remove(5));
    EXPECT_TRUE(removeElement(expected, expectedCount, 5));
    inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(avl.size(), expectedCount);
    EXPECT_TRUE(isAVL(avl.root()));

    // 删除只有一个子树的结点：删除 30（具体结构可能会因旋转略有不同）
    EXPECT_TRUE(avl.remove(30));
    EXPECT_TRUE(removeElement(expected, expectedCount, 30));
    inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(avl.size(), expectedCount);
    EXPECT_TRUE(isAVL(avl.root()));

    // 删除有两个子结点的结点：删除 20
    EXPECT_TRUE(avl.remove(20));
    EXPECT_TRUE(removeElement(expected, expectedCount, 20));
    inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(avl.size(), expectedCount);
    EXPECT_TRUE(isAVL(avl.root()));

    // 删除不存在的元素：删除 100 应返回 false，树结构不变
    EXPECT_FALSE(avl.remove(100));
    inCount = getInOrder(avl, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(avl.size(), expectedCount);
    EXPECT_TRUE(isAVL(avl.root()));
}

// 测试 4：删除后，被删除的元素在 AVL 树中应无法查找到
TEST(AVLTest, SearchAfterRemoval) {
    AVL<int> avl;
    avl.insert(50);
    avl.insert(30);
    avl.insert(70);
    EXPECT_TRUE(avl.remove(30));

    // search 返回 BinNode<T>* 的引用，删除后应返回 nullptr
    BinNode<int>*& result = avl.search(30);
    EXPECT_EQ(result, nullptr)
        << "删除后的元素在 AVL 树中应查找不到。";
    EXPECT_TRUE(isAVL(avl.root()));
}

TEST(AVLTest, RemoveTriggersRebalance) {
    AVL<int> avl;
    int nums[] = {3, 2, 4, 1};
    for (int i = 0; i < 4; ++i) {
        avl.insert(nums[i]);
    }
    EXPECT_TRUE(isAVL(avl.root()));

    EXPECT_TRUE(avl.remove(4));
    EXPECT_TRUE(isAVL(avl.root()));
    ASSERT_NE(avl.root(), nullptr);
    EXPECT_EQ(avl.root()->data, 2);
}
