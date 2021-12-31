#pragma once
#include "../def.hpp"
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
    virtual void keysWithPrefix(String s) = 0;
    virtual void keysThatMatch(String s) = 0;
    
};
