#pragma once
#include "../def.hpp"

template<typename T>
struct Node {
    T val;
    Vector<Node<T>*> next;
    Node():val(0), next(R, R, nullptr){}
};

template<typename T>
class Trie : public StringST<T> {
private:
    Node<T>* root{nullptr};

    Node<T>* get(Node<T>* x, String& key, int d);
    Node<T>* put(Node<T>* x, const String& key, T val, int d);
    Node<T>* remove(Node<T>* x, const String& key, int d);

    void collect(Node<T>* x, String pre, Vector<String>& q);
    void collect(Node<T>* x, String pre, const String &pat, Vector<String>& q);
    int search(Node<T>* x, String s, int d, int length);
    
public:
    Trie() = default;
    ~Trie() {   if(0 < this->s) destruct(root); }

    T get(String& key);
    void put(const String& key, T val);
    void remove(const String& key);

    void keysWithPrefix(String pre);
    void keysThatMatch(String pat);
    String longestPrefixOf(String s);
};

template<typename T>
static void destruct(Node<T>* x){
    Queue<Node<T>*> Q;
    Q.enqueue(x);
    while(!Q.empty()){
        Node<T>* x = Q.dequeue();
        for(int i = 0; i < R; i++){
            if(x->next[i])
                Q.enqueue(x->next[i]);
        }
        release(x->val);
        release(x);
    }
}

template<typename T>
void Trie<T>::put(const String& key, T val){
    if(val == 0)
        return;
    root = put(root, key, val, 0);
}

template<typename T>
Node<T>* Trie<T>::put(Node<T>* x, const String& key, T val, int d){
    if(x == nullptr) x = new Node<T>();
    if(d == key.size()){
        x->val = val;
        this->s++;
        return x;
    }
    char c = key[d];
    x->next[c] = put(x->next[c], key, val, d+1);
    return x;
}

template<typename T>
T Trie<T>::get(String& key){
    Node<T>* x = get(root, key, 0);
    if(x)
        return x->val;
    else 
        return 0;
}

template<typename T>
Node<T>* Trie<T>::get(Node<T>* x, String& key, int d){
    if(x == nullptr)
        return nullptr;
    if(d == key.size()) return x;
    char c = key[d];
    return get(x->next[c], key, d+1);
}

template<typename T>
void Trie<T>::keysWithPrefix(String pre){
    Vector<String> q;
    collect(get(root, pre, 0), pre, q);
    print(q);
}

template<typename T>
void Trie<T>::collect(Node<T>* x, String pre, Vector<String>& q){
    if(x == nullptr)
        return;

    if(x->val != 0){
        q.insert(pre);
    }
    for(short c = 0; c < R; c++){
        char temp = *(char*)&c;
        collect(x->next[c], pre+temp, q);
    }
        
}

template<typename T>
void Trie<T>::keysThatMatch(String pat){
    Vector<String> q;
    collect(root, "", pat, q);
    print(q);
}

template<typename T>
void Trie<T>::collect(Node<T>* x, String pre, const String& pat, Vector<String>& q){
    int d = pre.size();
    if(x == nullptr) return;
    if(d == pat.size() && x->val != 0) q.insert(pre);
    if(d == pat.size()) return;

    char next = pat[d];
    for(short i = 0; i < R; i++){
        char c = *(char*)&i;
        if(next =='.' || next == c)
            collect(x->next[c], pre+c, pat, q);
    }
}

template<typename T>
String Trie<T>::longestPrefixOf(String s){
    int length = search(root, s, 0, 0);
    return s.substr(0, length);
}

template<typename T>
int Trie<T>::search(Node<T>* x, String s, int d, int length){
    if(x == nullptr) return length;
    if(x->val != 0) length = d;
    if(d == s.size()) return length;
    char c = s[d];
    return search(x->next[c], s, d+1, length);
}

template<typename T>
void Trie<T>::remove(const String& key){
    root = remove(root, key, 0);
}

template<typename T>
Node<T>* Trie<T>::remove(Node<T>* x, const String& key, int d){
    if(x == nullptr)    return nullptr;
    if(d == key.size()){
        x->val = 0;
        --this->s;
    }
    else{
        char c = key[d];
        x->next[c] = remove(x->next[c], key, d+1);
    }

    if(x->val != 0) return x;
    for(short i = 0; i < R; i++){
        char c = *(char*)&i;
        if(x->next[c] != nullptr)
            return x;
    }
    release(x->val);
    release(x);
    return nullptr;
}
