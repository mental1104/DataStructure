/*
 * @Author: mental1104 mental1104@gmail.com
 * @Date: 2023-05-06 19:43:26
 * @LastEditors: mental1104 mental1104@gmail.com
 * @LastEditTime: 2023-05-13 23:28:50
 * @FilePath: /espeon/code/DataStructure/src/include/pq/Heap.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DSA_HEAP
#define __DSA_HEAP

#include "PQ.h"  
#include "Vector.h"

template <typename T, bool MAX = true> 
class Heap : public PQ<T, MAX>, public Vector<T> { //完全二叉堆include_directories(${TOP_DIR}/src/include)
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

template<typename T, bool MAX> 
Rank Heap<T, MAX>::Bigger(Rank i, Rank j){
    return Priority<T, MAX>::higher(this->_elem[j], this->_elem[i]) ? j : i;
}

template<typename T, bool MAX> 
Rank Heap<T, MAX>::ProperParent(Rank n, Rank i){/*父子（至多）三者中的更优者*/
    if(RChildValid(n, i))
        return Bigger(Bigger(i, LChild(i)), RChild(i));
    else if(LChildValid(n, i))
        return Bigger(i, LChild(i));
    else 
        return i;//相等时父节点优先，如此可避免不必要的交换
}


template <typename T, bool MAX> 
void Heap<T, MAX>::heapify(const Rank n ) { //Floyd建堆算法，O(n)时间
   for ( int i = n/2 - 1; 0 <= i; i-- ) //自底而上，依次
      percolateDown (n, i ); //下滤各内部节点
}

template <typename T, bool MAX> 
Rank Heap<T, MAX>::percolateUp (Rank i) {
    while ( 0 < i ) { //在抵达堆顶之前，反复地
        Rank j = Parent(i); //考查[i]之父亲[j]
        if (Priority<T, MAX>::higher(this->_elem[j], this->_elem[i])) 
            break; //一旦父子顺序正确，上滤完成；否则

        ::swap (this->_elem[i], this->_elem[j]); i = j; //父子换位，并继续考查上一层（显式用全局swap避免与std::swap产生歧义；Linux/macOS常靠ADL和不同重载集合躲过去，MSVC更严格而报错）
   } //while
   return i; //返回上滤最终抵达的位置
}

template <typename T, bool MAX> 
Rank Heap<T, MAX>::percolateDown(Rank n, Rank i) {
   Rank j; //i及其（至多两个）孩子中，堪为父者
   while (i != (j = ProperParent(n, i ))){ 
       ::swap(this->_elem[i], this->_elem[j] ); // 显式调用全局swap（MSVC重载集更严格会歧义，Linux/macOS通常ADL优先解析到全局版本而未触发）
       i = j; 
   } //二者换位，并继续考查下降后的i
   return i; //返回下滤抵达的位置（亦i亦j）
}

template <typename T, bool MAX> 
void Heap<T, MAX>::insert(T e){ //将词条插入完全二叉堆中
   Vector<T>::insert ( e ); //首先将新词条接至向量末尾
   percolateUp(this->_size - 1); //再对该词条实施上滤调整
}

template <typename T, bool MAX> 
DSA_NOINLINE T Heap<T, MAX>::getMax(){
    return this->_elem[0];
}

template <typename T, bool MAX> 
T Heap<T, MAX>::delMax() { //删除非空完全二叉堆中优先级最高的词条
   T maxElem = this->_elem[0]; 
   this->_elem[0] = this->_elem[--this->_size]; //摘除堆顶（首词条），代之以末词条
   percolateDown (this->_size, 0); //对新堆顶实施下滤
   return maxElem; //返回此前备份的最大词条
}


#endif
