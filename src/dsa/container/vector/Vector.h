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

/// 将普通指针直接转换为原始地址，兼容 C++11 环境下尚未提供的 std::to_address。
template<typename T>
T* toAddress(T* pointer) noexcept {
    return pointer;
}

/// 递归解引用 fancy pointer，最终取得其指向对象的原始地址。
template<typename Pointer>
auto toAddress(const Pointer& pointer) noexcept
    -> decltype(detail::toAddress(pointer.operator->())) {
    return detail::toAddress(pointer.operator->());
}

} // namespace detail

/// 面向学习实现的 allocator-aware 连续数组容器，核心语义对齐 std::vector。
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

    /// 将工业 Vector 的 allocator 与对象生命周期操作适配到共享 VectorAlgorithm。
    class Storage {
    public:
        typedef typename Vector::size_type size_type;

        /// 绑定当前要执行增删流程的 Vector 实例。
        explicit Storage(Vector& owner);

        /// 清理尚未提交的临时存储，保证异常路径不泄漏。
        ~Storage();

        /// 返回当前逻辑元素数量。
        size_type size() const;

        /// 返回当前 allocator 允许的最大元素数量。
        size_type maxSize() const;

        /// 为一次插入准备足够容量，但暂不提交新的存储。
        void ensureCapacity(size_type required);

        /// 在指定位置打开 count 个槽位，并搬运原有元素。
        void openGap(size_type position, size_type count, size_type oldSize);

        /// 将待插入值写入已经打开的槽位。
        template<typename Value>
        void writeGap(size_type position, Value&& value);

        /// 插入失败时回滚当前 Storage 已创建的临时对象。
        void rollbackGap(
            size_type position,
            size_type count,
            size_type oldSize
        ) noexcept;

        /// 删除区间后将后续元素前移，并析构退出逻辑区间的尾部对象。
        void closeGap(size_type first, size_type last, size_type oldSize);

        /// 提交新的 size；若使用了新存储，则同时切换底层内存。
        void commitSize(size_type newSize);

        /// 执行删除后的容器策略；工业版保持 capacity 不变。
        void afterErase();

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

        /// 禁止复制 Storage，避免重复管理同一份临时存储。
        Storage(const Storage&);

        /// 禁止复制赋值 Storage，避免重复管理同一份临时存储。
        Storage& operator=(const Storage&);

        /// 在新申请的存储中构造插入位置两侧的元素。
        void openGapInPendingStorage(
            size_type position,
            size_type count,
            size_type oldSize
        );

        /// 在现有存储中原地后移元素，为插入打开一个槽位。
        void openGapInCurrentStorage(
            size_type position,
            size_type count,
            size_type oldSize
        );

        /// 析构并释放尚未提交的新存储。
        void discardPending() noexcept;
    };

    typedef dsa::core::VectorAlgorithm<Storage> MutationAlgorithm;

public:
    /// 构造空 Vector，并默认构造 allocator。
    Vector() noexcept(std::is_nothrow_default_constructible<allocator_type>::value);

    /// 使用指定 allocator 构造空 Vector。
    explicit Vector(const allocator_type& allocator) noexcept;

    /// 构造 count 个值初始化元素。
    explicit Vector(
        size_type count,
        const allocator_type& allocator = allocator_type()
    );

    /// 构造 count 个 value 副本。
    Vector(
        size_type count,
        const value_type& value,
        const allocator_type& allocator = allocator_type()
    );

    /// 从迭代器区间 [first, last) 构造 Vector。
    template<typename InputIt>
    Vector(
        InputIt first,
        InputIt last,
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIt>::value>::type* = nullptr
    );

    /// 从 initializer_list 构造 Vector。
    Vector(
        std::initializer_list<value_type> values,
        const allocator_type& allocator = allocator_type()
    );

    /// 使用 select_on_container_copy_construction 选择 allocator 后深拷贝元素。
    Vector(const Vector& other);

    /// 使用调用方指定的 allocator 深拷贝另一个 Vector。
    Vector(const Vector& other, const allocator_type& allocator);

    /// 移动构造并直接接管另一个 Vector 的存储。
    Vector(Vector&& other) noexcept(
        std::is_nothrow_move_constructible<allocator_type>::value
    );

    /// 使用指定 allocator 移动构造；allocator 不同时逐个移动元素。
    Vector(Vector&& other, const allocator_type& allocator);

    /// 析构所有有效元素并释放底层存储。
    ~Vector();

    /// 按 allocator propagation 规则执行拷贝赋值。
    Vector& operator=(const Vector& other);

    /// 按 allocator propagation 规则执行移动赋值。
    Vector& operator=(Vector&& other) noexcept(
        allocator_traits::propagate_on_container_move_assignment::value &&
        std::is_nothrow_move_assignable<allocator_type>::value
    );

    /// 使用 initializer_list 替换当前所有元素。
    Vector& operator=(std::initializer_list<value_type> values);

    /// 将容器内容替换为 count 个 value 副本。
    void assign(size_type count, const value_type& value);

    /// 将容器内容替换为迭代器区间 [first, last) 的元素。
    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
    assign(InputIt first, InputIt last);

    /// 将容器内容替换为 initializer_list 中的元素。
    void assign(std::initializer_list<value_type> values);

    /// 返回当前使用的 allocator 副本。
    allocator_type get_allocator() const noexcept;

    /// 返回指定位置元素，并在越界时抛出 std::out_of_range。
    reference at(size_type position);

    /// 返回指定位置只读元素，并在越界时抛出 std::out_of_range。
    const_reference at(size_type position) const;

    /// 不检查边界地返回指定位置元素。
    reference operator[](size_type position) noexcept;

    /// 不检查边界地返回指定位置只读元素。
    const_reference operator[](size_type position) const noexcept;

    /// 返回首元素引用；空容器调用行为与 std::vector 一样未定义。
    reference front() noexcept;

    /// 返回首元素只读引用；空容器调用行为与 std::vector 一样未定义。
    const_reference front() const noexcept;

    /// 返回尾元素引用；空容器调用行为与 std::vector 一样未定义。
    reference back() noexcept;

    /// 返回尾元素只读引用；空容器调用行为与 std::vector 一样未定义。
    const_reference back() const noexcept;

    /// 返回连续存储首地址。
    pointer data() noexcept;

    /// 返回连续存储首地址的只读指针。
    const_pointer data() const noexcept;

    /// 返回首元素迭代器。
    iterator begin() noexcept;

    /// 返回首元素只读迭代器。
    const_iterator begin() const noexcept;

    /// 返回首元素只读迭代器。
    const_iterator cbegin() const noexcept;

    /// 返回尾后迭代器。
    iterator end() noexcept;

    /// 返回尾后只读迭代器。
    const_iterator end() const noexcept;

    /// 返回尾后只读迭代器。
    const_iterator cend() const noexcept;

    /// 返回反向首迭代器。
    reverse_iterator rbegin() noexcept;

    /// 返回反向首只读迭代器。
    const_reverse_iterator rbegin() const noexcept;

    /// 返回反向首只读迭代器。
    const_reverse_iterator crbegin() const noexcept;

    /// 返回反向尾后迭代器。
    reverse_iterator rend() noexcept;

    /// 返回反向尾后只读迭代器。
    const_reverse_iterator rend() const noexcept;

    /// 返回反向尾后只读迭代器。
    const_reverse_iterator crend() const noexcept;

    /// 判断容器是否不含元素。
    bool empty() const noexcept;

    /// 返回当前有效元素数量。
    size_type size() const noexcept;

    /// 返回 allocator 和 difference_type 共同允许的最大元素数量。
    size_type max_size() const noexcept;

    /// 至少预留 requestedCapacity 个元素的存储空间。
    void reserve(size_type requestedCapacity);

    /// 返回当前可容纳且无需重新分配的元素数量。
    size_type capacity() const noexcept;

    /// 尝试将 capacity 压缩到 size；空容器会释放全部存储。
    void shrink_to_fit();

    /// 析构所有有效元素，但保留底层容量。
    void clear() noexcept;

    /// 在 position 前插入 value 的副本，并返回新元素迭代器。
    iterator insert(const_iterator position, const value_type& value);

    /// 在 position 前移动插入 value，并返回新元素迭代器。
    iterator insert(const_iterator position, value_type&& value);

    /// 在 position 前原地构造元素，并返回新元素迭代器。
    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args);

    /// 删除 position 指向的元素，并返回其后继迭代器。
    iterator erase(const_iterator position);

    /// 删除区间 [first, last)，并返回删除位置的后继迭代器。
    iterator erase(const_iterator first, const_iterator last);

    /// 在尾部复制追加一个元素。
    void push_back(const value_type& value);

    /// 在尾部移动追加一个元素。
    void push_back(value_type&& value);

    /// 在尾部原地构造一个元素，并返回其引用。
    template<typename... Args>
    reference emplace_back(Args&&... args);

    /// 在首部复制插入一个元素；连续存储下复杂度为 O(n)。
    void push_front(const value_type& value);

    /// 在首部移动插入一个元素；连续存储下复杂度为 O(n)。
    void push_front(value_type&& value);

    /// 在首部原地构造一个元素；连续存储下复杂度为 O(n)。
    template<typename... Args>
    reference emplace_front(Args&&... args);

    /// 删除尾元素；空容器调用行为与 std::vector 一样未定义。
    void pop_back();

    /// 删除首元素；这是非标准扩展且复杂度为 O(n)。
    void pop_front();

    /// 将逻辑大小调整为 count，新增元素使用值初始化。
    void resize(size_type count);

    /// 将逻辑大小调整为 count，新增元素复制 value。
    void resize(size_type count, const value_type& value);

    /// 按 allocator propagation 规则交换两个 Vector。
    void swap(Vector& other);

private:
    /// 根据当前容量、目标容量和 max_size 计算几何增长后的推荐容量。
    size_type recommendedCapacity(size_type required) const;

    /// 仅在容量不足时按推荐容量执行 reserve。
    void reserveRecommended(size_type required);

    /// 根据下标构造可写迭代器，兼容空容器的 nullptr。
    iterator iteratorAt(size_type index) noexcept;

    /// 根据下标构造只读迭代器，兼容空容器的 nullptr。
    const_iterator iteratorAt(size_type index) const noexcept;

    /// 将本容器迭代器转换为下标，并验证所属关系。
    size_type indexOf(const_iterator position) const;

    /// 将已准备好的值交给共享 VectorAlgorithm 完成插入。
    template<typename Value>
    iterator insertPrepared(const_iterator position, Value&& value);

    /// 检查元素访问位置是否合法。
    void checkPosition(size_type position) const;

    /// 初始化 count 个值初始化元素。
    void initializeDefault(size_type count);

    /// 初始化 count 个 value 副本。
    void initializeFill(size_type count, const value_type& value);

    /// 使用单遍输入迭代器逐个追加元素。
    template<typename InputIt>
    void initializeRange(InputIt first, InputIt last, std::input_iterator_tag);

    /// 使用前向迭代器预先计算距离并一次申请存储。
    template<typename ForwardIt>
    void initializeRange(ForwardIt first, ForwardIt last, std::forward_iterator_tag);

    /// 使用 allocator 申请指定容量的未初始化存储。
    void allocateStorage(size_type capacity);

    /// 使用 allocator 释放当前底层存储。
    void deallocateStorage() noexcept;

    /// 逆序析构下标区间 [first, last) 内的元素。
    void destroyRange(size_type first, size_type last) noexcept;

    /// 精确重新分配到 newCapacity，并搬运所有有效元素。
    void reallocateExact(size_type newCapacity);

    /// 将存储字段重置为空状态，不析构也不释放原存储。
    void resetStorage() noexcept;

    /// 接管 other 的存储，并将 other 重置为空状态。
    void stealStorage(Vector& other) noexcept;

    /// 仅交换存储字段，不交换 allocator。
    void swapStorage(Vector& other) noexcept;

    /// allocator 允许传播时执行拷贝赋值。
    void copyAssign(const Vector& other, std::true_type);

    /// allocator 不允许传播时使用当前 allocator 执行拷贝赋值。
    void copyAssign(const Vector& other, std::false_type);

    /// allocator 允许传播时执行移动赋值。
    void moveAssign(Vector& other, std::true_type);

    /// allocator 不允许传播时根据 allocator 是否相等决定接管或逐个移动。
    void moveAssign(Vector& other, std::false_type);

    /// allocator 允许传播时交换 allocator 与存储。
    void swapImpl(Vector& other, std::true_type);

    /// allocator 不允许传播时仅允许相等 allocator 交换存储。
    void swapImpl(Vector& other, std::false_type);
};

/// 比较两个 Vector 的长度和逐项元素是否相等。
template<typename T, typename Allocator>
bool operator==(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
);

/// 判断两个 Vector 是否不相等。
template<typename T, typename Allocator>
bool operator!=(
    const Vector<T, Allocator>& left,
    const Vector<T, Allocator>& right
);

/// 调用成员 swap 交换两个 Vector。
template<typename T, typename Allocator>
void swap(Vector<T, Allocator>& left, Vector<T, Allocator>& right);

// -----------------------------------------------------------------------------
// detail 辅助函数实现
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// Vector 构造、析构与赋值实现
// -----------------------------------------------------------------------------

/// 默认构造空 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector() noexcept(
    std::is_nothrow_default_constructible<allocator_type>::value
)
    : allocator_(),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
}

/// 使用指定 allocator 构造空 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(const allocator_type& allocator) noexcept
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
}

/// 构造 count 个值初始化元素。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    size_type count,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeDefault(count);
}

/// 构造 count 个 value 副本。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    size_type count,
    const value_type& value,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    initializeFill(count, value);
}

/// 从迭代器区间构造 Vector，并按迭代器类别选择初始化策略。
template<typename T, typename Allocator>
template<typename InputIt>
Vector<T, Allocator>::Vector(
    InputIt first,
    InputIt last,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIt>::value>::type*
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

/// 从 initializer_list 构造 Vector。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    std::initializer_list<value_type> values,
    const allocator_type& allocator
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

/// 深拷贝构造，并遵循 allocator 的 copy-construction 选择规则。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Vector& other)
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

/// 使用指定 allocator 深拷贝构造。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    const Vector& other,
    const allocator_type& allocator
)
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

/// 移动构造并直接接管 other 的存储。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other) noexcept(
    std::is_nothrow_move_constructible<allocator_type>::value
)
    : allocator_(std::move(other.allocator_)),
      allocation_(other.allocation_),
      data_(other.data_),
      size_(other.size_),
      capacity_(other.capacity_) {
    other.resetStorage();
}

/// 使用指定 allocator 移动构造。
template<typename T, typename Allocator>
Vector<T, Allocator>::Vector(
    Vector&& other,
    const allocator_type& allocator
)
    : allocator_(allocator),
      allocation_(),
      data_(nullptr),
      size_(0),
      capacity_(0) {
    if (allocator_ == other.allocator_) {
        stealStorage(other);
        return;
    }

    initializeRange(
        std::make_move_iterator(other.begin()),
        std::make_move_iterator(other.end()),
        std::forward_iterator_tag()
    );
    other.clear();
}

/// 析构有效元素并释放底层存储。
template<typename T, typename Allocator>
Vector<T, Allocator>::~Vector() {
    clear();
    deallocateStorage();
}

/// 执行拷贝赋值并根据 allocator trait 选择传播策略。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(const Vector& other) {
    if (this == &other)
        return *this;

    copyAssign(
        other,
        typename allocator_traits::propagate_on_container_copy_assignment()
    );
    return *this;
}

/// 执行移动赋值并根据 allocator trait 选择传播策略。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(Vector&& other) noexcept(
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

/// 使用 initializer_list 替换当前内容。
template<typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(
    std::initializer_list<value_type> values
) {
    assign(values.begin(), values.end());
    return *this;
}

/// 将内容替换为 count 个 value 副本。
template<typename T, typename Allocator>
void Vector<T, Allocator>::assign(
    size_type count,
    const value_type& value
) {
    Vector replacement(count, value, allocator_);
    swapStorage(replacement);
}

/// 将内容替换为迭代器区间元素。
template<typename T, typename Allocator>
template<typename InputIt>
typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
Vector<T, Allocator>::assign(InputIt first, InputIt last) {
    Vector replacement(first, last, allocator_);
    swapStorage(replacement);
}

/// 将内容替换为 initializer_list 元素。
template<typename T, typename Allocator>
void Vector<T, Allocator>::assign(
    std::initializer_list<value_type> values
) {
    assign(values.begin(), values.end());
}

/// 返回 allocator 副本。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::allocator_type
Vector<T, Allocator>::get_allocator() const noexcept {
    return allocator_;
}

// -----------------------------------------------------------------------------
// 元素访问与迭代器实现
// -----------------------------------------------------------------------------

/// 返回带越界检查的可写元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::at(size_type position) {
    checkPosition(position);
    return data_[position];
}

/// 返回带越界检查的只读元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::at(size_type position) const {
    checkPosition(position);
    return data_[position];
}

/// 返回不做边界检查的可写元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::operator[](size_type position) noexcept {
    return data_[position];
}

/// 返回不做边界检查的只读元素引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::operator[](size_type position) const noexcept {
    return data_[position];
}

/// 返回首元素可写引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::front() noexcept {
    return data_[0];
}

/// 返回首元素只读引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::front() const noexcept {
    return data_[0];
}

/// 返回尾元素可写引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reference
Vector<T, Allocator>::back() noexcept {
    return data_[size_ - 1];
}

/// 返回尾元素只读引用。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reference
Vector<T, Allocator>::back() const noexcept {
    return data_[size_ - 1];
}

/// 返回连续存储首地址。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::pointer
Vector<T, Allocator>::data() noexcept {
    return data_;
}

/// 返回连续存储首地址的只读指针。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_pointer
Vector<T, Allocator>::data() const noexcept {
    return data_;
}

/// 返回首元素迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::begin() noexcept {
    return data_;
}

/// 返回首元素只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::begin() const noexcept {
    return data_;
}

/// 返回首元素只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::cbegin() const noexcept {
    return data_;
}

/// 返回尾后迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::end() noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::end() const noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator
Vector<T, Allocator>::cend() const noexcept {
    return size_ == 0 ? data_ : data_ + size_;
}

/// 返回反向首迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator
Vector<T, Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

/// 返回反向首只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

/// 返回反向首只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(cend());
}

/// 返回反向尾后迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator
Vector<T, Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

/// 返回反向尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

/// 返回反向尾后只读迭代器。
template<typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator
Vector<T, Allocator>::crend() const noexcept {
    return const_reverse_iterator(cbegin());
}

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

} // namespace container
} // namespace dsa

#endif
