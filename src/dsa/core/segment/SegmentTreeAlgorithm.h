#ifndef DSA_CORE_SEGMENT_SEGMENT_TREE_ALGORITHM_H
#define DSA_CORE_SEGMENT_SEGMENT_TREE_ALGORITHM_H

#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace dsa {
namespace core {

// 线段树共享流程。Access 负责存储和生命周期，算法层只编排构建、更新与查询。
template<typename Access>
class SegmentTreeAlgorithm {
public:
    typedef typename Access::size_type size_type;
    typedef typename Access::value_type value_type;

    static size_type nextPowerOfTwo(size_type count) {
        size_type result = 1;
        while (result < count)
            result <<= 1;
        return result;
    }

    // 构建要求至少为前向迭代器，以便先统计长度再遍历写入。
    template<typename ForwardIterator, typename Operation>
    static void build(
        Access& access,
        ForwardIterator first,
        ForwardIterator last,
        const Operation& operation,
        const value_type& identity
    ) {
        size_type count = 0;
        for (ForwardIterator probe = first; probe != last; ++probe)
            ++count;
        const size_type leaves = nextPowerOfTwo(count);
        access.reset(count, leaves, identity);

        size_type index = 0;
        for (; first != last; ++first, ++index)
            access.assign(leaves + index, *first);

        for (size_type current = leaves; current > 1; --current) {
            const size_type parent = current - 1;
            access.assign(
                parent,
                operation(access.value(parent << 1), access.value(parent << 1 | 1))
            );
        }
    }

    template<typename Operation>
    static void update(
        Access& access,
        size_type position,
        const value_type& value,
        const Operation& operation
    ) {
        if (position >= access.valueCount())
            throw std::out_of_range("SegmentTree::update position out of range");

        size_type current = access.leafCount() + position;
        access.assign(current, value);
        while (current > 1) {
            current >>= 1;
            access.assign(
                current,
                operation(access.value(current << 1), access.value(current << 1 | 1))
            );
        }
    }

    template<typename Operation>
    static value_type query(
        const Access& access,
        size_type first,
        size_type last,
        const Operation& operation,
        const value_type& identity
    ) {
        if (first > last || last > access.valueCount())
            throw std::out_of_range("SegmentTree::query range out of range");

        value_type leftResult(identity);
        value_type rightResult(identity);
        size_type left = access.leafCount() + first;
        size_type right = access.leafCount() + last;
        while (left < right) {
            if (left & 1)
                leftResult = operation(leftResult, access.value(left++));
            if (right & 1)
                rightResult = operation(access.value(--right), rightResult);
            left >>= 1;
            right >>= 1;
        }
        return operation(leftResult, rightResult);
    }
};

} // namespace core
} // namespace dsa

#endif
