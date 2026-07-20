#ifndef __DSA_SKEW_HEAP
#define __DSA_SKEW_HEAP

#include <stdexcept>
#include <utility>

#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"
#include <dsa/core/heap/HeapAlgorithm.h>

// 教学斜堆：保留 BinTree 外观，使用共享的非递归右脊 meld 算法。
template<typename T, bool MAX = true>
class SkewHeap : public PQ<T, MAX>, public BinTree<T> {
    friend class UniPrint;

private:
    struct TeachingAccess {
        typedef BinNode<T> node_type;

        static node_type*& parent(node_type* node) { return node->parent; }
        static node_type*& left(node_type* node) { return node->lc; }
        static node_type*& right(node_type* node) { return node->rc; }
        static T& value(node_type* node) { return node->data; }
        static const T& value(const node_type* node) { return node->data; }
    };

    typedef Priority<T, MAX> Higher;
    typedef dsa::core::SkewHeapAlgorithm<TeachingAccess, Higher> Algorithm;

    BinNode<T>* merge(BinNode<T>* first, BinNode<T>* second);
    void clearOwned() noexcept;
    void copyValuesFrom(const SkewHeap& other);
    void swapState(SkewHeap& other) noexcept;

public:
    SkewHeap() : BinTree<T>() {}
    SkewHeap(T* values, int count) : BinTree<T>() {
        for (int index = 0; index < count; ++index)
            insert(values[index]);
    }
    explicit SkewHeap(Vector<T>& vector) : BinTree<T>() {
        for (int index = 0; index < vector.size(); ++index)
            insert(vector[index]);
    }
    SkewHeap(const SkewHeap& other) : BinTree<T>() { copyValuesFrom(other); }
    SkewHeap(SkewHeap&& other) noexcept : BinTree<T>() {
        this->_root = other._root; this->_size = other._size;
        other._root = nullptr; other._size = 0;
    }
    SkewHeap& operator=(const SkewHeap& other) {
        if (this != &other) { SkewHeap replacement(other); swapState(replacement); }
        return *this;
    }
    SkewHeap& operator=(SkewHeap&& other) noexcept {
        if (this != &other) {
            clearOwned(); this->_root = other._root; this->_size = other._size;
            other._root = nullptr; other._size = 0;
        }
        return *this;
    }

    void merge(SkewHeap& other);
    void insert(T value);
    T getMax();
    T delMax();
};

template<typename T, bool MAX>
BinNode<T>* SkewHeap<T, MAX>::merge(BinNode<T>* first, BinNode<T>* second) {
    return Algorithm::merge(first, second, Higher());
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::clearOwned() noexcept {
    if (this->_root)
        removeAt(this->_root);
    this->_root = nullptr; this->_size = 0;
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::copyValuesFrom(const SkewHeap& other) {
    dsa::core::forEachBinaryHeapNode<TeachingAccess>(
        const_cast<BinNode<T>*>(other._root),
        [this](BinNode<T>* node) { insert(node->data); }
    );
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::swapState(SkewHeap& other) noexcept {
    using std::swap;
    swap(this->_root, other._root); swap(this->_size, other._size);
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::insert(T value) {
    BinNode<T>* node = new BinNode<T>(value, nullptr);
    try { this->_root = merge(this->_root, node); ++this->_size; }
    catch (...) { delete node; throw; }
}

template<typename T, bool MAX>
DSA_NOINLINE T SkewHeap<T, MAX>::getMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");
    return this->_root->data;
}

template<typename T, bool MAX>
T SkewHeap<T, MAX>::delMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");
    BinNode<T>* removed = this->_root;
    T result = removed->data;
    BinNode<T>* merged = merge(removed->lc, removed->rc);
    removed->lc = nullptr; removed->rc = nullptr;
    this->_root = merged;
    delete removed; --this->_size;
    return result;
}

template<typename T, bool MAX>
void SkewHeap<T, MAX>::merge(SkewHeap& other) {
    if (this == &other || !other._root)
        return;
    this->_root = merge(this->_root, other._root);
    this->_size += other._size;
    other._root = nullptr; other._size = 0;
}

#endif
