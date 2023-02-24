/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> //�������������䣩�Ĺ鲢
void Vector<T>::merge ( Rank lo, Rank mi, Rank hi ) { //[lo, mi)��[mi, hi)��������lo < mi < hi
   Rank i = 0; T* A = _elem + lo; //�ϲ������������A[0, hi - lo) = _elem[lo, hi)���͵�
   Rank j = 0, lb = mi - lo; T* B = new T[lb]; //ǰ������B[0, lb) <-- _elem[lo, mi)
   for ( Rank i = 0; i < lb; i++ ) B[i] = A[i]; //������A��ǰ׺
   Rank k = 0, lc = hi - mi; T* C = _elem + mi; //��������C[0, lc) = _elem[mi, hi)���͵�
   while ( ( j < lb ) && ( k < lc ) ) //�����رȽ�B��C����Ԫ��
      A[i++] = ( B[j] <= C[k] ) ? B[j++] : C[k++]; //����С�߹���A��
   while ( j < lb ) //��C�Ⱥľ�����
      A[i++] = B[j++]; //��B����ĺ�׺����A�С�����B�Ⱥľ��أ�
   delete [] B; //�ͷ���ʱ�ռ䣺mergeSort()�����У���α�����෴����new/delete��
}
