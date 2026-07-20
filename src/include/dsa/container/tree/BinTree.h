#ifndef DSA_CONTAINER_TREE_BIN_TREE_H
#define DSA_CONTAINER_TREE_BIN_TREE_H

#include <dsa/core/tree/BinTreeAlgorithm.h>

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace dsa {
namespace container {

// allocator-aware 工业二叉树。
//
// 该类型没有对应的 std 容器，因而只提供明确的节点所有权、值迭代器和基础结构修改接口。
// 节点地址在自身未被删除时保持稳定；插入不会使既有节点或 iterator 失效。
template<typename T, typename Allocator = std::allocator<T> >
class BinTree {
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    class Node {
        friend class BinTree;

        value_type value_;
        Node* parent_;
        Node* left_;
        Node* right_;
        int height_;

    public:
        // 由容器 allocator 构造节点；parent 只建立向上的观察链接，不转移所有权。
        template<typename... Args>
        explicit Node(Node* parent, Args&&... args);

        // 返回节点存储值的可修改引用。
        reference value();

        // 返回节点存储值的只读引用。
        const_reference value() const;

        // 返回父节点；根节点返回 nullptr。
        Node* parent();
        const Node* parent() const;

        // 返回左孩子；空孩子返回 nullptr。
        Node* left();
        const Node* left() const;

        // 返回右孩子；空孩子返回 nullptr。
        Node* right();
        const Node* right() const;

        // 返回以叶子为 0 的节点高度。
        int height() const;
    };

private:
    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

    struct NodeAccess {
        typedef Node node_type;

        static Node* left(Node* node) { return node ? node->left_ : nullptr; }
        static const Node* left(const Node* node) { return node ? node->left_ : nullptr; }
        static Node* right(Node* node) { return node ? node->right_ : nullptr; }
        static const Node* right(const Node* node) { return node ? node->right_ : nullptr; }
        static Node* parent(Node* node) { return node ? node->parent_ : nullptr; }
        static const Node* parent(const Node* node) { return node ? node->parent_ : nullptr; }
        static reference value(Node* node) { return node->value_; }
        static const_reference value(const Node* node) { return node->value_; }
        static int& height(Node* node) { return node->height_; }
        static int height(const Node* node) { return node->height_; }
    };

    typedef dsa::core::BinTreeAlgorithm<NodeAccess> algorithm_type;

public:
    template<bool IsConst>
    class basic_iterator {
        friend class BinTree;
        template<bool>
        friend class basic_iterator;

        typedef typename std::conditional<IsConst, const Node*, Node*>::type node_pointer;
        node_pointer node_;

        // 仅容器和兼容 iterator 可以从裸节点构造。
        explicit basic_iterator(node_pointer node);

    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef typename std::conditional<IsConst, const T*, T*>::type pointer;
        typedef typename std::conditional<IsConst, const T&, T&>::type reference;

        // 构造尾后 iterator。
        basic_iterator();

        // 允许 iterator 隐式转换为 const_iterator，禁止反向转换。
        template<bool Enabled = IsConst, typename std::enable_if<Enabled, int>::type = 0>
        basic_iterator(const basic_iterator<false>& other);

        // 返回当前值引用；不得解引用 end iterator。
        reference operator*() const;

        // 返回当前值地址。
        pointer operator->() const;

        // 前置递增到中序后继。
        basic_iterator& operator++();

        // 后置递增并返回旧位置。
        basic_iterator operator++(int);

        // 比较两个 iterator 的节点身份。
        template<bool OtherConst>
        bool operator==(const basic_iterator<OtherConst>& other) const;

        // 比较两个 iterator 是否不同。
        template<bool OtherConst>
        bool operator!=(const basic_iterator<OtherConst>& other) const;

        // 返回 iterator 当前节点，供显式节点 API 使用。
        node_pointer node() const;
    };

    typedef basic_iterator<false> iterator;
    typedef basic_iterator<true> const_iterator;

private:
    allocator_type allocator_;
    Node* root_;
    size_type size_;

    // 使用当前 allocator 申请并构造一个尚未链接的节点。
    template<typename... Args>
    Node* createNode(Node* parent, Args&&... args);

    // 析构并释放一个已经脱离树结构的节点。
    void destroyNode(Node* node);

    // 以后序迭代算法销毁子树并返回释放节点数。
    size_type destroySubtree(Node* root);

    // 深拷贝来源子树；任一构造失败时回滚已创建节点。
    Node* cloneSubtree(const Node* source, Node* parent);

    // 使用移动构造复制树形；allocator 不相等的移动赋值使用该路径并提供 basic guarantee。
    Node* moveCloneSubtree(Node* source, Node* parent);

    // 判断节点是否由当前树拥有；复杂度为 O(height)。
    bool owns(const Node* node) const;

    // 校验父节点属于当前树。
    void requireOwned(const Node* node) const;

    // 交换根和 size，不交换 allocator。
    void swapRoots(BinTree& other);

    // 从空目标接管来源节点链。
    void stealFrom(BinTree& other);

    // 根据 allocator propagation 规则执行拷贝赋值。
    void copyAssign(const BinTree& other, std::true_type);
    void copyAssign(const BinTree& other, std::false_type);

    // 根据 allocator propagation 规则执行移动赋值。
    void moveAssign(BinTree& other, std::true_type);
    void moveAssign(BinTree& other, std::false_type);

    // 根据 allocator propagation 规则执行 swap。
    void swapImpl(BinTree& other, std::true_type);
    void swapImpl(BinTree& other, std::false_type);

public:
    // 使用默认 allocator 构造空树。
    BinTree();

    // 使用指定 allocator 构造空树。
    explicit BinTree(const allocator_type& allocator);

    // 深拷贝树形和节点值，allocator 采用 select_on_container_copy_construction。
    BinTree(const BinTree& other);

    // 使用指定 allocator 深拷贝来源树。
    BinTree(const BinTree& other, const allocator_type& allocator);

    // 移动构造并接管节点所有权。
    BinTree(BinTree&& other) noexcept(std::is_nothrow_move_constructible<allocator_type>::value);

    // 使用指定 allocator 移动构造；allocator 不等时逐节点移动。
    BinTree(BinTree&& other, const allocator_type& allocator);

    // 释放所有节点。
    ~BinTree();

    // 深拷贝赋值；传播规则由 allocator_traits 决定。
    BinTree& operator=(const BinTree& other);

    // 移动赋值；allocator 不可传播且不相等时逐节点移动。
    BinTree& operator=(BinTree&& other);

    // 返回构造该树所使用的 allocator。
    allocator_type get_allocator() const;

    // 返回节点数量。
    size_type size() const;

    // 判断树是否为空。
    bool empty() const;

    // 返回根节点可修改指针。
    Node* root();

    // 返回根节点只读指针。
    const Node* root() const;

    // 返回树高；空树返回 -1。
    int height() const;

    // 在空树中原位构造根节点；非空树调用会抛出 logic_error。
    template<typename... Args>
    Node* emplace_root(Args&&... args);

    // 拷贝值构造根节点。
    Node* insert_root(const value_type& value);

    // 移动值构造根节点。
    Node* insert_root(value_type&& value);

    // 在空左孩子位置原位构造节点；父节点必须属于当前树。
    template<typename... Args>
    Node* emplace_left(Node* parent, Args&&... args);

    // 在空右孩子位置原位构造节点；父节点必须属于当前树。
    template<typename... Args>
    Node* emplace_right(Node* parent, Args&&... args);

    // 删除指定节点及其整棵子树，返回删除节点数。
    size_type erase_subtree(Node* node);

    // 立即析构并释放全部节点。
    void clear();

    // 返回中序首元素 iterator。
    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    // 返回中序尾后 iterator。
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    // 以前序迭代顺序访问可修改值。
    template<typename Visitor>
    void traverse_preorder(Visitor&& visitor);

    // 以前序迭代顺序访问只读值。
    template<typename Visitor>
    void traverse_preorder(Visitor&& visitor) const;

    // 以中序迭代顺序访问可修改值。
    template<typename Visitor>
    void traverse_inorder(Visitor&& visitor);

    // 以中序迭代顺序访问只读值。
    template<typename Visitor>
    void traverse_inorder(Visitor&& visitor) const;

    // 以后序迭代顺序访问可修改值。
    template<typename Visitor>
    void traverse_postorder(Visitor&& visitor);

    // 以后序迭代顺序访问只读值。
    template<typename Visitor>
    void traverse_postorder(Visitor&& visitor) const;

    // 以层序迭代顺序访问可修改值。
    template<typename Visitor>
    void traverse_levelorder(Visitor&& visitor);

    // 以层序迭代顺序访问只读值。
    template<typename Visitor>
    void traverse_levelorder(Visitor&& visitor) const;

    // 按 allocator 规则交换两棵树；不可传播且 allocator 不等时抛出 logic_error。
    void swap(BinTree& other);
};

template<typename T, typename Allocator>
template<typename... Args>
BinTree<T, Allocator>::Node::Node(Node* parent, Args&&... args)
    : value_(std::forward<Args>(args)...),
      parent_(parent),
      left_(nullptr),
      right_(nullptr),
      height_(0) {}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::reference BinTree<T, Allocator>::Node::value() {
    return value_;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::const_reference BinTree<T, Allocator>::Node::value() const {
    return value_;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::parent() {
    return parent_;
}

template<typename T, typename Allocator>
const typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::parent() const {
    return parent_;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::left() {
    return left_;
}

template<typename T, typename Allocator>
const typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::left() const {
    return left_;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::right() {
    return right_;
}

template<typename T, typename Allocator>
const typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::Node::right() const {
    return right_;
}

template<typename T, typename Allocator>
int BinTree<T, Allocator>::Node::height() const {
    return height_;
}

template<typename T, typename Allocator>
template<bool IsConst>
BinTree<T, Allocator>::basic_iterator<IsConst>::basic_iterator(node_pointer node) : node_(node) {}

template<typename T, typename Allocator>
template<bool IsConst>
BinTree<T, Allocator>::basic_iterator<IsConst>::basic_iterator() : node_(nullptr) {}

template<typename T, typename Allocator>
template<bool IsConst>
template<bool Enabled, typename std::enable_if<Enabled, int>::type>
BinTree<T, Allocator>::basic_iterator<IsConst>::basic_iterator(const basic_iterator<false>& other)
    : node_(other.node_) {}

template<typename T, typename Allocator>
template<bool IsConst>
typename BinTree<T, Allocator>::template basic_iterator<IsConst>::reference
BinTree<T, Allocator>::basic_iterator<IsConst>::operator*() const {
    return node_->value_;
}

template<typename T, typename Allocator>
template<bool IsConst>
typename BinTree<T, Allocator>::template basic_iterator<IsConst>::pointer
BinTree<T, Allocator>::basic_iterator<IsConst>::operator->() const {
    return &node_->value_;
}

template<typename T, typename Allocator>
template<bool IsConst>
typename BinTree<T, Allocator>::template basic_iterator<IsConst>&
BinTree<T, Allocator>::basic_iterator<IsConst>::operator++() {
    node_ = algorithm_type::successor(node_);
    return *this;
}

template<typename T, typename Allocator>
template<bool IsConst>
typename BinTree<T, Allocator>::template basic_iterator<IsConst>
BinTree<T, Allocator>::basic_iterator<IsConst>::operator++(int) {
    basic_iterator previous(*this);
    ++(*this);
    return previous;
}

template<typename T, typename Allocator>
template<bool IsConst>
template<bool OtherConst>
bool BinTree<T, Allocator>::basic_iterator<IsConst>::operator==(
    const basic_iterator<OtherConst>& other
) const {
    return node_ == other.node_;
}

template<typename T, typename Allocator>
template<bool IsConst>
template<bool OtherConst>
bool BinTree<T, Allocator>::basic_iterator<IsConst>::operator!=(
    const basic_iterator<OtherConst>& other
) const {
    return !(*this == other);
}

template<typename T, typename Allocator>
template<bool IsConst>
typename BinTree<T, Allocator>::template basic_iterator<IsConst>::node_pointer
BinTree<T, Allocator>::basic_iterator<IsConst>::node() const {
    return node_;
}

template<typename T, typename Allocator>
template<typename... Args>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::createNode(Node* parent, Args&&... args) {
    node_allocator_type nodeAllocator(allocator_);
    Node* node = node_allocator_traits::allocate(nodeAllocator, 1);
    try {
        node_allocator_traits::construct(
            nodeAllocator,
            node,
            parent,
            std::forward<Args>(args)...
        );
    } catch (...) {
        node_allocator_traits::deallocate(nodeAllocator, node, 1);
        throw;
    }
    return node;
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::destroyNode(Node* node) {
    node_allocator_type nodeAllocator(allocator_);
    node_allocator_traits::destroy(nodeAllocator, node);
    node_allocator_traits::deallocate(nodeAllocator, node, 1);
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::size_type BinTree<T, Allocator>::destroySubtree(Node* root) {
    return algorithm_type::destroySubtree(root, [this](Node* node) {
        destroyNode(node);
    });
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::cloneSubtree(
    const Node* source,
    Node* parent
) {
    if (!source)
        return nullptr;

    Node* copy = createNode(parent, source->value_);
    copy->height_ = source->height_;
    try {
        copy->left_ = cloneSubtree(source->left_, copy);
        copy->right_ = cloneSubtree(source->right_, copy);
    } catch (...) {
        destroySubtree(copy);
        throw;
    }
    return copy;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::moveCloneSubtree(
    Node* source,
    Node* parent
) {
    if (!source)
        return nullptr;

    Node* copy = createNode(parent, std::move(source->value_));
    copy->height_ = source->height_;
    try {
        copy->left_ = moveCloneSubtree(source->left_, copy);
        copy->right_ = moveCloneSubtree(source->right_, copy);
    } catch (...) {
        destroySubtree(copy);
        throw;
    }
    return copy;
}

template<typename T, typename Allocator>
bool BinTree<T, Allocator>::owns(const Node* node) const {
    if (!node)
        return false;
    while (node->parent_)
        node = node->parent_;
    return node == root_;
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::requireOwned(const Node* node) const {
    if (!owns(node))
        throw std::invalid_argument("BinTree node does not belong to this tree");
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::swapRoots(BinTree& other) {
    using std::swap;
    swap(root_, other.root_);
    swap(size_, other.size_);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::stealFrom(BinTree& other) {
    root_ = other.root_;
    size_ = other.size_;
    other.root_ = nullptr;
    other.size_ = 0;
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::copyAssign(const BinTree& other, std::true_type) {
    allocator_type targetAllocator(other.allocator_);
    BinTree temporary(other, targetAllocator);
    clear();
    allocator_ = targetAllocator;
    stealFrom(temporary);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::copyAssign(const BinTree& other, std::false_type) {
    BinTree temporary(other, allocator_);
    swapRoots(temporary);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::moveAssign(BinTree& other, std::true_type) {
    clear();
    allocator_ = std::move(other.allocator_);
    stealFrom(other);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::moveAssign(BinTree& other, std::false_type) {
    if (allocator_ == other.allocator_) {
        clear();
        stealFrom(other);
        return;
    }

    BinTree temporary(std::move(other), allocator_);
    swapRoots(temporary);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::swapImpl(BinTree& other, std::true_type) {
    using std::swap;
    swap(allocator_, other.allocator_);
    swapRoots(other);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::swapImpl(BinTree& other, std::false_type) {
    if (!(allocator_ == other.allocator_))
        throw std::logic_error("BinTree cannot swap unequal non-propagating allocators");
    swapRoots(other);
}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree()
    : allocator_(), root_(nullptr), size_(0) {}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree(const allocator_type& allocator)
    : allocator_(allocator), root_(nullptr), size_(0) {}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree(const BinTree& other)
    : allocator_(allocator_traits_type::select_on_container_copy_construction(other.allocator_)),
      root_(nullptr),
      size_(0) {
    root_ = cloneSubtree(other.root_, nullptr);
    size_ = other.size_;
}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree(const BinTree& other, const allocator_type& allocator)
    : allocator_(allocator), root_(nullptr), size_(0) {
    root_ = cloneSubtree(other.root_, nullptr);
    size_ = other.size_;
}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree(BinTree&& other) noexcept(
    std::is_nothrow_move_constructible<allocator_type>::value
)
    : allocator_(std::move(other.allocator_)), root_(nullptr), size_(0) {
    stealFrom(other);
}

template<typename T, typename Allocator>
BinTree<T, Allocator>::BinTree(BinTree&& other, const allocator_type& allocator)
    : allocator_(allocator), root_(nullptr), size_(0) {
    if (allocator_ == other.allocator_) {
        stealFrom(other);
        return;
    }

    root_ = moveCloneSubtree(other.root_, nullptr);
    size_ = other.size_;
    other.clear();
}

template<typename T, typename Allocator>
BinTree<T, Allocator>::~BinTree() {
    clear();
}

template<typename T, typename Allocator>
BinTree<T, Allocator>& BinTree<T, Allocator>::operator=(const BinTree& other) {
    if (this != &other)
        copyAssign(other, typename allocator_traits_type::propagate_on_container_copy_assignment());
    return *this;
}

template<typename T, typename Allocator>
BinTree<T, Allocator>& BinTree<T, Allocator>::operator=(BinTree&& other) {
    if (this != &other)
        moveAssign(other, typename allocator_traits_type::propagate_on_container_move_assignment());
    return *this;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::allocator_type BinTree<T, Allocator>::get_allocator() const {
    return allocator_;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::size_type BinTree<T, Allocator>::size() const {
    return size_;
}

template<typename T, typename Allocator>
bool BinTree<T, Allocator>::empty() const {
    return root_ == nullptr;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::root() {
    return root_;
}

template<typename T, typename Allocator>
const typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::root() const {
    return root_;
}

template<typename T, typename Allocator>
int BinTree<T, Allocator>::height() const {
    return root_ ? root_->height_ : -1;
}

template<typename T, typename Allocator>
template<typename... Args>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::emplace_root(Args&&... args) {
    if (root_)
        throw std::logic_error("BinTree root already exists");

    Node* node = createNode(nullptr, std::forward<Args>(args)...);
    root_ = node;
    size_ = 1;
    return node;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::insert_root(const value_type& value) {
    return emplace_root(value);
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::insert_root(value_type&& value) {
    return emplace_root(std::move(value));
}

template<typename T, typename Allocator>
template<typename... Args>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::emplace_left(
    Node* parent,
    Args&&... args
) {
    requireOwned(parent);
    if (parent->left_)
        throw std::logic_error("BinTree left child already exists");

    Node* child = createNode(parent, std::forward<Args>(args)...);
    parent->left_ = child;
    ++size_;
    algorithm_type::updateHeightAbove(parent);
    return child;
}

template<typename T, typename Allocator>
template<typename... Args>
typename BinTree<T, Allocator>::Node* BinTree<T, Allocator>::emplace_right(
    Node* parent,
    Args&&... args
) {
    requireOwned(parent);
    if (parent->right_)
        throw std::logic_error("BinTree right child already exists");

    Node* child = createNode(parent, std::forward<Args>(args)...);
    parent->right_ = child;
    ++size_;
    algorithm_type::updateHeightAbove(parent);
    return child;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::size_type BinTree<T, Allocator>::erase_subtree(Node* node) {
    requireOwned(node);

    Node* parent = node->parent_;
    if (!parent) {
        root_ = nullptr;
    } else if (parent->left_ == node) {
        parent->left_ = nullptr;
    } else {
        parent->right_ = nullptr;
    }
    node->parent_ = nullptr;

    const size_type removed = destroySubtree(node);
    size_ -= removed;
    if (parent)
        algorithm_type::updateHeightAbove(parent);
    return removed;
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::clear() {
    if (root_)
        destroySubtree(root_);
    root_ = nullptr;
    size_ = 0;
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::iterator BinTree<T, Allocator>::begin() {
    return iterator(algorithm_type::leftmost(root_));
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::const_iterator BinTree<T, Allocator>::begin() const {
    return cbegin();
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::const_iterator BinTree<T, Allocator>::cbegin() const {
    return const_iterator(algorithm_type::leftmost(static_cast<const Node*>(root_)));
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::iterator BinTree<T, Allocator>::end() {
    return iterator();
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::const_iterator BinTree<T, Allocator>::end() const {
    return cend();
}

template<typename T, typename Allocator>
typename BinTree<T, Allocator>::const_iterator BinTree<T, Allocator>::cend() const {
    return const_iterator();
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_preorder(Visitor&& visitor) {
    algorithm_type::traversePreOrder(root_, visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_preorder(Visitor&& visitor) const {
    algorithm_type::traversePreOrder(static_cast<const Node*>(root_), visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_inorder(Visitor&& visitor) {
    algorithm_type::traverseInOrder(root_, visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_inorder(Visitor&& visitor) const {
    algorithm_type::traverseInOrder(static_cast<const Node*>(root_), visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_postorder(Visitor&& visitor) {
    algorithm_type::traversePostOrder(root_, visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_postorder(Visitor&& visitor) const {
    algorithm_type::traversePostOrder(static_cast<const Node*>(root_), visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_levelorder(Visitor&& visitor) {
    algorithm_type::traverseLevelOrder(root_, visitor);
}

template<typename T, typename Allocator>
template<typename Visitor>
void BinTree<T, Allocator>::traverse_levelorder(Visitor&& visitor) const {
    algorithm_type::traverseLevelOrder(static_cast<const Node*>(root_), visitor);
}

template<typename T, typename Allocator>
void BinTree<T, Allocator>::swap(BinTree& other) {
    if (this == &other)
        return;
    swapImpl(other, typename allocator_traits_type::propagate_on_container_swap());
}

// 按成员 swap 语义交换两棵工业二叉树。
template<typename T, typename Allocator>
void swap(BinTree<T, Allocator>& lhs, BinTree<T, Allocator>& rhs) {
    lhs.swap(rhs);
}

} // namespace container
} // namespace dsa

#endif
