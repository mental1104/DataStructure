#ifndef __DSA_HASHTABLE
#define __DSA_HASHTABLE

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Bitmap.h"
#include "Dictionary.h"
#include "Entry.h"
#include "dsa/core/hash/HashTableAlgorithm.h"

namespace dsa_teaching_detail {

/// 教学版线性探测的装填因子与扩容策略。
struct TeachingLinearProbing : dsa::core::LinearProbing {
    /// 在下一次插入会使装填因子超过 50% 时扩容。
    static bool needsGrowth(std::size_t size, std::size_t bucketCount) {
        return size + 1 > bucketCount / 2;
    }

    /// 保留原教学实现“扩容后容量至少约为元素数四倍”的策略。
    static std::size_t growthRequest(std::size_t size, std::size_t bucketCount) {
        const std::size_t required = size + 1;
        if (required > (std::numeric_limits<std::size_t>::max)() / 4)
            throw std::length_error("Hashtable capacity overflow");
        const std::size_t requested = required * 4;
        return requested > bucketCount ? requested : bucketCount + 1;
    }
};

/// 教学版双向平方探测的容量与扩容策略。
struct TeachingQuadraticProbing : dsa::core::QuadraticProbing {
    /// 在下一次插入将占满桶数组前扩容。
    static bool needsGrowth(std::size_t size, std::size_t bucketCount) {
        return size + 1 >= bucketCount;
    }

    /// 保留原教学实现“扩容后容量至少约为元素数两倍”的策略。
    static std::size_t growthRequest(std::size_t size, std::size_t bucketCount) {
        const std::size_t required = size + 1;
        if (required > (std::numeric_limits<std::size_t>::max)() / 2)
            throw std::length_error("QuadraticHT capacity overflow");
        const std::size_t requested = required * 2;
        return requested > bucketCount ? requested : bucketCount + 1;
    }
};

/// 教学版开放寻址哈希表的公共实现，具体探测序列由 ProbePolicy 决定。
template<typename K, typename V, typename ProbePolicy>
class OpenAddressTeachingTable : public Dictionary<K, V> {
private:
    static const std::size_t MAX_BUCKET_COUNT = 1048576;

    Entry<K, V>** ht;
    int M;
    int N;
    Bitmap* lazyRemoval;

    /// 将教学版桶数组和墓碑位图适配到共享 HashTableAlgorithm。
    class Storage {
    public:
        typedef std::size_t size_type;

        /// 绑定当前教学哈希表，并初始化空的 rehash 临时状态。
        explicit Storage(OpenAddressTeachingTable& owner);

        /// 放弃尚未提交的新桶数组，避免异常路径泄漏。
        ~Storage();

        /// 返回当前桶数量。
        size_type bucketCount() const;

        /// 判断指定桶是否保存词条。
        bool isOccupied(size_type index) const;

        /// 判断指定空桶是否为懒惰删除墓碑。
        bool isDeleted(size_type index) const;

        /// 使用教学版 hashCode 计算键的散列值。
        template<typename Key>
        std::size_t hashKey(const Key& key) const;

        /// 比较桶内键与查询键是否相等。
        template<typename Key>
        bool keysEqual(size_type index, const Key& key) const;

        /// 判断下一次成功插入前是否需要扩容。
        bool needsRehashForInsert() const;

        /// 返回下一次 rehash 的请求桶数量。
        size_type growthBucketCount() const;

        /// 在目标桶构造词条，成功后再提交 size 与墓碑状态。
        void insertAt(size_type index, const K& key, const V& value);

        /// 析构指定词条并提交墓碑和 size。
        void eraseAt(size_type index);

        /// 申请尚未提交的新桶数组和新墓碑位图。
        void beginRehash(size_type bucketCount);

        /// 将旧桶中的词条指针放入临时桶数组，不改变旧表所有权。
        void transferBucketToPending(size_type oldIndex);

        /// 原子切换到新桶数组，并释放旧桶数组与旧墓碑位图。
        void commitRehash() noexcept;

        /// 放弃临时桶数组；其中的词条仍由旧表拥有。
        void rollbackRehash() noexcept;

    private:
        OpenAddressTeachingTable& owner_;
        Entry<K, V>** pendingHt_;
        Bitmap* pendingLazyRemoval_;
        size_type pendingBucketCount_;

        Storage(const Storage&);
        Storage& operator=(const Storage&);
    };

    typedef dsa::core::HashTableAlgorithm<Storage, ProbePolicy> MutationAlgorithm;
    typedef typename MutationAlgorithm::probe_result probe_result;

    /// 为指定桶数量分配空桶数组和墓碑位图。
    void initialize(std::size_t requestedBucketCount);

    /// 析构全部词条并释放桶数组与墓碑位图。
    void destroyStorage() noexcept;

    /// 深拷贝另一个教学哈希表的词条和墓碑状态。
    void copyFrom(const OpenAddressTeachingTable& other);

    /// 交换两张教学哈希表的全部存储状态。
    void swapStorage(OpenAddressTeachingTable& other) noexcept;

protected:
    /// 按共享探测流程查找命中桶；未命中时返回可插入桶或 -1。
    int probe4Hit(const K& key);

    /// 按共享探测流程查找可插入桶；表满时返回 -1。
    int probe4Free(const K& key);

    /// 使用共享事务流程清理墓碑并迁移全部词条。
    void rehash();

public:
    /// 构造指定初始容量的教学哈希表。
    explicit OpenAddressTeachingTable(int capacity);

    /// 深拷贝词条和墓碑状态，避免原实现的浅拷贝双重释放。
    OpenAddressTeachingTable(const OpenAddressTeachingTable& other);

    /// 通过 copy-and-swap 提供强异常保证的深拷贝赋值。
    OpenAddressTeachingTable& operator=(const OpenAddressTeachingTable& other);

    /// 析构全部词条、桶数组和墓碑位图。
    virtual ~OpenAddressTeachingTable();

    /// 返回当前词条数量。
    int size() const;

    /// 返回当前桶数量。
    int capacity() const;

    /// 键不存在时插入词条，存在时保持原值并返回 false。
    bool put(K key, V value);

    /// 返回键对应值的地址；键不存在时返回 nullptr。
    V* get(K key);

    /// 删除键对应词条并留下墓碑；键不存在时返回 false。
    bool remove(K key);

    /// 教学打印接口：返回桶数量。
    int _M();

    /// 教学打印接口：返回词条数量。
    int _N();

    /// 教学打印接口：返回墓碑位图。
    Bitmap* _lazyRemoval();

    /// 教学打印接口：返回指定桶中的词条指针。
    Entry<K, V>* _ht(int index);
};

// ===== Storage 实现 =====

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::Storage(
    OpenAddressTeachingTable& owner
)
    : owner_(owner),
      pendingHt_(NULL),
      pendingLazyRemoval_(NULL),
      pendingBucketCount_(0) {}

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::~Storage() {
    rollbackRehash();
}

template<typename K, typename V, typename ProbePolicy>
typename OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::size_type
OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::bucketCount() const {
    return static_cast<size_type>(owner_.M);
}

template<typename K, typename V, typename ProbePolicy>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::isOccupied(
    size_type index
) const {
    return owner_.ht[index] != NULL;
}

template<typename K, typename V, typename ProbePolicy>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::isDeleted(
    size_type index
) const {
    return owner_.ht[index] == NULL && owner_.lazyRemoval->test(static_cast<int>(index));
}

template<typename K, typename V, typename ProbePolicy>
template<typename Key>
std::size_t OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::hashKey(
    const Key& key
) const {
    return hashCode(key);
}

template<typename K, typename V, typename ProbePolicy>
template<typename Key>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::keysEqual(
    size_type index,
    const Key& key
) const {
    return owner_.ht[index]->key == key;
}

template<typename K, typename V, typename ProbePolicy>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::needsRehashForInsert() const {
    return ProbePolicy::needsGrowth(
        static_cast<size_type>(owner_.N),
        static_cast<size_type>(owner_.M)
    );
}

template<typename K, typename V, typename ProbePolicy>
typename OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::size_type
OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::growthBucketCount() const {
    const size_type requested = ProbePolicy::growthRequest(
        static_cast<size_type>(owner_.N),
        static_cast<size_type>(owner_.M)
    );
    if (requested > MAX_BUCKET_COUNT)
        throw std::length_error("Teaching hash table exceeds maximum bucket count");
    return requested;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::insertAt(
    size_type index,
    const K& key,
    const V& value
) {
    Entry<K, V>* entry = new Entry<K, V>(key, value);
    owner_.ht[index] = entry;
    owner_.lazyRemoval->clear(static_cast<int>(index));
    ++owner_.N;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::eraseAt(size_type index) {
    delete owner_.ht[index];
    owner_.ht[index] = NULL;
    owner_.lazyRemoval->set(static_cast<int>(index));
    --owner_.N;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::beginRehash(
    size_type bucketCount
) {
    rollbackRehash();
    if (bucketCount > MAX_BUCKET_COUNT)
        throw std::length_error("Teaching hash table exceeds maximum bucket count");

    pendingHt_ = new Entry<K, V>*[bucketCount]();
    try {
        pendingLazyRemoval_ = new Bitmap(static_cast<int>(bucketCount));
    } catch (...) {
        delete [] pendingHt_;
        pendingHt_ = NULL;
        throw;
    }
    pendingBucketCount_ = bucketCount;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::transferBucketToPending(
    size_type oldIndex
) {
    Entry<K, V>* entry = owner_.ht[oldIndex];
    const std::size_t hash = hashCode(entry->key);
    for (size_type step = 0; step < pendingBucketCount_; ++step) {
        const size_type index = static_cast<size_type>(
            ProbePolicy::index(hash, step, pendingBucketCount_)
        );
        if (pendingHt_[index] == NULL) {
            pendingHt_[index] = entry;
            return;
        }
    }
    throw std::length_error("Teaching hash table rehash target is full");
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::commitRehash() noexcept {
    delete [] owner_.ht;
    delete owner_.lazyRemoval;
    owner_.ht = pendingHt_;
    owner_.lazyRemoval = pendingLazyRemoval_;
    owner_.M = static_cast<int>(pendingBucketCount_);
    pendingHt_ = NULL;
    pendingLazyRemoval_ = NULL;
    pendingBucketCount_ = 0;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::Storage::rollbackRehash() noexcept {
    delete [] pendingHt_;
    delete pendingLazyRemoval_;
    pendingHt_ = NULL;
    pendingLazyRemoval_ = NULL;
    pendingBucketCount_ = 0;
}

// ===== 教学容器实现 =====

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::initialize(
    std::size_t requestedBucketCount
) {
    const std::size_t normalized = ProbePolicy::bucketCountAtLeast(requestedBucketCount);
    if (normalized > MAX_BUCKET_COUNT)
        throw std::length_error("Teaching hash table exceeds maximum bucket count");
    M = static_cast<int>(normalized);
    N = 0;
    ht = new Entry<K, V>*[M]();
    try {
        lazyRemoval = new Bitmap(M);
    } catch (...) {
        delete [] ht;
        ht = NULL;
        M = 0;
        throw;
    }
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::destroyStorage() noexcept {
    if (ht != NULL) {
        for (int index = 0; index < M; ++index)
            delete ht[index];
    }
    delete [] ht;
    delete lazyRemoval;
    ht = NULL;
    lazyRemoval = NULL;
    M = 0;
    N = 0;
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::copyFrom(
    const OpenAddressTeachingTable& other
) {
    initialize(static_cast<std::size_t>(other.M));
    try {
        for (int index = 0; index < other.M; ++index) {
            if (other.ht[index] != NULL) {
                ht[index] = new Entry<K, V>(*other.ht[index]);
                ++N;
            } else if (other.lazyRemoval->test(index)) {
                lazyRemoval->set(index);
            }
        }
    } catch (...) {
        destroyStorage();
        throw;
    }
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::swapStorage(
    OpenAddressTeachingTable& other
) noexcept {
    std::swap(ht, other.ht);
    std::swap(M, other.M);
    std::swap(N, other.N);
    std::swap(lazyRemoval, other.lazyRemoval);
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::probe4Hit(const K& key) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::locate(storage, key);
    const std::size_t index = result.found ? result.foundIndex : result.insertionIndex;
    return index == probe_result::npos() ? -1 : static_cast<int>(index);
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::probe4Free(const K& key) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::locate(storage, key);
    return result.insertionIndex == probe_result::npos()
        ? -1
        : static_cast<int>(result.insertionIndex);
}

template<typename K, typename V, typename ProbePolicy>
void OpenAddressTeachingTable<K, V, ProbePolicy>::rehash() {
    Storage storage(*this);
    MutationAlgorithm::rehash(storage, storage.growthBucketCount());
}

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>::OpenAddressTeachingTable(int capacity)
    : ht(NULL), M(0), N(0), lazyRemoval(NULL) {
    initialize(capacity > 0 ? static_cast<std::size_t>(capacity) : 3U);
}

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>::OpenAddressTeachingTable(
    const OpenAddressTeachingTable& other
)
    : ht(NULL), M(0), N(0), lazyRemoval(NULL) {
    copyFrom(other);
}

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>&
OpenAddressTeachingTable<K, V, ProbePolicy>::operator=(
    const OpenAddressTeachingTable& other
) {
    if (this != &other) {
        OpenAddressTeachingTable copy(other);
        swapStorage(copy);
    }
    return *this;
}

template<typename K, typename V, typename ProbePolicy>
OpenAddressTeachingTable<K, V, ProbePolicy>::~OpenAddressTeachingTable() {
    destroyStorage();
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::size() const {
    return N;
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::capacity() const {
    return M;
}

template<typename K, typename V, typename ProbePolicy>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::put(K key, V value) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::prepareInsert(storage, key);
    if (result.found)
        return false;
    storage.insertAt(result.insertionIndex, key, value);
    return true;
}

template<typename K, typename V, typename ProbePolicy>
V* OpenAddressTeachingTable<K, V, ProbePolicy>::get(K key) {
    Storage storage(*this);
    const probe_result result = MutationAlgorithm::locate(storage, key);
    return result.found ? &ht[result.foundIndex]->value : NULL;
}

template<typename K, typename V, typename ProbePolicy>
bool OpenAddressTeachingTable<K, V, ProbePolicy>::remove(K key) {
    Storage storage(*this);
    return MutationAlgorithm::erase(storage, key);
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::_M() {
    return M;
}

template<typename K, typename V, typename ProbePolicy>
int OpenAddressTeachingTable<K, V, ProbePolicy>::_N() {
    return N;
}

template<typename K, typename V, typename ProbePolicy>
Bitmap* OpenAddressTeachingTable<K, V, ProbePolicy>::_lazyRemoval() {
    return lazyRemoval;
}

template<typename K, typename V, typename ProbePolicy>
Entry<K, V>* OpenAddressTeachingTable<K, V, ProbePolicy>::_ht(int index) {
    return ht[index];
}

} // namespace dsa_teaching_detail

/// 保留原公开 API 的线性探测教学哈希表。
template<typename K, typename V>
class Hashtable
    : public dsa_teaching_detail::OpenAddressTeachingTable<
          K,
          V,
          dsa_teaching_detail::TeachingLinearProbing
      > {
private:
    typedef dsa_teaching_detail::OpenAddressTeachingTable<
        K,
        V,
        dsa_teaching_detail::TeachingLinearProbing
    > Base;

public:
    /// 构造线性探测教学哈希表。
    explicit Hashtable(int capacity = 3) : Base(capacity) {}
};

#endif
