#ifndef DSA_BT_BIN_TREE_H
#define DSA_BT_BIN_TREE_H

#include "BinNode.h"

#include <cstddef>
#include <iterator>
#include <stdexcept>

template<typename T>
class BinTree {
protected:
    int _size{0};
    BinNode<T>* _root{nullptr};

    // 通过共享算法按左右子树高度刷新节点高度；派生平衡树仍可覆盖该策略。
    virtual int updateHeight(BinNode<T>* x);

    // 从指定节点向根方向更新高度。
    void updateHeightAbove(BinNode<T>* x);

    // 返回父节点指向当前节点的链接引用；根节点返回 _root。
    BinNode<T>*& FromParentTo(const BinNode<T>& node);

    // 兼容旧 protected helper；实际区间聚合由共享迭代算法完成。
    template<typename Res, typename Agg>
    Res rangeAggregateRec(BinNode<T>* x, const T& lo, const T& hi, Res acc, Agg&& agg) const;

public:
    struct iterator;

    // 构造空的教学二叉树。
    BinTree();

    // 迭代销毁全部节点；声明为 virtual 以匹配类已有的多态接口。
    virtual ~BinTree();

    // 返回教学树维护的节点数量。
    int size() const;

    // 判断树是否没有根节点。
    bool empty() const;

    // 返回可修改根节点指针。
    BinNode<T>* root();

    // 返回只读根节点指针。
    const BinNode<T>* root() const;

    // 插入新根；已有树会被完整释放，最终 size 为 1。
    BinNode<T>* insertAsRoot(T const& e);

    // 插入或替换左子树，并同步修正 size 与高度。
    BinNode<T>* insertAsLC(BinNode<T>* x, T const& e);

    // 插入或替换右子树，并同步修正 size 与高度。
    BinNode<T>* insertAsRC(BinNode<T>* x, T const& e);

    // 将来源树转移为左子树；来源树对象按旧 API 约定被释放并置空。
    BinNode<T>* attachAsLC(BinNode<T>* x, BinTree<T>*& source);

    // 将来源树转移为右子树；来源树对象按旧 API 约定被释放并置空。
    BinNode<T>* attachAsRC(BinNode<T>* x, BinTree<T>*& source);

    // 删除指定节点及其整棵子树，返回删除节点数。
    int remove(BinNode<T>* x);

    // 将指定子树从当前树分离为新的教学 BinTree。
    BinTree<T>* secede(BinNode<T>* x);

    // 返回指向中序首节点的教学 iterator。
    iterator begin();

    // 返回中序尾后 iterator。
    iterator end();

    // 保留旧签名：const 树仍返回教学 iterator 值。
    const iterator begin() const;

    // 保留旧签名：const 树仍返回教学 iterator 尾后值。
    const iterator end() const;

    // 对按 BST 顺序组织的树执行闭区间聚合。
    template<typename Res, typename Agg>
    Res rangeAggregate(const T& lo, const T& hi, Res identity, Agg&& agg) const;

    // 通过共享算法执行迭代层序遍历。
    template<typename VST>
    void travLevel(VST&& visit);

    // 通过共享算法执行迭代前序遍历。
    template<typename VST>
    void travPre(VST&& visit);

    // 通过共享算法执行迭代中序遍历。
    template<typename VST>
    void travIn(VST&& visit);

    // 通过共享算法执行迭代后序遍历。
    template<typename VST>
    void travPost(VST&& visit);

    // 教学版树比较保持历史语义：比较根节点存储值。
    bool operator<(const BinTree<T>& tree) const;
    bool operator>(const BinTree<T>& tree) const;

    // 教学版树相等保持历史语义：仅判断非空树是否共享同一根节点。
    bool operator==(const BinTree<T>& tree) const;
    bool operator!=(const BinTree<T>& tree) const;
};

// 兼容旧 helper：迭代释放以 root 为根的整棵教学子树。
template<typename T>
static void levelRemove(BinNode<T>* root) {
    removeAt(root);
}

template<typename T>
BinTree<T>::BinTree() = default;

template<typename T>
BinTree<T>::~BinTree() {
    if (_root)
        removeAt(_root);
    _root = nullptr;
    _size = 0;
}

template<typename T>
int BinTree<T>::size() const {
    return _size;
}

template<typename T>
bool BinTree<T>::empty() const {
    return !_root;
}

template<typename T>
BinNode<T>* BinTree<T>::root() {
    return _root;
}

template<typename T>
const BinNode<T>* BinTree<T>::root() const {
    return _root;
}

template<typename T>
int BinTree<T>::updateHeight(BinNode<T>* x) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    return dsa::core::BinTreeAlgorithm<Access>::updateHeight(x);
}

template<typename T>
void BinTree<T>::updateHeightAbove(BinNode<T>* x) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::updateHeightAbove(x);
}

template<typename T>
BinNode<T>* BinTree<T>::insertAsRoot(T const& e) {
    BinNode<T>* replacement = new BinNode<T>(e);
    BinNode<T>* oldRoot = _root;
    _root = replacement;
    _size = 1;
    if (oldRoot)
        removeAt(oldRoot);
    return _root;
}

template<typename T>
BinNode<T>* BinTree<T>::insertAsRC(BinNode<T>* x, T const& e) {
    if (!x)
        throw std::invalid_argument("BinTree::insertAsRC requires a parent node");

    const int removed = x->rc ? x->rc->size() : 0;
    BinNode<T>* child = x->insertAsRC(e);
    _size = _size - removed + 1;
    updateHeightAbove(x);
    return child;
}

template<typename T>
BinNode<T>* BinTree<T>::insertAsLC(BinNode<T>* x, T const& e) {
    if (!x)
        throw std::invalid_argument("BinTree::insertAsLC requires a parent node");

    const int removed = x->lc ? x->lc->size() : 0;
    BinNode<T>* child = x->insertAsLC(e);
    _size = _size - removed + 1;
    updateHeightAbove(x);
    return child;
}

template<typename T>
BinNode<T>* BinTree<T>::attachAsRC(BinNode<T>* x, BinTree<T>*& source) {
    if (!x || !source)
        throw std::invalid_argument("BinTree::attachAsRC requires node and source tree");
    if (source == this)
        throw std::invalid_argument("BinTree cannot attach itself");

    BinNode<T>* oldChild = x->rc;
    const int removed = oldChild ? oldChild->size() : 0;
    const int inserted = source->_size;
    BinNode<T>* incoming = source->_root;

    source->_root = nullptr;
    source->_size = 0;
    x->rc = incoming;
    if (incoming)
        incoming->parent = x;

    _size = _size - removed + inserted;
    if (oldChild)
        removeAt(oldChild);
    updateHeightAbove(x);

    delete source;
    source = nullptr;
    return x;
}

template<typename T>
BinNode<T>* BinTree<T>::attachAsLC(BinNode<T>* x, BinTree<T>*& source) {
    if (!x || !source)
        throw std::invalid_argument("BinTree::attachAsLC requires node and source tree");
    if (source == this)
        throw std::invalid_argument("BinTree cannot attach itself");

    BinNode<T>* oldChild = x->lc;
    const int removed = oldChild ? oldChild->size() : 0;
    const int inserted = source->_size;
    BinNode<T>* incoming = source->_root;

    source->_root = nullptr;
    source->_size = 0;
    x->lc = incoming;
    if (incoming)
        incoming->parent = x;

    _size = _size - removed + inserted;
    if (oldChild)
        removeAt(oldChild);
    updateHeightAbove(x);

    delete source;
    source = nullptr;
    return x;
}

template<typename T>
BinNode<T>*& BinTree<T>::FromParentTo(const BinNode<T>& node) {
    if (IsRoot(node))
        return _root;
    if (IsLChild(node))
        return node.parent->lc;
    return node.parent->rc;
}

template<typename T>
int BinTree<T>::remove(BinNode<T>* x) {
    if (!x)
        return 0;

    BinNode<T>* parent = x->parent;
    FromParentTo(*x) = nullptr;
    x->parent = nullptr;
    updateHeightAbove(parent);
    const int removed = removeAt(x);
    _size -= removed;
    return removed;
}

template<typename T>
BinTree<T>* BinTree<T>::secede(BinNode<T>* x) {
    if (!x)
        return nullptr;

    BinTree<T>* subtree = new BinTree<T>();
    BinNode<T>* parent = x->parent;
    const int subtreeSize = x->size();

    FromParentTo(*x) = nullptr;
    x->parent = nullptr;
    updateHeightAbove(parent);

    subtree->_root = x;
    subtree->_size = subtreeSize;
    _size -= subtreeSize;
    return subtree;
}

template<typename T>
struct BinTree<T>::iterator {
    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    BinNode<T>* cur;

    // 构造指向指定节点的中序迭代器。
    explicit iterator(BinNode<T>* node = nullptr) : cur(node) {}

    // 比较两个迭代器是否指向同一节点。
    bool operator==(const iterator& other) const { return cur == other.cur; }

    // 比较两个迭代器是否指向不同节点。
    bool operator!=(const iterator& other) const { return !(*this == other); }

    // 返回当前节点值引用；调用方不得解引用 end iterator。
    reference operator*() const { return cur->data; }

    // 返回当前节点值地址。
    pointer operator->() const { return &cur->data; }

    // 前置递增到中序后继。
    iterator& operator++() {
        cur = cur ? cur->succ() : nullptr;
        return *this;
    }

    // 后置递增并返回递增前副本。
    iterator operator++(int) {
        iterator previous(*this);
        ++(*this);
        return previous;
    }
};

template<typename T>
typename BinTree<T>::iterator BinTree<T>::begin() {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    return iterator(dsa::core::BinTreeAlgorithm<Access>::leftmost(_root));
}

template<typename T>
typename BinTree<T>::iterator BinTree<T>::end() {
    return iterator(nullptr);
}

template<typename T>
const typename BinTree<T>::iterator BinTree<T>::begin() const {
    return const_cast<BinTree<T>*>(this)->begin();
}

template<typename T>
const typename BinTree<T>::iterator BinTree<T>::end() const {
    return const_cast<BinTree<T>*>(this)->end();
}

template<typename T>
template<typename Res, typename Agg>
Res BinTree<T>::rangeAggregateRec(BinNode<T>* x, const T& lo, const T& hi, Res acc, Agg&& agg) const {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    return dsa::core::BinTreeAlgorithm<Access>::rangeAggregateOrdered(
        x,
        lo,
        hi,
        acc,
        std::forward<Agg>(agg)
    );
}

template<typename T>
template<typename Res, typename Agg>
Res BinTree<T>::rangeAggregate(const T& lo, const T& hi, Res identity, Agg&& agg) const {
    return rangeAggregateRec(_root, lo, hi, identity, std::forward<Agg>(agg));
}

template<typename T>
template<typename VST>
void BinTree<T>::travLevel(VST&& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traverseLevelOrder(_root, visit);
}

template<typename T>
template<typename VST>
void BinTree<T>::travPre(VST&& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traversePreOrder(_root, visit);
}

template<typename T>
template<typename VST>
void BinTree<T>::travIn(VST&& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traverseInOrder(_root, visit);
}

template<typename T>
template<typename VST>
void BinTree<T>::travPost(VST&& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traversePostOrder(_root, visit);
}

template<typename T>
bool BinTree<T>::operator<(const BinTree<T>& tree) const {
    return _root && tree._root && ((*_root) < (*tree._root));
}

template<typename T>
bool BinTree<T>::operator>(const BinTree<T>& tree) const {
    return _root && tree._root && ((*_root) > (*tree._root));
}

template<typename T>
bool BinTree<T>::operator==(const BinTree<T>& tree) const {
    return _root && tree._root && (_root == tree._root);
}

template<typename T>
bool BinTree<T>::operator!=(const BinTree<T>& tree) const {
    return !(*this == tree);
}

#endif
