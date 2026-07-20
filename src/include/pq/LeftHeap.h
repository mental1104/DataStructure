#ifndef __DSA_LEFT_HEAP
#define __DSA_LEFT_HEAP

#include <stdexcept>
#include <utility>

#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"
#include <dsa/core/heap/HeapAlgorithm.h>

// 教学左式堆：保留 BinNode/BinTree 可视结构，meld 流程由共享算法统一实现。
template<typename T, bool MAX = true>
class LeftHeap : public PQ<T, MAX>, public BinTree<T> {
    friend class UniPrint;

private:
    struct TeachingAccess {
        typedef BinNode<T> node_type;

        static node_type*& parent(node_type* node) { return node->parent; }
        static node_type*& left(node_type* node) { return node->lc; }
        static node_type*& right(node_type* node) { return node->rc; }
        static T& value(node_type* node) { return node->data; }
        static const T& value(const node_type* node) { return node->data; }
        static int nplValue(const node_type* node) { return node ? node->npl : 0; }
        static int& nplRef(node_type* node) { return node->npl; }
    };

    typedef Priority<T, MAX> Higher;
    typedef dsa::core::LeftistHeapAlgorithm<TeachingAccess, Higher> Algorithm;

    BinNode<T>* merge(BinNode<T>* first, BinNode<T>* second);
    void clearOwned() noexcept;
    void copyValuesFrom(const LeftHeap& other);
    void swapState(LeftHeap& other) noexcept;

public:
    // 构造空左式堆。
    LeftHeap() : BinTree<T>() {}

    // 从数组逐项插入构造。
    LeftHeap(T* values, int count) : BinTree<T>() {
        for (int index = 0; index < count; ++index)
            insert(values[index]);
    }

    // 从教学 Vector 逐项插入构造。
    explicit LeftHeap(Vector<T>& vector) : BinTree<T>() {
        for (int index = 0; index < vector.size(); ++index)
            insert(vector[index]);
    }

    // 深拷贝另一个教学左式堆，避免继承层默认浅拷贝根指针。
    LeftHeap(const LeftHeap& other) : BinTree<T>() {
        copyValuesFrom(other);
    }

    // 移动构造并转移节点所有权。
    LeftHeap(LeftHeap&& other) noexcept : BinTree<T>() {
        this->_root = other._root;
        this->_size = other._size;
        other._root = nullptr;
        other._size = 0;
    }

    // 使用 copy-and-swap 提供深拷贝赋值。
    LeftHeap& operator=(const LeftHeap& other) {
        if (this != &other) {
            LeftHeap replacement(other);
            swapState(replacement);
        }
        return *this;
    }

    // 释放原节点后接管来源节点。
    LeftHeap& operator=(LeftHeap&& other) noexcept {
        if (this != &other) {
            clearOwned();
            this->_root = other._root;
            this->_size = other._size;
            other._root = nullptr;
            other._size = 0;
        }
        return *this;
    }

    // 破坏性合并另一个左式堆，成功后来源为空。
    void merge(LeftHeap& other);

    // 插入一个元素。
    void insert(T value);

    // 返回优先级最高元素；空堆抛出 runtime_error。
    T getMax();

    // 删除并返回优先级最高元素；空堆抛出 runtime_error。
    T delMax();
};

template<typename T, bool MAX>
BinNode<T>* LeftHeap<T, MAX>::merge(BinNode<T>* first, BinNode<T>* second) {
    return Algorithm::merge(first, second, Higher());
}

template<typename T, bool MAX>
void LeftHeap<T, MAX>::clearOwned() noexcept {
    if (this->_root)
        removeAt(this->_root);
    this->_root = nullptr;
    this->_size = 0;
}

template<typename T, bool MAX>
void LeftHeap<T, MAX>::copyValuesFrom(const LeftHeap& other) {
    dsa::core::forEachBinaryHeapNode<TeachingAccess>(
        const_cast<BinNode<T>*>(other._root),
        [this](BinNode<T>* node) { insert(node->data); }
    );
}

template<typename T, bool MAX>
void LeftHeap<T, MAX>::swapState(LeftHeap& other) noexcept {
    using std::swap;
    swap(this->_root, other._root);
    swap(this->_size, other._size);
}

template<typename T, bool MAX>
void LeftHeap<T, MAX>::insert(T value) {
    BinNode<T>* node = new BinNode<T>(value, nullptr);
    try {
        this->_root = merge(this->_root, node);
        ++this->_size;
    } catch (...) {
        delete node;
        throw;
    }
}

template<typename T, bool MAX>
DSA_NOINLINE T LeftHeap<T, MAX>::getMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");
    return this->_root->data;
}

template<typename T, bool MAX>
T LeftHeap<T, MAX>::delMax() {
    if (!this->_root)
        throw std::runtime_error("Heap is empty");

    BinNode<T>* removed = this->_root;
    T result = removed->data;
    BinNode<T>* merged = merge(removed->lc, removed->rc);
    removed->lc = nullptr;
    removed->rc = nullptr;
    this->_root = merged;
    delete removed;
    --this->_size;
    return result;
}

template<typename T, bool MAX>
void LeftHeap<T, MAX>::merge(LeftHeap& other) {
    if (this == &other || !other._root)
        return;
    BinNode<T>* merged = merge(this->_root, other._root);
    this->_root = merged;
    this->_size += other._size;
    other._root = nullptr;
    other._size = 0;
}

#endif
