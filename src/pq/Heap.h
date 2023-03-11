#pragma once 

#include "PQ.h"  
#include "Vector.h"

template <typename T> 
class Heap : public PQ<T>, public Vector<T> { //完全二叉堆
   /*DSA*/friend class UniPrint; //演示输出使用，否则不必设置友类
private:
    Rank Parent(Rank i){    return (i - 1)/2;   }
    Rank LChild(Rank i){    return i*2 + 1;     }//PQ[i]的左孩子
    Rank RChild(Rank i){    return (i + 1)*2;   }////PQ[i]的右孩子
    bool InHeap(Rank n, Rank i){    return -1 < i && i < n; }//判断PQ[i]是否合法
    bool LChildValid(Rank n, Rank i){   return InHeap(n, LChild(i));    }
    bool RChildValid(Rank n, Rank i){   return InHeap(n, RChild(i));    }
    Rank Bigger(Rank i, Rank j);
    Rank ProperParent(Rank n, Rank i);

    void heapify ( Rank n ); //Floyd建堆算法
    Rank percolateDown (Rank n, Rank i ); //下滤
    Rank percolateUp (Rank i );//上滤
public:
    Heap() {} //默认构造
    Heap(Vector<T>& vec):Vector<T>::Vector(vec){
        heapify(vec.size());
    }
    Heap ( T* A, Rank n ) { this->copyFrom ( A, 0, n ); heapify (n); } //批量构造

    void insert (T); //按照比较器确定的优先级次序，插入词条
    T getMax(); //读取优先级最高的词条
    T delMax(); //删除优先级最高的词条
}; //PQ_ComplHeap

template<typename T> 
Rank Heap<T>::Bigger(Rank i, Rank j){
    if(this->_elem[i] < this->_elem[j])
        return j;
    else 
        return i;
}

template<typename T> 
Rank Heap<T>::ProperParent(Rank n, Rank i){/*父子（至多）三者中的大者*/
    if(RChildValid(n, i))
        return Bigger(Bigger(i, LChild(i)), RChild(i));
    else if(LChildValid(n, i))
        return Bigger(i, LChild(i));
    else 
        return i;//相等时父节点优先，如此可避免不必要的交换
}


template <typename T> 
void Heap<T>::heapify(const Rank n ) { //Floyd建堆算法，O(n)时间
   for ( int i = n/2 - 1; 0 <= i; i-- ) //自底而上，依次
      percolateDown (n, i ); //下滤各内部节点
}

template <typename T> 
Rank Heap<T>::percolateUp (Rank i) {
    while ( 0 < i ) { //在抵达堆顶之前，反复地
        Rank j = Parent(i); //考查[i]之父亲[j]
        if (this->_elem[i] < this->_elem[j]) 
            break; //一旦父子顺序，上滤旋即完成；否则

        swap (this->_elem[i], this->_elem[j]); i = j; //父子换位，并继续考查上一层
   } //while
   return i; //返回上滤最终抵达的位置
}

template <typename T> 
Rank Heap<T>::percolateDown(Rank n, Rank i) {
   Rank j; //i及其（至多两个）孩子中，堪为父者
   while (i != (j = ProperParent(n, i ))){ 
       swap(this->_elem[i], this->_elem[j] ); 
       i = j; 
   } //二者换位，并继续考查下降后的i
   return i; //返回下滤抵达的位置（亦i亦j）
}

template <typename T> 
void Heap<T>::insert(T e){ //将词条插入完全二叉堆中
   Vector<T>::insert ( e ); //首先将新词条接至向量末尾
   percolateUp(this->_size - 1); //再对该词条实施上滤调整
}

template <typename T> 
T Heap<T>::getMax(){  
    return this->_elem[0];  
} //取优先级最高的词条

template <typename T> 
T Heap<T>::delMax() { //删除非空完全二叉堆中优先级最高的词条
   T maxElem = this->_elem[0]; 
   this->_elem[0] = this->_elem[--this->_size]; //摘除堆顶（首词条），代之以末词条
   percolateDown (this->_size, 0); //对新堆顶实施下滤
   return maxElem; //返回此前备份的最大词条
}
