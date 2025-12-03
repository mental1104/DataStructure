#ifndef __DSA_SKEW_HEAP
#define __DSA_SKEW_HEAP

#include <stdexcept>
#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"

template <typename T, bool MAX = true>
class SkewHeap : public PQ<T, MAX>, public BinTree<T> { //基于二叉树，以斜堆形式实现的PQ
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    BinNode<T>* merge(BinNode<T>* a, BinNode<T>* b);
public:
    SkewHeap() { } //默认构造
    SkewHeap ( T* E, int n ) //批量构造
    {  for ( int i = 0; i < n; i++ ) insert ( E[i] );  }
    SkewHeap(Vector<T>& vec){
        for (int i = 0; i < vec.size(); i++)
            insert(vec[i]);
    }
   
    void merge(SkewHeap<T, MAX>& right);
    void insert(T); //按照比较器确定的优先级次序插入元素
    T getMax(); //取出优先级最高的元素
    T delMax(); //删除优先级最高的元素
}; //SkewHeap

template <typename T, bool MAX> //合并以a和b为根节点的两个斜堆
BinNode<T>* SkewHeap<T, MAX>::merge(BinNode<T>* a, BinNode<T>* b) {
    if ( !a ) return b;
    if ( !b ) return a;
    if ( Priority<T, MAX>::higher(b->data, a->data) )
        swap ( a, b ); //一般情况：首先确保a优先

    a->rc = merge(a->rc, b); //将a的右子堆，与b合并
    if (a->rc) a->rc->parent = a;

    swap (a->lc, a->rc); //斜堆特性：每次合并后交换左右子堆
    return a; //返回合并后的堆顶
} //本算法只实现结构上的合并，堆的规模须由上层调用者负责更新

template <typename T, bool MAX> 
void SkewHeap<T, MAX>::insert (T e){
   this->_root = merge(this->_root, new BinNode<T>(e, nullptr)); //将e封装为斜堆，与当前斜堆合并
   this->_size++; //更新规模
}

template <typename T, bool MAX> 
T SkewHeap<T, MAX>::getMax(){
    if (!this->_root) 
        throw std::runtime_error("Heap is empty");
    return this->_root->data; 
} //按照此处约定，堆顶即优先级最高的词条

template <typename T, bool MAX> 
T SkewHeap<T, MAX>::delMax() {
   if (!this->_root) 
      throw std::runtime_error("Heap is empty");
   BinNode<T>* lHeap = this->_root->lc; if (lHeap) lHeap->parent = NULL; //左子堆
   BinNode<T>* rHeap = this->_root->rc; if (rHeap) rHeap->parent = NULL; //右子堆
   T e = this->_root->data; 
   delete this->_root; 
   this->_size--; //删除根节点
   this->_root = merge ( lHeap, rHeap ); //合并原左、右子堆
   return e; //返回原根节点的数据项
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::merge(SkewHeap<T, MAX>& right){
    this->_root = merge(this->_root, right._root);
    right._root = nullptr;
    this->_size += right._size;
    right._size = 0;
    return;
}

#endif
