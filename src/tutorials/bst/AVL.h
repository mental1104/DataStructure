#ifndef __DSA_AVL
#define __DSA_AVL

#include "BST.h"

#include <cmath>

// 教学版 AVL：沿用 BST 的公开契约，仅把平衡策略保留在派生类。
template<typename T>
class AVL : public BST<T> {
public:
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
BinNode<T>* AVL<T>::insert(const T& e) {
    BinNode<T>*& slot = this->search(e);
    if (slot)
        return slot;

    BinNode<T>* inserted = new BinNode<T>(e, this->_hot);
    slot = inserted;
    ++this->_size;

    BinNode<T>* current = this->_hot;
    while (current) {
        this->updateHeight(current);
        if (!AvlBalanced(*current)) {
            BinNode<T>* subtree_root = this->rotateAt(tallerChild(tallerChild(current)));
            current = subtree_root ? subtree_root->parent : nullptr;
        } else {
            current = current->parent;
        }
    }
    return inserted;
}

template<typename T>
bool AVL<T>::remove(const T& e) {
    BinNode<T>*& slot = this->search(e);
    if (!slot)
        return false;

    typename BST<T>::EraseResult result = this->detachAt(slot);
    this->_hot = result.fix_parent;
    this->destroyDetached(result.removed);
    --this->_size;

    BinNode<T>* current = result.rebalance_from;
    while (current) {
        this->updateHeight(current);
        if (!AvlBalanced(*current)) {
            BinNode<T>* subtree_root = this->rotateAt(tallerChild(tallerChild(current)));
            current = subtree_root ? subtree_root->parent : nullptr;
        } else {
            current = current->parent;
        }
    }
    return true;
}

#endif
