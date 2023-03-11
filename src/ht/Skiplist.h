#pragma once

#include "Entry.h"
#include "List.h"
#include "Dictionary.h"
#include "Quadlist.h" 


template<typename K, typename V>
class Skiplist : public Dictionary<K, V>, public List<Quadlist<Entry<K, V>>*> {
protected:
    bool skipSearch(
        ListNode<Quadlist<Entry<K, V>>*>* &qlist,
        QuadlistNode<Entry<K, V>>* &p,
        K& k
    );
public:
    int size() const {  return this->empty()? 
                        0 : this->last()->data->size();  }
    int level()  {  return List<Quadlist<Entry<K, V>>*>::size();  }//层高
    bool put(K, V);//插入
    V* get(K k);//读取
    bool remove(K k);//删除
};

template<typename K, typename V>
V* Skiplist<K, V>::get(K k){
    if(this->empty()) return nullptr;
    ListNode<Quadlist<Entry<K, V>>*>* qlist = this->first();
    QuadlistNode<Entry<K, V>>* p = qlist->data->first();//最左上角
    return skipSearch(qlist, p, k) ? &(p->entry.value) : nullptr;
}

template<typename K, typename V>
bool Skiplist<K, V>::skipSearch(
    ListNode<Quadlist<Entry<K, V>>*>* &qlist,
    QuadlistNode<Entry<K, V>>* &p,
    K& k
){
    while(true){
        while(p->succ && (p->entry.key <= k))
            p = p->succ;
        p = p->pred;  
        if(p->pred && (k == p->entry.key))  return true;//第一个条件是判断当前是否为header
        qlist = qlist->succ;//List的向下一层
        if(!qlist->succ) return false;//没有后继就意味着已经到了List的Trailer层，直接失败。
        p = (p->pred) ? p->below : qlist->data->first();//判断p是不是header,如果不是，直接向下。
                                                        //如果是，那么沿用下一层的首节点。  
    }
}

template<typename K, typename V> 
bool Skiplist<K, V>::put(K k, V v){
    Entry<K, V> e = Entry<K, V>(k, v);
    if(this->empty()) this->insertAsFirst(new Quadlist<Entry<K, V>>);
    ListNode<Quadlist<Entry<K, V>>*>* qlist = this->first();
    QuadlistNode<Entry<K, V>>* p = qlist->data->first();//该链中第一个节点
    if(skipSearch(qlist, p, k))
        while(p->below) p = p->below;//有雷同词条，强制转至塔底
    qlist = this->last();//紧邻p右侧一座新塔开始成长
    QuadlistNode<Entry<K, V>>* b = qlist->data->insertAfterAbove(e, p);

    while(dis(eng)&1) {//随机向上生长
        while(qlist->data->valid(p) && !p->above) p = p->pred;
        if(!qlist->data->valid(p)){
            if(qlist == this->first())
                this->insertAsFirst(new Quadlist<Entry<K, V>>);
            p = qlist->pred->data->first()->pred;
        } else 
            p = p->above;
        qlist = qlist->pred;
        b = qlist->data->insertAfterAbove(e, p, b);
    }
    return true;
}

template<typename K, typename V>
bool Skiplist<K, V>::remove(K k){
    if(this->empty()) return false;
    ListNode<Quadlist<Entry<K, V>>*>* qlist = this->first();
    QuadlistNode<Entry<K, V>>* p = qlist->data->first();
    if(!skipSearch(qlist, p, k)) return false;
    do {
        QuadlistNode<Entry<K, V>>* lower = p->below;//向下删除
        qlist->data->remove(p);
        p = lower;
        qlist = qlist->succ;
    } while(qlist->succ);  
    while(!this->empty() && this->first()->data->empty())//更新链表
        List<Quadlist<Entry<K, V>>*>::remove(this->first());
    return true;
}