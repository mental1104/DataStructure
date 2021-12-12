#pragma once

/******************************************************************************************
 * 二叉树输出打印
 ******************************************************************************************/
//使用位图记录分支转向

#define ROOT 0
#define L_CHILD 1
#define R_CHILD -1*L_CHILD

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

/*

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

*/



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