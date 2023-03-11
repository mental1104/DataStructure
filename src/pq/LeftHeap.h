#pragma once
#include "BinTree.h"

/*
template <typename T> //根据相对优先级确定适宜的方式，合并以a和b为根节点的两个左式堆
static BinNode<T>* merge(BinNode<T>* a, BinNode<T>* b) {
    if ( ! a ) return b; //退化情况
    if ( ! b ) return a; //退化情况
    if ( a->data < b->data ) 
        swap ( a, b ); //一般情况：首先确保b不大

    a->rc = merge(a->rc, b); //将a的右子堆，与b合并
    a->rc->parent = a;

    if ( !a->lc || a->lc->npl < a->rc->npl ) //若有必要
        swap (a->lc, a->rc); //交换a的左、右子堆，以确保右子堆的npl不大
    a->npl = a->rc ? a->rc->npl + 1 : 1; //更新a的npl
    return a; //返回合并后的堆顶
} //本算法只实现结构上的合并，堆的规模须由上层调用者负责更新*/

template <typename T>
class LeftHeap : public PQ<T>, public BinTree<T> { //基于二叉树，以左式堆形式实现的PQ
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    BinNode<T>* merge(BinNode<T>* a, BinNode<T>* b);
public:
    LeftHeap() { } //默认构造
    LeftHeap ( T* E, int n ) //批量构造：可改进为Floyd建堆算法
    {  for ( int i = 0; i < n; i++ ) insert ( E[i] );  }
    LeftHeap(Vector<T>& vec){
        for (int i = 0; i < vec.size(); i++)
            insert(vec[i]);
    }
   
    void merge(LeftHeap<T>& right);
    void insert(T); //按照比较器确定的优先级次序插入元素
    T getMax(); //取出优先级最高的元素
    T delMax(); //删除优先级最高的元素
}; //LeftHeap

template <typename T> //根据相对优先级确定适宜的方式，合并以a和b为根节点的两个左式堆
BinNode<T>* LeftHeap<T>::merge(BinNode<T>* a, BinNode<T>* b) {
    if ( ! a ) return b; //退化情况
    if ( ! b ) return a; //退化情况
    if ( a->data < b->data ) 
        swap ( a, b ); //一般情况：首先确保b不大

    a->rc = merge(a->rc, b); //将a的右子堆，与b合并
    a->rc->parent = a;

    if ( !a->lc || a->lc->npl < a->rc->npl ) //若有必要
        swap (a->lc, a->rc); //交换a的左、右子堆，以确保右子堆的npl不大
    a->npl = a->rc ? a->rc->npl + 1 : 1; //更新a的npl
    return a; //返回合并后的堆顶
} //本算法只实现结构上的合并，堆的规模须由上层调用者负责更新

template <typename T> 
void LeftHeap<T>::insert (T e){
   this->_root = merge(this->_root, new BinNode<T>(e, nullptr)); //将e封装为左式堆，与当前左式堆合并
   this->_size++; //更新规模
}

template <typename T> 
T LeftHeap<T>::getMax(){ 
    return this->_root->data; 
} //按照此处约定，堆顶即优先级最高的词条

template <typename T> 
T LeftHeap<T>::delMax() {
   BinNode<T>* lHeap = this->_root->lc; if (lHeap) lHeap->parent = NULL; //左子堆
   BinNode<T>* rHeap = this->_root->rc; if (rHeap) rHeap->parent = NULL; //右子堆
   T e = this->_root->data; 
   delete this->_root; 
   this->_size--; //删除根节点
   this->_root = merge ( lHeap, rHeap ); //合并原左、右子堆
   return e; //返回原根节点的数据项
}

template<typename T>
void LeftHeap<T>::merge(LeftHeap<T>& right){
    merge(this->_root, right._root);
    right._root = nullptr;
    this->_size += right._size;
    right._size = 0;
    return;
}