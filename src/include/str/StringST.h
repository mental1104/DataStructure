#ifndef __DSA_STRINGST
#define __DSA_STRINGST

#include "String.h"
#include "Vector.h"
#define R 128

template<typename T>
class StringST {
public:
    int s{0};
    virtual void put(const String& key, T val) = 0;
    virtual T get(String& key) = 0;
    virtual void remove(const String& key) = 0;

    bool contains(String key) { return get(key); }
    bool empty() {  return !size(); };
    int size() {    return s; }

    void keys() { keysWithPrefix(""); };

    virtual String longestPrefixOf(String s) = 0;
    virtual Vector<String> keysWithPrefix(String s) = 0;
    virtual Vector<String> keysThatMatch(String s) = 0;
    
};

#endif