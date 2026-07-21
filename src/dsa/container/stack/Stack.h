#ifndef DSA_CONTAINER_STACK_STACK_H
#define DSA_CONTAINER_STACK_STACK_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "../vector/Vector.h"

namespace dsa {
namespace container {

/// STL 风格栈容器适配器；自身不管理元素存储，只将 LIFO 语义映射到底层容器尾部。
template<typename T, typename Container = dsa::container::Vector<T> >
class Stack {
public:
    typedef Container container_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;

    static_assert(
        std::is_same<T, value_type>::value,
        "Stack<T, Container> requires T to match Container::value_type"
    );

protected:
    /// 保存实际元素；适配器只通过 back/push_back/pop_back 操作该容器。
    container_type c;

public:
    /// 默认构造底层容器。
    Stack() noexcept(noexcept(container_type()));

    /// 复制指定底层容器，已有尾元素成为栈顶。
    explicit Stack(const container_type& container);

    /// 移动接管指定底层容器。
    explicit Stack(container_type&& container) noexcept(
        std::is_nothrow_move_constructible<container_type>::value
    );

    /// 使用 allocator 构造底层容器；仅在底层容器声明 uses_allocator 时参与重载。
    template<typename Allocator>
    explicit Stack(
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用 allocator 复制构造底层容器。
    template<typename Allocator>
    Stack(
        const container_type& container,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用 allocator 移动构造底层容器。
    template<typename Allocator>
    Stack(
        container_type&& container,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用指定 allocator 复制另一个适配器保存的底层容器。
    template<typename Allocator>
    Stack(
        const Stack& other,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用指定 allocator 移动另一个适配器保存的底层容器。
    template<typename Allocator>
    Stack(
        Stack&& other,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 判断栈是否为空，复杂度取决于底层容器，标准序列容器为 O(1)。
    bool empty() const noexcept(noexcept(c.empty()));

    /// 返回元素数量，复杂度取决于底层容器，默认工业 Vector 为 O(1)。
    size_type size() const noexcept(noexcept(c.size()));

    /// 返回栈顶可写引用；空栈调用与 std::stack 一样属于未定义行为。
    reference top() noexcept(noexcept(c.back()));

    /// 返回栈顶只读引用；空栈调用与 std::stack 一样属于未定义行为。
    const_reference top() const noexcept(noexcept(c.back()));

    /// 将 value 复制到栈顶；异常保证由底层 push_back 决定。
    void push(const value_type& value);

    /// 将 value 移动到栈顶；支持 move-only 元素。
    void push(value_type&& value);

    /// 在底层容器尾部原地构造栈顶元素。
    template<typename... Args>
    void emplace(Args&&... args);

    /// 删除栈顶元素但不返回值；空栈调用与 std::stack 一样属于未定义行为。
    void pop();

    /// 交换两个适配器的底层容器。
    void swap(Stack& other) noexcept(noexcept(c.swap(other.c)));

    template<typename U, typename OtherContainer>
    friend bool operator==(
        const Stack<U, OtherContainer>& left,
        const Stack<U, OtherContainer>& right
    );

    template<typename U, typename OtherContainer>
    friend bool operator!=(
        const Stack<U, OtherContainer>& left,
        const Stack<U, OtherContainer>& right
    );
};

/// 默认构造底层容器。
template<typename T, typename Container>
Stack<T, Container>::Stack() noexcept(noexcept(container_type()))
    : c() {
}

/// 复制指定底层容器。
template<typename T, typename Container>
Stack<T, Container>::Stack(const container_type& container)
    : c(container) {
}

/// 移动接管指定底层容器。
template<typename T, typename Container>
Stack<T, Container>::Stack(container_type&& container) noexcept(
    std::is_nothrow_move_constructible<container_type>::value
)
    : c(std::move(container)) {
}

/// 使用 allocator 构造底层容器。
template<typename T, typename Container>
template<typename Allocator>
Stack<T, Container>::Stack(
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(allocator) {
}

/// 使用 allocator 复制构造底层容器。
template<typename T, typename Container>
template<typename Allocator>
Stack<T, Container>::Stack(
    const container_type& container,
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(container, allocator) {
}

/// 使用 allocator 移动构造底层容器。
template<typename T, typename Container>
template<typename Allocator>
Stack<T, Container>::Stack(
    container_type&& container,
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(std::move(container), allocator) {
}

/// 使用指定 allocator 复制另一个适配器的底层容器。
template<typename T, typename Container>
template<typename Allocator>
Stack<T, Container>::Stack(
    const Stack& other,
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(other.c, allocator) {
}

/// 使用指定 allocator 移动另一个适配器的底层容器。
template<typename T, typename Container>
template<typename Allocator>
Stack<T, Container>::Stack(
    Stack&& other,
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(std::move(other.c), allocator) {
}

/// 判断栈是否为空。
template<typename T, typename Container>
bool Stack<T, Container>::empty() const noexcept(noexcept(c.empty())) {
    return c.empty();
}

/// 返回元素数量。
template<typename T, typename Container>
typename Stack<T, Container>::size_type
Stack<T, Container>::size() const noexcept(noexcept(c.size())) {
    return c.size();
}

/// 返回栈顶可写引用。
template<typename T, typename Container>
typename Stack<T, Container>::reference
Stack<T, Container>::top() noexcept(noexcept(c.back())) {
    return c.back();
}

/// 返回栈顶只读引用。
template<typename T, typename Container>
typename Stack<T, Container>::const_reference
Stack<T, Container>::top() const noexcept(noexcept(c.back())) {
    return c.back();
}

/// 复制压栈。
template<typename T, typename Container>
void Stack<T, Container>::push(const value_type& value) {
    c.push_back(value);
}

/// 移动压栈。
template<typename T, typename Container>
void Stack<T, Container>::push(value_type&& value) {
    c.push_back(std::move(value));
}

/// 原地构造栈顶元素。
template<typename T, typename Container>
template<typename... Args>
void Stack<T, Container>::emplace(Args&&... args) {
    c.emplace_back(std::forward<Args>(args)...);
}

/// 删除栈顶元素。
template<typename T, typename Container>
void Stack<T, Container>::pop() {
    c.pop_back();
}

/// 交换底层容器。
template<typename T, typename Container>
void Stack<T, Container>::swap(Stack& other) noexcept(noexcept(c.swap(other.c))) {
    c.swap(other.c);
}

/// 比较两个栈保存的底层序列是否相等。
template<typename T, typename Container>
bool operator==(
    const Stack<T, Container>& left,
    const Stack<T, Container>& right
) {
    return left.c == right.c;
}

/// 比较两个栈保存的底层序列是否不同。
template<typename T, typename Container>
bool operator!=(
    const Stack<T, Container>& left,
    const Stack<T, Container>& right
) {
    return !(left == right);
}

/// 通过成员 swap 交换两个栈适配器。
template<typename T, typename Container>
void swap(Stack<T, Container>& left, Stack<T, Container>& right)
    noexcept(noexcept(left.swap(right))) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
