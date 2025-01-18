#include "gtest/gtest.h"
#include "Vector.h"

// 测试 Vector 的构造函数
TEST(VectorTest, Constructor) {
    Vector<int> vec1(10, 5, 1);
    EXPECT_EQ(vec1.size(), 5);
    EXPECT_EQ(vec1.capacity(), 10);
    for (int i = 0; i < vec1.size(); ++i) {
        EXPECT_EQ(vec1[i], 1);
    }

    int arr[] = {1, 2, 3, 4, 5};
    Vector<int> vec2(arr, 5);
    EXPECT_EQ(vec2.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec2[i], arr[i]);
    }
}

// 测试插入功能
TEST(VectorTest, Insert) {
    Vector<int> vec;
    vec.insert(0, 10);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 10);

    vec.insert(1, 20);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[1], 20);

    vec.insert(1, 15);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[1], 15);
    EXPECT_EQ(vec[2], 20);
}

// 测试删除功能
TEST(VectorTest, Remove) {
    Vector<int> vec(10, 5, 1);
    EXPECT_EQ(vec.size(), 5);

    vec.remove(1);
    EXPECT_EQ(vec.size(), 4);

    vec.remove(0, 2);
    EXPECT_EQ(vec.size(), 2);
}

// 测试查找功能
TEST(VectorTest, Find) {
    int arr[] = {1, 2, 3, 4, 5};
    Vector<int> vec(arr, 5);

    EXPECT_EQ(vec.find(3), 2);
    EXPECT_EQ(vec.find(6), -1);
}

// 测试唯一化功能
TEST(VectorTest, Uniquify) {
    int arr[] = {1, 1, 2, 2, 3, 3};
    Vector<int> vec(arr, 6);
    EXPECT_EQ(vec.uniquify(), 3);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

// 测试遍历功能
void doubleValue(int& val) {
    val *= 2;
}

TEST(VectorTest, Traverse) {
    int arr[] = {1, 2, 3, 4, 5};
    Vector<int> vec(arr, 5);
    vec.traverse(doubleValue);

    EXPECT_EQ(vec[0], 2);
    EXPECT_EQ(vec[1], 4);
    EXPECT_EQ(vec[2], 6);
    EXPECT_EQ(vec[3], 8);
    EXPECT_EQ(vec[4], 10);
}

// 测试是否无序
TEST(VectorTest, Disordered) {
    int arr1[] = {1, 2, 3, 4, 5};
    Vector<int> vec1(arr1, 5);
    EXPECT_EQ(vec1.disordered(), 0);

    int arr2[] = {5, 3, 2, 1};
    Vector<int> vec2(arr2, 4);
    EXPECT_EQ(vec2.disordered(), 3);
}

