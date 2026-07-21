#ifndef DSA_CONTAINER_TREE_B_STAR_TREE_H
#define DSA_CONTAINER_TREE_B_STAR_TREE_H

#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include <dsa/container/tree/detail/PackedBPlusTree.h>

namespace dsa {
namespace container {

// B* 家族有序集合。节点按 2/3 目标占用率事务重建；根及其直接孩子允许占用率例外。
template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T> >
class BStarTree {
    typedef unsigned char unit_type;
    typedef std::pair<const T, unit_type> stored_type;
    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<stored_type> stored_allocator_type;
    typedef detail::PackedBPlusTree<T, unit_type, Compare, stored_allocator_type, 2, 3> implementation_type;

    Compare compare_;
    implementation_type implementation_;

public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

    class const_iterator {
        friend class BStarTree;
        typename implementation_type::const_iterator iterator_;
        explicit const_iterator(typename implementation_type::const_iterator iterator)
            : iterator_(iterator) {}
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;
        const_iterator() : iterator_() {}
        reference operator*() const { return iterator_->first; }
        pointer operator->() const { return &iterator_->first; }
        const_iterator& operator++() { ++iterator_; return *this; }
        friend bool operator==(const const_iterator& left, const const_iterator& right) {
            return left.iterator_ == right.iterator_;
        }
        friend bool operator!=(const const_iterator& left, const const_iterator& right) {
            return !(left == right);
        }
    };

    typedef const_iterator iterator;

    explicit BStarTree(
        size_type order = 32,
        const value_compare& compare = value_compare(),
        const allocator_type& allocator = allocator_type()
    ) : compare_(compare), implementation_(order, compare, stored_allocator_type(allocator)) {}

    bool insert(const value_type& value) {
        return implementation_.insert(value, static_cast<unit_type>(0));
    }
    size_type erase(const value_type& value) { return implementation_.erase(value); }
    bool remove(const value_type& value) { return erase(value) != 0; }
    bool contains(const value_type& value) const { return implementation_.contains(value); }
    iterator find(const value_type& value) const { return iterator(implementation_.find(value)); }
    iterator lower_bound(const value_type& value) const { return iterator(implementation_.lower_bound(value)); }
    iterator begin() const { return iterator(implementation_.begin()); }
    iterator end() const { return iterator(implementation_.end()); }
    bool empty() const { return implementation_.empty(); }
    size_type size() const { return implementation_.size(); }
    size_type order() const { return implementation_.order(); }
    void clear() { implementation_.clear(); }
    bool validate() const { return implementation_.validate(); }

    template<typename Result, typename Aggregate>
    Result range_aggregate(
        const value_type& first,
        const value_type& last,
        Result identity,
        Aggregate aggregate
    ) const {
        iterator it = lower_bound(first);
        while (it != end() && !compare_(last, *it)) {
            identity = aggregate(identity, *it);
            ++it;
        }
        return identity;
    }
};

} // namespace container
} // namespace dsa

#endif
