#pragma once
#include <cstdio>

#include "../def.hpp"

static void print ( char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理
static void print ( const char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理

class UniPrint {
public:
   static void p ( int );
   static void p ( float );
   static void p ( double );
   static void p ( char );
   static void p ( size_t );
   static void p ( VStatus ); //图顶点的状态
   static void p ( EType ); //图边的类型

   template <typename K, typename V> static void p ( Entry<K, V>& ); //Entry
   template <typename T> static void p ( BinNode<T>&); //BinTree节点
   template <typename T> static void p ( BinTree<T>& ); //二叉树
   template <typename T> static void p ( BST<T> &);
   template <typename T> static void p ( AVL<T> &);
   template <typename T> static void p ( Splay<T> &);
   template <typename T> static void p ( BTree<T>& ); //B-树
   template <typename T> static void p ( RedBlack<T>& ); //红黑树
   template <typename T> static void p ( Quadlist<T>& ); //Quadlist
   template <typename K, typename V> static void p ( Skiplist<K, V>& ); //Skiplist
   template <typename K, typename V> static void p ( Hashtable<K, V>& ); //Hashtable
   template <typename Tv, typename Te> static void p ( GraphMatrix<Tv, Te>& ); //Graph
   template <typename T> static void p ( T& ); //向量、列表等支持traverse()遍历操作的线性结构
   template <typename T> static void p ( T* s ) //所有指针
   {  s ? p ( *s ) : print ( "<NULL>" ); } //统一转为引用
}; //UniPrint


void UniPrint::p ( int e ) {  printf ( " %04d", e ); }
void UniPrint::p ( float e ) { printf ( " %4.2f", e ); }
void UniPrint::p ( double e ) { printf ( " %4.2f", e ); }
void UniPrint::p ( char e ) { printf ( " %c", ( 31 < e ) && ( e < 128 ) ? e : '$' ); }
void UniPrint::p ( size_t e) {   printf ("-%lu", e); }

void UniPrint::p ( VStatus e ) {
   switch ( e ) {
      case VStatus::UNDISCOVERED:   printf ( "U" ); break;
      case VStatus::DISCOVERED:     printf ( "D" ); break;
      case VStatus::VISITED:        printf ( "V" ); break;
      case VStatus::SOURCE:         printf ( "S" ); break;
      default:             printf ( "X" ); break;
   }
}
void UniPrint::p ( EType e ) {
   switch ( e ) {
      case EType::UNDETERMINED:   printf ( "U" ); break;
      case EType::TREE:           printf ( "T" ); break;
      case EType::CROSS:          printf ( "C" ); break;
      case EType::BACKWARD:       printf ( "B" ); break;
      case EType::FORWARD:        printf ( "F" ); break;
      default:             printf ( "X" ); break;
   }
}

template <typename T> static void print ( T& x ) {  UniPrint::p ( x );  }
template <typename T> static void print ( const T& x ) {  UniPrint::p ( x );  } //for Stack
template <typename T> static void print ( T* x ) { 
   if(x)  
      print(*x); 
   else
      printf("<NULL>\n");
}

template<typename T> struct Print{
    virtual void operator()(T& e) {  print(e); }
};

#include "./print_traversable.hpp"
#include "./print_binNode.hpp"
#include "./print_bintree.hpp"
#include "./print_btree.hpp"
#include "./print_RBTree.hpp"
#include "./print_Entry.hpp"
#include "./primeNLT.hpp"
#include "./print_HashTable.hpp"
#include "./print_QuadList.hpp"
#include "./print_SkipList.hpp"
#include "./print_GraphMatrix.hpp"