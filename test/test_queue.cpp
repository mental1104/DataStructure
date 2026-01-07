/*
 * @Date: 2025-01-26 16:24:52
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-26 16:32:28
 */
#include "gtest/gtest.h"
#include "Queue.h"

// 测试 Queue 的构造函数
TEST(QueueTest, Constructor) {
    Queue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// 测试 enqueue 和 size 方法
TEST(QueueTest, Enqueue) {
    Queue<int> q;
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 3);
}

// 测试 front 方法
TEST(QueueTest, Front) {
    Queue<int> q;
    q.enqueue(10);
    q.enqueue(20);

    EXPECT_EQ(q.front(), 10);

    q.dequeue();
    EXPECT_EQ(q.front(), 20);
}

// 测试 dequeue 方法
TEST(QueueTest, Dequeue) {
    Queue<int> q;
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    EXPECT_EQ(q.dequeue(), 10);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.dequeue(), 20);
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.dequeue(), 30);
    EXPECT_TRUE(q.empty());
}

// 测试边界情况：对空队列调用 dequeue
TEST(QueueTest, DequeueEmptyQueue) {
    Queue<int> q;

    // 测试从空队列中出队时是否抛出异常或处理错误
    EXPECT_THROW(q.dequeue(), std::runtime_error); // 需要确保实现中抛出异常
}

TEST(QueueTest, FrontEmptyQueue) {
    Queue<int> q;
    EXPECT_THROW(q.front(), std::runtime_error);
}

// 测试队列的清空
TEST(QueueTest, ClearQueue) {
    Queue<int> q;
    q.enqueue(10);
    q.enqueue(20);

    while (!q.empty()) {
        q.dequeue();
    }

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}
