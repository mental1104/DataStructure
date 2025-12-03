#ifndef __DSA_FIBONACCI_HEAP
#define __DSA_FIBONACCI_HEAP

#include <stdexcept>
#include "BinTree.h"
#include "PQ.h"

template <typename T>
class FibonacciHeap : public PQ<T>, public BinTree<T> { //基于二叉树与兄弟链表实现的斐波那契堆（最大堆）
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    BinNode<T>* _maxRoot{nullptr}; //当前根表中的最大根

    BinNode<T>* mergeTrees(BinNode<T>* a, BinNode<T>* b); //按度相同合并两棵树，保留较大者为根
    void consolidate(); //根表合并，保持各度唯一
public:
    FibonacciHeap() {} //默认构造
    FibonacciHeap ( T* E, int n ) { for ( int i = 0; i < n; i++ ) insert ( E[i] ); }
    FibonacciHeap(Vector<T>& vec){
        for (int i = 0; i < vec.size(); i++)
            insert(vec[i]);
    }

    void merge(FibonacciHeap<T>& right); //合并两个堆
    void insert(T); //按照比较器确定的优先级次序插入元素
    T getMax(); //取出优先级最高的元素
    T delMax(); //删除优先级最高的元素
}; //FibonacciHeap

template <typename T> //将度数相同的两棵树合并，返回新根
BinNode<T>* FibonacciHeap<T>::mergeTrees(BinNode<T>* a, BinNode<T>* b) {
    if (!a) return b;
    if (!b) return a;
    if (a->data < b->data) swap(a, b); //保持a为较大根

    // b 挂到 a 的孩子链表，使用 rc 作为兄弟指针
    b->parent = a;
    b->rc = a->lc;
    if (b->rc) b->rc->parent = a;
    a->lc = b;
    a->height++; //height字段作为度数使用
    return a;
} //规模由调用者负责更新

template <typename T> 
void FibonacciHeap<T>::insert (T e){
   BinNode<T>* node = new BinNode<T>(e, nullptr);
   node->height = 0; //度数初始化为0
   node->color = RBColor::RED; //未标记
   node->rc = this->_root; //加入根表
   this->_root = node;
   if (!_maxRoot || _maxRoot->data < node->data) _maxRoot = node;
   this->_size++; //更新规模
}

template <typename T> 
T FibonacciHeap<T>::getMax(){
    if (!_maxRoot) 
        throw std::runtime_error("Heap is empty");
    return _maxRoot->data; 
} //按照此处约定，堆顶即优先级最高的词条

template <typename T> 
T FibonacciHeap<T>::delMax() {
   if (!_maxRoot) 
      throw std::runtime_error("Heap is empty");

   BinNode<T>* maxNode = _maxRoot;

   //从根表删除 maxNode
   BinNode<T>* prev = nullptr;
   BinNode<T>* iter = this->_root;
   while (iter && iter != maxNode) {
       prev = iter;
       iter = iter->rc;
   }
   if (iter == maxNode) {
       if (prev) prev->rc = maxNode->rc;
       else this->_root = maxNode->rc;
   }

   //将子节点提升到根表
   BinNode<T>* child = maxNode->lc;
   while (child) {
       BinNode<T>* next = child->rc;
       child->parent = nullptr;
       child->rc = this->_root;
       child->color = RBColor::RED;
       this->_root = child;
       child = next;
   }

   T e = maxNode->data; 
   delete maxNode; 
   this->_size--; //删除根节点
   _maxRoot = nullptr;
   if (this->_root) consolidate(); //合并根表以维护度数唯一并更新最大值
   return e; //返回原根节点的数据项
}

template <typename T> //根表整理：对相同度数的树两两合并
void FibonacciHeap<T>::consolidate() {
    if (!this->_root) return;
    int bound = 0;
    int n = this->_size <= 0 ? 1 : this->_size;
    while ((1 << bound) <= n) bound++;
    bound += 2; //安全余量

    BinNode<T>** table = new BinNode<T>*[bound];
    for (int i = 0; i < bound; i++) table[i] = nullptr;

    BinNode<T>* iter = this->_root;
    while (iter) {
        BinNode<T>* x = iter;
        iter = iter->rc;
        x->rc = nullptr; //摘下，便于重新接回
        int d = x->height;
        while (table[d]) {
            x = mergeTrees(x, table[d]);
            table[d] = nullptr;
            d = x->height;
        }
        table[d] = x;
    }

    this->_root = nullptr;
    _maxRoot = nullptr;
    for (int i = 0; i < bound; i++) {
        if (table[i]) {
            table[i]->parent = nullptr;
            table[i]->rc = this->_root;
            this->_root = table[i];
            if (!_maxRoot || _maxRoot->data < table[i]->data)
                _maxRoot = table[i];
        }
    }
    delete [] table;
}

template<typename T>
void FibonacciHeap<T>::merge(FibonacciHeap<T>& right){
    if (!right._root) return;
    if (!this->_root) {
        this->_root = right._root;
        _maxRoot = right._maxRoot;
    } else {
        //将右堆根表接到当前根表前端
        BinNode<T>* tail = right._root;
        while (tail->rc) tail = tail->rc;
        tail->rc = this->_root;
        this->_root = right._root;
        if (_maxRoot && right._maxRoot && _maxRoot->data < right._maxRoot->data)
            _maxRoot = right._maxRoot;
        else if (!_maxRoot)
            _maxRoot = right._maxRoot;
    }
    this->_size += right._size;
    right._root = nullptr;
    right._maxRoot = nullptr;
    right._size = 0;
}

#endif
