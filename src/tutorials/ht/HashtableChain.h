#ifndef __DSA_HASHTABLE_CHAIN
#define __DSA_HASHTABLE_CHAIN

#include "primeNLT.h"
#include "utils.h"
#include "Entry.h"
#include "Dictionary.h"
#include "Eratosthenes.h"
#include "List.h"

template<typename K, typename V>
class HashtableChain : public Dictionary<K, V> {
private:
    Bitmap* primes;
    List<Entry<K, V>>* ht;
    int M;
    int N;

    int indexOf(const K& k) const {
        return static_cast<int>(hashCode(k) % static_cast<size_t>(M));
    }

    ListNode<Entry<K, V>>* findInBucket(List<Entry<K, V>>& bucket, const K& k) {
        for (auto p = bucket.first(); bucket.valid(p); p = p->succ) {
            if (p->data.key == k) {
                return p;
            }
        }
        return nullptr;
    }

    void rehash();

public:
    explicit HashtableChain(int c = 3);
    ~HashtableChain();
    int size() const { return N; }
    int capacity() const { return M; }
    bool put(K, V);
    V* get(K k);
    bool remove(K k);

    int _M() const { return M; }
    int _N() const { return N; }
};

template<typename K, typename V>
HashtableChain<K, V>::HashtableChain(int c) {
    primes = eratosthenes(1050000);
    M = primeNLT_mem(c, 1048576, primes);
    N = 0;
    ht = new List<Entry<K, V>>[M];
}

template<typename K, typename V>
HashtableChain<K, V>::~HashtableChain() {
    delete [] ht;
    release(primes);
}

template<typename K, typename V>
V* HashtableChain<K, V>::get(K k) {
    int r = indexOf(k);
    List<Entry<K, V>>& bucket = ht[r];
    ListNode<Entry<K, V>>* node = findInBucket(bucket, k);
    return node ? &node->data.value : nullptr;
}

template<typename K, typename V>
bool HashtableChain<K, V>::put(K k, V v) {
    int r = indexOf(k);
    List<Entry<K, V>>& bucket = ht[r];
    if (findInBucket(bucket, k)) {
        return false;
    }
    bucket.insertAsLast(Entry<K, V>(k, v));
    ++N;
    if (N > M * 2) {
        rehash();
    }
    return true;
}

template<typename K, typename V>
bool HashtableChain<K, V>::remove(K k) {
    int r = indexOf(k);
    List<Entry<K, V>>& bucket = ht[r];
    ListNode<Entry<K, V>>* node = findInBucket(bucket, k);
    if (!node) {
        return false;
    }
    bucket.remove(node);
    --N;
    return true;
}

template<typename K, typename V>
void HashtableChain<K, V>::rehash() {
    int oldM = M;
    List<Entry<K, V>>* oldHt = ht;

    M = primeNLT_mem(4 * N, 1048576, primes);
    ht = new List<Entry<K, V>>[M];
    N = 0;

    for (int i = 0; i < oldM; ++i) {
        for (auto p = oldHt[i].first(); oldHt[i].valid(p); p = p->succ) {
            put(p->data.key, p->data.value);
        }
    }

    delete [] oldHt;
}

#endif
