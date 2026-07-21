#ifndef DSA_CONTAINER_TREE_SPLAY_H
#define DSA_CONTAINER_TREE_SPLAY_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <dsa/core/tree/SearchTreeAlgorithm.h>
#include <dsa/core/tree/SplayTreeAlgorithm.h>

namespace dsa {
namespace container {

// allocator-aware 伸展树集合。成功查找、插入和删除会把热点节点伸展到根。
template<
    typename T,
    typename Compare = std::less<T>,
    typename Allocator = std::allocator<T>
>
class SplayTree {
public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type& const_reference;
    typedef const value_type* const_pointer;

private:
    struct Node {
        value_type value;
        Node* parent;
        Node* left;
        Node* right;
        int height;

        template<typename U>
        Node(Node* parentValue, U&& valueValue)
            : value(std::forward<U>(valueValue)),
              parent(parentValue), left(0), right(0), height(0) {}
    };

    struct NodeAccess {
        typedef Node node_type;
        static Node* left(Node* node) { return node ? node->left : 0; }
        static const Node* left(const Node* node) { return node ? node->left : 0; }
        static Node* right(Node* node) { return node ? node->right : 0; }
        static const Node* right(const Node* node) { return node ? node->right : 0; }
        static Node* parent(Node* node) { return node ? node->parent : 0; }
        static const Node* parent(const Node* node) { return node ? node->parent : 0; }
        static value_type& value(Node* node) { return node->value; }
        static const value_type& value(const Node* node) { return node->value; }
        static Node*& leftRef(Node* node) { return node->left; }
        static Node*& rightRef(Node* node) { return node->right; }
        static Node*& parentRef(Node* node) { return node->parent; }
        static int height(const Node* node) { return node->height; }
        static int& heightRef(Node* node) { return node->height; }
    };

    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;
    typedef dsa::core::SearchTreeAlgorithm<NodeAccess> search_algorithm_type;
    typedef dsa::core::SplayTreeAlgorithm<NodeAccess> splay_algorithm_type;

public:
    class const_iterator {
        friend class SplayTree;
        const Node* node_;
        const SplayTree* owner_;

        const_iterator(const Node* node, const SplayTree* owner)
            : node_(node), owner_(owner) {}

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;

        const_iterator() : node_(0), owner_(0) {}

        reference operator*() const { return node_->value; }
        pointer operator->() const { return &node_->value; }

        const_iterator& operator++() {
            node_ = search_algorithm_type::successor(node_);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator copy(*this);
            ++(*this);
            return copy;
        }

        const_iterator& operator--() {
            if (!node_) {
                if (!owner_)
                    throw std::out_of_range("SplayTree iterator has no owner");
                node_ = search_algorithm_type::maximum(owner_->root_);
            } else {
                node_ = search_algorithm_type::predecessor(node_);
            }
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator copy(*this);
            --(*this);
            return copy;
        }

        friend bool operator==(const const_iterator& left, const const_iterator& right) {
            return left.node_ == right.node_ && left.owner_ == right.owner_;
        }

        friend bool operator!=(const const_iterator& left, const const_iterator& right) {
            return !(left == right);
        }
    };

    typedef const_iterator iterator;

private:
    allocator_type allocator_;
    value_compare compare_;
    Node* root_;
    size_type size_;

    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    template<typename U>
    Node* createNode(Node* parent, U&& value) {
        node_allocator_type allocator = nodeAllocator();
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(
                allocator, node, parent, std::forward<U>(value)
            );
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

    void destroySubtree(Node* root) {
        if (!root)
            return;
        destroySubtree(root->left);
        destroySubtree(root->right);
        destroyNode(root);
    }

    bool equivalent(const value_type& left, const value_type& right) const {
        return !compare_(left, right) && !compare_(right, left);
    }

    const Node* findNode(const value_type& value) const {
        const Node* node = root_;
        while (node) {
            if (compare_(value, node->value))
                node = node->left;
            else if (compare_(node->value, value))
                node = node->right;
            else
                return node;
        }
        return 0;
    }

    Node* findNodeAndSplay(const value_type& value) {
        Node* node = splay_algorithm_type::searchAndSplay(root_, value, compare_);
        return node && equivalent(node->value, value) ? node : 0;
    }

    template<typename U>
    std::pair<iterator, bool> insertValue(U&& value) {
        if (!root_) {
            root_ = createNode(0, std::forward<U>(value));
            size_ = 1;
            return std::make_pair(iterator(root_, this), true);
        }

        const value_type& lookup = value;
        Node* hot = splay_algorithm_type::searchAndSplay(root_, lookup, compare_);
        if (hot && equivalent(hot->value, lookup))
            return std::make_pair(iterator(hot, this), false);

        Node* created = createNode(0, std::forward<U>(value));
        if (compare_(hot->value, created->value)) {
            created->left = hot;
            created->right = hot->right;
            if (created->right)
                created->right->parent = created;
            hot->right = 0;
            hot->parent = created;
        } else {
            created->right = hot;
            created->left = hot->left;
            if (created->left)
                created->left->parent = created;
            hot->left = 0;
            hot->parent = created;
        }
        root_ = created;
        ++size_;
        search_algorithm_type::updateHeight(hot);
        search_algorithm_type::updateHeight(created);
        return std::make_pair(iterator(created, this), true);
    }

    void copyFrom(const SplayTree& other) {
        try {
            for (const_iterator it = other.begin(); it != other.end(); ++it)
                insert(*it);
        } catch (...) {
            clear();
            throw;
        }
    }

    void stealFrom(SplayTree& other) {
        root_ = other.root_;
        size_ = other.size_;
        other.root_ = 0;
        other.size_ = 0;
    }

    bool validateNode(
        const Node* node,
        const Node* parent,
        const value_type* lower,
        const value_type* upper,
        size_type& count
    ) const {
        if (!node)
            return true;
        if (node->parent != parent)
            return false;
        if (lower && !compare_(*lower, node->value))
            return false;
        if (upper && !compare_(node->value, *upper))
            return false;
        const int expected = 1 + std::max(
            node->left ? node->left->height : -1,
            node->right ? node->right->height : -1
        );
        if (node->height != expected)
            return false;
        ++count;
        return validateNode(node->left, node, lower, &node->value, count) &&
               validateNode(node->right, node, &node->value, upper, count);
    }

public:
    SplayTree()
        : allocator_(), compare_(), root_(0), size_(0) {}

    explicit SplayTree(
        const value_compare& compare,
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), compare_(compare), root_(0), size_(0) {}

    explicit SplayTree(const allocator_type& allocator)
        : allocator_(allocator), compare_(), root_(0), size_(0) {}

    template<typename InputIterator>
    SplayTree(
        InputIterator first,
        InputIterator last,
        const value_compare& compare = value_compare(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), compare_(compare), root_(0), size_(0) {
        try {
            insert(first, last);
        } catch (...) {
            clear();
            throw;
        }
    }

    SplayTree(const SplayTree& other)
        : allocator_(allocator_traits_type::select_on_container_copy_construction(
              other.allocator_)),
          compare_(other.compare_), root_(0), size_(0) {
        copyFrom(other);
    }

    SplayTree(const SplayTree& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(other.compare_), root_(0), size_(0) {
        copyFrom(other);
    }

    // 移动后源树保持可复用空状态；allocator 与比较器保留可用副本。
    SplayTree(SplayTree&& other)
        : allocator_(other.allocator_), compare_(other.compare_), root_(0), size_(0) {
        stealFrom(other);
    }

    SplayTree(SplayTree&& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(other.compare_), root_(0), size_(0) {
        if (allocator_ == other.allocator_)
            stealFrom(other);
        else {
            for (const_iterator it = other.begin(); it != other.end(); ++it)
                insert(*it);
            other.clear();
        }
    }

    ~SplayTree() { clear(); }

    SplayTree& operator=(const SplayTree& other) {
        if (this == &other)
            return *this;
        const bool propagate = allocator_traits_type::propagate_on_container_copy_assignment::value;
        allocator_type target = propagate ? other.allocator_ : allocator_;
        SplayTree copy(other, target);
        clear();
        if (propagate)
            allocator_ = other.allocator_;
        compare_ = other.compare_;
        stealFrom(copy);
        return *this;
    }

    SplayTree& operator=(SplayTree&& other) {
        if (this == &other)
            return *this;
        const bool propagate = allocator_traits_type::propagate_on_container_move_assignment::value;
        if (propagate || allocator_ == other.allocator_) {
            clear();
            if (propagate)
                allocator_ = other.allocator_;
            compare_ = other.compare_;
            stealFrom(other);
        } else {
            SplayTree moved(other.begin(), other.end(), other.compare_, allocator_);
            clear();
            compare_ = other.compare_;
            stealFrom(moved);
            other.clear();
        }
        return *this;
    }

    allocator_type get_allocator() const { return allocator_; }
    value_compare value_comp() const { return compare_; }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const value_type* root_value() const { return root_ ? &root_->value : 0; }

    iterator begin() const { return iterator(search_algorithm_type::minimum(root_), this); }
    iterator cbegin() const { return begin(); }
    iterator end() const { return iterator(0, this); }
    iterator cend() const { return end(); }

    std::pair<iterator, bool> insert(const value_type& value) { return insertValue(value); }
    std::pair<iterator, bool> insert(value_type&& value) { return insertValue(std::move(value)); }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first)
            insert(*first);
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type value(std::forward<Args>(args)...);
        return insert(std::move(value));
    }

    // 非 const 查找会伸展命中节点或最后访问节点。
    iterator find(const value_type& value) {
        Node* node = findNodeAndSplay(value);
        return iterator(node, this);
    }

    // const 查找不改变树形，便于只读并发场景使用。
    const_iterator find(const value_type& value) const {
        return const_iterator(findNode(value), this);
    }

    bool contains(const value_type& value) const { return findNode(value) != 0; }

    size_type erase(const value_type& value) {
        Node* target = findNodeAndSplay(value);
        if (!target)
            return 0;

        Node* left = target->left;
        Node* right = target->right;
        if (left)
            left->parent = 0;
        if (right)
            right->parent = 0;
        target->left = 0;
        target->right = 0;

        if (!right) {
            root_ = left;
        } else {
            root_ = right;
            Node* minimum = search_algorithm_type::minimum(right);
            root_ = splay_algorithm_type::splay(root_, minimum);
            root_->left = left;
            if (left)
                left->parent = root_;
            search_algorithm_type::updateHeight(root_);
        }

        destroyNode(target);
        --size_;
        if (root_)
            search_algorithm_type::updateHeightAbove(root_);
        return 1;
    }

    void clear() {
        destroySubtree(root_);
        root_ = 0;
        size_ = 0;
    }

    template<typename Result, typename Aggregate>
    Result range_aggregate(
        const value_type& low,
        const value_type& high,
        Result identity,
        Aggregate aggregate
    ) const {
        const Node* node = search_algorithm_type::lowerBound(root_, low, compare_);
        Result result = identity;
        while (node && !compare_(high, node->value)) {
            result = aggregate(result, node->value);
            node = search_algorithm_type::successor(node);
        }
        return result;
    }

    bool validate() const {
        size_type count = 0;
        return validateNode(root_, 0, 0, 0, count) && count == size_;
    }

    void swap(SplayTree& other) {
        using std::swap;
        const bool propagate = allocator_traits_type::propagate_on_container_swap::value;
        if (!propagate && allocator_ != other.allocator_)
            throw std::logic_error("SplayTree::swap requires equal allocators");
        if (propagate)
            swap(allocator_, other.allocator_);
        swap(compare_, other.compare_);
        swap(root_, other.root_);
        swap(size_, other.size_);
    }
};

template<typename T, typename Compare, typename Allocator>
void swap(
    SplayTree<T, Compare, Allocator>& left,
    SplayTree<T, Compare, Allocator>& right
) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
