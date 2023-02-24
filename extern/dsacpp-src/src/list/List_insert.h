/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename T> ListNodePosi<T> List<T>::insertAsFirst ( T const& e )
{  _size++; return header->insertAsSucc ( e );  } //e�����׽ڵ����

template <typename T> ListNodePosi<T> List<T>::insertAsLast ( T const& e )
{  _size++; return trailer->insertAsPred ( e );  } //e����ĩ�ڵ����

template <typename T> ListNodePosi<T> List<T>::insert ( ListNodePosi<T> p, T const& e )
{  _size++; return p->insertAsSucc ( e );  } //e����p�ĺ�̲���

template <typename T> ListNodePosi<T> List<T>::insert ( T const& e, ListNodePosi<T> p )
{  _size++; return p->insertAsPred ( e );  } //e����p��ǰ������
