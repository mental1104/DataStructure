// splay_test.cpp
#include "gtest/gtest.h"
#include "Splay.h"  // 包含 Splay 以及 BST、BinTree、BinNode 等的定义
#include <cstdio>

// ================= 辅助函数 =================

// 1. 利用 Splay 树继承自 BinTree 的 travIn 方法，将中序遍历结果依次存入 C 数组 arr 中。
//    capacity 为数组容量，返回实际存入的元素个数。
template<typename T>
int getInOrder(Splay<T>& tree, T arr[], int capacity) {
    int count = 0;
    tree.travIn([&](const T &value) {
        if(count < capacity)
            arr[count++] = value;
    });
    return count;
}

// 2. 用冒泡排序对 C 数组进行升序排序
void sortArray(int arr[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if(arr[i] > arr[j]){
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

// 3. 比较两个 C 数组是否相等。若长度不同或对应位置元素不等，则返回 false。
bool arraysEqual(const int arr1[], int count1, const int arr2[], int count2) {
    if(count1 != count2)
        return false;
    for(int i = 0; i < count1; i++){
        if(arr1[i] != arr2[i])
            return false;
    }
    return true;
}

// 4. 从 C 数组中删除第一次出现的值等于 value 的元素，删除后将后续元素左移，并将 count 减 1。
//    删除成功返回 true，否则返回 false。
bool removeElement(int arr[], int &count, int value) {
    for(int i = 0; i < count; i++){
        if(arr[i] == value){
            for(int j = i; j < count - 1; j++){
                arr[j] = arr[j+1];
            }
            count--;
            return true;
        }
    }
    return false;
}

// ================= 单元测试 =================

/*
 * 测试1：插入操作后，Splay 树的中序遍历结果应始终有序，
 *       同时节点数应正确。由于插入时内部会执行伸展操作，
 *       可观察到最新插入的结点应成为根结点（除重复插入情况）。
 */
TEST(SplayTest, InsertionMaintainsInOrder) {
    Splay<int> splay;
    int expected[100];      // 预期中序序列
    int expectedCount = 0;
    int inOrder[100];

    // 采用一组数据，插入时可能触发伸展操作
    int nums[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35};
    int nCount = sizeof(nums)/sizeof(nums[0]);
    
    for (int i = 0; i < nCount; i++) {
        int num = nums[i];
        splay.insert(num);
        expected[expectedCount++] = num;
        sortArray(expected, expectedCount);

        int inCount = getInOrder(splay, inOrder, 100);
        EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
            << "插入 " << num << " 后，中序遍历结果不符合预期。";
        EXPECT_EQ(splay.size(), expectedCount)
            << "插入 " << num << " 后，Splay 树的节点数应为 " << expectedCount;
    }
}

/*
 * 测试2：验证搜索操作后，被查结点“伸展”至根。
 *       例如，构造 Splay 树后，搜索某个存在的元素，
 *       检查返回的根结点数据是否与之相等。
 */
TEST(SplayTest, SplayOperationOnSearch) {
    Splay<int> splay;
    splay.insert(50);
    splay.insert(30);
    splay.insert(70);
    splay.insert(40);
    // 搜索 30 后，结点 30 应该伸展至根
    splay.search(30);
    EXPECT_EQ(splay.root()->data, 30)
        << "搜索后，被查结点应伸展至根。";
}

/*
 * 测试3：重复插入相同元素时，不应增加节点数，
 *       且中序遍历结果不变，且被查结点应伸展至根。
 */
TEST(SplayTest, DuplicateInsertion) {
    Splay<int> splay;
    splay.insert(15);
    splay.insert(10);
    splay.insert(20);
    int oldSize = splay.size();

    // 重复插入 15，search 内部会伸展已有结点至根
    BinNode<int>*& nodePtr = splay.search(15);
    BinNode<int>* ret = splay.insert(15);
    EXPECT_EQ(splay.size(), oldSize)
        << "重复插入元素不应增加 Splay 树的节点数。";
    ASSERT_TRUE(ret != nullptr);
    EXPECT_EQ(ret->data, 15)
        << "重复插入后返回的结点数据应为 15。";

    // 检查中序遍历结果
    int expected[] = {10, 15, 20};
    int expectedCount = 3;
    int inOrder[100];
    int inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
        << "重复插入后，中序遍历结果应保持不变。";
}

/*
 * 测试4：删除操作后，Splay 树的中序遍历结果与节点数应正确。
 *       测试包括删除叶结点、只有一个子结点的结点以及双子结点情况。
 */
TEST(SplayTest, RemoveMaintainsInOrder) {
    Splay<int> splay;
    int expected[100];
    int expectedCount = 0;
    int inOrder[100];

    int nums[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35};
    int nCount = sizeof(nums)/sizeof(nums[0]);
    for (int i = 0; i < nCount; i++) {
        splay.insert(nums[i]);
        expected[expectedCount++] = nums[i];
    }
    sortArray(expected, expectedCount);
    int inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount));
    EXPECT_EQ(splay.size(), expectedCount);

    // 删除叶结点：例如 10（假设 10 为叶结点）
    EXPECT_TRUE(splay.remove(10));
    EXPECT_TRUE(removeElement(expected, expectedCount, 10));
    inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
        << "删除叶结点 10 后，中序遍历不符合预期。";
    EXPECT_EQ(splay.size(), expectedCount);

    // 删除只有一个子结点的结点：例如 20（视具体结构而定）
    EXPECT_TRUE(splay.remove(20));
    EXPECT_TRUE(removeElement(expected, expectedCount, 20));
    inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
        << "删除结点 20 后，中序遍历不符合预期。";
    EXPECT_EQ(splay.size(), expectedCount);

    // 删除双子结点的结点：例如 50
    EXPECT_TRUE(splay.remove(50));
    EXPECT_TRUE(removeElement(expected, expectedCount, 50));
    inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
        << "删除结点 50 后，中序遍历不符合预期。";
    EXPECT_EQ(splay.size(), expectedCount);

    // 尝试删除不存在的元素：例如 100，应返回 false，树结构不变
    EXPECT_FALSE(splay.remove(100));
    inCount = getInOrder(splay, inOrder, 100);
    EXPECT_TRUE(arraysEqual(inOrder, inCount, expected, expectedCount))
        << "删除不存在的元素后，中序遍历应不变。";
    EXPECT_EQ(splay.size(), expectedCount);
}

/*
 * 测试5：删除操作后，被删除的元素应在 Splay 树中查找不到。
 *       注意：Splay 的 search 操作若未找到目标，会伸展最后访问的结点，
 *       因此返回的根结点数据不应等于被查找的元素。
 */
TEST(SplayTest, SearchAfterRemoval) {
    Splay<int> splay;
    splay.insert(50);
    splay.insert(30);
    splay.insert(70);
    // 删除 30
    EXPECT_TRUE(splay.remove(30));

    // 搜索 30 后，由于不存在，应返回非 30 的结点（即最后访问结点被伸展至根）
    BinNode<int>*& result = splay.search(30);
    EXPECT_NE(result->data, 30)
        << "删除后的元素在 Splay 树中应查找不到。";

    // 同时检查中序遍历中不包含 30
    int inOrder[100];
    int inCount = getInOrder(splay, inOrder, 100);
    for (int i = 0; i < inCount; i++) {
        EXPECT_NE(inOrder[i], 30) << "中序遍历中不应存在已删除的元素 30。";
    }
}
