/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

typedef unsigned int U; //Լ��������T�����U�����ת��ΪU�������˶���

template <typename T> //���б�����ʼ��λ��p�����Ϊn����������������
void List<T>::radixSort ( ListNodePosi<T> p, int n ) { //valid(p) && rank(p) + n <= size
   /*DSA*///printf ( "RadixSort ...\n" );
   ListNodePosi<T> head = p->pred; ListNodePosi<T> tail = p;
   for ( int i = 0; i < n; i++ ) tail = tail->succ; //����������Ϊ(head, tail)
   for ( U radixBit = 0x1; radixBit && (p = head); radixBit <<= 1 ) //���·�����
      for ( int i = 0; i < n; i++ ) //���ݵ�ǰ����λ�������нڵ�
         radixBit & U (p->succ->data) ? //�ּ�Ϊ��׺��1����ǰ׺��0��
            insert( remove( p->succ ), tail ) : p = p->succ;
}
//˼����ĳ�˷ּ����ǰ׺����׺û�б仯���Ƿ�����漴�����㷨��
//���Ľ�����ǰ�ҳ����Ԫ�ز�����������Чλ���Ӷ���ʡ���õķּ�
//���Ľ���Ϊ����remove()��insertB()�ĵ�Ч�ʣ�ʵ��List::moveB(t,p)�ӿڣ����ڵ�p�ƶ���t֮ǰ
