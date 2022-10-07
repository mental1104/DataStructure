/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> //有序列表的归并：当前列表中自p起的n个元素，与列表L中自q起的m个元素归并
ListNodePosi<T> List<T>::merge ( ListNodePosi<T> p, int n, List<T> & L, ListNodePosi<T> q, int m ) {
// assert:  this.valid(p) && rank(p) + n <= size && this.sorted(p, n)
//          L.valid(q) && rank(q) + m <= L._size && L.sorted(q, m)
// 注意：在被mergeSort()调用时，this == &L && rank(p) + n = rank(q)
   ListNodePosi<T> pp = p->pred; //归并之后p可能不再指向首节点，故需先记忆，以便在返回前更新
   while ( ( 0 < m ) && ( q != p ) ) //q尚未出界（或在mergeSort()中，p亦尚未出界）之前
      if ( ( 0 < n ) && ( p->data <= q->data ) ) //若p尚未出界且v(p) <= v(q)，则
         { p = p->succ; n--; } //p直接后移，即完成归入
      else //否则，将q转移至p之前，以完成归入
         { insert ( L.remove ( ( q = q->succ )->pred ), p ); m--; }
   return pp->succ; //更新的首节点
}