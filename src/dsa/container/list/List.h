#ifndef DSA_CONTAINER_LIST_LIST_H
#define DSA_CONTAINER_LIST_LIST_H

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "../../core/list/ListAlgorithm.h"

namespace dsa {
namespace container {
namespace list_detail {

/// 将普通指针转换为原始地址，兼容 C++11 尚未提供的 std::to_address。
template<typename U>
U* toAddress(U* pointer) noexcept {
    return pointer;
}

/// 递归解引用 fancy pointer，最终取得其指向对象的原始地址。
template<typename Pointer>
auto toAddress(const Pointer& pointer) noexcept
    -> decltype(list_detail::toAddress(pointer.operator->())) {
    return list_detail::toAddress(pointer.operator->());
}

} // namespace list_detail

/// 面向学习实现的 allocator-aware 双向链表，核心语义对齐 std::list。
template<typename T, typename Allocator = std::allocator<T> >
class List {
public:
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::const_pointer const_pointer;

private:
    /// 不含 T 的链接基类，使空链表哨兵不要求 T 可默认构造。
    struct NodeBase {
        NodeBase* previous;
        NodeBase* next;

        /// 初始化脱链节点；哨兵随后由 resetSentinel 建立自环。
        NodeBase() noexcept
            : previous(nullptr), next(nullptr) {
        }
    };

    /// 真正拥有一个 T 对象的链表节点。
    struct Node : NodeBase {
        value_type value;

        /// 使用完美转发直接构造节点中的业务值。
        template<typename... Args>
        explicit Node(Args&&... args)
            : NodeBase(), value(std::forward<Args>(args)...) {
        }
    };

    typedef typename allocator_traits::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;
    typedef typename node_allocator_traits::pointer node_pointer;

public:
    /// 同时服务 iterator 与 const_iterator 的双向迭代器实现。
    template<bool IsConst>
    class BasicIterator {
        friend class List;
        template<bool>
        friend class BasicIterator;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef typename List::difference_type difference_type;
        typedef typename std::conditional<IsConst, const T*, T*>::type pointer;
        typedef typename std::conditional<IsConst, const T&, T&>::type reference;

        /// 构造空迭代器，仅用于默认初始化和容器内部赋值。
        BasicIterator() noexcept
            : node_(nullptr) {
        }

        /// 允许 iterator 隐式转换为 const_iterator，禁止反向转换。
        template<bool OtherConst>
        BasicIterator(
            const BasicIterator<OtherConst>& other,
            typename std::enable_if<IsConst && !OtherConst>::type* = nullptr
        ) noexcept
            : node_(other.node_) {
        }

        /// 返回当前节点保存的元素引用。
        reference operator*() const {
            return static_cast<Node*>(node_)->value;
        }

        /// 返回当前节点保存元素的地址。
        pointer operator->() const {
            return std::addressof(static_cast<Node*>(node_)->value);
        }

        /// 前置移动到后继节点。
        BasicIterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }

        /// 后置移动到后继节点并返回旧位置。
        BasicIterator operator++(int) noexcept {
            BasicIterator old(*this);
            ++(*this);
            return old;
        }

        /// 前置移动到前驱节点。
        BasicIterator& operator--() noexcept {
            node_ = node_->previous;
            return *this;
        }

        /// 后置移动到前驱节点并返回旧位置。
        BasicIterator operator--(int) noexcept {
            BasicIterator old(*this);
            --(*this);
            return old;
        }

        /// 比较两个可写或只读迭代器是否指向同一节点。
        template<bool OtherConst>
        bool operator==(const BasicIterator<OtherConst>& other) const noexcept {
            return node_ == other.node_;
        }

        /// 比较两个可写或只读迭代器是否指向不同节点。
        template<bool OtherConst>
        bool operator!=(const BasicIterator<OtherConst>& other) const noexcept {
            return node_ != other.node_;
        }

    private:
        NodeBase* node_;

        /// 由容器内部将节点句柄包装为迭代器。
        explicit BasicIterator(NodeBase* node) noexcept
            : node_(node) {
        }
    };

    typedef BasicIterator<false> iterator;
    typedef BasicIterator<true> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    allocator_type allocator_;
    NodeBase sentinel_;
    size_type size_;

    /// 将工业 List 的节点 allocator、链接和 size 操作适配到共享 ListAlgorithm。
    class Storage {
    public:
        typedef NodeBase node_type;
        typedef typename List::size_type size_type;

        /// 绑定当前 mutation 所属的 List。
        explicit Storage(List& owner) noexcept
            : owner_(owner) {
        }

        /// 申请并构造一个尚未接入链表的节点；失败时释放已申请存储。
        template<typename... Args>
        node_type* createNode(Args&&... args) {
            return owner_.createNode(std::forward<Args>(args)...);
        }

        /// 析构并释放已经从链表摘除的节点。
        void destroyNode(node_type* node) noexcept {
            owner_.destroyNode(static_cast<Node*>(node));
        }

        /// 将脱链节点接到 position 前，操作本身不修改 size。
        void linkBefore(node_type* position, node_type* node) noexcept {
            NodeBase* before = position->previous;
            node->previous = before;
            node->next = position;
            before->next = node;
            position->previous = node;
        }

        /// 从双向链中摘除节点，操作本身不修改 size。
        void unlink(node_type* node) noexcept {
            node->previous->next = node->next;
            node->next->previous = node->previous;
            node->previous = nullptr;
            node->next = nullptr;
        }

        /// 返回节点后继，供共享删除流程生成返回迭代器。
        node_type* next(node_type* node) const noexcept {
            return node->next;
        }

        /// 线性计算 [first, last) 的节点数量；跨容器 range splice 需要更新 size。
        size_type countRange(node_type* first, node_type* last) const {
            size_type count = 0;
            while (first != last) {
                ++count;
                first = first->next;
            }
            return count;
        }

        /// 判断两个 Storage 是否操作同一个 List 实例。
        bool sameOwner(const Storage& other) const noexcept {
            return &owner_ == &other.owner_;
        }

        /// 判断 target 是否落在 [first, last) 内，用于防御 self-splice 链接破坏。
        bool contains(
            node_type* first,
            node_type* last,
            node_type* target
        ) const {
            while (first != last) {
                if (first == target)
                    return true;
                first = first->next;
            }
            return false;
        }

        /// 将 [first, last) 从原链摘下并整体接到 position 前。
        void transferBefore(
            node_type* position,
            node_type* first,
            node_type* last
        ) noexcept {
            NodeBase* beforeFirst = first->previous;
            NodeBase* lastMoved = last->previous;
            NodeBase* beforePosition = position->previous;

            beforeFirst->next = last;
            last->previous = beforeFirst;

            beforePosition->next = first;
            first->previous = beforePosition;
            lastMoved->next = position;
            position->previous = lastMoved;
        }

        /// 提交插入后的逻辑节点数量。
        void commitInsert(size_type count) noexcept {
            owner_.size_ += count;
        }

        /// 提交删除或跨容器转移后的逻辑节点数量。
        void commitErase(size_type count) noexcept {
            owner_.size_ -= count;
        }

    private:
        List& owner_;
    };

    typedef dsa::core::ListAlgorithm<Storage> MutationAlgorithm;

public:
    /// 构造空 List，并默认构造 allocator。
    List() noexcept(std::is_nothrow_default_constructible<allocator_type>::value);

    /// 使用指定 allocator 构造空 List。
    explicit List(const allocator_type& allocator) noexcept(
        std::is_nothrow_copy_constructible<allocator_type>::value
    );

    /// 构造 count 个值初始化元素。
    explicit List(
        size_type count,
        const allocator_type& allocator = allocator_type()
    );

    /// 构造 count 个 value 副本。
    List(
        size_type count,
        const value_type& value,
        const allocator_type& allocator = allocator_type()
    );

    /// 从迭代器区间 [first, last) 构造 List。
    template<typename InputIt>
    List(
        InputIt first,
        InputIt last,
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIt>::value>::type* = nullptr
    );

    /// 从 initializer_list 构造 List。
    List(
        std::initializer_list<value_type> values,
        const allocator_type& allocator = allocator_type()
    );

    /// 使用 select_on_container_copy_construction 选择 allocator 后深拷贝节点。
    List(const List& other);

    /// 使用调用方指定的 allocator 深拷贝另一个 List。
    List(const List& other, const allocator_type& allocator);

    /// 移动构造并直接接管另一个 List 的节点链。
    List(List&& other) noexcept(
        std::is_nothrow_move_constructible<allocator_type>::value
    );

    /// 使用指定 allocator 移动构造；allocator 不同时逐个移动元素。
    List(List&& other, const allocator_type& allocator);

    /// 析构所有节点并通过对应 allocator 释放存储。
    ~List();

    /// 按 allocator propagation 规则执行拷贝赋值。
    List& operator=(const List& other);

    /// 按 allocator propagation 规则执行移动赋值。
    List& operator=(List&& other) noexcept(
        allocator_traits::propagate_on_container_move_assignment::value &&
        std::is_nothrow_move_assignable<allocator_type>::value
    );

    /// 使用 initializer_list 替换当前内容。
    List& operator=(std::initializer_list<value_type> values);

    /// 将内容替换为 count 个 value 副本。
    void assign(size_type count, const value_type& value);

    /// 将内容替换为迭代器区间 [first, last) 的元素。
    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
    assign(InputIt first, InputIt last);

    /// 将内容替换为 initializer_list 中的元素。
    void assign(std::initializer_list<value_type> values);

    /// 返回当前使用的 allocator 副本。
    allocator_type get_allocator() const noexcept;

    /// 返回首元素引用；空容器调用行为与 std::list 一样未定义。
    reference front() noexcept;

    /// 返回首元素只读引用；空容器调用行为与 std::list 一样未定义。
    const_reference front() const noexcept;

    /// 返回尾元素引用；空容器调用行为与 std::list 一样未定义。
    reference back() noexcept;

    /// 返回尾元素只读引用；空容器调用行为与 std::list 一样未定义。
    const_reference back() const noexcept;

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

    /// 返回当前节点数量，复杂度为 O(1)。
    size_type size() const noexcept;

    /// 返回节点 allocator 允许的最大节点数量。
    size_type max_size() const noexcept;

    /// 析构并释放所有业务节点，哨兵仍保持有效自环。
    void clear() noexcept;

    /// 在 position 前复制插入 value，并返回新节点迭代器。
    iterator insert(const_iterator position, const value_type& value);

    /// 在 position 前移动插入 value，并返回新节点迭代器。
    iterator insert(const_iterator position, value_type&& value);

    /// 在 position 前原地构造元素，并返回新节点迭代器。
    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args);

    /// 删除 position 节点，并返回其原后继迭代器。
    iterator erase(const_iterator position);

    /// 删除区间 [first, last)，并返回删除区间后的迭代器。
    iterator erase(const_iterator first, const_iterator last);

    /// 在链表尾部复制追加一个元素。
    void push_back(const value_type& value);

    /// 在链表尾部移动追加一个元素。
    void push_back(value_type&& value);

    /// 在链表尾部原地构造一个元素并返回其引用。
    template<typename... Args>
    reference emplace_back(Args&&... args);

    /// 在链表首部复制插入一个元素。
    void push_front(const value_type& value);

    /// 在链表首部移动插入一个元素。
    void push_front(value_type&& value);

    /// 在链表首部原地构造一个元素并返回其引用。
    template<typename... Args>
    reference emplace_front(Args&&... args);

    /// 删除尾元素；空容器调用行为与 std::list 一样未定义。
    void pop_back();

    /// 删除首元素；空容器调用行为与 std::list 一样未定义。
    void pop_front();

    /// 将逻辑大小调整为 count，新增元素使用值初始化。
    void resize(size_type count);

    /// 将逻辑大小调整为 count，新增元素复制 value。
    void resize(size_type count, const value_type& value);

    /// 将 other 的全部节点转移到 position 前；allocator 不同会显式抛错。
    void splice(const_iterator position, List& other);

    /// 将 other 中 element 指向的单节点转移到 position 前。
    void splice(const_iterator position, List& other, const_iterator element);

    /// 将 other 的 [first, last) 节点区间转移到 position 前。
    void splice(
        const_iterator position,
        List& other,
        const_iterator first,
        const_iterator last
    );

    /// 删除所有与 value 相等的元素。
    void remove(const value_type& value);

    /// 删除所有满足 predicate 的元素。
    template<typename Predicate>
    void remove_if(Predicate predicate);

    /// 删除相邻重复元素，保留每组的第一个节点。
    void unique();

    /// 使用 predicate 判断相邻等价关系并删除后续节点。
    template<typename BinaryPredicate>
    void unique(BinaryPredicate predicate);

    /// 将已排序 other 稳定合并到当前已排序链表。
    void merge(List& other);

    /// 使用 compare 将已排序 other 稳定合并到当前链表。
    template<typename Compare>
    void merge(List& other, Compare compare);

    /// 原地反转全部节点链接，不移动或复制 T。
    void reverse() noexcept;

    /// 使用稳定归并排序按 operator< 排序节点。
    void sort();

    /// 使用 compare 执行稳定归并排序；比较抛异常时保证节点不丢失。
    template<typename Compare>
    void sort(Compare compare);

    /// 按 allocator propagation 规则交换两个 List。
    void swap(List& other);

private:
    /// 建立空链表哨兵自环不变量。
    void resetSentinel() noexcept;

    /// 通过 rebind 后的节点 allocator 申请并构造节点。
    template<typename... Args>
    Node* createNode(Args&&... args);

    /// 通过构造该节点的同类 allocator 析构并释放节点。
    void destroyNode(Node* node) noexcept;

    /// 将 other 的完整节点链接管到当前空链表，并清空 other。
    void stealNodes(List& other) noexcept;

    /// 仅交换节点链和 size，不交换 allocator。
    void swapNodeChains(List& other) noexcept;

    /// 检查跨容器转移节点时两个 allocator 是否兼容。
    void ensureTransferCompatible(const List& other) const;

    /// allocator 允许传播时执行拷贝赋值。
    void copyAssign(const List& other, std::true_type);

    /// allocator 不允许传播时使用当前 allocator 执行拷贝赋值。
    void copyAssign(const List& other, std::false_type);

    /// allocator 允许传播时执行移动赋值。
    void moveAssign(List& other, std::true_type);

    /// allocator 不允许传播时根据 allocator 是否相等选择接管或逐个移动。
    void moveAssign(List& other, std::false_type);

    /// allocator 允许传播时交换 allocator 与节点链。
    void swapImpl(List& other, std::true_type);

    /// allocator 不允许传播时仅允许相等 allocator 交换节点链。
    void swapImpl(List& other, std::false_type);
};

/// 比较两个 List 的长度和逐项元素是否相等。
template<typename T, typename Allocator>
bool operator==(
    const List<T, Allocator>& left,
    const List<T, Allocator>& right
);

/// 判断两个 List 是否不相等。
template<typename T, typename Allocator>
bool operator!=(
    const List<T, Allocator>& left,
    const List<T, Allocator>& right
);

/// 调用成员 swap 交换两个 List。
template<typename T, typename Allocator>
void swap(List<T, Allocator>& left, List<T, Allocator>& right);

// -----------------------------------------------------------------------------
// 构造、析构与赋值
// -----------------------------------------------------------------------------

/// 初始化默认 allocator 和空哨兵。
template<typename T, typename Allocator>
List<T, Allocator>::List() noexcept(
    std::is_nothrow_default_constructible<allocator_type>::value
) : allocator_(), sentinel_(), size_(0) {
    resetSentinel();
}

/// 初始化指定 allocator 和空哨兵。
template<typename T, typename Allocator>
List<T, Allocator>::List(const allocator_type& allocator) noexcept(
    std::is_nothrow_copy_constructible<allocator_type>::value
)
    : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
}

/// 逐个值初始化 count 个节点；任一构造失败时回收已完成节点。
template<typename T, typename Allocator>
List<T, Allocator>::List(size_type count, const allocator_type& allocator)
    : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    try {
        while (count-- > 0)
            emplace_back();
    } catch (...) {
        clear();
        throw;
    }
}

/// 逐个复制构造 count 个 value 节点；失败时回收已完成节点。
template<typename T, typename Allocator>
List<T, Allocator>::List(
    size_type count,
    const value_type& value,
    const allocator_type& allocator
) : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    try {
        while (count-- > 0)
            push_back(value);
    } catch (...) {
        clear();
        throw;
    }
}

/// 单遍消费输入区间并构造节点；失败时回收已完成节点。
template<typename T, typename Allocator>
template<typename InputIt>
List<T, Allocator>::List(
    InputIt first,
    InputIt last,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIt>::value>::type*
) : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    try {
        while (first != last) {
            emplace_back(*first);
            ++first;
        }
    } catch (...) {
        clear();
        throw;
    }
}

/// 转发 initializer_list 区间完成构造。
template<typename T, typename Allocator>
List<T, Allocator>::List(
    std::initializer_list<value_type> values,
    const allocator_type& allocator
) : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    try {
        for (typename std::initializer_list<value_type>::const_iterator current = values.begin();
             current != values.end();
             ++current) {
            push_back(*current);
        }
    } catch (...) {
        clear();
        throw;
    }
}

/// 使用 allocator_traits 选择复制构造时的 allocator。
template<typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : allocator_(allocator_traits::select_on_container_copy_construction(other.allocator_)),
      sentinel_(),
      size_(0) {
    resetSentinel();
    try {
        for (const_iterator current = other.begin(); current != other.end(); ++current)
            push_back(*current);
    } catch (...) {
        clear();
        throw;
    }
}

/// 使用指定 allocator 深拷贝另一个 List。
template<typename T, typename Allocator>
List<T, Allocator>::List(const List& other, const allocator_type& allocator)
    : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    try {
        for (const_iterator current = other.begin(); current != other.end(); ++current)
            push_back(*current);
    } catch (...) {
        clear();
        throw;
    }
}

/// 移动 allocator 后常数时间接管节点链。
template<typename T, typename Allocator>
List<T, Allocator>::List(List&& other) noexcept(
    std::is_nothrow_move_constructible<allocator_type>::value
) : allocator_(std::move(other.allocator_)), sentinel_(), size_(0) {
    resetSentinel();
    stealNodes(other);
}

/// allocator 相等时接管节点，否则逐个移动构造并清空源容器。
template<typename T, typename Allocator>
List<T, Allocator>::List(List&& other, const allocator_type& allocator)
    : allocator_(allocator), sentinel_(), size_(0) {
    resetSentinel();
    if (allocator_ == other.allocator_) {
        stealNodes(other);
        return;
    }

    try {
        for (iterator current = other.begin(); current != other.end(); ++current)
            emplace_back(std::move(*current));
    } catch (...) {
        clear();
        throw;
    }
    other.clear();
}

/// 清空全部业务节点；嵌入式哨兵无需单独释放。
template<typename T, typename Allocator>
List<T, Allocator>::~List() {
    clear();
}

/// 根据 POCCA 规则选择 allocator 传播路径。
template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
    if (this != &other) {
        copyAssign(
            other,
            typename allocator_traits::propagate_on_container_copy_assignment()
        );
    }
    return *this;
}

/// 根据 POCMA 规则选择节点接管或逐个移动路径。
template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(List&& other) noexcept(
    allocator_traits::propagate_on_container_move_assignment::value &&
    std::is_nothrow_move_assignable<allocator_type>::value
) {
    if (this != &other) {
        moveAssign(
            other,
            typename allocator_traits::propagate_on_container_move_assignment()
        );
    }
    return *this;
}

/// 使用同 allocator 临时对象替换当前内容。
template<typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    std::initializer_list<value_type> values
) {
    assign(values);
    return *this;
}

/// 先完整构造临时链表，再交换节点链，提供强异常保证。
template<typename T, typename Allocator>
void List<T, Allocator>::assign(size_type count, const value_type& value) {
    List temporary(count, value, allocator_);
    swapNodeChains(temporary);
}

/// 先完整构造区间副本，再交换节点链，支持单遍输入迭代器。
template<typename T, typename Allocator>
template<typename InputIt>
typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
List<T, Allocator>::assign(InputIt first, InputIt last) {
    List temporary(first, last, allocator_);
    swapNodeChains(temporary);
}

/// 使用 initializer_list 区间替换当前内容。
template<typename T, typename Allocator>
void List<T, Allocator>::assign(std::initializer_list<value_type> values) {
    List temporary(values, allocator_);
    swapNodeChains(temporary);
}

/// 返回当前 value allocator 副本。
template<typename T, typename Allocator>
typename List<T, Allocator>::allocator_type
List<T, Allocator>::get_allocator() const noexcept {
    return allocator_;
}

// -----------------------------------------------------------------------------
// 元素访问与迭代器
// -----------------------------------------------------------------------------

/// 返回首节点保存的值。
template<typename T, typename Allocator>
typename List<T, Allocator>::reference
List<T, Allocator>::front() noexcept {
    return static_cast<Node*>(sentinel_.next)->value;
}

/// 返回首节点保存的只读值。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reference
List<T, Allocator>::front() const noexcept {
    return static_cast<const Node*>(sentinel_.next)->value;
}

/// 返回尾节点保存的值。
template<typename T, typename Allocator>
typename List<T, Allocator>::reference
List<T, Allocator>::back() noexcept {
    return static_cast<Node*>(sentinel_.previous)->value;
}

/// 返回尾节点保存的只读值。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reference
List<T, Allocator>::back() const noexcept {
    return static_cast<const Node*>(sentinel_.previous)->value;
}

/// 将首节点包装为可写迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::begin() noexcept {
    return iterator(sentinel_.next);
}

/// 将首节点包装为只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator
List<T, Allocator>::begin() const noexcept {
    return const_iterator(sentinel_.next);
}

/// 返回首节点只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator
List<T, Allocator>::cbegin() const noexcept {
    return begin();
}

/// 将哨兵包装为尾后迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::end() noexcept {
    return iterator(&sentinel_);
}

/// 将哨兵包装为尾后只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator
List<T, Allocator>::end() const noexcept {
    return const_iterator(const_cast<NodeBase*>(&sentinel_));
}

/// 返回尾后只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator
List<T, Allocator>::cend() const noexcept {
    return end();
}

/// 由 end 构造反向首迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator
List<T, Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

/// 由只读 end 构造反向首只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

/// 返回反向首只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const noexcept {
    return rbegin();
}

/// 由 begin 构造反向尾后迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator
List<T, Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

/// 由只读 begin 构造反向尾后只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

/// 返回反向尾后只读迭代器。
template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crend() const noexcept {
    return rend();
}

/// 判断 size 是否为零。
template<typename T, typename Allocator>
bool List<T, Allocator>::empty() const noexcept {
    return size_ == 0;
}

/// 返回维护的 O(1) 节点计数。
template<typename T, typename Allocator>
typename List<T, Allocator>::size_type
List<T, Allocator>::size() const noexcept {
    return size_;
}

/// 返回节点 allocator 与 difference_type 共同允许的上限。
template<typename T, typename Allocator>
typename List<T, Allocator>::size_type
List<T, Allocator>::max_size() const noexcept {
    node_allocator_type nodeAllocator(allocator_);
    const size_type allocatorMaximum = node_allocator_traits::max_size(nodeAllocator);
    const size_type differenceMaximum = static_cast<size_type>(
        (std::numeric_limits<difference_type>::max)()
    );
    return allocatorMaximum < differenceMaximum ? allocatorMaximum : differenceMaximum;
}

// -----------------------------------------------------------------------------
// 修改操作
// -----------------------------------------------------------------------------

/// 逐个通过共享删除流程销毁节点，最终恢复空哨兵自环。
template<typename T, typename Allocator>
void List<T, Allocator>::clear() noexcept {
    Storage storage(*this);
    while (sentinel_.next != &sentinel_)
        MutationAlgorithm::erase(storage, sentinel_.next);
    resetSentinel();
}

/// 复制插入一个新节点。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::insert(
    const_iterator position,
    const value_type& value
) {
    return emplace(position, value);
}

/// 移动插入一个新节点。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::insert(
    const_iterator position,
    value_type&& value
) {
    return emplace(position, std::move(value));
}

/// 通过共享 ListAlgorithm 完成“构造、链接、提交”流程。
template<typename T, typename Allocator>
template<typename... Args>
typename List<T, Allocator>::iterator
List<T, Allocator>::emplace(const_iterator position, Args&&... args) {
    if (size_ == max_size())
        throw std::length_error("List size exceeds max_size");
    Storage storage(*this);
    NodeBase* node = MutationAlgorithm::emplaceBefore(
        storage,
        position.node_,
        std::forward<Args>(args)...
    );
    return iterator(node);
}

/// 通过共享 ListAlgorithm 摘除、提交并销毁单个节点。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::erase(const_iterator position) {
    Storage storage(*this);
    return iterator(MutationAlgorithm::erase(storage, position.node_));
}

/// 逐个删除区间节点；链表中除被删节点外的迭代器保持有效。
template<typename T, typename Allocator>
typename List<T, Allocator>::iterator
List<T, Allocator>::erase(const_iterator first, const_iterator last) {
    iterator current(first.node_);
    const iterator finish(last.node_);
    while (current != finish)
        current = erase(current);
    return current;
}

/// 在尾后位置复制插入。
template<typename T, typename Allocator>
void List<T, Allocator>::push_back(const value_type& value) {
    insert(end(), value);
}

/// 在尾后位置移动插入。
template<typename T, typename Allocator>
void List<T, Allocator>::push_back(value_type&& value) {
    insert(end(), std::move(value));
}

/// 在尾后位置原地构造并返回新值引用。
template<typename T, typename Allocator>
template<typename... Args>
typename List<T, Allocator>::reference
List<T, Allocator>::emplace_back(Args&&... args) {
    return *emplace(end(), std::forward<Args>(args)...);
}

/// 在首节点前复制插入。
template<typename T, typename Allocator>
void List<T, Allocator>::push_front(const value_type& value) {
    insert(begin(), value);
}

/// 在首节点前移动插入。
template<typename T, typename Allocator>
void List<T, Allocator>::push_front(value_type&& value) {
    insert(begin(), std::move(value));
}

/// 在首节点前原地构造并返回新值引用。
template<typename T, typename Allocator>
template<typename... Args>
typename List<T, Allocator>::reference
List<T, Allocator>::emplace_front(Args&&... args) {
    return *emplace(begin(), std::forward<Args>(args)...);
}

/// 删除哨兵前的尾节点。
template<typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
    erase(const_iterator(sentinel_.previous));
}

/// 删除哨兵后的首节点。
template<typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
    erase(begin());
}

/// 缩小时从尾部删除，增长时值初始化新节点。
template<typename T, typename Allocator>
void List<T, Allocator>::resize(size_type count) {
    while (size_ > count)
        pop_back();
    while (size_ < count)
        emplace_back();
}

/// 缩小时从尾部删除，增长时复制 value。
template<typename T, typename Allocator>
void List<T, Allocator>::resize(size_type count, const value_type& value) {
    while (size_ > count)
        pop_back();
    while (size_ < count)
        push_back(value);
}

/// 常数时间转移 other 的完整节点链。
template<typename T, typename Allocator>
void List<T, Allocator>::splice(const_iterator position, List& other) {
    if (this == &other || other.empty())
        return;
    ensureTransferCompatible(other);
    splice(position, other, other.begin(), other.end());
}

/// 常数时间转移一个节点；self-splice 相邻位置由共享层安全忽略。
template<typename T, typename Allocator>
void List<T, Allocator>::splice(
    const_iterator position,
    List& other,
    const_iterator element
) {
    const_iterator after = element;
    ++after;
    splice(position, other, element, after);
}

/// 通过共享 ListAlgorithm 转移节点区间；跨容器时线性计算区间数量。
template<typename T, typename Allocator>
void List<T, Allocator>::splice(
    const_iterator position,
    List& other,
    const_iterator first,
    const_iterator last
) {
    if (first == last)
        return;
    if (this != &other)
        ensureTransferCompatible(other);

    Storage destination(*this);
    Storage source(other);
    MutationAlgorithm::spliceBefore(
        destination,
        position.node_,
        source,
        first.node_,
        last.node_
    );
}

/// 删除全部与 value 相等的节点。
template<typename T, typename Allocator>
void List<T, Allocator>::remove(const value_type& value) {
    remove_if([&value](const value_type& current) { return current == value; });
}

/// 在遍历过程中保存后继，再安全删除满足条件的当前节点。
template<typename T, typename Allocator>
template<typename Predicate>
void List<T, Allocator>::remove_if(Predicate predicate) {
    iterator current = begin();
    while (current != end()) {
        if (predicate(*current))
            current = erase(current);
        else
            ++current;
    }
}

/// 使用 operator== 删除相邻重复节点。
template<typename T, typename Allocator>
void List<T, Allocator>::unique() {
    unique(std::equal_to<value_type>());
}

/// 保留每组首节点，并删除所有与其相邻等价的后继节点。
template<typename T, typename Allocator>
template<typename BinaryPredicate>
void List<T, Allocator>::unique(BinaryPredicate predicate) {
    if (size_ < 2)
        return;

    iterator previous = begin();
    iterator current = previous;
    ++current;
    while (current != end()) {
        if (predicate(*previous, *current))
            current = erase(current);
        else {
            previous = current;
            ++current;
        }
    }
}

/// 使用 std::less 稳定合并两个已排序链表。
template<typename T, typename Allocator>
void List<T, Allocator>::merge(List& other) {
    merge(other, std::less<value_type>());
}

/// 仅转移节点完成稳定归并；compare 抛异常时两个链表仍各自有效。
template<typename T, typename Allocator>
template<typename Compare>
void List<T, Allocator>::merge(List& other, Compare compare) {
    if (this == &other || other.empty())
        return;
    ensureTransferCompatible(other);

    iterator left = begin();
    iterator right = other.begin();
    while (left != end() && right != other.end()) {
        if (compare(*right, *left)) {
            iterator moving = right;
            ++right;
            splice(left, other, moving);
        } else {
            ++left;
        }
    }
    if (right != other.end())
        splice(end(), other, right, other.end());
}

/// 交换每个链接节点的前后指针，包括哨兵自身。
template<typename T, typename Allocator>
void List<T, Allocator>::reverse() noexcept {
    NodeBase* current = &sentinel_;
    do {
        NodeBase* oldNext = current->next;
        current->next = current->previous;
        current->previous = oldNext;
        current = oldNext;
    } while (current != &sentinel_);
}

/// 使用 std::less 进入稳定归并排序。
template<typename T, typename Allocator>
void List<T, Allocator>::sort() {
    sort(std::less<value_type>());
}

/// 递归拆分节点链并稳定归并；异常路径将临时链表剩余节点接回当前容器。
template<typename T, typename Allocator>
template<typename Compare>
void List<T, Allocator>::sort(Compare compare) {
    if (size_ < 2)
        return;

    List second(allocator_);
    iterator middle = begin();
    const size_type half = size_ / 2;
    for (size_type index = 0; index < half; ++index)
        ++middle;
    second.splice(second.begin(), *this, middle, end());

    try {
        sort(compare);
        second.sort(compare);
        merge(second, compare);
    } catch (...) {
        splice(end(), second);
        throw;
    }
}

/// 根据 POCS 规则交换 allocator 或仅交换节点链。
template<typename T, typename Allocator>
void List<T, Allocator>::swap(List& other) {
    if (this == &other)
        return;
    swapImpl(other, typename allocator_traits::propagate_on_container_swap());
}

// -----------------------------------------------------------------------------
// 私有生命周期与 allocator 辅助
// -----------------------------------------------------------------------------

/// 重置哨兵为 previous/next 都指向自身的空状态。
template<typename T, typename Allocator>
void List<T, Allocator>::resetSentinel() noexcept {
    sentinel_.previous = &sentinel_;
    sentinel_.next = &sentinel_;
    size_ = 0;
}

/// 先申请单节点存储，再构造 Node；构造失败时立即释放。
template<typename T, typename Allocator>
template<typename... Args>
typename List<T, Allocator>::Node*
List<T, Allocator>::createNode(Args&&... args) {
    node_allocator_type nodeAllocator(allocator_);
    node_pointer allocation = node_allocator_traits::allocate(nodeAllocator, 1);
    Node* node = list_detail::toAddress(allocation);
    try {
        node_allocator_traits::construct(
            nodeAllocator,
            node,
            std::forward<Args>(args)...
        );
    } catch (...) {
        node_allocator_traits::deallocate(nodeAllocator, allocation, 1);
        throw;
    }
    return node;
}

/// 使用由当前 value allocator rebind 得到的节点 allocator 成对销毁和释放。
template<typename T, typename Allocator>
void List<T, Allocator>::destroyNode(Node* node) noexcept {
    node_allocator_type nodeAllocator(allocator_);
    node_pointer allocation = std::pointer_traits<node_pointer>::pointer_to(*node);
    node_allocator_traits::destroy(nodeAllocator, node);
    node_allocator_traits::deallocate(nodeAllocator, allocation, 1);
}

/// 将 other 链接改挂到当前哨兵，并把 other 恢复为空状态。
template<typename T, typename Allocator>
void List<T, Allocator>::stealNodes(List& other) noexcept {
    if (other.empty()) {
        resetSentinel();
        return;
    }

    sentinel_.next = other.sentinel_.next;
    sentinel_.previous = other.sentinel_.previous;
    sentinel_.next->previous = &sentinel_;
    sentinel_.previous->next = &sentinel_;
    size_ = other.size_;
    other.resetSentinel();
}

/// 交换两条完整节点链，并分别修正新的哨兵边界链接。
template<typename T, typename Allocator>
void List<T, Allocator>::swapNodeChains(List& other) noexcept {
    const bool thisEmpty = empty();
    const bool otherEmpty = other.empty();
    NodeBase* thisFirst = sentinel_.next;
    NodeBase* thisLast = sentinel_.previous;
    NodeBase* otherFirst = other.sentinel_.next;
    NodeBase* otherLast = other.sentinel_.previous;

    if (otherEmpty) {
        sentinel_.next = &sentinel_;
        sentinel_.previous = &sentinel_;
    } else {
        sentinel_.next = otherFirst;
        sentinel_.previous = otherLast;
        otherFirst->previous = &sentinel_;
        otherLast->next = &sentinel_;
    }

    if (thisEmpty) {
        other.sentinel_.next = &other.sentinel_;
        other.sentinel_.previous = &other.sentinel_;
    } else {
        other.sentinel_.next = thisFirst;
        other.sentinel_.previous = thisLast;
        thisFirst->previous = &other.sentinel_;
        thisLast->next = &other.sentinel_;
    }

    using std::swap;
    swap(size_, other.size_);
}

/// 不同 allocator 不能互相接管节点，否则未来释放会使用错误 allocator。
template<typename T, typename Allocator>
void List<T, Allocator>::ensureTransferCompatible(const List& other) const {
    if (allocator_ != other.allocator_)
        throw std::logic_error("Cannot transfer List nodes with unequal allocators");
}

/// allocator 传播时，先用目标 allocator 完整复制，再切换 allocator 和节点所有权。
template<typename T, typename Allocator>
void List<T, Allocator>::copyAssign(const List& other, std::true_type) {
    if (allocator_ == other.allocator_) {
        List temporary(other, allocator_);
        swapNodeChains(temporary);
        return;
    }

    allocator_type replacementAllocator(other.allocator_);
    List temporary(other, replacementAllocator);
    clear();
    allocator_ = replacementAllocator;
    stealNodes(temporary);
}

/// allocator 不传播时，始终使用当前 allocator 构造强异常保证的临时副本。
template<typename T, typename Allocator>
void List<T, Allocator>::copyAssign(const List& other, std::false_type) {
    List temporary(other, allocator_);
    swapNodeChains(temporary);
}

/// allocator 传播时清理旧节点、移动 allocator 并常数时间接管节点链。
template<typename T, typename Allocator>
void List<T, Allocator>::moveAssign(List& other, std::true_type) {
    clear();
    allocator_ = std::move(other.allocator_);
    stealNodes(other);
}

/// allocator 不传播时，相等 allocator 可接管；否则逐个移动构造到当前 allocator。
template<typename T, typename Allocator>
void List<T, Allocator>::moveAssign(List& other, std::false_type) {
    if (allocator_ == other.allocator_) {
        clear();
        stealNodes(other);
        return;
    }

    List temporary(
        std::make_move_iterator(other.begin()),
        std::make_move_iterator(other.end()),
        allocator_
    );
    swapNodeChains(temporary);
    other.clear();
}

/// allocator 允许传播时同时交换 allocator 和节点链。
template<typename T, typename Allocator>
void List<T, Allocator>::swapImpl(List& other, std::true_type) {
    using std::swap;
    swap(allocator_, other.allocator_);
    swapNodeChains(other);
}

/// allocator 不传播时拒绝不同 allocator 的节点所有权交换。
template<typename T, typename Allocator>
void List<T, Allocator>::swapImpl(List& other, std::false_type) {
    if (allocator_ != other.allocator_)
        throw std::logic_error("Cannot swap List instances with unequal allocators");
    swapNodeChains(other);
}

// -----------------------------------------------------------------------------
// 非成员操作
// -----------------------------------------------------------------------------

/// 逐项比较两个链表的值；节点地址和 allocator 不参与比较。
template<typename T, typename Allocator>
bool operator==(
    const List<T, Allocator>& left,
    const List<T, Allocator>& right
) {
    if (left.size() != right.size())
        return false;

    typename List<T, Allocator>::const_iterator leftIt = left.begin();
    typename List<T, Allocator>::const_iterator rightIt = right.begin();
    while (leftIt != left.end()) {
        if (!(*leftIt == *rightIt))
            return false;
        ++leftIt;
        ++rightIt;
    }
    return true;
}

/// 复用 operator== 判断不等。
template<typename T, typename Allocator>
bool operator!=(
    const List<T, Allocator>& left,
    const List<T, Allocator>& right
) {
    return !(left == right);
}

/// 转发到成员 swap，保持 allocator 规则一致。
template<typename T, typename Allocator>
void swap(List<T, Allocator>& left, List<T, Allocator>& right) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
