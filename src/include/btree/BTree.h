#ifndef __DSA_BTREE
#define __DSA_BTREE

#include "BTNode.h"  

template<typename T>
class BTree {
protected:
    int _size;
    int _order;
    BTNode<T>* _root;
    BTNode<T>* _hot;
    void solveOverflow(BTNode<T>*);
    void solveUnderflow(BTNode<T>*);
public:
    BTree(int order = 512):_order(order), _size(0){   _root = new BTNode<T>();    }
    ~BTree(){   if(_root) release(_root);   }

    int const order() { return _order; }
    int const size()  {  return _size; }
    BTNode<T>*& root() {  return _root; }
    bool  empty() const { return !_root || _root->key.empty(); }

    BTNode<T>* search (const T& e);
    bool insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
BTNode<T>* 
BTree<T>::search(const T& e){
    BTNode<T>* v = _root;
    _hot = nullptr;
    while(v){
        Rank r = v->key.search(e);//查找返回的要求是不大于给定值的最大值。
        if((0 <= r) && (e ==  v->key[r]))  return v;
        _hot = v; 
        v = v->child[r+1];//返回大于e值的前一个位置，故r+1.
    }
    return nullptr;
}

template<typename T> 
bool BTree<T>::insert(const T& e){
    BTNode<T>* v = search(e);
    if(v)
        return false;
    Rank r = _hot->key.search(e);
    _hot->key.insert(r+1, e);
    _hot->child.insert(r+2, nullptr);
    _size++;
    solveOverflow(_hot);
    return true;
}

template<typename T>
void BTree<T>::solveOverflow(BTNode<T>* v){
    if ( _order >= v->child.size() ) return; //递归基：当前节点并未上溢
    Rank s = _order / 2; //轴点（此时应有_order = key.size() = child.size() - 1）
    BTNode<T>* u = new BTNode<T>(); //注意：新节点已有一个空孩子

    /* 原节点一分为二，并将右侧节点更新至新节点 */
    for ( Rank j = 0; j < _order - s - 1; j++ ) { //v右侧_order-s-1个孩子及关键码分裂为右侧节点u
        u->child.insert ( j, v->child.remove ( s + 1 ) ); //逐个移动效率低
        u->key.insert ( j, v->key.remove ( s + 1 ) ); //此策略可改进
    }

    u->child[_order - s - 1] = v->child.remove ( s + 1 ); //单独一次移动v最靠右的孩子
    /* 原节点一分为二，并将右侧节点更新至新节点 */


    /*更新父节点 */
    if ( u->child[0] ) //若u的孩子们非空，则
        for ( Rank j = 0; j < _order - s; j++ ) //令它们的父节点统一
            u->child[j]->parent = u; //指向u
    /*更新父节点 */

    BTNode<T>* p = v->parent; //v当前的父节点p

    /* 上溢到根节点时 */
    if ( !p ) { 
        _root = p = new BTNode<T>(); 
        p->child[0] = v; 
        v->parent = p; 
    } //若p空则创建之
    /* 上溢到根节点时 */


    Rank r = 1 + p->key.search ( v->key[0] ); //在父节点中找到待插入的位置
    p->key.insert ( r, v->key.remove ( s ) ); //轴点关键码上升

    p->child.insert ( r + 1, u );  
    u->parent = p; //新节点u与父节点p互联
    
    solveOverflow ( p ); //上升一层，如有必要则继续分裂——至多递归O(logn)层
}

template<typename T>
bool BTree<T>::remove(const T& e){
    BTNode<T>* v = search(e);
    if(!v)  return false;
    Rank r = v->key.search(e);
    if(v->child[0]){
        BTNode<T>* u = v->child[r+1];
        while(u->child[0]) u = u->child[0];//类似于二叉搜索树的直接后继，先右然后一左到底
        v->key[r] = u->key[0];//用直接后继覆盖当前被删除节点
        v = u;//转交节点控制权
        r = 0;//准备删除原直接后继
    }
    v->key.remove(r);
    v->child.remove(r+1);
    _size--;
    solveUnderflow(v);
    return true;
}

template<typename T>
void BTree<T>::solveUnderflow(BTNode<T>* v){
    if ( ( _order + 1 ) / 2 <= v->child.size() ) return; //递归基：当前节点并未下溢
    BTNode<T>* p = v->parent;
    if ( !p ) { //递归基：已到根节点，没有孩子的下限
        if ( !v->key.size() && v->child[0] ) {
            //但倘若作为树根的v已不含关键码，却有（唯一的）非空孩子，则
            ///*DSA*/printf ( "collapse\n" );
            _root = v->child[0]; _root->parent = NULL; //这个节点可被跳过
            v->child[0] = NULL; 
            release ( v ); //并因不再有用而被销毁
        } //整树高度降低一层
        return;
    }
    Rank r = 0; while ( p->child[r] != v ) r++;
    //确定v是p的第r个孩子——此时v可能不含关键码，故不能通过关键码查找

    //另外，在实现了孩子指针的判等器之后，也可直接调用Vector::find()定位
    ///*DSA*/printf ( "\nrank = %d", r );
    // 情况1：向左兄弟借关键码 - 旋转

    if ( 0 < r ) { //若v不是p的第一个孩子，则
        BTNode<T>* ls = p->child[r - 1]; //左兄弟必存在
        if ( ( _order + 1 ) / 2 < ls->child.size() ) { //若该兄弟足够“胖”，则
            ///*DSA*/printf ( " ... case 1\n" );
            v->key.insert ( 0, p->key[r - 1] ); //p借出一个关键码给v（作为最小关键码）
            p->key[r - 1] = ls->key.remove ( ls->key.size() - 1 ); //ls的最大关键码转入p
            //r - 1的原因是左兄弟
            v->child.insert ( 0, ls->child.remove ( ls->child.size() - 1 ) );
            //同时ls的最右侧孩子过继给v
            if ( v->child[0] ) v->child[0]->parent = v; //作为v的最左侧孩子
            return; //至此，通过右旋已完成当前层（以及所有层）的下溢处理
        }
    } //至此，左兄弟要么为空，要么太“瘦”

    // 情况2：向右兄弟借关键码 - 旋转
    if ( p->child.size() - 1 > r ) { //若v不是p的最后一个孩子，则
        BTNode<T>* rs = p->child[r + 1]; //右兄弟必存在
        if ( ( _order + 1 ) / 2 < rs->child.size() ) { //若该兄弟足够“胖”，则
            ///*DSA*/printf ( " ... case 2\n" );
            v->key.insert ( v->key.size(), p->key[r] ); //p借出一个关键码给v（作为最大关键码）
            p->key[r] = rs->key.remove ( 0 ); //rs的最小关键码转入p
            v->child.insert ( v->child.size(), rs->child.remove ( 0 ) );
            //同时rs的最左侧孩子过继给v
            if ( v->child[v->child.size() - 1] ) //作为v的最右侧孩子
            v->child[v->child.size() - 1]->parent = v;
            return; //至此，通过左旋已完成当前层（以及所有层）的下溢处理
        }
    } //至此，右兄弟要么为空，要么太“瘦”

    // 情况3：左、右兄弟要么为空（但不可能同时），要么都太“瘦”——合并
    if ( 0 < r ) { //与左兄弟合并
        ///*DSA*/printf ( " ... case 3L\n" );
        BTNode<T>* ls = p->child[r - 1]; //左兄弟必存在
        ls->key.insert ( ls->key.size(), p->key.remove ( r - 1 ) ); p->child.remove ( r );
        //p的第r - 1个关键码转入ls，v不再是p的第r个孩子
        ls->child.insert ( ls->child.size(), v->child.remove ( 0 ) );
        if ( ls->child[ls->child.size() - 1] ) //v的最左侧孩子过继给ls做最右侧孩子
            ls->child[ls->child.size() - 1]->parent = ls;
        while ( !v->key.empty() ) { //v剩余的关键码和孩子，依次转入ls
            ls->key.insert ( ls->key.size(), v->key.remove ( 0 ) );
            ls->child.insert ( ls->child.size(), v->child.remove ( 0 ) );
            if ( ls->child[ls->child.size() - 1] ) ls->child[ls->child.size() - 1]->parent = ls;
        }
        release ( v ); //释放v
    } else { //与右兄弟合并
        ///*DSA*/printf ( " ... case 3R\n" );
        BTNode<T>* rs = p->child[r + 1]; //右兄弟必存在
        rs->key.insert ( 0, p->key.remove ( r ) ); p->child.remove ( r );
        //p的第r个关键码转入rs，v不再是p的第r个孩子
        rs->child.insert ( 0, v->child.remove ( v->child.size() - 1 ) );
        if ( rs->child[0] ) rs->child[0]->parent = rs; //v的最右侧孩子过继给rs做最左侧孩子
        while ( !v->key.empty() ) { //v剩余的关键码和孩子，依次转入rs
            rs->key.insert ( 0, v->key.remove ( v->key.size() - 1 ) );
            rs->child.insert ( 0, v->child.remove ( v->child.size() - 1 ) );
            if ( rs->child[0] ) rs->child[0]->parent = rs;
        }
        release ( v ); //释放v
    }
    solveUnderflow ( p ); //上升一层，如有必要则继续分裂——至多递归O(logn)层
    return;
}


#endif