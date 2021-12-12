/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2020. All rights reserved.
 ******************************************************************************************/

#pragma once

/******************************************************************************************
 * Hashtable
 ******************************************************************************************/

template <typename K, typename V> //e、value
void UniPrint::p ( Hashtable<K, V>& ht ) { //引用
   printf ( "%s[%p]*(%d)/%d:\n", typeid ( ht ).name(), reinterpret_cast<unsigned*>(&ht), ht._N(), ht._M() ); //基本信息
   for ( int i = 0; i < ht._M(); i++ ) //输出桶编号
      printf ( "  %4d  ", i );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "-<%04d>-", ht._ht(i)->key ); //演示用，仅适用于int
      else if ( ht._lazyRemoval()->test(i) ) printf ( "-<****>-" );
      else printf ( "--------" );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "    %c   ", ht._ht(i)->value ); //演示用，仅适用于char
//      if (ht.ht[i]) printf("%8s", ht.ht[i]->value); //针对Huffman编码中使用的散列表
      else if ( ht._lazyRemoval()->test(i) ) printf ( "    *   " );
      else printf ( "        " );
   printf ( "\n" );
}
