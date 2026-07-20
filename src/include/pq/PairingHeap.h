#ifndef __DSA_PAIRING_HEAP
#define __DSA_PAIRING_HEAP

#include <stdexcept>
#include <utility>

#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"
#include <dsa/core/heap/HeapAlgorithm.h>

// 教学配对堆：BinNode::lc 表示最左孩子，rc 表示右兄弟；因此自行管理销毁。
template<typename T, bool MAX = true>
class PairingHeap : public PQ<T, MAX>, public BinTree<T> {
    friend class UniPrint;

private:
    struct TeachingAccess {
        typedef BinNode<T> node_type;
        typedef std::size_t size_type;

        static node_type*& parent(node_type* node) { return node->parent; }
        static node_type*& child(node_type* node) { return node->lc; }
        static node_type*& sibling(node_type* node) { return node->rc; }
        static T& value(node_type* node) { return node->data; }
        static const T& value(const node_type* node) { return node->data; }
        static std::size_t degreeValue(const node_type*) { return 0; }
        static void incrementDegree(node_type*) {}
        static void resetDegree(node_type*) {}
        static void setMarked(node_type*, bool) {}
    };

    typedef Priority<T, MAX> Higher;
    typedef dsa::core::PairingHeapAlgorithm<TeachingAccess, Higher> Algorithm;

    BinNode<T>* merge(BinNode<T>* first, BinNode<T>* second);
    BinNode<T>* mergePairs(BinNode<T>* firstSibling);
    void clearOwned() noexcept;
    void copyValuesFrom(const PairingHeap& other);
    void swapState(PairingHeap& other) noexcept;

public:
    PairingHeap() : BinTree<T>() {}
    PairingHeap(T* values, int count) : BinTree<T>() {
        for (int index = 0; index < count; ++index)
            insert(values[index]);
    }
    explicit PairingHeap(Vector<T>& vector) : BinTree<T>() {
        for (int index = 0; index < vector.size(); ++index)
            insert(vector[index]);
    }

    // 深拷贝时重新插入值，避免复制 child/sibling 裸指针。
    PairingHeap(const PairingHeap& other) : BinTree<T>() {
        try { copyValuesFrom(other); }
        catch (...) { clearOwned(); throw; }
    }

    PairingHeap(PairingHeap&& other) noexcept : BinTree<T>() {
        this->_root = other._root; this->_size = other._size;
        other._root = nullptr; other._size = 0;
    }

    ~PairingHeap() { clearOwned(); }

    PairingHeap& operator=(const PairingHeap& other) {
        if (this != &other) { PairingHeap replacement(other); swapState(replacement); }
        return *this;
    }

    PairingHeap& operator=(PairingHeap&& other) noexcept {
        if (this != &other) {
            clearOwned(); this->_root = other._root; this->_size = other._size;
            other._root = nullptr; other._size = 0;
        }
        return *this;
    }

    void merge(PairingHeap& other);
    void insert(T value);
    T getMax();
    T delMax();
};

template<typename T, bool MAX>
BinNode<T>* PairingHeap<T, MAX>::merge(BinNode<T>* first, BinNode<T>* second) {
    return Algorithm::merge(first, second, Higher());
}

template<typename T, bool MAX>
BinNode<T>* PairingHeap<T, MAX>::mergePairs(BinNode<T>* firstSibling) {
    return Algorithm::mergePairs(firstSibling, Higher());
}

template<typename T, bool MAX>
void PairingHeap<T, MAX>::clearOwned() noexcept {
    dsa::core::destroyChildSiblingHeapForest<TeachingAccess>(
        this->_root,
        [](BinNode<T>* node) {
            release(node->data);
            delete node;
        }
    );
    this->_root = nullptr; this->_size = 0;
}

template<typename T, bool MAX>
void PairingHeap<T, MAX>::copyValuesFrom(const PairingHeap& other) {
    dsa::core::forEachChildSiblingHeapNode<TeachingAccess>(
        const_cast<BinNode<T>*>(other._root),
        [this](BinNode<T>* node) { insert(node->data); }
    );
}

template<typename T, bool MAX>
void PairingHeap<T, MAX>::swapState(PairingHeap& other) noexcept {
    using std::swap;
    swap(this->_root, other._root); swap(this->_size, other._size);
}

template<typename T, bool MAX>
void PairingHeap<T, MAX>::insert(T value) {
    BinNode<T>* node = new BinNode<T>(value, nullptr);
    try { this->_root = merge(this->_root, node); ++this->_size; }
    catch (...) { delete node; throw; }
}

template<typename T, bool MAX>
DSA_NOINLINE T PairingHeap<T, MAX>::getMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");
    return this->_root->data;
}

template<typename T, bool MAX>
T PairingHeap<T, MAX>::delMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");

    BinNode<T>* removed = this->_root;
    T result = removed->data;
    BinNode<T>* merged = mergePairs(removed->lc);
    removed->lc = nullptr;
    removed->rc = nullptr;
    this->_root = merged;
    delete removed;
    --this->_size;
    return result;
}

template<typename T, bool MAX>
void PairingHeap<T, MAX>::merge(PairingHeap& other) {
    if (this == &other || !other._root)
        return;
    this->_root = merge(this->_root, other._root);
    this->_size += other._size;
    other._root = nullptr; other._size = 0;
}

#endif
