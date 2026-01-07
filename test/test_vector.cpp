#include "gtest/gtest.h"
#include <atomic>
#include "Vector.h"

namespace {
struct TrackedValue {
    static std::atomic<int> live;
    int value;
    TrackedValue(int v = 0) : value(v) { live.fetch_add(1, std::memory_order_relaxed); }
    TrackedValue(const TrackedValue& other) : value(other.value) { live.fetch_add(1, std::memory_order_relaxed); }
    TrackedValue& operator=(const TrackedValue& other) {
        value = other.value;
        return *this;
    }
    ~TrackedValue() { live.fetch_sub(1, std::memory_order_relaxed); }
};

std::atomic<int> TrackedValue::live{0};
}

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

TEST(VectorTest, RemoveNonTrivialType) {
    TrackedValue::live.store(0, std::memory_order_relaxed);
    {
        Vector<TrackedValue> vec;
        vec.insert(TrackedValue(1));
        vec.insert(TrackedValue(2));
        TrackedValue removed = vec.remove(0);
        EXPECT_EQ(removed.value, 1);
        EXPECT_EQ(vec.size(), 1);
        EXPECT_GT(TrackedValue::live.load(std::memory_order_relaxed), 0);
    }
    EXPECT_EQ(TrackedValue::live.load(std::memory_order_relaxed), 0);
}

// 测试查找功能
TEST(VectorTest, Find) {
    int arr[] = {1, 2, 3, 4, 5};
    Vector<int> vec(arr, 5);

    EXPECT_EQ(vec.find(3), 2);
    EXPECT_EQ(vec.find(6), -1);
}

TEST(VectorTest, FindRangeAndRemoveReturn) {
    int arr[] = {1, 2, 3, 4, 5};
    Vector<int> vec(arr, 5);
    EXPECT_EQ(vec.find(4, 0, vec.size()), 3);
    EXPECT_EQ(vec.find(9, 0, vec.size()), -1);

    int removed = vec.remove(2);
    EXPECT_EQ(removed, 3);
    EXPECT_EQ(vec.size(), 4);
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

TEST(VectorTest, IteratorEnd) {
    Vector<int> vec;
    vec.insert(1);
    vec.insert(2);
    int sum = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 3);
    EXPECT_TRUE(vec.begin() != vec.end());
}

TEST(VectorTest, UnsortAndShrink) {
    Vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.insert(i);
    }
    vec.unsort();
    EXPECT_EQ(vec.size(), 10);

    Vector<int> big(4096, 10, 7);
    int oldCap = big.capacity();
    big.remove(0, big.size() - 2);
    EXPECT_LT(big.capacity(), oldCap);
    EXPECT_EQ(big.size(), 2);
    EXPECT_EQ(big[0], 7);
    EXPECT_EQ(big[1], 7);
}
