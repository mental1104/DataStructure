#ifndef __DSA_BTREE
#define __DSA_BTREE

#include "BTNode.h"
#include <dsa/core/tree/BTreeAlgorithm.h>

#include <functional>
#include <utility>

namespace dsa {
namespace core {

// 将教学版 BTNode 的 Vector 布局适配为多路搜索树共享算法语义。
template<typename T>
struct TeachingBTreeNodeAccess {
    typedef ::BTNode<T> node_type;
    typedef T key_type;

    static std::size_t keyCount(const node_type* node) {
        return static_cast<std::size_t>(node->key.size());
    }
    static T& key(node_type* node, std::size_t index) {
        return node->key[static_cast<Rank>(index)];
    }
    static const T& key(const node_type* node, std::size_t index) {
        return node->key[static_cast<Rank>(index)];
    }
    // 教学节点的叶子保留 key_count + 1 个空孩子槽；算法层将其视为 0 个结构孩子。
    static std::size_t childCount(const node_type* node) {
        if (node->child.empty() || !node->child[0])
            return 0;
        return static_cast<std::size_t>(node->child.size());
    }
    static node_type* child(node_type* node, std::size_t index) {
        return node->child[static_cast<Rank>(index)];
    }
    static const node_type* child(const node_type* node, std::size_t index) {
        return node->child[static_cast<Rank>(index)];
    }
    static node_type* parent(node_type* node) { return node ? node->parent : nullptr; }
    static const node_type* parent(const node_type* node) { return node ? node->parent : nullptr; }
};

} // namespace core
} // namespace dsa

// 教学版 B-Tree：保留原有 Vector / BTNode 可观察布局，
// 节点内定位、下降、孩子索引和迭代销毁统一复用 BTreeAlgorithm。
template<typename T>
class BTree {
protected:
    typedef dsa::core::TeachingBTreeNodeAccess<T> Access;
    typedef dsa::core::BTreeAlgorithm<Access> Algorithm;

    int _size;
    int _order;
    BTNode<T>* _root;
    BTNode<T>* _hot;

    void solveOverflow(BTNode<T>* node);
    void solveUnderflow(BTNode<T>* node);

    static void destroyTree(BTNode<T>* root) {
        Algorithm::destroySubtree(root, [](BTNode<T>* node) { delete node; });
    }

    static BTNode<T>* cloneTree(const BTNode<T>* source, BTNode<T>* parent) {
        if (!source)
            return nullptr;
        BTNode<T>* clone = new BTNode<T>();
        clone->parent = parent;
        try {
            for (Rank index = 0; index < source->key.size(); ++index)
                clone->key.insert(clone->key.size(), source->key[index]);
            while (!clone->child.empty())
                clone->child.remove(clone->child.size() - 1);
            for (Rank index = 0; index < source->child.size(); ++index)
                clone->child.insert(clone->child.size(), cloneTree(source->child[index], clone));
        } catch (...) {
            destroyTree(clone);
            throw;
        }
        return clone;
    }

public:
    explicit BTree(int order = 512)
        : _size(0), _order(order < 3 ? 3 : order), _root(new BTNode<T>()), _hot(nullptr) {}

    BTree(const BTree& other)
        : _size(other._size), _order(other._order), _root(nullptr), _hot(nullptr) {
        _root = cloneTree(other._root, nullptr);
    }

    BTree(BTree&& other) noexcept
        : _size(other._size), _order(other._order), _root(other._root), _hot(nullptr) {
        other._size = 0;
        other._root = nullptr;
        other._hot = nullptr;
    }

    BTree& operator=(BTree other) {
        swap(other);
        return *this;
    }

    ~BTree() { destroyTree(_root); }

    void swap(BTree& other) noexcept {
        using std::swap;
        swap(_size, other._size);
        swap(_order, other._order);
        swap(_root, other._root);
        _hot = nullptr;
        other._hot = nullptr;
    }

    int order() const { return _order; }
    int size() const { return _size; }
    BTNode<T>*& root() { return _root; }
    const BTNode<T>* root() const { return _root; }
    bool empty() const { return !_root || _root->key.empty(); }

    BTNode<T>* search(const T& e);
    bool insert(const T& e);
    bool remove(const T& e);

    template<typename Res, typename Agg>
    Res rangeAggregate(const T& lo, const T& hi, Res identity, Agg&& agg) const {
        return rangeAggregateRec(_root, lo, hi, identity, agg);
    }

private:
    template<typename Res, typename Agg>
    Res rangeAggregateRec(BTNode<T>* node, const T& lo, const T& hi, Res acc, Agg&& agg) const {
        if (!node)
            return acc;
        const int key_count = node->key.size();
        for (int index = 0; index < key_count; ++index) {
            BTNode<T>* left = node->child[index];
            if (left)
                acc = rangeAggregateRec(left, lo, hi, acc, agg);
            if (!(node->key[index] < lo) && !(hi < node->key[index]))
                acc = agg(acc, node->key[index]);
        }
        if (node->child.size() == key_count + 1 && node->child[key_count])
            acc = rangeAggregateRec(node->child[key_count], lo, hi, acc, agg);
        return acc;
    }
};

template<typename T>
BTNode<T>* BTree<T>::search(const T& e) {
    if (!_root) {
        _hot = nullptr;
        return nullptr;
    }
    typename Algorithm::SearchResult result = Algorithm::search(_root, e, std::less<T>());
    _hot = result.found ? result.node->parent : result.node;
    return result.found ? result.node : nullptr;
}

template<typename T>
bool BTree<T>::insert(const T& e) {
    if (!_root)
        _root = new BTNode<T>();
    BTNode<T>* found = search(e);
    if (found)
        return false;

    const Rank index = static_cast<Rank>(Algorithm::lowerBound(_hot, e, std::less<T>()));
    _hot->key.insert(index, e);
    _hot->child.insert(index + 1, nullptr);
    ++_size;
    solveOverflow(_hot);
    return true;
}

template<typename T>
void BTree<T>::solveOverflow(BTNode<T>* node) {
    while (node && node->child.size() > _order) {
        const Rank middle = node->key.size() / 2;
        BTNode<T>* right = new BTNode<T>();
        while (!right->child.empty())
            right->child.remove(right->child.size() - 1);

        for (Rank index = middle + 1; index < node->key.size();) {
            right->key.insert(right->key.size(), node->key.remove(middle + 1));
        }
        for (Rank index = middle + 1; index < node->child.size();) {
            BTNode<T>* child = node->child.remove(middle + 1);
            right->child.insert(right->child.size(), child);
            if (child)
                child->parent = right;
        }

        T promoted = node->key.remove(middle);
        BTNode<T>* parent = node->parent;
        if (!parent) {
            parent = new BTNode<T>();
            parent->child[0] = node;
            node->parent = parent;
            _root = parent;
        }

        const Rank position = static_cast<Rank>(Algorithm::childIndex(parent, node));
        parent->key.insert(position, promoted);
        parent->child.insert(position + 1, right);
        right->parent = parent;
        node = parent;
    }
}

template<typename T>
bool BTree<T>::remove(const T& e) {
    BTNode<T>* node = search(e);
    if (!node)
        return false;

    Rank index = static_cast<Rank>(Algorithm::lowerBound(node, e, std::less<T>()));
    if (node->child[0]) {
        BTNode<T>* successor = node->child[index + 1];
        while (successor->child[0])
            successor = successor->child[0];
        node->key[index] = successor->key[0];
        node = successor;
        index = 0;
    }

    node->key.remove(index);
    node->child.remove(index + 1);
    --_size;
    solveUnderflow(node);
    return true;
}

template<typename T>
void BTree<T>::solveUnderflow(BTNode<T>* node) {
    while (node && node != _root && node->child.size() < (_order + 1) / 2) {
        BTNode<T>* parent = node->parent;
        const Rank position = static_cast<Rank>(Algorithm::childIndex(parent, node));

        if (position > 0) {
            BTNode<T>* left = parent->child[position - 1];
            if (left->child.size() > (_order + 1) / 2) {
                node->key.insert(0, parent->key[position - 1]);
                parent->key[position - 1] = left->key.remove(left->key.size() - 1);
                node->child.insert(0, left->child.remove(left->child.size() - 1));
                if (node->child[0])
                    node->child[0]->parent = node;
                return;
            }
        }

        if (position + 1 < parent->child.size()) {
            BTNode<T>* right = parent->child[position + 1];
            if (right->child.size() > (_order + 1) / 2) {
                node->key.insert(node->key.size(), parent->key[position]);
                parent->key[position] = right->key.remove(0);
                node->child.insert(node->child.size(), right->child.remove(0));
                if (node->child[node->child.size() - 1])
                    node->child[node->child.size() - 1]->parent = node;
                return;
            }
        }

        if (position > 0) {
            BTNode<T>* left = parent->child[position - 1];
            left->key.insert(left->key.size(), parent->key.remove(position - 1));
            parent->child.remove(position);
            while (!node->key.empty()) {
                left->key.insert(left->key.size(), node->key.remove(0));
            }
            while (!node->child.empty()) {
                BTNode<T>* child = node->child.remove(0);
                left->child.insert(left->child.size(), child);
                if (child)
                    child->parent = left;
            }
            delete node;
        } else {
            BTNode<T>* right = parent->child[position + 1];
            node->key.insert(node->key.size(), parent->key.remove(position));
            parent->child.remove(position + 1);
            while (!right->key.empty()) {
                node->key.insert(node->key.size(), right->key.remove(0));
            }
            while (!right->child.empty()) {
                BTNode<T>* child = right->child.remove(0);
                node->child.insert(node->child.size(), child);
                if (child)
                    child->parent = node;
            }
            delete right;
        }
        node = parent;
    }

    if (_root && _root->key.empty() && _root->child[0]) {
        BTNode<T>* old_root = _root;
        _root = old_root->child[0];
        _root->parent = nullptr;
        old_root->child[0] = nullptr;
        delete old_root;
    }
}

#endif
