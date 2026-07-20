#ifndef __DSA_FIBONACCI_HEAP
#define __DSA_FIBONACCI_HEAP

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "BinTree.h"
#include "PQ.h"
#include "Vector.h"
#include <dsa/core/heap/HeapAlgorithm.h>

// 教学斐波那契堆：根表和孩子表均使用单向右兄弟链；当前仅提供 push/top/pop/meld 核心子集。
template<typename T, bool MAX = true>
class FibonacciHeap : public PQ<T, MAX>, public BinTree<T> {
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
        static std::size_t degreeValue(const node_type* node) {
            return static_cast<std::size_t>(node->height < 0 ? 0 : node->height);
        }
        static void incrementDegree(node_type* node) { ++node->height; }
        static void resetDegree(node_type* node) { node->height = 0; }
        static void setMarked(node_type* node, bool marked) {
            node->color = marked ? RBColor::BLACK : RBColor::RED;
        }
    };

    typedef Priority<T, MAX> Higher;
    typedef dsa::core::FibonacciHeapAlgorithm<TeachingAccess, Higher> Algorithm;

    BinNode<T>* _bestRoot;
    BinNode<T>* _rootTail;

    BinNode<T>* mergeTrees(BinNode<T>* first, BinNode<T>* second);
    void consolidate();
    void clearOwned() noexcept;
    void copyValuesFrom(const FibonacciHeap& other);
    void swapState(FibonacciHeap& other) noexcept;
    BinNode<T>* findRootTail();

public:
    FibonacciHeap() : BinTree<T>(), _bestRoot(nullptr), _rootTail(nullptr) {}
    FibonacciHeap(T* values, int count)
        : BinTree<T>(), _bestRoot(nullptr), _rootTail(nullptr) {
        for (int index = 0; index < count; ++index)
            insert(values[index]);
    }
    explicit FibonacciHeap(Vector<T>& vector)
        : BinTree<T>(), _bestRoot(nullptr), _rootTail(nullptr) {
        for (int index = 0; index < vector.size(); ++index)
            insert(vector[index]);
    }

    FibonacciHeap(const FibonacciHeap& other)
        : BinTree<T>(), _bestRoot(nullptr), _rootTail(nullptr) {
        try {
            copyValuesFrom(other);
        } catch (...) {
            clearOwned();
            throw;
        }
    }

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : BinTree<T>(), _bestRoot(other._bestRoot), _rootTail(other._rootTail) {
        this->_root = other._root;
        this->_size = other._size;
        other._root = nullptr;
        other._size = 0;
        other._bestRoot = nullptr;
        other._rootTail = nullptr;
    }

    ~FibonacciHeap() { clearOwned(); }

    FibonacciHeap& operator=(const FibonacciHeap& other) {
        if (this != &other) {
            FibonacciHeap replacement(other);
            swapState(replacement);
        }
        return *this;
    }

    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
        if (this != &other) {
            clearOwned();
            this->_root = other._root;
            this->_size = other._size;
            _bestRoot = other._bestRoot;
            _rootTail = other._rootTail;
            other._root = nullptr;
            other._size = 0;
            other._bestRoot = nullptr;
            other._rootTail = nullptr;
        }
        return *this;
    }

    void merge(FibonacciHeap& other);
    void insert(T value);
    T getMax();
    T delMax();
};

template<typename T, bool MAX>
BinNode<T>* FibonacciHeap<T, MAX>::mergeTrees(BinNode<T>* first, BinNode<T>* second) {
    return Algorithm::linkTrees(first, second, Higher());
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::consolidate() {
    Algorithm::consolidate(this->_root, _rootTail, _bestRoot, Higher());
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::clearOwned() noexcept {
    dsa::core::destroyChildSiblingHeapForest<TeachingAccess>(
        this->_root,
        [](BinNode<T>* node) {
            release(node->data);
            delete node;
        }
    );
    this->_root = nullptr;
    this->_size = 0;
    _bestRoot = nullptr;
    _rootTail = nullptr;
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::copyValuesFrom(const FibonacciHeap& other) {
    dsa::core::forEachChildSiblingHeapNode<TeachingAccess>(
        const_cast<BinNode<T>*>(other._root),
        [this](BinNode<T>* node) { insert(node->data); }
    );
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::swapState(FibonacciHeap& other) noexcept {
    std::swap(this->_root, other._root);
    std::swap(this->_size, other._size);
    std::swap(_bestRoot, other._bestRoot);
    std::swap(_rootTail, other._rootTail);
}

template<typename T, bool MAX>
BinNode<T>* FibonacciHeap<T, MAX>::findRootTail() {
    BinNode<T>* tail = this->_root;
    while (tail && tail->rc)
        tail = tail->rc;
    return tail;
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::insert(T value) {
    BinNode<T>* node = new BinNode<T>(value, nullptr);
    node->height = 0;
    node->color = RBColor::RED;

    bool becomesBest = false;
    try {
        becomesBest = !_bestRoot || Higher::higher(node->data, _bestRoot->data);
    } catch (...) {
        delete node;
        throw;
    }

    if (!this->_root)
        this->_root = node;
    else {
        if (!_rootTail)
            _rootTail = findRootTail();
        _rootTail->rc = node;
    }
    _rootTail = node;
    if (becomesBest)
        _bestRoot = node;
    ++this->_size;
}

template<typename T, bool MAX>
DSA_NOINLINE T FibonacciHeap<T, MAX>::getMax() {
    if (!_bestRoot)
        throw std::runtime_error("Heap is empty");
    return _bestRoot->data;
}

template<typename T, bool MAX>
T FibonacciHeap<T, MAX>::delMax() {
    if (!_bestRoot)
        throw std::runtime_error("Heap is empty");

    T result = _bestRoot->data;
    BinNode<T>* removed = Algorithm::removeBest(
        this->_root,
        _rootTail,
        _bestRoot,
        static_cast<std::size_t>(this->_size),
        Higher()
    );
    --this->_size;
    delete removed;
    return result;
}

template<typename T, bool MAX>
void FibonacciHeap<T, MAX>::merge(FibonacciHeap& other) {
    if (this == &other || !other._root)
        return;

    bool otherWins = false;
    if (!_bestRoot)
        otherWins = true;
    else if (other._bestRoot)
        otherWins = Higher::higher(other._bestRoot->data, _bestRoot->data);

    if (!this->_root) {
        this->_root = other._root;
        _rootTail = other._rootTail ? other._rootTail : other.findRootTail();
    } else {
        if (!_rootTail)
            _rootTail = findRootTail();
        BinNode<T>* otherTail = other._rootTail ? other._rootTail : other.findRootTail();
        _rootTail->rc = other._root;
        _rootTail = otherTail;
    }
    if (otherWins)
        _bestRoot = other._bestRoot;

    this->_size += other._size;
    other._root = nullptr;
    other._size = 0;
    other._bestRoot = nullptr;
    other._rootTail = nullptr;
}

#endif
