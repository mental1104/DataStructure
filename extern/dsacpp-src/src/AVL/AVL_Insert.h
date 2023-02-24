/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> BinNodePosi<T> AVL<T>::insert ( const T& e ) { //���ؼ���e����AVL����
   BinNodePosi<T> & x = search ( e ); if ( x ) return x; //ȷ��Ŀ��ڵ㲻����
   BinNodePosi<T> xx = x = new BinNode<T> ( e, _hot ); _size++; //�����½ڵ�x
// ��ʱ��x�ĸ���_hot�����ߣ������游�п���ʧ��
   for ( BinNodePosi<T> g = _hot; g; g = g->parent ) //��x֮���������ϣ�������������g
      if ( !AvlBalanced ( *g ) ) { //һ������gʧ�⣬�򣨲��á�3 + 4���㷨��ʹ֮���⣬��������
         FromParentTo ( *g ) = rotateAt ( tallerChild ( tallerChild ( g ) ) ); //���½���ԭ��
         break; //�ֲ���������󣬸߶ȱ�Ȼ��ԭ�������������ˣ��ʵ�������
      } else //����g��ƽ�⣩
         updateHeight ( g ); //ֻ�������߶ȣ�ע�⣺����gδʧ�⣬�߶���������ӣ�
   return xx; //�����½ڵ�λ��
} //����e�Ƿ������ԭ���У�����AVL::insert(e)->data == e
