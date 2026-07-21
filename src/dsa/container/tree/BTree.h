#ifndef DSA_CONTAINER_TREE_B_TREE_H
#define DSA_CONTAINER_TREE_B_TREE_H

#include <dsa/core/tree/BTreeAlgorithm.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace dsa {
namespace container {

// allocator-aware 的唯一键 B-Tree。
// order 表示节点最大孩子数，最小合法值为 3；非根节点至少保持 ceil(order / 2) 个孩子。
// 插入和删除保持迭代器整体失效语义，因为节点内 vector 可能移动键值。
template<
    typename T,
    typename Compare = std::less<T>,
    typename Allocator = std::allocator<T>
>
class BTree {
public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type& const_reference;
    typedef const value_type* const_pointer;

    class Node {
        friend class BTree;

        typedef std::vector<value_type, allocator_type> key_container_type;
        typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<Node*>
            child_allocator_type;
        typedef std::vector<Node*, child_allocator_type> child_container_type;

        Node* parent_;
        key_container_type keys_;
        child_container_type children_;

    public:
        // 仅由 BTree 通过 rebound allocator 构造；公开构造用于 allocator_traits::construct。
        explicit Node(Node* parent, const allocator_type& allocator)
            : parent_(parent), keys_(allocator), children_(child_allocator_type(allocator)) {}

        const Node* parent() const { return parent_; }
        size_type key_count() const { return keys_.size(); }
        size_type child_count() const { return children_.size(); }
        const_reference key(size_type index) const { return keys_[index]; }
        const Node* child(size_type index) const { return children_[index]; }
        bool leaf() const { return children_.empty(); }
    };

private:
    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

    struct NodeAccess {
        typedef Node node_type;
        typedef value_type key_type;

        static size_type keyCount(const Node* node) { return node->keys_.size(); }
        static value_type& key(Node* node, size_type index) { return node->keys_[index]; }
        static const value_type& key(const Node* node, size_type index) {
            return node->keys_[index];
        }
        static size_type childCount(const Node* node) { return node->children_.size(); }
        static Node* child(Node* node, size_type index) { return node->children_[index]; }
        static const Node* child(const Node* node, size_type index) {
            return node->children_[index];
        }
        static Node* parent(Node* node) { return node ? node->parent_ : nullptr; }
        static const Node* parent(const Node* node) { return node ? node->parent_ : nullptr; }
    };

    typedef dsa::core::BTreeAlgorithm<NodeAccess> algorithm_type;

public:
    class const_iterator {
        friend class BTree;

        Node* node_;
        size_type index_;
        const BTree* owner_;

        const_iterator(Node* node, size_type index, const BTree* owner)
            : node_(node), index_(index), owner_(owner) {}

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;

        const_iterator() : node_(nullptr), index_(0), owner_(nullptr) {}

        reference operator*() const { return node_->keys_[index_]; }
        pointer operator->() const { return &node_->keys_[index_]; }

        const_iterator& operator++() {
            owner_->increment(*this);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator previous(*this);
            ++(*this);
            return previous;
        }

        const_iterator& operator--() {
            owner_->decrement(*this);
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator previous(*this);
            --(*this);
            return previous;
        }

        bool operator==(const const_iterator& other) const {
            return node_ == other.node_ && index_ == other.index_ && owner_ == other.owner_;
        }

        bool operator!=(const const_iterator& other) const { return !(*this == other); }

        const Node* node() const { return node_; }
        size_type index() const { return index_; }
    };

    typedef const_iterator iterator;

private:
    allocator_type allocator_;
    value_compare compare_;
    size_type order_;
    size_type size_;
    Node* root_;

    size_type maxKeys() const { return order_ - 1; }
    size_type minChildren() const { return (order_ + 1) / 2; }
    size_type minKeys() const { return minChildren() - 1; }

    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    Node* createNode(Node* parent) {
        node_allocator_type allocator(nodeAllocator());
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(allocator, node, parent, allocator_);
        } catch (...) {
            node_allocator_traits::deallocate(allocator, node, 1);
            throw;
        }
        return node;
    }

    void destroyNode(Node* node) {
        node_allocator_type allocator(nodeAllocator());
        node_allocator_traits::destroy(allocator, node);
        node_allocator_traits::deallocate(allocator, node, 1);
    }

    void destroySubtree(Node* root) {
        algorithm_type::destroySubtree(root, [this](Node* node) { destroyNode(node); });
    }

    bool equivalent(const value_type& left, const value_type& right) const {
        return algorithm_type::equivalent(left, right, compare_);
    }

    size_type childIndex(const Node* parent, const Node* child) const {
        return algorithm_type::childIndex(parent, child);
    }

    void increment(const_iterator& iterator_value) const {
        if (!iterator_value.node_)
            return;

        Node* node = iterator_value.node_;
        const size_type index = iterator_value.index_;
        if (!node->children_.empty()) {
            Node* successor = algorithm_type::leftmostLeaf(node->children_[index + 1]);
            iterator_value.node_ = successor;
            iterator_value.index_ = 0;
            return;
        }
        if (index + 1 < node->keys_.size()) {
            ++iterator_value.index_;
            return;
        }

        Node* child = node;
        Node* parent = node->parent_;
        while (parent) {
            const size_type position = childIndex(parent, child);
            if (position < parent->keys_.size()) {
                iterator_value.node_ = parent;
                iterator_value.index_ = position;
                return;
            }
            child = parent;
            parent = parent->parent_;
        }
        iterator_value.node_ = nullptr;
        iterator_value.index_ = 0;
    }

    void decrement(const_iterator& iterator_value) const {
        if (!iterator_value.node_) {
            Node* node = algorithm_type::rightmostLeaf(root_);
            iterator_value.node_ = node;
            iterator_value.index_ = node ? node->keys_.size() - 1 : 0;
            return;
        }

        Node* node = iterator_value.node_;
        const size_type index = iterator_value.index_;
        if (!node->children_.empty()) {
            Node* predecessor = algorithm_type::rightmostLeaf(node->children_[index]);
            iterator_value.node_ = predecessor;
            iterator_value.index_ = predecessor->keys_.size() - 1;
            return;
        }
        if (index > 0) {
            --iterator_value.index_;
            return;
        }

        Node* child = node;
        Node* parent = node->parent_;
        while (parent) {
            const size_type position = childIndex(parent, child);
            if (position > 0) {
                iterator_value.node_ = parent;
                iterator_value.index_ = position - 1;
                return;
            }
            child = parent;
            parent = parent->parent_;
        }
        iterator_value.node_ = nullptr;
        iterator_value.index_ = 0;
    }

    template<typename U>
    std::pair<iterator, bool> insertValue(U&& value) {
        if (!root_)
            root_ = createNode(nullptr);

        typename algorithm_type::SearchResult result =
            algorithm_type::search(root_, value, compare_);
        if (result.found)
            return std::make_pair(iterator(result.node, result.index, this), false);

        result.node->keys_.insert(
            result.node->keys_.begin() + static_cast<difference_type>(result.index),
            std::forward<U>(value)
        );
        ++size_;
        Node* inserted_node = result.node;
        size_type inserted_index = result.index;
        splitOverflow(result.node, inserted_node, inserted_index);
        return std::make_pair(iterator(inserted_node, inserted_index, this), true);
    }

    // 分裂所有上溢节点。若新插入键被提升，更新其最终 iterator 位置。
    void splitOverflow(Node* node, Node*& inserted_node, size_type& inserted_index) {
        while (node && node->keys_.size() > maxKeys()) {
            const size_type middle = node->keys_.size() / 2;
            value_type promoted(std::move(node->keys_[middle]));
            Node* right = createNode(node->parent_);
            try {
                right->keys_.reserve(node->keys_.size() - middle - 1);
                for (size_type index = middle + 1; index < node->keys_.size(); ++index)
                    right->keys_.push_back(std::move(node->keys_[index]));

                if (!node->children_.empty()) {
                    right->children_.reserve(node->children_.size() - middle - 1);
                    for (size_type index = middle + 1; index < node->children_.size(); ++index) {
                        Node* child = node->children_[index];
                        right->children_.push_back(child);
                        child->parent_ = right;
                    }
                }
            } catch (...) {
                destroyNode(right);
                throw;
            }

            const bool inserted_promoted = inserted_node == node && inserted_index == middle;
            const bool inserted_right = inserted_node == node && inserted_index > middle;
            if (inserted_right) {
                inserted_node = right;
                inserted_index -= middle + 1;
            }

            node->keys_.resize(middle);
            if (!node->children_.empty())
                node->children_.resize(middle + 1);

            Node* parent = node->parent_;
            if (!parent) {
                parent = createNode(nullptr);
                parent->keys_.push_back(std::move(promoted));
                parent->children_.push_back(node);
                parent->children_.push_back(right);
                node->parent_ = parent;
                right->parent_ = parent;
                root_ = parent;
                if (inserted_promoted) {
                    inserted_node = parent;
                    inserted_index = 0;
                }
                node = parent;
                continue;
            }

            const size_type position = childIndex(parent, node);
            parent->keys_.insert(
                parent->keys_.begin() + static_cast<difference_type>(position),
                std::move(promoted)
            );
            parent->children_.insert(
                parent->children_.begin() + static_cast<difference_type>(position + 1), right
            );
            right->parent_ = parent;
            if (inserted_promoted) {
                inserted_node = parent;
                inserted_index = position;
            }
            node = parent;
        }
    }

    void borrowFromLeft(Node* node, Node* left, Node* parent, size_type position) {
        node->keys_.insert(node->keys_.begin(), std::move(parent->keys_[position - 1]));
        parent->keys_[position - 1] = std::move(left->keys_.back());
        left->keys_.pop_back();
        if (!left->children_.empty()) {
            Node* child = left->children_.back();
            left->children_.pop_back();
            node->children_.insert(node->children_.begin(), child);
            child->parent_ = node;
        }
    }

    void borrowFromRight(Node* node, Node* right, Node* parent, size_type position) {
        node->keys_.push_back(std::move(parent->keys_[position]));
        parent->keys_[position] = std::move(right->keys_.front());
        right->keys_.erase(right->keys_.begin());
        if (!right->children_.empty()) {
            Node* child = right->children_.front();
            right->children_.erase(right->children_.begin());
            node->children_.push_back(child);
            child->parent_ = node;
        }
    }

    Node* mergeWithRight(Node* left, Node* right, Node* parent, size_type separator) {
        left->keys_.push_back(std::move(parent->keys_[separator]));
        for (size_type index = 0; index < right->keys_.size(); ++index)
            left->keys_.push_back(std::move(right->keys_[index]));
        for (size_type index = 0; index < right->children_.size(); ++index) {
            Node* child = right->children_[index];
            left->children_.push_back(child);
            child->parent_ = left;
        }
        parent->keys_.erase(parent->keys_.begin() + static_cast<difference_type>(separator));
        parent->children_.erase(
            parent->children_.begin() + static_cast<difference_type>(separator + 1)
        );
        right->children_.clear();
        destroyNode(right);
        return left;
    }

    void repairUnderflow(Node* node) {
        while (node && node != root_ && node->keys_.size() < minKeys()) {
            Node* parent = node->parent_;
            const size_type position = childIndex(parent, node);
            Node* left = position > 0 ? parent->children_[position - 1] : nullptr;
            Node* right = position + 1 < parent->children_.size()
                ? parent->children_[position + 1] : nullptr;

            if (left && left->keys_.size() > minKeys()) {
                borrowFromLeft(node, left, parent, position);
                return;
            }
            if (right && right->keys_.size() > minKeys()) {
                borrowFromRight(node, right, parent, position);
                return;
            }

            if (left) {
                mergeWithRight(left, node, parent, position - 1);
            } else if (right) {
                mergeWithRight(node, right, parent, position);
            }
            node = parent;
        }

        if (root_ && root_->keys_.empty()) {
            if (root_->children_.empty()) {
                destroyNode(root_);
                root_ = nullptr;
            } else {
                Node* old_root = root_;
                root_ = old_root->children_[0];
                root_->parent_ = nullptr;
                old_root->children_.clear();
                destroyNode(old_root);
            }
        }
    }

    void copyFrom(const BTree& other) {
        for (const_iterator current = other.begin(); current != other.end(); ++current)
            insert(*current);
    }

    void stealFrom(BTree& other) {
        order_ = other.order_;
        size_ = other.size_;
        root_ = other.root_;
        other.size_ = 0;
        other.root_ = nullptr;
    }

    void swapPayload(BTree& other) {
        using std::swap;
        swap(compare_, other.compare_);
        swap(order_, other.order_);
        swap(size_, other.size_);
        swap(root_, other.root_);
    }

    void copyAssign(const BTree& other, std::true_type) {
        BTree temporary(other, other.allocator_);
        clear();
        allocator_ = other.allocator_;
        compare_ = std::move(temporary.compare_);
        order_ = temporary.order_;
        size_ = temporary.size_;
        root_ = temporary.root_;
        temporary.size_ = 0;
        temporary.root_ = nullptr;
    }

    void copyAssign(const BTree& other, std::false_type) {
        BTree temporary(other, allocator_);
        swapPayload(temporary);
    }

    void moveAssign(BTree& other, std::true_type) {
        clear();
        allocator_ = std::move(other.allocator_);
        compare_ = std::move(other.compare_);
        stealFrom(other);
    }

    void moveAssign(BTree& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            clear();
            compare_ = std::move(other.compare_);
            stealFrom(other);
            return;
        }
        BTree temporary(other, allocator_);
        other.clear();
        swapPayload(temporary);
    }

    void swapImpl(BTree& other, std::true_type) {
        using std::swap;
        swap(allocator_, other.allocator_);
        swapPayload(other);
    }

    void swapImpl(BTree& other, std::false_type) {
        if (!(allocator_ == other.allocator_))
            throw std::logic_error("cannot swap B-Trees with unequal allocators");
        swapPayload(other);
    }

    struct ValidationResult {
        bool valid;
        size_type key_count;
        size_type leaf_depth;
        bool has_leaf_depth;

        ValidationResult(bool state = true, size_type count = 0)
            : valid(state), key_count(count), leaf_depth(0), has_leaf_depth(false) {}
    };

    ValidationResult validateNode(
        const Node* node,
        const value_type* lower,
        const value_type* upper,
        const Node* expected_parent,
        size_type depth,
        bool root
    ) const {
        if (!node)
            return ValidationResult(false);
        if (node->parent_ != expected_parent)
            return ValidationResult(false);
        if (!root && (node->keys_.size() < minKeys() || node->keys_.size() > maxKeys()))
            return ValidationResult(false);
        if (root && node->keys_.size() > maxKeys())
            return ValidationResult(false);
        if (!node->children_.empty() && node->children_.size() != node->keys_.size() + 1)
            return ValidationResult(false);

        for (size_type index = 0; index < node->keys_.size(); ++index) {
            if (index > 0 && !compare_(node->keys_[index - 1], node->keys_[index]))
                return ValidationResult(false);
            if (lower && !compare_(*lower, node->keys_[index]))
                return ValidationResult(false);
            if (upper && !compare_(node->keys_[index], *upper))
                return ValidationResult(false);
        }

        ValidationResult result(true, node->keys_.size());
        if (node->children_.empty()) {
            result.leaf_depth = depth;
            result.has_leaf_depth = true;
            return result;
        }

        for (size_type index = 0; index < node->children_.size(); ++index) {
            const value_type* child_lower = index == 0 ? lower : &node->keys_[index - 1];
            const value_type* child_upper = index == node->keys_.size() ? upper : &node->keys_[index];
            ValidationResult child = validateNode(
                node->children_[index], child_lower, child_upper, node, depth + 1, false
            );
            if (!child.valid)
                return ValidationResult(false);
            result.key_count += child.key_count;
            if (!result.has_leaf_depth) {
                result.leaf_depth = child.leaf_depth;
                result.has_leaf_depth = child.has_leaf_depth;
            } else if (child.has_leaf_depth && result.leaf_depth != child.leaf_depth) {
                return ValidationResult(false);
            }
        }
        return result;
    }

public:
    explicit BTree(
        size_type order = 4,
        const value_compare& compare = value_compare(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), compare_(compare), order_(order), size_(0), root_(nullptr) {
        if (order_ < 3)
            throw std::invalid_argument("BTree order must be at least 3");
    }

    explicit BTree(const allocator_type& allocator)
        : allocator_(allocator), compare_(), order_(4), size_(0), root_(nullptr) {}

    template<typename InputIterator>
    BTree(
        InputIterator first,
        InputIterator last,
        size_type order = 4,
        const value_compare& compare = value_compare(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), compare_(compare), order_(order), size_(0), root_(nullptr) {
        if (order_ < 3)
            throw std::invalid_argument("BTree order must be at least 3");
        try {
            insert(first, last);
        } catch (...) {
            clear();
            throw;
        }
    }

    BTree(const BTree& other)
        : allocator_(allocator_traits_type::select_on_container_copy_construction(other.allocator_)),
          compare_(other.compare_), order_(other.order_), size_(0), root_(nullptr) {
        try {
            copyFrom(other);
        } catch (...) {
            clear();
            throw;
        }
    }

    BTree(const BTree& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(other.compare_), order_(other.order_), size_(0), root_(nullptr) {
        try {
            copyFrom(other);
        } catch (...) {
            clear();
            throw;
        }
    }

    BTree(BTree&& other)
        noexcept(std::is_nothrow_move_constructible<allocator_type>::value &&
                 std::is_nothrow_move_constructible<value_compare>::value)
        : allocator_(std::move(other.allocator_)), compare_(std::move(other.compare_)),
          order_(other.order_), size_(0), root_(nullptr) {
        stealFrom(other);
    }

    BTree(BTree&& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(std::move(other.compare_)), order_(other.order_),
          size_(0), root_(nullptr) {
        if (allocator_ == other.allocator_) {
            stealFrom(other);
        } else {
            copyFrom(other);
            other.clear();
        }
    }

    ~BTree() { clear(); }

    BTree& operator=(const BTree& other) {
        if (this != &other)
            copyAssign(other, typename allocator_traits_type::propagate_on_container_copy_assignment());
        return *this;
    }

    BTree& operator=(BTree&& other) {
        if (this != &other)
            moveAssign(other, typename allocator_traits_type::propagate_on_container_move_assignment());
        return *this;
    }

    allocator_type get_allocator() const { return allocator_; }
    value_compare value_comp() const { return compare_; }
    size_type order() const { return order_; }
    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }
    const Node* root() const { return root_; }

    iterator begin() const {
        Node* node = algorithm_type::leftmostLeaf(root_);
        return node ? iterator(node, 0, this) : end();
    }
    iterator cbegin() const { return begin(); }
    iterator end() const { return iterator(nullptr, 0, this); }
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

    iterator find(const value_type& value) const {
        if (!root_)
            return end();
        typename algorithm_type::SearchResult result = algorithm_type::search(root_, value, compare_);
        return result.found ? iterator(result.node, result.index, this) : end();
    }

    bool contains(const value_type& value) const { return find(value) != end(); }

    iterator lower_bound(const value_type& value) const {
        Node* current = root_;
        Node* candidate = nullptr;
        size_type candidate_index = 0;
        while (current) {
            const size_type index = algorithm_type::lowerBound(current, value, compare_);
            if (index < current->keys_.size()) {
                candidate = current;
                candidate_index = index;
            }
            if (current->children_.empty())
                break;
            current = current->children_[index];
        }
        return candidate ? iterator(candidate, candidate_index, this) : end();
    }

    iterator upper_bound(const value_type& value) const {
        iterator current = lower_bound(value);
        if (current != end() && equivalent(*current, value))
            ++current;
        return current;
    }

    size_type erase(const value_type& value) {
        if (!root_)
            return 0;
        typename algorithm_type::SearchResult result = algorithm_type::search(root_, value, compare_);
        if (!result.found)
            return 0;

        Node* node = result.node;
        size_type index = result.index;
        if (!node->children_.empty()) {
            Node* successor = algorithm_type::leftmostLeaf(node->children_[index + 1]);
            node->keys_[index] = std::move(successor->keys_[0]);
            node = successor;
            index = 0;
        }

        node->keys_.erase(node->keys_.begin() + static_cast<difference_type>(index));
        --size_;
        repairUnderflow(node);
        return 1;
    }

    void clear() {
        destroySubtree(root_);
        root_ = nullptr;
        size_ = 0;
    }

    template<typename Result, typename Aggregate>
    Result range_aggregate(
        const value_type& lower,
        const value_type& upper,
        Result identity,
        Aggregate aggregate
    ) const {
        Result result = identity;
        for (iterator current = lower_bound(lower);
             current != end() && !compare_(upper, *current); ++current)
            result = aggregate(result, *current);
        return result;
    }

    bool validate() const {
        if (!root_)
            return size_ == 0;
        if (root_->parent_)
            return false;
        if (size_ > 0 && root_->keys_.empty())
            return false;
        ValidationResult result = validateNode(root_, nullptr, nullptr, nullptr, 0, true);
        return result.valid && result.key_count == size_;
    }

    void swap(BTree& other) {
        swapImpl(other, typename allocator_traits_type::propagate_on_container_swap());
    }
};

template<typename T, typename Compare, typename Allocator>
void swap(BTree<T, Compare, Allocator>& left, BTree<T, Compare, Allocator>& right) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
