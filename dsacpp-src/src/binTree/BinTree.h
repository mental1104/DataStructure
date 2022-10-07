/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

#include "BinNode.h" //引入二叉树节点类
template <typename T> class BinTree { //二叉树模板类
protected:
   int _size; BinNodePosi<T> _root; //规模、根节点
   virtual int updateHeight ( BinNodePosi<T> x ); //更新节点x的高度
   void updateHeightAbove ( BinNodePosi<T> x ); //更新节点x及其祖先的高度
public:
   BinTree() : _size ( 0 ), _root ( NULL ) { } //构造函数
   ~BinTree() { if ( 0 < _size ) remove ( _root ); } //析构函数
   int size() const { return _size; } //规模
   bool empty() const { return !_root; } //判空
   BinNodePosi<T> root() const { return _root; } //树根
   BinNodePosi<T> insert ( T const & ); //插入根节点
   BinNodePosi<T> insert ( T const &, BinNodePosi<T> ); //插入左孩子
   BinNodePosi<T> insert ( BinNodePosi<T>, T const & ); //插入右孩子
   BinNodePosi<T> attach ( BinTree<T> * &, BinNodePosi<T> ); //接入左子树
   BinNodePosi<T> attach ( BinNodePosi<T>, BinTree<T> * & ); //接入右子树
   int remove ( BinNodePosi<T> ); //子树删除
   BinTree<T>* secede ( BinNodePosi<T> ); //子树分离
   template <typename VST> //操作器
   void travLevel ( VST& visit ) { if ( _root ) _root->travLevel ( visit ); } //层次遍历
   template <typename VST> //操作器
   void travPre ( VST& visit ) { if ( _root ) _root->travPre ( visit ); } //先序遍历
   template <typename VST> //操作器
   void travIn ( VST& visit ) { if ( _root ) _root->travIn ( visit ); } //中序遍历
   template <typename VST> //操作器
   void travPost ( VST& visit ) { if ( _root ) _root->travPost ( visit ); } //后序遍历
   bool operator< ( BinTree<T> const& t ) //比较器（其余自行补充）
   { return _root && t._root && lt ( _root, t._root ); }
   bool operator== ( BinTree<T> const& t ) //判等器
   { return _root && t._root && ( _root == t._root ); }
}; //BinTree

#include "BinTree_implementation.h"