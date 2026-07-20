#ifndef DSA_CONTAINER_QUEUE_QUEUE_H
#define DSA_CONTAINER_QUEUE_QUEUE_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "../list/List.h"

namespace dsa {
namespace container {

/// STL 风格队列容器适配器；自身不管理元素存储，只将 FIFO 语义映射到底层容器首尾。
template<typename T, typename Container = dsa::container::List<T> >
class Queue {
public:
    typedef Container container_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;

    static_assert(
        std::is_same<T, value_type>::value,
        "Queue<T, Container> requires T to match Container::value_type"
    );

protected:
    /// 保存实际元素；适配器只通过 front/back/push_back/pop_front 操作该容器。
    container_type c;

public:
    /// 默认构造底层容器。
    Queue() noexcept(noexcept(container_type()));

    /// 复制指定底层容器，首元素成为队头、尾元素成为队尾。
    explicit Queue(const container_type& container);

    /// 移动接管指定底层容器。
    explicit Queue(container_type&& container) noexcept(
        std::is_nothrow_move_constructible<container_type>::value
    );

    /// 使用 allocator 构造底层容器；仅在底层容器声明 uses_allocator 时参与重载。
    template<typename Allocator>
    explicit Queue(
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用 allocator 复制构造底层容器。
    template<typename Allocator>
    Queue(
        const container_type& container,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用 allocator 移动构造底层容器。
    template<typename Allocator>
    Queue(
        container_type&& container,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用指定 allocator 复制另一个适配器保存的底层容器。
    template<typename Allocator>
    Queue(
        const Queue& other,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 使用指定 allocator 移动另一个适配器保存的底层容器。
    template<typename Allocator>
    Queue(
        Queue&& other,
        const Allocator& allocator,
        typename std::enable_if<
            std::uses_allocator<container_type, Allocator>::value
        >::type* = nullptr
    );

    /// 判断队列是否为空，复杂度取决于底层容器，默认工业 List 为 O(1)。
    bool empty() const noexcept(noexcept(c.empty()));

    /// 返回元素数量，复杂度取决于底层容器，默认工业 List 为 O(1)。
    size_type size() const noexcept(noexcept(c.size()));

    /// 返回队头可写引用；空队列调用与 std::queue 一样属于未定义行为。
    reference front() noexcept(noexcept(c.front()));

    /// 返回队头只读引用；空队列调用与 std::queue 一样属于未定义行为。
    const_reference front() const noexcept(noexcept(c.front()));

    /// 返回队尾可写引用；空队列调用与 std::queue 一样属于未定义行为。
    reference back() noexcept(noexcept(c.back()));

    /// 返回队尾只读引用；空队列调用与 std::queue 一样属于未定义行为。
    const_reference back() const noexcept(noexcept(c.back()));

    /// 将 value 复制到队尾；异常保证由底层 push_back 决定。
    void push(const value_type& value);

    /// 将 value 移动到队尾；支持 move-only 元素。
    void push(value_type&& value);

    /// 在底层容器尾部原地构造队尾元素。
    template<typename... Args>
    void emplace(Args&&... args);

    /// 删除队头元素但不返回值；空队列调用与 std::queue 一样属于未定义行为。
    void pop();

    /// 交换两个适配器的底层容器。
    void swap(Queue& other) noexcept(noexcept(c.swap(other.c)));

    template<typename U, typename OtherContainer>
    friend bool operator==(
        const Queue<U, OtherContainer>& left,
        const Queue<U, OtherContainer>& right
    );

    template<typename U, typename OtherContainer>
    friend bool operator!=(
        const Queue<U, OtherContainer>& left,
        const Queue<U, OtherContainer>& right
    );
};

/// 默认构造底层容器。
template<typename T, typename Container>
Queue<T, Container>::Queue() noexcept(noexcept(container_type()))
    : c() {
}

/// 复制指定底层容器。
template<typename T, typename Container>
Queue<T, Container>::Queue(const container_type& container)
    : c(container) {
}

/// 移动接管指定底层容器。
template<typename T, typename Container>
Queue<T, Container>::Queue(container_type&& container) noexcept(
    std::is_nothrow_move_constructible<container_type>::value
)
    : c(std::move(container)) {
}

/// 使用 allocator 构造底层容器。
template<typename T, typename Container>
template<typename Allocator>
Queue<T, Container>::Queue(
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
Queue<T, Container>::Queue(
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
Queue<T, Container>::Queue(
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
Queue<T, Container>::Queue(
    const Queue& other,
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
Queue<T, Container>::Queue(
    Queue&& other,
    const Allocator& allocator,
    typename std::enable_if<
        std::uses_allocator<container_type, Allocator>::value
    >::type*
)
    : c(std::move(other.c), allocator) {
}

/// 判断队列是否为空。
template<typename T, typename Container>
bool Queue<T, Container>::empty() const noexcept(noexcept(c.empty())) {
    return c.empty();
}

/// 返回元素数量。
template<typename T, typename Container>
typename Queue<T, Container>::size_type
Queue<T, Container>::size() const noexcept(noexcept(c.size())) {
    return c.size();
}

/// 返回队头可写引用。
template<typename T, typename Container>
typename Queue<T, Container>::reference
Queue<T, Container>::front() noexcept(noexcept(c.front())) {
    return c.front();
}

/// 返回队头只读引用。
template<typename T, typename Container>
typename Queue<T, Container>::const_reference
Queue<T, Container>::front() const noexcept(noexcept(c.front())) {
    return c.front();
}

/// 返回队尾可写引用。
template<typename T, typename Container>
typename Queue<T, Container>::reference
Queue<T, Container>::back() noexcept(noexcept(c.back())) {
    return c.back();
}

/// 返回队尾只读引用。
template<typename T, typename Container>
typename Queue<T, Container>::const_reference
Queue<T, Container>::back() const noexcept(noexcept(c.back())) {
    return c.back();
}

/// 复制入队。
template<typename T, typename Container>
void Queue<T, Container>::push(const value_type& value) {
    c.push_back(value);
}

/// 移动入队。
template<typename T, typename Container>
void Queue<T, Container>::push(value_type&& value) {
    c.push_back(std::move(value));
}

/// 原地构造队尾元素。
template<typename T, typename Container>
template<typename... Args>
void Queue<T, Container>::emplace(Args&&... args) {
    c.emplace_back(std::forward<Args>(args)...);
}

/// 删除队头元素。
template<typename T, typename Container>
void Queue<T, Container>::pop() {
    c.pop_front();
}

/// 交换底层容器。
template<typename T, typename Container>
void Queue<T, Container>::swap(Queue& other) noexcept(noexcept(c.swap(other.c))) {
    c.swap(other.c);
}

/// 比较两个队列保存的底层序列是否相等。
template<typename T, typename Container>
bool operator==(
    const Queue<T, Container>& left,
    const Queue<T, Container>& right
) {
    return left.c == right.c;
}

/// 比较两个队列保存的底层序列是否不同。
template<typename T, typename Container>
bool operator!=(
    const Queue<T, Container>& left,
    const Queue<T, Container>& right
) {
    return !(left == right);
}

/// 通过成员 swap 交换两个队列适配器。
template<typename T, typename Container>
void swap(Queue<T, Container>& left, Queue<T, Container>& right)
    noexcept(noexcept(left.swap(right))) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
