#ifndef __DSA_DICTIONARY
#define __DSA_DICTIONARY

#include <cstdio>
#include <cstring>

inline size_t hashCode ( char c ) { return ( size_t ) c; } //字符
inline size_t hashCode ( int k ) { return ( size_t ) k; } //整数以及长长整数
inline size_t hashCode ( long long i ) { return ( size_t ) ( ( i >> 32 ) + ( int ) i ); }
inline size_t hashCode ( const char s[] ) { //生成字符串的循环移位散列码（cyclic shift hash code）
    unsigned int h = 0; //散列码
    for ( size_t n = strlen ( s ), i = 0; i < n; i++ ) //自左向右，逐个处理每一字符
    { 
        h = ( h << 5 ) | ( h >> 27 ); 
        h += (int) s[i]; 
        printf("%d\n", (int) s[i]);
    } //散列码循环左移5位，再累加当前字符
   return ( size_t ) h; //如此所得的散列码，实际上可理解为近似的“多项式散列码”
} //对于英语单词，"循环左移5位"是实验统计得出的最佳值

template<typename K, typename V> struct Dictionary {
    virtual int size() const = 0;
    virtual bool put(K, V) = 0;
    virtual V* get(K k) = 0;
    virtual bool remove(K k) = 0;
};

#endif
