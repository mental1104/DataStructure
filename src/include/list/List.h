#pragma once

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
    void merge(ListNode<T>*&, int, List<T>&, ListNode<T>*, int);
    void mergeSort(ListNode<T>*&, int);
    void selectionSort(ListNode<T>*, int);
    void insertionSort(ListNode<T>*, int);

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

    bool valid(ListNode<T>* p) {  return p && (trailer != p) && (header!=p); }
    int disordered() const;
    ListNode<T>* find(T const& e) const {  return find(e, _size, trailer); }
    ListNode<T>* find(T const& e, int n, ListNode<T>* p) const;
    ListNode<T>* search(T const& e) const {  return search(e, _size, trailer); }
    ListNode<T>* search(T const& e, int n, ListNode<T>* p) const;
    ListNode<T>* selectMax(ListNode<T>* p, int n);
    ListNode<T>* selectMax() {    return selectMax(header->succ,_size);  }
    ListNode<T>* insertAsFirst(T const& e);
    ListNode<T>* insertAsLast(T const& e);
    ListNode<T>* insert(T const& e) {  return insertAsLast(e); }
    ListNode<T>* insertA(ListNode<T>* p, T const& e);
    ListNode<T>* insertB(ListNode<T>* p, T const& e);
    T remove(ListNode<T>* p);
    void merge(List<T>& L) { merge(first(), _size, L, L.first(), L._size); }
    void sort(ListNode<T>* p, int n);
    void sort() { sort(first(), _size); }
    int deduplicate();
    int uniquify();
    void reverse();
    void radixSort(ListNode<T>* p, int n);

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
void List<T>::sort(ListNode<T>* p, int n){
    mergeSort(p, n);
    /*
    switch(dice(2021)%3){
        case 1: 
            printf("InsertionSort.\n");
            insertionSort(p, n); 
            break;
        case 2: 
            printf("SelectionSort.\n");
            selectionSort(p, n); 
            break;
        default: 
            printf("MergeSort.\n");
            mergeSort(p, n); 
            break;
    }
    */
}

template<typename T>
void List<T>::insertionSort(ListNode<T>* p, int n){
    for(int r = 0; r < n; r++){
        insertA(search(p->data, r, p), p->data);
        p = p->succ;
        remove(p->pred);
    }
}

template<typename T>
void List<T>::selectionSort(ListNode<T>* p, int n){
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++)
        tail = tail->succ;
    while(1 < n){
        ListNode<T>* max = selectMax(head->succ, n);
        insertB(tail, remove(max));
        tail = tail->pred;
        n--;
    }
}

template<typename T>
ListNode<T>* List<T>::selectMax(ListNode<T>* p, int n){
    ListNode<T>* max = p;
    for(ListNode<T>* cur = p; 1 < n; n--){
        cur = cur->succ;
        if(!(cur->data < max->data))
            max = cur;
    }
    return max;
}

template<typename T>
void List<T>::merge(ListNode<T>*& p, int n, List<T>& L, ListNode<T>* q, int m){
    ListNode<T>* pp = p->pred;//借助前驱充当头哨兵节点
    while(0 < m)
        if((0 < n) && (p->data <= q->data)){
            if(q == (p = p->succ))
                break;
            n--;
        } else {
            insertB(p, L.remove((q = q->succ)->pred)); 
            m--;
        }
    p = pp->succ;
}

template<typename T>
void List<T>::mergeSort(ListNode<T>*& p, int n){
    if(n < 2)
        return;
    int m = n/2;//1->2->3->4->5->6->7
    ListNode<T>* q = p;
    for(int i = 0; i < m; i++)
        q = q->succ;//将链表一分为二
    mergeSort(p, m);//1->2->3
    mergeSort(q, n-m);//4->5->6->7
    merge(p, m, *this, q, n-m);
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
void List<T>::radixSort(ListNode<T>* p, int n){
    ListNode<T>* head = p->pred;
    ListNode<T>* tail = p;
    for(int i = 0; i < n; i++) tail = tail->succ;
    for(U radixBit = 0x1; radixBit && (p = head); radixBit <<= 1)
        for(int i = 0; i < n; i++){
            if(radixBit & U(p->succ->data))
                tail->insertAsPred(remove(p->succ));
            else
                p = p->succ;
        }
    return;
}

template<typename T>
struct List<T>::iterator {
    ListNode<T>* cur;

    explicit iterator(ListNode<T>* rhs)
        : cur{rhs} {}
    
    bool operator!=(const iterator& other){
        return cur != other.cur;
    }

    T& operator*() {
        return cur->data;
    }

    iterator& operator++()
    {
        cur = cur->succ;
        return *this;
    }
};

template<typename T>
typename List<T>::iterator
List<T>::begin() {
    return iterator{header->succ};
}

template<typename T>
typename List<T>::iterator
List<T>::end() {
    return iterator{trailer};
}
