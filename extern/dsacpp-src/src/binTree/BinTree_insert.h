/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> BinNodePosi<T> BinTree<T>::insert ( T const& e )
{ _size = 1; return _root = new BinNode<T> ( e ); } //将e当作根节点插入空的二叉树

template <typename T> BinNodePosi<T> BinTree<T>::insert ( T const& e, BinNodePosi<T> x )
{ _size++; x->insertAsLC ( e ); updateHeightAbove ( x ); return x->lc; } //e插入为x的左孩子

template <typename T> BinNodePosi<T> BinTree<T>::insert ( BinNodePosi<T> x, T const& e )
{ _size++; x->insertAsRC ( e ); updateHeightAbove ( x ); return x->rc; } //e插入为x的右孩子
