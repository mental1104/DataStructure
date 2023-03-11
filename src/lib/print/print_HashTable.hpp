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

template <typename K, typename V> //e��value
void UniPrint::p ( Hashtable<K, V>& ht ) { //����
   printf ( "%s[%p]*(%d)/%d:\n", typeid ( ht ).name(), reinterpret_cast<unsigned*>(&ht), ht._N(), ht._M() ); //������Ϣ
   for ( int i = 0; i < ht._M(); i++ ) //���Ͱ���
      printf ( "  %4d  ", i );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //�������Ԫ��
      if ( ht._ht(i) ) printf ( "-<%04d>-", ht._ht(i)->key ); //��ʾ�ã���������int
      else if ( ht._lazyRemoval()->test(i) ) printf ( "-<****>-" );
      else printf ( "--------" );
   printf ( "\n" );
   /*
   for ( int i = 0; i < ht._M(); i++ ) //�������Ԫ��
      if ( ht._ht(i) ) printf ( "    %c   ", ht._ht(i)->value ); //��ʾ�ã���������char
//      if (ht.ht[i]) printf("%8s", ht.ht[i]->value); //���Huffman������ʹ�õ�ɢ�б�
      else if ( ht._lazyRemoval()->test(i) ) printf ( "    *   " );
      else printf ( "        " );
   printf ( "\n" );
   */
}

template <typename K, typename V> //e��value
void UniPrint::p ( QuadraticHT<K, V>& ht ) { //����
   printf ( "%s[%p]*(%d)/%d:\n", typeid ( ht ).name(), reinterpret_cast<unsigned*>(&ht), ht._N(), ht._M() ); //������Ϣ
   for ( int i = 0; i < ht._M(); i++ ) //���Ͱ���
      printf ( "  %4d  ", i );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //�������Ԫ��
      if ( ht._ht(i) ) printf ( "-<%04d>-", ht._ht(i)->key ); //��ʾ�ã���������int
      else if ( ht._lazyRemoval()->test(i) ) printf ( "-<****>-" );
      else printf ( "--------" );
   printf ( "\n" );
   /*
   for ( int i = 0; i < ht._M(); i++ ) //�������Ԫ��
      if ( ht._ht(i) ) printf ( "    %c   ", ht._ht(i)->value ); //��ʾ�ã���������char
//      if (ht.ht[i]) printf("%8s", ht.ht[i]->value); //���Huffman������ʹ�õ�ɢ�б�
      else if ( ht._lazyRemoval()->test(i) ) printf ( "    *   " );
      else printf ( "        " );
   printf ( "\n" );*/
}