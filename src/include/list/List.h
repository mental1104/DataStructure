#ifndef __DSA_LIST
#define __DSA_LIST

#include "ListNode.h" 
#include "utils.h"

template<typename T>
class List {

private:
    int _size;
    ListNode<T>* header;
    ListNode<T>* trailer;

protected:
    void init();
    int clear();
    void copyNodes(ListNode<T>*, int);

public:
    List() {  init(); }
    List(List<T> const& L);// copy constructor
    List(List<T> const& L, Rank r, int n);
    List(ListNode<T>* p, int n);
    ~List();

    Rank size() const { return _size; }
    bool empty() const {  return _size <= 0; }
    T& operator[](Rank r) const;

    ListNode<T>* first() const {  return header->succ;  }
    ListNode<T>* last()  const {  return trailer->pred; }

    struct iterator;
    iterator begin();
    iterator end();

    // 声明 const 版本的 begin/end
    const iterator begin() const;
    const iterator end() const;

    bool valid(ListNode<T>* p) {  return p && (trailer != p) && (header!=p); }
    int disordered() const;
    ListNode<T>* find(T const& e) const {  return find(e, _size, trailer); }
    ListNode<T>* find(T const& e, int n, ListNode<T>* p) const;
    ListNode<T>* search(T const& e) const {  return search(e, _size, trailer); }
    ListNode<T>* search(T const& e, int n, ListNode<T>* p) const;

    ListNode<T>* insertAsFirst(T const& e);
    ListNode<T>* insertAsLast(T const& e);
    ListNode<T>* insert(T const& e) {  return insertAsLast(e); }
    ListNode<T>* insertA(ListNode<T>* p, T const& e);
    ListNode<T>* insertB(ListNode<T>* p, T const& e);
    T remove(ListNode<T>* p);

    int deduplicate();
    int uniquify();
    void reverse();

    void traverse(void(*)(T&));
    template<typename VST> void traverse(VST&&);
};

template<typename T>
void List<T>::init(){
    header = new ListNode<T>;
    trailer = new ListNode<T>;
    header->succ = trailer; header->pred = nullptr;
    trailer->pred = header; trailer->succ = nullptr;
    _size = 0;
}

template<typename T>
T& List<T>::operator[](Rank r) const {
    ListNode<T>* p = first();
    while(0 < r--) p = p->succ;
    return p->data;
}

template<typename T>
ListNode<T>* List<T>::find(T const& e, int n, ListNode<T>* p) const {
    while(0 < n--)
        if(e == (p = p->pred)->data) return p;
    return nullptr;
}

template<typename T>
ListNode<T>* List<T>::insertAsFirst(T const& e){
    _size++;
    return header->insertAsSucc(e);
}

template<typename T>
ListNode<T>* List<T>::insertAsLast(T const& e){
    _size++;
    return trailer->insertAsPred(e);
}

template<typename T>
ListNode<T>* List<T>::insertA(ListNode<T>* p, T const& e){
    _size++;
    return p->insertAsSucc(e);
}

template<typename T>
ListNode<T>* List<T>::insertB(ListNode<T>* p, T const& e){
    _size++;
    return p->insertAsPred(e);
}

template<typename T>
void List<T>::copyNodes(ListNode<T>* p, int n){
    init();
    while(n--){
        insertAsLast(p->data);
        p = p->succ;
    }
}

template<typename T>
List<T>::List(ListNode<T>* p, int n) { copyNodes(p, n); }

template<typename T>
List<T>::List(List<T> const& L) {   copyNodes(L.first(), L._size); }

template<typename T>
List<T>::List(List<T> const& L, int r, int n) { copyNodes(L[r], n); }

template<typename T>
T List<T>::remove(ListNode<T>* p){
    T e = p->data;
    p->pred->succ = p->succ;
    p->succ->pred = p->pred;
    delete p;
    _size--;
    return e;
}

template<typename T>
List<T>::~List(){
    clear();
    delete header;
    delete trailer;
}

template<typename T>
int List<T>::clear(){
    int oldSize = _size;
    while(0 < _size) remove(header->succ);
    return oldSize;
}

template<typename T>
int List<T>::deduplicate(){
    if(_size < 2) return 0;
    int oldSize = _size;
    ListNode<T>* p = header;
    Rank r = 0;
    while(trailer != (p = p->succ)){
        ListNode<T>* q = find(p->data, r, p);
        q ? remove(q):r++;
    }
    return oldSize - _size;
}

template<typename T>
void List<T>::traverse(void (*visit)(T&)){
    for(ListNode<T>* p = header->succ; p != trailer; p = p->succ)
        visit(p->data);
}

template<typename T>
template<typename VST>
void List<T>::traverse(VST&& visit){
    for(ListNode<T>* p = header->succ; p != trailer; p = p->succ)
        visit(p->data);
}

template<typename T>
int List<T>::uniquify(){
    if(_size < 2) 
        return 0;
    int oldSize = _size;
    ListNode<T>* p = first();
    ListNode<T>* q;
    while(trailer != (q = p->succ))
        if(p->data != q->data) p = q;
        else remove(q);
    return oldSize - _size;
}

template<typename T>
ListNode<T>* List<T>::search(T const& e, int n, ListNode<T>* p) const {
    while(0 <= n--)
        if(((p = p->pred)->data) <= e) break;
    return p;
}

template<typename T>
void List<T>::reverse(){
    ListNode<T>* node = header->succ;
    if(node == trailer)
        return;
    ListNode<T>* next = node->succ;
    while(node != trailer){
        node->succ = node->pred;
        node = next;
        next = next->succ;
    }

    ListNode<T>* temp = trailer;
    trailer = header;
    header = temp;
    trailer->succ = nullptr;

    header->succ = header->pred;
    header->pred = nullptr;

    ListNode<T>* prev = header;
    next = header->succ;

    while(next!=nullptr){
        next->pred = prev;
        prev = next;
        next = next->succ;
    }

    return;
}

template<typename T>
struct List<T>::iterator {
    ListNode<T>* cur;

    // 构造函数
    explicit iterator(ListNode<T>* rhs);

    // 比较运算符：加上 const 限定，保证能用于 const 对象的比较
    bool operator!=(const iterator& other) const;
    bool operator==(const iterator& other) const;

    // 解引用与前置 ++ 操作
    T& operator*();
    iterator& operator++();
};

// 构造函数实现
template<typename T>
inline List<T>::iterator::iterator(ListNode<T>* rhs) : cur(rhs) { }

// operator!= 实现
template<typename T>
inline bool List<T>::iterator::operator!=(const iterator& other) const {
    return cur != other.cur;
}

// operator== 实现
template<typename T>
inline bool List<T>::iterator::operator==(const iterator& other) const {
    return cur == other.cur;
}

// operator* 实现
template<typename T>
inline T& List<T>::iterator::operator*() {
    return cur->data;
}

// operator++ 实现（前置++）
template<typename T>
inline typename List<T>::iterator& List<T>::iterator::operator++() {
    cur = cur->succ;
    return *this;
}

// ---------- List::begin 和 end 的实现 ----------

// 非 const 版本
template<typename T>
inline typename List<T>::iterator List<T>::begin() {
    return iterator(header->succ);
}

template<typename T>
inline typename List<T>::iterator List<T>::end() {
    return iterator(trailer);
}

// const 版本
template<typename T>
inline const typename List<T>::iterator List<T>::begin() const {
    return iterator(header->succ);
}

template<typename T>
inline const typename List<T>::iterator List<T>::end() const {
    return iterator(trailer);
}


#endif