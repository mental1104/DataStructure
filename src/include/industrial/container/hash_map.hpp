#ifndef DSA_INDUSTRIAL_CONTAINER_HASH_MAP_HPP
#define DSA_INDUSTRIAL_CONTAINER_HASH_MAP_HPP

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../tree/rb_tree.hpp"

namespace dsa {
namespace industrial {

template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class TreeCompare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key, T> >
>
class hash_map {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const Key, T> value_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef TreeCompare tree_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

private:
    static const size_type default_bucket_count = 16;
    static const size_type treeify_threshold = 8;

    struct value_compare {
        explicit value_compare(const tree_compare& compare = tree_compare())
            : compare(compare) {}

        bool operator()(const value_type& lhs, const value_type& rhs) const {
            return compare(lhs.first, rhs.first);
        }

        tree_compare compare;
    };

    struct lookup_compare {
        explicit lookup_compare(const tree_compare& compare = tree_compare())
            : compare(compare) {}

        bool operator()(const value_type& lhs, const key_type& rhs) const {
            return compare(lhs.first, rhs);
        }

        bool operator()(const key_type& lhs, const value_type& rhs) const {
            return compare(lhs, rhs.first);
        }

        tree_compare compare;
    };

    typedef std::list<value_type, allocator_type> list_type;
    typedef rb_tree<value_type, value_compare, allocator_type> tree_type;

    struct bucket {
        bucket(const allocator_type& allocator, const value_compare& compare)
            : is_tree(false),
              list(allocator),
              tree(compare, allocator) {}

        size_type size() const {
            return is_tree ? tree.size() : list.size();
        }

        void clear() {
            list.clear();
            tree.clear();
            is_tree = false;
        }

        bool is_tree;
        list_type list;
        tree_type tree;
    };

public:
    explicit hash_map(
        size_type bucket_count = default_bucket_count,
        const hasher& hash = hasher(),
        const key_equal& equal = key_equal(),
        const tree_compare& tree_comp = tree_compare(),
        const allocator_type& allocator = allocator_type())
        : buckets_(),
          size_(0),
          hash_(hash),
          equal_(equal),
          tree_comp_(tree_comp),
          allocator_(allocator),
          max_load_factor_(1.0f) {
        initialize_buckets(bucket_count);
    }

    hash_map(const hash_map&) = delete;
    hash_map& operator=(const hash_map&) = delete;

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    size_type bucket_count() const { return buckets_.size(); }

    float load_factor() const {
        return buckets_.empty()
            ? 0.0f
            : static_cast<float>(size_) / static_cast<float>(buckets_.size());
    }

    float max_load_factor() const { return max_load_factor_; }

    void max_load_factor(float value) {
        if (value <= 0.0f) {
            throw std::invalid_argument("max_load_factor must be positive");
        }
        max_load_factor_ = value;
    }

    bool insert(const value_type& value) {
        return insert_value(value);
    }

    bool insert(value_type&& value) {
        return insert_value(std::move(value));
    }

    template <class... Args>
    bool emplace(Args&&... args) {
        value_type value(std::forward<Args>(args)...);
        return insert(std::move(value));
    }

    mapped_type* find(const key_type& key) {
        if (buckets_.empty()) {
            return 0;
        }

        bucket& selected = *buckets_[bucket_index(key, buckets_.size())];
        if (selected.is_tree) {
            typename tree_type::iterator it =
                selected.tree.find_as(key, lookup_compare(tree_comp_));
            return it == selected.tree.end() ? 0 : &(*it).second;
        }

        for (typename list_type::iterator it = selected.list.begin();
             it != selected.list.end();
             ++it) {
            if (equal_(it->first, key)) {
                return &it->second;
            }
        }
        return 0;
    }

    const mapped_type* find(const key_type& key) const {
        if (buckets_.empty()) {
            return 0;
        }

        const bucket& selected = *buckets_[bucket_index(key, buckets_.size())];
        if (selected.is_tree) {
            typename tree_type::const_iterator it =
                selected.tree.find_as(key, lookup_compare(tree_comp_));
            return it == selected.tree.end() ? 0 : &(*it).second;
        }

        for (typename list_type::const_iterator it = selected.list.begin();
             it != selected.list.end();
             ++it) {
            if (equal_(it->first, key)) {
                return &it->second;
            }
        }
        return 0;
    }

    bool contains(const key_type& key) const {
        return find(key) != 0;
    }

    mapped_type& operator[](const key_type& key) {
        mapped_type* found = find(key);
        if (found) {
            return *found;
        }

        emplace(key, mapped_type());
        return *find(key);
    }

    mapped_type& at(const key_type& key) {
        mapped_type* found = find(key);
        if (!found) {
            throw std::out_of_range("hash_map::at");
        }
        return *found;
    }

    const mapped_type& at(const key_type& key) const {
        const mapped_type* found = find(key);
        if (!found) {
            throw std::out_of_range("hash_map::at");
        }
        return *found;
    }

    size_type erase(const key_type& key) {
        if (buckets_.empty()) {
            return 0;
        }

        bucket& selected = *buckets_[bucket_index(key, buckets_.size())];
        if (selected.is_tree) {
            typename tree_type::iterator it =
                selected.tree.find_as(key, lookup_compare(tree_comp_));
            if (it == selected.tree.end()) {
                return 0;
            }
            selected.tree.erase(it);
            --size_;
            return 1;
        }

        for (typename list_type::iterator it = selected.list.begin();
             it != selected.list.end();
             ++it) {
            if (equal_(it->first, key)) {
                selected.list.erase(it);
                --size_;
                return 1;
            }
        }
        return 0;
    }

    void clear() {
        for (size_type i = 0; i < buckets_.size(); ++i) {
            buckets_[i]->clear();
        }
        size_ = 0;
    }

    void reserve(size_type element_count) {
        size_type required = bucket_count_for_elements(element_count);
        if (required > buckets_.size()) {
            rehash(required);
        }
    }

    void rehash(size_type requested_bucket_count) {
        size_type required = bucket_count_for_elements(size_);
        size_type new_count = requested_bucket_count < required
            ? required
            : requested_bucket_count;
        if (new_count == 0) {
            new_count = 1;
        }
        if (new_count == buckets_.size()) {
            return;
        }

        std::vector<std::unique_ptr<bucket> > new_buckets;
        fill_buckets(new_buckets, new_count);

        for (size_type i = 0; i < buckets_.size(); ++i) {
            const bucket& current = *buckets_[i];
            if (current.is_tree) {
                for (typename tree_type::const_iterator it = current.tree.begin();
                     it != current.tree.end();
                     ++it) {
                    insert_into_buckets(new_buckets, *it);
                }
            } else {
                for (typename list_type::const_iterator it = current.list.begin();
                     it != current.list.end();
                     ++it) {
                    insert_into_buckets(new_buckets, *it);
                }
            }
        }

        buckets_.swap(new_buckets);
    }

    size_type bucket_size(size_type index) const {
        return buckets_[index]->size();
    }

    bool bucket_is_tree(size_type index) const {
        return buckets_[index]->is_tree;
    }

private:
    std::unique_ptr<bucket> make_bucket() const {
        return std::unique_ptr<bucket>(
            new bucket(allocator_, value_compare(tree_comp_)));
    }

    void initialize_buckets(size_type count) {
        if (count == 0) {
            count = 1;
        }
        fill_buckets(buckets_, count);
    }

    void fill_buckets(
        std::vector<std::unique_ptr<bucket> >& target,
        size_type count) const {
        target.reserve(count);
        for (size_type i = 0; i < count; ++i) {
            target.push_back(make_bucket());
        }
    }

    size_type bucket_count_for_elements(size_type element_count) const {
        size_type count = static_cast<size_type>(
            static_cast<float>(element_count) / max_load_factor_);
        if (static_cast<float>(count) * max_load_factor_ <
            static_cast<float>(element_count)) {
            ++count;
        }
        return count == 0 ? 1 : count;
    }

    size_type bucket_index(const key_type& key, size_type count) const {
        return hash_(key) % count;
    }

    template <class Value>
    bool insert_value(Value&& value) {
        const key_type& key = value.first;
        if (contains(key)) {
            return false;
        }

        ensure_capacity_for_insert();
        bucket& selected = *buckets_[bucket_index(key, buckets_.size())];
        insert_into_bucket(selected, std::forward<Value>(value));
        ++size_;
        return true;
    }

    void ensure_capacity_for_insert() {
        if (buckets_.empty()) {
            rehash(default_bucket_count);
            return;
        }

        if (static_cast<float>(size_ + 1) >
            max_load_factor_ * static_cast<float>(buckets_.size())) {
            rehash(buckets_.size() * 2);
        }
    }

    template <class Value>
    void insert_into_bucket(bucket& selected, Value&& value) {
        if (selected.is_tree) {
            selected.tree.insert(std::forward<Value>(value));
            return;
        }

        selected.list.push_back(std::forward<Value>(value));
        if (selected.list.size() >= treeify_threshold) {
            treeify(selected);
        }
    }

    void insert_into_buckets(
        std::vector<std::unique_ptr<bucket> >& target,
        const value_type& value) const {
        bucket& selected = *target[bucket_index(value.first, target.size())];
        if (selected.is_tree) {
            selected.tree.insert(value);
            return;
        }

        selected.list.push_back(value);
        if (selected.list.size() >= treeify_threshold) {
            treeify(selected);
        }
    }

    void treeify(bucket& selected) const {
        if (selected.is_tree) {
            return;
        }

        tree_type new_tree(value_compare(tree_comp_), allocator_);
        for (typename list_type::const_iterator it = selected.list.begin();
             it != selected.list.end();
             ++it) {
            new_tree.insert(*it);
        }

        selected.tree.swap(new_tree);
        selected.list.clear();
        selected.is_tree = true;
    }

    std::vector<std::unique_ptr<bucket> > buckets_;
    size_type size_;
    hasher hash_;
    key_equal equal_;
    tree_compare tree_comp_;
    allocator_type allocator_;
    float max_load_factor_;
};

}  // namespace industrial
}  // namespace dsa

#endif  // DSA_INDUSTRIAL_CONTAINER_HASH_MAP_HPP
