/******************************************************************************************
* Data Structures in C++
* ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
* Junhui DENG, deng@tsinghua.edu.cn
* Computer Science & Technology, Tsinghua University
* Copyright (c) 2003-2021. All rights reserved.
******************************************************************************************/

#pragma once

template <typename T> BinNodePosi<T> & BST<T>::search ( const T & e ) { //��BST�в��ҹؼ���e
   if ( !_root || e == _root->data ) { _hot = NULL; return _root; } //��������ǡ����������
   for ( _hot = _root; ; ) { //�����Զ�����
      BinNodePosi<T> & v = ( e < _hot->data ) ? _hot->lc : _hot->rc; //ȷ����������һ��
      if ( !v || e == v->data ) return v; _hot = v; //һ�����л�ִ�Ҷ�ӣ��漴����
   } //����Ŀ��ڵ�λ�õ����ã��Ա�������롢ɾ������
} //�������л�ʧ�ܣ�_hot��ָ��v֮���ף�v�Ǹ�ʱ��hotΪNULL��
