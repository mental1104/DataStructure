
#include "dsa_string.h"

#define printString(s) { for (size_t m = strlen(s), k = 0; k < m; k++) printf("%4c", (s)[k]); }

/******************************************************************************************
 * 显示Next[]表，供演示分析
 ******************************************************************************************/
void printNext ( int* N, int offset, int length ) {
   for ( int i = 0; i < length; i++ ) printf ( "%4d", i ); printf ( "\n" );
   for ( int i = 0; i < offset; i++ ) printf ( "    " );
   for ( int i = 0; i < length; i++ ) printf ( "%4d", N[i] ); printf ( "\n\n" );
}

int* buildNext ( char* P ) { //构造模式串P的next表
   int m = strlen ( P ), j = 0; //“主”串指针
   int* next = new int[m]; int t = next[0] = -1; //next表，首项必为-1
   while ( j < m - 1 )
      if ( 0 > t || P[t] == P[j] ) { //匹配
         ++t; ++j; next[j] = t; //则递增赋值：此处可改进...
      } else //否则
         t = next[t]; //继续尝试下一值得尝试的位置
   /*DSA*/printString ( P ); printf ( "\n" );
   /*DSA*/printNext ( next, 0, m );
   return next;
}

/******************************************************************************************
 * Text     :  0   1   2   .   .   .   i   i+1 .   .   .   i+j .   .   n-1
 *             ------------------------|-------------------|------------
 * Pattern  :                          0   1   .   .   .   j   .   .
 *                                     |-------------------|
 ******************************************************************************************
 * 动态显示匹配进展
 * i为P相对于T的起始位置，j为P的当前字符
 ******************************************************************************************/
void showProgress ( char* T, char* P, int i, int j ) {
   /*DSA*/static int step = 0; //操作计数器
   /*DSA*/printf ( "\n-- Step %2d: --\n", ++step );
   /*DSA*/for ( size_t n = strlen ( T ), t = 0; t < n; t++ ) printf ( "%4d", t ); printf ( "\n" );
   /*DSA*/printString ( T ); printf ( "\n" );
   /*DSA*/if ( 0 <= i + j )
      /*DSA*/{  for ( int t = 0; t < i + j; t++ ) printf ( "%4c", ' ' ); printf ( "%4c", '|' );  }
   /*DSA*/printf ( "\n" );
   /*DSA*/for ( int t = 0; t < i; t++ ) printf ( "%4c", ' ' );
   /*DSA*/printString ( P ); printf ( "\n" );
}

int* buildNextImproved ( char* P ) { //构造模式串P的next表（改进版本）
   int m = strlen ( P ), j = 0; //“主”串指针
   int* next = new int[m]; int t = next[0] = -1; //next表，首项必为-1
   while ( j < m - 1 )
      if ( 0 <= t && P[t] != P[j] ) //失配
         t = next[t]; //继续尝试下一值得尝试的位置
      else //匹配
         if ( P[++t] != P[++j] ) //附加条件判断
            next[j] = t; //唯当新的一对字符也匹配时，方照原方法赋值
         else
            next[j] = next[t]; //否则，改用next[t]（此时必有：P[next[t]] != P[t] == P[j]）
   /*DSA*/printString ( P ); printf ( "\n" );
   /*DSA*/printNext ( next, 0, m );
   return next;
}


int match (const String &P, const String&T) {  //KMP算法
   int* next = buildNext ( P ); //构造next表
   int n = ( int ) strlen ( T ), i = 0; //文本串指针
   int m = ( int ) strlen ( P ), j = 0; //模式串指针
   while ( (j < m) && (i < n) ) //自左向右逐个比对字符
      /*DSA*/{
      /*DSA*/showProgress ( T, P, i - j, j );
      /*DSA*/printNext ( next, i - j, strlen ( P ) );
      /*DSA*/getchar(); printf ( "\n" );
      if ( 0 > j || T[i] == P[j] ) //若匹配，或P已移出最左侧（两个判断的次序不可交换）
         { i ++;  j ++; } //则转到下一字符
      else //否则
         j = next[j]; //模式串右移（注意：文本串不用回退）
      /*DSA*/}
   delete [] next; //释放next表
   return i - j;
}