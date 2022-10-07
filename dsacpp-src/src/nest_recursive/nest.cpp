/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

/*DSA*/#include "nest_stack/nest.h"

void trim ( const char exp[], int& lo, int& hi ) { //ɾ��exp[lo, hi]�������ŵ��ǰ׺����׺
   while ( ( lo <= hi ) && ( exp[lo] != '(' ) && ( exp[lo] != ')' ) ) lo++; //���ҵ�һ����
   while ( ( lo <= hi ) && ( exp[hi] != '(' ) && ( exp[hi] != ')' ) ) hi--; //���һ������
}

int divide ( const char exp[], int lo, int hi ) { //�з�exp[lo, hi]��ʹexpƥ������ӱ��ʽƥ��
   int crc = 1; //crcΪ[lo, mi]��Χ������������Ŀ֮��
   while ( ( 0 < crc ) && ( ++lo < hi ) ) //��������ַ���ֱ������������Ŀ��ȣ�����Խ��
      if ( exp[lo] == '(' ) crc ++;
      else if ( exp[lo] == ')' ) crc --;
   return lo;
}

bool paren ( const char exp[], int lo, int hi ) { //�����ʽexp[lo, hi]�Ƿ�����ƥ�䣨�ݹ�棩
   /*DSA*/displaySubstring ( exp, lo, hi );
   trim ( exp, lo, hi ); if ( lo > hi ) return true; //����������ŵ�ǰ׺����׺
   if ( ( exp[lo] != '(' ) || ( exp[hi] != ')' ) ) return false; //�ס�ĩ�ַ����������ţ���ز�ƥ��
   int mi = divide ( exp, lo, hi ); //ȷ���ʵ����зֵ�
   return paren ( exp, lo + 1, mi - 1 ) && paren ( exp, mi + 1, hi ); //�ֱ��������ӱ��ʽ
}