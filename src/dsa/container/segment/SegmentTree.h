#ifndef DSA_CONTAINER_SEGMENT_SEGMENT_TREE_H
#define DSA_CONTAINER_SEGMENT_SEGMENT_TREE_H

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>

#include <dsa/container/vector/Vector.h>

#include <dsa/core/segment/SegmentTreeAlgorithm.h>

namespace dsa {
namespace container {

// 默认区间合并运算：加法。自定义 Operation 必须满足结合律，并由调用方提供幺元。
template<typename T>
struct SegmentSum {
    T operator()(const T& left, const T& right) const { return left + right; }
};

// allocator-aware 迭代线段树，支持 O(n) 构建与 O(log n) 单点更新、半开区间查询。
template<
    typename T,
    typename Operation = SegmentSum<T>,
    typename Allocator = std::allocator<T>
>
class SegmentTree {
public:
    typedef T value_type;
    typedef Operation operation_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;

private:
    typedef dsa::container::Vector<value_type, allocator_type> storage_type;

    class Access {
    public:
        typedef typename SegmentTree::value_type value_type;
        typedef typename SegmentTree::size_type size_type;

        explicit Access(SegmentTree& tree) : tree_(tree) {}
        explicit Access(const SegmentTree& tree) : tree_(const_cast<SegmentTree&>(tree)) {}

        void reset(
            size_type valueCount,
            size_type leafCount,
            const value_type& identity
        ) {
            tree_.value_count_ = valueCount;
            tree_.leaf_count_ = leafCount;
            tree_.storage_.assign(leafCount << 1, identity);
        }

        void assign(size_type index, const value_type& value) {
            tree_.storage_[index] = value;
        }

        const value_type& value(size_type index) const {
            return tree_.storage_[index];
        }

        size_type valueCount() const { return tree_.value_count_; }
        size_type leafCount() const { return tree_.leaf_count_; }

    private:
        SegmentTree& tree_;
    };

    typedef dsa::core::SegmentTreeAlgorithm<Access> algorithm_type;

    allocator_type allocator_;
    operation_type operation_;
    value_type identity_;
    size_type value_count_;
    size_type leaf_count_;
    storage_type storage_;

public:
    explicit SegmentTree(
        const operation_type& operation = operation_type(),
        const value_type& identity = value_type(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), operation_(operation), identity_(identity),
        value_count_(0), leaf_count_(1), storage_(allocator) {
        storage_.assign(2, identity_);
    }

    template<typename ForwardIterator>
    SegmentTree(
        ForwardIterator first,
        ForwardIterator last,
        const operation_type& operation = operation_type(),
        const value_type& identity = value_type(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), operation_(operation), identity_(identity),
        value_count_(0), leaf_count_(1), storage_(allocator) {
        build(first, last);
    }

    SegmentTree(
        std::initializer_list<value_type> values,
        const operation_type& operation = operation_type(),
        const value_type& identity = value_type(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), operation_(operation), identity_(identity),
        value_count_(0), leaf_count_(1), storage_(allocator) {
        build(values.begin(), values.end());
    }

    template<typename ForwardIterator>
    void build(ForwardIterator first, ForwardIterator last) {
        Access access(*this);
        algorithm_type::build(access, first, last, operation_, identity_);
    }

    void update(size_type position, const value_type& value) {
        Access access(*this);
        algorithm_type::update(access, position, value, operation_);
    }

    value_type query(size_type first, size_type last) const {
        const Access access(*this);
        return algorithm_type::query(access, first, last, operation_, identity_);
    }

    bool empty() const { return value_count_ == 0; }
    size_type size() const { return value_count_; }
    size_type leaf_capacity() const { return leaf_count_; }
    operation_type operation() const { return operation_; }
    const value_type& identity() const { return identity_; }
    allocator_type get_allocator() const { return allocator_; }
};

} // namespace container
} // namespace dsa

#endif
