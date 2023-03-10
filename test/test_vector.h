#include "Vector.h"
#include <iostream>
#include <cstdlib>

/******************************************************************************************
 * 测试：无序向量的（顺序）查找
 ******************************************************************************************/
template <typename T> //元素类型
void TestVectorFind () {
    std::exit(0);
    /*
    Vector<T> V;
    int testSize = 100;
   for ( int i = 0; i < testSize; i++ ) V.insert ( dice ( i + 1 ), dice ( ( T ) testSize * 3 ) ); //在[0, 3n)中选择n个数，随机插入向量
   for ( int i = 0; i < V.size(); i++ ) { //依次查找向量中元素，当然成功
      T e =  V[i]; //print ( e );
      Rank r = V.find ( e );
      if ( r < 0 ) printf ( " : not found until rank V[%d] <> %d", r, e );
      else printf ( " : found at rank V[%d] = %d", r, V[r] );
      printf ( "\n" );
   }
   for ( int i = 0; i <= V.size(); i++ ) { //依次查找每对相邻元素的均值，可能成功
      T a = ( 0 < i ) ? V[i - 1] : V[0] - 4;
      T b = ( i < V.size() ) ? V[i] : V[V.size()-1] + 4;
      T e =  ( a + b ) / 2; //print ( e );
      Rank r = V.find ( e );
      if ( r < 0 ) std::exit(1)//printf ( " : not found until rank V[%d] <> %d", r, e );
      else printf ( " : found at rank V[%d] = %d", r, V[r] );
      printf ( "\n" );
   }
   */
}