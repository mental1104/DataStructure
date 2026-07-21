#ifndef DSA_CONTAINER_SKIPLIST_QUAD_LIST_H
#define DSA_CONTAINER_SKIPLIST_QUAD_LIST_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

namespace dsa {
namespace container {

// allocator-aware 单层四向链表。横向节点归本容器所有，above/below 仅为非拥有观察指针。
template<typename T, typename Allocator = std::allocator<T> >
class QuadList {
    struct BaseNode {
        BaseNode* previous;
        BaseNode* next;
        BaseNode* above;
        BaseNode* below;
        BaseNode() : previous(0), next(0), above(0), below(0) {}
    };

public:
    struct Node : BaseNode {
        T value;
        template<typename U>
        explicit Node(U&& input) : BaseNode(), value(std::forward<U>(input)) {}
    };

    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

private:
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

public:
    class iterator {
        friend class QuadList;
        BaseNode* node_;
        explicit iterator(BaseNode* node) : node_(node) {}
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;
        iterator() : node_(0) {}
        reference operator*() const { return static_cast<Node*>(node_)->value; }
        pointer operator->() const { return &static_cast<Node*>(node_)->value; }
        iterator& operator++() { node_ = node_->next; return *this; }
        iterator& operator--() { node_ = node_->previous; return *this; }
        friend bool operator==(const iterator& left, const iterator& right) { return left.node_ == right.node_; }
        friend bool operator!=(const iterator& left, const iterator& right) { return !(left == right); }
    };

private:
    allocator_type allocator_;
    BaseNode head_;
    BaseNode tail_;
    size_type size_;

    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    template<typename U>
    Node* createNode(U&& value) {
        node_allocator_type allocator = nodeAllocator();
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(allocator, node, std::forward<U>(value));
        } catch (...) {
            node_allocator_traits::deallocate(allocator, node, 1);
            throw;
        }
        return node;
    }

    void destroyNode(Node* node) {
        node_allocator_type allocator = nodeAllocator();
        node_allocator_traits::destroy(allocator, node);
        node_allocator_traits::deallocate(allocator, node, 1);
    }

    void initialize() {
        head_.next = &tail_;
        tail_.previous = &head_;
        head_.previous = 0;
        tail_.next = 0;
        head_.above = head_.below = tail_.above = tail_.below = 0;
    }

public:
    explicit QuadList(const allocator_type& allocator = allocator_type())
        : allocator_(allocator), head_(), tail_(), size_(0) { initialize(); }

    QuadList(const QuadList& other)
        : allocator_(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator_)),
          head_(), tail_(), size_(0) {
        initialize();
        for (BaseNode* node = other.head_.next; node != &other.tail_; node = node->next)
            insert_after(last_node(), static_cast<Node*>(node)->value);
    }

    ~QuadList() { clear(); }

    QuadList& operator=(const QuadList& other) {
        if (this != &other) {
            QuadList copy(other);
            swap(copy);
        }
        return *this;
    }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    iterator begin() { return iterator(head_.next); }
    iterator end() { return iterator(&tail_); }
    Node* first_node() const { return head_.next == &tail_ ? 0 : static_cast<Node*>(head_.next); }
    Node* last_node() const { return tail_.previous == &head_ ? 0 : static_cast<Node*>(tail_.previous); }

    template<typename U>
    Node* insert_after(Node* position, U&& value, Node* below = 0) {
        BaseNode* base = position ? static_cast<BaseNode*>(position) : &head_;
        Node* node = createNode(std::forward<U>(value));
        node->previous = base;
        node->next = base->next;
        base->next->previous = node;
        base->next = node;
        node->below = below;
        if (below)
            below->above = node;
        ++size_;
        return node;
    }

    T erase(Node* node) {
        T value(std::move(node->value));
        node->previous->next = node->next;
        node->next->previous = node->previous;
        if (node->above)
            node->above->below = 0;
        if (node->below)
            node->below->above = 0;
        destroyNode(node);
        --size_;
        return value;
    }

    void clear() {
        BaseNode* node = head_.next;
        while (node != &tail_) {
            BaseNode* next = node->next;
            Node* valueNode = static_cast<Node*>(node);
            if (valueNode->above)
                valueNode->above->below = 0;
            if (valueNode->below)
                valueNode->below->above = 0;
            destroyNode(valueNode);
            node = next;
        }
        size_ = 0;
        initialize();
    }

    bool validate() const {
        size_type count = 0;
        const BaseNode* previous = &head_;
        for (const BaseNode* node = head_.next; node != &tail_; node = node->next) {
            if (node->previous != previous)
                return false;
            if (node->below && node->below->above != node)
                return false;
            previous = node;
            ++count;
        }
        return tail_.previous == previous && count == size_;
    }

    void swap(QuadList& other) {
        if (this == &other)
            return;
        QuadList temporary(allocator_);
        while (!empty())
            temporary.insert_after(temporary.last_node(), erase(first_node()));
        while (!other.empty())
            insert_after(last_node(), other.erase(other.first_node()));
        while (!temporary.empty())
            other.insert_after(other.last_node(), temporary.erase(temporary.first_node()));
    }
};

} // namespace container
} // namespace dsa

#endif
