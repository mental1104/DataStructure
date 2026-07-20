#ifndef __DSA_LIST
#define __DSA_LIST

#include <cstddef>
#include <iterator>
#include <utility>

#include "ListNode.h"
#include "utils.h"
#include "dsa/algorithm/Sequence.h"
#include "dsa/core/list/ListAlgorithm.h"

template<typename T>
class List {
private:
    int _size;
    ListNode<T>* header;
    ListNode<T>* trailer;

    /// 将教学版 new/delete 节点模型适配到共享 ListAlgorithm。
    class TeachingStorage {
    public:
        typedef ListNode<T> node_type;
        typedef int size_type;

        /// 绑定当前要修改的教学 List。
        explicit TeachingStorage(List& owner) noexcept
            : owner_(owner) {
        }

        /// 构造一个尚未接入链表的教学节点。
        template<typename Value>
        node_type* createNode(Value&& value) {
            return new node_type(std::forward<Value>(value));
        }

        /// 销毁已经从链中摘除的节点。
        void destroyNode(node_type* node) noexcept {
            delete node;
        }

        /// 将脱链节点接到 position 前，不直接修改 _size。
        void linkBefore(node_type* position, node_type* node) noexcept {
            node_type* before = position->pred;
            node->pred = before;
            node->succ = position;
            before->succ = node;
            position->pred = node;
        }

        /// 从双向链摘除节点，不直接修改 _size。
        void unlink(node_type* node) noexcept {
            node->pred->succ = node->succ;
            node->succ->pred = node->pred;
            node->pred = nullptr;
            node->succ = nullptr;
        }

        /// 返回节点后继，供共享删除流程返回。
        node_type* next(node_type* node) const noexcept {
            return node->succ;
        }

        /// 计算 [first, last) 的节点数量，满足共享 splice contract。
        size_type countRange(node_type* first, node_type* last) const {
            size_type count = 0;
            while (first != last) {
                ++count;
                first = first->succ;
            }
            return count;
        }

        /// 判断两个 adapter 是否绑定同一个教学 List。
        bool sameOwner(const TeachingStorage& other) const noexcept {
            return &owner_ == &other.owner_;
        }

        /// 判断 target 是否位于 [first, last) 内。
        bool contains(
            node_type* first,
            node_type* last,
            node_type* target
        ) const {
            while (first != last) {
                if (first == target)
                    return true;
                first = first->succ;
            }
            return false;
        }

        /// 整体转移 [first, last) 节点区间，教学版当前不公开 splice 但保留完整 contract。
        void transferBefore(
            node_type* position,
            node_type* first,
            node_type* last
        ) noexcept {
            node_type* beforeFirst = first->pred;
            node_type* lastMoved = last->pred;
            node_type* beforePosition = position->pred;

            beforeFirst->succ = last;
            last->pred = beforeFirst;
            beforePosition->succ = first;
            first->pred = beforePosition;
            lastMoved->succ = position;
            position->pred = lastMoved;
        }

        /// 提交插入后的教学 size。
        void commitInsert(size_type count) noexcept {
            owner_._size += count;
        }

        /// 提交删除后的教学 size。
        void commitErase(size_type count) noexcept {
            owner_._size -= count;
        }

    private:
        List& owner_;
    };

    typedef dsa::core::ListAlgorithm<TeachingStorage> MutationAlgorithm;

    /// 仅交换教学版哨兵所有权和 size，用于 copy-and-swap 赋值。
    void swapStorage(List& other) noexcept;

protected:
    void init();
    int clear();
    void copyNodes(ListNode<T>*, int);

public:
    List();
    List(List<T> const& L);
    List(List<T> const& L, Rank r, int n);
    List(ListNode<T>* p, int n);
    List<T>& operator=(List<T> const& L);
    ~List();

    Rank size() const;
    bool empty() const;
    T& operator[](Rank r) const;

    ListNode<T>* first() const;
    ListNode<T>* last() const;

    struct iterator;
    iterator begin();
    iterator end();
    const iterator begin() const;
    const iterator end() const;

    bool valid(ListNode<T>* p);
    int disordered() const;
    ListNode<T>* find(T const& e) const;
    ListNode<T>* find(T const& e, int n, ListNode<T>* p) const;
    ListNode<T>* search(T const& e) const;
    ListNode<T>* search(T const& e, int n, ListNode<T>* p) const;

    ListNode<T>* insertAsFirst(T const& e);
    ListNode<T>* insertAsLast(T const& e);
    ListNode<T>* insert(T const& e);
    ListNode<T>* insertA(ListNode<T>* p, T const& e);
    ListNode<T>* insertB(ListNode<T>* p, T const& e);
    T remove(ListNode<T>* p);

    int deduplicate();
    int uniquify();
    void reverse();

    void traverse(void(*)(T&));
    template<typename VST>
    void traverse(VST&&);
};

/// 初始化一对教学哨兵；第二次分配失败时释放第一哨兵。
template<typename T>
void List<T>::init() {
    header = new ListNode<T>;
    try {
        trailer = new ListNode<T>;
    } catch (...) {
        delete header;
        header = nullptr;
        throw;
    }
    header->succ = trailer;
    header->pred = nullptr;
    trailer->pred = header;
    trailer->succ = nullptr;
    _size = 0;
}

/// 构造空教学 List。
template<typename T>
List<T>::List() {
    init();
}

/// 线性定位第 r 个元素并返回其可写引用，保留原有 const 签名兼容性。
template<typename T>
T& List<T>::operator[](Rank r) const {
    ListNode<T>* node = first();
    while (0 < r--)
        node = node->succ;
    return node->data;
}

/// 从 p 向前检查最多 n 个节点，返回最后一个匹配节点。
template<typename T>
ListNode<T>* List<T>::find(T const& e, int n, ListNode<T>* p) const {
    while (0 < n--) {
        p = p->pred;
        if (e == p->data)
            return p;
    }
    return nullptr;
}

/// 通过共享 mutation workflow 在首节点前复制插入。
template<typename T>
ListNode<T>* List<T>::insertAsFirst(T const& e) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::emplaceBefore(storage, header->succ, e);
}

/// 通过共享 mutation workflow 在尾哨兵前复制插入。
template<typename T>
ListNode<T>* List<T>::insertAsLast(T const& e) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::emplaceBefore(storage, trailer, e);
}

/// 保留教学版 insert 默认尾插行为。
template<typename T>
ListNode<T>* List<T>::insert(T const& e) {
    return insertAsLast(e);
}

/// 通过共享 mutation workflow 在 p 后插入。
template<typename T>
ListNode<T>* List<T>::insertA(ListNode<T>* p, T const& e) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::emplaceBefore(storage, p->succ, e);
}

/// 通过共享 mutation workflow 在 p 前插入。
template<typename T>
ListNode<T>* List<T>::insertB(ListNode<T>* p, T const& e) {
    TeachingStorage storage(*this);
    return MutationAlgorithm::emplaceBefore(storage, p, e);
}

/// 初始化哨兵后复制 n 个连续节点；复制失败时清理完整的局部构造状态。
template<typename T>
void List<T>::copyNodes(ListNode<T>* p, int n) {
    init();
    try {
        while (n-- > 0) {
            insertAsLast(p->data);
            p = p->succ;
        }
    } catch (...) {
        clear();
        delete header;
        delete trailer;
        header = nullptr;
        trailer = nullptr;
        throw;
    }
}

/// 从节点指针开始复制 n 个节点。
template<typename T>
List<T>::List(ListNode<T>* p, int n) {
    copyNodes(p, n);
}

/// 深拷贝完整教学 List。
template<typename T>
List<T>::List(List<T> const& L) {
    copyNodes(L.first(), L._size);
}

/// 从第 r 个节点开始复制 n 个节点，修复旧实现将元素引用误传为节点指针的问题。
template<typename T>
List<T>::List(List<T> const& L, Rank r, int n) {
    ListNode<T>* node = L.first();
    while (r-- > 0)
        node = node->succ;
    copyNodes(node, n);
}

/// 使用 copy-and-swap 保持自赋值安全，并在复制失败时保留原链表。
template<typename T>
List<T>& List<T>::operator=(List<T> const& L) {
    if (this == &L)
        return *this;
    List<T> temporary(L);
    swapStorage(temporary);
    return *this;
}

/// 先复制待返回值，再通过共享流程摘除并销毁节点；复制失败时链表不变。
template<typename T>
T List<T>::remove(ListNode<T>* p) {
    T value = p->data;
    TeachingStorage storage(*this);
    MutationAlgorithm::erase(storage, p);
    return value;
}

/// 清理业务节点后释放两个教学哨兵。
template<typename T>
List<T>::~List() {
    clear();
    delete header;
    delete trailer;
}

/// 通过共享删除流程清空业务节点并返回原 size。
template<typename T>
int List<T>::clear() {
    const int oldSize = _size;
    TeachingStorage storage(*this);
    while (_size > 0)
        MutationAlgorithm::erase(storage, header->succ);
    return oldSize;
}

/// 保留教学版“删除前方重复节点、保留最后出现节点”的去重语义。
template<typename T>
int List<T>::deduplicate() {
    if (_size < 2)
        return 0;
    const int oldSize = _size;
    ListNode<T>* node = header;
    Rank rank = 0;
    while (trailer != (node = node->succ)) {
        ListNode<T>* duplicate = find(node->data, rank, node);
        duplicate ? remove(duplicate) : ++rank;
    }
    return oldSize - _size;
}

/// 使用 iterator-first forEach 执行函数指针访问。
template<typename T>
void List<T>::traverse(void (*visit)(T&)) {
    dsa::algorithm::forEach(begin(), end(), visit);
}

/// 使用 iterator-first forEach 执行任意访问器。
template<typename T>
template<typename VST>
void List<T>::traverse(VST&& visit) {
    dsa::algorithm::forEach(begin(), end(), std::forward<VST>(visit));
}

/// 删除有序链表中的相邻重复节点，保留每组首节点。
template<typename T>
int List<T>::uniquify() {
    if (_size < 2)
        return 0;
    const int oldSize = _size;
    ListNode<T>* node = first();
    ListNode<T>* nextNode = nullptr;
    while (trailer != (nextNode = node->succ)) {
        if (node->data != nextNode->data)
            node = nextNode;
        else
            remove(nextNode);
    }
    return oldSize - _size;
}

/// 在有序前缀中向前查找不大于 e 的最后节点。
template<typename T>
ListNode<T>* List<T>::search(T const& e, int n, ListNode<T>* p) const {
    while (0 <= n--) {
        p = p->pred;
        if (p->data <= e)
            break;
    }
    return p;
}

/// 逐节点交换前后链接，再交换两个哨兵指针，复杂度 O(n)。
template<typename T>
void List<T>::reverse() {
    ListNode<T>* current = header;
    while (current != nullptr) {
        ListNode<T>* oldSuccessor = current->succ;
        current->succ = current->pred;
        current->pred = oldSuccessor;
        current = oldSuccessor;
    }
    ListNode<T>* oldHeader = header;
    header = trailer;
    trailer = oldHeader;
}

/// 使用 iterator-first disorderCount 统计相邻逆序对数量。
template<typename T>
int List<T>::disordered() const {
    return static_cast<int>(dsa::algorithm::disorderCount(begin(), end()));
}

/// 使用限定 std::swap 交换哨兵所有权和 size，避免全局 swap 参与 ADL 造成二义性。
template<typename T>
void List<T>::swapStorage(List& other) noexcept {
    std::swap(_size, other._size);
    std::swap(header, other.header);
    std::swap(trailer, other.trailer);
}

/// 返回教学版元素数量。
template<typename T>
Rank List<T>::size() const {
    return _size;
}

/// 判断教学版链表是否为空。
template<typename T>
bool List<T>::empty() const {
    return _size <= 0;
}

/// 返回首业务节点；空链表时返回 trailer。
template<typename T>
ListNode<T>* List<T>::first() const {
    return header->succ;
}

/// 返回尾业务节点；空链表时返回 header。
template<typename T>
ListNode<T>* List<T>::last() const {
    return trailer->pred;
}

/// 判断节点是否为当前链表中的非哨兵节点。
template<typename T>
bool List<T>::valid(ListNode<T>* p) {
    return p && trailer != p && header != p;
}

/// 在完整链表中查找最后一个等于 e 的节点。
template<typename T>
ListNode<T>* List<T>::find(T const& e) const {
    return find(e, _size, trailer);
}

/// 在完整有序链表中查找不大于 e 的最后节点。
template<typename T>
ListNode<T>* List<T>::search(T const& e) const {
    return search(e, _size, trailer);
}

/// 教学版双向迭代器，补齐标准 iterator_traits 所需类型信息。
template<typename T>
struct List<T>::iterator {
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    ListNode<T>* cur;

    explicit iterator(ListNode<T>* rhs = nullptr);
    bool operator!=(const iterator& other) const;
    bool operator==(const iterator& other) const;
    T& operator*() const;
    T* operator->() const;
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
};

/// 包装教学节点指针。
template<typename T>
List<T>::iterator::iterator(ListNode<T>* rhs)
    : cur(rhs) {
}

/// 比较节点位置是否不同。
template<typename T>
bool List<T>::iterator::operator!=(const iterator& other) const {
    return cur != other.cur;
}

/// 比较节点位置是否相同。
template<typename T>
bool List<T>::iterator::operator==(const iterator& other) const {
    return cur == other.cur;
}

/// 返回当前教学节点数据引用。
template<typename T>
T& List<T>::iterator::operator*() const {
    return cur->data;
}

/// 返回当前教学节点数据地址。
template<typename T>
T* List<T>::iterator::operator->() const {
    return &cur->data;
}

/// 移动到后继节点。
template<typename T>
typename List<T>::iterator& List<T>::iterator::operator++() {
    cur = cur->succ;
    return *this;
}

/// 后置移动到后继节点。
template<typename T>
typename List<T>::iterator List<T>::iterator::operator++(int) {
    iterator old(*this);
    ++(*this);
    return old;
}

/// 移动到前驱节点。
template<typename T>
typename List<T>::iterator& List<T>::iterator::operator--() {
    cur = cur->pred;
    return *this;
}

/// 后置移动到前驱节点。
template<typename T>
typename List<T>::iterator List<T>::iterator::operator--(int) {
    iterator old(*this);
    --(*this);
    return old;
}

/// 返回首业务节点迭代器。
template<typename T>
typename List<T>::iterator List<T>::begin() {
    return iterator(header->succ);
}

/// 返回尾哨兵迭代器。
template<typename T>
typename List<T>::iterator List<T>::end() {
    return iterator(trailer);
}

/// 保留旧 const begin 返回 iterator 的签名，避免破坏教学调用方。
template<typename T>
const typename List<T>::iterator List<T>::begin() const {
    return iterator(header->succ);
}

/// 保留旧 const end 返回 iterator 的签名，避免破坏教学调用方。
template<typename T>
const typename List<T>::iterator List<T>::end() const {
    return iterator(trailer);
}

#endif
