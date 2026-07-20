#ifndef __DSA_BINTREE
#define __DSA_BINTREE

#include "BinNode.h"
#include "dsa/core/tree/BinTreeAlgorithm.h"
#include "release.h"

template<typename T>
class BinTree
    : protected dsa::core::BinTreeAlgorithm<BinTree<T>, T, BinNode<T>> {

protected:
    using Node = BinNode<T>;
    using Algorithm = dsa::core::BinTreeAlgorithm<BinTree<T>, T, Node>;

    friend class dsa::core::BinTreeAlgorithm<BinTree<T>, T, Node>;

    int _size{0};
    Node* _root{nullptr};

    Node*& rootRef() {
        return _root;
    }

    virtual int updateHeight(Node* x);
    void updateHeightAbove(Node* x);
    Node*& FromParentTo(const Node& node);

    template<typename Res, typename Agg>
    Res rangeAggregateRec(
        Node* x,
        const T& lo,
        const T& hi,
        Res acc,
        Agg&& agg
    ) const;

    int destroySubtree(Node* root);

public:
    BinTree() {}

    virtual ~BinTree() {
        destroySubtree(_root);
    }

    int size() const {
        return _size;
    }

    bool empty() const {
        return !_root;
    }

    Node* root() {
        return _root;
    }

    const Node* root() const {
        return _root;
    }

    Node* insertAsRoot(T const& e);
    Node* insertAsLC(Node* x, T const& e);
    Node* insertAsRC(Node* x, T const& e);
    Node* attachAsLC(Node* x, BinTree<T>*& S);
    Node* attachAsRC(Node* x, BinTree<T>*& S);
    int remove(Node* x);
    BinTree<T>* secede(Node* x);

    struct iterator;
    iterator begin();
    iterator end();

    const iterator begin() const {
        return const_cast<BinTree<T>*>(this)->begin();
    }

    const iterator end() const {
        return const_cast<BinTree<T>*>(this)->end();
    }

    template<typename Res, typename Agg>
    Res rangeAggregate(
        const T& lo,
        const T& hi,
        Res identity,
        Agg&& agg
    ) const {
        return rangeAggregateRec(_root, lo, hi, identity, agg);
    }

    template<typename VST>
    void travLevel(VST&& visit) {
        Algorithm::traverseLevelImpl(_root, visit);
    }

    template<typename VST>
    void travPre(VST&& visit) {
        Algorithm::traversePreImpl(_root, visit);
    }

    template<typename VST>
    void travIn(VST&& visit) {
        Algorithm::traverseInImpl(_root, visit);
    }

    template<typename VST>
    void travPost(VST&& visit) {
        Algorithm::traversePostImpl(_root, visit);
    }

    bool operator<(const BinTree<T>& tree) {
        return _root && tree._root && (*_root < *tree._root);
    }

    bool operator>(const BinTree<T>& tree) {
        return _root && tree._root && (*_root > *tree._root);
    }

    bool operator==(const BinTree<T>& tree) {
        return _root && tree._root && (_root == tree._root);
    }

    bool operator!=(const BinTree<T>& tree) {
        return !(*this == tree);
    }
};

template<typename T>
int BinTree<T>::destroySubtree(Node* root) {
    if (!root)
        return 0;

    auto destroyNode = [](Node* node) {
        release(node->data);
        release(node);
    };

    return Algorithm::destroySubtreeImpl(root, destroyNode);
}

template<typename T>
int BinTree<T>::updateHeight(Node* x) {
    return Algorithm::updateHeightImpl(x);
}

template<typename T>
void BinTree<T>::updateHeightAbove(Node* x) {
    Algorithm::updateHeightAboveImpl(x);
}

template<typename T>
typename BinTree<T>::Node*
BinTree<T>::insertAsRoot(T const& e) {
    destroySubtree(_root);
    _size = 1;
    return _root = new Node(e);
}

template<typename T>
typename BinTree<T>::Node*
BinTree<T>::insertAsRC(Node* x, T const& e) {
    _size -= destroySubtree(x->rc);
    x->rc = new Node(e, x);
    ++_size;
    updateHeightAbove(x);
    return x->rc;
}

template<typename T>
typename BinTree<T>::Node*
BinTree<T>::insertAsLC(Node* x, T const& e) {
    _size -= destroySubtree(x->lc);
    x->lc = new Node(e, x);
    ++_size;
    updateHeightAbove(x);
    return x->lc;
}

template<typename T>
typename BinTree<T>::Node*
BinTree<T>::attachAsRC(Node* x, BinTree<T>*& S) {
    BinTree<T>* temp = secede(x->rc);

    if ((x->rc = S->_root))
        x->rc->parent = x;

    if (temp)
        release(temp);

    _size += S->_size;
    updateHeightAbove(x);
    S->_root = nullptr;
    S->_size = 0;
    release(S);
    S = nullptr;
    return x;
}

template<typename T>
typename BinTree<T>::Node*
BinTree<T>::attachAsLC(Node* x, BinTree<T>*& S) {
    BinTree<T>* temp = secede(x->rc);

    if ((x->lc = S->_root))
        x->lc->parent = x;

    if (temp)
        release(temp);

    _size += S->_size;
    updateHeightAbove(x);
    S->_root = nullptr;
    S->_size = 0;
    release(S);
    S = nullptr;
    return x;
}

template<typename T>
typename BinTree<T>::Node*&
BinTree<T>::FromParentTo(const Node& node) {
    return Algorithm::fromParentToImpl(node);
}

template<typename T>
int BinTree<T>::remove(Node* x) {
    FromParentTo(*x) = nullptr;
    updateHeightAbove(x->parent);

    const int removed = destroySubtree(x);
    _size -= removed;
    return removed;
}

template<typename T>
BinTree<T>* BinTree<T>::secede(Node* x) {
    if (!x)
        return nullptr;

    FromParentTo(*x) = nullptr;
    updateHeightAbove(x->parent);

    BinTree<T>* subtree = new BinTree<T>();
    subtree->_root = x;
    x->parent = nullptr;
    subtree->_size = Algorithm::subtreeSizeImpl(x);
    _size -= subtree->_size;
    return subtree;
}

template<typename T>
struct BinTree<T>::iterator {
    Node* cur;

    explicit iterator(Node* rhs)
        : cur(rhs) {}

    bool operator!=(const iterator& other) {
        return cur != other.cur;
    }

    T& operator*() {
        return cur->data;
    }

    iterator& operator++() {
        cur = Node::Algorithm::successor(cur);
        return *this;
    }
};

template<typename T>
typename BinTree<T>::iterator
BinTree<T>::begin() {
    Node* node = _root;

    if (node)
        while (node->lc)
            node = node->lc;

    return iterator(node);
}

template<typename T>
typename BinTree<T>::iterator
BinTree<T>::end() {
    return iterator(nullptr);
}

template<typename T>
template<typename Res, typename Agg>
Res BinTree<T>::rangeAggregateRec(
    Node* x,
    const T& lo,
    const T& hi,
    Res acc,
    Agg&& agg
) const {
    return Algorithm::rangeAggregateRecImpl(x, lo, hi, acc, agg);
}

#endif
