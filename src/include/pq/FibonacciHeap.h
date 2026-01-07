#ifndef __DSA_FIBONACCI_HEAP
#define __DSA_FIBONACCI_HEAP

#include <stdexcept>
#include "BinTree.h"
#include "PQ.h"

template <typename T, bool MAX = true>
class FibonacciHeap : public PQ<T, MAX>, public BinTree<T> { //基于二叉树与兄弟链表实现的斐波那契堆（双顶可选）
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    BinNode<T>* _bestRoot{nullptr}; //当前根表中的最优根

    BinNode<T>* mergeTrees(BinNode<T>* a, BinNode<T>* b); //按度相同合并两棵树，保留较大者为根
    void consolidate(); //根表合并，保持各度唯一
public:
    FibonacciHeap() {} //默认构造
    FibonacciHeap ( T* E, int n ) { for ( int i = 0; i < n; i++ ) insert ( E[i] ); }
    FibonacciHeap(Vector<T>& vec){
        for (int i = 0; i < vec.size(); i++)
            insert(vec[i]);
    }

    void merge(FibonacciHeap<T, MAX>& right); //合并两个堆
    void insert(T); //按照比较器确定的优先级次序插入元素
    T getMax(); //取出优先级最高的元素
    T delMax(); //删除优先级最高的元素
}; //FibonacciHeap

template <typename T, bool MAX> //将度数相同的两棵树合并，返回新根
BinNode<T>* FibonacciHeap<T, MAX>::mergeTrees(BinNode<T>* a, BinNode<T>* b) {
    if (!a) return b;
    if (!b) return a;
    if (Priority<T, MAX>::higher(b->data, a->data)) swap(a, b); //保持a为更优根

    // b 挂到 a 的孩子链表，使用 rc 作为兄弟指针
    b->parent = a;
    b->rc = a->lc;
    if (b->rc) b->rc->parent = a;
    a->lc = b;
    a->height++; //height字段作为度数使用
    return a;
} //规模由调用者负责更新

template <typename T, bool MAX> 
void FibonacciHeap<T, MAX>::insert (T e){
   BinNode<T>* node = new BinNode<T>(e, nullptr);
   node->height = 0; //度数初始化为0
   node->color = RBColor::RED; //未标记
   node->rc = this->_root; //加入根表
   this->_root = node;
    if (!_bestRoot || Priority<T, MAX>::higher(node->data, _bestRoot->data)) _bestRoot = node;
   this->_size++; //更新规模
}

template <typename T, bool MAX> 
DSA_NOINLINE T FibonacciHeap<T, MAX>::getMax(){
    if (!_bestRoot) 
        throw std::runtime_error("Heap is empty");
    return _bestRoot->data; 
}

template <typename T, bool MAX> 
T FibonacciHeap<T, MAX>::delMax() {
   if (!_bestRoot) 
      throw std::runtime_error("Heap is empty");

   BinNode<T>* maxNode = _bestRoot;

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
   _bestRoot = nullptr;
   if (this->_root) consolidate(); //合并根表以维护度数唯一并更新最大值
   return e; //返回原根节点的数据项
}

template <typename T, bool MAX> //根表整理：对相同度数的树两两合并
void FibonacciHeap<T, MAX>::consolidate() {
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
    _bestRoot = nullptr;
    for (int i = 0; i < bound; i++) {
        if (table[i]) {
            table[i]->parent = nullptr;
            table[i]->rc = this->_root;
            this->_root = table[i];
            if (!_bestRoot || Priority<T, MAX>::higher(table[i]->data, _bestRoot->data))
                _bestRoot = table[i];
        }
    }
    delete [] table;
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::merge(FibonacciHeap<T, MAX>& right){
    if (!right._root) return;
    if (!this->_root) {
        this->_root = right._root;
        _bestRoot = right._bestRoot;
    } else {
        //将右堆根表接到当前根表前端
        BinNode<T>* tail = right._root;
        while (tail->rc) tail = tail->rc;
        tail->rc = this->_root;
        this->_root = right._root;
        if (_bestRoot && right._bestRoot && Priority<T, MAX>::higher(right._bestRoot->data, _bestRoot->data))
            _bestRoot = right._bestRoot;
        else if (!_bestRoot)
            _bestRoot = right._bestRoot;
    }
    this->_size += right._size;
    right._root = nullptr;
    right._bestRoot = nullptr;
    right._size = 0;
}

#endif
