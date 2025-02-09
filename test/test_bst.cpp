// bst_test.cpp
#include "gtest/gtest.h"
#include "BST.h"  // 包含 BST 及其依赖的头文件
#include <cstdio> // 用于 printf 等

//===================== 辅助函数 =====================

// 用于从 BST 中获取中序遍历的结果，结果存放在传入的 C 数组 arr 中，capacity 为数组容量。
// 返回实际存入的元素个数。
template<typename T>
int getInOrder(BST<T>& tree, T arr[], int capacity) {
    int count = 0;
    tree.travIn([&](const T& value) {
        if (count < capacity) {
            arr[count++] = value;
        }
    });
    return count;
}

// 实现一个简单的排序算法（冒泡排序），对 C 数组 arr 进行排序（升序）。
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

// 比较两个 C 数组是否相等。两个数组长度不同或对应位置元素不同则返回 false。
bool arraysEqual(const int arr1[], int count1, const int arr2[], int count2) {
    if (count1 != count2)
        return false;
    for (int i = 0; i < count1; i++) {
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

// 从 C 数组 arr 中删除第一个值等于 value 的元素，删除后将后续元素左移，并将 count 减 1。
// 如果删除成功返回 true，否则返回 false。
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

//===================== 测试用例 =====================

// 测试1：每次插入后都验证中序遍历是否有序，并检查 BST 的节点数
TEST(BSTTest, InsertionMaintainsInOrder) {
    BST<int> bst;
    int expected[100];      // 预期中序结果数组
    int expectedCount = 0;  // 当前预期元素个数
    int inOrder[100];       // BST 中序遍历结果数组

    int nums[] = {5, 3, 7, 2, 4, 6, 8};
    int nCount = sizeof(nums) / sizeof(nums[0]);

    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        bst.insert(num);
        // 将新插入的元素加入预期数组，然后排序
        expected[expectedCount++] = num;
        sortArray(expected, expectedCount);

        int inCount = getInOrder(bst, inOrder, 100);
        EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
            << "插入 " << num << " 后，中序遍历结果不符合预期。";
        EXPECT_EQ(bst.size(), expectedCount)
            << "插入 " << num << " 后，BST 的节点数应为 " << expectedCount;
    }
}

// 测试2：重复插入同一元素不应增加节点数
TEST(BSTTest, DuplicateInsertion) {
    BST<int> bst;
    bst.insert(10);
    bst.insert(5);
    bst.insert(15);
    int oldSize = bst.size();

    // 对元素 10 重复插入，insert 应返回已有节点的指针，BST 大小不变
    BinNode<int>*& nodePtr = bst.search(10);
    nodePtr = bst.insert(10);
    EXPECT_EQ(bst.size(), oldSize) << "重复插入元素不应增加 BST 的节点数。";
    ASSERT_TRUE(nodePtr != nullptr);
    EXPECT_EQ(nodePtr->data, 10);

    // 验证中序遍历结果应为 {5, 10, 15}
    int expected[] = {5, 10, 15};
    int expectedCount = 3;
    int inOrder[100];
    int inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
}

// 测试3：删除操作后 BST 依然保持中序有序，覆盖叶节点、单子节点及双子节点情况
TEST(BSTTest, RemoveMaintainsInOrder) {
    BST<int> bst;
    int expected[100];
    int expectedCount = 0;
    int inOrder[100];

    int nums[] = {5, 3, 7, 2, 4, 6, 8};
    int nCount = sizeof(nums) / sizeof(nums[0]);
    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        bst.insert(num);
        expected[expectedCount++] = num;
    }
    // 初始预期中序序列排序后应为 {2, 3, 4, 5, 6, 7, 8}
    sortArray(expected, expectedCount);
    int inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));

    // 删除叶节点：删除 2
    EXPECT_TRUE(bst.remove(2));
    EXPECT_TRUE(removeElement(expected, expectedCount, 2));
    inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(bst.size(), expectedCount);

    // 删除只有一个子树的节点：删除 3
    EXPECT_TRUE(bst.remove(3));
    EXPECT_TRUE(removeElement(expected, expectedCount, 3));
    inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(bst.size(), expectedCount);

    // 删除有两个子节点的节点：删除 7
    EXPECT_TRUE(bst.remove(7));
    EXPECT_TRUE(removeElement(expected, expectedCount, 7));
    inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(bst.size(), expectedCount);

    // 尝试删除不存在的元素：删除 100 应返回 false，BST 结构不变
    EXPECT_FALSE(bst.remove(100));
    inCount = getInOrder(bst, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(bst.size(), expectedCount);
}

// 测试4：验证删除后被删除的元素无法查找到
TEST(BSTTest, SearchAfterRemoval) {
    BST<int> bst;
    bst.insert(10);
    bst.insert(5);
    bst.insert(15);
    EXPECT_TRUE(bst.remove(5));

    // search 返回的是 BinNode<T>* 的引用，删除后应返回 nullptr
    BinNode<int>*& result = bst.search(5);
    EXPECT_EQ(result, nullptr) << "删除后的元素在 BST 中应查找不到。";
}