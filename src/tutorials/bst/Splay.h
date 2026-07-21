#ifndef __DSA_SPLAY
#define __DSA_SPLAY

#include "BST.h"
#include <dsa/core/tree/SplayTreeAlgorithm.h>

// 教学版伸展树：使用 BST 提供的共享单旋转组合出 zig / zig-zig / zig-zag。
template<typename T>
class Splay : public BST<T> {
protected:
    BinNode<T>* splay(BinNode<T>* node);

public:
    BinNode<T>*& search(const T& e);
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
BinNode<T>* Splay<T>::splay(BinNode<T>* node) {
    typedef dsa::core::SplayTreeAlgorithm<typename BST<T>::Access> SplayAlgorithm;
    typedef typename BST<T>::Algorithm SearchAlgorithm;
    BinNode<T>* root = SplayAlgorithm::splay(this->_root, node);
    if (root) {
        SearchAlgorithm::updateHeight(root->lc);
        SearchAlgorithm::updateHeight(root->rc);
        SearchAlgorithm::updateHeight(root);
    }
    return root;
}

template<typename T>
BinNode<T>*& Splay<T>::search(const T& e) {
    BinNode<T>*& slot = BST<T>::search(e);
    this->_root = splay(slot ? slot : this->_hot);
    return this->_root;
}

template<typename T>
BinNode<T>* Splay<T>::insert(const T& e) {
    if (!this->_root) {
        ++this->_size;
        return this->_root = new BinNode<T>(e);
    }

    BinNode<T>* root = search(e);
    if (root->data == e)
        return root;

    BinNode<T>* created = new BinNode<T>(e);
    if (root->data < e) {
        created->lc = root;
        created->rc = root->rc;
        if (created->rc)
            created->rc->parent = created;
        root->rc = nullptr;
        root->parent = created;
    } else {
        created->rc = root;
        created->lc = root->lc;
        if (created->lc)
            created->lc->parent = created;
        root->lc = nullptr;
        root->parent = created;
    }

    this->_root = created;
    ++this->_size;
    this->updateHeight(root);
    this->updateHeight(created);
    return created;
}

template<typename T>
bool Splay<T>::remove(const T& e) {
    if (!this->_root || search(e)->data != e)
        return false;

    BinNode<T>* removed = this->_root;
    BinNode<T>* left = removed->lc;
    BinNode<T>* right = removed->rc;
    if (left)
        left->parent = nullptr;
    if (right)
        right->parent = nullptr;
    removed->lc = nullptr;
    removed->rc = nullptr;

    if (!right) {
        this->_root = left;
    } else {
        this->_root = right;
        typedef typename BST<T>::Algorithm Algorithm;
        BinNode<T>* minimum = Algorithm::minimum(right);
        this->_root = splay(minimum);
        this->_root->lc = left;
        if (left)
            left->parent = this->_root;
        this->updateHeight(this->_root);
    }

    this->destroyDetached(removed);
    --this->_size;
    return true;
}

#endif
