#ifndef __DSA_BPLUSTREE
#define __DSA_BPLUSTREE

#include <functional>
#include <utility>

#include "Vector.h"

template<typename Key, typename Value>
struct BPTNode {
    bool leaf{false};
    BPTNode<Key, Value>* parent{nullptr};
    Vector<Key> key;
    Vector<Value> value;
    Vector<BPTNode<Key, Value>*> child;
    BPTNode<Key, Value>* next{nullptr};
    BPTNode<Key, Value>* prev{nullptr};

    explicit BPTNode(bool isLeaf) : leaf(isLeaf) {}
};

// 教学版 B+ Tree：独立维护多路节点和叶链，不再伪继承二叉 BST。
template<typename Key, typename Value, typename Compare = std::less<Key> >
class BPlusTree {
private:
    typedef BPTNode<Key, Value> Node;

    Node* _root{nullptr};
    int _order;
    int _size{0};
    Compare _cmp;

    int maxKeys() const { return _order - 1; }
    int minLeafKeys() const { return _order / 2; }
    int minInternalKeys() const { return (_order + 1) / 2 - 1; }

    bool equal(const Key& left, const Key& right) const {
        return !_cmp(left, right) && !_cmp(right, left);
    }

    int lowerBound(const Vector<Key>& keys, const Key& key) const {
        int first = 0;
        int count = keys.size();
        while (count > 0) {
            const int step = count / 2;
            const int middle = first + step;
            if (_cmp(keys[middle], key)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    int upperBound(const Vector<Key>& keys, const Key& key) const {
        int first = 0;
        int count = keys.size();
        while (count > 0) {
            const int step = count / 2;
            const int middle = first + step;
            if (!_cmp(key, keys[middle])) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    Node* findLeaf(const Key& key) const {
        Node* node = _root;
        while (node && !node->leaf)
            node = node->child[upperBound(node->key, key)];
        return node;
    }

    Node* firstLeaf() const {
        Node* node = _root;
        while (node && !node->leaf)
            node = node->child[0];
        return node;
    }

    int childIndex(Node* parent, Node* child) const {
        for (int index = 0; index < parent->child.size(); ++index) {
            if (parent->child[index] == child)
                return index;
        }
        return -1;
    }

    Node* splitLeaf(Node* leaf) {
        const int middle = leaf->key.size() / 2;
        Node* right = new Node(true);
        right->parent = leaf->parent;
        while (leaf->key.size() > middle) {
            right->key.insert(right->key.size(), leaf->key.remove(middle));
            right->value.insert(right->value.size(), leaf->value.remove(middle));
        }

        right->next = leaf->next;
        if (right->next)
            right->next->prev = right;
        leaf->next = right;
        right->prev = leaf;
        return right;
    }

    std::pair<Node*, Key> splitInternal(Node* node) {
        const int middle = node->key.size() / 2;
        Key promoted = node->key[middle];
        Node* right = new Node(false);
        right->parent = node->parent;

        while (node->key.size() > middle + 1)
            right->key.insert(right->key.size(), node->key.remove(middle + 1));
        while (node->child.size() > middle + 1) {
            Node* child = node->child.remove(middle + 1);
            right->child.insert(right->child.size(), child);
            if (child)
                child->parent = right;
        }
        node->key.remove(middle);
        return std::make_pair(right, promoted);
    }

    void insertIntoParent(Node* left, const Key& key, Node* right) {
        if (!left->parent) {
            Node* root = new Node(false);
            root->key.insert(0, key);
            root->child.insert(0, left);
            root->child.insert(1, right);
            left->parent = root;
            right->parent = root;
            _root = root;
            return;
        }

        Node* parent = left->parent;
        const int index = childIndex(parent, left);
        parent->key.insert(index, key);
        parent->child.insert(index + 1, right);
        right->parent = parent;
        if (parent->key.size() > maxKeys()) {
            std::pair<Node*, Key> split = splitInternal(parent);
            insertIntoParent(parent, split.second, split.first);
        }
    }

    void updateAncestorMinimum(Node* node) {
        while (node && node->parent) {
            Node* parent = node->parent;
            const int index = childIndex(parent, node);
            if (index > 0) {
                if (!node->key.empty())
                    parent->key[index - 1] = node->key[0];
                return;
            }
            node = parent;
        }
    }

    void borrowFromLeftLeaf(Node* node, Node* left, int indexInParent) {
        node->key.insert(0, left->key.remove(left->key.size() - 1));
        node->value.insert(0, left->value.remove(left->value.size() - 1));
        node->parent->key[indexInParent - 1] = node->key[0];
    }

    void borrowFromRightLeaf(Node* node, Node* right, int indexInParent) {
        node->key.insert(node->key.size(), right->key.remove(0));
        node->value.insert(node->value.size(), right->value.remove(0));
        node->parent->key[indexInParent] = right->key[0];
    }

    void borrowFromLeftInternal(Node* node, Node* left, int indexInParent) {
        node->key.insert(0, node->parent->key[indexInParent - 1]);
        Node* child = left->child.remove(left->child.size() - 1);
        node->child.insert(0, child);
        if (child)
            child->parent = node;
        node->parent->key[indexInParent - 1] = left->key.remove(left->key.size() - 1);
    }

    void borrowFromRightInternal(Node* node, Node* right, int indexInParent) {
        node->key.insert(node->key.size(), node->parent->key[indexInParent]);
        Node* child = right->child.remove(0);
        node->child.insert(node->child.size(), child);
        if (child)
            child->parent = node;
        node->parent->key[indexInParent] = right->key.remove(0);
    }

    void mergeLeaves(Node* left, Node* right, int indexInParent) {
        while (!right->key.empty()) {
            left->key.insert(left->key.size(), right->key.remove(0));
            left->value.insert(left->value.size(), right->value.remove(0));
        }
        left->next = right->next;
        if (left->next)
            left->next->prev = left;

        Node* parent = left->parent;
        parent->key.remove(indexInParent);
        parent->child.remove(indexInParent + 1);
        delete right;

        if (parent == _root && parent->key.empty()) {
            _root = left;
            left->parent = nullptr;
            delete parent;
        } else if (parent != _root && parent->key.size() < minInternalKeys()) {
            handleUnderflow(parent);
        }
    }

    void mergeInternal(Node* left, Node* right, int indexInParent) {
        Node* parent = left->parent;
        left->key.insert(left->key.size(), parent->key.remove(indexInParent));
        while (!right->key.empty())
            left->key.insert(left->key.size(), right->key.remove(0));
        while (!right->child.empty()) {
            Node* child = right->child.remove(0);
            left->child.insert(left->child.size(), child);
            if (child)
                child->parent = left;
        }
        parent->child.remove(indexInParent + 1);
        delete right;

        if (parent == _root && parent->key.empty()) {
            _root = left;
            left->parent = nullptr;
            delete parent;
        } else if (parent != _root && parent->key.size() < minInternalKeys()) {
            handleUnderflow(parent);
        }
    }

    void handleUnderflow(Node* node) {
        Node* parent = node->parent;
        if (!parent)
            return;

        const int index = childIndex(parent, node);
        Node* left = index > 0 ? parent->child[index - 1] : nullptr;
        Node* right = index + 1 < parent->child.size() ? parent->child[index + 1] : nullptr;

        if (node->leaf) {
            if (left && left->key.size() > minLeafKeys()) {
                borrowFromLeftLeaf(node, left, index);
                return;
            }
            if (right && right->key.size() > minLeafKeys()) {
                borrowFromRightLeaf(node, right, index);
                updateAncestorMinimum(node);
                return;
            }
            if (left)
                mergeLeaves(left, node, index - 1);
            else if (right)
                mergeLeaves(node, right, index);
            return;
        }

        if (left && left->key.size() > minInternalKeys()) {
            borrowFromLeftInternal(node, left, index);
            return;
        }
        if (right && right->key.size() > minInternalKeys()) {
            borrowFromRightInternal(node, right, index);
            return;
        }
        if (left)
            mergeInternal(left, node, index - 1);
        else if (right)
            mergeInternal(node, right, index);
    }

    void clear(Node* node) {
        if (!node)
            return;
        if (!node->leaf) {
            while (!node->child.empty()) {
                Node* child = node->child.remove(node->child.size() - 1);
                clear(child);
            }
        }
        delete node;
    }

    const Key* refreshSeparators(Node* node) {
        if (!node)
            return nullptr;
        if (node->leaf)
            return node->key.empty() ? nullptr : &node->key[0];

        const Key* minimum = nullptr;
        for (int index = 0; index < node->child.size(); ++index) {
            const Key* child_minimum = refreshSeparators(node->child[index]);
            if (index == 0)
                minimum = child_minimum;
            else if (child_minimum)
                node->key[index - 1] = *child_minimum;
        }
        return minimum;
    }

    void copyFrom(const BPlusTree& other) {
        for (Node* leaf = other.firstLeaf(); leaf; leaf = leaf->next) {
            for (int index = 0; index < leaf->key.size(); ++index)
                insert(leaf->key[index], leaf->value[index]);
        }
    }

public:
    explicit BPlusTree(int order = 4, Compare compare = Compare())
        : _order(order < 3 ? 3 : order), _cmp(compare) {}

    BPlusTree(const BPlusTree& other)
        : _order(other._order), _cmp(other._cmp) {
        copyFrom(other);
    }

    BPlusTree(BPlusTree&& other) noexcept
        : _root(other._root), _order(other._order), _size(other._size), _cmp(std::move(other._cmp)) {
        other._root = nullptr;
        other._size = 0;
    }

    BPlusTree& operator=(BPlusTree other) {
        swap(other);
        return *this;
    }

    ~BPlusTree() { clear(_root); }

    void swap(BPlusTree& other) {
        using std::swap;
        swap(_root, other._root);
        swap(_order, other._order);
        swap(_size, other._size);
        swap(_cmp, other._cmp);
    }

    int size() const { return _size; }
    bool empty() const { return _size == 0; }

    const Value* search(const Key& key) const {
        Node* leaf = findLeaf(key);
        if (!leaf)
            return nullptr;
        const int index = lowerBound(leaf->key, key);
        if (index < leaf->key.size() && equal(leaf->key[index], key))
            return &leaf->value[index];
        return nullptr;
    }

    bool insert(const Key& key, const Value& value) {
        if (!_root) {
            _root = new Node(true);
            _root->key.insert(0, key);
            _root->value.insert(0, value);
            _size = 1;
            refreshSeparators(_root);
            return true;
        }

        Node* leaf = findLeaf(key);
        const int index = lowerBound(leaf->key, key);
        if (index < leaf->key.size() && equal(leaf->key[index], key))
            return false;

        leaf->key.insert(index, key);
        leaf->value.insert(index, value);
        ++_size;
        if (index == 0)
            updateAncestorMinimum(leaf);
        if (leaf->key.size() > maxKeys()) {
            Node* right = splitLeaf(leaf);
            insertIntoParent(leaf, right->key[0], right);
        }
        refreshSeparators(_root);
        return true;
    }

    bool remove(const Key& key) {
        Node* leaf = findLeaf(key);
        if (!leaf)
            return false;
        const int index = lowerBound(leaf->key, key);
        if (index >= leaf->key.size() || !equal(leaf->key[index], key))
            return false;

        leaf->key.remove(index);
        leaf->value.remove(index);
        --_size;

        if (leaf == _root) {
            if (leaf->key.empty()) {
                delete leaf;
                _root = nullptr;
            }
            refreshSeparators(_root);
            return true;
        }

        if (index == 0 && !leaf->key.empty())
            updateAncestorMinimum(leaf);
        if (leaf->key.size() < minLeafKeys())
            handleUnderflow(leaf);
        refreshSeparators(_root);
        return true;
    }

    template<typename Result, typename Aggregate>
    Result rangeAggregate(
        const Key& low,
        const Key& high,
        Result identity,
        Aggregate&& aggregate
    ) const {
        Node* leaf = findLeaf(low);
        if (!leaf)
            return identity;
        int index = lowerBound(leaf->key, low);
        Result result = identity;
        for (Node* current = leaf; current; current = current->next) {
            for (; index < current->key.size(); ++index) {
                if (_cmp(high, current->key[index]))
                    return result;
                result = aggregate(result, current->value[index]);
            }
            index = 0;
        }
        return result;
    }

    Vector<Value> rangeQuery(const Key& low, const Key& high) const {
        Vector<Value> result;
        Node* leaf = findLeaf(low);
        if (!leaf)
            return result;
        int index = lowerBound(leaf->key, low);
        for (Node* current = leaf; current; current = current->next) {
            for (; index < current->key.size(); ++index) {
                if (_cmp(high, current->key[index]))
                    return result;
                result.insert(result.size(), current->value[index]);
            }
            index = 0;
        }
        return result;
    }
};

#endif
