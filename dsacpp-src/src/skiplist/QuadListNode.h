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
template <typename T> using QListNodePosi = QuadlistNode<T>*; //跳转表节点位置
template <typename T> struct QuadlistNode { //QuadlistNode模板类
   T entry; //所存词条
   QListNodePosi<T> pred;  QListNodePosi<T> succ; //前驱、后继
   QListNodePosi<T> above; QListNodePosi<T> below; //上邻、下邻
   QuadlistNode //构造器
   ( T e = T(), QListNodePosi<T> p = NULL, QListNodePosi<T> s = NULL,
     QListNodePosi<T> a = NULL, QListNodePosi<T> b = NULL )
      : entry ( e ), pred ( p ), succ ( s ), above ( a ), below ( b ) {}
   QListNodePosi<T> insertAsSuccAbove //插入新节点，以当前节点为前驱，以节点b为下邻
   ( T const& e, QListNodePosi<T> b = NULL );
};

#include "QuadlistNode_implementation.h"