#include "dsa_string.h"

/******************************************************************************************
 * 显示bc[]表，供演示分析
 ******************************************************************************************/
void printBC ( int* bc ) {
   printf ( "\n-- bc[] Table ---------------\n" );
   for ( size_t j = 0; j < 256; j++ ) if ( 0 <= bc[j] ) printf ( "%4c", ( char ) j ); printf ( "\n" );
   for ( size_t j = 0; j < 256; j++ ) if ( 0 <= bc[j] ) printf ( "%4d", bc[j] ); printf ( "\n\n" );
}


#define printString(s) { for (size_t m = strlen(s), k = 0; k < m; k++) printf("%4c", (s)[k]); }

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

/******************************************************************************************
 *    0                       bc['X']                                m-1
 *    |                       |                                      |
 *    ........................X***************************************
 *                            .|<------------- 'X' free ------------>|
 ******************************************************************************************/
int* buildBC( char* P ) { //构造Bad Charactor Shift表：O(m + 256)
   int* bc = new int[256]; // BC表，与字符表等长
   for ( size_t j = 0; j < 256; j++ ) bc[j] = -1; //初始化：首先假设所有字符均未在P中出现
   for ( size_t m = strlen( P ), j = 0; j < m; j++ ) //自左向右扫描模式串P
      bc[P[j]] = j; //将字符P[j]的BC项更新为j（单调递增）——画家算法
   /*DSA*/ printBC( bc );
   return bc;
}


int match( char* P, char* T ) { //Boyer-Morre算法（简化版，只考虑Bad Character Shift）
   int* bc = buildBC( P ); //预处理
   int n = strlen( T ), i; //文本串长度、与模式串首字符的对齐位置
   int m = strlen( P ), j; //模式串长度、模式串当前字符位置
   for ( i = 0; i+m <= n; i += max(1, j - bc[ T[i+j] ]) ) { //不断右移P
      for ( j = m-1; (0 <= j) && (P[j] == T[i+j]); j-- ); //自右向左逐个比对
      /*DSA*/showProgress ( T, P, i, j );
      if ( j < 0 ) break; //一旦完全匹配，随即收工
   }
   delete [] bc; return i; //销毁BC表，返回最终的对齐位置
}

int* buildSS ( char* P ) { //构造最大匹配后缀长度表：O(m)
   int m = strlen ( P ); int* ss = new int[m]; //Suffix Size表
   ss[m - 1]  =  m; //对最后一个字符而言，与之匹配的最长后缀就是整个P串
// 以下，从倒数第二个字符起自右向左扫描P，依次计算出ss[]其余各项
   for ( int lo = m - 1, hi = m - 1, j = lo - 1; j >= 0; j -- )
      if ( ( lo < j ) && ( ss[m - hi + j - 1] < j - lo ) ) //情况一
         ss[j] =  ss[m - hi + j - 1]; //直接利用此前已计算出的ss[]
      else { //情况二
         hi = j; lo = min ( lo, hi );
         while ( ( 0 <= lo ) && ( P[lo] == P[m - hi + lo - 1] ) ) //二重循环？
            lo--; //逐个对比处于(lo, hi]前端的字符
         ss[j] = hi - lo;
      }
   /*DSA*/printf ( "-- ss[] Table -------\n" );
   /*DSA*/for ( int i = 0; i < m; i ++ ) printf ( "%4d", i ); printf ( "\n" );
   /*DSA*/printString ( P ); printf ( "\n" );
   /*DSA*/for ( int i = 0; i < m; i ++ ) printf ( "%4d", ss[i] ); printf ( "\n\n" );
   return ss;
}


/******************************************************************************************
 * 显示GS[]表，供演示分析
 ******************************************************************************************/
void printGS ( char* P, int* GS ) {
   printf ( "-- gs[] Table ---------------\n" );
   for ( size_t m = strlen ( P ), j = 0; j < m; j++ ) printf ( "%4d", j ); printf ( "\n" );
   printString ( P ); printf ( "\n" );
   for ( size_t m = strlen ( P ), j = 0; j < m; j++ ) printf ( "%4d", GS[j] ); printf ( "\n\n" );
}

int* buildGS ( char* P ) { //构造好后缀位移量表：O(m)
   int* ss = buildSS ( P ); //Suffix Size table
   size_t m = strlen ( P ); int* gs = new int[m]; //Good Suffix shift table
   for ( size_t j = 0; j < m; j ++ ) gs[j] = m; //初始化
   for ( size_t i = 0, j = m - 1; j < UINT_MAX; j-- ) //逆向逐一扫描各字符P[j]
      if ( j + 1 == ss[j] ) //若P[0, j] = P[m - j - 1, m)，则
         while ( i < m - j - 1 ) //对于P[m - j - 1]左侧的每个字符P[i]而言（二重循环？）
            gs[i++] = m - j - 1; //m - j - 1都是gs[i]的一种选择
   for ( size_t j = 0; j < m - 1; j ++ ) //画家算法：正向扫描P[]各字符，gs[j]不断递减，直至最小
      gs[m - ss[j] - 1] = m - j - 1; //m - j - 1必是其gs[m - ss[j] - 1]值的一种选择
   /*DSA*/printGS ( P, gs );
   delete [] ss; return gs;
}

int match2 ( char* P, char* T ) { //Boyer-Morre算法（完全版，兼顾Bad Character与Good Suffix）
   int* bc = buildBC ( P ); int* gs = buildGS ( P ); //构造BC表和GS表
   size_t i = 0; //模式串相对于文本串的起始位置（初始时与文本串左对齐）
   while ( strlen ( T ) >= i + strlen ( P ) ) { //不断右移（距离可能不止一个字符）模式串
      int j = strlen ( P ) - 1; //从模式串最末尾的字符开始
      while ( P[j] == T[i + j] ) //自右向左比对
         if ( 0 > --j ) break; /*DSA*/showProgress ( T, P, i, j ); printf ( "\n" ); getchar();
      if ( 0 > j ) //若极大匹配后缀 == 整个模式串（说明已经完全匹配）
         break; //返回匹配位置
      else //否则，适当地移动模式串
         i += max ( gs[j], j - bc[ T[i + j] ] ); //位移量根据BC表和GS表选择大者
   }
   delete [] gs; delete [] bc; //销毁GS表和BC表
   return i;
}