#include "gtest/gtest.h"
#include "StackB.h"

// 测试初始化状态
TEST(StackTest, Initialization) {
    Stack<int> stack;

    EXPECT_EQ(stack.size(), 0);      // 初始大小应为0
    EXPECT_TRUE(stack.empty());     // 栈应为空
}

// 测试push和top操作
TEST(StackTest, PushAndTop) {
    Stack<int> stack;

    stack.push(10);
    EXPECT_EQ(stack.size(), 1);     // 插入后大小应增加
    EXPECT_EQ(stack.top(), 10);     // 栈顶元素应为插入的值

    stack.push(20);
    EXPECT_EQ(stack.size(), 2);     // 再次插入大小增加
    EXPECT_EQ(stack.top(), 20);     // 栈顶更新
}

// 测试pop操作
TEST(StackTest, Pop) {
    Stack<int> stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    EXPECT_EQ(stack.pop(), 30);     // 出栈的值应为最后插入的值
    EXPECT_EQ(stack.size(), 2);     // 出栈后大小减少
    EXPECT_EQ(stack.top(), 20);     // 栈顶更新为上一个值

    EXPECT_EQ(stack.pop(), 20);
    EXPECT_EQ(stack.pop(), 10);

    EXPECT_TRUE(stack.empty());     // 所有元素出栈后栈应为空
}

// 测试边界条件：对空栈调用pop和top
TEST(StackTest, EmptyStackOperations) {
    Stack<int> stack;

    EXPECT_THROW(stack.pop(), std::out_of_range); // pop操作应抛出异常
    EXPECT_THROW(stack.top(), std::out_of_range); // top操作应抛出异常
}

// 测试混合操作
TEST(StackTest, MixedOperations) {
    Stack<int> stack;

    stack.push(5);
    stack.push(15);
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), 15);

    stack.pop();
    EXPECT_EQ(stack.size(), 1);
    EXPECT_EQ(stack.top(), 5);

    stack.push(25);
    EXPECT_EQ(stack.size(), 2);
    EXPECT_EQ(stack.top(), 25);

    stack.pop();
    stack.pop();
    EXPECT_TRUE(stack.empty());
    EXPECT_THROW(stack.pop(), std::out_of_range);
}
