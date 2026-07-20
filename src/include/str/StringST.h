#ifndef __DSA_STRINGST
#define __DSA_STRINGST

#include "dsa_string.h"
#include "Vector.h"

static const size_type R = 256;

/// 教学字符串符号表接口；保留原有虚函数签名和 T() 未命中哨兵。
template<typename T>
class StringST {
public:
    StringST() : s(0) {}
    virtual ~StringST() {}

    virtual void put(const String& key, T value) = 0;
    virtual T get(String& key) = 0;
    virtual void remove(const String& key) = 0;

    bool contains(String key) {
        return !(get(key) == T());
    }

    bool empty() {
        return s == 0;
    }

    int size() {
        return s;
    }

    /// 保留旧 void API；调用方若需要结果应使用 keysWithPrefix("")。
    void keys() {
        (void)keysWithPrefix(String());
    }

    virtual String longestPrefixOf(String input) = 0;
    virtual Vector<String> keysWithPrefix(String prefix) = 0;
    virtual Vector<String> keysThatMatch(String pattern) = 0;

    int s;
};

#endif
