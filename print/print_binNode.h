#pragma once

template <typename T> void UniPrint::p ( BinNode<T>& node) {
   p ( node.data ); //数值
   
   printf (
      ( node.lc && &node != node.lc->parent ) ||
      ( node.rc && &node != node.rc->parent ) ?
      "@" : " "
   );

}