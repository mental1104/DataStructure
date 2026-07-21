#ifndef DSA_CONTAINER_HASH_HASH_MAP_H
#define DSA_CONTAINER_HASH_HASH_MAP_H

#include <functional>
#include <memory>
#include <utility>

#include <dsa/container/hash/Dictionary.h>
#include <dsa/container/hash/HashTable.h>

namespace dsa {
namespace container {

// 旧 HashMap 语义与工业 HashTable 的兼容门面；开放寻址生命周期由 HashTable 负责。
template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >
>
class HashMap : public Dictionary<Key, T> {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef HashTable<Key, T, Hash, KeyEqual, Allocator> table_type;
    typedef typename table_type::value_type value_type;
    typedef typename table_type::size_type size_type;
    typedef typename table_type::iterator iterator;
    typedef typename table_type::const_iterator const_iterator;

    HashMap() : table_() {}
    explicit HashMap(size_type bucketCount) : table_(bucketCount) {}

    bool put(const key_type& key, const mapped_type& value) {
        return table_.insert_or_assign(key, value).second;
    }

    template<typename M>
    bool put(const key_type& key, M&& value) {
        return table_.insert_or_assign(key, std::forward<M>(value)).second;
    }

    mapped_type* get(const key_type& key) {
        iterator found = table_.find(key);
        return found == table_.end() ? 0 : &found->second;
    }

    const mapped_type* get(const key_type& key) const {
        const_iterator found = table_.find(key);
        return found == table_.end() ? 0 : &found->second;
    }

    bool remove(const key_type& key) { return table_.erase(key) != 0; }
    size_type size() const { return table_.size(); }
    bool empty() const { return table_.empty(); }
    bool contains(const key_type& key) const { return table_.contains(key); }
    void clear() { table_.clear(); }

    mapped_type& operator[](const key_type& key) { return table_[key]; }
    mapped_type& at(const key_type& key) { return table_.at(key); }
    const mapped_type& at(const key_type& key) const { return table_.at(key); }

    iterator begin() { return table_.begin(); }
    const_iterator begin() const { return table_.begin(); }
    iterator end() { return table_.end(); }
    const_iterator end() const { return table_.end(); }

    table_type& table() { return table_; }
    const table_type& table() const { return table_; }

private:
    table_type table_;
};

} // namespace container
} // namespace dsa

#endif
