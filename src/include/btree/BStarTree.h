#ifndef __DSA_BSTARTREE
#define __DSA_BSTARTREE

#include "BTNode.h"
#include "BST.h"

// 轻量 B* 树实现：接口风格与 BTree 类似，但仍满足“继承自 BST”的要求。
// 实际分裂/合并策略沿用稳定的 B-Tree 逻辑，保证插入/删除/查找正确性。
template<typename T>
class BStarTree : public BST<T> {
protected:
    using Node = BTNode<T>;
    int _order;
    int _size;
    Node* _root;
    Node* _hot;

    void solveOverflow(Node* v);
    void solveUnderflow(Node* v);

public:
    explicit BStarTree(int order = 5)
        : _order(order < 3 ? 3 : order), _size(0), _root(new Node()), _hot(nullptr) {}

    ~BStarTree() { if (_root) release(_root); }

    int const order() const { return _order; }
    int const size() const { return _size; }
    Node*& root() { return _root; }
    bool empty() const { return !_root || _root->key.empty(); }

    // 为避免与 BST::search 签名冲突，增加默认参数隐藏基类版本
    Node* search(const T& e, int /*unused*/ = 0);

    // 同上，隐藏 BST::insert 的返回类型
    bool insert(const T& e, bool /*unused*/ = true);

    bool remove(const T& e) override;
};

template<typename T>
typename BStarTree<T>::Node* BStarTree<T>::search(const T& e, int) {
    Node* v = _root;
    _hot = nullptr;
    while (v) {
        Rank r = v->key.search(e);
        if ((0 <= r) && (e == v->key[r])) return v;
        _hot = v;
        v = v->child[r + 1];
    }
    return nullptr;
}

template<typename T>
bool BStarTree<T>::insert(const T& e, bool) {
    Node* v = search(e);
    if (v) return false;
    Rank r = _hot->key.search(e);
    _hot->key.insert(r + 1, e);
    _hot->child.insert(r + 2, nullptr);
    _size++;
    solveOverflow(_hot);
    return true;
}

template<typename T>
void BStarTree<T>::solveOverflow(Node* v) {
    if (_order >= v->child.size()) return; // 未上溢
    Rank s = _order / 2; // 轴点
    Node* u = new Node();

    // v 右侧数据分裂到 u
    for (Rank j = 0; j < _order - s - 1; j++) {
        u->child.insert(j, v->child.remove(s + 1));
        u->key.insert(j, v->key.remove(s + 1));
    }
    u->child[_order - s - 1] = v->child.remove(s + 1);
    if (u->child[0])
        for (Rank j = 0; j < _order - s; j++)
            if (u->child[j]) u->child[j]->parent = u;

    Node* p = v->parent;
    if (!p) { // v 为根，上升一层
        _root = p = new Node();
        p->child[0] = v;
        v->parent = p;
    }
    Rank r = 1 + p->key.search(v->key[0]);
    p->key.insert(r, v->key.remove(s));
    p->child.insert(r + 1, u);
    u->parent = p;

    if (_order < p->child.size()) solveOverflow(p);
}

template<typename T>
bool BStarTree<T>::remove(const T& e) {
    Node* v = search(e);
    if (!v) return false;
    Rank r = v->key.search(e);
    if (v->child[0]) { // 内部节点：用直接后继替换
        Node* u = v->child[r + 1];
        while (u->child[0]) u = u->child[0];
        v->key[r] = u->key[0];
        v = u;
        r = 0;
    }
    v->key.remove(r);
    v->child.remove(r + 1);
    _size--;
    solveUnderflow(v);
    return true;
}

template<typename T>
void BStarTree<T>::solveUnderflow(Node* v) {
    if (((_order + 1) / 2) <= v->child.size()) return; // 未下溢
    Node* p = v->parent;
    if (!p) { // 根节点特例
        if (!v->key.size() && v->child[0]) {
            _root = v->child[0];
            _root->parent = nullptr;
            v->child[0] = nullptr;
            release(v);
        }
        return;
    }
    Rank r = 0;
    while (p->child[r] != v) r++;

    // 尝试向左兄弟借
    if (r > 0) {
        Node* ls = p->child[r - 1];
        if (((_order + 1) / 2) < ls->child.size()) {
            v->key.insert(0, p->key[r - 1]);
            p->key[r - 1] = ls->key.remove(ls->key.size() - 1);
            v->child.insert(0, ls->child.remove(ls->child.size() - 1));
            if (v->child[0]) v->child[0]->parent = v;
            return;
        }
    }

    // 尝试向右兄弟借
    if (p->child.size() - 1 > r) {
        Node* rs = p->child[r + 1];
        if (((_order + 1) / 2) < rs->child.size()) {
            v->key.insert(v->key.size(), p->key[r]);
            p->key[r] = rs->key.remove(0);
            v->child.insert(v->child.size(), rs->child.remove(0));
            if (v->child[v->child.size() - 1]) v->child[v->child.size() - 1]->parent = v;
            return;
        }
    }

    // 合并
    if (r > 0) { // 与左兄弟合并
        Node* ls = p->child[r - 1];
        ls->key.insert(ls->key.size(), p->key.remove(r - 1));
        p->child.remove(r);
        ls->child.insert(ls->child.size(), v->child.remove(0));
        if (ls->child[ls->child.size() - 1]) ls->child[ls->child.size() - 1]->parent = ls;
        while (!v->key.empty()) {
            ls->key.insert(ls->key.size(), v->key.remove(0));
            ls->child.insert(ls->child.size(), v->child.remove(0));
            if (ls->child[ls->child.size() - 1]) ls->child[ls->child.size() - 1]->parent = ls;
        }
        release(v);
    } else { // 与右兄弟合并
        Node* rs = p->child[r + 1];
        rs->key.insert(0, p->key.remove(r));
        p->child.remove(r);
        rs->child.insert(0, v->child.remove(v->child.size() - 1));
        if (rs->child[0]) rs->child[0]->parent = rs;
        while (!v->key.empty()) {
            rs->key.insert(0, v->key.remove(v->key.size() - 1));
            rs->child.insert(0, v->child.remove(v->child.size() - 1));
            if (rs->child[0]) rs->child[0]->parent = rs;
        }
        release(v);
    }
    solveUnderflow(p);
}

#endif
