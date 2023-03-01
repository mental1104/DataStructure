/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

#define ROOT 0
#define L_CHILD 1
#define R_CHILD -1*L_CHILD

/******************************************************************************************
 * 打印输出PQ_ComplHeap
 ******************************************************************************************/
template <typename T> //元素类型
void UniPrint::p ( PQ_ComplHeap<T> & pq ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( pq ).name(), reinterpret_cast<int*>(&pq), pq.size()); //基本信息
   int branchType[256]; //最深256层 <= 2^256 = 1.16*10^77
   printComplHeap (pq, pq.size(), 0, 0, ROOT, branchType ); //树状结构
   printf ( "\n" );
}

/******************************************************************************************
 * 递归打印输出
 ******************************************************************************************/
template<typename T> //元素类型
void UniPrint::printComplHeap (PQ_ComplHeap<T>& pq, int n, int k, int depth, int type, int* bType ) {
   if ( k >= n ) return; //递归基
   bType[depth] = type;
   printComplHeap ( pq, n, pq.RChild(k), depth + 1, R_CHILD, bType ); //右子树（在上）
   print ( pq._elem[k] ); ( 0 < k ) && ( pq._elem[pq.Parent(k)] < pq._elem[k] ) ? printf ( "X" ) : printf ( " " ); printf ( "*" );
   for ( int i = 0; i < depth; i++ ) //根据相邻各层
      if ( bType[i] + bType[i+1] ) //的拐向是否一致，即可确定
         printf ( "      " ); //是否应该
      else  printf ( "│    " ); //打印横线
   switch ( type ) {
      case  R_CHILD  :  printf ( "┌─" );  break;
      case  L_CHILD  :  printf ( "└─" );  break;
      default        :  printf ( "──" );  break; //root
   }
   print(pq._elem[k]); ( 0 < k ) && ( pq._elem[pq.Parent ( k ) ] < pq._elem[k] ) ? printf ( "X" ) : printf ( " " ); printf ( "\n" );
   printComplHeap (pq, n, pq.LChild(k), depth + 1, L_CHILD, bType ); //左子树（在下）
}
