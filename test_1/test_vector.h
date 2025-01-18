#include "Vector.h"
#include <iostream>
#include <cstdlib>

const int testSize = 100000;

/******************************************************************************************
 * 测试：无序向量的（顺序）查找
 ******************************************************************************************/
template <typename T> //元素类型
void testVectorFind () {
    Vector<T> V;
    for ( int i = 0; i < testSize; i++ ) V.insert (i); //在[0, 3n)中选择n个数，随机插入向量
    V.unsort();
    for (int i = 0; i < V.size(); i++ ) { //依次查找向量中元素，当然成功
        T e =  V[i]; //print ( e );

        Rank r = V.find ( e );
        if ( r < 0 ) {
            printf ( " : not found until rank V[%d] <> %d\n", r, e );
            std::exit(1);
        }
    }
}

template <typename T> //元素类型
void testVectorSearch () {
    Vector<T> V;
    for ( int i = 0; i < testSize; i++ ) V.insert (i); //在[0, 3n)中选择n个数，随机插入向量
    V.unsort();
    for ( int i = 0; i < V.size(); i++ ) { //依次查找向量中元素，当然成功
        T e =  V[i]; 
        Rank r = V.search ( e );
        if ( V[r] != e ) {
            printf ( " : not found until rank V[%d] <> %d\n", r, e );
            std::exit(1);
        }
    }
}