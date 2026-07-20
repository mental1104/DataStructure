#ifndef DSA_ALGORITHM_STRING_SORT_H
#define DSA_ALGORITHM_STRING_SORT_H

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "String.h"

namespace dsa {
namespace algorithm {

namespace detail {

/// 返回序列指定深度的字节值；越过末尾时返回 -1 作为短串哨兵。
template<typename Sequence>
int stringSymbolAt(const Sequence& sequence, std::size_t depth) {
    if (depth >= sequenceSize(sequence))
        return -1;
    return static_cast<int>(static_cast<unsigned char>(sequenceAt(sequence, depth)));
}

/// 从 depth 开始比较两个字符串序列的字典序，不构造临时子串。
template<typename Left, typename Right>
bool suffixLess(const Left& left, const Right& right, std::size_t depth) {
    const std::size_t leftSize = sequenceSize(left);
    const std::size_t rightSize = sequenceSize(right);
    std::size_t index = depth;
    while (index < leftSize && index < rightSize) {
        if (sequenceAt(left, index) < sequenceAt(right, index))
            return true;
        if (sequenceAt(right, index) < sequenceAt(left, index))
            return false;
        ++index;
    }
    return leftSize < rightSize;
}

/// 对小区间执行从 depth 开始比较的插入排序。
template<typename Collection>
void insertionStringSort(
    Collection& values,
    std::ptrdiff_t low,
    std::ptrdiff_t high,
    std::size_t depth
) {
    for (std::ptrdiff_t index = low + 1; index <= high; ++index) {
        for (std::ptrdiff_t current = index;
             current > low && suffixLess(
                 values[static_cast<std::size_t>(current)],
                 values[static_cast<std::size_t>(current - 1)],
                 depth
             );
             --current) {
            using std::swap;
            swap(
                values[static_cast<std::size_t>(current)],
                values[static_cast<std::size_t>(current - 1)]
            );
        }
    }
}

/// MSD 递归核心；辅助数组只依赖元素值类型，不依赖具体集合实现。
template<typename Collection, typename Value>
void msdStringSortImpl(
    Collection& values,
    std::vector<Value>& auxiliary,
    std::ptrdiff_t low,
    std::ptrdiff_t high,
    std::size_t depth
) {
    static const std::ptrdiff_t cutoff = 12;
    static const std::size_t alphabet = 256;
    if (high <= low)
        return;
    if (high - low <= cutoff) {
        insertionStringSort(values, low, high, depth);
        return;
    }

    std::vector<std::size_t> count(alphabet + 2, 0);
    for (std::ptrdiff_t index = low; index <= high; ++index) {
        const int symbol = stringSymbolAt(
            values[static_cast<std::size_t>(index)],
            depth
        );
        ++count[static_cast<std::size_t>(symbol + 2)];
    }
    for (std::size_t index = 0; index < alphabet + 1; ++index)
        count[index + 1] += count[index];

    const std::vector<std::size_t> starts = count;
    for (std::ptrdiff_t index = low; index <= high; ++index) {
        const int symbol = stringSymbolAt(
            values[static_cast<std::size_t>(index)],
            depth
        );
        auxiliary[count[static_cast<std::size_t>(symbol + 1)]++] =
            values[static_cast<std::size_t>(index)];
    }
    for (std::ptrdiff_t index = low; index <= high; ++index) {
        values[static_cast<std::size_t>(index)] = auxiliary[
            static_cast<std::size_t>(index - low)
        ];
    }

    for (std::size_t symbol = 0; symbol < alphabet; ++symbol) {
        const std::ptrdiff_t bucketLow = low + static_cast<std::ptrdiff_t>(starts[symbol + 1]);
        const std::ptrdiff_t bucketHigh = low + static_cast<std::ptrdiff_t>(starts[symbol + 2]) - 1;
        if (bucketLow <= bucketHigh)
            msdStringSortImpl(values, auxiliary, bucketLow, bucketHigh, depth + 1);
    }
}

/// 三向字符串快排递归核心。
template<typename Collection>
void quick3StringSortImpl(
    Collection& values,
    std::ptrdiff_t low,
    std::ptrdiff_t high,
    std::size_t depth
) {
    if (high <= low)
        return;

    std::ptrdiff_t less = low;
    std::ptrdiff_t greater = high;
    const int pivot = stringSymbolAt(
        values[static_cast<std::size_t>(low)],
        depth
    );
    std::ptrdiff_t index = low + 1;
    while (index <= greater) {
        const int current = stringSymbolAt(
            values[static_cast<std::size_t>(index)],
            depth
        );
        if (current < pivot) {
            using std::swap;
            swap(
                values[static_cast<std::size_t>(less++)],
                values[static_cast<std::size_t>(index++)]
            );
        } else if (pivot < current) {
            using std::swap;
            swap(
                values[static_cast<std::size_t>(index)],
                values[static_cast<std::size_t>(greater--)]
            );
        } else {
            ++index;
        }
    }

    quick3StringSortImpl(values, low, less - 1, depth);
    if (pivot >= 0)
        quick3StringSortImpl(values, less, greater, depth + 1);
    quick3StringSortImpl(values, greater + 1, high, depth);
}

} // namespace detail

/// 对等长字符串序列执行稳定 LSD 基数排序；元素不足 width 时抛出异常。
template<typename Collection>
void lsdStringSort(Collection& values, std::size_t width) {
    static const std::size_t alphabet = 256;
    typedef typename Collection::value_type value_type;
    const std::size_t count = static_cast<std::size_t>(values.size());
    std::vector<value_type> auxiliary(count);

    for (std::size_t index = 0; index < count; ++index) {
        if (sequenceSize(values[index]) < width)
            throw std::invalid_argument("lsdStringSort requires fixed-width strings");
    }

    for (std::size_t depth = width; depth > 0; --depth) {
        const std::size_t currentDepth = depth - 1;
        std::vector<std::size_t> frequency(alphabet + 1, 0);
        for (std::size_t index = 0; index < count; ++index) {
            const unsigned char symbol = static_cast<unsigned char>(
                sequenceAt(values[index], currentDepth)
            );
            ++frequency[static_cast<std::size_t>(symbol) + 1];
        }
        for (std::size_t symbol = 0; symbol < alphabet; ++symbol)
            frequency[symbol + 1] += frequency[symbol];
        for (std::size_t index = 0; index < count; ++index) {
            const unsigned char symbol = static_cast<unsigned char>(
                sequenceAt(values[index], currentDepth)
            );
            auxiliary[frequency[static_cast<std::size_t>(symbol)]++] = values[index];
        }
        for (std::size_t index = 0; index < count; ++index)
            values[index] = auxiliary[index];
    }
}

/// 对变长字符串序列执行 MSD 基数排序。
template<typename Collection>
void msdStringSort(Collection& values) {
    typedef typename Collection::value_type value_type;
    const std::size_t count = static_cast<std::size_t>(values.size());
    if (count < 2)
        return;
    std::vector<value_type> auxiliary(count);
    detail::msdStringSortImpl(
        values,
        auxiliary,
        0,
        static_cast<std::ptrdiff_t>(count) - 1,
        0
    );
}

/// 对变长字符串序列执行三向字符串快排。
template<typename Collection>
void quick3StringSort(Collection& values) {
    const std::size_t count = static_cast<std::size_t>(values.size());
    if (count < 2)
        return;
    detail::quick3StringSortImpl(
        values,
        0,
        static_cast<std::ptrdiff_t>(count) - 1,
        0
    );
}

} // namespace algorithm
} // namespace dsa

#endif
