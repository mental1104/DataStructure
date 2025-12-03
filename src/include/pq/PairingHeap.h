#ifndef __DSA_PAIRING_HEAP
#define __DSA_PAIRING_HEAP

#include <stdexcept>
#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"

template <typename T>
class PairingHeap : public PQ<T>, public BinTree<T> { //基于二叉树，以配对堆形式实现的PQ（左孩子-右兄弟）
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    BinNode<T>* merge(BinNode<T>* a, BinNode<T>* b);          //合并两个堆根
    BinNode<T>* mergePairs(BinNode<T>* firstSibling);         //配对合并兄弟链
public:
    PairingHeap() {} //默认构造
    PairingHeap ( T* E, int n ) { for ( int i = 0; i < n; i++ ) insert ( E[i] ); }
    PairingHeap(Vector<T>& vec){
        for (int i = 0; i < vec.size(); i++)
            insert(vec[i]);
    }

    void merge(PairingHeap<T>& right); //合并另一配对堆
    void insert(T); //按照比较器确定的优先级次序插入元素
    T getMax(); //取出优先级最高的元素
    T delMax(); //删除优先级最高的元素
}; //PairingHeap

template <typename T> //合并以a和b为根节点的两个配对堆
BinNode<T>* PairingHeap<T>::merge(BinNode<T>* a, BinNode<T>* b) {
    if ( !a ) return b;
    if ( !b ) return a;
    if ( a->data < b->data ) { //保持a为较大根
        BinNode<T>* tmp = a; a = b; b = tmp;
    }

    //将b挂为a的新的最左孩子，使用右兄弟指针串联
    b->parent = a;
    b->rc = a->lc;
    if (b->rc) b->rc->parent = a;
    a->lc = b;
    return a;
} //规模由调用者负责更新

template <typename T> //两两配对合并兄弟链
BinNode<T>* PairingHeap<T>::mergePairs(BinNode<T>* firstSibling) {
    if ( !firstSibling ) return nullptr;
    if ( !firstSibling->rc ) return firstSibling;

    BinNode<T>* a = firstSibling;
    BinNode<T>* b = firstSibling->rc;
    BinNode<T>* rest = b->rc;
    a->rc = b->rc = nullptr; //断开兄弟指针，避免污染后续结构

    BinNode<T>* merged = merge(a, b);
    return merge(merged, mergePairs(rest));
}

template <typename T> 
void PairingHeap<T>::insert (T e){
   this->_root = merge(this->_root, new BinNode<T>(e, nullptr)); //将e封装为配对堆，与当前配对堆合并
   this->_size++; //更新规模
}

template <typename T> 
T PairingHeap<T>::getMax(){
    if (!this->_root) 
        throw std::runtime_error("Heap is empty");
    return this->_root->data; 
} //按照此处约定，堆顶即优先级最高的词条

template <typename T> 
T PairingHeap<T>::delMax() {
   if (!this->_root) 
      throw std::runtime_error("Heap is empty");

   //取出子链表
   BinNode<T>* child = this->_root->lc;
   T e = this->_root->data; 
   delete this->_root; 
   this->_size--; //删除根节点

   //断开孩子与旧父的关系，避免悬挂parent
   BinNode<T>* iter = child;
   while (iter) {
       iter->parent = nullptr;
       iter = iter->rc;
   }

   this->_root = mergePairs(child); //配对合并兄弟链
   return e; //返回原根节点的数据项
}

template<typename T>
void PairingHeap<T>::merge(PairingHeap<T>& right){
    this->_root = merge(this->_root, right._root);
    right._root = nullptr;
    this->_size += right._size;
    right._size = 0;
    return;
}

#endif
