#ifndef __DSA_BINTREE
#define __DSA_BINTREE

#include "BinNode.h"
#include "dsa/core/tree/BinTreeAlgorithm.h"

template<typename T> 
class BinTree : protected dsa::core::BinTreeAlgorithm<BinTree<T>, T, BinNode<T>> {
protected:
    using Algorithm = dsa::core::BinTreeAlgorithm<BinTree<T>, T, BinNode<T>>;
    friend class dsa::core::BinTreeAlgorithm<BinTree<T>, T, BinNode<T>>;

    int _size{0}; 
    BinNode<T>* _root{nullptr};
    BinNode<T>*& rootRef() { return _root; }
    virtual int updateHeight(BinNode<T>* x);
    void updateHeightAbove(BinNode<T>* x);
    BinNode<T>*& FromParentTo(const BinNode<T>& node);
    template<typename Res, typename Agg>
    Res rangeAggregateRec(BinNode<T>* x, const T& lo, const T& hi, Res acc, Agg&& agg) const;
public:
    BinTree(){}
    virtual ~BinTree() {  if(0 < _size) levelRemove(_root);  }
    int size() const {  return _size; }
    bool empty() const {    return !_root; }
    BinNode<T>* root() { return _root; }
    BinNode<T>* insertAsRoot(T const& e);
    BinNode<T>* insertAsLC(BinNode<T>* x, T const& e);
    BinNode<T>* insertAsRC(BinNode<T>* x, T const& e);
    BinNode<T>* attachAsLC(BinNode<T>* x, BinTree<T>*& S);
    BinNode<T>* attachAsRC(BinNode<T>* x, BinTree<T>*& S);
    int remove(BinNode<T>* x);
    BinTree<T>* secede(BinNode<T>* x);

    struct iterator;
    iterator begin();
    iterator end();
    const iterator begin() const { return const_cast<BinTree<T>*>(this)->begin(); }
    const iterator end() const { return const_cast<BinTree<T>*>(this)->end(); }

    template<typename Res, typename Agg>
    Res rangeAggregate(const T& lo, const T& hi, Res identity, Agg&& agg) const {
        return rangeAggregateRec(_root, lo, hi, identity, agg);
    }
    template<typename VST> void travLevel(VST&& visit){  if(_root) _root->travLevel(visit); }
    template<typename VST> void travPre(VST&& visit){  if(_root) _root->travPre(visit); }
    template<typename VST> void travIn(VST&& visit){  if(_root) _root->travIn(visit); }
    template<typename VST> void travPost(VST&& visit){  if(_root) _root->travPost(visit); }
    bool operator< (const BinTree<T> &t){   return _root && t._root && ((*_root) < (*t._root));   }
    bool operator> (const BinTree<T> &t){   return _root && t._root && ((*_root) > (*t._root));   }
    bool operator== (const BinTree<T> &t){   return _root && t._root && (_root == t._root);   }
    bool operator!= (const BinTree<T> &t){   return !(*this==t);    }
};

template<typename T>
static void levelRemove(BinNode<T>* root){
    Queue<BinNode<T>*> Q;
    Q.enqueue(root);
    while(!Q.empty()){
        BinNode<T>* node = Q.dequeue();
        if(HasLChild(*node)) Q.enqueue(node->lc);
        if(HasRChild(*node)) Q.enqueue(node->rc);
        release(node->data);
        release(node);
    }
}

template<typename T>
int BinTree<T>::updateHeight(BinNode<T>* x){
    return Algorithm::updateHeightImpl(x);
}

template<typename T>
void BinTree<T>::updateHeightAbove(BinNode<T>* x){
    Algorithm::updateHeightAboveImpl(x);
}

template<typename T>
BinNode<T>*
BinTree<T>::insertAsRoot(T const& e){
    _size = 1;
    return _root = new BinNode<T>(e);
}

template<typename T>
BinNode<T>* 
BinTree<T>::insertAsRC(BinNode<T>* x, T const& e){
    _size++;
    x->insertAsRC(e);
    updateHeightAbove(x);
    return x->rc;
}

template<typename T>
BinNode<T>*
BinTree<T>::insertAsLC(BinNode<T>* x, T const& e){
    _size++;
    x->insertAsLC(e);
    updateHeightAbove(x);
    return x->lc;
}

template<typename T>
BinNode<T>*
BinTree<T>::attachAsRC(BinNode<T>* x, BinTree<T>*& S){
    BinTree<T>* temp = secede(x->rc);

    if((x->rc = S->_root)) {
        x->rc->parent = x;
    }

    if(temp)
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
BinNode<T>*
BinTree<T>::attachAsLC(BinNode<T>* x, BinTree<T>*& S){
    BinTree<T>* temp = secede(x->rc);

    if((x->lc = S->_root)) {
        x->lc->parent = x;
    }

    if(temp)
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
static int removeAt(BinNode<T>* x){
    if(!x) return 0;
    int n = 1 + removeAt(x->lc) + removeAt(x->rc);
    release(x->data);
    release(x);
    return n;
}

template<typename T>
BinNode<T>*& 
BinTree<T>::FromParentTo(const BinNode<T>& node){
    return Algorithm::fromParentToImpl(node);
}

template<typename T>
int BinTree<T>::remove(BinNode<T>* x){
    this->FromParentTo(*x) = nullptr;
    updateHeightAbove(x->parent);
    int n = removeAt(x);
    _size -= n;
    return n;
}

template<typename T>
BinTree<T>* 
BinTree<T>::secede(BinNode<T>* x){
    if(x == nullptr)
        return nullptr;
    this->FromParentTo(*x) = nullptr;
    updateHeightAbove(x->parent);
    BinTree<T>* S = new BinTree<T>();
    S->_root = x;
    x->parent = nullptr;
    S->_size = x->size();
    _size -= S->_size;
    return S;
}

template<typename T>
struct BinTree<T>::iterator {
    BinNode<T>* cur;

    explicit iterator(BinNode<T>* rhs)
        : cur{rhs} {}
    
    bool operator!=(const iterator& other){
        return cur != other.cur;
    }

    T& operator*() { 
        return cur->data; 
    }

    iterator& operator++()
    {
        cur = cur->succ();
        return *this;
    }
};

template<typename T>
typename BinTree<T>::iterator
BinTree<T>::begin() {
    BinNode<T>* n = _root;

    if (n) 
        while(n->lc)
            n = n->lc;
    return iterator{n};
}

template<typename T>
typename BinTree<T>::iterator
BinTree<T>::end() {
    return iterator{nullptr};
}

template<typename T>
template<typename Res, typename Agg>
Res BinTree<T>::rangeAggregateRec(BinNode<T>* x, const T& lo, const T& hi, Res acc, Agg&& agg) const {
    return Algorithm::rangeAggregateRecImpl(x, lo, hi, acc, agg);
}

#endif
