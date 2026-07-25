#ifndef DSA_ALGORITHM_SEQUENCE_H
#define DSA_ALGORITHM_SEQUENCE_H

#include <algorithm>
#include <iterator>
#include <random>
#include <utility>

#include "../functional/Compare.h"

namespace dsa {
namespace algorithm {

/**
 * 使用 Fisher-Yates 算法原地打乱随机访问区间 [first, last)。
 *
 * 每轮只在尚未固定的前缀中随机选择一个元素，与当前尾部元素交换。
 * 时间复杂度 O(n)，额外空间复杂度 O(1)。
 */
template<typename RandomIt, typename UniformRandomBitGenerator>
void shuffle(
    RandomIt first,
    RandomIt last,
    UniformRandomBitGenerator& generator
) {
    typedef typename std::iterator_traits<RandomIt>::difference_type Difference;

    for (Difference count = last - first; count > 1; --count) {
        // 每轮可选择的范围都是 [0, count)，上界随剩余元素数量变化。
        // distribution 只描述本轮范围，随机状态仍由外部 generator 统一维护。
        std::uniform_int_distribution<Difference> distribution(0, count - 1);
        std::iter_swap(first + (count - 1), first + distribution(generator));
    }
}

/// 按顺序访问区间 [first, last) 的每个元素。
template<typename InputIt, typename Visitor>
void forEach(InputIt first, InputIt last, Visitor&& visitor) {
    while (first != last) {
        visitor(*first);
        ++first;
    }
}

/**
 * 稳定移除区间中的重复值，并返回去重后逻辑区间的尾后迭代器。
 *
 * 这是用于展示“已写前缀 + 当前读位置”模型的教学实现。每读取一个
 * 元素，都在线性扫描已写前缀判断是否出现，因此时间复杂度 O(n^2)，
 * 额外空间复杂度 O(1)。大规模数据通常应根据约束选择哈希或排序方案。
 */
template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt deduplicate(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    typedef typename std::iterator_traits<ForwardIt>::value_type Value;

    // 在已经保留的前缀 [first, prefixLast) 中寻找 value。
    const auto findInWrittenPrefix = [first, &equal](
        ForwardIt prefixLast,
        const Value& value
    ) -> ForwardIt {
        ForwardIt current = first;
        while (current != prefixLast && !equal(*current, value))
            ++current;
        return current;
    };

    ForwardIt write = first;
    for (ForwardIt read = first; read != last; ++read) {
        // 未在已写前缀出现时保留当前元素，重复值则直接跳过。
        if (findInWrittenPrefix(write, *read) == write) {
            if (write != read)
                *write = std::move(*read);
            ++write;
        }
    }

    return write;
}

/**
 * 对有序区间执行相邻去重，并返回唯一化后逻辑区间的尾后迭代器。
 *
 * write 指向最后一个保留元素，read 扫描后续元素：
 *
 *   [1, 1, 2, 2, 3]
 *    ^  ^
 *  write read
 *
 * 当 *read 与 *write 不同时，先推进 write，再把新值写入该位置。
 * 时间复杂度 O(n)，额外空间复杂度 O(1)。
 */
template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt uniqueAdjacent(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    if (first == last)
        return last;

    ForwardIt write = first;
    ForwardIt read = first;
    while (++read != last) {
        if (!equal(*write, *read)) {
            ++write;
            if (write != read)
                *write = std::move(*read);
        }
    }

    return ++write;
}

/// 统计相邻元素逆序的次数，用于衡量序列的局部无序程度。
template<typename ForwardIt, typename Compare = DefaultLess>
typename std::iterator_traits<ForwardIt>::difference_type disorderCount(
    ForwardIt first,
    ForwardIt last,
    Compare compare = Compare()
) {
    typedef typename std::iterator_traits<ForwardIt>::difference_type Difference;

    if (first == last)
        return Difference(0);

    Difference count = 0;
    ForwardIt previous = first;
    ForwardIt current = first;
    ++current;

    while (current != last) {
        if (compare(*current, *previous))
            ++count;
        previous = current;
        ++current;
    }

    return count;
}

/**
 * 使用 Boyer-Moore 抵消法寻找多数候选。
 *
 * 相同元素增加票数，不同元素相互抵消；票数归零时，当前元素成为
 * 新候选。若区间中确实存在出现次数超过一半的多数元素，最终候选
 * 一定是该元素。
 *
 * 示例：
 *   [A, B, A, A, C, A, A]
 *   A 与 B 抵消后，剩余的 A 最终成为候选。
 *
 * 本函数只筛选候选，不验证其出现次数是否超过区间长度的一半。
 * 空区间返回 last。时间复杂度 O(n)，额外空间复杂度 O(1)。
 */
template<typename ForwardIt, typename Equal = DefaultEqual>
ForwardIt majorityCandidate(
    ForwardIt first,
    ForwardIt last,
    Equal equal = Equal()
) {
    ForwardIt candidate = last;
    int count = 0;

    for (ForwardIt current = first; current != last; ++current) {
        if (count == 0) {
            // 之前的票已完全抵消，当前元素成为新的候选。
            candidate = current;
            count = 1;
        } else if (equal(*candidate, *current)) {
            // 与候选相同，增加一票。
            ++count;
        } else {
            // 与候选不同，双方各抵消一票。
            --count;
        }
    }

    return candidate;
}

} // namespace algorithm
} // namespace dsa

#endif
