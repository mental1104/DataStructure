#ifndef __DSA_HEAP
#define __DSA_HEAP

#include <stdexcept>
#include <utility>

#include "PQ.h"
#include "Vector.h"
#include <dsa/core/heap/HeapAlgorithm.h>

// 教学完全二叉堆：继续继承 Vector 暴露原有遍历能力，堆序调整复用共享算法。
template<typename T, bool MAX = true>
class Heap : public PQ<T, MAX>, public Vector<T> {
    friend class UniPrint;

private:
    class TeachingAccess {
    public:
        typedef Rank size_type;

        explicit TeachingAccess(Heap& heap) : heap_(heap) {}
        T& value(size_type index) { return heap_._elem[index]; }
        const T& value(size_type index) const { return heap_._elem[index]; }
        void swapAt(size_type first, size_type second) {
            using std::swap;
            swap(heap_._elem[first], heap_._elem[second]);
        }

    private:
        Heap& heap_;
    };

    typedef Priority<T, MAX> Higher;
    typedef dsa::core::BinaryHeapAlgorithm<TeachingAccess, Higher> Algorithm;

    // 兼容旧 helper：返回父节点下标。
    Rank Parent(Rank index) { return Algorithm::parent(index); }

    // 兼容旧 helper：返回左孩子下标。
    Rank LChild(Rank index) { return Algorithm::leftChild(index); }

    // 兼容旧 helper：返回右孩子下标。
    Rank RChild(Rank index) { return Algorithm::rightChild(index); }

    // 兼容旧 helper：判断下标是否位于当前堆区间。
    bool InHeap(Rank count, Rank index) { return 0 <= index && index < count; }

    // 兼容旧 helper：判断左孩子是否存在。
    bool LChildValid(Rank count, Rank index) { return InHeap(count, LChild(index)); }

    // 兼容旧 helper：判断右孩子是否存在。
    bool RChildValid(Rank count, Rank index) { return InHeap(count, RChild(index)); }

    Rank Bigger(Rank first, Rank second);
    Rank ProperParent(Rank count, Rank index);
    void heapify(Rank count);
    Rank percolateDown(Rank count, Rank index);
    Rank percolateUp(Rank index);

public:
    // 构造空教学堆。
    Heap() : Vector<T>() {}

    // 从教学 Vector 深拷贝并执行 Floyd 建堆。
    explicit Heap(Vector<T>& vector) : Vector<T>(vector) {
        heapify(this->_size);
    }

    // 从数组区间构造并执行 Floyd 建堆。
    Heap(T* values, Rank count) : Vector<T>(values, count) {
        heapify(this->_size);
    }

    // 按优先级插入元素。
    void insert(T value);

    // 返回优先级最高元素；空堆抛出 runtime_error。
    T getMax();

    // 删除并返回优先级最高元素；空堆抛出 runtime_error。
    T delMax();
};

template<typename T, bool MAX>
Rank Heap<T, MAX>::Bigger(Rank first, Rank second) {
    return Higher::higher(this->_elem[second], this->_elem[first]) ? second : first;
}

template<typename T, bool MAX>
Rank Heap<T, MAX>::ProperParent(Rank count, Rank index) {
    if (RChildValid(count, index))
        return Bigger(Bigger(index, LChild(index)), RChild(index));
    if (LChildValid(count, index))
        return Bigger(index, LChild(index));
    return index;
}

template<typename T, bool MAX>
void Heap<T, MAX>::heapify(Rank count) {
    TeachingAccess access(*this);
    Algorithm::heapify(access, count, Higher());
}

template<typename T, bool MAX>
Rank Heap<T, MAX>::percolateUp(Rank index) {
    TeachingAccess access(*this);
    return Algorithm::siftUp(access, index, Higher());
}

template<typename T, bool MAX>
Rank Heap<T, MAX>::percolateDown(Rank count, Rank index) {
    TeachingAccess access(*this);
    return Algorithm::siftDown(access, count, index, Higher());
}

template<typename T, bool MAX>
void Heap<T, MAX>::insert(T value) {
    Vector<T>::insert(value);
    percolateUp(this->_size - 1);
}

template<typename T, bool MAX>
DSA_NOINLINE T Heap<T, MAX>::getMax() {
    if (this->_size == 0)
        throw std::runtime_error("Heap is empty");
    return this->_elem[0];
}

template<typename T, bool MAX>
T Heap<T, MAX>::delMax() {
    if (this->_size == 0)
        throw std::runtime_error("Heap is empty");

    T result = this->_elem[0];
    --this->_size;
    if (this->_size > 0) {
        this->_elem[0] = this->_elem[this->_size];
        percolateDown(this->_size, 0);
    }
    return result;
}

#endif
