/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> QListNodePosi<T> //将e作为p的后继、b的上邻插入Quadlist
Quadlist<T>::insertAfterAbove ( T const& e, QListNodePosi<T> p, QListNodePosi<T> b )
{  _size++; return p->insertAsSuccAbove ( e, b );  } //返回新节点位置（below = NULL）