/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> //对列表中起始于位置p、宽度为n的区间做插入排序
void List<T>::insertionSort ( ListNodePosi<T> p, int n ) { //valid(p) && rank(p) + n <= size
   /*DSA*///printf ( "InsertionSort ...\n" );
   for ( int r = 0; r < n; r++ ) { //逐一为各节点
      insert ( search ( p->data, r, p ), p->data ); //查找适当的位置并插入
      p = p->succ; remove ( p->pred ); //转向下一节点
   }
}