#pragma once 
#include "String.h"
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
    TSTNode<T>* get(TSTNode<T>* x, String& key, int d);
    TSTNode<T>* put(TSTNode<T>* x, const String& key, T val, int d);
    TSTNode<T>* remove(TSTNode<T>* x, const String& key, int d);

    void collect(TSTNode<T>* x, String prefix, Vector<String>& q);
    void collect(TSTNode<T>* x, String prefix, int i, String pattern, Vector<String>& q);
public:
    TST() = default;
    ~TST();
    T get(String& key);
    void put(const String& key, T val) {  root = put(root, key, val, 0);  }
    void remove(const String& key);

    String longestPrefixOf(String s);
    Vector<String> keysWithPrefix(String pre);
    Vector<String> keysThatMatch(String s);
};

template<typename T>
TST<T>::~TST(){
    Queue<TSTNode<T>*> Q;
    this->s = 0;
    TSTNode<T>* x = root;
    Q.enqueue(x);
    while(!Q.empty()){
        TSTNode<T>* x = Q.dequeue();
        if(x->left) Q.enqueue(x->left);
        if(x->mid) Q.enqueue(x->mid);
        if(x->right) Q.enqueue(x->right);
        release(x->val);
        release(x);
    }
}

template<typename T>
TSTNode<T>* TST<T>::put(TSTNode<T>* x, const String& key, T val, int d){
    char c = key[d];
    if(x == nullptr){
        x = new TSTNode<T>(c);
    }
    if      (c < x->c) x->left = put(x->left, key, val, d);
    else if (c > x->c) x->right = put(x->right, key, val, d);
    else if (d < key.size()-1) x->mid = put(x->mid, key, val, d+1);
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
TSTNode<T>* TST<T>::get(TSTNode<T>* x, String& key, int d){
    if(x == nullptr)
        return nullptr;

    char c = key[d];
    if      (c < x->c)  return get(x->left, key, d);
    else if (c > x->c)  return get(x->right, key, d);
    else if (d < key.size()-1) 
                        return get(x->mid, key, d+1);
    else return x;
}

template<typename T>
void TST<T>::remove(const String& key){
    root = remove(root, key, 0);
}

template<typename T>
TSTNode<T>* TST<T>::remove(TSTNode<T>* x, const String& key, int d){
    if(x == nullptr)
        return nullptr;
   
    char c = key[d];
    if      (c < x->c) x->left = remove(x->left, key, d);
    else if (c > x->c) x->right = remove(x->right, key, d);
    else if (d < key.size()-1) x->mid = remove(x->mid, key, d+1);
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
String TST<T>::longestPrefixOf(String s){
    if(s.size() == 0)
        return s;
    int length = 0;
    TSTNode<T>* x = root;
    int i = 0;
    while (x != nullptr && i < s.size()){
        char c = s[i];
        if      (c < x->c) x = x->left;
        else if (c > x->c) x = x->right;
        else {
            i++;
            if(x->val != 0) length = i;
            x = x->mid;
        }
    }
    return s.substr(0, length);
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
    if(x->val != 0) q.insert(prefix + x->c);
    collect(x->mid, prefix + x->c, q);
    collect(x->right, prefix, q);
}

template<typename T>
Vector<String> TST<T>::keysThatMatch(String s){
    Vector<String> q;
    collect(root, "", 0, s, q);
    return q;
}

template<typename T>
void TST<T>::collect(TSTNode<T>* x, String prefix, int i, String pattern, Vector<String>& q){
    if(x == nullptr) return;
    char c = pattern[i];
    if(c == '.' || c < x->c) 
        collect(x->left, prefix, i, pattern, q);
    if(c == '.' || c == x->c){
        if(i == pattern.size()-1 && x->val != 0)
            q.insert(prefix + x->c);
        if(i < pattern.size() - 1)
            collect(x->mid, prefix+x->c, i+1, pattern, q);
    }
    if(c == '.' || c > x->c) collect(x->right, prefix, i, pattern, q);
}