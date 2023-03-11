#pragma once
#include "Vector.h"  

template<typename T>
struct BTNode {
    BTNode<T>* parent{nullptr};
    Vector<T> key;
    Vector<BTNode<T>*> child;//长度总比key多一
    BTNode() { child.insert(0, nullptr);}
    BTNode(T e, BTNode<T>* lc = nullptr, BTNode<T>* rc = nullptr) {
        parent = nullptr;
        key.insert(0, e);
        child.insert(0, lc);
        child.insert(1, rc);
        if(lc) lc->parent = this;
        if(rc) rc->parent = this;
    }
};