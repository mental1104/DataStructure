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
T* toAddress(T* pointer) noexcept;

/// 递归解引用 fancy pointer，最终取得其指向对象的原始地址。
template<typename Pointer>
auto toAddress(const Pointer& pointer) noexcept
    -> decltype(detail::toAddress(pointer.operator->()));

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

#include "detail/VectorStorage.inl"
#include "detail/VectorLifecycle.inl"
#include "detail/VectorModifiers.inl"
#include "detail/VectorHelpers.inl"

} // namespace container
} // namespace dsa

#endif
