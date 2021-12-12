#pragma once

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