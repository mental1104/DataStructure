/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

/******************************************************************************************
 * 重散列：空桶太少时对散列表重新整理：扩容，再将词条逐一移入新表
 * 散列函数的定址与表长M直接相关，故不可简单地批量复制原桶数组
 ******************************************************************************************/
template <typename K, typename V> void Hashtable<K, V>::rehash() {
   int oldM = M; Entry<K, V>** oldHt = ht;
   M = primeNLT( 4 * N, 1048576, PRIME_TABLE ); //容量至少加倍（若懒惰删除很多，可能反而缩容）
   ht = new Entry<K, V>*[M]; N = 0; memset( ht, 0, sizeof( Entry<K, V>* ) * M ); //桶数组
   release( removed ); removed = new Bitmap( M ); L = 0; //懒惰删除标记
   //*DSA*/printf("A bucket array has been created with capacity = %d\n\n", M);
   for ( int i = 0; i < oldM; i++ ) //扫描原表
      if ( oldHt[i] ) //将每个非空桶中的词条
         put( oldHt[i]->key, oldHt[i]->value ); //转入新表
   release( oldHt ); //释放——因所有词条均已转移，故只需释放桶数组本身
}
