#ifndef __DSA_VECTOR
#define __DSA_VECTOR

#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "utils.h"
#include <dsa/core/vector/VectorAlgorithm.h>
#include <dsa/algorithm/Search.h>
#include <dsa/algorithm/Sequence.h>

template<typename T>
class Vector {
protected:
    int _size;
    int _capacity;
    T* _elem;
    int heap;

    void copyFrom(T const* A, Rank lo, Rank hi);
    void expand();
    void shrink();

    /**
     * 将教学版 Vector 的字段、扩缩容和元素搬移操作，
     * 适配为 VectorAlgorithm 所需的 Storage 操作合同。
     *
     * 这里使用适配器模式连接旧教学存储模型，并通过模板策略实现
     * 静态多态：VectorAlgorithm 负责编排增删流程，TeachingStorage
     * 负责 new T[] 存储模型下的具体操作。
     */
    class TeachingStorage {
    public:
        typedef Rank size_type;

        /// 绑定当前要执行增删流程的教学版 Vector。
        explicit TeachingStorage(Vector<T>& vector)
            : vector_(vector) {
        }

        /// 返回当前逻辑元素数量。
        size_type size() const {
            return vector_._size;
        }

        /**
         * 返回教学版 Vector 逻辑元素数量的理论上限。
         *
         * 该值用于插入流程检查 size 增长是否越界，不表示当前 capacity，
         * 也不保证系统能够实际分配如此大的数组。
         */
        size_type maxSize() const {
            return std::numeric_limits<size_type>::max();
        }

        /// 反复执行教学版 expand，直到容量能够容纳 required 个元素。
        void ensureCapacity(size_type required) {
            while (required > vector_._capacity)
                vector_.expand();
        }

        /**
         * 将 [position, oldSize) 中的元素从右向左搬移 count 位，
         * 为待插入值打开连续槽位，但暂不修改 _size。
         *
         * 示例：position = 1，count = 1
         *
         * 搬移前：
         * 下标  0   1   2   3   4
         *      [A] [B] [C] [D] [_]
         *
         * 搬移后：
         * 下标  0   1   2   3   4
         *      [A] [B] [B] [C] [D]
         *           ^
         *       待写入槽位
         *
         * 必须从右向左搬移，否则先写入右侧位置会覆盖尚未搬走的元素。
         */
        void openGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            for (size_type i = oldSize; i > position; --i)
                vector_._elem[i + count - 1] = vector_._elem[i - 1];
        }

        /// 将待插入值写入已经打开的槽位。
        template<typename Value>
        void writeGap(size_type position, Value&& value) {
            vector_._elem[position] = std::forward<Value>(value);
        }

        /**
         * writeGap 抛出异常时，将 openGap 右移的原元素重新向左搬回。
         *
         * openGap 后：
         *      [A] [_] [B] [C] [D]
         *           ^
         *       写入失败
         *
         * 回滚过程：
         *      elem[1] = elem[2]  // B
         *      elem[2] = elem[3]  // C
         *      elem[3] = elem[4]  // D
         *
         * 回滚后：
         *      [A] [B] [C] [D] [D]
         *       └──逻辑区间──┘
         *
         * _size 尚未提交，尾部重复元素位于逻辑区间之外。
         * 从左向右恢复不会覆盖后续仍需读取的来源元素。
         */
        void rollbackGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            for (size_type i = position; i < oldSize; ++i)
                vector_._elem[i] = vector_._elem[i + count];
        }

        /**
         * 删除区间 [first, last) 后，将右侧后缀向左搬移以关闭空隙。
         *
         * 示例：删除 [1, 3)
         *
         * 删除前：
         * 下标  0   1   2   3   4
         *      [A] [B] [C] [D] [E]
         *           └──删除──┘
         *
         * 搬移后：
         *      [A] [D] [E] [D] [E]
         *       └─新逻辑区间─┘
         *
         * 此函数只恢复元素布局，不提交新的 _size；逻辑尾部的旧值由
         * 教学版数组对象模型保留，随后 commitSize 决定新的有效区间。
         */
        void closeGap(
            size_type first,
            size_type last,
            size_type oldSize
        ) {
            while (last < oldSize)
                vector_._elem[first++] = vector_._elem[last++];
        }

        /// 提交增删操作成功后的新逻辑大小。
        void commitSize(size_type newSize) {
            vector_._size = newSize;
        }

        /// 保留教学版“删除后按旧策略自动缩容”的行为。
        void afterErase() {
            vector_.shrink();
        }

    private:
        Vector<T>& vector_;
    };

    typedef dsa::core::VectorAlgorithm<TeachingStorage> MutationAlgorithm;

public:
    typedef T value_type;

    Vector(int c = DEFAULT_CAPACITY, int s = 0, T v = T()) {
        _capacity = c;
        _elem = new T[_capacity];
        for (_size = 0; _size < s; _elem[_size++] = v) {
        }
    }

    Vector(T const* A, Rank n) {
        copyFrom(A, 0, n);
    }

    Vector(T const* A, Rank lo, Rank hi) {
        copyFrom(A, lo, hi);
    }

    Vector(Vector<T> const& V) {
        copyFrom(V._elem, 0, V._size);
    }

    Vector(Vector<T> const& V, Rank lo, Rank hi) {
        copyFrom(V._elem, lo, hi);
    }

    ~Vector() {
        delete [] _elem;
    }

    Vector<T>& operator=(Vector<T> const&);
    T& operator[](Rank r) const;

    Rank size() const {
        return _size;
    }

    bool empty() const {
        return !_size;
    }

    Rank capacity() const {
        return _capacity;
    }

    struct iterator;
    iterator begin();
    iterator end();
    const iterator begin() const;
    const iterator end() const;

    int disordered() const;
    Rank find(T const& e) const {
        return find(e, 0, _size);
    }
    Rank find(T const& e, Rank lo, Rank hi) const;
    Rank binSearch(T* A, T const& e, Rank lo, Rank hi) const;
    Rank fibSearch(T* A, T const& e, Rank lo, Rank hi) const;
    Rank search(T const& e) const {
        return (0 >= _size) ? -1 : search(e, 0, _size);
    }
    Rank search(T const& e, Rank lo, Rank hi) const;

    T remove(Rank r);
    int remove(Rank lo, Rank hi);
    Rank insert(Rank r, T const& e);
    Rank insert(T const& e) {
        return insert(_size, e);
    }

    /**
     * 返回当前 Vector 的多数候选。
     *
     * 该函数只完成候选筛选，不保证返回值一定是多数元素；
     * 调用方仍需统计出现次数并验证其是否超过 size / 2。
     * 空 Vector 不存在可返回的候选。
     */
    T majEleCandidate();
    void range(int k);
    void unsort(Rank lo, Rank hi);
    void unsort() {
        unsort(0, _size);
    }
    int deduplicate();
    int uniquify();

    void traverse(void (*)(T&));

    template<typename VST>
    void traverse(VST&&);
};

template<typename T>
void Vector<T>::copyFrom(T const* A, Rank lo, Rank hi) {
    _elem = new T[_capacity = 2 * (hi - lo)];
    _size = 0;
    while (lo < hi)
        _elem[_size++] = A[lo++];
}

template<typename T>
Vector<T>& Vector<T>::operator=(Vector<T> const& V) {
    if (this == &V)
        return *this;

    if (_capacity < V._size) {
        delete [] _elem;
        _capacity = 2 * V._size;
        _elem = new T[_capacity];
    }

    _size = V._size;
    for (int i = 0; i < _size; ++i)
        _elem[i] = V._elem[i];
    return *this;
}

template<typename T>
void Vector<T>::expand() {
    if (_size < _capacity)
        return;

    T* oldElem = _elem;
    _capacity = MutationAlgorithm::recommendCapacity(
        _capacity,
        _size + 1,
        DEFAULT_CAPACITY
    );
    _elem = new T[_capacity];

    for (int i = 0; i < _size; ++i)
        _elem[i] = oldElem[i];
    delete [] oldElem;
}

template<typename T>
void Vector<T>::shrink() {
    if (_capacity < (DEFAULT_CAPACITY << 1))
        return;
    if ((_size << 2) > _capacity)
        return;

    T* oldElem = _elem;
    _capacity >>= 1;
    _elem = new T[_capacity];

    for (int i = 0; i < _size; ++i)
        _elem[i] = oldElem[i];
    delete [] oldElem;
}

template<typename T>
T& Vector<T>::operator[](Rank r) const {
    return _elem[r];
}

template<typename T>
void Vector<T>::unsort(Rank lo, Rank hi) {
    dsa::algorithm::shuffle(_elem + lo, _elem + hi, eng);
}

template<typename T>
Rank Vector<T>::find(T const& e, Rank lo, Rank hi) const {
    T* const last = _elem + hi;
    T* const result = dsa::algorithm::findLast(_elem + lo, last, e);
    return result == last ? -1 : static_cast<Rank>(result - _elem);
}

template<typename T>
Rank Vector<T>::insert(Rank r, T const& e) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::insert(storage, r, e);
}

template<typename T>
int Vector<T>::remove(Rank lo, Rank hi) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::erase(storage, lo, hi);
}

template<typename T>
T Vector<T>::remove(Rank r) {
    T value = _elem[r];
    remove(r, r + 1);
    return value;
}

template<typename T>
int Vector<T>::deduplicate() {
    const Rank oldSize = _size;
    T* const newEnd = dsa::algorithm::deduplicate(_elem, _elem + _size);
    const Rank newSize = static_cast<Rank>(newEnd - _elem);
    remove(newSize, oldSize);
    return oldSize - newSize;
}

template<typename T>
void Vector<T>::traverse(void (*visit)(T&)) {
    dsa::algorithm::forEach(_elem, _elem + _size, visit);
}

template<typename T>
template<typename VST>
void Vector<T>::traverse(VST&& visit) {
    dsa::algorithm::forEach(
        _elem,
        _elem + _size,
        std::forward<VST>(visit)
    );
}

template<typename T>
int Vector<T>::disordered() const {
    return static_cast<int>(
        dsa::algorithm::disorderCount(_elem, _elem + _size)
    );
}

template<typename T>
int Vector<T>::uniquify() {
    const Rank oldSize = _size;
    T* const newEnd = dsa::algorithm::uniqueAdjacent(_elem, _elem + _size);
    const Rank newSize = static_cast<Rank>(newEnd - _elem);
    remove(newSize, oldSize);
    return oldSize - newSize;
}

template<typename T>
Rank Vector<T>::search(T const& e, Rank lo, Rank hi) const {
    T* const result = dsa::algorithm::upperBound(
        _elem + lo,
        _elem + hi,
        e
    );

    // upperBound 返回第一个大于 e 的元素；向前一位就是教材 search
    // 所要求的“最后一个不大于 e 的元素”。
    return static_cast<Rank>(result - _elem) - 1;
}

template<typename T>
Rank Vector<T>::binSearch(
    T* A,
    T const& e,
    Rank lo,
    Rank hi
) const {
    // 教材 binSearch 使用二分策略寻找第一个大于 e 的位置，
    // 再减一得到“最后一个不大于 e 的元素”，不是仅查找精确匹配。
    return static_cast<Rank>(
        dsa::algorithm::upperBound(A + lo, A + hi, e) - A
    ) - 1;
}

template<typename T>
Rank Vector<T>::fibSearch(
    T* A,
    T const& e,
    Rank lo,
    Rank hi
) const {
    // fibonacciUpperBound 与普通 upperBound 返回语义相同，
    // 但使用斐波那契分割区间；减一后得到最后一个不大于 e 的位置。
    return static_cast<Rank>(
        dsa::algorithm::fibonacciUpperBound(A + lo, A + hi, e) - A
    ) - 1;
}

template<typename T>
T Vector<T>::majEleCandidate() {
    if (_size == 0)
        throw std::logic_error("majEleCandidate requires a non-empty vector");

    T* const candidate = dsa::algorithm::majorityCandidate(
        _elem,
        _elem + _size
    );
    return *candidate;
}

template<typename T>
void Vector<T>::range(int k) {
    Vector<int> candidates(k - 1, k - 1, -1);
    Vector<int> counts(k - 1, k - 1, 0);

    for (int i = 0; i < size(); ++i) {
        Rank index = counts.find(0);
        if (index >= 0) {
            candidates[index] = _elem[i];
            counts[index] = 1;
        } else {
            Rank candidate = candidates.find(_elem[i]);
            if (candidate != -1) {
                ++counts[candidate];
            } else {
                for (int j = 0; j < counts.size(); ++j)
                    --counts[j];
            }
        }
    }

    candidates.deduplicate();
    for (int j = 0; j < candidates.size(); ++j) {
        int occurrence = 0;
        for (int i = 0; i < size(); ++i) {
            if (candidates[j] == _elem[i])
                ++occurrence;
        }
        (void)occurrence;
    }
}

template<typename T>
struct Vector<T>::iterator {
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    T* cur;

    explicit iterator(T* rhs = nullptr) : cur(rhs) {}

    reference operator*() const { return *cur; }
    pointer operator->() const { return cur; }
    reference operator[](difference_type offset) const { return cur[offset]; }

    iterator& operator++() {
        ++cur;
        return *this;
    }

    iterator operator++(int) {
        iterator old(*this);
        ++(*this);
        return old;
    }

    iterator& operator--() {
        --cur;
        return *this;
    }

    iterator operator--(int) {
        iterator old(*this);
        --(*this);
        return old;
    }

    iterator& operator+=(difference_type offset) { cur += offset; return *this; }
    iterator& operator-=(difference_type offset) { cur -= offset; return *this; }

    friend iterator operator+(iterator it, difference_type offset) { it += offset; return it; }
    friend iterator operator+(difference_type offset, iterator it) { it += offset; return it; }
    friend iterator operator-(iterator it, difference_type offset) { it -= offset; return it; }
    friend difference_type operator-(const iterator& left, const iterator& right) {
        return left.cur - right.cur;
    }
    friend bool operator==(const iterator& left, const iterator& right) { return left.cur == right.cur; }
    friend bool operator!=(const iterator& left, const iterator& right) { return !(left == right); }
    friend bool operator<(const iterator& left, const iterator& right) { return left.cur < right.cur; }
    friend bool operator>(const iterator& left, const iterator& right) { return right < left; }
    friend bool operator<=(const iterator& left, const iterator& right) { return !(right < left); }
    friend bool operator>=(const iterator& left, const iterator& right) { return !(left < right); }
};

template<typename T>
typename Vector<T>::iterator Vector<T>::begin() {
    return iterator(_elem);
}

template<typename T>
typename Vector<T>::iterator Vector<T>::end() {
    return iterator(_elem + _size);
}

template<typename T>
inline const typename Vector<T>::iterator Vector<T>::begin() const {
    return iterator(_elem);
}

template<typename T>
inline const typename Vector<T>::iterator Vector<T>::end() const {
    return iterator(_elem + _size);
}

#endif
