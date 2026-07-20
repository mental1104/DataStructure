#ifndef __DSA_BINNODE
#define __DSA_BINNODE

#include "dsa/core/tree/BinNodeAlgorithm.h"
#include "release.h"

enum class RBColor{
    RED,
    BLACK,
};

template<typename T>
struct BinNode {
    using Node = BinNode<T>;
    using Algorithm = dsa::core::BinNodeAlgorithm<Node>;

    T data;
    Node* parent;
    Node* lc;
    Node* rc;
    int height;
    int npl; // Null Path Length
    RBColor color;

    BinNode()
        : parent(nullptr), lc(nullptr), rc(nullptr),
          height(0), npl(1), color(RBColor::RED) {}

    BinNode(
        T e,
        Node* p = nullptr,
        Node* left = nullptr,
        Node* right = nullptr,
        int h = 0,
        int l = 1,
        RBColor c = RBColor::RED
    )
        : data(e), parent(p), lc(left), rc(right),
          height(h), npl(l), color(c) {}

    int size();
    Node* insertAsLC(T const&);
    Node* insertAsRC(T const&);
    Node* succ();

    template<typename VST>
    void travLevel(Node* x, VST& visit);

    template<typename VST>
    void travLevel(VST& visit) {
        travLevel(this, visit);
    }

    template<typename VST>
    void travPre(Node* x, VST& visit);

    template<typename VST>
    void travPre(VST& visit) {
        travPre(this, visit);
    }

    template<typename VST>
    void travIn(Node* x, VST& visit);

    template<typename VST>
    void travIn(VST& visit) {
        travIn(this, visit);
    }

    template<typename VST>
    void travPost(Node* x, VST& visit);

    template<typename VST>
    void travPost(VST& visit) {
        travPost(this, visit);
    }

    bool operator<(BinNode const& node) {
        return data < node.data;
    }

    bool operator>(BinNode const& node) {
        return data > node.data;
    }

    bool operator==(BinNode const& node) {
        return data == node.data;
    }

    bool operator!=(BinNode const& node) {
        return data != node.data;
    }
};

template<typename T>
inline int stature(BinNode<T>* node) {
    return BinNode<T>::Algorithm::stature(node);
}

template<typename T>
inline bool IsRoot(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::isRoot(node);
}

template<typename T>
inline bool IsLChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::isLeftChild(node);
}

template<typename T>
inline bool IsRChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::isRightChild(node);
}

template<typename T>
inline bool HasParent(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::hasParent(node);
}

template<typename T>
inline BinNode<T>* HasLChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::leftChild(node);
}

template<typename T>
inline BinNode<T>* HasRChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::rightChild(node);
}

template<typename T>
inline bool HasChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::hasChild(node);
}

template<typename T>
inline bool HasBothChild(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::hasBothChildren(node);
}

template<typename T>
inline bool IsLeaf(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::isLeaf(node);
}

template<typename T>
inline bool Balanced(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::balanced(node);
}

template<typename T>
inline int BalFac(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::balanceFactor(node);
}

template<typename T>
inline bool AvlBalanced(const BinNode<T>& node) {
    return BinNode<T>::Algorithm::avlBalanced(node);
}

template<typename T>
inline BinNode<T>* tallerChild(const BinNode<T>* node) {
    return BinNode<T>::Algorithm::tallerChild(node);
}

template<typename T>
inline BinNode<T>* sibling(const BinNode<T>*& node) {
    return BinNode<T>::Algorithm::sibling(node);
}

template<typename T>
inline BinNode<T>* uncle(BinNode<T>* node) {
    return BinNode<T>::Algorithm::uncle(node);
}

template<typename T>
BinNode<T>* BinNode<T>::insertAsLC(T const& value) {
    if (lc) {
        auto destroyNode = [](Node* node) {
            release(node->data);
            release(node);
        };
        Algorithm::destroySubtree(lc, destroyNode);
    }

    return lc = new Node(value, this);
}

template<typename T>
BinNode<T>* BinNode<T>::insertAsRC(T const& value) {
    if (rc) {
        auto destroyNode = [](Node* node) {
            release(node->data);
            release(node);
        };
        Algorithm::destroySubtree(rc, destroyNode);
    }

    return rc = new Node(value, this);
}

template<typename T>
int BinNode<T>::size() {
    return Algorithm::subtreeSize(this);
}

template<typename T>
BinNode<T>* BinNode<T>::succ() {
    return Algorithm::successor(this);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPre(Node* node, VST& visit) {
    Algorithm::traversePre(node, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travIn(Node* node, VST& visit) {
    Algorithm::traverseIn(node, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travPost(Node* node, VST& visit) {
    Algorithm::traversePost(node, visit);
}

template<typename T>
template<typename VST>
void BinNode<T>::travLevel(Node* node, VST& visit) {
    Algorithm::traverseLevel(node, visit);
}

#endif
