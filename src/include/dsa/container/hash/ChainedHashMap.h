#ifndef DSA_CONTAINER_HASH_CHAINED_HASH_MAP_H
#define DSA_CONTAINER_HASH_CHAINED_HASH_MAP_H

#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <dsa/container/hash/Dictionary.h>

namespace dsa {
namespace container {

// allocator-aware 独立链式哈希表。rehash 先计算全部目标桶，再无抛异常地 splice 节点提交。
template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >
>
class ChainedHashMap : public Dictionary<Key, T> {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

private:
    typedef std::list<value_type, allocator_type> bucket_type;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<bucket_type>
        bucket_allocator_type;
    typedef std::vector<bucket_type, bucket_allocator_type> bucket_array_type;

    template<bool IsConst>
    class BasicIterator {
        friend class ChainedHashMap;
        template<bool> friend class BasicIterator;
        typedef typename std::conditional<IsConst, const ChainedHashMap, ChainedHashMap>::type owner_type;
        typedef typename std::conditional<IsConst, typename bucket_type::const_iterator, typename bucket_type::iterator>::type local_iterator;

        owner_type* owner_;
        size_type bucket_;
        local_iterator current_;

        BasicIterator(owner_type* owner, size_type bucket, local_iterator current)
            : owner_(owner), bucket_(bucket), current_(current) {
            skipEmpty();
        }

        void skipEmpty() {
            while (owner_ && bucket_ < owner_->buckets_.size() && current_ == owner_->buckets_[bucket_].end()) {
                ++bucket_;
                if (bucket_ < owner_->buckets_.size())
                    current_ = owner_->buckets_[bucket_].begin();
            }
        }

    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename ChainedHashMap::value_type value_type;
        typedef typename ChainedHashMap::difference_type difference_type;
        typedef typename std::conditional<IsConst, const value_type*, value_type*>::type pointer;
        typedef typename std::conditional<IsConst, const value_type&, value_type&>::type reference;

        BasicIterator() : owner_(0), bucket_(0), current_() {}

        template<bool OtherConst>
        BasicIterator(
            const BasicIterator<OtherConst>& other,
            typename std::enable_if<IsConst && !OtherConst, int>::type = 0
        ) : owner_(other.owner_), bucket_(other.bucket_), current_(other.current_) {}

        reference operator*() const { return *current_; }
        pointer operator->() const { return &*current_; }

        BasicIterator& operator++() {
            ++current_;
            skipEmpty();
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator copy(*this);
            ++(*this);
            return copy;
        }

        template<bool OtherConst>
        bool operator==(const BasicIterator<OtherConst>& other) const {
            if (owner_ != other.owner_ || bucket_ != other.bucket_)
                return false;
            return !owner_ || bucket_ == owner_->buckets_.size() || current_ == other.current_;
        }

        template<bool OtherConst>
        bool operator!=(const BasicIterator<OtherConst>& other) const { return !(*this == other); }
    };

public:
    typedef BasicIterator<false> iterator;
    typedef BasicIterator<true> const_iterator;

private:
    allocator_type allocator_;
    hasher hash_;
    key_equal equal_;
    float max_load_factor_;
    size_type size_;
    bucket_array_type buckets_;

    bucket_array_type makeBuckets(size_type count) const {
        bucket_array_type result{bucket_allocator_type(allocator_)};
        result.reserve(count);
        for (size_type i = 0; i < count; ++i)
            result.push_back(bucket_type(allocator_));
        return result;
    }

    size_type bucketIndex(const key_type& key, size_type count) const {
        return static_cast<size_type>(hash_(key) % count);
    }

    typename bucket_type::iterator findLocal(size_type bucket, const key_type& key) {
        typename bucket_type::iterator it = buckets_[bucket].begin();
        while (it != buckets_[bucket].end() && !equal_(it->first, key))
            ++it;
        return it;
    }

    typename bucket_type::const_iterator findLocal(size_type bucket, const key_type& key) const {
        typename bucket_type::const_iterator it = buckets_[bucket].begin();
        while (it != buckets_[bucket].end() && !equal_(it->first, key))
            ++it;
        return it;
    }

    void ensureInsertCapacity() {
        if (static_cast<float>(size_ + 1) > max_load_factor_ * buckets_.size())
            rehash(buckets_.size() * 2 + 1);
    }

public:
    explicit ChainedHashMap(
        size_type bucketCount = 17,
        const hasher& hash = hasher(),
        const key_equal& equal = key_equal(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), hash_(hash), equal_(equal), max_load_factor_(1.0f),
        size_(0), buckets_(makeBuckets(bucketCount ? bucketCount : 1)) {}

    ChainedHashMap(const ChainedHashMap&) = default;
    ChainedHashMap& operator=(const ChainedHashMap&) = default;

    // 移动后源表保留至少一个空桶，可继续执行 find、put 和 reserve。
    ChainedHashMap(ChainedHashMap&& other)
        : ChainedHashMap(
            other.bucket_count(), other.hash_, other.equal_, other.allocator_
        ) {
        swap(other);
    }

    ChainedHashMap& operator=(ChainedHashMap&& other) {
        if (this != &other) {
            ChainedHashMap replacement(std::move(other));
            swap(replacement);
        }
        return *this;
    }

    allocator_type get_allocator() const { return allocator_; }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    size_type bucket_count() const { return buckets_.size(); }
    float load_factor() const { return buckets_.empty() ? 0.0f : static_cast<float>(size_) / buckets_.size(); }
    float max_load_factor() const { return max_load_factor_; }

    void max_load_factor(float value) {
        if (!(value > 0.0f) || !std::isfinite(value))
            throw std::invalid_argument("ChainedHashMap max_load_factor must be finite and positive");
        max_load_factor_ = value;
        reserve(size_);
    }

    iterator begin() {
        return iterator(this, 0, buckets_[0].begin());
    }
    const_iterator begin() const {
        return const_iterator(this, 0, buckets_[0].begin());
    }
    iterator end() {
        return iterator(this, buckets_.size(), typename bucket_type::iterator());
    }
    const_iterator end() const {
        return const_iterator(this, buckets_.size(), typename bucket_type::const_iterator());
    }

    iterator find(const key_type& key) {
        const size_type bucket = bucketIndex(key, buckets_.size());
        typename bucket_type::iterator local = findLocal(bucket, key);
        return local == buckets_[bucket].end() ? end() : iterator(this, bucket, local);
    }

    const_iterator find(const key_type& key) const {
        const size_type bucket = bucketIndex(key, buckets_.size());
        typename bucket_type::const_iterator local = findLocal(bucket, key);
        return local == buckets_[bucket].end() ? end() : const_iterator(this, bucket, local);
    }

    mapped_type* get(const key_type& key) {
        iterator found = find(key);
        return found == end() ? 0 : &found->second;
    }
    const mapped_type* get(const key_type& key) const {
        const_iterator found = find(key);
        return found == end() ? 0 : &found->second;
    }

    bool contains(const key_type& key) const { return find(key) != end(); }

    std::pair<iterator, bool> insert(const value_type& value) {
        iterator existing = find(value.first);
        if (existing != end())
            return std::make_pair(existing, false);
        ensureInsertCapacity();
        const size_type bucket = bucketIndex(value.first, buckets_.size());
        buckets_[bucket].push_back(value);
        ++size_;
        typename bucket_type::iterator inserted = buckets_[bucket].end();
        --inserted;
        return std::make_pair(iterator(this, bucket, inserted), true);
    }

    template<typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& value) {
        iterator existing = find(key);
        if (existing != end()) {
            existing->second = std::forward<M>(value);
            return std::make_pair(existing, false);
        }
        ensureInsertCapacity();
        const size_type bucket = bucketIndex(key, buckets_.size());
        buckets_[bucket].emplace_back(key, std::forward<M>(value));
        ++size_;
        typename bucket_type::iterator inserted = buckets_[bucket].end();
        --inserted;
        return std::make_pair(iterator(this, bucket, inserted), true);
    }

    bool put(const key_type& key, const mapped_type& value) {
        return insert_or_assign(key, value).second;
    }

    bool remove(const key_type& key) { return erase(key) != 0; }

    size_type erase(const key_type& key) {
        const size_type bucket = bucketIndex(key, buckets_.size());
        typename bucket_type::iterator local = findLocal(bucket, key);
        if (local == buckets_[bucket].end())
            return 0;
        buckets_[bucket].erase(local);
        --size_;
        return 1;
    }

    mapped_type& operator[](const key_type& key) {
        return insert_or_assign(key, mapped_type()).first->second;
    }

    mapped_type& at(const key_type& key) {
        mapped_type* value = get(key);
        if (!value)
            throw std::out_of_range("ChainedHashMap key not found");
        return *value;
    }
    const mapped_type& at(const key_type& key) const {
        const mapped_type* value = get(key);
        if (!value)
            throw std::out_of_range("ChainedHashMap key not found");
        return *value;
    }

    void clear() {
        for (size_type i = 0; i < buckets_.size(); ++i)
            buckets_[i].clear();
        size_ = 0;
    }

    void reserve(size_type count) {
        const size_type required = static_cast<size_type>(
            static_cast<float>(count) / max_load_factor_
        ) + 1;
        if (required > buckets_.size())
            rehash(required);
    }

    // 交换桶所有权；不可传播且不等 allocator 禁止交换，避免错误释放节点。
    void swap(ChainedHashMap& other) {
        typedef std::allocator_traits<allocator_type> traits;
        const bool propagate = traits::propagate_on_container_swap::value;
        if (!propagate && allocator_ != other.allocator_)
            throw std::logic_error("ChainedHashMap::swap requires equal allocators");
        using std::swap;
        if (propagate)
            swap(allocator_, other.allocator_);
        swap(hash_, other.hash_);
        swap(equal_, other.equal_);
        swap(max_load_factor_, other.max_load_factor_);
        swap(size_, other.size_);
        buckets_.swap(other.buckets_);
    }

    void rehash(size_type count) {
        if (count < 1)
            count = 1;
        const size_type minimum = static_cast<size_type>(
            static_cast<float>(size_) / max_load_factor_
        ) + 1;
        if (count < minimum)
            count = minimum;
        if (count == buckets_.size())
            return;

        bucket_array_type replacement = makeBuckets(count);
        std::vector<size_type> destinations;
        destinations.reserve(size_);
        for (size_type bucket = 0; bucket < buckets_.size(); ++bucket) {
            for (typename bucket_type::const_iterator it = buckets_[bucket].begin();
                 it != buckets_[bucket].end(); ++it)
                destinations.push_back(bucketIndex(it->first, count));
        }

        size_type destinationIndex = 0;
        for (size_type bucket = 0; bucket < buckets_.size(); ++bucket) {
            while (!buckets_[bucket].empty()) {
                const size_type target = destinations[destinationIndex++];
                replacement[target].splice(
                    replacement[target].end(), buckets_[bucket], buckets_[bucket].begin()
                );
            }
        }
        buckets_.swap(replacement);
    }
};

} // namespace container
} // namespace dsa

#endif
