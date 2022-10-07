/******************************************************************************************
 * Data Structures in C++
 * ISBN: 7-302-33064-6 & 7-302-33065-3 & 7-302-29652-2 & 7-302-26883-3
 * Junhui DENG, deng@tsinghua.edu.cn
 * Computer Science & Technology, Tsinghua University
 * Copyright (c) 2003-2021. All rights reserved.
 ******************************************************************************************/

#pragma once

template <typename NodePosi> inline //�ڽڵ�*p��*lc������Ϊ�գ�֮�佨���������ӹ�ϵ
void attachAsLC ( NodePosi lc, NodePosi p ) { p->lc = lc; if ( lc ) lc->parent = p; }

template <typename NodePosi> inline //�ڽڵ�*p��*rc������Ϊ�գ�֮�佨�������ң��ӹ�ϵ
void attachAsRC ( NodePosi p, NodePosi rc ) { p->rc = rc; if ( rc ) rc->parent = p; }

template <typename T> //Splay����չ�㷨���ӽڵ�v���������չ
BinNodePosi<T> Splay<T>::splay ( BinNodePosi<T> v ) { //vΪ��������ʶ�����չ�Ľڵ�λ��
   if ( !v ) return NULL; BinNodePosi<T> p; BinNodePosi<T> g; //*v�ĸ������游
   while ( ( p = v->parent ) && ( g = p->parent ) ) { //���¶��ϣ�������*v��˫����չ
      BinNodePosi<T> gg = g->parent; //ÿ��֮��*v����ԭ���游��great-grand parent��Ϊ��
      if ( IsLChild ( *v ) )
         if ( IsLChild ( *p ) ) { //zig-zig
            /*DSA*/printf ( "\tzIg-zIg :" ); print ( g ); print ( p ); print ( v ); printf ( "\n" );
            attachAsLC ( p->rc, g ); attachAsLC ( v->rc, p );
            attachAsRC ( p, g ); attachAsRC ( v, p );
         } else { //zig-zag
            /*DSA*/printf ( "\tzIg-zAg :" ); print ( g ); print ( p ); print ( v ); printf ( "\n" );
            attachAsLC ( v->rc, p ); attachAsRC ( g, v->lc );
            attachAsLC ( g, v ); attachAsRC ( v, p );
         }
      else if ( IsRChild ( *p ) ) { //zag-zag
         /*DSA*/printf ( "\tzAg-zAg :" ); print ( g ); print ( p ); print ( v ); printf ( "\n" );
         attachAsRC ( g, p->lc ); attachAsRC ( p, v->lc );
         attachAsLC ( g, p ); attachAsLC ( p, v );
      } else { //zag-zig
         /*DSA*/printf ( "\tzAg-zIg :" ); print ( g ); print ( p ); print ( v ); printf ( "\n" );
         attachAsRC ( p, v->lc ); attachAsLC ( v->rc, g );
         attachAsRC ( v, g ); attachAsLC ( p, v );
      }
      if ( !gg ) v->parent = NULL; //��*vԭ�ȵ����游*gg�����ڣ���*v����ӦΪ����
      else //����*gg�˺�Ӧ����*v��Ϊ����Һ���
         ( g == gg->lc ) ? attachAsLC ( v, gg ) : attachAsRC ( gg, v );
      updateHeight ( g ); updateHeight ( p ); updateHeight ( v );
   } //˫����չ����ʱ������g == NULL����p���ܷǿ�
   if ( p = v->parent ) { //��p����ǿգ����������һ�ε���
      /*DSA*/if ( IsLChild ( *v ) ) { printf ( "\tzIg :" ); print ( p ); print ( v ); printf ( "\n" ); }
      /*DSA*/else              { printf ( "\tzAg :" ); print ( p ); print ( v ); printf ( "\n" ); }
      if ( IsLChild ( *v ) ) { attachAsLC ( v->rc, p ); attachAsRC ( v, p ); }
      else                   { attachAsRC ( p, v->lc ); attachAsLC ( p, v ); }
      updateHeight ( p ); updateHeight ( v );
   }
   v->parent = NULL; return v;
} //����֮��������ӦΪ����չ�Ľڵ㣬�ʷ��ظýڵ��λ���Ա��ϲ㺯����������