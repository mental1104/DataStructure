#ifndef __DSA_BST
#define __DSA_BST

#include "BinTree.h"
#include <dsa/core/tree/SearchTreeAlgorithm.h>

#include <functional>

namespace dsa {
namespace core {

// 在既有 TeachingBinNodeAccess 上补充可写链接语义，避免扩大教学 BinNode 的公开 API。
template<typename T>
struct TeachingSearchNodeAccess : TeachingBinNodeAccess<T> {
    typedef ::BinNode<T> node_type;

    static node_type*& leftRef(node_type* node) { return node->lc; }
    static node_type*& rightRef(node_type* node) { return node->rc; }
    static node_type*& parentRef(node_type* node) { return node->parent; }
    static int& heightRef(node_type* node) { return node->height; }
};

} // namespace core
} // namespace dsa

// 教学版 BST：保留 search 返回链接引用、_hot 与继承接口，
// 但搜索、旋转和物理摘除统一复用 SearchTreeAlgorithm。
template<typename T>
class BST : public BinTree<T> {
protected:
    typedef dsa::core::TeachingSearchNodeAccess<T> Access;
    typedef dsa::core::SearchTreeAlgorithm<Access> Algorithm;
    typedef typename Algorithm::EraseResult EraseResult;

    BinNode<T>* _hot{nullptr};

    // 兼容旧三节点重构 helper；只负责重连节点，不管理节点生命周期。
    BinNode<T>* connect34(
        BinNode<T>*, BinNode<T>*, BinNode<T>*,
        BinNode<T>*, BinNode<T>*, BinNode<T>*, BinNode<T>*
    );

    // 对 x、其父节点和祖父节点执行四类三节点重构，并直接提交父链接或根链接。
    BinNode<T>* rotateAt(BinNode<T>* x);

    // 供 AVL、红黑树和伸展树复用的单旋转；旋转后同步局部结构高度。
    BinNode<T>* rotateLeft(BinNode<T>* pivot);
    BinNode<T>* rotateRight(BinNode<T>* pivot);

    // 从 search 返回的链接中物理摘除节点；不复制或交换键值。
    EraseResult detachAt(BinNode<T>*& slot);

    // 释放已经完全脱离树结构的单个教学节点。
    void destroyDetached(BinNode<T>* node);

public:
    virtual BinNode<T>*& search(const T& e);
    virtual BinNode<T>* insert(const T& e);
    virtual bool remove(const T& e);
};

template<typename T>
BinNode<T>*& BST<T>::search(const T& e) {
    return Algorithm::searchSlot(this->_root, _hot, e, std::less<T>());
}

template<typename T>
BinNode<T>* BST<T>::insert(const T& e) {
    BinNode<T>*& slot = search(e);
    if (slot)
        return slot;

    BinNode<T>* created = new BinNode<T>(e, _hot);
    slot = created;
    ++this->_size;
    this->updateHeightAbove(created);
    return created;
}

template<typename T>
typename BST<T>::EraseResult BST<T>::detachAt(BinNode<T>*& slot) {
    return Algorithm::detachFromSlot(slot, slot);
}

template<typename T>
void BST<T>::destroyDetached(BinNode<T>* node) {
    if (!node)
        return;
    release(node->data);
    delete node;
}

template<typename T>
bool BST<T>::remove(const T& e) {
    BinNode<T>*& slot = search(e);
    if (!slot)
        return false;

    EraseResult result = detachAt(slot);
    _hot = result.fix_parent;
    destroyDetached(result.removed);
    --this->_size;

    if (result.moved_node)
        Algorithm::updateHeight(result.moved_node);
    Algorithm::updateHeightAbove(result.rebalance_from);
    return true;
}

template<typename T>
BinNode<T>* BST<T>::connect34(
    BinNode<T>* a, BinNode<T>* b, BinNode<T>* c,
    BinNode<T>* t0, BinNode<T>* t1, BinNode<T>* t2, BinNode<T>* t3
) {
    Algorithm::setLeft(a, t0);
    Algorithm::setRight(a, t1);
    Algorithm::updateHeight(a);

    Algorithm::setLeft(c, t2);
    Algorithm::setRight(c, t3);
    Algorithm::updateHeight(c);

    Algorithm::setLeft(b, a);
    Algorithm::setRight(b, c);
    Algorithm::updateHeight(b);
    return b;
}

template<typename T>
BinNode<T>* BST<T>::rotateLeft(BinNode<T>* pivot) {
    BinNode<T>* result = Algorithm::rotateLeft(this->_root, pivot);
    Algorithm::updateHeight(pivot);
    Algorithm::updateHeight(result);
    return result;
}

template<typename T>
BinNode<T>* BST<T>::rotateRight(BinNode<T>* pivot) {
    BinNode<T>* result = Algorithm::rotateRight(this->_root, pivot);
    Algorithm::updateHeight(pivot);
    Algorithm::updateHeight(result);
    return result;
}

template<typename T>
BinNode<T>* BST<T>::rotateAt(BinNode<T>* x) {
    BinNode<T>* result = Algorithm::restructure(this->_root, x);
    if (!result)
        return nullptr;
    Algorithm::updateHeight(result->lc);
    Algorithm::updateHeight(result->rc);
    Algorithm::updateHeight(result);
    return result;
}

// 兼容旧 helper：执行物理摘除并立即销毁被删除节点。
// hot 返回双黑修复或高度更新所需的父节点，返回值为替代空位的孩子。
template<typename T>
static BinNode<T>* removeAt(BinNode<T>*& slot, BinNode<T>*& hot) {
    typedef dsa::core::TeachingSearchNodeAccess<T> Access;
    typedef dsa::core::SearchTreeAlgorithm<Access> Algorithm;
    typename Algorithm::EraseResult result = Algorithm::detachFromSlot(slot, slot);
    hot = result.fix_parent;
    if (result.removed) {
        release(result.removed->data);
        delete result.removed;
    }
    if (result.moved_node)
        Algorithm::updateHeight(result.moved_node);
    return result.fix_node;
}

#endif
