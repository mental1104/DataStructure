/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> //有序向量（区间）的归并
void Vector<T>::merge ( Rank lo, Rank mi, Rank hi ) { //[lo, mi)和[mi, hi)各自有序，lo < mi < hi
   Rank i = 0; T* A = _elem + lo; //合并后的有序向量A[0, hi - lo) = _elem[lo, hi)，就地
   Rank j = 0, lb = mi - lo; T* B = new T[lb]; //前子向量B[0, lb) <-- _elem[lo, mi)
   for ( Rank i = 0; i < lb; i++ ) B[i] = A[i]; //复制自A的前缀
   Rank k = 0, lc = hi - mi; T* C = _elem + mi; //后子向量C[0, lc) = _elem[mi, hi)，就地
   while ( ( j < lb ) && ( k < lc ) ) //反复地比较B、C的首元素
      A[i++] = ( B[j] <= C[k] ) ? B[j++] : C[k++]; //将更小者归入A中
   while ( j < lb ) //若C先耗尽，则
      A[i++] = B[j++]; //将B残余的后缀归入A中——若B先耗尽呢？
   delete [] B; //释放临时空间：mergeSort()过程中，如何避免此类反复的new/delete？
}
