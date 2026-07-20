// -----------------------------------------------------------------------------
// 私有辅助函数实现
// -----------------------------------------------------------------------------

/// 计算几何增长后的推荐容量。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::recommendedCapacity(size_type required) const {
    return MutationAlgorithm::recommendCapacity(
        capacity_,
        required,
        size_type(1),
        max_size()
    );
}

/// 容量不足时按推荐容量扩容。
template<typename T, typename Allocator>
void Vector<T, Allocator>::reserveRecommended(size_type required) {
    if (required > capacity_)
        reserve(recommendedCapacity(required));
}

/// 将下标转换为可写迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::iteratorAt(size_type index) noexcept {
    return index == 0 ? data_ : data_ + index;
}

/// 将下标转换为只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::iteratorAt(size_type index) const noexcept {
    return index == 0 ? data_ : data_ + index;
}

/// 验证迭代器属于当前容器并转换为下标。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::indexOf(const_iterator position) const {
    if (size_ == 0) {
        if (position != data_)
            throw std::out_of_range("Vector iterator does not belong to container");
        return 0;
    }

    if (position < data_ || position > data_ + size_)
        throw std::out_of_range("Vector iterator does not belong to container");
    return static_cast<size_type>(position - data_);
}

/// 通过共享 VectorAlgorithm 插入已经准备好的值。
template<typename T, typename Allocator>
template<typename Value>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::insertPrepared(
    const_iterator position,
    Value&& value
) {
    const size_type index = indexOf(position);
    Storage storage(*this);
    MutationAlgorithm::insert(storage, index, std::forward<Value>(value));
    return iteratorAt(index);
}

/// 检查 at 访问位置是否越界。
template<typename T, typename Allocator>
void Vector<T, Allocator>::checkPosition(size_type position) const {
    if (position >= size_)
        throw std::out_of_range("Vector::at position out of range");
}

/// 初始化 count 个值初始化元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::initializeDefault(size_type count) {
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

/// 初始化 count 个 value 副本。
template<typename T, typename Allocator>
void Vector<T, Allocator>::initializeFill(
    size_type count,
    const value_type& value
) {
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

/// 使用单遍输入迭代器逐个追加元素。
template<typename T, typename Allocator>
template<typename InputIt>
void Vector<T, Allocator>::initializeRange(
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

/// 使用前向迭代器一次申请足够存储并逐个构造元素。
template<typename T, typename Allocator>
template<typename ForwardIt>
void Vector<T, Allocator>::initializeRange(
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

/// 申请指定容量的未初始化存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::allocateStorage(size_type capacity) {
    if (capacity == 0)
        return;
    allocation_ = allocator_traits::allocate(allocator_, capacity);
    data_ = detail::toAddress(allocation_);
    capacity_ = capacity;
}

/// 释放当前底层存储并重置容量字段。
template<typename T, typename Allocator>
void Vector<T, Allocator>::deallocateStorage() noexcept {
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

/// 逆序析构指定下标区间内的元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::destroyRange(
    size_type first,
    size_type last
) noexcept {
    while (last > first) {
        --last;
        allocator_traits::destroy(allocator_, data_ + last);
    }
}

/// 重新分配精确容量，并在失败时回滚新存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::reallocateExact(size_type newCapacity) {
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

/// 将存储字段重置为空状态。
template<typename T, typename Allocator>
void Vector<T, Allocator>::resetStorage() noexcept {
    allocation_ = allocation_pointer();
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
}

/// 接管 other 的存储并清空 other 的所有权字段。
template<typename T, typename Allocator>
void Vector<T, Allocator>::stealStorage(Vector& other) noexcept {
    allocation_ = other.allocation_;
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.resetStorage();
}

/// 仅交换存储字段。
template<typename T, typename Allocator>
void Vector<T, Allocator>::swapStorage(Vector& other) noexcept {
    using std::swap;
    swap(allocation_, other.allocation_);
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
}

/// allocator 允许传播时执行拷贝赋值。
template<typename T, typename Allocator>
void Vector<T, Allocator>::copyAssign(
    const Vector& other,
    std::true_type
) {
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

/// allocator 不允许传播时使用当前 allocator 深拷贝。
template<typename T, typename Allocator>
void Vector<T, Allocator>::copyAssign(
    const Vector& other,
    std::false_type
) {
    Vector replacement(other, allocator_);
    swapStorage(replacement);
}

/// allocator 允许传播时执行移动赋值并接管存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::moveAssign(Vector& other, std::true_type) {
    clear();
    deallocateStorage();
    allocator_ = std::move(other.allocator_);
    stealStorage(other);
}

/// allocator 不允许传播时根据 allocator 是否相等选择接管或逐个移动。
template<typename T, typename Allocator>
void Vector<T, Allocator>::moveAssign(Vector& other, std::false_type) {
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

/// allocator 允许传播时同时交换 allocator 与存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::swapImpl(Vector& other, std::true_type) {
    using std::swap;
    swap(allocator_, other.allocator_);
    swapStorage(other);
}

/// allocator 不允许传播时只交换相等 allocator 的存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::swapImpl(Vector& other, std::false_type) {
    if (allocator_ != other.allocator_) {
        throw std::logic_error(
            "Vector::swap requires equal non-propagating allocators"
        );
    }
    swapStorage(other);
}

// -----------------------------------------------------------------------------
// 非成员函数实现
// -----------------------------------------------------------------------------

/// 比较两个 Vector 是否逐项相等。
template<typename T, typename Allocator>
bool operator==(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin());
}

/// 判断两个 Vector 是否不相等。
template<typename T, typename Allocator>
bool operator!=(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
) {
    return !(left == right);
}

/// 通过成员 swap 交换两个 Vector。
template<typename T, typename Allocator>
void swap(Vector<T, Allocator>& left, Vector<T, Allocator>& right) {
    left.swap(right);
}
