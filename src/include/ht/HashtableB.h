#ifndef __DSA_HASHTABLEB
#define __DSA_HASHTABLEB

#include "primeNLT.h"
#include "utils.h"
#include "Entry.h"
#include "Bitmap.h"
#include "Dictionary.h"
#include "Eratosthenes.h"

template<typename K, typename V>
class QuadraticHT : public Dictionary<K, V> {
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
    QuadraticHT(int c = 5);
    ~QuadraticHT();
    int size() const { return N; }
    bool put(K, V);
    V* get(K k);
    bool remove(K k);

    int _M() {   return M;   }
    int _N() {   return N;   }
    Bitmap* _lazyRemoval()  {    return lazyRemoval;     }
    Entry<K, V>* _ht(int i)  {   return ht[i];       }
};


template<typename K, typename V>
QuadraticHT<K, V>::QuadraticHT(int c){
    primes = eratosthenes(1050000);
    M = primeQHT_mem(c, 1048576, primes);
    N = 0;
    ht = new Entry<K, V>*[M];
    memset(ht, 0, sizeof(Entry<K, V>*)*M);
    lazyRemoval = new Bitmap(M);
}

template<typename K, typename V>
QuadraticHT<K, V>::~QuadraticHT(){
    for(int i = 0; i < M; i++)
        if(ht[i]) release(ht[i]);
    release(ht);
    release(lazyRemoval);
    release(primes);
}

template<typename K, typename V>
V* QuadraticHT<K, V>::get(K k){
    int r = probe4Hit(k);
    return ht[r] ? &(ht[r]->value) : nullptr;
}

template<typename K, typename V>
int QuadraticHT<K, V>::probe4Hit(const K& k){
    int r = hashCode(k)%M;
    int offset = 1;
    int cnt = 0;

    while((ht[r] && (k != ht[r]->key)) || (!ht[r] && lazilyRemoved(r))){
        size_t hash = hashCode(k);
        int pos = (hash + offset*offset) % M;
        int neg = (hash - offset*offset) % M;
        r = (cnt%2) ? neg : pos;
        offset += cnt % 2;
        ++cnt;
    }

    return r;
}

template<typename K, typename V>
bool QuadraticHT<K, V>::remove(K k){
    int r = probe4Hit(k);
    if(!ht[r]) return false;
    release(ht[r]);
    ht[r] = nullptr;
    markAsRemoved(r);//In reality, it is not removed
    N--;
    return true;
}

template<typename K, typename V>
bool QuadraticHT<K, V>::put(K k, V v){
    if(ht[probe4Hit(k)]) return false;
    int r = probe4Free(k);
    ht[r] = new Entry<K, V>(k, v);
    ++N;
    if(N >= M) rehash();
    return true;
}

template<typename K, typename V>
int QuadraticHT<K, V>::probe4Free(const K& k){
    int r = hashCode(k)%M;
    int offset = 1;
    int cnt = 0;

    while(ht[r]){
        if(cnt%2 == 0){
            r = (hashCode(k) + offset*offset) % M; 
        } else {
            r = (hashCode(k) - offset*offset) % M; 
        }  

        if(cnt%2 == 1)
            offset++;
        cnt++;
    }
    return r;
}

template <typename K, typename V> 
void QuadraticHT<K, V>::rehash() {
   int oldM = M; 
   Entry<K, V>** oldHt = ht;

   M = primeQHT_mem( 2 * N, 1048576, primes); //容量至少加倍（若懒惰删除很多，可能反而缩容）
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