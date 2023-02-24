/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

#define Put( K, s, t ) { if ( 1 < (t) - (s) ) { K.push(s); K.push(t); } }
#define Get( K, s, t ) { t = K.pop(); s = K.pop(); }

template <typename T> //������������
void Vector<T>::quickSort( Rank lo, Rank hi ) { //0 <= lo < hi <= size
   Stack<Rank> Task; Put( Task, lo, hi );
   while ( !Task.empty() ) {
      Get( Task, lo, hi );
      /* DSA */ //printf ( "\tQUICKsort: " ); for ( Rank i = 0; i < Task.size(); i+=2 ) printf ( " " ); printf ( " [%3d, %3d)\n", lo, hi );
      Rank mi = partition( lo, hi ); //��[lo, hi)�ڹ������
      if ( mi - lo < hi - mi ) {
         Put( Task, mi+1, hi ); Put( Task, lo, mi );
      } else {
         Put( Task, lo, mi ); Put( Task, mi+1, hi );
      }
   } //������������ջ��С�������ȳ�ջִ�У����ɱ�֤�ݹ���ȣ��ռ�ɱ�������O(logn)
}