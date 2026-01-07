#ifndef __DSA_REDBLACK
#define __DSA_REDBLACK

#include "BST.h"  

template<typename T>
class RedBlack : public BST<T> {
protected:
    void solveDoubleRed(BinNode<T>* x);
    void solveDoubleBlack(BinNode<T>* x);
    int updateHeight(BinNode<T>* x); 
public: 
    BinNode<T>* insert(const T& e);
    bool remove(const T& e);
};

template<typename T>
inline bool IsBlack(BinNode<T>* p){
    return !p || p->color == RBColor::BLACK;
}

template<typename T>
inline bool IsRed(BinNode<T>* p){
    return !IsBlack(p);
}

template<typename T>
inline bool BlackHeightUpdated(BinNode<T>& x){
    return (stature(x.lc) == stature(x.rc)) && x.height == (IsRed(&x)?stature(x.lc):stature(x.lc)+1); 
}

template<typename T>
int RedBlack<T>::updateHeight(BinNode<T>* x){
    x->height = max(stature(x->lc), stature(x->rc));
    return IsBlack(x) ? x->height++ : x->height;//黑高度
}

template<typename T>
BinNode<T>* RedBlack<T>::insert(const T& e){
    BinNode<T>*& x = this->search(e);
    if(x)
        return x;
    x = new BinNode<T>(e, this->_hot, nullptr, nullptr, -1);
    this->_size++;
    solveDoubleRed(x);
    return x?x:this->_hot->parent;
}

template<typename T>
void RedBlack<T>::solveDoubleRed(BinNode<T>* x){
    if(IsRoot(*x)){//已经是根节点
        this->_root->color = RBColor::BLACK;
        this->_root->height++;
        return;
    }

    BinNode<T>* p = x->parent;
    if(IsBlack(p)) return;

    BinNode<T>* g = p->parent;
    BinNode<T>* u = uncle(x);

    if(IsBlack(u)){// RR-1-u为黑色 (黑节点(含NULL) )
        if(IsLChild(*x) == IsLChild(*p))//同侧：zig-zig或zag-zag
            p->color = RBColor::BLACK;// parent转黑;
        else//异测：zig-zag或zag-zig
            x->color = RBColor::BLACK;// x转黑;
        g->color = RBColor::RED;

        BinNode<T>* gg = g->parent;//曾祖父节点
        BinNode<T>*& fromParent = this->FromParentTo(*g);
        BinNode<T>* r = this->rotateAt(x);//新根节点
        fromParent = r;
        r->parent = gg;
    } else {// RR-2-u为红色
        p->color = RBColor::BLACK;//向上传递红色
        p->height++;//黑高度更新
        u->color = RBColor::BLACK;//向上传递红色
        u->height++;//黑高度更新
        if(!IsRoot(*g)) g->color = RBColor::RED;
        solveDoubleRed(g);
    }
}

template<typename T>
bool RedBlack<T>::remove(const T& e){
    BinNode<T>*& x = this->search(e);//寻找位置
    if(!x)
        return false;

    BinNode<T>* r = removeAt(x, this->_hot);
    if(!(--this->_size))//若只有根节点被删除了
        return true;//直接返回

    if(!this->_hot){//如果删除的是根节点，后序还有其他节点
        this->_root->color = RBColor::BLACK;
        this->updateHeight(this->_root);
        return true;
    }

    if(BlackHeightUpdated(*this->_hot))//若仍然黑平衡 
        return true;//无需调整

    if(IsRed(r)){// (b) 
        r->color = RBColor::BLACK;//只需单纯地将后继变为黑色
        r->height++;//更新黑高度
        return true;
    }

    solveDoubleBlack(r);//双黑调整，黑高度冲突
    return true;
}

template<typename T>
void RedBlack<T>::solveDoubleBlack(BinNode<T>* r){
    BinNode<T>* p = r ? r->parent: this->_hot;//后继节点,没有则以_hot代替
    if(!p) 
        return;
    BinNode<T>* s = (r == p->lc) ? p->rc:p->lc;

    if(IsBlack(s)){//兄弟s为黑
        BinNode<T>* t = nullptr;// s的红孩子，皆红时左者优先。
        if(IsRed(s->rc)) t = s->rc;
        if(IsRed(s->lc)) t = s->lc;
        if(t){// BB-1：黑s有至少一个红孩子
            RBColor oldcolor = p->color;//保存p原先的颜色
            BinNode<T>*& fromParent = this->FromParentTo(*p);
            BinNode<T>* b = this->rotateAt(t);//zig-zig,  return s
            fromParent = b;
            
            if(HasLChild(*b)){ 
                b->lc->color = RBColor::BLACK;//将平衡后的左孩子设为黑
                this->updateHeight(b->lc);
            }

            if(HasRChild(*b)){
                b->rc->color = RBColor::BLACK;//将平衡后的右孩子设为黑
                this->updateHeight(b->rc);
            }

            b->color = oldcolor;//s继承p的颜色
            this->updateHeight(b);
        } else {//黑s没有红孩子
            s->color = RBColor::RED;//先将s自己染为红色
            s->height--;
            if(IsRed(p))//BB-2R, p为红
                p->color = RBColor::BLACK;//若p为红色，调换s和p的颜色
                //p的另一侧黑高度变为正常，原黑高度依然不变
            else{ //BB-2B, p为黑
                p->height--;//下层下溢引发上层下溢，因此p的黑高度减1
                solveDoubleBlack(p);//继续递归处理上述节点。 
            }
        }
    } else { //BB-3 兄弟s本来即为红
        s->color = RBColor::BLACK;
        p->color = RBColor::RED;
        BinNode<T>* t = IsLChild(*s)?s->lc:s->rc;
        this->_hot = p;
        BinNode<T>*& fromParent = this->FromParentTo(*p);
        BinNode<T>* b = this->rotateAt(t);
        fromParent = b;
        solveDoubleBlack(r);//x的后继节点r此时的兄弟必为黑，转入BB-1 或 BB-2-R
    }
}

#endif

    
