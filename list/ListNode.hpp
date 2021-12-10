#pragma once  

using Rank = int;

template<typename T>
struct ListNode {
    T data;
    ListNode<T>* pred;
    ListNode<T>* succ;

    ListNode(){}
    ListNode(T e, ListNode<T>* p = nullptr, ListNode<T>* s = nullptr)
        : data(e), pred(p), succ(s) {}

    ListNode<T>* insertAsPred(T const& e);
    ListNode<T>* insertAsSucc(T const& e);  
};

template<typename T>
ListNode<T>* ListNode<T>::insertAsPred(T const& e){
    ListNode<T>* x = new ListNode(e, pred, this);
    pred->succ = x;
    pred = x;
    return x;
}

template<typename T>
ListNode<T>* ListNode<T>::insertAsSucc(T const& e){
    ListNode<T>* x = new ListNode(e, this, succ);
    succ->pred = x;
    succ = x;
    return x;
}