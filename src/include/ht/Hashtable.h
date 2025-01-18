#ifndef __DSA_HASHTABLE
#define __DSA_HASHTABLE

#include "primeNLT.h"
#include "utils.h"
#include "Entry.h"
#include "Bitmap.h"
#include "Dictionary.h"
#include "Eratosthenes.h"


template<typename K, typename V>
class Hashtable : public Dictionary<K, V> {
private:
    Bitmap* primes;
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

template<typename K, typename V>
Hashtable<K, V>::Hashtable(int c){
    primes = eratosthenes(1050000);
    M = primeNLT_mem(c, 1048576, primes);
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
    release(primes);
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

   M = primeNLT_mem( 4 * N, 1048576, primes); //容量至少加倍（若懒惰删除很多，可能反而缩容）
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

#endif