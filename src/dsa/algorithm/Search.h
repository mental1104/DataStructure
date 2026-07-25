#ifndef DSA_ALGORITHM_SEARCH_H
#define DSA_ALGORITHM_SEARCH_H

#include <iterator>

#include "../functional/Compare.h"

namespace dsa {
namespace algorithm {

/**
 * 从区间末尾向前寻找最后一个等于 value 的元素。
 *
 * 区间采用 [first, last) 形式；找到时返回对应迭代器，
 * 未找到时返回 last。算法需要双向迭代器，因为搜索从尾部开始。
 *
 * 时间复杂度 O(n)，额外空间复杂度 O(1)。
 */
template<
    typename BidirectionalIt,
    typename Value,
    typename Equal = DefaultEqual
>
BidirectionalIt findLast(
    BidirectionalIt first,
    BidirectionalIt last,
    const Value& value,
    Equal equal = Equal()
) {
    BidirectionalIt current = last;
    while (current != first) {
        --current;
        if (equal(*current, value))
            return current;
    }
    return last;
}

/**
 * 在有序随机访问区间 [first, last) 中执行二分查找，
 * 返回第一个严格大于 value 的元素；不存在时返回 last。
 *
 * std::iterator_traits 统一取得不同迭代器定义的 difference_type，
 * 该类型用于表达两个迭代器之间的有符号距离，避免把距离固定为 int。
 *
 * 示例：
 *   [1, 3, 3, 5]，value = 3
 *            ^
 *   返回元素 5 的位置。
 *
 * 时间复杂度 O(log n)，额外空间复杂度 O(1)。
 */
template<
    typename RandomIt,
    typename Value,
    typename Compare = DefaultLess
>
RandomIt upperBound(
    RandomIt first,
    RandomIt last,
    const Value& value,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    while (first != last) {
        // 当前候选区间长度，以及位于区间中点的探测位置。
        const Difference count = last - first;
        RandomIt middle = first + (count >> 1);

        if (compare(value, *middle))
            // value < *middle：第一个大于 value 的位置只能在左半区。
            last = middle;
        else
            // *middle <= value：跳过 middle，继续在右半区寻找。
            first = middle + 1;
    }
    return first;
}

/**
 * 使用斐波那契分割在有序随机访问区间中寻找 upper bound。
 *
 * 返回语义与 upperBound 相同：返回第一个严格大于 value 的元素，
 * 不存在时返回 last。区别在于它使用相邻斐波那契数确定探测位置，
 * 而不是每轮从中点将区间对半划分。
 *
 * 时间复杂度 O(log n)，额外空间复杂度 O(1)。
 */
template<
    typename RandomIt,
    typename Value,
    typename Compare = DefaultLess
>
RandomIt fibonacciUpperBound(
    RandomIt first,
    RandomIt last,
    const Value& value,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    const Difference size = last - first;
    if (size <= 0)
        return first;

    // 构造最小的斐波那契数 current，使其能够覆盖整个搜索区间。
    Difference previousPrevious = 0;
    Difference previous = 1;
    Difference current = previousPrevious + previous;
    while (current < size) {
        previousPrevious = previous;
        previous = current;
        current = previousPrevious + previous;
    }

    // offset 表示已经确认不大于 value 的最右位置；-1 表示尚未确认任何元素。
    Difference offset = -1;
    while (current > 1) {
        // 使用较小的斐波那契分段作为本轮探测偏移，并限制在有效区间内。
        Difference index = offset + previousPrevious;
        if (index >= size)
            index = size - 1;

        if (!compare(value, *(first + index))) {
            // 探测元素 <= value：它仍属于 upper bound 左侧，向右缩小区间。
            current = previous;
            previous = previousPrevious;
            previousPrevious = current - previous;
            offset = index;
        } else {
            // 探测元素 > value：upper bound 位于当前探测点或其左侧。
            current = previousPrevious;
            previous = previous - previousPrevious;
            previousPrevious = current - previous;
        }
    }

    // 最后检查 offset 右侧紧邻元素，确认它是否仍然不大于 value。
    if (
        previous &&
        offset + 1 < size &&
        !compare(value, *(first + offset + 1))
    ) {
        ++offset;
    }

    // offset 是最后一个 <= value 的位置，因此其后一位就是第一个 > value 的位置。
    return first + (offset + 1);
}

} // namespace algorithm
} // namespace dsa

#endif
