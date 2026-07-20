#ifndef __DSA_REDBLACK
#define __DSA_REDBLACK

#include "BST.h"

// 空链接按红黑树约定视为黑色。
template<typename T>
inline bool IsBlack(BinNode<T>* node) {
    return !node || node->color == RBColor::BLACK;
}

template<typename T>
inline bool IsRed(BinNode<T>* node) {
    return node && node->color == RBColor::RED;
}

template<typename T>
static int redBlackHeight(BinNode<T>* node) {
    if (!node)
        return 0;
    const int left = redBlackHeight(node->lc);
    const int right = redBlackHeight(node->rc);
    if (left < 0 || right < 0 || left != right)
        return -1;
    return left + (IsBlack(node) ? 1 : 0);
}

// 兼容旧 helper：验证当前子树的黑高度是否一致。
template<typename T>
inline bool BlackHeightUpdated(BinNode<T>& node) {
    return redBlackHeight(&node) >= 0;
}

// 教学版红黑树：保留 solveDoubleRed / solveDoubleBlack 作为可观察教学步骤，
// 查找、旋转和物理摘除则复用 BST 共享流程。
template<typename T>
class RedBlack : public BST<T> {
protected:
    void solveDoubleRed(BinNode<T>* x);
    void solveDoubleBlack(BinNode<T>* x);
    int updateHeight(BinNode<T>* x);

public:
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
int RedBlack<T>::updateHeight(BinNode<T>* x) {
    return BST<T>::updateHeight(x);
}

template<typename T>
BinNode<T>* RedBlack<T>::insert(const T& e) {
    BinNode<T>*& slot = this->search(e);
    if (slot)
        return slot;

    BinNode<T>* inserted = new BinNode<T>(e, this->_hot, nullptr, nullptr, 0, 1, RBColor::RED);
    slot = inserted;
    ++this->_size;
    solveDoubleRed(inserted);
    return inserted;
}

template<typename T>
void RedBlack<T>::solveDoubleRed(BinNode<T>* x) {
    while (x != this->_root && IsRed(x->parent)) {
        BinNode<T>* parent = x->parent;
        BinNode<T>* grand = parent->parent;
        if (parent == grand->lc) {
            BinNode<T>* uncle_node = grand->rc;
            if (IsRed(uncle_node)) {
                parent->color = RBColor::BLACK;
                uncle_node->color = RBColor::BLACK;
                grand->color = RBColor::RED;
                x = grand;
                continue;
            }
            if (x == parent->rc) {
                x = parent;
                this->rotateLeft(x);
                parent = x->parent;
                grand = parent->parent;
            }
            parent->color = RBColor::BLACK;
            grand->color = RBColor::RED;
            this->rotateRight(grand);
        } else {
            BinNode<T>* uncle_node = grand->lc;
            if (IsRed(uncle_node)) {
                parent->color = RBColor::BLACK;
                uncle_node->color = RBColor::BLACK;
                grand->color = RBColor::RED;
                x = grand;
                continue;
            }
            if (x == parent->lc) {
                x = parent;
                this->rotateRight(x);
                parent = x->parent;
                grand = parent->parent;
            }
            parent->color = RBColor::BLACK;
            grand->color = RBColor::RED;
            this->rotateLeft(grand);
        }
    }

    if (this->_root)
        this->_root->color = RBColor::BLACK;
    this->updateHeightAbove(x);
}

template<typename T>
bool RedBlack<T>::remove(const T& e) {
    BinNode<T>*& slot = this->search(e);
    if (!slot)
        return false;

    typedef typename BST<T>::Algorithm Algorithm;
    BinNode<T>* target = slot;
    BinNode<T>* structural_removed = target;
    if (target->lc && target->rc)
        structural_removed = Algorithm::minimum(target->rc);

    const RBColor removed_color = structural_removed->color;
    const RBColor target_color = target->color;
    const int target_height = target->height;

    typename BST<T>::EraseResult result = this->detachAt(slot);
    if (result.moved_node) {
        result.moved_node->color = target_color;
        result.moved_node->height = target_height;
    }

    this->_hot = result.fix_parent;
    this->destroyDetached(result.removed);
    --this->_size;

    if (removed_color == RBColor::BLACK)
        solveDoubleBlack(result.fix_node);
    if (this->_root)
        this->_root->color = RBColor::BLACK;

    if (result.moved_node)
        this->updateHeight(result.moved_node);
    this->updateHeightAbove(result.fix_node ? result.fix_node : result.fix_parent);
    return true;
}

template<typename T>
void RedBlack<T>::solveDoubleBlack(BinNode<T>* x) {
    BinNode<T>* parent = x ? x->parent : this->_hot;
    BinNode<T>* height_anchor = parent ? parent : x;
    while (x != this->_root && IsBlack(x)) {
        if (!parent)
            break;

        if (x == parent->lc) {
            BinNode<T>* sibling = parent->rc;
            if (IsRed(sibling)) {
                sibling->color = RBColor::BLACK;
                parent->color = RBColor::RED;
                this->rotateLeft(parent);
                sibling = parent->rc;
            }

            if (!sibling) {
                x = parent;
                parent = x->parent;
                continue;
            }

            if (IsBlack(sibling->lc) && IsBlack(sibling->rc)) {
                sibling->color = RBColor::RED;
                x = parent;
                parent = x->parent;
            } else {
                if (IsBlack(sibling->rc)) {
                    if (sibling->lc)
                        sibling->lc->color = RBColor::BLACK;
                    sibling->color = RBColor::RED;
                    this->rotateRight(sibling);
                    sibling = parent->rc;
                }
                sibling->color = parent->color;
                parent->color = RBColor::BLACK;
                if (sibling->rc)
                    sibling->rc->color = RBColor::BLACK;
                this->rotateLeft(parent);
                x = this->_root;
                parent = nullptr;
            }
        } else {
            BinNode<T>* sibling = parent->lc;
            if (IsRed(sibling)) {
                sibling->color = RBColor::BLACK;
                parent->color = RBColor::RED;
                this->rotateRight(parent);
                sibling = parent->lc;
            }

            if (!sibling) {
                x = parent;
                parent = x->parent;
                continue;
            }

            if (IsBlack(sibling->lc) && IsBlack(sibling->rc)) {
                sibling->color = RBColor::RED;
                x = parent;
                parent = x->parent;
            } else {
                if (IsBlack(sibling->lc)) {
                    if (sibling->rc)
                        sibling->rc->color = RBColor::BLACK;
                    sibling->color = RBColor::RED;
                    this->rotateLeft(sibling);
                    sibling = parent->lc;
                }
                sibling->color = parent->color;
                parent->color = RBColor::BLACK;
                if (sibling->lc)
                    sibling->lc->color = RBColor::BLACK;
                this->rotateRight(parent);
                x = this->_root;
                parent = nullptr;
            }
        }
    }
    if (x)
        x->color = RBColor::BLACK;
    // solveDoubleBlack 是可独立调用的教学步骤；退出前收敛旋转或夹具遗留的结构高度。
    this->updateHeightAbove(height_anchor);
}

#endif
