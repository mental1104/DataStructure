#ifndef __DSA_HASHMAP
#define __DSA_HASHMAP

#include "primeNLT.h"
#include "utils.h"
#include "Entry.h"
#include "Dictionary.h"
#include "Eratosthenes.h"
#include "List.h"
#include "RedBlack.h"

template<typename K, typename V>
class HashMap : public Dictionary<K, V> {
private:
    struct Bucket {
        bool isTree;
        List<Entry<K, V>> list;
        RedBlack<Entry<K, V>> tree;
        Bucket() : isTree(false) {}
        int size() const { return isTree ? tree.size() : list.size(); }
    };

    static const int TREEIFY_THRESHOLD = 8;

    Bitmap* primes;
    Bucket* buckets;
    int M;
    int N;

    int indexOf(const K& k) const {
        return static_cast<int>(hashCode(k) % static_cast<size_t>(M));
    }

    ListNode<Entry<K, V>>* findInList(Bucket& bucket, const K& k) {
        for (auto p = bucket.list.first(); bucket.list.valid(p); p = p->succ) {
            if (p->data.key == k) {
                return p;
            }
        }
        return nullptr;
    }

    void treeify(Bucket& bucket) {
        if (bucket.isTree) {
            return;
        }
        ListNode<Entry<K, V>>* p = bucket.list.first();
        while (bucket.list.valid(p)) {
            ListNode<Entry<K, V>>* next = p->succ;
            bucket.tree.insert(p->data);
            bucket.list.remove(p);
            p = next;
        }
        bucket.isTree = true;
    }

    void rehash();

public:
    explicit HashMap(int c = 3);
    ~HashMap();
    int size() const { return N; }
    int capacity() const { return M; }
    bool put(K, V);
    V* get(K k);
    bool remove(K k);

    bool bucketIsTree(int index) const { return buckets[index].isTree; }
    int bucketSize(int index) const { return buckets[index].size(); }
    int _M() const { return M; }
    int _N() const { return N; }
};

template<typename K, typename V>
HashMap<K, V>::HashMap(int c) {
    primes = eratosthenes(1050000);
    M = primeNLT_mem(c, 1048576, primes);
    N = 0;
    buckets = new Bucket[M];
}

template<typename K, typename V>
HashMap<K, V>::~HashMap() {
    delete [] buckets;
    release(primes);
}

template<typename K, typename V>
V* HashMap<K, V>::get(K k) {
    int r = indexOf(k);
    Bucket& bucket = buckets[r];
    if (bucket.isTree) {
        Entry<K, V> probe(k, V());
        auto node = bucket.tree.search(probe);
        return node ? &node->data.value : nullptr;
    }
    ListNode<Entry<K, V>>* node = findInList(bucket, k);
    return node ? &node->data.value : nullptr;
}

template<typename K, typename V>
bool HashMap<K, V>::put(K k, V v) {
    int r = indexOf(k);
    Bucket& bucket = buckets[r];
    if (bucket.isTree) {
        Entry<K, V> probe(k, V());
        if (bucket.tree.search(probe)) {
            return false;
        }
        bucket.tree.insert(Entry<K, V>(k, v));
        ++N;
        if (N > M * 2) {
            rehash();
        }
        return true;
    }

    if (findInList(bucket, k)) {
        return false;
    }
    bucket.list.insertAsLast(Entry<K, V>(k, v));
    ++N;
    if (bucket.list.size() >= TREEIFY_THRESHOLD) {
        treeify(bucket);
    }
    if (N > M * 2) {
        rehash();
    }
    return true;
}

template<typename K, typename V>
bool HashMap<K, V>::remove(K k) {
    int r = indexOf(k);
    Bucket& bucket = buckets[r];
    if (bucket.isTree) {
        Entry<K, V> probe(k, V());
        if (!bucket.tree.remove(probe)) {
            return false;
        }
        --N;
        return true;
    }
    ListNode<Entry<K, V>>* node = findInList(bucket, k);
    if (!node) {
        return false;
    }
    bucket.list.remove(node);
    --N;
    return true;
}

template<typename K, typename V>
void HashMap<K, V>::rehash() {
    int oldM = M;
    Bucket* oldBuckets = buckets;

    M = primeNLT_mem(4 * N, 1048576, primes);
    buckets = new Bucket[M];
    N = 0;

    for (int i = 0; i < oldM; ++i) {
        if (oldBuckets[i].isTree) {
            oldBuckets[i].tree.travIn([this](Entry<K, V>& entry) {
                put(entry.key, entry.value);
            });
        } else {
            for (auto p = oldBuckets[i].list.first(); oldBuckets[i].list.valid(p); p = p->succ) {
                put(p->data.key, p->data.value);
            }
        }
    }

    delete [] oldBuckets;
}

#endif
