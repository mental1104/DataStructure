#ifndef __DSA_VECTOR
#define __DSA_VECTOR

#include <utility>

#include "utils.h"
#include "../dsa/core/vector/VectorAlgorithm.h"
#include "../dsa/algorithm/Search.h"
#include "../dsa/algorithm/Sequence.h"

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

    class TeachingStorage {
    public:
        typedef Rank size_type;

        explicit TeachingStorage(Vector<T>& vector)
            : vector_(vector) {
        }

        size_type size() const {
            return vector_._size;
        }

        size_type maxSize() const {
            return std::numeric_limits<size_type>::max();
        }

        void ensureCapacity(size_type required) {
            while (required > vector_._capacity)
                vector_.expand();
        }

        void openGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            for (size_type i = oldSize; i > position; --i)
                vector_._elem[i + count - 1] = vector_._elem[i - 1];
        }

        template<typename Value>
        void writeGap(size_type position, Value&& value) {
            vector_._elem[position] = std::forward<Value>(value);
        }

        void rollbackGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            for (size_type i = position; i < oldSize; ++i)
                vector_._elem[i] = vector_._elem[i + count];
        }

        void closeGap(
            size_type first,
            size_type last,
            size_type oldSize
        ) {
            while (last < oldSize)
                vector_._elem[first++] = vector_._elem[last++];
        }

        void commitSize(size_type newSize) {
            vector_._size = newSize;
        }

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
        DEFAULT_CAPACITY,
        std::numeric_limits<int>::max()
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
    return static_cast<Rank>(result - _elem) - 1;
}

template<typename T>
Rank Vector<T>::binSearch(
    T* A,
    T const& e,
    Rank lo,
    Rank hi
) const {
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
    return static_cast<Rank>(
        dsa::algorithm::fibonacciUpperBound(A + lo, A + hi, e) - A
    ) - 1;
}

template<typename T>
T Vector<T>::majEleCandidate() {
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
    T* cur;

    explicit iterator(T* rhs)
        : cur(rhs) {}

    bool operator!=(const iterator& other) {
        return cur != other.cur;
    }

    T& operator*() {
        return *cur;
    }

    iterator& operator++() {
        ++cur;
        return *this;
    }
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
