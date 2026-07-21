#ifndef DSA_CONTAINER_HEAP_DETAIL_NODE_HEAP_H
#define DSA_CONTAINER_HEAP_DETAIL_NODE_HEAP_H

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "HeapPolicy.h"

namespace dsa {
namespace container {
namespace detail {

// 四类节点堆共用的 allocator-aware 生命周期容器；Policy 只负责结构状态机。
template<
    typename T,
    typename Compare,
    typename Allocator,
    template<typename, typename> class PolicyTemplate,
    bool ChildSibling
>
class NodeHeap {
private:
    typedef HeapNode<T> Node;
    typedef typename std::conditional<
        ChildSibling,
        ChildSiblingNodeAccess<Node>,
        BinaryNodeAccess<Node>
    >::type Access;
    typedef HeapHigher<T, Compare> Higher;
    typedef PolicyTemplate<Access, Higher> Policy;
    typedef typename Policy::State State;
    typedef std::allocator_traits<Allocator> allocator_traits;
    typedef typename allocator_traits::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    NodeHeap()
        : state_(), size_(0), compare_(), allocator_() {}

    explicit NodeHeap(
        const value_compare& compare,
        const allocator_type& allocator = allocator_type()
    ) : state_(), size_(0), compare_(compare), allocator_(allocator) {}

    explicit NodeHeap(const allocator_type& allocator)
        : state_(), size_(0), compare_(), allocator_(allocator) {}

    template<typename InputIt>
    NodeHeap(
        InputIt first,
        InputIt last,
        const value_compare& compare = value_compare(),
        const allocator_type& allocator = allocator_type()
    ) : state_(), size_(0), compare_(compare), allocator_(allocator) {
        try {
            for (; first != last; ++first)
                push(*first);
        } catch (...) {
            clear();
            throw;
        }
    }

    NodeHeap(const NodeHeap& other)
        : state_(), size_(0), compare_(other.compare_), allocator_(
              allocator_traits::select_on_container_copy_construction(other.get_allocator())
          ) {
        cloneValues(other);
    }

    NodeHeap(const NodeHeap& other, const allocator_type& allocator)
        : state_(), size_(0), compare_(other.compare_), allocator_(allocator) {
        cloneValues(other);
    }

    NodeHeap(NodeHeap&& other) noexcept(
        std::is_nothrow_move_constructible<value_compare>::value &&
        std::is_nothrow_move_constructible<node_allocator_type>::value
    ) : state_(), size_(0), compare_(std::move(other.compare_)),
        allocator_(std::move(other.allocator_)) {
        stealFrom(other);
    }

    NodeHeap(NodeHeap&& other, const allocator_type& allocator)
        : state_(), size_(0), compare_(std::move(other.compare_)), allocator_(allocator) {
        if (allocator_ == other.allocator_)
            stealFrom(other);
        else
            moveValuesFrom(other);
    }

    ~NodeHeap() { clear(); }

    NodeHeap& operator=(const NodeHeap& other) {
        if (this != &other)
            copyAssign(other, typename allocator_traits::propagate_on_container_copy_assignment());
        return *this;
    }

    NodeHeap& operator=(NodeHeap&& other) {
        if (this != &other)
            moveAssign(other, typename allocator_traits::propagate_on_container_move_assignment());
        return *this;
    }

    allocator_type get_allocator() const { return allocator_type(allocator_); }
    bool empty() const noexcept { return Policy::empty(state_); }
    size_type size() const noexcept { return size_; }

    const_reference top() const {
        if (empty())
            throw std::out_of_range("NodeHeap::top on empty heap");
        return Policy::best(state_)->value;
    }

    void push(const value_type& value) { emplace(value); }
    void push(value_type&& value) { emplace(std::move(value)); }

    template<typename... Args>
    void emplace(Args&&... args) {
        Node* node = createNode(std::forward<Args>(args)...);
        try {
            Policy::insert(state_, node, Higher(compare_));
            ++size_;
        } catch (...) {
            destroyNode(node);
            throw;
        }
    }

    void pop() {
        Node* removed = detachTop();
        destroyNode(removed);
    }

    value_type extract_top() {
        if (empty())
            throw std::out_of_range("NodeHeap::extract_top on empty heap");
        value_type result(std::move(Policy::best(state_)->value));
        Node* removed = detachTop();
        destroyNode(removed);
        return result;
    }

    void meld(NodeHeap& other) {
        if (this == &other || other.empty())
            return;
        if (allocator_ != other.allocator_)
            throw std::logic_error("NodeHeap::meld requires compatible allocators");
        Policy::meld(state_, other.state_, Higher(compare_));
        size_ += other.size_;
        other.size_ = 0;
    }

    void merge(NodeHeap& other) { meld(other); }

    void clear() noexcept {
        Policy::destroy(state_, [this](Node* node) { destroyNode(node); });
        size_ = 0;
    }

    void swap(NodeHeap& other) {
        typedef typename allocator_traits::propagate_on_container_swap Propagate;
        if (!Propagate::value && allocator_ != other.allocator_)
            throw std::logic_error("NodeHeap::swap requires compatible allocators");
        using std::swap;
        if (Propagate::value)
            swap(allocator_, other.allocator_);
        Policy::swapState(state_, other.state_);
        swap(size_, other.size_);
        swap(compare_, other.compare_);
    }

private:
    State state_;
    size_type size_;
    value_compare compare_;
    node_allocator_type allocator_;

    template<typename... Args>
    Node* createNode(Args&&... args) {
        Node* node = node_allocator_traits::allocate(allocator_, 1);
        try {
            node_allocator_traits::construct(allocator_, node, std::forward<Args>(args)...);
        } catch (...) {
            node_allocator_traits::deallocate(allocator_, node, 1);
            throw;
        }
        return node;
    }

    void destroyNode(Node* node) noexcept {
        node_allocator_traits::destroy(allocator_, node);
        node_allocator_traits::deallocate(allocator_, node, 1);
    }

    Node* detachTop() {
        if (empty())
            throw std::out_of_range("NodeHeap::pop on empty heap");
        Node* removed = Policy::detachBest(state_, size_, Higher(compare_));
        --size_;
        return removed;
    }

    void cloneValues(const NodeHeap& other) {
        try {
            State& source = const_cast<State&>(other.state_);
            Policy::forEach(source, [this](Node* node) { push(node->value); });
        } catch (...) {
            clear();
            throw;
        }
    }

    void moveValuesFrom(NodeHeap& other) {
        try {
            Policy::forEach(other.state_, [this](Node* node) {
                emplace(std::move(node->value));
            });
        } catch (...) {
            clear();
            throw;
        }
        other.clear();
    }

    void stealFrom(NodeHeap& other) noexcept {
        Policy::swapState(state_, other.state_);
        size_ = other.size_;
        other.size_ = 0;
    }

    void copyAssign(const NodeHeap& other, std::true_type) {
        NodeHeap replacement(other, other.get_allocator());
        clear();
        allocator_ = replacement.allocator_;
        compare_ = replacement.compare_;
        stealFrom(replacement);
    }

    void copyAssign(const NodeHeap& other, std::false_type) {
        NodeHeap replacement(other, get_allocator());
        swap(replacement);
    }

    void moveAssign(NodeHeap& other, std::true_type) {
        clear();
        allocator_ = std::move(other.allocator_);
        compare_ = std::move(other.compare_);
        stealFrom(other);
    }

    void moveAssign(NodeHeap& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            clear();
            compare_ = std::move(other.compare_);
            stealFrom(other);
            return;
        }
        NodeHeap replacement(std::move(other), get_allocator());
        swap(replacement);
    }
};

} // namespace detail
} // namespace container
} // namespace dsa

#endif
