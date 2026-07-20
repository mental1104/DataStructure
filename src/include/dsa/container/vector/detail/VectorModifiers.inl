// -----------------------------------------------------------------------------
// 容量与修改操作实现
// -----------------------------------------------------------------------------

/// 判断容器是否为空。
template<typename T, typename Allocator>
bool Vector<T, Allocator>::empty() const noexcept {
    return size_ == 0;
}

/// 返回当前有效元素数量。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::size() const noexcept {
    return size_;
}

/// 返回当前容器允许的最大元素数量。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::max_size() const noexcept {
    const size_type allocatorMaximum = allocator_traits::max_size(allocator_);
    const size_type differenceMaximum = static_cast<size_type>(
        (std::numeric_limits<difference_type>::max)()
    );
    return allocatorMaximum < differenceMaximum
        ? allocatorMaximum
        : differenceMaximum;
}

/// 至少预留 requestedCapacity 个元素的存储空间。
template<typename T, typename Allocator>
void Vector<T, Allocator>::reserve(size_type requestedCapacity) {
    if (requestedCapacity > max_size())
        throw std::length_error("Vector::reserve exceeds max_size");
    if (requestedCapacity <= capacity_)
        return;
    reallocateExact(requestedCapacity);
}

/// 返回当前容量。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::capacity() const noexcept {
    return capacity_;
}

/// 将底层容量压缩到当前 size。
template<typename T, typename Allocator>
void Vector<T, Allocator>::shrink_to_fit() {
    if (size_ == capacity_)
        return;
    if (size_ == 0) {
        deallocateStorage();
        return;
    }
    reallocateExact(size_);
}

/// 析构全部有效元素但保留容量。
template<typename T, typename Allocator>
void Vector<T, Allocator>::clear() noexcept {
    destroyRange(0, size_);
    size_ = 0;
}

/// 复制插入单个元素。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::insert(
    const_iterator position,
    const value_type& value
) {
    value_type pending(value);
    return insertPrepared(position, std::move(pending));
}

/// 移动插入单个元素。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::insert(
    const_iterator position,
    value_type&& value
) {
    value_type pending(std::move(value));
    return insertPrepared(position, std::move(pending));
}

/// 在指定位置前构造并插入元素。
template<typename T, typename Allocator>
template<typename... Args>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::emplace(const_iterator position, Args&&... args) {
    value_type pending(std::forward<Args>(args)...);
    return insertPrepared(position, std::move(pending));
}

/// 删除单个元素并返回后继迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::erase(const_iterator position) {
    return erase(position, position + 1);
}

/// 删除指定区间并返回删除位置的后继迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::erase(
    const_iterator first,
    const_iterator last
) {
    const size_type firstIndex = indexOf(first);
    const size_type lastIndex = indexOf(last);
    if (firstIndex > lastIndex || lastIndex > size_)
        throw std::out_of_range("Vector::erase invalid range");

    Storage storage(*this);
    MutationAlgorithm::erase(storage, firstIndex, lastIndex);
    return iteratorAt(firstIndex);
}

/// 在尾部复制追加元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const value_type& value) {
    insert(cend(), value);
}

/// 在尾部移动追加元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::push_back(value_type&& value) {
    insert(cend(), std::move(value));
}

/// 在尾部原地构造元素。
template<typename T, typename Allocator>
template<typename... Args>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::emplace_back(Args&&... args) {
    iterator inserted = emplace(cend(), std::forward<Args>(args)...);
    return *inserted;
}

/// 在首部复制插入元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::push_front(const value_type& value) {
    insert(cbegin(), value);
}

/// 在首部移动插入元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::push_front(value_type&& value) {
    insert(cbegin(), std::move(value));
}

/// 在首部原地构造元素。
template<typename T, typename Allocator>
template<typename... Args>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::emplace_front(Args&&... args) {
    iterator inserted = emplace(cbegin(), std::forward<Args>(args)...);
    return *inserted;
}

/// 删除尾元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::pop_back() {
    erase(cend() - 1);
}

/// 删除首元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::pop_front() {
    erase(cbegin());
}

/// 调整大小并对新增元素执行值初始化。
template<typename T, typename Allocator>
void Vector<T, Allocator>::resize(size_type count) {
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

/// 调整大小并使用 value 构造新增元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::resize(
    size_type count,
    const value_type& value
) {
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

/// 按 allocator propagation 规则交换两个 Vector。
template<typename T, typename Allocator>
void Vector<T, Allocator>::swap(Vector& other) {
    swapImpl(
        other,
        typename allocator_traits::propagate_on_container_swap()
    );
}
