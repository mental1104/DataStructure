#ifndef __DSA_SPLAY
#define __DSA_SPLAY

#include "BST.h"

template<typename T>
class Splay : public BST<T>{
protected:
    BinNode<T>* splay(BinNode<T>* v);

public:
    BinNode<T>*& search(const T& e);
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T> inline
void attachAsLChild(BinNode<T>*& p, BinNode<T>*& lc){
    p->lc = lc;
    if(lc)
        lc->parent = p;
}

template<typename T> inline
void attachAsRChild(BinNode<T>*& p, BinNode<T>*& rc){
    p->rc = rc;
    if(rc)
        rc->parent = p;
}

template<typename T>
BinNode<T>* Splay<T>::splay(BinNode<T>* v){
    if(!v)  return nullptr;
    BinNode<T>* p;
    BinNode<T>* g;
    /* 双层向上伸展 */
    while((p = v->parent) && (g = p->parent)){
        BinNode<T>* gg = g->parent;
        if(IsLChild(*v))
            if(IsLChild(*p)){//zig-zig
                attachAsLChild(g, p->rc);  attachAsLChild(p, v->rc);
                attachAsRChild(p, g);      attachAsRChild(v, p);
            } else {         //zig-zag
                attachAsLChild(p, v->rc);  attachAsRChild(g, v->lc);
                attachAsLChild(v, g);      attachAsRChild(v, p);
            }
        else if(IsRChild(*p)){//zag-zag
                attachAsRChild(g, p->lc);  attachAsRChild(p, v->lc);
                attachAsLChild(p, g);      attachAsLChild(v, p);
        } else {              //zag-zig
                attachAsRChild(p, v->lc);  attachAsLChild(g, v->rc);
                attachAsRChild(v, g);      attachAsLChild(v, p);
        }

        if(!gg) 
            v->parent = nullptr;
        else
            (g == gg->lc)?attachAsLChild(gg, v):attachAsRChild(gg, v);
        this->updateHeight(g);
        this->updateHeight(p);
        this->updateHeight(v); 
    }
    /* 树高为奇数时，单层伸展 */
    if((p = v->parent)){
        if(IsLChild(*v)) {  
            attachAsLChild(p, v->rc); 
            attachAsRChild(v, p);
        } else {
            attachAsRChild(p, v->lc);
            attachAsLChild(v, p);
        }
        this->updateHeight(p);
        this->updateHeight(v);
    }
    v->parent = nullptr;
    return v;
}

template<typename T>
BinNode<T>*& Splay<T>::search(const T& e){
    BinNode<T>* p = BST<T>::search(e);
    this->_root = splay(p?p:this->_hot);//若查找失败，则对_hot伸展，方便插入删除算法的实施
    return this->_root;
}

template<typename T>
BinNode<T>* Splay<T>::insert(const T& e){
    if(!this->_root) { 
        this->_size++;
        return this->_root = new BinNode<T>(e);
    }

    if(e == search(e)->data) 
        return this->_root;
    this->_size++;
    BinNode<T>* t = this->_root;

    //若查找元素大于根节点，则作为左孩子插入
    if(this->_root->data < e){
        t->parent = this->_root = new BinNode<T>(e, nullptr, t, t->rc);
        if(HasRChild(*t)){//将原右子树接入根节点
            t->rc->parent = this->_root;
            t->rc = nullptr;
        } 
    } else {//若查找元素小于根节点，则作为右孩子插入
        t->parent = this->_root = new BinNode<T>(e, nullptr, t->lc, t);
        if(HasLChild(*t)){//将原左子树接入根节点
            t->lc->parent = this->_root;
            t->lc = nullptr;
        }
    }
    this->updateHeightAbove(t);
    return this->_root;
}

template<typename T>
bool Splay<T>::remove(const T& e){
    if(!this->_root || (e!=search(e)->data)) 
        return false;
    
    BinNode<T>* w = this->_root;
    if(!HasLChild(*this->_root)){
        this->_root = this->_root->rc; if(this->_root)  this->_root->parent = nullptr;
    } else if(!HasRChild(*this->_root)){
        this->_root = this->_root->lc; if(this->_root)  this->_root->parent = nullptr;
    } else {
        BinNode<T>* lTree = this->_root->lc;
        lTree->parent = nullptr;
        this->_root->lc = nullptr;
        this->_root = this->_root->rc;
        this->_root->parent = nullptr;
        search(w->data);//以原树根为目标，做一次（必定失败的）查找
///// assert: 至此，右子树中最小节点必伸展至根，且（因无雷同节点）其左子树必空，于是
        this->_root->lc = lTree;
        lTree->parent = this->_root;
    }
    release(w->data); 
    release(w);
    this->_size--;
    if(this->_root) this->updateHeight(this->_root);
    return true;
}


#endif