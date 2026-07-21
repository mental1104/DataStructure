#ifndef DSA_CONTAINER_HEAP_HEAP_H
#define DSA_CONTAINER_HEAP_HEAP_H

#include <functional>
#include <stdexcept>
#include <utility>

#include <dsa/container/vector/Vector.h>
#include <dsa/core/heap/HeapAlgorithm.h>
#include "detail/NodeHeap.h"

namespace dsa {
namespace container {

// allocator-aware 完全二叉堆，默认 std::less 形成最大堆。
template<
    typename T,
    typename Compare = std::less<T>,
    typename Container = dsa::container::Vector<T>
>
class BinaryHeap {
public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Container container_type;
    typedef typename container_type::size_type size_type;
    typedef typename container_type::const_reference const_reference;

private:
    class Access {
    public:
        typedef typename BinaryHeap::size_type size_type;
        explicit Access(container_type& container) : container_(container) {}
        value_type& value(size_type index) { return container_[index]; }
        const value_type& value(size_type index) const { return container_[index]; }
        void swapAt(size_type first, size_type second) {
            using std::swap;
            swap(container_[first], container_[second]);
        }
    private:
        container_type& container_;
    };

    typedef detail::HeapHigher<value_type, value_compare> Higher;
    typedef dsa::core::BinaryHeapAlgorithm<Access, Higher> Algorithm;

public:
    BinaryHeap() : container_(), compare_() {}
    explicit BinaryHeap(const value_compare& compare) : container_(), compare_(compare) {}
    BinaryHeap(const value_compare& compare, const container_type& container)
        : container_(container), compare_(compare) { heapify(); }
    BinaryHeap(const value_compare& compare, container_type&& container)
        : container_(std::move(container)), compare_(compare) { heapify(); }

    template<typename InputIt>
    BinaryHeap(InputIt first, InputIt last, const value_compare& compare = value_compare())
        : container_(first, last), compare_(compare) { heapify(); }

    bool empty() const noexcept { return container_.empty(); }
    size_type size() const noexcept { return container_.size(); }

    const_reference top() const {
        if (empty())
            throw std::out_of_range("BinaryHeap::top on empty heap");
        return container_[0];
    }

    void push(const value_type& value) {
        container_.push_back(value);
        siftUpLast();
    }

    void push(value_type&& value) {
        container_.push_back(std::move(value));
        siftUpLast();
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        container_.emplace_back(std::forward<Args>(args)...);
        siftUpLast();
    }

    void pop() {
        if (empty())
            throw std::out_of_range("BinaryHeap::pop on empty heap");
        Access access(container_);
        access.swapAt(0, size() - 1);
        container_.pop_back();
        if (!empty())
            Algorithm::siftDown(access, size(), 0, Higher(compare_));
    }

    value_type extract_top() {
        if (empty())
            throw std::out_of_range("BinaryHeap::extract_top on empty heap");
        value_type result(std::move(container_[0]));
        pop();
        return result;
    }

    value_compare value_comp() const { return compare_; }

    void swap(BinaryHeap& other) {
        using std::swap;
        container_.swap(other.container_);
        swap(compare_, other.compare_);
    }

private:
    container_type container_;
    value_compare compare_;

    void heapify() {
        Access access(container_);
        Algorithm::heapify(access, size(), Higher(compare_));
    }

    void siftUpLast() {
        Access access(container_);
        Algorithm::siftUp(access, size() - 1, Higher(compare_));
    }
};

// 工业左式堆、斜堆、配对堆和斐波那契堆共享 NodeHeap 生命周期实现。
template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
using LeftistHeap = detail::NodeHeap<T, Compare, Allocator, detail::LeftistPolicy, false>;

template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
using SkewHeap = detail::NodeHeap<T, Compare, Allocator, detail::SkewPolicy, false>;

template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
using PairingHeap = detail::NodeHeap<T, Compare, Allocator, detail::PairingPolicy, true>;

template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
using FibonacciHeap = detail::NodeHeap<T, Compare, Allocator, detail::FibonacciPolicy, true>;

template<typename T, typename C, typename Container>
void swap(BinaryHeap<T, C, Container>& first, BinaryHeap<T, C, Container>& second) {
    first.swap(second);
}

template<typename T, typename C, typename A, template<typename, typename> class P, bool S>
void swap(
    detail::NodeHeap<T, C, A, P, S>& first,
    detail::NodeHeap<T, C, A, P, S>& second
) {
    first.swap(second);
}

} // namespace container
} // namespace dsa

#endif
