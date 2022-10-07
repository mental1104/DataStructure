/******************************************************************************************
* Data Structures in C++
* ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
* Junhui DENG, deng@tsinghua.edu.cn
* Computer Science & Technology, Tsinghua University
* Copyright (c) 2003-2021. All rights reserved.
******************************************************************************************/

#pragma once

template <typename T> BinNodePosi<T> & BST<T>::search ( const T & e ) { //在BST中查找关键码e
   if ( !_root || e == _root->data ) { _hot = NULL; return _root; } //空树，或恰在树根命中
   for ( _hot = _root; ; ) { //否则，自顶而下
      BinNodePosi<T> & v = ( e < _hot->data ) ? _hot->lc : _hot->rc; //确定方向，深入一层
      if ( !v || e == v->data ) return v; _hot = v; //一旦命中或抵达叶子，随即返回
   } //返回目标节点位置的引用，以便后续插入、删除操作
} //无论命中或失败，_hot均指向v之父亲（v是根时，hot为NULL）
