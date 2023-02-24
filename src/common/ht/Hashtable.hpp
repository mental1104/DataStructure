#pragma once

#include "../print/primeNLT.hpp"
#include "../def.hpp"

template<typename K, typename V>
class Hashtable : public Dictionary<K, V> {
private:
    Entry<K, V>** ht;
    int M;
    int N;
    Bitmap* lazyRemoval;  
    bool lazilyRemoved(int x) {     return lazyRemoval->test(x);   }
    void markAsRemoved(int x) {     lazyRemoval->set(x);           }

protected:  
    int probe4Hit(const K& k);
    int probe4Free(const K& k);
    void rehash();

public:
    Hashtable(int c = 3);
    ~Hashtable();
    int size() const { return N; }
    int capacity() const { return M; }
    bool put(K, V);
    V* get(K k);
    bool remove(K k);

    int _M() {   return M;   }
    int _N() {   return N;   }
    Bitmap* _lazyRemoval()  {    return lazyRemoval;     }
    Entry<K, V>* _ht(int i)  {   return ht[i];       }
};

static size_t hashCode ( char c ) { return ( size_t ) c; } //字符
static size_t hashCode ( int k ) { return ( size_t ) k; } //整数以及长长整数
static size_t hashCode ( long long i ) { return ( size_t ) ( ( i >> 32 ) + ( int ) i ); }
static size_t hashCode ( const char s[] ) { //生成字符串的循环移位散列码（cyclic shift hash code）
    unsigned int h = 0; //散列码
    for ( size_t n = strlen ( s ), i = 0; i < n; i++ ) //自左向右，逐个处理每一字符
    { 
        h = ( h << 5 ) | ( h >> 27 ); 
        h += (int) s[i]; 
        printf("%d\n", (int) s[i]);
    } //散列码循环左移5位，再累加当前字符
   return ( size_t ) h; //如此所得的散列码，实际上可理解为近似的“多项式散列码”
} //对于英语单词，"循环左移5位"是实验统计得出的最佳值


template<typename K, typename V>
Hashtable<K, V>::Hashtable(int c){
    M = primeNLT(c, 1048576, "../print/prime-bitmap.txt");
    N = 0;
    ht = new Entry<K, V>*[M];
    memset(ht, 0, sizeof(Entry<K, V>*)*M);
    lazyRemoval = new Bitmap(M);
}

template<typename K, typename V>
Hashtable<K, V>::~Hashtable(){
    for(int i = 0; i < M; i++)
        if(ht[i]) release(ht[i]);
    release(ht);
    release(lazyRemoval);
}

template<typename K, typename V>
V* Hashtable<K, V>::get(K k){
    int r = probe4Hit(k);
    return ht[r] ? &(ht[r]->value) : nullptr;
}

template<typename K, typename V>
int Hashtable<K, V>::probe4Hit(const K& k){
    int r = hashCode(k) % M;
    while((ht[r] && (k != ht[r]->key)) || (!ht[r] && lazilyRemoved(r))){
        r = (r + 1) % M;
    }      
    return r;
}

template<typename K, typename V>
bool Hashtable<K, V>::remove(K k){
    int r = probe4Hit(k);
    if(!ht[r]) return false;
    release(ht[r]);
    ht[r] = nullptr;
    markAsRemoved(r);//In reality, it is not removed
    N--;
    return true;
}

template<typename K, typename V>
bool Hashtable<K, V>::put(K k, V v){
    if(ht[probe4Hit(k)]) return false;
    int r = probe4Free(k);
    ht[r] = new Entry<K, V>(k, v);
    ++N;
    if(N * 2 > M) rehash();//装填因子控制在50%以下
    return true;
}

template<typename K, typename V>
int Hashtable<K, V>::probe4Free(const K& k){
    int r = hashCode(k) % M;
    while(ht[r])
        r = (r + 1) % M;
    return r;
}

template <typename K, typename V> 
void Hashtable<K, V>::rehash() {
   int oldM = M; 
   Entry<K, V>** oldHt = ht;

   M = primeNLT( 4 * N, 1048576, "../print/prime-bitmap.txt"); //容量至少加倍（若懒惰删除很多，可能反而缩容）
   ht = new Entry<K, V>*[M]; 
   
   N = 0; 
   memset( ht, 0, sizeof( Entry<K, V>* ) * M ); //桶数组
   release(lazyRemoval); 
   lazyRemoval = new Bitmap( M ); //懒惰删除标记
   //*DSA*/printf("A bucket array has been created with capacity = %d\n\n", M);
   for ( int i = 0; i < oldM; i++ ) //扫描原表
      if ( oldHt[i] ) //将每个非空桶中的词条
         put( oldHt[i]->key, oldHt[i]->value ); //转入新表
   release( oldHt ); //释放——因所有词条均已转移，故只需释放桶数组本身
}
