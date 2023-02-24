/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> //T为基本类型，或已重载赋值操作符'='
void Vector<T>::copyFrom ( T const* A, Rank lo, Rank hi ) { //以数组区间A[lo, hi)为蓝本复制向量
   _elem = new T[ _capacity = 2 * ( hi - lo ) ]; //分配空间
   for (_size = 0; lo < hi; _size++, lo++ ) //A[lo, hi)内的元素逐一
      _elem[ _size ] = A[ lo ]; //复制至_elem[0, hi - lo)
} //用const修饰，保证A中的元素不致被篡改；运行时间 = O(hi-lo)