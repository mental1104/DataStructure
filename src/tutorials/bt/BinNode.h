#ifndef DSA_BT_BIN_NODE_H
#define DSA_BT_BIN_NODE_H

#include "utils.h"
#include <dsa/core/tree/BinTreeAlgorithm.h>

#include <utility>

enum class RBColor {
    RED,
    BLACK,
};

template<typename T>
struct BinNode {
    T data;
    BinNode<T>* parent;
    BinNode<T>* lc;
    BinNode<T>* rc;
    int height;
    int npl; // Null Path Length
    RBColor color;

    // 构造一个不链接父子节点的教学节点，并初始化平衡树元数据。
    BinNode();

    // 按教学版公开布局构造节点；父子所有权仍由上层树结构负责。
    BinNode(
        T e,
        BinNode<T>* p = nullptr,
        BinNode<T>* left = nullptr,
        BinNode<T>* right = nullptr,
        int h = 0,
        int l = 1,
        RBColor c = RBColor::RED
    );

    // 通过共享迭代算法统计当前子树节点数。
    int size();

    // 替换左孩子；保留教学版“已有子树会被销毁”的历史语义。
    BinNode<T>* insertAsLC(T const& e);

    // 替换右孩子；保留教学版“已有子树会被销毁”的历史语义。
    BinNode<T>* insertAsRC(T const& e);

    // 返回当前节点的中序后继，未找到时返回 nullptr。
    BinNode<T>* succ();

    // 兼容旧 API：从给定节点开始执行迭代层序遍历。
    template<typename VST>
    void travLevel(BinNode<T>* x, VST& visit);

    // 兼容旧 API：从当前节点开始执行迭代层序遍历。
    template<typename VST>
    void travLevel(VST& visit);

    // 兼容旧 API：从给定节点开始执行迭代前序遍历。
    template<typename VST>
    void travPre(BinNode<T>* x, VST& visit);

    // 兼容旧 API：从当前节点开始执行迭代前序遍历。
    template<typename VST>
    void travPre(VST& visit);

    // 兼容旧 API：从给定节点开始执行迭代中序遍历。
    template<typename VST>
    void travIn(BinNode<T>* x, VST& visit);

    // 兼容旧 API：从当前节点开始执行迭代中序遍历。
    template<typename VST>
    void travIn(VST& visit);

    // 兼容旧 API：从给定节点开始执行迭代后序遍历。
    template<typename VST>
    void travPost(BinNode<T>* x, VST& visit);

    // 兼容旧 API：从当前节点开始执行迭代后序遍历。
    template<typename VST>
    void travPost(VST& visit);

    // 教学节点比较只比较存储值，不比较结构和元数据。
    bool operator<(BinNode const& node) const;
    bool operator>(BinNode const& node) const;
    bool operator==(BinNode const& node) const;
    bool operator!=(BinNode const& node) const;
};

namespace dsa {
namespace core {

// 将教学版 BinNode 的公开字段适配为共享树算法所需的语义访问接口。
template<typename T>
struct TeachingBinNodeAccess {
    typedef ::BinNode<T> node_type;

    static node_type* left(node_type* node) { return node ? node->lc : nullptr; }
    static const node_type* left(const node_type* node) { return node ? node->lc : nullptr; }
    static node_type* right(node_type* node) { return node ? node->rc : nullptr; }
    static const node_type* right(const node_type* node) { return node ? node->rc : nullptr; }
    static node_type* parent(node_type* node) { return node ? node->parent : nullptr; }
    static const node_type* parent(const node_type* node) { return node ? node->parent : nullptr; }
    static T& value(node_type* node) { return node->data; }
    static const T& value(const node_type* node) { return node->data; }
    static int& height(node_type* node) { return node->height; }
    static int height(const node_type* node) { return node->height; }
};

} // namespace core
} // namespace dsa

// 通过教学版 release 约定释放节点值，并以后序迭代方式销毁子树。
template<typename T>
int removeAt(BinNode<T>* root) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    typedef dsa::core::BinTreeAlgorithm<Access> Algorithm;
    const std::size_t removed = Algorithm::destroySubtree(root, [](BinNode<T>* node) {
        release(node->data);
        delete node;
    });
    return static_cast<int>(removed);
}

// 返回节点高度；空指针按教学约定记为 -1。
template<typename T>
inline int stature(BinNode<T>* node) {
    return node ? node->height : -1;
}

// 判断节点是否为根节点。
template<typename T>
inline bool IsRoot(const BinNode<T>& node) {
    return !node.parent;
}

// 判断节点是否为父节点的左孩子。
template<typename T>
inline bool IsLChild(const BinNode<T>& node) {
    return !IsRoot(node) && (&node == node.parent->lc);
}

// 判断节点是否为父节点的右孩子。
template<typename T>
inline bool IsRChild(const BinNode<T>& node) {
    return !IsRoot(node) && (&node == node.parent->rc);
}

// 判断节点是否存在父节点。
template<typename T>
inline bool HasParent(const BinNode<T>& node) {
    return !IsRoot(node);
}

// 返回左孩子指针，兼容旧代码将其直接用于条件判断的方式。
template<typename T>
inline BinNode<T>* HasLChild(const BinNode<T>& node) {
    return node.lc;
}

// 返回右孩子指针，兼容旧代码将其直接用于条件判断的方式。
template<typename T>
inline BinNode<T>* HasRChild(const BinNode<T>& node) {
    return node.rc;
}

// 判断节点是否至少存在一个孩子。
template<typename T>
inline bool HasChild(const BinNode<T>& node) {
    return HasLChild(node) || HasRChild(node);
}

// 判断节点是否同时存在左右孩子。
template<typename T>
inline bool HasBothChild(const BinNode<T>& node) {
    return HasLChild(node) && HasRChild(node);
}

// 判断节点是否为叶子节点。
template<typename T>
inline bool IsLeaf(const BinNode<T>& node) {
    return !HasChild(node);
}

// 判断左右子树高度是否完全相等。
template<typename T>
inline bool Balanced(const BinNode<T>& node) {
    return stature(node.lc) == stature(node.rc);
}

// 计算 AVL 平衡因子：左子树高度减右子树高度。
template<typename T>
inline int BalFac(const BinNode<T>& node) {
    return stature(node.lc) - stature(node.rc);
}

// 判断节点是否满足 AVL 的高度差约束。
template<typename T>
inline bool AvlBalanced(const BinNode<T>& node) {
    return -2 < BalFac(node) && BalFac(node) < 2;
}

// 返回更高的孩子；等高时沿节点自身所在方向打破平局。
template<typename T>
inline BinNode<T>* tallerChild(const BinNode<T>* node) {
    return stature(node->lc) > stature(node->rc) ? node->lc
         : stature(node->lc) < stature(node->rc) ? node->rc
         : IsLChild(*node) ? node->lc : node->rc;
}

// 返回节点的兄弟节点。
template<typename T>
inline BinNode<T>* sibling(const BinNode<T>*& node) {
    return IsLChild(*node) ? node->parent->rc : node->parent->lc;
}

// 返回节点的叔父节点；调用方负责保证父节点和祖父节点存在。
template<typename T>
inline BinNode<T>* uncle(BinNode<T>* node) {
    return IsLChild(*(node->parent)) ? node->parent->parent->rc : node->parent->parent->lc;
}

template<typename T>
BinNode<T>::BinNode()
    : data(), parent(nullptr), lc(nullptr), rc(nullptr), height(0), npl(1), color(RBColor::RED) {}

template<typename T>
BinNode<T>::BinNode(
    T e,
    BinNode<T>* p,
    BinNode<T>* left,
    BinNode<T>* right,
    int h,
    int l,
    RBColor c
)
    : data(e), parent(p), lc(left), rc(right), height(h), npl(l), color(c) {}

template<typename T>
int BinNode<T>::size() {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    return static_cast<int>(dsa::core::BinTreeAlgorithm<Access>::subtreeSize(this));
}

template<typename T>
BinNode<T>* BinNode<T>::insertAsLC(T const& e) {
    BinNode<T>* replacement = new BinNode<T>(e, this);
    BinNode<T>* oldChild = lc;
    lc = replacement;
    if (oldChild)
        removeAt(oldChild);
    return replacement;
}

template<typename T>
BinNode<T>* BinNode<T>::insertAsRC(T const& e) {
    BinNode<T>* replacement = new BinNode<T>(e, this);
    BinNode<T>* oldChild = rc;
    rc = replacement;
    if (oldChild)
        removeAt(oldChild);
    return replacement;
}

template<typename T>
BinNode<T>* BinNode<T>::succ() {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    return dsa::core::BinTreeAlgorithm<Access>::successor(this);
}

template<typename T>
template<typename VST>
void BinNode<T>::travLevel(BinNode<T>* x, VST& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traverseLevelOrder(x, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travLevel(VST& visit) {
    travLevel(this, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPre(BinNode<T>* x, VST& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traversePreOrder(x, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPre(VST& visit) {
    travPre(this, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travIn(BinNode<T>* x, VST& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traverseInOrder(x, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travIn(VST& visit) {
    travIn(this, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPost(BinNode<T>* x, VST& visit) {
    typedef dsa::core::TeachingBinNodeAccess<T> Access;
    dsa::core::BinTreeAlgorithm<Access>::traversePostOrder(x, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPost(VST& visit) {
    travPost(this, visit);
}

template<typename T>
bool BinNode<T>::operator<(BinNode const& node) const {
    return data < node.data;
}

template<typename T>
bool BinNode<T>::operator>(BinNode const& node) const {
    return data > node.data;
}

template<typename T>
bool BinNode<T>::operator==(BinNode const& node) const {
    return data == node.data;
}

template<typename T>
bool BinNode<T>::operator!=(BinNode const& node) const {
    return data != node.data;
}

#endif
