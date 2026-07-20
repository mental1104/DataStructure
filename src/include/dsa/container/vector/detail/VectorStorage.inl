// -----------------------------------------------------------------------------
// detail 辅助函数实现
// -----------------------------------------------------------------------------

/// 实现普通指针到原始地址的直接转换。
template<typename T>
T* detail::toAddress(T* pointer) noexcept {
    return pointer;
}

/// 实现 fancy pointer 到原始地址的递归转换。
template<typename Pointer>
auto detail::toAddress(const Pointer& pointer) noexcept
    -> decltype(detail::toAddress(pointer.operator->())) {
    return detail::toAddress(pointer.operator->());
}

// -----------------------------------------------------------------------------
// Storage policy 实现
// -----------------------------------------------------------------------------

/// 初始化一次增删操作所需的临时状态。
template<typename T, typename Allocator>
Vector<T, Allocator>::Storage::Storage(Vector& owner)
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

/// 在离开作用域时清理尚未提交的新存储。
template<typename T, typename Allocator>
Vector<T, Allocator>::Storage::~Storage() {
    discardPending();
}

/// 返回宿主 Vector 的当前 size。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::Storage::size() const {
    return owner_.size_;
}

/// 返回宿主 Vector 的最大容量限制。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::size_type
Vector<T, Allocator>::Storage::maxSize() const {
    return owner_.max_size();
}

/// 容量不足时申请新存储，原存储暂不改变。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::ensureCapacity(size_type required) {
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

/// 根据是否发生扩容，选择新存储构造或原地搬移方案。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::openGap(
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

/// 将待插入值构造或赋值到打开的槽位。
template<typename T, typename Allocator>
template<typename Value>
void Vector<T, Allocator>::Storage::writeGap(
    size_type position,
    Value&& value
) {
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

/// 回滚插入失败时已经构造的临时对象。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::rollbackGap(
    size_type,
    size_type,
    size_type
) noexcept {
    if (usingPending_) {
        discardPending();
        return;
    }

    // 原地移动赋值抛异常时只能提供 basic guarantee；此处至少销毁额外构造的尾元素。
    if (tailConstructed_) {
        allocator_traits::destroy(
            owner_.allocator_,
            owner_.data_ + oldSize_
        );
        tailConstructed_ = false;
    }
}

/// 前移删除区间后的元素，并析构逻辑尾部退出的对象。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::closeGap(
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

/// 提交新的 size，并在扩容路径中原子切换到新存储。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::commitSize(size_type newSize) {
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

/// 工业 Vector 的 erase 不自动缩容，因此删除后无需额外动作。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::afterErase() {
}

/// 在新存储中分别构造插入点前缀和插入点后缀。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::openGapInPendingStorage(
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
                std::move_if_noexcept(owner_.data_[prefixConstructed_])
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

/// 在已有容量内构造新尾元素并从后向前移动赋值。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::openGapInCurrentStorage(
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

/// 逆序析构新存储中的已构造对象并释放内存。
template<typename T, typename Allocator>
void Vector<T, Allocator>::Storage::discardPending() noexcept {
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
