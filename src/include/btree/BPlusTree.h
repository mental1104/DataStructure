#ifndef __DSA_BPLUSTREE
#define __DSA_BPLUSTREE

#include <functional>
#include <utility>
#include "BST.h"
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
    explicit BPTNode(bool isLeaf):leaf(isLeaf){}
};

template<typename Key, typename Value, typename Compare = std::less<Key>>
class BPlusTree: public BST<Key> {
private:
    using Node = BPTNode<Key, Value>;
    Node* _root{nullptr};
    int _order;
    int _size{0};
    Compare _cmp;

    int maxKeys() const { return _order - 1; }
    int minLeafKeys() const { return _order / 2; }
    int minInternalKeys() const { return (_order + 1) / 2 - 1; }

    bool equal(const Key& ls, const Key& rs) const { return !_cmp(ls, rs) && !_cmp(rs, ls); }

    int lowerBound(const Vector<Key>& keys, const Key& k) const {
        int lo = 0, hi = keys.size();
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (_cmp(keys[mid], k)) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }

    int upperBound(const Vector<Key>& keys, const Key& k) const {
        int lo = 0, hi = keys.size();
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (_cmp(k, keys[mid])) hi = mid;
            else lo = mid + 1;
        }
        return lo;
    }

    Node* findLeaf(const Key& k) const {
        Node* x = _root;
        while (x && !x->leaf) {
            int idx = upperBound(x->key, k);
            x = x->child[idx];
        }
        return x;
    }

    int childIndex(Node* parent, Node* child) const {
        for (int i = 0; i < parent->child.size(); i++)
            if (parent->child[i] == child)
                return i;
        return -1;
    }

    Node* splitLeaf(Node* leaf) {
        int mid = leaf->key.size() / 2;
        Node* neo = new Node(true);
        neo->parent = leaf->parent;
        while (leaf->key.size() > mid) {
            neo->key.insert(neo->key.size(), leaf->key.remove(mid));
            neo->value.insert(neo->value.size(), leaf->value.remove(mid));
        }
        neo->next = leaf->next;
        if (neo->next) neo->next->prev = neo;
        leaf->next = neo;
        neo->prev = leaf;
        return neo;
    }

    std::pair<Node*, Key> splitInternal(Node* node) {
        int mid = node->key.size() / 2;
        Key up = node->key[mid];
        Node* neo = new Node(false);
        neo->parent = node->parent;

        // move keys
        for (int i = mid + 1; i < node->key.size();) {
            neo->key.insert(neo->key.size(), node->key.remove(mid + 1));
        }
        // move children
        for (int i = mid + 1; i < node->child.size();) {
            Node* c = node->child.remove(mid + 1);
            neo->child.insert(neo->child.size(), c);
            if (c) c->parent = neo;
        }
        node->key.remove(mid);
        return {neo, up};
    }

    void insertIntoParent(Node* left, const Key& k, Node* right) {
        if (!left->parent) {
            Node* root = new Node(false);
            root->key.insert(0, k);
            root->child.insert(0, left);
            root->child.insert(1, right);
            left->parent = right->parent = root;
            _root = root;
            return;
        }
        Node* parent = left->parent;
        int idx = childIndex(parent, left);
        parent->key.insert(idx, k);
        parent->child.insert(idx + 1, right);
        right->parent = parent;
        if (parent->key.size() > maxKeys()) {
            auto res = splitInternal(parent);
            insertIntoParent(parent, res.second, res.first);
        }
    }

    void borrowFromLeftLeaf(Node* node, Node* left, int idxInParent) {
        node->key.insert(0, left->key.remove(left->key.size() - 1));
        node->value.insert(0, left->value.remove(left->value.size() - 1));
        node->parent->key[idxInParent - 1] = node->key[0];
    }

    void borrowFromRightLeaf(Node* node, Node* right, int idxInParent) {
        node->key.insert(node->key.size(), right->key.remove(0));
        node->value.insert(node->value.size(), right->value.remove(0));
        node->parent->key[idxInParent] = right->key[0];
    }

    void borrowFromLeftInternal(Node* node, Node* left, int idxInParent) {
        node->key.insert(0, node->parent->key[idxInParent - 1]);
        Node* child = left->child.remove(left->child.size() - 1);
        node->child.insert(0, child);
        if (child) child->parent = node;
        node->parent->key[idxInParent - 1] = left->key.remove(left->key.size() - 1);
    }

    void borrowFromRightInternal(Node* node, Node* right, int idxInParent) {
        node->key.insert(node->key.size(), node->parent->key[idxInParent]);
        Node* child = right->child.remove(0);
        node->child.insert(node->child.size(), child);
        if (child) child->parent = node;
        node->parent->key[idxInParent] = right->key.remove(0);
    }

    void mergeLeaves(Node* left, Node* right, int idxInParent) {
        for (int i = 0; i < right->key.size(); i++) {
            left->key.insert(left->key.size(), right->key[i]);
            left->value.insert(left->value.size(), right->value[i]);
        }
        left->next = right->next;
        if (right->next) right->next->prev = left;
        Node* parent = left->parent;
        parent->key.remove(idxInParent);
        parent->child.remove(idxInParent + 1);
        delete right;
        if (parent == _root && parent->key.size() == 0) {
            _root = left;
            left->parent = nullptr;
            delete parent;
        } else if (parent != _root && parent->key.size() < minInternalKeys()) {
            handleUnderflow(parent);
        }
    }

    void mergeInternal(Node* left, Node* right, int idxInParent) {
        left->key.insert(left->key.size(), left->parent->key.remove(idxInParent));
        for (int i = 0; i < right->key.size(); i++)
            left->key.insert(left->key.size(), right->key[i]);
        for (int i = 0; i < right->child.size(); i++) {
            Node* c = right->child[i];
            left->child.insert(left->child.size(), c);
            if (c) c->parent = left;
        }
        left->parent->child.remove(idxInParent + 1);
        delete right;
        Node* parent = left->parent;
        if (parent == _root && parent->key.size() == 0) {
            _root = left;
            left->parent = nullptr;
            delete parent;
        } else if (parent != _root && parent->key.size() < minInternalKeys()) {
            handleUnderflow(parent);
        }
    }

    void handleUnderflow(Node* node) {
        Node* parent = node->parent;
        if (!parent) return;
        int idx = childIndex(parent, node);
        Node* left = (idx > 0) ? parent->child[idx - 1] : nullptr;
        Node* right = (idx + 1 < parent->child.size()) ? parent->child[idx + 1] : nullptr;

        if (node->leaf) {
            if (left && left->key.size() > minLeafKeys()) {
                borrowFromLeftLeaf(node, left, idx);
                return;
            }
            if (right && right->key.size() > minLeafKeys()) {
                borrowFromRightLeaf(node, right, idx);
                return;
            }
            if (left) mergeLeaves(left, node, idx - 1);
            else if (right) mergeLeaves(node, right, idx);
        } else {
            if (left && left->key.size() > minInternalKeys()) {
                borrowFromLeftInternal(node, left, idx);
                return;
            }
            if (right && right->key.size() > minInternalKeys()) {
                borrowFromRightInternal(node, right, idx);
                return;
            }
            if (left) mergeInternal(left, node, idx - 1);
            else if (right) mergeInternal(node, right, idx);
        }
    }

    void clear(Node* node) {
        if (!node) return;
        if (!node->leaf) {
            for (int i = 0; i < node->child.size(); i++)
                clear(node->child[i]);
        }
        delete node;
    }

public:
    explicit BPlusTree(int order = 4, Compare cmp = Compare()):_order(order < 3 ? 3 : order), _cmp(cmp){ }
    ~BPlusTree(){
        clear(_root);
        this->_root = nullptr;
        this->_size = 0;
    }

    int size() const { return _size; }
    bool empty() const { return _size == 0; }

    const Value* search(const Key& k) const {
        Node* leaf = findLeaf(k);
        if (!leaf) return nullptr;
        int idx = lowerBound(leaf->key, k);
        if (idx < leaf->key.size() && equal(leaf->key[idx], k))
            return &(leaf->value[idx]);
        return nullptr;
    }

    bool insert(const Key& k, const Value& v) {
        if (!_root) {
            _root = new Node(true);
            _root->key.insert(0, k);
            _root->value.insert(0, v);
            _size = 1;
            return true;
        }
        Node* leaf = findLeaf(k);
        int idx = lowerBound(leaf->key, k);
        if (idx < leaf->key.size() && equal(leaf->key[idx], k))
            return false;
        leaf->key.insert(idx, k);
        leaf->value.insert(idx, v);
        _size++;
        if (leaf->key.size() > maxKeys()) {
            Node* neo = splitLeaf(leaf);
            insertIntoParent(leaf, neo->key[0], neo);
        }
        return true;
    }

    bool remove(const Key& k) override {
        if (!_root) return false;
        Node* leaf = findLeaf(k);
        if (!leaf) return false;
        int idx = lowerBound(leaf->key, k);
        if (!(idx < leaf->key.size() && equal(leaf->key[idx], k)))
            return false;
        leaf->key.remove(idx);
        leaf->value.remove(idx);
        _size--;

        if (leaf == _root) {
            if (leaf->key.size() == 0) {
                delete leaf;
                _root = nullptr;
            }
            return true;
        }
        if (leaf->key.size() < minLeafKeys())
            handleUnderflow(leaf);
        return true;
    }

    Vector<Value> rangeQuery(const Key& low, const Key& high) const {
        Vector<Value> res;
        if (!_root) return res;
        Node* leaf = findLeaf(low);
        if (!leaf) return res;
        int idx = lowerBound(leaf->key, low);
        for (Node* cur = leaf; cur; cur = cur->next) {
            for (int i = idx; i < cur->key.size(); i++) {
                if (_cmp(high, cur->key[i])) return res;
                res.insert(res.size(), cur->value[i]);
            }
            idx = 0;
        }
        return res;
    }
};

#endif
