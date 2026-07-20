#ifndef DSA_CONTAINER_VECTOR_VECTOR_H
#define DSA_CONTAINER_VECTOR_VECTOR_H

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "../../core/vector/VectorAlgorithm.h"

namespace dsa {
namespace container {
namespace detail {

template<typename T>
T* toAddress(T* pointer) noexcept {
    return pointer;
}

template<typename Pointer>
auto toAddress(const Pointer& pointer) noexcept
    -> decltype(detail::toAddress(pointer.operator->())) {
    return detail::toAddress(pointer.operator->());
}

} // namespace detail

// A readable allocator-aware vector implementation modelled after std::vector.
//
// Design goals:
// - contiguous storage and random-access iterators;
// - only [0, size) contains constructed objects;
// - allocator-aware copy/move/swap semantics;
// - amortized O(1) push_back/emplace_back through geometric growth;
// - shared insert/erase workflow through dsa::core::VectorAlgorithm;
// - no automatic shrinking on erase, matching std::vector.
template<typename T, typename Allocator = std::allocator<T> >
class Vector {
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    typedef typename allocator_traits::pointer allocation_pointer;

    allocator_type allocator_;
    allocation_pointer allocation_;
    pointer data_;
    size_type size_;
    size_type capacity_;

    class Storage {
    public:
        typedef typename Vector::size_type size_type;

        explicit Storage(Vector& owner)
            : owner_(owner),
              pendingAllocation_(),
              pendingData_(nullptr),
              pendingCapacity_(0),
              usingPending_(false),
              prefixConstructed_(0),
              suffixBegin_(0),
              suffixConstructed_(0),
              gapConstructed_(false),
              tailConstructed_(false),
              position_(0),
              oldSize_(0) {
        }

        ~Storage() {
            discardPending();
        }

        size_type size() const {
            return owner_.size_;
        }

        size_type maxSize() const {
            return owner_.max_size();
        }

        void ensureCapacity(size_type required) {
            if (required <= owner_.capacity_)
                return;

            pendingCapacity_ = owner_.recommendedCapacity(required);
            pendingAllocation_ = allocator_traits::allocate(
                owner_.allocator_,
                pendingCapacity_
            );
            pendingData_ = detail::toAddress(pendingAllocation_);
            usingPending_ = true;
        }

        void openGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            position_ = position;
            oldSize_ = oldSize;

            if (usingPending_) {
                openGapInPendingStorage(position, count, oldSize);
                return;
            }

            openGapInCurrentStorage(position, count, oldSize);
        }

        template<typename Value>
        void writeGap(size_type position, Value&& value) {
            if (usingPending_) {
                allocator_traits::construct(
                    owner_.allocator_,
                    pendingData_ + position,
                    std::forward<Value>(value)
                );
                gapConstructed_ = true;
                return;
            }

            if (position == oldSize_) {
                allocator_traits::construct(
                    owner_.allocator_,
                    owner_.data_ + position,
                    std::forward<Value>(value)
                );
                tailConstructed_ = true;
                return;
            }

            owner_.data_[position] = std::forward<Value>(value);
        }

        void rollbackGap(
            size_type,
            size_type,
            size_type
        ) noexcept {
            if (usingPending_) {
                discardPending();
                return;
            }

            // When an in-place assignment throws, std::vector can only provide
            // the basic guarantee for element types whose move/copy assignment
            // may throw. The shifted range remains valid but unspecified.
            if (tailConstructed_) {
                allocator_traits::destroy(
                    owner_.allocator_,
                    owner_.data_ + oldSize_
                );
                tailConstructed_ = false;
            }
        }

        void closeGap(
            size_type first,
            size_type last,
            size_type oldSize
        ) {
            const size_type removed = last - first;
            size_type destination = first;
            size_type source = last;

            while (source < oldSize) {
                owner_.data_[destination] = std::move_if_noexcept(
                    owner_.data_[source]
                );
                ++destination;
                ++source;
            }

            owner_.destroyRange(oldSize - removed, oldSize);
        }

        void commitSize(size_type newSize) {
            if (!usingPending_) {
                owner_.size_ = newSize;
                tailConstructed_ = false;
                return;
            }

            owner_.destroyRange(0, owner_.size_);
            owner_.deallocateStorage();

            owner_.allocation_ = pendingAllocation_;
            owner_.data_ = pendingData_;
            owner_.size_ = newSize;
            owner_.capacity_ = pendingCapacity_;

            pendingAllocation_ = allocation_pointer();
            pendingData_ = nullptr;
            pendingCapacity_ = 0;
            usingPending_ = false;
            prefixConstructed_ = 0;
            suffixConstructed_ = 0;
            gapConstructed_ = false;
        }

        void afterErase() {
            // std::vector::erase does not reduce capacity. Users can request a
            // compaction explicitly with shrink_to_fit().
        }

    private:
        Vector& owner_;
        allocation_pointer pendingAllocation_;
        pointer pendingData_;
        size_type pendingCapacity_;
        bool usingPending_;
        size_type prefixConstructed_;
        size_type suffixBegin_;
        size_type suffixConstructed_;
        bool gapConstructed_;
        bool tailConstructed_;
        size_type position_;
        size_type oldSize_;

        Storage(const Storage&);
        Storage& operator=(const Storage&);

        void openGapInPendingStorage(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            suffixBegin_ = position + count;

            try {
                for (; prefixConstructed_ < position; ++prefixConstructed_) {
                    allocator_traits::construct(
                        owner_.allocator_,
                        pendingData_ + prefixConstructed_,
                        std::move_if_noexcept(
                            owner_.data_[prefixConstructed_]
                        )
                    );
                }

                for (
                    size_type source = position;
                    source < oldSize;
                    ++source, ++suffixConstructed_
                ) {
                    allocator_traits::construct(
                        owner_.allocator_,
                        pendingData_ + suffixBegin_ + suffixConstructed_,
                        std::move_if_noexcept(owner_.data_[source])
                    );
                }
            } catch (...) {
                discardPending();
                throw;
            }
        }

        void openGapInCurrentStorage(
            size_type position,
            size_type count,
            size_type oldSize
        ) {
            if (count != 1)
                throw std::logic_error("Vector currently opens one slot at a time");
            if (position == oldSize)
                return;

            allocator_traits::construct(
                owner_.allocator_,
                owner_.data_ + oldSize,
                std::move_if_noexcept(owner_.data_[oldSize - 1])
            );
            tailConstructed_ = true;

            try {
                for (size_type index = oldSize - 1; index > position; --index) {
                    owner_.data_[index] = std::move_if_noexcept(
                        owner_.data_[index - 1]
                    );
                }
            } catch (...) {
                allocator_traits::destroy(
                    owner_.allocator_,
                    owner_.data_ + oldSize
                );
                tailConstructed_ = false;
                throw;
            }
        }

        void discardPending() noexcept {
            if (!usingPending_)
                return;

            if (gapConstructed_) {
                allocator_traits::destroy(
                    owner_.allocator_,
                    pendingData_ + position_
                );
                gapConstructed_ = false;
            }

            while (suffixConstructed_ > 0) {
                --suffixConstructed_;
                allocator_traits::destroy(
                    owner_.allocator_,
                    pendingData_ + suffixBegin_ + suffixConstructed_
                );
            }

            while (prefixConstructed_ > 0) {
                --prefixConstructed_;
                allocator_traits::destroy(
                    owner_.allocator_,
                    pendingData_ + prefixConstructed_
                );
            }

            allocator_traits::deallocate(
                owner_.allocator_,
                pendingAllocation_,
                pendingCapacity_
            );

            pendingAllocation_ = allocation_pointer();
            pendingData_ = nullptr;
            pendingCapacity_ = 0;
            usingPending_ = false;
        }
    };

    typedef dsa::core::VectorAlgorithm<Storage> MutationAlgorithm;

public:
    Vector() noexcept(std::is_nothrow_default_constructible<allocator_type>::value)
        : allocator_(),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
    }

    explicit Vector(const allocator_type& allocator) noexcept
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
    }

    explicit Vector(
        size_type count,
        const allocator_type& allocator = allocator_type()
    )
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeDefault(count);
    }

    Vector(
        size_type count,
        const value_type& value,
        const allocator_type& allocator = allocator_type()
    )
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeFill(count, value);
    }

    template<typename InputIt>
    Vector(
        InputIt first,
        InputIt last,
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIt>::value>::type* = nullptr
    )
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeRange(
            first,
            last,
            typename std::iterator_traits<InputIt>::iterator_category()
        );
    }

    Vector(
        std::initializer_list<value_type> values,
        const allocator_type& allocator = allocator_type()
    )
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeRange(
            values.begin(),
            values.end(),
            std::forward_iterator_tag()
        );
    }

    Vector(const Vector& other)
        : allocator_(
              allocator_traits::select_on_container_copy_construction(
                  other.allocator_
              )
          ),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeRange(
            other.begin(),
            other.end(),
            std::forward_iterator_tag()
        );
    }

    Vector(const Vector& other, const allocator_type& allocator)
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        initializeRange(
            other.begin(),
            other.end(),
            std::forward_iterator_tag()
        );
    }

    Vector(Vector&& other) noexcept(
        std::is_nothrow_move_constructible<allocator_type>::value
    )
        : allocator_(std::move(other.allocator_)),
          allocation_(other.allocation_),
          data_(other.data_),
          size_(other.size_),
          capacity_(other.capacity_) {
        other.resetStorage();
    }

    Vector(Vector&& other, const allocator_type& allocator)
        : allocator_(allocator),
          allocation_(),
          data_(nullptr),
          size_(0),
          capacity_(0) {
        if (allocator_ == other.allocator_) {
            stealStorage(other);
        } else {
            initializeRange(
                std::make_move_iterator(other.begin()),
                std::make_move_iterator(other.end()),
                std::forward_iterator_tag()
            );
            other.clear();
        }
    }

    ~Vector() {
        clear();
        deallocateStorage();
    }

    Vector& operator=(const Vector& other) {
        if (this == &other)
            return *this;

        copyAssign(
            other,
            typename allocator_traits::propagate_on_container_copy_assignment()
        );
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept(
        allocator_traits::propagate_on_container_move_assignment::value &&
        std::is_nothrow_move_assignable<allocator_type>::value
    ) {
        if (this == &other)
            return *this;

        moveAssign(
            other,
            typename allocator_traits::propagate_on_container_move_assignment()
        );
        return *this;
    }

    Vector& operator=(std::initializer_list<value_type> values) {
        assign(values.begin(), values.end());
        return *this;
    }

    void assign(size_type count, const value_type& value) {
        Vector replacement(count, value, allocator_);
        swapStorage(replacement);
    }

    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
    assign(InputIt first, InputIt last) {
        Vector replacement(first, last, allocator_);
        swapStorage(replacement);
    }

    void assign(std::initializer_list<value_type> values) {
        assign(values.begin(), values.end());
    }

    allocator_type get_allocator() const {
        return allocator_;
    }

    reference at(size_type position) {
        checkPosition(position);
        return data_[position];
    }

    const_reference at(size_type position) const {
        checkPosition(position);
        return data_[position];
    }

    reference operator[](size_type position) noexcept {
        return data_[position];
    }

    const_reference operator[](size_type position) const noexcept {
        return data_[position];
    }

    reference front() noexcept {
        return data_[0];
    }

    const_reference front() const noexcept {
        return data_[0];
    }

    reference back() noexcept {
        return data_[size_ - 1];
    }

    const_reference back() const noexcept {
        return data_[size_ - 1];
    }

    pointer data() noexcept {
        return data_;
    }

    const_pointer data() const noexcept {
        return data_;
    }

    iterator begin() noexcept {
        return data_;
    }

    const_iterator begin() const noexcept {
        return data_;
    }

    const_iterator cbegin() const noexcept {
        return data_;
    }

    iterator end() noexcept {
        return size_ == 0 ? data_ : data_ + size_;
    }

    const_iterator end() const noexcept {
        return size_ == 0 ? data_ : data_ + size_;
    }

    const_iterator cend() const noexcept {
        return size_ == 0 ? data_ : data_ + size_;
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    size_type max_size() const noexcept {
        const size_type allocatorMaximum = allocator_traits::max_size(allocator_);
        const size_type differenceMaximum = static_cast<size_type>(
            (std::numeric_limits<difference_type>::max)()
        );
        return allocatorMaximum < differenceMaximum
            ? allocatorMaximum
            : differenceMaximum;
    }

    void reserve(size_type requestedCapacity) {
        if (requestedCapacity > max_size())
            throw std::length_error("Vector::reserve exceeds max_size");
        if (requestedCapacity <= capacity_)
            return;
        reallocateExact(requestedCapacity);
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    void shrink_to_fit() {
        if (size_ == capacity_)
            return;
        if (size_ == 0) {
            deallocateStorage();
            return;
        }
        reallocateExact(size_);
    }

    void clear() noexcept {
        destroyRange(0, size_);
        size_ = 0;
    }

    iterator insert(const_iterator position, const value_type& value) {
        value_type pending(value);
        return insertPrepared(position, std::move(pending));
    }

    iterator insert(const_iterator position, value_type&& value) {
        value_type pending(std::move(value));
        return insertPrepared(position, std::move(pending));
    }

    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args) {
        value_type pending(std::forward<Args>(args)...);
        return insertPrepared(position, std::move(pending));
    }

    iterator erase(const_iterator position) {
        return erase(position, position + 1);
    }

    iterator erase(const_iterator first, const_iterator last) {
        const size_type firstIndex = indexOf(first);
        const size_type lastIndex = indexOf(last);
        if (firstIndex > lastIndex || lastIndex > size_)
            throw std::out_of_range("Vector::erase invalid range");

        Storage storage(*this);
        MutationAlgorithm::erase(storage, firstIndex, lastIndex);
        return iteratorAt(firstIndex);
    }

    void push_back(const value_type& value) {
        insert(cend(), value);
    }

    void push_back(value_type&& value) {
        insert(cend(), std::move(value));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        iterator inserted = emplace(cend(), std::forward<Args>(args)...);
        return *inserted;
    }

    // Non-standard convenience extension. Like any front insertion in a
    // contiguous vector, this is O(n).
    void push_front(const value_type& value) {
        insert(cbegin(), value);
    }

    void push_front(value_type&& value) {
        insert(cbegin(), std::move(value));
    }

    template<typename... Args>
    reference emplace_front(Args&&... args) {
        iterator inserted = emplace(cbegin(), std::forward<Args>(args)...);
        return *inserted;
    }

    void pop_back() {
        erase(cend() - 1);
    }

    void pop_front() {
        erase(cbegin());
    }

    void resize(size_type count) {
        if (count < size_) {
            destroyRange(count, size_);
            size_ = count;
            return;
        }

        if (count == size_)
            return;

        reserveRecommended(count);
        const size_type oldSize = size_;
        try {
            while (size_ < count) {
                allocator_traits::construct(allocator_, data_ + size_);
                ++size_;
            }
        } catch (...) {
            destroyRange(oldSize, size_);
            size_ = oldSize;
            throw;
        }
    }

    void resize(size_type count, const value_type& value) {
        if (count < size_) {
            destroyRange(count, size_);
            size_ = count;
            return;
        }

        if (count == size_)
            return;

        reserveRecommended(count);
        const size_type oldSize = size_;
        try {
            while (size_ < count) {
                allocator_traits::construct(
                    allocator_,
                    data_ + size_,
                    value
                );
                ++size_;
            }
        } catch (...) {
            destroyRange(oldSize, size_);
            size_ = oldSize;
            throw;
        }
    }

    void swap(Vector& other) {
        swapImpl(
            other,
            typename allocator_traits::propagate_on_container_swap()
        );
    }

private:
    size_type recommendedCapacity(size_type required) const {
        return MutationAlgorithm::recommendCapacity(
            capacity_,
            required,
            size_type(1),
            max_size()
        );
    }

    void reserveRecommended(size_type required) {
        if (required > capacity_)
            reserve(recommendedCapacity(required));
    }

    iterator iteratorAt(size_type index) noexcept {
        if (index == 0)
            return data_;
        return data_ + index;
    }

    const_iterator iteratorAt(size_type index) const noexcept {
        if (index == 0)
            return data_;
        return data_ + index;
    }

    size_type indexOf(const_iterator position) const {
        if (size_ == 0) {
            if (position != data_)
                throw std::out_of_range("Vector iterator does not belong to container");
            return 0;
        }

        if (position < data_ || position > data_ + size_)
            throw std::out_of_range("Vector iterator does not belong to container");
        return static_cast<size_type>(position - data_);
    }

    template<typename Value>
    iterator insertPrepared(const_iterator position, Value&& value) {
        const size_type index = indexOf(position);
        Storage storage(*this);
        MutationAlgorithm::insert(storage, index, std::forward<Value>(value));
        return iteratorAt(index);
    }

    void checkPosition(size_type position) const {
        if (position >= size_)
            throw std::out_of_range("Vector::at position out of range");
    }

    void initializeDefault(size_type count) {
        if (count == 0)
            return;
        if (count > max_size())
            throw std::length_error("Vector size exceeds max_size");

        allocateStorage(count);
        try {
            while (size_ < count) {
                allocator_traits::construct(allocator_, data_ + size_);
                ++size_;
            }
        } catch (...) {
            destroyRange(0, size_);
            deallocateStorage();
            throw;
        }
    }

    void initializeFill(size_type count, const value_type& value) {
        if (count == 0)
            return;
        if (count > max_size())
            throw std::length_error("Vector size exceeds max_size");

        allocateStorage(count);
        try {
            while (size_ < count) {
                allocator_traits::construct(
                    allocator_,
                    data_ + size_,
                    value
                );
                ++size_;
            }
        } catch (...) {
            destroyRange(0, size_);
            deallocateStorage();
            throw;
        }
    }

    template<typename InputIt>
    void initializeRange(
        InputIt first,
        InputIt last,
        std::input_iterator_tag
    ) {
        try {
            for (; first != last; ++first)
                emplace_back(*first);
        } catch (...) {
            clear();
            deallocateStorage();
            throw;
        }
    }

    template<typename ForwardIt>
    void initializeRange(
        ForwardIt first,
        ForwardIt last,
        std::forward_iterator_tag
    ) {
        const difference_type distance = std::distance(first, last);
        if (distance <= 0)
            return;

        const size_type count = static_cast<size_type>(distance);
        if (count > max_size())
            throw std::length_error("Vector size exceeds max_size");

        allocateStorage(count);
        try {
            for (; first != last; ++first) {
                allocator_traits::construct(
                    allocator_,
                    data_ + size_,
                    *first
                );
                ++size_;
            }
        } catch (...) {
            destroyRange(0, size_);
            deallocateStorage();
            throw;
        }
    }

    void allocateStorage(size_type capacity) {
        if (capacity == 0)
            return;
        allocation_ = allocator_traits::allocate(allocator_, capacity);
        data_ = detail::toAddress(allocation_);
        capacity_ = capacity;
    }

    void deallocateStorage() noexcept {
        if (capacity_ != 0) {
            allocator_traits::deallocate(
                allocator_,
                allocation_,
                capacity_
            );
        }
        allocation_ = allocation_pointer();
        data_ = nullptr;
        capacity_ = 0;
    }

    void destroyRange(size_type first, size_type last) noexcept {
        while (last > first) {
            --last;
            allocator_traits::destroy(allocator_, data_ + last);
        }
    }

    void reallocateExact(size_type newCapacity) {
        allocation_pointer newAllocation = allocator_traits::allocate(
            allocator_,
            newCapacity
        );
        pointer newData = detail::toAddress(newAllocation);
        size_type constructed = 0;

        try {
            for (; constructed < size_; ++constructed) {
                allocator_traits::construct(
                    allocator_,
                    newData + constructed,
                    std::move_if_noexcept(data_[constructed])
                );
            }
        } catch (...) {
            while (constructed > 0) {
                --constructed;
                allocator_traits::destroy(
                    allocator_,
                    newData + constructed
                );
            }
            allocator_traits::deallocate(
                allocator_,
                newAllocation,
                newCapacity
            );
            throw;
        }

        destroyRange(0, size_);
        deallocateStorage();
        allocation_ = newAllocation;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void resetStorage() noexcept {
        allocation_ = allocation_pointer();
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void stealStorage(Vector& other) noexcept {
        allocation_ = other.allocation_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.resetStorage();
    }

    void swapStorage(Vector& other) noexcept {
        using std::swap;
        swap(allocation_, other.allocation_);
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    void copyAssign(const Vector& other, std::true_type) {
        if (allocator_ != other.allocator_) {
            Vector replacement(other, other.allocator_);
            clear();
            deallocateStorage();
            allocator_ = other.allocator_;
            stealStorage(replacement);
            return;
        }

        Vector replacement(other, allocator_);
        swapStorage(replacement);
    }

    void copyAssign(const Vector& other, std::false_type) {
        Vector replacement(other, allocator_);
        swapStorage(replacement);
    }

    void moveAssign(Vector& other, std::true_type) {
        clear();
        deallocateStorage();
        allocator_ = std::move(other.allocator_);
        stealStorage(other);
    }

    void moveAssign(Vector& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            clear();
            deallocateStorage();
            stealStorage(other);
            return;
        }

        Vector replacement(
            std::make_move_iterator(other.begin()),
            std::make_move_iterator(other.end()),
            allocator_
        );
        swapStorage(replacement);
        other.clear();
    }

    void swapImpl(Vector& other, std::true_type) {
        using std::swap;
        swap(allocator_, other.allocator_);
        swapStorage(other);
    }

    void swapImpl(Vector& other, std::false_type) {
        if (allocator_ != other.allocator_) {
            throw std::logic_error(
                "Vector::swap requires equal non-propagating allocators"
            );
        }
        swapStorage(other);
    }
};

template<typename T, typename Allocator>
bool operator==(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin());
}

template<typename T, typename Allocator>
bool operator!=(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
) {
    return !(left == right);
}

template<typename T, typename Allocator>
void swap(Vector<T, Allocator>& left, Vector<T, Allocator>& right) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
