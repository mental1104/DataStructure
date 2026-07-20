#ifndef __DSA_BSTARTREE
#define __DSA_BSTARTREE

#include "BTree.h"

// 兼容旧 BStarTree 名称与调用签名。
// 当前仓库尚未实现 B* 树特有的 2/3 分裂与约 2/3 占用率策略，
// 因而这里明确复用正确的 B-Tree 实现，而不再伪装继承二叉 BST。
template<typename T>
class BStarTree : public BTree<T> {
    typedef BTree<T> Base;

public:
    typedef BTNode<T> Node;

    explicit BStarTree(int order = 5) : Base(order) {}

    Node* search(const T& e, int = 0) { return Base::search(e); }
    bool insert(const T& e, bool = true) { return Base::insert(e); }
    bool remove(const T& e) { return Base::remove(e); }
};

#endif
