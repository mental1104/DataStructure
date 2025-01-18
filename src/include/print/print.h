#ifndef __DSA_PRINT
#define __DSA_PRINT

#include <iostream>
#include "utils.h"
#include "Entry.h"
#include "Vector.h"
#include "List.h"
#include "Stack.h"
#include "Queue.h"
#include "BinNode.h"
#include "BinTree.h"
#include "BST.h"
#include "AVL.h"
#include "Splay.h"
#include "RedBlack.h"
#include "BTree.h"
#include "Quadlist.h"
#include "Skiplist.h"
#include "Hashtable.h"
#include "HashtableB.h"
#include "Heap.h"
#include "LeftHeap.h"
#include "String.h"
#include "Graph.h"
#include "GraphMatrix.h"
#include "primeNLT.h"

static void print ( char* x );
static void print ( const char* x );
template <typename T> static void print ( T& x );
template <typename T> static void print ( const T& x );
template <typename T> static void print ( T* x );

static void print ( char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理
static void print ( const char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理


static void print ( String e) {
   const char* c = e.c_str();
   while(*c){
      putchar(*c);
      c++;
   }
   putchar('\n');
}

class UniPrint {
public:
   static void p ( int );
   static void p ( float );
   static void p ( double );
   static void p ( char );
   static void p ( unsigned int);
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
   template <typename K, typename V> static void p ( QuadraticHT<K, V>& ); // Quadratic Probing Hashtable
   template <typename T> static void printComplHeap (Heap<T>& pq, int n, int k, int depth, int type, int* bType );
   template <typename T> static void p ( Heap<T> & pq );
   template <typename T> static void p ( LeftHeap<T>& lh);//LeftHeap
   template <typename Tv, typename Te> static void p ( GraphMatrix<Tv, Te>& ); //Graph
   template <typename T> static void p ( T& ); //向量、列表等支持traverse()遍历操作的线性结构
   template <typename T> static void p ( T* s ) //所有指针
   {  s ? p ( *s ) : print ( "<NULL>" ); } //统一转为引用
}; //UniPrint


void UniPrint::p ( int e ) {  printf ( " %04d", e ); }
void UniPrint::p ( float e ) { printf ( " %4.2f", e ); }
void UniPrint::p ( double e ) { printf ( " %4.2f", e ); }
void UniPrint::p ( char e ) { printf ( "%c", ( 31 < e ) && ( e < 128 ) ? e : '$' ); }
void UniPrint::p ( unsigned int e ) {  printf ( " %04u", e ); }
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

/******************************************************************************************
 * Entry
 ******************************************************************************************/
template <typename K, typename V>
void UniPrint::p ( Entry<K, V>& e ) 
{  printf ( "-<" ); print ( e.key ); printf ( ":" ); print ( e.value ); printf ( ">-" );  }





/******************************************************************************************
 * 向量、列表等支持traverse()遍历操作的线性结构
 ******************************************************************************************/
template <typename T> 
void UniPrint::p ( T& s ) { 
   //printf ( "Type: %s Address: [%p] Size: %d:\n", typeid(s).name(), reinterpret_cast<unsigned*>(&s), s.size()); //fundamental information
   s.traverse ( print ); //traverse
   printf ("\n");
}




/******************************************************************************************
 * BinTree节点
 ******************************************************************************************/
template <typename T> 
void UniPrint::p ( BinNode<T>& node) {
   p ( node.data ); //数值
   
   printf (
      ( node.lc && &node != node.lc->parent ) ||
      ( node.rc && &node != node.rc->parent ) ?
      "@" : " "
   );
}

/******************************************************************************************
 * 二叉树输出打印
 ******************************************************************************************/
//使用位图记录分支转向

#define ROOT 0
#define L_CHILD 1
#define R_CHILD -1*L_CHILD


/******************************************************************************************
 * 二叉树各种派生类的统一打印
 ******************************************************************************************/
template <typename T> //元素类型
static void printBinTree ( BinNode<T>* bt, int depth, int type, Bitmap* bType ) {
   if ( !bt ) return;
   if ( -1 < depth ) //设置当前层的拐向标志
      R_CHILD == type ? bType->set ( depth ) : bType->clear ( depth );
   printBinTree ( bt->rc, depth + 1, R_CHILD, bType ); //右子树（在上）
   print ( bt ); printf ( " *" );
   for ( int i = -1; i < depth; i++ ) //根据相邻各层
      if ( ( 0 > i ) || bType->test ( i ) == bType->test ( i + 1 ) ) //的拐向是否一致，即可确定
         printf ( "      " ); //是否应该
      else  printf ( "│    " ); //打印横线
   switch ( type ) {
      case  R_CHILD  :  printf ( "┌─" );  break;
      case  L_CHILD  :  printf ( "└─" );  break;
      default        :  printf ( "──" );  break; //root
   }
   print ( bt );
#if defined(DSA_HUFFMAN)
   if ( IsLeaf ( *bt ) ) bType->print ( depth + 1 ); //输出Huffman编码
#endif
   printf ( "\n" );
   printBinTree ( bt->lc, depth + 1, L_CHILD, bType ); //左子树（在下）
}


/******************************************************************************************
 * 基础BinTree
 ******************************************************************************************/
template <typename T> //元素类型
void UniPrint::p ( BinTree<T> & bt ) { //引用
   printf ( "%s[%p]*%d:\n", typeid(bt).name(), reinterpret_cast<unsigned*>(&bt), bt.size() ); //基本信息
   Bitmap* branchType = new Bitmap; //记录当前节点祖先的方向
   printBinTree ( bt.root(), -1, ROOT, branchType ); //树状结构
   release ( branchType ); 
   printf ( "\n" );
}



template <typename T> //元素类型
void UniPrint::p ( BST<T> & bt ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( bt ).name(), reinterpret_cast<unsigned*>(&bt), bt.size() ); //基本信息
   Bitmap* branchType = new Bitmap; //记录当前节点祖先的方向
   printBinTree ( bt.root(), -1, ROOT, branchType ); //树状结构
   release ( branchType ); 
   printf ( "\n" );
}


template <typename T> //元素类型
void UniPrint::p ( AVL<T> & avl ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( avl ).name(), reinterpret_cast<unsigned*>(&avl), avl.size() ); //基本信息
   Bitmap* branchType = new Bitmap; //记录当前节点祖先的方向
   printBinTree ( avl.root(), -1, ROOT, branchType ); //树状结构
   release ( branchType ); 
   printf ( "\n" );
}

template <typename T> //元素类型
void UniPrint::p ( Splay<T> & bt ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( bt ).name(), reinterpret_cast<unsigned*>(&bt), bt.size() ); //基本信息
   Bitmap* branchType = new Bitmap; //记录当前节点祖先的方向
   printBinTree ( bt.root(), -1, ROOT, branchType ); //树状结构
   release ( branchType ); 
   printf ( "\n" );
}





/******************************************************************************************
 * BTree输出打印
 ******************************************************************************************/

/******************************************************************************************
 * BTree打印（入口）
 ******************************************************************************************/
template <typename T> //元素类型
void UniPrint::p ( BTree<T> & bt ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( bt ).name(), reinterpret_cast<unsigned*>(&bt), bt.size() ); //基本信息
   Bitmap* leftmosts = new Bitmap; //记录当前节点祖先的方向
   Bitmap* rightmosts = new Bitmap; //记录当前节点祖先的方向
   printBTree ( bt.root(), 0, true, true, leftmosts, rightmosts ); //输出树状结构
   release ( leftmosts ); release ( rightmosts ); 
   printf ( "\n" );
}

/******************************************************************************************
 * BTree打印（递归）
 ******************************************************************************************/
template <typename T> //元素类型
static void printBTree ( BTNode<T>* bt, int depth, bool isLeftmost, bool isRightmost, Bitmap* leftmosts, Bitmap* rightmosts ) {
   if ( !bt ) return;
   isLeftmost ? leftmosts->set ( depth ) : leftmosts->clear ( depth ); //设置或清除当前层的拐向标志
   isRightmost ? rightmosts->set ( depth ) : rightmosts->clear ( depth ); //设置或清除当前层的拐向标志
   int k = bt->child.size() - 1; //找到当前节点的最右侧孩子，并
   printBTree ( bt->child[k], depth + 1, false, true, leftmosts, rightmosts ); //递归输出之
   /*DSA*/bool parentOK = false; BTNode<T>* p = bt->parent;
   /*DSA*/if ( !p ) parentOK = true;
   /*DSA*/else for ( int i = 0; i < p->child.size(); i++ ) if ( p->child[i] == bt ) parentOK = true;
   while ( 0 < k-- ) { //自右向左，输出各子树及其右侧的关键码
      /*DSA*/printf ( parentOK ? " " : "X" );
      print ( bt->key[k] ); printf ( " *>" );
      for ( int i = 0; i < depth; i++ ) //根据相邻各层
         ( leftmosts->test ( i ) && leftmosts->test ( i + 1 ) || rightmosts->test ( i ) && rightmosts->test ( i + 1 ) ) ? //的拐向是否一致，即可确定
         printf ( "      " ) : printf ( "│    " ); //是否应该打印横向联接线
      if ( ( ( 0 == depth && 1 < bt->key.size() ) || !isLeftmost && isRightmost ) && bt->key.size() - 1 == k ) printf ( "┌─" );
      else if ( ( ( 0 == depth && 1 < bt->key.size() ) || isLeftmost && !isRightmost ) && 0 == k )            printf ( "└─" );
      else                                                                                               printf ( "├─" );
      print ( bt->key[k] ); printf ( "\n" );
      printBTree ( bt->child[k], depth + 1, 0 == k, false, leftmosts, rightmosts ); //递归输出子树
   }
}


/******************************************************************************************
 * RedBlackTree输出打印
 ******************************************************************************************/


void printRB ( int e , bool red) { 
   if(red)
      printf(" \033[0m\033[1;31m%04d\033[0m", e ); 
   else 
      printf(" %04d", e);
}

void printRB ( float e , bool red) { 
   if(red)
      printf (" \033[0m\033[1;31m%4.3f\033[0m", e ); 
   else 
      printf (" %4.3f", e );

}
void printRB ( double e , bool red) { 
   if(red)
      printf (" \033[0m\033[1;31m%4.3f\033[0m", e ); 
   else 
      printf (" %4.3f", e);
}

void printRB ( char e , bool red) { 
   if(red)
      printf ( " \033[0m\033[1;31m%c\033[0m", ( 31 < e ) && ( e < 128 ) ? e : '$' ); 
   else 
      printf ( " %c", ( 31 < e ) && ( e < 128 ) ? e : '$' ); 
}

template <typename T> void printRBNode( BinNode<T>& node) {
   bool flag = false;
   if(node.color == RBColor::RED)
      flag = true;
   printRB ( node.data, flag); //数值
   
   printf (
      ( node.lc && &node != node.lc->parent ) ||
      ( node.rc && &node != node.rc->parent ) ?
      "@" : " "
   );

}

template <typename T> //元素类型
static void printRBTree ( BinNode<T>* bt, int depth, int type, Bitmap* bType ) {
   if ( !bt ) return;
   if ( -1 < depth ) //设置当前层的拐向标志
      R_CHILD == type ? bType->set ( depth ) : bType->clear ( depth );
   printRBTree ( bt->rc, depth + 1, R_CHILD, bType ); //右子树（在上）
   printRBNode ( *bt ); printf ( " *" );
   for ( int i = -1; i < depth; i++ ) //根据相邻各层
      if ( ( 0 > i ) || bType->test ( i ) == bType->test ( i + 1 ) ) //的拐向是否一致，即可确定
         printf ( "      " ); //是否应该
      else  printf ( "│    " ); //打印横线
   switch ( type ) {
      case  R_CHILD  :  printf ( "┌─" );  break;
      case  L_CHILD  :  printf ( "└─" );  break;
      default        :  printf ( "──" );  break; //root
   }
   printRBNode( *bt );

   printf ( "\n" );
   printRBTree ( bt->lc, depth + 1, L_CHILD, bType ); //左子树（在下）
}

template <typename T> //元素类型
void printRedBlack ( RedBlack<T> & rb ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( rb ).name(), reinterpret_cast<unsigned*>(&rb), rb.size() ); //基本信息
   Bitmap* branchType = new Bitmap; //记录当前节点祖先的方向
   printRBTree ( rb.root(), -1, ROOT, branchType ); //树状结构
   release ( branchType ); 
   printf ( "\n" );
}


template <typename T> static void printRedBlackTree ( T& x ) {  printRedBlack ( x );  }
template <typename T> static void printRedBlackTree ( const T& x ) {  printRedBlack ( x );  }
template <typename T> static void printRedBlackTree ( T* x ) { 
   if(x)  
      printRedBlackTree(*x); 
   else
      printf("<NULL>\n");
}

template <typename T>
void UniPrint::p ( RedBlack<T>& q ) { 
   printRedBlackTree(q);
}





/******************************************************************************************
 * Hashtable
 ******************************************************************************************/

template <typename K, typename V> //e、value
void UniPrint::p ( Hashtable<K, V>& ht ) { //引用
   printf ( "%s[%p]*(%d)/%d:\n", typeid ( ht ).name(), reinterpret_cast<unsigned*>(&ht), ht._N(), ht._M() ); //基本信息
   for ( int i = 0; i < ht._M(); i++ ) //输出桶编号
      printf ( "  %4d  ", i );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "-<%04d>-", ht._ht(i)->key ); //演示用，仅适用于int
      else if ( ht._lazyRemoval()->test(i) ) printf ( "-<****>-" );
      else printf ( "--------" );
   printf ( "\n" );
   /*
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "    %c   ", ht._ht(i)->value ); //演示用，仅适用于char
//      if (ht.ht[i]) printf("%8s", ht.ht[i]->value); //针对Huffman编码中使用的散列表
      else if ( ht._lazyRemoval()->test(i) ) printf ( "    *   " );
      else printf ( "        " );
   printf ( "\n" );
   */
}

template <typename K, typename V> //e、value
void UniPrint::p ( QuadraticHT<K, V>& ht ) { //引用
   printf ( "%s[%p]*(%d)/%d:\n", typeid ( ht ).name(), reinterpret_cast<unsigned*>(&ht), ht._N(), ht._M() ); //基本信息
   for ( int i = 0; i < ht._M(); i++ ) //输出桶编号
      printf ( "  %4d  ", i );
   printf ( "\n" );
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "-<%04d>-", ht._ht(i)->key ); //演示用，仅适用于int
      else if ( ht._lazyRemoval()->test(i) ) printf ( "-<****>-" );
      else printf ( "--------" );
   printf ( "\n" );
   /*
   for ( int i = 0; i < ht._M(); i++ ) //输出所有元素
      if ( ht._ht(i) ) printf ( "    %c   ", ht._ht(i)->value ); //演示用，仅适用于char
//      if (ht.ht[i]) printf("%8s", ht.ht[i]->value); //针对Huffman编码中使用的散列表
      else if ( ht._lazyRemoval()->test(i) ) printf ( "    *   " );
      else printf ( "        " );
   printf ( "\n" );*/
}




/******************************************************************************************
 * Quadlist
 ******************************************************************************************/

template <typename T>
void UniPrint::p ( Quadlist<T>& q ) { //����
   printf ( "%s[%p]*%03d: ", typeid(q).name(), reinterpret_cast<unsigned*>(&q), q.size() ); //������Ϣ
   if ( q.size() <= 0 ) {  printf ( "\n" ); return;  }
   QuadlistNode<T>* curr = q.first()->pred; 
   QuadlistNode<T>* base = q.first(); 
   while ( base->below ) base = base->below; 
   while ( base->pred ) base = base->pred; 
   for ( int i = 0; i < q.size(); i++ ) { 
      curr = curr->succ; //curr
      QuadlistNode<T>* proj = curr; 
      while ( proj->below ) proj = proj->below; 
      while ( ( base = base->succ ) != proj ) 
         printf ( "---------------" ); 
      print ( curr->entry ); 
   }
   printf ( "\n" );
}




/******************************************************************************************
 * Skiplist
 ******************************************************************************************/

template <typename K, typename V> 
void UniPrint::p ( Skiplist<K, V>& s ) { 
   printf ( "%s[%p]*%d*%d:\n", typeid ( s ).name(), reinterpret_cast<unsigned*>(&s), s.level(), s.size() ); //������Ϣ
   s.traverse ( print ); 
   printf ( "\n" );
}


/******************************************************************************************
 * 打印输出PQ_ComplHeap
 ******************************************************************************************/
template <typename T> //元素类型
void UniPrint::p (Heap<T> & pq ) { //引用
   printf ( "%s[%p]*%d:\n", typeid ( pq ).name(), reinterpret_cast<int*>(&pq), pq.size()); //基本信息
   int branchType[256]; //最深256层 <= 2^256 = 1.16*10^77
   printComplHeap (pq, pq.size(), 0, 0, ROOT, branchType ); //树状结构
   printf ( "\n" );
}

/******************************************************************************************
 * 递归打印输出
 ******************************************************************************************/
template<typename T> //元素类型
void UniPrint::printComplHeap (Heap<T>& pq, int n, int k, int depth, int type, int* bType ) {
   if ( k >= n ) return; //递归基
   bType[depth] = type;
   printComplHeap ( pq, n, pq.RChild(k), depth + 1, R_CHILD, bType ); //右子树（在上）
   print ( pq._elem[k] ); ( 0 < k ) && ( pq._elem[pq.Parent(k)] < pq._elem[k] ) ? printf ( "X" ) : printf ( " " ); printf ( "*" );
   for ( int i = 0; i < depth; i++ ) //根据相邻各层
      if ( bType[i] + bType[i+1] ) //的拐向是否一致，即可确定
         printf ( "      " ); //是否应该
      else  printf ( "│    " ); //打印横线
   switch ( type ) {
      case  R_CHILD  :  printf ( "┌─" );  break;
      case  L_CHILD  :  printf ( "└─" );  break;
      default        :  printf ( "──" );  break; //root
   }
   print(pq._elem[k]); ( 0 < k ) && ( pq._elem[pq.Parent ( k ) ] < pq._elem[k] ) ? printf ( "X" ) : printf ( " " ); printf ( "\n" );
   printComplHeap (pq, n, pq.LChild(k), depth + 1, L_CHILD, bType ); //左子树（在下）
}



/******************************************************************************************
 * 打印输出PQ_LeftHeap
 ******************************************************************************************/
template <typename T> //元素类型
void UniPrint::p (LeftHeap<T> & lh ) { //引用
   p ( ( BinTree<T>& ) lh );
}


/******************************************************************************************
 * 图Graph
 ******************************************************************************************/
template <typename Tv, typename Te> //顶点类型、边类型
void UniPrint::p ( GraphMatrix<Tv, Te>& s ) { //引用
   int inD = 0; for ( int i = 0; i < s.n; i++ ) inD += s.inDegree ( i );
   int outD = 0; for ( int i = 0; i < s.n; i++ ) outD += s.outDegree ( i );
   printf ( "%s[%p]*(%d, %d):\n", typeid ( s ).name(), reinterpret_cast<unsigned*>(&s), s.n, s.e ); //基本信息
// 标题行
   print ( s.n ); printf ( "    " ); print ( inD ); printf ( "|" );
   for ( int i = 0; i < s.n; i++ ) { print ( s.vertex ( i ) ); printf(" ");/*printf ( "[" ); print ( s.status ( i ) ); printf ( "] " );*/ }
   printf ( "\n" );
// 标题行（续）
   print ( outD ); printf ( "    " ); print ( s.e ); printf ( "|" );
   for ( int i = 0; i < s.n; i++ ) { print ( s.inDegree ( i ) ); printf ( " " ); }
   printf ( "| dTime fTime Parent Weight\n" );
// 水平分隔线
   printf ( "-----------+" ); for ( int i = 0; i < s.n; i++ ) printf ( "------" );
   printf ( "+----------------------------\n" );
// 逐行输出各顶点
   for ( int i = 0; i < s.n; i++ ) {
      print ( s.vertex ( i ) ); printf ( "[" ); print ( s.status ( i ) ); printf ( "] " ); print ( s.outDegree ( i ) ); printf ( "|" );
      for ( int j = 0; j < s.n; j++ )
         if ( s.exists ( i, j ) ) { print ( s.edge ( i, j ) ); print ( s.type ( i, j ) ); }
         else printf ( "     ." );
      printf ( "| " ); print ( s.dTime ( i ) ); printf ( " " ); print ( s.fTime ( i ) );
      printf ( "  " ); if ( 0 > s.parent ( i ) ) print ( "^   " ); else print ( s.vertex ( s.parent ( i ) ) );
      printf ( "  " ); if ( INT_MAX > s.priority ( i ) ) print ( s.priority ( i ) ); else print ( " INF" );
      printf ( "\n" );
   }
   printf ( "\n" );
}

//template<template <typename...> class Outer, typename OuterT,
//         template <typename...> class Inner, typename InnerT>
template<template <typename> class Outer, typename T>
void display(Outer<T>& structure){
   int times = 30;
   T random =  100;
   for(int i = 0; i < times; i++){
      system("clear");
      T val = dice(random); 
      std::cout << "Insert: \t" << val << std::endl;
      structure.insert(val);
      print(structure);
      //sleep(1);
   }
}

#endif