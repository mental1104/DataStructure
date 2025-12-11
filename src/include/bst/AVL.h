#ifndef __DSA_AVL
#define __DSA_AVL

#include "BST.h" 

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
            BinNode<T>*& parentLink = this->FromParentTo(*g); // C++14 下赋值号两边求值顺序未定义，先拿到父链接再旋转以避免未定义行为
            g = parentLink = this->rotateAt(tallerChild(tallerChild(g)));
        }
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
        if(!AvlBalanced(*g)) {
            BinNode<T>*& parentLink = this->FromParentTo(*g); // 同上：先抓父链接，再旋转，避免未定义的求值顺序污染树结构
            g = parentLink = this->rotateAt(tallerChild(tallerChild(g)));
        }
        this->updateHeight(g);
    }
    return true;
}


#endif
