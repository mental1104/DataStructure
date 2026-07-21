#ifndef DSA_CONTAINER_STRING_STRING_SYMBOL_TABLE_H
#define DSA_CONTAINER_STRING_STRING_SYMBOL_TABLE_H

#include <cstddef>
#include <utility>
#include <type_traits>

namespace dsa {
namespace container {

// 对 Trie / TernarySearchTrie 等实现提供统一符号表门面，不引入 T() 未命中哨兵。
template<typename Implementation>
class StringSymbolTable {
public:
    typedef Implementation implementation_type;
    typedef typename implementation_type::mapped_type mapped_type;
    typedef typename implementation_type::string_type string_type;
    typedef typename implementation_type::size_type size_type;

    StringSymbolTable() : implementation_() {}
    explicit StringSymbolTable(const implementation_type& implementation)
        : implementation_(implementation) {}
    explicit StringSymbolTable(implementation_type&& implementation)
        : implementation_(std::move(implementation)) {}

    template<typename Key, typename Value>
    bool put(const Key& key, Value&& value) {
        return implementation_.insert(key, std::forward<Value>(value));
    }

    template<typename Key>
    mapped_type* get(const Key& key) { return implementation_.find(key); }
    template<typename Key>
    const mapped_type* get(const Key& key) const { return implementation_.find(key); }
    template<typename Key>
    bool remove(const Key& key) { return implementation_.erase(key); }
    template<typename Key>
    bool contains(const Key& key) const { return implementation_.contains(key); }

    bool empty() const { return implementation_.empty(); }
    size_type size() const { return implementation_.size(); }
    void clear() { implementation_.clear(); }

    template<typename Key>
    string_type longestPrefixOf(const Key& key) const {
        return implementation_.longestPrefixOf(key);
    }
    template<typename Key>
    auto keysWithPrefix(const Key& key) const -> decltype(std::declval<const implementation_type&>().keysWithPrefix(key)) {
        return implementation_.keysWithPrefix(key);
    }
    template<typename Pattern>
    auto keysThatMatch(const Pattern& pattern) const -> decltype(std::declval<const implementation_type&>().keysThatMatch(pattern)) {
        return implementation_.keysThatMatch(pattern);
    }

    implementation_type& implementation() { return implementation_; }
    const implementation_type& implementation() const { return implementation_; }

private:
    implementation_type implementation_;
};

} // namespace container
} // namespace dsa

#endif
