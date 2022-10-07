/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

#include "Entry/Entry.h"
template <typename T> struct QuadlistNode;
template <typename T> using QListNodePosi = QuadlistNode<T>*; //��ת��ڵ�λ��
template <typename T> struct QuadlistNode { //QuadlistNodeģ����
   T entry; //�������
   QListNodePosi<T> pred;  QListNodePosi<T> succ; //ǰ�������
   QListNodePosi<T> above; QListNodePosi<T> below; //���ڡ�����
   QuadlistNode //������
   ( T e = T(), QListNodePosi<T> p = NULL, QListNodePosi<T> s = NULL,
     QListNodePosi<T> a = NULL, QListNodePosi<T> b = NULL )
      : entry ( e ), pred ( p ), succ ( s ), above ( a ), below ( b ) {}
   QListNodePosi<T> insertAsSuccAbove //�����½ڵ㣬�Ե�ǰ�ڵ�Ϊǰ�����Խڵ�bΪ����
   ( T const& e, QListNodePosi<T> b = NULL );
};

#include "QuadlistNode_implementation.h"