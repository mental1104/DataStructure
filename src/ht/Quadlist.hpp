#pragma once

using Rank = int;

#include "./QuadlistNode.hpp"

template<typename T>
class Quadlist {
private:
    int _size;
    QuadlistNode<T>* header;
    QuadlistNode<T>* trailer;
protected:
    void init();
    int clear();
public:
    Quadlist() {   init();  }
    ~Quadlist() {   clear(); delete header; delete trailer; }
    Rank size() const { return _size; }
    bool empty() const {  return _size <= 0; }
    QuadlistNode<T>* first()  const {  return header->succ;  }
    QuadlistNode<T>* last()   const {  return trailer->pred;  }
    bool valid(QuadlistNode<T>* p)  {  return p && (trailer != p) && (header != p);  }

    T remove(QuadlistNode<T>* p);
    QuadlistNode<T>* insertAfterAbove(T const& e, QuadlistNode<T>* p, QuadlistNode<T>* b = nullptr);

    void traverse(void (*)(T&));
    template<typename VST> void traverse(VST&&);
};

template<typename T>
void Quadlist<T>::init() {
    header = new QuadlistNode<T>;
    trailer = new QuadlistNode<T>;
    header->succ = trailer;
    header->pred = nullptr;
    trailer->pred = header;
    trailer->succ = nullptr;
    header->above = trailer->above = nullptr;
    header->below = trailer->below = nullptr;
    _size = 0;
}

template<typename T>
QuadlistNode<T>* Quadlist<T>::insertAfterAbove(T const& e, QuadlistNode<T>* p, QuadlistNode<T>* b){
    _size++;
    return p->insertAsSuccAbove(e, b);
}

template <typename T> //删除Quadlist内位置p处的节点，返回其中存放的词条
T Quadlist<T>::remove ( QuadlistNode<T>* p ) { //assert: p为Quadlist中的合法位置
   p->pred->succ = p->succ; p->succ->pred = p->pred; _size--;//摘除节点
   T e = p->entry; delete p; //备份词条，释放节点
   return e; //返回词条
}

template<typename T>
int Quadlist<T>::clear(){
    int oldSize = _size;
    while (0 < _size) remove(header->succ);
    return oldSize;
}

template <typename T> //遍历Quadlist，对各节点依次实施visit操作
void Quadlist<T>::traverse ( void ( *visit ) ( T& ) ) { //利用函数指针机制，只读或局部性修改
   QuadlistNode<T>* p = header;
   while ( ( p = p->succ ) != trailer ) visit ( p->data );
}

template <typename T> template <typename VST> //遍历Quadlist，对各节点依次实施visit操作
void Quadlist<T>::traverse( VST&& visit ) { //利用函数对象机制，可全局性修改
   QuadlistNode<T>* p = header;
   while ( ( p = p->succ ) != trailer ) visit ( p->data );
}

