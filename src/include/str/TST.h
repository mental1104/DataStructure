#ifndef __DSA_TST
#define __DSA_TST

#include "dsa_string.h"
#include "StringST.h"
#include "Vector.h"
#include "Queue.h"

template<typename T>
struct TSTNode {
    char c;
    TSTNode* left;
    TSTNode* mid;
    TSTNode* right;
    T val;
    TSTNode() = delete;
    TSTNode(char rhs):c(rhs), left(nullptr), mid(nullptr), right(nullptr), val(0){}
};

template<typename T>
class TST : public StringST<T>{
private:
    TSTNode<T>* root{nullptr};
    TSTNode<T>* get(TSTNode<T>* x, String& key, size_type d);
    TSTNode<T>* put(TSTNode<T>* x, const String& key, T val, size_type d);
    TSTNode<T>* remove(TSTNode<T>* x, const String& key, size_type d);

    void collect(TSTNode<T>* x, String prefix, Vector<String>& q);
    void collect(TSTNode<T>* x, String prefix, size_type i, String pattern, Vector<String>& q);
public:
    TST() = default;
    ~TST();
    T get(String& key);
    T get(const char*);
    void put(const String& key, T val) {  root = put(root, key, val, 0);  }
    void remove(const String& key);

    String longestPrefixOf(String input);
    Vector<String> keysWithPrefix(String pre);
    Vector<String> keysThatMatch(String pattern);
};

template<typename T>
TST<T>::~TST(){
    Queue<TSTNode<T>*> Q;
    this->s = 0;
    TSTNode<T>* node = root;
    Q.enqueue(node);
    while(!Q.empty()){
        TSTNode<T>* current = Q.dequeue();
        if(current->left) Q.enqueue(current->left);
        if(current->mid) Q.enqueue(current->mid);
        if(current->right) Q.enqueue(current->right);
        release(current->val);
        release(current);
    }
}

template<typename T>
TSTNode<T>* TST<T>::put(TSTNode<T>* x, const String& key, T val, size_type d){
    char c = key[d];
    if(x == nullptr){
        x = new TSTNode<T>(c);
    }
    if      (c < x->c) x->left = put(x->left, key, val, d);
    else if (c > x->c) x->right = put(x->right, key, val, d);
    else if (d + 1 < key.size()) x->mid = put(x->mid, key, val, d+1);
    else {
        x->val = val;
        ++this->s;
    }
    return x;
}

template<typename T>
T TST<T>::get(String& key){
    TSTNode<T>* x = get(root, key, 0);
    if(x)
        return x->val;
    return 0;
}

template<typename T>
T TST<T>::get(const char* key) {
    String strKey(key);  // 将 C 字符串转换为 std::string
    return get(strKey);       // 复用已有的 `get` 方法
}

template<typename T>
TSTNode<T>* TST<T>::get(TSTNode<T>* x, String& key, size_type d){
    if(x == nullptr)
        return nullptr;

    char c = key[d];
    if      (c < x->c)  return get(x->left, key, d);
    else if (c > x->c)  return get(x->right, key, d);
    else if (d + 1 < key.size()) 
                        return get(x->mid, key, d+1);
    else return x;
}

template<typename T>
void TST<T>::remove(const String& key){
    root = remove(root, key, 0);
}

template<typename T>
TSTNode<T>* TST<T>::remove(TSTNode<T>* x, const String& key, size_type d){
    if(x == nullptr)
        return nullptr;

    char c = key[d];
    if      (c < x->c) x->left = remove(x->left, key, d);
    else if (c > x->c) x->right = remove(x->right, key, d);
    else if (d + 1 < key.size()) x->mid = remove(x->mid, key, d+1);
    else {
        x->val = 0;
        --this->s;
    }

    if(x->val != 0) return x;
    
    if(x->left || x->mid || x->right)
        return x;
    else{
        release(x->val);
        release(x);
        return nullptr;
    }
}

template<typename T>
String TST<T>::longestPrefixOf(String input){
    if(input.size() == 0)
        return input;
    size_type length = 0;
    TSTNode<T>* x = root;
    size_type i = 0;
    while (x != nullptr && i < input.size()){
        char c = input[i];
        if      (c < x->c) x = x->left;
        else if (c > x->c) x = x->right;
        else {
            i++;
            if(x->val != 0) length = i;
            x = x->mid;
        }
    }
    return input.substr(0, length);
}

template<typename T>
Vector<String> TST<T>::keysWithPrefix(String pre){
    Vector<String> q;
    if(pre == String("")){
        collect(root, pre, q);
        return q;
    }
    TSTNode<T>* x = get(root, pre, 0);
    if(x == nullptr) return q;
    if(x->val != 0) q.insert(pre);
    collect(x->mid, pre, q);
    return q;
}

template<typename T>
void TST<T>::collect(TSTNode<T>* x, String prefix, Vector<String>& q){
    if(x == nullptr)
        return;
    collect(x->left, prefix, q);
    String connected_str = prefix + String(x->c);
    if(x->val != 0) q.insert(connected_str);
    collect(x->mid, connected_str, q);
    collect(x->right, prefix, q);
}

template<typename T>
Vector<String> TST<T>::keysThatMatch(String pattern){
    Vector<String> q;
    collect(root, "", 0, pattern, q);
    return q;
}

template<typename T>
void TST<T>::collect(TSTNode<T>* x, String prefix, size_type i, String pattern, Vector<String>& q){
    if(x == nullptr) return;
    char c = pattern[i];
    if(c == '.' || c < x->c) 
        collect(x->left, prefix, i, pattern, q);
    if(c == '.' || c == x->c){
        if(i + 1 == pattern.size() && x->val != 0)
            q.insert(prefix + x->c);
        if(i + 1 < pattern.size())
            collect(x->mid, prefix+x->c, i+1, pattern, q);
    }
    if(c == '.' || c > x->c) collect(x->right, prefix, i, pattern, q);
}

#endif
