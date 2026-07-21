#ifndef __DSA_DICTIONARY
#define __DSA_DICTIONARY

#include <cstddef>
#include <dsa/algorithm/Hash.h>

inline std::size_t hashCode(char value) { return dsa::algorithm::hashCode(value); }
inline std::size_t hashCode(int value) { return dsa::algorithm::hashCode(value); }
inline std::size_t hashCode(long long value) { return dsa::algorithm::hashCode(value); }
inline std::size_t hashCode(const char value[]) { return dsa::algorithm::hashCode(value); }

template<typename K, typename V>
struct Dictionary {
    virtual ~Dictionary() {}
    virtual int size() const = 0;
    virtual bool put(K, V) = 0;
    virtual V* get(K key) = 0;
    virtual bool remove(K key) = 0;
};

#endif
