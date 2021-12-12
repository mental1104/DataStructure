#pragma once 

#include "./BST.hpp" 

template<typename T>
class AVL: public BST<T> {
public:
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
BinNode<T>* 
AVL<T>::insert(const T& e){
    BinNode<T>*& x = this->search(e);
    if(x) return x;
    BinNode<T>* xx = x = new BinNode<T>(e, this->_hot);
    this->_size++;
    for(BinNode<T>* g = this->_hot; g; g = g->parent){
        if(!AvlBalanced(*g)){
            this->FromParentTo(*g) = this->rotateAt(tallerChild(tallerChild(g)));
            break;
        } else
            this->updateHeight(g);
    }
    return xx;
}

template<typename T>
bool AVL<T>::remove(const T& e){
    BinNode<T>*& x = this->search(e);
    if(!x)
        return false;
    removeAt(x, this->_hot);
    this->_size--;
    for(BinNode<T>* g = this->_hot; g; g = g->parent){
        if(!AvlBalanced(*g))
            g = this->FromParentTo(*g) = this->rotateAt(tallerChild(tallerChild(g)));
        this->updateHeight(g);
    }
    return true;
}
