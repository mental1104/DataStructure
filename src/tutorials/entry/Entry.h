#ifndef __DSA_ENTRY
#define __DSA_ENTRY

template<typename K, typename V> 
struct Entry {
    K key; 
    V value;
    Entry(K k = K(), V v = V()): key(k), value(v){}
    Entry(Entry<K, V> const& e): key(e.key), value(e.value){}
    Entry<K, V>& operator=(Entry<K, V> const&) = default;
    bool operator< (Entry<K, V> const& e) const { return key < e.key; }
    bool operator> (Entry<K, V> const& e) const { return key > e.key; }
    bool operator== (Entry<K, V> const& e) const {  return key == e.key; }
    bool operator!= (Entry<K, V> const& e) const {  return key != e.key; }
};

#endif
