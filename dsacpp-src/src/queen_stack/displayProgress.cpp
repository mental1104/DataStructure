/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#include "queen_stack.h"

int N = 0; //���̴�С

void displayRow ( Queen& q ) { //��ӡ��ǰ�ʺ󣨷�����col�У�������
   printf ( "%2d: ", q.x );
   int i = 0;
   while ( i++ < q.y ) printf ( "[]" );
   printf ( "��" );
   while ( i++ < N ) printf ( "[]" );
   printf ( "%2d\n", q.y );
}

void displayProgress ( Stack<Queen>& S, int nQueen ) { //����������ʾ�Ѳ�Ľ�չ
   system ( "clear" );
   N = nQueen; S.traverse ( displayRow );
   if ( nQueen <= S.size() )
      printf("%d solution(s) found after %d check(s)\a", nSolu, nCheck);
   getchar();
}