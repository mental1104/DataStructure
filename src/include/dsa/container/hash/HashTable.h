#ifndef DSA_CONTAINER_HASH_HASH_TABLE_H
#define DSA_CONTAINER_HASH_HASH_TABLE_H

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../../core/hash/HashTableAlgorithm.h"

namespace dsa {
namespace container {

/// allocator-aware 的开放寻址键值容器，核心接口参考 std::unordered_map。
///
/// 每个元素由 value allocator 独立构造，桶数组只保存节点指针与三态标记。
/// 因此 rehash 只迁移节点指针，成功前不改变旧表，并保持元素引用与指针有效。
template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >,
    typename ProbePolicy = dsa::core::LinearProbing
>
class HashTable {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

private:
    enum BucketState {
        BUCKET_EMPTY,
        BUCKET_OCCUPIED,
        BUCKET_DELETED
    };

    struct Bucket {
        pointer node;
        BucketState state;

        /// 构造一个从未使用过的空桶。
        Bucket();
    };

    typedef typename allocator_traits::template rebind_alloc<Bucket> bucket_allocator_type;
    typedef std::allocator_traits<bucket_allocator_type> bucket_allocator_traits;

    static_assert(
        std::is_same<typename allocator_traits::pointer, pointer>::value,
        "HashTable currently requires an allocator with raw value_type pointers"
    );
    static_assert(
        std::is_same<typename bucket_allocator_traits::pointer, Bucket*>::value,
        "HashTable currently requires an allocator with raw rebound pointers"
    );

public:
    template<bool IsConst>
    class BasicIterator {
    private:
        typedef typename std::conditional<IsConst, const HashTable, HashTable>::type owner_type;
        owner_type* owner_;
        size_type index_;

        /// 构造指向指定桶下标的内部迭代器，并跳过非占用桶。
        BasicIterator(owner_type* owner, size_type index);

        /// 前进到下一个占用桶或尾后位置。
        void skipToOccupied();

        template<bool>
        friend class BasicIterator;
        friend class HashTable;

    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename HashTable::value_type value_type;
        typedef typename HashTable::difference_type difference_type;
        typedef typename std::conditional<IsConst, const value_type*, value_type*>::type pointer;
        typedef typename std::conditional<IsConst, const value_type&, value_type&>::type reference;

        /// 构造无归属的尾后样式迭代器。
        BasicIterator();

        /// 将可写迭代器转换为只读迭代器。
        template<bool Enabled = IsConst>
        BasicIterator(
            const BasicIterator<false>& other,
            typename std::enable_if<Enabled, int>::type* = NULL
        );

        /// 返回当前键值对引用。
        reference operator*() const;

        /// 返回当前键值对地址。
        pointer operator->() const;

        /// 前置递增到下一个元素。
        BasicIterator& operator++();

        /// 后置递增并返回递增前副本。
        BasicIterator operator++(int);

        /// 比较两个 const/非 const 迭代器是否指向同一位置。
        template<bool OtherConst>
        bool operator==(const BasicIterator<OtherConst>& other) const;

        /// 比较两个 const/非 const 迭代器是否指向不同位置。
        template<bool OtherConst>
        bool operator!=(const BasicIterator<OtherConst>& other) const;
    };

    typedef BasicIterator<false> iterator;
    typedef BasicIterator<true> const_iterator;

private:
    allocator_type allocator_;
    hasher hash_;
    key_equal equal_;
    Bucket* buckets_;
    size_type bucketCount_;
    size_type size_;
    size_type tombstoneCount_;
    float maxLoadFactor_;

    /// 只读适配器：向共享算法暴露桶状态、哈希器和键比较器。
    class LookupStorage {
    public:
        typedef typename HashTable::size_type size_type;

        /// 绑定只读 HashTable。
        explicit LookupStorage(const HashTable& owner);

        /// 返回桶数量。
        size_type bucketCount() const;

        /// 判断指定桶是否占用。
        bool isOccupied(size_type index) const;

        /// 判断指定桶是否为墓碑。
        bool isDeleted(size_type index) const;

        /// 使用容器保存的 hasher 计算键散列值。
        template<typename LookupKey>
        std::size_t hashKey(const LookupKey& key) const;

        /// 使用容器保存的 key_equal 比较键。
        template<typename LookupKey>
        bool keysEqual(size_type index, const LookupKey& key) const;

    private:
        const HashTable& owner_;
    };

    /// 可变适配器：在 LookupStorage 语义上增加删除、扩容与 rehash 事务。
    class Storage {
    public:
        typedef typename HashTable::size_type size_type;

        /// 绑定可变 HashTable，并初始化空的临时桶数组。
        explicit Storage(HashTable& owner);

        /// 放弃尚未提交的临时桶数组。
        ~Storage();

        /// 返回桶数量。
        size_type bucketCount() const;

        /// 判断指定桶是否占用。
        bool isOccupied(size_type index) const;

        /// 判断指定桶是否为墓碑。
        bool isDeleted(size_type index) const;

        /// 使用容器保存的 hasher 计算键散列值。
        template<typename LookupKey>
        std::size_t hashKey(const LookupKey& key) const;

        /// 使用容器保存的 key_equal 比较键。
        template<typename LookupKey>
        bool keysEqual(size_type index, const LookupKey& key) const;

        /// 判断插入前是否需要扩容或清理过多墓碑。
        bool needsRehashForInsert() const;

        /// 返回扩容或清墓碑所需的目标桶数量。
        size_type growthBucketCount() const;

        /// 删除指定桶中的节点并提交墓碑状态。
        void eraseAt(size_type index);

        /// 分配尚未提交的新桶数组。
        void beginRehash(size_type bucketCount);

        /// 将旧桶节点指针放入临时桶数组，不移动或复制 value_type。
        void transferBucketToPending(size_type oldIndex);

        /// 提交新桶数组并释放旧桶数组，元素节点保持原地址。
        void commitRehash() noexcept;

        /// 放弃临时桶数组，旧表保持不变。
        void rollbackRehash() noexcept;

    private:
        HashTable& owner_;
        Bucket* pendingBuckets_;
        size_type pendingBucketCount_;

        Storage(const Storage&);
        Storage& operator=(const Storage&);
    };

    typedef dsa::core::HashTableAlgorithm<Storage, ProbePolicy> MutationAlgorithm;
    typedef dsa::core::HashTableAlgorithm<LookupStorage, ProbePolicy> LookupAlgorithm;
    typedef typename MutationAlgorithm::probe_result probe_result;

    /// 分配并默认构造指定数量的桶。
    Bucket* allocateBucketArray(size_type bucketCount);

    /// 析构并释放桶数组，但不析构其中指向的元素节点。
    void deallocateBucketArray(Bucket* buckets, size_type bucketCount) noexcept;

    /// 使用 value allocator 分配并构造一个元素节点。
    template<typename... Args>
    pointer createNode(Args&&... args);

    /// 析构并释放一个元素节点。
    void destroyNode(pointer node) noexcept;

    /// 初始化规范化后的空桶数组。
    void initializeBuckets(size_type requestedBucketCount);

    /// 析构全部元素节点并释放桶数组。
    void destroyStorage() noexcept;

    /// 接管 another 的桶数组，并将 another 置为合法空状态。
    void stealStorage(HashTable& another) noexcept;

    /// 交换除 allocator 外的全部容器状态。
    void swapContent(HashTable& other);

    /// 交换 allocator 与全部容器状态，使存储始终由匹配 allocator 管理。
    void swapAll(HashTable& other);

    /// 根据元素数量和 max_load_factor 计算最少桶数。
    size_type requiredBucketsForElements(size_type elementCount) const;

    /// 将已构造节点提交到指定桶。
    void commitNode(size_type index, pointer node) noexcept;

    /// 插入一个现成 value_type，并返回迭代器与是否成功。
    template<typename Value>
    std::pair<iterator, bool> insertValue(Value&& value);

    /// 以独立键和值构造节点，避免 operator[] 对 mapped_type 的额外复制。
    template<typename KeyArg, typename MappedArg>
    std::pair<iterator, bool> insertKeyValue(KeyArg&& key, MappedArg&& mapped);

    /// 将另一个容器中的元素深拷贝到当前空表。
    void copyElementsFrom(const HashTable& other);

public:
    /// 构造默认桶数、默认 hasher/key_equal/allocator 的空表。
    HashTable();

    /// 使用指定 allocator 构造空表。
    explicit HashTable(const allocator_type& allocator);

    /// 使用指定桶数、hasher、key_equal 和 allocator 构造空表。
    explicit HashTable(
        size_type bucketCount,
        const hasher& hash = hasher(),
        const key_equal& equal = key_equal(),
        const allocator_type& allocator = allocator_type()
    );

    /// 从迭代器区间构造哈希表。
    template<typename InputIt>
    HashTable(
        InputIt first,
        InputIt last,
        size_type bucketCount = 3,
        const hasher& hash = hasher(),
        const key_equal& equal = key_equal(),
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIt>::value>::type* = NULL
    );

    /// 从 initializer_list 构造哈希表。
    HashTable(
        std::initializer_list<value_type> values,
        size_type bucketCount = 3,
        const hasher& hash = hasher(),
        const key_equal& equal = key_equal(),
        const allocator_type& allocator = allocator_type()
    );

    /// 使用 select_on_container_copy_construction 选择 allocator 后深拷贝。
    HashTable(const HashTable& other);

    /// 使用调用方指定的 allocator 深拷贝。
    HashTable(const HashTable& other, const allocator_type& allocator);

    /// 移动构造并接管桶数组与节点所有权。
    HashTable(HashTable&& other) noexcept(
        std::is_nothrow_move_constructible<allocator_type>::value &&
        std::is_nothrow_move_constructible<hasher>::value &&
        std::is_nothrow_move_constructible<key_equal>::value
    );

    /// 使用指定 allocator 移动构造；allocator 不同时逐元素迁移。
    HashTable(HashTable&& other, const allocator_type& allocator);

    /// 析构全部元素节点和桶数组。
    ~HashTable();

    /// 按 allocator propagation 规则执行深拷贝赋值。
    HashTable& operator=(const HashTable& other);

    /// 按 allocator propagation 规则执行移动赋值。
    HashTable& operator=(HashTable&& other);

    /// 使用 initializer_list 替换当前内容。
    HashTable& operator=(std::initializer_list<value_type> values);

    /// 返回 allocator 副本。
    allocator_type get_allocator() const;

    /// 返回首元素迭代器。
    iterator begin();

    /// 返回首元素只读迭代器。
    const_iterator begin() const;

    /// 返回首元素只读迭代器。
    const_iterator cbegin() const;

    /// 返回尾后迭代器。
    iterator end();

    /// 返回尾后只读迭代器。
    const_iterator end() const;

    /// 返回尾后只读迭代器。
    const_iterator cend() const;

    /// 判断容器是否为空。
    bool empty() const;

    /// 返回元素数量。
    size_type size() const;

    /// 返回 allocator 理论允许的最大元素数量。
    size_type max_size() const;

    /// 清空全部元素并消除墓碑，但保留桶容量。
    void clear() noexcept;

    /// 插入 value 副本；重复键不覆盖原值。
    std::pair<iterator, bool> insert(const value_type& value);

    /// 移动插入 value；重复键不覆盖原值。
    std::pair<iterator, bool> insert(value_type&& value);

    /// 插入迭代器区间中的全部键值对。
    template<typename InputIt>
    typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
    insert(InputIt first, InputIt last);

    /// 插入 initializer_list 中的全部键值对。
    void insert(std::initializer_list<value_type> values);

    /// 原地构造 value_type；重复键时销毁临时节点并返回已有元素。
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args);

    /// 键不存在时插入，存在时赋值 mapped value。
    template<typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& value);

    /// 删除迭代器所指元素，并返回其后的迭代器。
    iterator erase(const_iterator position);

    /// 删除指定键并返回删除数量 0 或 1。
    size_type erase(const key_type& key);

    /// 交换两张表；非传播且 allocator 不等时拒绝交换所有权。
    void swap(HashTable& other);

    /// 返回键对应 mapped value；不存在时抛出 out_of_range。
    mapped_type& at(const key_type& key);

    /// 返回键对应只读 mapped value；不存在时抛出 out_of_range。
    const mapped_type& at(const key_type& key) const;

    /// 返回键对应 mapped value，不存在时默认构造并插入。
    mapped_type& operator[](const key_type& key);

    /// 返回键对应 mapped value，不存在时移动键并默认构造插入。
    mapped_type& operator[](key_type&& key);

    /// 查找键并返回迭代器。
    iterator find(const key_type& key);

    /// 查找键并返回只读迭代器。
    const_iterator find(const key_type& key) const;

    /// 返回键是否存在对应的 0 或 1。
    size_type count(const key_type& key) const;

    /// 非标准 C++11 便利接口：判断键是否存在。
    bool contains(const key_type& key) const;

    /// 返回当前桶数量。
    size_type bucket_count() const;

    /// 返回键的初始哈希桶下标；开放寻址后实际元素可能位于后续桶。
    size_type bucket(const key_type& key) const;

    /// 返回当前装填因子 size / bucket_count。
    float load_factor() const;

    /// 返回最大装填因子。
    float max_load_factor() const;

    /// 设置最大装填因子，并在当前容量不足时立即 rehash。
    void max_load_factor(float value);

    /// 将桶数调整到 requested 与当前元素最低需求中的较大者，并清理墓碑。
    void rehash(size_type requestedBucketCount);

    /// 为至少 elementCount 个元素预留容量。
    void reserve(size_type elementCount);

    /// 返回 hasher 副本。
    hasher hash_function() const;

    /// 返回 key_equal 副本。
    key_equal key_eq() const;
};

// ===== Bucket 与迭代器实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Bucket::Bucket()
    : node(NULL), state(BUCKET_EMPTY) {}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::BasicIterator(
    owner_type* owner,
    size_type index
)
    : owner_(owner), index_(index) {
    skipToOccupied();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::skipToOccupied() {
    if (owner_ == NULL)
        return;
    while (index_ < owner_->bucketCount_ &&
           owner_->buckets_[index_].state != BUCKET_OCCUPIED)
        ++index_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::BasicIterator()
    : owner_(NULL), index_(0) {}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
template<bool Enabled>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::BasicIterator(
    const BasicIterator<false>& other,
    typename std::enable_if<Enabled, int>::type*
)
    : owner_(other.owner_), index_(other.index_) {}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::template BasicIterator<IsConst>::reference
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator*() const {
    return *owner_->buckets_[index_].node;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::template BasicIterator<IsConst>::pointer
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator->() const {
    return owner_->buckets_[index_].node;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::template BasicIterator<IsConst>&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator++() {
    ++index_;
    skipToOccupied();
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::template BasicIterator<IsConst>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator++(int) {
    BasicIterator copy(*this);
    ++(*this);
    return copy;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
template<bool OtherConst>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator==(
    const BasicIterator<OtherConst>& other
) const {
    return owner_ == other.owner_ && index_ == other.index_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<bool IsConst>
template<bool OtherConst>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::BasicIterator<IsConst>::operator!=(
    const BasicIterator<OtherConst>& other
) const {
    return !(*this == other);
}

// ===== LookupStorage 实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::LookupStorage(
    const HashTable& owner
)
    : owner_(owner) {}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::bucketCount() const {
    return owner_.bucketCount_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::isOccupied(
    size_type index
) const {
    return owner_.buckets_[index].state == BUCKET_OCCUPIED;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::isDeleted(
    size_type index
) const {
    return owner_.buckets_[index].state == BUCKET_DELETED;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename LookupKey>
std::size_t HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::hashKey(
    const LookupKey& key
) const {
    return owner_.hash_(key);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename LookupKey>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::LookupStorage::keysEqual(
    size_type index,
    const LookupKey& key
) const {
    return owner_.equal_(owner_.buckets_[index].node->first, key);
}

// ===== Storage 实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::Storage(HashTable& owner)
    : owner_(owner), pendingBuckets_(NULL), pendingBucketCount_(0) {}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::~Storage() {
    rollbackRehash();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::bucketCount() const {
    return owner_.bucketCount_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::isOccupied(
    size_type index
) const {
    return owner_.buckets_[index].state == BUCKET_OCCUPIED;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::isDeleted(
    size_type index
) const {
    return owner_.buckets_[index].state == BUCKET_DELETED;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename LookupKey>
std::size_t HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::hashKey(
    const LookupKey& key
) const {
    return owner_.hash_(key);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename LookupKey>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::keysEqual(
    size_type index,
    const LookupKey& key
) const {
    return owner_.equal_(owner_.buckets_[index].node->first, key);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::needsRehashForInsert() const {
    if (owner_.bucketCount_ == 0)
        return true;
    if (static_cast<double>(owner_.size_ + 1) >
        static_cast<double>(owner_.bucketCount_) * owner_.maxLoadFactor_)
        return true;
    return owner_.tombstoneCount_ > owner_.bucketCount_ / 4 &&
           owner_.tombstoneCount_ > owner_.size_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::growthBucketCount() const {
    const size_type required = owner_.requiredBucketsForElements(owner_.size_ + 1);
    if (owner_.bucketCount_ == 0)
        return required < 3 ? 3 : required;

    const bool noUnusedBucket = owner_.size_ + owner_.tombstoneCount_ >= owner_.bucketCount_;
    if (required <= owner_.bucketCount_ && !noUnusedBucket)
        return owner_.bucketCount_;

    const size_type maximum = (std::numeric_limits<size_type>::max)();
    const size_type doubled = owner_.bucketCount_ > maximum / 2
        ? maximum
        : owner_.bucketCount_ * 2;
    return required > doubled ? required : doubled;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::eraseAt(
    size_type index
) {
    owner_.destroyNode(owner_.buckets_[index].node);
    owner_.buckets_[index].node = NULL;
    owner_.buckets_[index].state = BUCKET_DELETED;
    --owner_.size_;
    ++owner_.tombstoneCount_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::beginRehash(
    size_type bucketCount
) {
    rollbackRehash();
    pendingBuckets_ = owner_.allocateBucketArray(bucketCount);
    pendingBucketCount_ = bucketCount;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::transferBucketToPending(
    size_type oldIndex
) {
    pointer node = owner_.buckets_[oldIndex].node;
    const std::size_t hash = owner_.hash_(node->first);
    for (size_type step = 0; step < pendingBucketCount_; ++step) {
        const size_type index = static_cast<size_type>(
            ProbePolicy::index(hash, step, pendingBucketCount_)
        );
        if (pendingBuckets_[index].state == BUCKET_EMPTY) {
            pendingBuckets_[index].node = node;
            pendingBuckets_[index].state = BUCKET_OCCUPIED;
            return;
        }
    }
    throw std::length_error("HashTable rehash target is full");
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::commitRehash() noexcept {
    owner_.deallocateBucketArray(owner_.buckets_, owner_.bucketCount_);
    owner_.buckets_ = pendingBuckets_;
    owner_.bucketCount_ = pendingBucketCount_;
    owner_.tombstoneCount_ = 0;
    pendingBuckets_ = NULL;
    pendingBucketCount_ = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Storage::rollbackRehash() noexcept {
    owner_.deallocateBucketArray(pendingBuckets_, pendingBucketCount_);
    pendingBuckets_ = NULL;
    pendingBucketCount_ = 0;
}

// ===== 私有资源与插入辅助实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::Bucket*
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::allocateBucketArray(
    size_type bucketCount
) {
    if (bucketCount == 0)
        return NULL;
    bucket_allocator_type allocator(allocator_);
    Bucket* buckets = bucket_allocator_traits::allocate(allocator, bucketCount);
    size_type constructed = 0;
    try {
        for (; constructed < bucketCount; ++constructed)
            bucket_allocator_traits::construct(allocator, buckets + constructed);
    } catch (...) {
        while (constructed != 0) {
            --constructed;
            bucket_allocator_traits::destroy(allocator, buckets + constructed);
        }
        bucket_allocator_traits::deallocate(allocator, buckets, bucketCount);
        throw;
    }
    return buckets;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::deallocateBucketArray(
    Bucket* buckets,
    size_type bucketCount
) noexcept {
    if (buckets == NULL)
        return;
    bucket_allocator_type allocator(allocator_);
    for (size_type index = 0; index < bucketCount; ++index)
        bucket_allocator_traits::destroy(allocator, buckets + index);
    bucket_allocator_traits::deallocate(allocator, buckets, bucketCount);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename... Args>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::pointer
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::createNode(Args&&... args) {
    pointer node = allocator_traits::allocate(allocator_, 1);
    try {
        allocator_traits::construct(allocator_, node, std::forward<Args>(args)...);
    } catch (...) {
        allocator_traits::deallocate(allocator_, node, 1);
        throw;
    }
    return node;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::destroyNode(pointer node) noexcept {
    if (node == NULL)
        return;
    allocator_traits::destroy(allocator_, node);
    allocator_traits::deallocate(allocator_, node, 1);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::initializeBuckets(
    size_type requestedBucketCount
) {
    bucketCount_ = static_cast<size_type>(ProbePolicy::bucketCountAtLeast(requestedBucketCount));
    buckets_ = allocateBucketArray(bucketCount_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::destroyStorage() noexcept {
    clear();
    deallocateBucketArray(buckets_, bucketCount_);
    buckets_ = NULL;
    bucketCount_ = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::stealStorage(
    HashTable& another
) noexcept {
    buckets_ = another.buckets_;
    bucketCount_ = another.bucketCount_;
    size_ = another.size_;
    tombstoneCount_ = another.tombstoneCount_;
    another.buckets_ = NULL;
    another.bucketCount_ = 0;
    another.size_ = 0;
    another.tombstoneCount_ = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::swapContent(HashTable& other) {
    using std::swap;
    swap(hash_, other.hash_);
    swap(equal_, other.equal_);
    swap(buckets_, other.buckets_);
    swap(bucketCount_, other.bucketCount_);
    swap(size_, other.size_);
    swap(tombstoneCount_, other.tombstoneCount_);
    swap(maxLoadFactor_, other.maxLoadFactor_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::swapAll(HashTable& other) {
    using std::swap;
    // 先交换可能抛异常的策略对象；allocator 一旦交换，后续仅交换不抛异常的所有权字段。
    swap(hash_, other.hash_);
    swap(equal_, other.equal_);
    swap(maxLoadFactor_, other.maxLoadFactor_);
    swap(allocator_, other.allocator_);
    swap(buckets_, other.buckets_);
    swap(bucketCount_, other.bucketCount_);
    swap(size_, other.size_);
    swap(tombstoneCount_, other.tombstoneCount_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::requiredBucketsForElements(
    size_type elementCount
) const {
    if (elementCount == 0)
        return 3;
    const double exact = static_cast<double>(elementCount) /
                         static_cast<double>(maxLoadFactor_);
    if (exact > static_cast<double>((std::numeric_limits<size_type>::max)()))
        throw std::length_error("HashTable bucket count overflow");
    size_type required = static_cast<size_type>(exact);
    if (static_cast<double>(required) * maxLoadFactor_ <
        static_cast<double>(elementCount))
        ++required;
    return required < 3 ? 3 : required;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::commitNode(
    size_type index,
    pointer node
) noexcept {
    if (buckets_[index].state == BUCKET_DELETED)
        --tombstoneCount_;
    buckets_[index].node = node;
    buckets_[index].state = BUCKET_OCCUPIED;
    ++size_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename Value>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insertValue(Value&& value) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::prepareInsert(storage, value.first);
    if (result.found)
        return std::make_pair(iterator(this, result.foundIndex), false);
    pointer node = createNode(std::forward<Value>(value));
    commitNode(result.insertionIndex, node);
    return std::make_pair(iterator(this, result.insertionIndex), true);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename KeyArg, typename MappedArg>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insertKeyValue(
    KeyArg&& key,
    MappedArg&& mapped
) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::prepareInsert(storage, key);
    if (result.found)
        return std::make_pair(iterator(this, result.foundIndex), false);
    pointer node = createNode(
        std::piecewise_construct,
        std::forward_as_tuple(std::forward<KeyArg>(key)),
        std::forward_as_tuple(std::forward<MappedArg>(mapped))
    );
    commitNode(result.insertionIndex, node);
    return std::make_pair(iterator(this, result.insertionIndex), true);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::copyElementsFrom(
    const HashTable& other
) {
    try {
        for (const_iterator iterator = other.begin(); iterator != other.end(); ++iterator)
            insert(*iterator);
    } catch (...) {
        destroyStorage();
        throw;
    }
}

// ===== 构造、赋值与迭代器实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable()
    : allocator_(), hash_(), equal_(), buckets_(NULL), bucketCount_(0),
      size_(0), tombstoneCount_(0), maxLoadFactor_(0.5f) {
    initializeBuckets(3);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    const allocator_type& allocator
)
    : allocator_(allocator), hash_(), equal_(), buckets_(NULL), bucketCount_(0),
      size_(0), tombstoneCount_(0), maxLoadFactor_(0.5f) {
    initializeBuckets(3);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    size_type bucketCount,
    const hasher& hash,
    const key_equal& equal,
    const allocator_type& allocator
)
    : allocator_(allocator), hash_(hash), equal_(equal), buckets_(NULL), bucketCount_(0),
      size_(0), tombstoneCount_(0), maxLoadFactor_(0.5f) {
    initializeBuckets(bucketCount);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename InputIt>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    InputIt first,
    InputIt last,
    size_type bucketCount,
    const hasher& hash,
    const key_equal& equal,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIt>::value>::type*
)
    : allocator_(allocator), hash_(hash), equal_(equal), buckets_(NULL), bucketCount_(0),
      size_(0), tombstoneCount_(0), maxLoadFactor_(0.5f) {
    initializeBuckets(bucketCount);
    try {
        insert(first, last);
    } catch (...) {
        destroyStorage();
        throw;
    }
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    std::initializer_list<value_type> values,
    size_type bucketCount,
    const hasher& hash,
    const key_equal& equal,
    const allocator_type& allocator
)
    : allocator_(allocator), hash_(hash), equal_(equal), buckets_(NULL), bucketCount_(0),
      size_(0), tombstoneCount_(0), maxLoadFactor_(0.5f) {
    initializeBuckets(bucketCount);
    try {
        insert(values.begin(), values.end());
    } catch (...) {
        destroyStorage();
        throw;
    }
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(const HashTable& other)
    : allocator_(allocator_traits::select_on_container_copy_construction(other.allocator_)),
      hash_(other.hash_), equal_(other.equal_), buckets_(NULL), bucketCount_(0), size_(0),
      tombstoneCount_(0), maxLoadFactor_(other.maxLoadFactor_) {
    initializeBuckets(other.bucketCount_ == 0 ? 3 : other.bucketCount_);
    copyElementsFrom(other);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    const HashTable& other,
    const allocator_type& allocator
)
    : allocator_(allocator), hash_(other.hash_), equal_(other.equal_), buckets_(NULL),
      bucketCount_(0), size_(0), tombstoneCount_(0), maxLoadFactor_(other.maxLoadFactor_) {
    initializeBuckets(other.bucketCount_ == 0 ? 3 : other.bucketCount_);
    copyElementsFrom(other);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(HashTable&& other) noexcept(
    std::is_nothrow_move_constructible<allocator_type>::value &&
    std::is_nothrow_move_constructible<hasher>::value &&
    std::is_nothrow_move_constructible<key_equal>::value
)
    : allocator_(std::move(other.allocator_)), hash_(std::move(other.hash_)),
      equal_(std::move(other.equal_)), buckets_(NULL), bucketCount_(0), size_(0),
      tombstoneCount_(0), maxLoadFactor_(other.maxLoadFactor_) {
    stealStorage(other);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::HashTable(
    HashTable&& other,
    const allocator_type& allocator
)
    : allocator_(allocator), hash_(std::move(other.hash_)), equal_(std::move(other.equal_)),
      buckets_(NULL), bucketCount_(0), size_(0), tombstoneCount_(0),
      maxLoadFactor_(other.maxLoadFactor_) {
    if (allocator_ == other.allocator_) {
        stealStorage(other);
        return;
    }
    initializeBuckets(other.bucketCount_ == 0 ? 3 : other.bucketCount_);
    try {
        for (iterator iterator = other.begin(); iterator != other.end(); ++iterator)
            insertKeyValue(iterator->first, std::move(iterator->second));
    } catch (...) {
        destroyStorage();
        throw;
    }
    other.clear();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::~HashTable() {
    destroyStorage();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::operator=(const HashTable& other) {
    if (this == &other)
        return *this;
    if (allocator_traits::propagate_on_container_copy_assignment::value) {
        HashTable copy(other, other.allocator_);
        swapAll(copy);
    } else {
        HashTable copy(other, allocator_);
        swapContent(copy);
    }
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::operator=(HashTable&& other) {
    if (this == &other)
        return *this;

    if (allocator_traits::propagate_on_container_move_assignment::value) {
        destroyStorage();
        allocator_ = std::move(other.allocator_);
        hash_ = std::move(other.hash_);
        equal_ = std::move(other.equal_);
        maxLoadFactor_ = other.maxLoadFactor_;
        stealStorage(other);
        return *this;
    }

    if (allocator_ == other.allocator_) {
        destroyStorage();
        hash_ = std::move(other.hash_);
        equal_ = std::move(other.equal_);
        maxLoadFactor_ = other.maxLoadFactor_;
        stealStorage(other);
        return *this;
    }

    HashTable moved(3, other.hash_, other.equal_, allocator_);
    moved.maxLoadFactor_ = other.maxLoadFactor_;
    moved.reserve(other.size_);
    for (iterator iterator = other.begin(); iterator != other.end(); ++iterator)
        moved.insertKeyValue(iterator->first, std::move(iterator->second));
    swapContent(moved);
    other.clear();
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::operator=(
    std::initializer_list<value_type> values
) {
    clear();
    insert(values);
    return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::allocator_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::get_allocator() const {
    return allocator_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::begin() {
    return iterator(this, 0);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::begin() const {
    return const_iterator(this, 0);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::cbegin() const {
    return begin();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::end() {
    return iterator(this, bucketCount_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::end() const {
    return const_iterator(this, bucketCount_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::cend() const {
    return end();
}

// ===== 容量、修改、查找与桶接口实现 =====

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::empty() const {
    return size_ == 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size() const {
    return size_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::max_size() const {
    return allocator_traits::max_size(allocator_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::clear() noexcept {
    for (size_type index = 0; index < bucketCount_; ++index) {
        if (buckets_[index].state == BUCKET_OCCUPIED)
            destroyNode(buckets_[index].node);
        buckets_[index].node = NULL;
        buckets_[index].state = BUCKET_EMPTY;
    }
    size_ = 0;
    tombstoneCount_ = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insert(const value_type& value) {
    return insertValue(value);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insert(value_type&& value) {
    return insertValue(std::move(value));
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename InputIt>
typename std::enable_if<!std::is_integral<InputIt>::value, void>::type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insert(InputIt first, InputIt last) {
    for (; first != last; ++first)
        insert(*first);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insert(
    std::initializer_list<value_type> values
) {
    insert(values.begin(), values.end());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename... Args>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::emplace(Args&&... args) {
    pointer node = createNode(std::forward<Args>(args)...);
    try {
        Storage storage(*this);
        const probe_result result = MutationAlgorithm::prepareInsert(storage, node->first);
        if (result.found) {
            destroyNode(node);
            return std::make_pair(iterator(this, result.foundIndex), false);
        }
        commitNode(result.insertionIndex, node);
        return std::make_pair(iterator(this, result.insertionIndex), true);
    } catch (...) {
        destroyNode(node);
        throw;
    }
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
template<typename M>
std::pair<typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator, bool>
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::insert_or_assign(
    const key_type& key,
    M&& value
) {
    iterator existing = find(key);
    if (existing != end()) {
        existing->second = std::forward<M>(value);
        return std::make_pair(existing, false);
    }
    return insertKeyValue(key, std::forward<M>(value));
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::erase(const_iterator position) {
    if (position.owner_ != this || position.index_ >= bucketCount_)
        return end();
    const size_type index = position.index_;
    Storage storage(*this);
    storage.eraseAt(index);
    return iterator(this, index + 1);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::erase(const key_type& key) {
    Storage storage(*this);
    return MutationAlgorithm::erase(storage, key) ? 1 : 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::swap(HashTable& other) {
    if (allocator_traits::propagate_on_container_swap::value) {
        swapAll(other);
        return;
    }
    if (!(allocator_ == other.allocator_))
        throw std::logic_error("HashTable::swap requires equal allocators");
    swapContent(other);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::mapped_type&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::at(const key_type& key) {
    iterator iterator = find(key);
    if (iterator == end())
        throw std::out_of_range("HashTable::at key not found");
    return iterator->second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
const typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::mapped_type&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::at(const key_type& key) const {
    const_iterator iterator = find(key);
    if (iterator == end())
        throw std::out_of_range("HashTable::at key not found");
    return iterator->second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::mapped_type&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::operator[](const key_type& key) {
    return insertKeyValue(key, mapped_type()).first->second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::mapped_type&
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::operator[](key_type&& key) {
    return insertKeyValue(std::move(key), mapped_type()).first->second;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::find(const key_type& key) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::locate(storage, key);
    return result.found ? iterator(this, result.foundIndex) : end();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::find(const key_type& key) const {
    LookupStorage storage(*this);
    const typename LookupAlgorithm::probe_result result = LookupAlgorithm::locate(storage, key);
    return result.found ? const_iterator(this, result.foundIndex) : end();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::count(const key_type& key) const {
    return find(key) == end() ? 0 : 1;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::contains(const key_type& key) const {
    return find(key) != end();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::bucket_count() const {
    return bucketCount_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::size_type
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::bucket(const key_type& key) const {
    return bucketCount_ == 0 ? 0 : static_cast<size_type>(hash_(key) % bucketCount_);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
float HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::load_factor() const {
    return bucketCount_ == 0 ? 0.0f : static_cast<float>(size_) / bucketCount_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
float HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::max_load_factor() const {
    return maxLoadFactor_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::max_load_factor(float value) {
    if (!(value > 0.0f))
        throw std::invalid_argument("HashTable max_load_factor must be positive");
    maxLoadFactor_ = value;
    if (static_cast<double>(size_) > static_cast<double>(bucketCount_) * maxLoadFactor_)
        rehash(requiredBucketsForElements(size_));
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::rehash(
    size_type requestedBucketCount
) {
    const size_type minimum = requiredBucketsForElements(size_);
    const size_type requested = requestedBucketCount < minimum ? minimum : requestedBucketCount;
    const size_type target = static_cast<size_type>(ProbePolicy::bucketCountAtLeast(requested));
    if (target == bucketCount_ && tombstoneCount_ == 0)
        return;
    Storage storage(*this);
    MutationAlgorithm::rehash(storage, target);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::reserve(size_type elementCount) {
    const size_type required = requiredBucketsForElements(elementCount);
    if (required > bucketCount_)
        rehash(required);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::hasher
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::hash_function() const {
    return hash_;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::key_equal
HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::key_eq() const {
    return equal_;
}

/// 比较两张表是否包含相同键值对，不比较桶布局、墓碑或策略对象状态。
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool operator==(
    const HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& left,
    const HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& right
) {
    if (left.size() != right.size())
        return false;
    for (typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator
             iterator = left.begin();
         iterator != left.end();
         ++iterator) {
        typename HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>::const_iterator found =
            right.find(iterator->first);
        if (found == right.end() || !(found->second == iterator->second))
            return false;
    }
    return true;
}

/// 返回两张表是否具有不同键值内容。
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
bool operator!=(
    const HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& left,
    const HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& right
) {
    return !(left == right);
}

/// 调用成员 swap，并遵守 allocator 所有权约束。
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator, typename ProbePolicy>
void swap(
    HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& left,
    HashTable<Key, T, Hash, KeyEqual, Allocator, ProbePolicy>& right
) {
    left.swap(right);
}

/// 使用双向平方探测策略的工业哈希表别名。
template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >
>
using QuadraticHashTable = HashTable<
    Key,
    T,
    Hash,
    KeyEqual,
    Allocator,
    dsa::core::QuadraticProbing
>;

} // namespace container
} // namespace dsa

#endif
