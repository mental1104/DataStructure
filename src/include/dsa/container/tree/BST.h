#ifndef DSA_CONTAINER_TREE_BST_H
#define DSA_CONTAINER_TREE_BST_H

#include <dsa/core/tree/SearchTreeAlgorithm.h>

#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace dsa {
namespace container {
namespace detail {

struct UnbalancedTreeTag {};
struct AvlTreeTag {};
struct RedBlackTreeTag {};

enum class SearchTreeColor { red, black };

template<typename T, typename Compare, typename Allocator, typename BalanceTag>
class BasicSearchTree {
public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type& const_reference;
    typedef const value_type* const_pointer;

    class Node {
        friend class BasicSearchTree;
        value_type value_;
        Node* parent_;
        Node* left_;
        Node* right_;
        int height_;
        SearchTreeColor color_;

    public:
        template<typename... Args>
        explicit Node(Node* parent, Args&&... args)
            : value_(std::forward<Args>(args)...), parent_(parent), left_(nullptr),
              right_(nullptr), height_(0), color_(SearchTreeColor::red) {}

        const_reference value() const { return value_; }
        const Node* parent() const { return parent_; }
        const Node* left() const { return left_; }
        const Node* right() const { return right_; }
        int height() const { return height_; }
        bool is_red() const { return color_ == SearchTreeColor::red; }
        bool is_black() const { return color_ == SearchTreeColor::black; }
    };

private:
    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

    struct NodeAccess {
        typedef Node node_type;
        static Node* left(Node* node) { return node ? node->left_ : nullptr; }
        static const Node* left(const Node* node) { return node ? node->left_ : nullptr; }
        static Node*& leftRef(Node* node) { return node->left_; }
        static Node* right(Node* node) { return node ? node->right_ : nullptr; }
        static const Node* right(const Node* node) { return node ? node->right_ : nullptr; }
        static Node*& rightRef(Node* node) { return node->right_; }
        static Node* parent(Node* node) { return node ? node->parent_ : nullptr; }
        static const Node* parent(const Node* node) { return node ? node->parent_ : nullptr; }
        static Node*& parentRef(Node* node) { return node->parent_; }
        static value_type& value(Node* node) { return node->value_; }
        static const value_type& value(const Node* node) { return node->value_; }
        static int height(const Node* node) { return node->height_; }
        static int& heightRef(Node* node) { return node->height_; }
    };

    typedef dsa::core::SearchTreeAlgorithm<NodeAccess> algorithm_type;

public:
    class const_iterator {
        friend class BasicSearchTree;
        Node* node_;
        const BasicSearchTree* owner_;
        const_iterator(Node* node, const BasicSearchTree* owner) : node_(node), owner_(owner) {}

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;
        const_iterator() : node_(nullptr), owner_(nullptr) {}
        reference operator*() const { return node_->value_; }
        pointer operator->() const { return &node_->value_; }
        const_iterator& operator++() { node_ = algorithm_type::successor(node_); return *this; }
        const_iterator operator++(int) { const_iterator old(*this); ++(*this); return old; }
        const_iterator& operator--() {
            node_ = node_ ? algorithm_type::predecessor(node_)
                          : (owner_ ? algorithm_type::maximum(owner_->root_) : nullptr);
            return *this;
        }
        const_iterator operator--(int) { const_iterator old(*this); --(*this); return old; }
        bool operator==(const const_iterator& other) const {
            return node_ == other.node_ && owner_ == other.owner_;
        }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
        const Node* node() const { return node_; }
    };

    typedef const_iterator iterator;

private:
    allocator_type allocator_;
    value_compare compare_;
    Node* root_;
    size_type size_;

    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    template<typename... Args>
    Node* createNode(Node* parent, Args&&... args) {
        node_allocator_type allocator(nodeAllocator());
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(allocator, node, parent, std::forward<Args>(args)...);
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
        if (!root)
            return;
        Node* boundary = root->parent_;
        Node* previous = boundary;
        Node* current = root;
        while (current && current != boundary) {
            Node* next = nullptr;
            const bool descending = previous == current->parent_;
            const bool returning_from_left = previous == current->left_;
            if (descending && current->left_) {
                next = current->left_;
            } else if ((descending || returning_from_left) && current->right_) {
                next = current->right_;
            } else {
                next = current->parent_;
                Node* completed = current;
                previous = completed;
                current = next;
                destroyNode(completed);
                continue;
            }
            previous = current;
            current = next;
        }
    }

    static bool isRed(const Node* node) {
        return node && node->color_ == SearchTreeColor::red;
    }
    static bool isBlack(const Node* node) {
        return !node || node->color_ == SearchTreeColor::black;
    }
    static void setRed(Node* node) { if (node) node->color_ = SearchTreeColor::red; }
    static void setBlack(Node* node) { if (node) node->color_ = SearchTreeColor::black; }
    static int balanceFactor(const Node* node) {
        return node ? algorithm_type::nodeHeight(node->left_) -
                      algorithm_type::nodeHeight(node->right_) : 0;
    }

    Node* rotateLeft(Node* pivot) {
        Node* result = algorithm_type::rotateLeft(root_, pivot);
        algorithm_type::updateHeight(pivot);
        algorithm_type::updateHeight(result);
        return result;
    }
    Node* rotateRight(Node* pivot) {
        Node* result = algorithm_type::rotateRight(root_, pivot);
        algorithm_type::updateHeight(pivot);
        algorithm_type::updateHeight(result);
        return result;
    }
    Node* rebalanceAvlAt(Node* node) {
        if (balanceFactor(node) > 1) {
            if (balanceFactor(node->left_) < 0)
                rotateLeft(node->left_);
            return rotateRight(node);
        }
        if (balanceFactor(node) < -1) {
            if (balanceFactor(node->right_) > 0)
                rotateRight(node->right_);
            return rotateLeft(node);
        }
        return node;
    }

    void afterInsert(Node* node, UnbalancedTreeTag) {
        algorithm_type::updateHeightAbove(node->parent_);
        node->color_ = SearchTreeColor::black;
    }
    void afterInsert(Node* node, AvlTreeTag) {
        node->color_ = SearchTreeColor::black;
        Node* current = node->parent_;
        while (current) {
            algorithm_type::updateHeight(current);
            current = std::abs(balanceFactor(current)) > 1
                ? rebalanceAvlAt(current)->parent_ : current->parent_;
        }
    }
    void afterInsert(Node* node, RedBlackTreeTag) {
        Node* current = node;
        while (current != root_ && isRed(current->parent_)) {
            Node* parent = current->parent_;
            Node* grand = parent->parent_;
            if (parent == grand->left_) {
                Node* uncle = grand->right_;
                if (isRed(uncle)) {
                    setBlack(parent); setBlack(uncle); setRed(grand); current = grand;
                } else {
                    if (current == parent->right_) {
                        current = parent;
                        rotateLeft(current);
                        parent = current->parent_;
                        grand = parent->parent_;
                    }
                    setBlack(parent); setRed(grand); rotateRight(grand);
                }
            } else {
                Node* uncle = grand->left_;
                if (isRed(uncle)) {
                    setBlack(parent); setBlack(uncle); setRed(grand); current = grand;
                } else {
                    if (current == parent->left_) {
                        current = parent;
                        rotateRight(current);
                        parent = current->parent_;
                        grand = parent->parent_;
                    }
                    setBlack(parent); setRed(grand); rotateLeft(grand);
                }
            }
        }
        setBlack(root_);
        algorithm_type::updateHeightAbove(node);
    }

    void afterErase(const typename algorithm_type::EraseResult& result,
                    UnbalancedTreeTag, SearchTreeColor) {
        if (result.moved_node)
            algorithm_type::updateHeight(result.moved_node);
        algorithm_type::updateHeightAbove(result.rebalance_from);
    }
    void afterErase(const typename algorithm_type::EraseResult& result,
                    AvlTreeTag, SearchTreeColor) {
        Node* current = result.rebalance_from;
        while (current) {
            algorithm_type::updateHeight(current);
            current = std::abs(balanceFactor(current)) > 1
                ? rebalanceAvlAt(current)->parent_ : current->parent_;
        }
    }

    void fixDoubleBlack(Node* node, Node* parent) {
        while (node != root_ && isBlack(node)) {
            if (!parent)
                break;
            if (node == parent->left_) {
                Node* sibling = parent->right_;
                if (isRed(sibling)) {
                    setBlack(sibling); setRed(parent); rotateLeft(parent);
                    sibling = parent->right_;
                }
                if (!sibling) {
                    node = parent; parent = node->parent_; continue;
                }
                if (isBlack(sibling->left_) && isBlack(sibling->right_)) {
                    setRed(sibling); node = parent; parent = node->parent_;
                } else {
                    if (isBlack(sibling->right_)) {
                        setBlack(sibling->left_); setRed(sibling); rotateRight(sibling);
                        sibling = parent->right_;
                    }
                    sibling->color_ = parent->color_;
                    setBlack(parent); setBlack(sibling->right_); rotateLeft(parent);
                    node = root_; parent = nullptr;
                }
            } else {
                Node* sibling = parent->left_;
                if (isRed(sibling)) {
                    setBlack(sibling); setRed(parent); rotateRight(parent);
                    sibling = parent->left_;
                }
                if (!sibling) {
                    node = parent; parent = node->parent_; continue;
                }
                if (isBlack(sibling->left_) && isBlack(sibling->right_)) {
                    setRed(sibling); node = parent; parent = node->parent_;
                } else {
                    if (isBlack(sibling->left_)) {
                        setBlack(sibling->right_); setRed(sibling); rotateLeft(sibling);
                        sibling = parent->left_;
                    }
                    sibling->color_ = parent->color_;
                    setBlack(parent); setBlack(sibling->left_); rotateRight(parent);
                    node = root_; parent = nullptr;
                }
            }
        }
        setBlack(node);
    }

    void afterErase(const typename algorithm_type::EraseResult& result,
                    RedBlackTreeTag, SearchTreeColor removed_color) {
        if (removed_color == SearchTreeColor::black)
            fixDoubleBlack(result.fix_node, result.fix_parent);
        setBlack(root_);
        if (result.moved_node)
            algorithm_type::updateHeight(result.moved_node);
        algorithm_type::updateHeightAbove(result.fix_node ? result.fix_node : result.fix_parent);
    }

    template<typename U>
    std::pair<iterator, bool> insertValue(U&& value) {
        Node* hot = nullptr;
        Node*& slot = algorithm_type::searchSlot(root_, hot, value, compare_);
        if (slot)
            return std::make_pair(iterator(slot, this), false);
        Node* created = createNode(hot, std::forward<U>(value));
        slot = created;
        ++size_;
        afterInsert(created, BalanceTag());
        return std::make_pair(iterator(created, this), true);
    }

    void copyFrom(const BasicSearchTree& other) {
        for (const_iterator current = other.begin(); current != other.end(); ++current)
            insert(*current);
    }
    void stealFrom(BasicSearchTree& other) {
        root_ = other.root_; size_ = other.size_;
        other.root_ = nullptr; other.size_ = 0;
    }
    void swapRoots(BasicSearchTree& other) {
        using std::swap;
        swap(root_, other.root_); swap(size_, other.size_); swap(compare_, other.compare_);
    }
    void copyAssign(const BasicSearchTree& other, std::true_type) {
        BasicSearchTree temporary(other, other.allocator_);
        clear();
        allocator_ = other.allocator_;
        compare_ = std::move(temporary.compare_);
        root_ = temporary.root_; size_ = temporary.size_;
        temporary.root_ = nullptr; temporary.size_ = 0;
    }
    void copyAssign(const BasicSearchTree& other, std::false_type) {
        BasicSearchTree temporary(other, allocator_); swapRoots(temporary);
    }
    void moveAssign(BasicSearchTree& other, std::true_type) {
        clear(); allocator_ = std::move(other.allocator_);
        compare_ = std::move(other.compare_); stealFrom(other);
    }
    void moveAssign(BasicSearchTree& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            clear(); compare_ = std::move(other.compare_); stealFrom(other); return;
        }
        BasicSearchTree temporary(other, allocator_); other.clear(); swapRoots(temporary);
    }
    void swapImpl(BasicSearchTree& other, std::true_type) {
        using std::swap; swap(allocator_, other.allocator_); swapRoots(other);
    }
    void swapImpl(BasicSearchTree& other, std::false_type) {
        if (!(allocator_ == other.allocator_))
            throw std::logic_error("cannot swap search trees with unequal allocators");
        swapRoots(other);
    }

    struct ValidationResult {
        bool valid; size_type count; int height; int black_height;
        ValidationResult(bool state = true, size_type nodes = 0,
                         int tree_height = -1, int black = 1)
            : valid(state), count(nodes), height(tree_height), black_height(black) {}
    };

    ValidationResult validateNode(const Node* node, const value_type* lower,
                                  const value_type* upper, const Node* parent) const {
        if (!node)
            return ValidationResult(true, 0, -1, 1);
        if (node->parent_ != parent || (lower && !compare_(*lower, node->value_)) ||
            (upper && !compare_(node->value_, *upper)))
            return ValidationResult(false);
        ValidationResult left = validateNode(node->left_, lower, &node->value_, node);
        ValidationResult right = validateNode(node->right_, &node->value_, upper, node);
        if (!left.valid || !right.valid)
            return ValidationResult(false);
        const int expected_height = 1 + (left.height > right.height ? left.height : right.height);
        if (node->height_ != expected_height)
            return ValidationResult(false);
        if (std::is_same<BalanceTag, AvlTreeTag>::value &&
            std::abs(left.height - right.height) > 1)
            return ValidationResult(false);
        if (std::is_same<BalanceTag, RedBlackTreeTag>::value) {
            if (isRed(node) && (isRed(node->left_) || isRed(node->right_)))
                return ValidationResult(false);
            if (left.black_height != right.black_height)
                return ValidationResult(false);
        }
        const int black_height = left.black_height +
            ((std::is_same<BalanceTag, RedBlackTreeTag>::value && isBlack(node)) ? 1 : 0);
        return ValidationResult(true, left.count + right.count + 1,
                                expected_height, black_height);
    }

public:
    BasicSearchTree() : allocator_(), compare_(), root_(nullptr), size_(0) {}
    explicit BasicSearchTree(const value_compare& compare,
                             const allocator_type& allocator = allocator_type())
        : allocator_(allocator), compare_(compare), root_(nullptr), size_(0) {}
    explicit BasicSearchTree(const allocator_type& allocator)
        : allocator_(allocator), compare_(), root_(nullptr), size_(0) {}

    template<typename InputIterator>
    BasicSearchTree(InputIterator first, InputIterator last,
                    const value_compare& compare = value_compare(),
                    const allocator_type& allocator = allocator_type())
        : allocator_(allocator), compare_(compare), root_(nullptr), size_(0) {
        try { insert(first, last); } catch (...) { clear(); throw; }
    }
    BasicSearchTree(const BasicSearchTree& other)
        : allocator_(allocator_traits_type::select_on_container_copy_construction(other.allocator_)),
          compare_(other.compare_), root_(nullptr), size_(0) {
        try { copyFrom(other); } catch (...) { clear(); throw; }
    }
    BasicSearchTree(const BasicSearchTree& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(other.compare_), root_(nullptr), size_(0) {
        try { copyFrom(other); } catch (...) { clear(); throw; }
    }
    BasicSearchTree(BasicSearchTree&& other)
        noexcept(std::is_nothrow_move_constructible<allocator_type>::value &&
                 std::is_nothrow_move_constructible<value_compare>::value)
        : allocator_(std::move(other.allocator_)), compare_(std::move(other.compare_)),
          root_(nullptr), size_(0) { stealFrom(other); }
    BasicSearchTree(BasicSearchTree&& other, const allocator_type& allocator)
        : allocator_(allocator), compare_(std::move(other.compare_)), root_(nullptr), size_(0) {
        if (allocator_ == other.allocator_) stealFrom(other);
        else { copyFrom(other); other.clear(); }
    }
    ~BasicSearchTree() { clear(); }

    BasicSearchTree& operator=(const BasicSearchTree& other) {
        if (this != &other)
            copyAssign(other, typename allocator_traits_type::propagate_on_container_copy_assignment());
        return *this;
    }
    BasicSearchTree& operator=(BasicSearchTree&& other) {
        if (this != &other)
            moveAssign(other, typename allocator_traits_type::propagate_on_container_move_assignment());
        return *this;
    }

    allocator_type get_allocator() const { return allocator_; }
    value_compare value_comp() const { return compare_; }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    int height() const { return root_ ? root_->height_ : -1; }
    const Node* root() const { return root_; }
    iterator begin() const { return iterator(algorithm_type::minimum(root_), this); }
    iterator cbegin() const { return begin(); }
    iterator end() const { return iterator(nullptr, this); }
    iterator cend() const { return end(); }

    std::pair<iterator, bool> insert(const value_type& value) { return insertValue(value); }
    std::pair<iterator, bool> insert(value_type&& value) { return insertValue(std::move(value)); }
    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first) insert(*first);
    }
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type value(std::forward<Args>(args)...); return insert(std::move(value));
    }
    iterator find(const value_type& value) const {
        return iterator(algorithm_type::find(root_, value, compare_), this);
    }
    bool contains(const value_type& value) const {
        return algorithm_type::find(root_, value, compare_) != nullptr;
    }
    iterator lower_bound(const value_type& value) const {
        return iterator(algorithm_type::lowerBound(root_, value, compare_), this);
    }
    iterator upper_bound(const value_type& value) const {
        return iterator(algorithm_type::upperBound(root_, value, compare_), this);
    }
    size_type erase(const value_type& value) {
        Node* target = algorithm_type::find(root_, value, compare_);
        if (!target) return 0;
        erase(iterator(target, this)); return 1;
    }
    iterator erase(iterator position) {
        if (position.owner_ != this || !position.node_)
            throw std::invalid_argument("erase requires an iterator owned by this tree");
        Node* target = position.node_;
        Node* next = algorithm_type::successor(target);
        Node* structural_removed = target;
        if (target->left_ && target->right_)
            structural_removed = algorithm_type::minimum(target->right_);
        const SearchTreeColor removed_color = structural_removed->color_;
        const SearchTreeColor target_color = target->color_;
        const int target_height = target->height_;
        typename algorithm_type::EraseResult result = algorithm_type::detach(root_, target);
        if (result.moved_node) {
            result.moved_node->color_ = target_color;
            result.moved_node->height_ = target_height;
        }
        destroyNode(result.removed);
        --size_;
        afterErase(result, BalanceTag(), removed_color);
        return iterator(next, this);
    }
    void clear() { destroySubtree(root_); root_ = nullptr; size_ = 0; }

    template<typename Result, typename Aggregate>
    Result range_aggregate(const value_type& lower, const value_type& upper,
                           Result identity, Aggregate aggregate) const {
        Result result = identity;
        for (iterator current = lower_bound(lower);
             current != end() && !compare_(upper, *current); ++current)
            result = aggregate(result, *current);
        return result;
    }
    bool validate() const {
        if (root_ && root_->parent_)
            return false;
        if (std::is_same<BalanceTag, RedBlackTreeTag>::value && isRed(root_))
            return false;
        ValidationResult result = validateNode(root_, nullptr, nullptr, nullptr);
        return result.valid && result.count == size_;
    }
    void swap(BasicSearchTree& other) {
        swapImpl(other, typename allocator_traits_type::propagate_on_container_swap());
    }
};

} // namespace detail

template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
using BST = detail::BasicSearchTree<T, Compare, Allocator, detail::UnbalancedTreeTag>;

template<typename T, typename Compare, typename Allocator, typename BalanceTag>
void swap(detail::BasicSearchTree<T, Compare, Allocator, BalanceTag>& left,
          detail::BasicSearchTree<T, Compare, Allocator, BalanceTag>& right) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
