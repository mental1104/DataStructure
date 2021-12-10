#pragma once
#include <cstdio>

#include "../vector/Vector.hpp"

static void print ( char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理
static void print ( const char* x ) {  printf ( " %s", x ? x : "<NULL>" );  } //字符串特别处理

class UniPrint {
public:
   static void p ( int );
   static void p ( float );
   static void p ( double );
   static void p ( char );
   static void p ( size_t );

   template <typename T> static void p ( T& ); //向量、列表等支持traverse()遍历操作的线性结构
   template <typename T> static void p ( T* s ) //所有指针
   {  s ? p ( *s ) : print ( "<NULL>" ); } //统一转为引用
}; //UniPrint

template<typename T> struct Print{
    virtual void operator()(T& e) {  print(e); }
};

void UniPrint::p ( int e ) {  printf ( " %04d", e ); }
void UniPrint::p ( float e ) { printf ( " %4.3f", e ); }
void UniPrint::p ( double e ) { printf ( " %4.3f", e ); }
void UniPrint::p ( char e ) { printf ( " %c", ( 31 < e ) && ( e < 128 ) ? e : '$' ); }
void UniPrint::p ( size_t e) {   printf ("-%lu", e); }

template <typename T> static void print ( T& x ) {  UniPrint::p ( x );  }
template <typename T> static void print ( const T& x ) {  UniPrint::p ( x );  } //for Stack
template <typename T> static void print ( T* x ) { 
   if(x)  
      print(*x); 
   else
      printf("<NULL>\n");
}

#include "./print_traversable.hpp"