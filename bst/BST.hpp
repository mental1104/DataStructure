#pragma once

#include "../def.hpp"

template<typename T>
class BST: public BinTree<T> {
protected:
    BinNode<T>* _hot{nullptr};
    BinNode<T>* connect34(
        BinNode<T>*, BinNode<T>*, BinNode<T>*,
        BinNode<T>*, BinNode<T>*, BinNode<T>*, BinNode<T>*
    );
    BinNode<T>* rotateAt(BinNode<T>* x);
public:
    virtual BinNode<T>*& search(const T& e);
    virtual BinNode<T>*  insert(const T& e);
    virtual bool remove(const T& e);
};

template<typename T>
BinNode<T>*&
BST<T>::search(const T& e){
    if ( !this->_root || e == this->_root->data ) { 
        _hot = NULL; 
        return this->_root; 
    } //空树，或恰在树根命中
    
    for ( _hot = this->_root; ; ) { //否则，自顶而下
        BinNode<T>*& v = ( e < _hot->data ) ? _hot->lc : _hot->rc; //确定方向，深入一层
        if ( !v || e == v->data ) 
            return v; //一旦命中或抵达叶子，随即返回
        _hot = v; //返回目标节点位置的引用，以便后续插入、删除操作
   } 
}

template<typename T>
BinNode<T>* 
BST<T>::insert(const T& e){
    BinNode<T>*& x = search(e);
    if(x)
        return x;
    x = new BinNode<T>(e, _hot);
    this->_size++;
    this->updateHeightAbove(x);
    return x;
}

template<typename T>
static BinNode<T>* removeAt(BinNode<T>*& x, BinNode<T>*& hot){
    BinNode<T>* w = x;
    BinNode<T>* succ = nullptr;
    if(!HasLChild(*x))
        succ = x = x->rc;
    else if(!HasRChild(*x))
        succ = x = x->lc;
    else {
        w = w->succ();
        swap(x->data, w->data);
        BinNode<T>* u = w->parent;
        ((u==x)?u->rc:u->lc) = succ = w->rc;
    }
    hot = w->parent;
    if(succ) succ->parent = hot;
    //
    return succ;
}

template<typename T>
bool BST<T>::remove(const T& e){
    BinNode<T>*& x = search(e);
    if(!x) 
        return false;
    removeAt(x ,_hot);
    this->_size--;
    this->updateHeightAbove(_hot);
    return true;
}

template<typename T>
BinNode<T>* 
BST<T>::connect34(
    BinNode<T>* a, BinNode<T>* b, BinNode<T>* c,
    BinNode<T>* T0, BinNode<T>* T1, BinNode<T>* T2, BinNode<T>* T3
){
    a->lc = T0; if ( T0 ) T0->parent = a;
    a->rc = T1; if ( T1 ) T1->parent = a; this->updateHeight ( a );
    c->lc = T2; if ( T2 ) T2->parent = c;
    c->rc = T3; if ( T3 ) T3->parent = c; this->updateHeight ( c );
    b->lc = a; a->parent = b;
    b->rc = c; c->parent = b; this->updateHeight ( b );
    return b; //该子树新的根节点
}

template<typename T>
BinNode<T>* 
BST<T>::rotateAt(BinNode<T>* v){
    /*DSA*/if ( !v ) { printf ( "\a\nFail to rotate a null node\n" ); exit ( -1 ); }
   BinNode<T>* p = v->parent; BinNode<T>* g = p->parent; //视v、p和g相对位置分四种情况
   if ( IsLChild ( *p ) ) /* zig */
      if ( IsLChild ( *v ) ) { /* zig-zig */ 
         p->parent = g->parent; //向上联接
         return connect34 ( v, p, g, v->lc, v->rc, p->rc, g->rc );
      } else { /* zig-zag */  //*DSA*/printf("\tzIg-zAg: ");
         v->parent = g->parent; //向上联接
         return connect34 ( p, v, g, p->lc, v->lc, v->rc, g->rc );
      }
   else  /* zag */
      if ( IsRChild ( *v ) ) { /* zag-zag */
         p->parent = g->parent; //向上联接
         return connect34 ( g, p, v, g->lc, p->lc, v->lc, v->rc );
      } else { /* zag-zig */  //*DSA*/printf("\tzAg-zIg: ");
         v->parent = g->parent; //向上联接
         return connect34 ( g, v, p, g->lc, v->lc, v->rc, p->rc );
      }
}