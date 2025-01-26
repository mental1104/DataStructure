/*
 * @Date: 2025-01-24 13:55:48
 * @Author: mental1104 mental1104@gmail.com
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2025-01-26 16:25:29
 */
#include "gtest/gtest.h"
#include "Stack.h"  // 引入Stack类头文件

// 测试 Stack 类基本操作
TEST(StackTest, PushPopTop) {
    // 创建一个 Stack 实例
    Stack<int> stack;
    
    // 测试 push 操作
    stack.push(10);
    stack.push(20);
    stack.push(30);

    // 检查栈顶元素
    EXPECT_EQ(stack.top(), 30); // 最后一个插入的元素应该是栈顶元素

    // 测试 pop 操作
    EXPECT_EQ(stack.pop(), 30); // 弹出栈顶元素
    EXPECT_EQ(stack.top(), 20); // 新的栈顶元素应该是20

    // 测试栈大小变化
    EXPECT_EQ(stack.size(), 2); // 栈的大小应为2

    // 弹出另一个元素
    EXPECT_EQ(stack.pop(), 20);
    EXPECT_EQ(stack.top(), 10); // 新的栈顶元素应该是10

    // 测试栈是否空了
    EXPECT_EQ(stack.size(), 1);
}

// 测试栈空时操作
TEST(StackTest, EmptyStack) {
    Stack<int> stack;

    // 测试栈空时 top 操作
    try {
        stack.top();
        FAIL() << "Expected std::out_of_range exception";
    } catch (const std::out_of_range& e) {
        EXPECT_EQ(e.what(), std::string("Stack is empty"));
    } catch (...) {
        FAIL() << "Expected std::out_of_range exception";
    }

    // 测试栈空时 pop 操作
    try {
        stack.pop();
        FAIL() << "Expected std::out_of_range exception";
    } catch (const std::out_of_range& e) {
        EXPECT_EQ(e.what(), std::string("Stack is empty"));
    } catch (...) {
        FAIL() << "Expected std::out_of_range exception";
    }
}

// 测试栈的LIFO特性
TEST(StackTest, LifoBehavior) {
    Stack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    
    // 测试栈顶元素是否遵循后进先出原则
    EXPECT_EQ(stack.pop(), 3); // 先弹出3
    EXPECT_EQ(stack.pop(), 2); // 再弹出2
    EXPECT_EQ(stack.pop(), 1); // 最后弹出1
}

// 测试 Stack 在栈为空时的操作
TEST(StackTest, StackSizeEmpty) {
    Stack<int> stack;

    // 栈为空时，size应为0
    EXPECT_EQ(stack.size(), 0);
    
    stack.push(100);  // 入栈100
    EXPECT_EQ(stack.size(), 1); // 栈大小应为1

    stack.pop();  // 弹出
    EXPECT_EQ(stack.size(), 0); // 栈应为空
}

// 测试栈扩容功能
TEST(StackTest, StackExpand) {
    Stack<int> stack;

    // 默认栈的初始容量为DEFAULT_CAPACITY
    int initial_capacity = stack.capacity();
    const int size = 2000;
    for (int i = 0; i < size; i++) {
        stack.push(i);
    }

    // 检查栈是否自动扩容
    EXPECT_GT(stack.capacity(), initial_capacity);
    EXPECT_EQ(stack.size(), size);  // 栈的大小应为2
}

