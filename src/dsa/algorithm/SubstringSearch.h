#ifndef DSA_ALGORITHM_SUBSTRING_SEARCH_H
#define DSA_ALGORITHM_SUBSTRING_SEARCH_H

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <dsa/container/vector/Vector.h>

#include "String.h"

namespace dsa {
namespace algorithm {

/// KMP next 表构造策略。
enum class KmpStrategy {
    Basic,
    Improved
};

/// Boyer-Moore 启发式策略。
enum class BoyerMooreStrategy {
    BadCharacter,
    Full
};

/// 默认观察器不产生任何输出，使算法核心不依赖教学打印逻辑。
struct NullSubstringSearchObserver {
    void onKmpTable(const int*, std::size_t) const {}
    void onBadCharacterTable(const int*, std::size_t) const {}
    void onGoodSuffixTable(const int*, std::size_t) const {}
    void onProgress(std::ptrdiff_t, std::ptrdiff_t, const int*, std::size_t) const {}
    void onRabinKarpProgress(std::size_t, std::int64_t, std::int64_t) const {}
    void onPause() const {}
};

/// 构造基础 KMP next 表；空模式返回空表。
template<typename Pattern>
dsa::container::Vector<int> buildKmpNext(const Pattern& pattern) {
    const std::size_t length = sequenceSize(pattern);
    if (length == 0)
        return dsa::container::Vector<int>();

    dsa::container::Vector<int> next(length, 0);
    int patternIndex = 0;
    int fallback = -1;
    next[0] = -1;
    while (patternIndex < static_cast<int>(length) - 1) {
        if (fallback < 0 ||
            sequenceAt(pattern, static_cast<std::size_t>(fallback)) ==
                sequenceAt(pattern, static_cast<std::size_t>(patternIndex))) {
            ++patternIndex;
            ++fallback;
            next[static_cast<std::size_t>(patternIndex)] = fallback;
        } else {
            fallback = next[static_cast<std::size_t>(fallback)];
        }
    }
    return next;
}

/// 构造改进 KMP next 表，跳过必然重复失败的比较位置。
template<typename Pattern>
dsa::container::Vector<int> buildKmpNextImproved(const Pattern& pattern) {
    const std::size_t length = sequenceSize(pattern);
    if (length == 0)
        return dsa::container::Vector<int>();

    dsa::container::Vector<int> next(length, 0);
    int patternIndex = 0;
    int fallback = -1;
    next[0] = -1;
    while (patternIndex < static_cast<int>(length) - 1) {
        if (fallback >= 0 &&
            !(sequenceAt(pattern, static_cast<std::size_t>(fallback)) ==
              sequenceAt(pattern, static_cast<std::size_t>(patternIndex)))) {
            fallback = next[static_cast<std::size_t>(fallback)];
            continue;
        }

        ++patternIndex;
        ++fallback;
        if (!(sequenceAt(pattern, static_cast<std::size_t>(patternIndex)) ==
              sequenceAt(pattern, static_cast<std::size_t>(fallback)))) {
            next[static_cast<std::size_t>(patternIndex)] = fallback;
        } else {
            next[static_cast<std::size_t>(patternIndex)] =
                next[static_cast<std::size_t>(fallback)];
        }
    }
    return next;
}

/// 使用 KMP 查找 pattern，未找到返回 stringNpos。
template<typename Pattern, typename Text, typename Observer>
std::size_t kmpSearch(
    const Pattern& pattern,
    const Text& text,
    KmpStrategy strategy,
    Observer& observer
) {
    const std::size_t patternSize = sequenceSize(pattern);
    const std::size_t textSize = sequenceSize(text);
    if (patternSize == 0)
        return 0;
    if (patternSize > textSize)
        return stringNpos;

    dsa::container::Vector<int> next = strategy == KmpStrategy::Improved
        ? buildKmpNextImproved(pattern)
        : buildKmpNext(pattern);
    observer.onKmpTable(next.data(), next.size());

    std::ptrdiff_t textIndex = 0;
    std::ptrdiff_t patternIndex = 0;
    while (patternIndex < static_cast<std::ptrdiff_t>(patternSize) &&
           textIndex < static_cast<std::ptrdiff_t>(textSize)) {
        observer.onProgress(
            textIndex - patternIndex,
            patternIndex,
            next.data(),
            next.size()
        );
        observer.onPause();
        if (patternIndex < 0 ||
            sequenceAt(text, static_cast<std::size_t>(textIndex)) ==
                sequenceAt(pattern, static_cast<std::size_t>(patternIndex))) {
            ++textIndex;
            ++patternIndex;
        } else {
            patternIndex = next[static_cast<std::size_t>(patternIndex)];
        }
    }

    return patternIndex == static_cast<std::ptrdiff_t>(patternSize)
        ? static_cast<std::size_t>(textIndex - patternIndex)
        : stringNpos;
}

/// 使用 KMP 查找 pattern，默认不产生教学输出。
template<typename Pattern, typename Text>
std::size_t kmpSearch(
    const Pattern& pattern,
    const Text& text,
    KmpStrategy strategy = KmpStrategy::Basic
) {
    NullSubstringSearchObserver observer;
    return kmpSearch(pattern, text, strategy, observer);
}

/// 构造 Boyer-Moore 坏字符表；当前实现面向单字节字符序列。
template<typename Pattern>
dsa::container::Vector<int> buildBadCharacterTable(const Pattern& pattern) {
    dsa::container::Vector<int> table(256, -1);
    const std::size_t length = sequenceSize(pattern);
    for (std::size_t index = 0; index < length; ++index) {
        table[static_cast<unsigned char>(sequenceAt(pattern, index))] =
            static_cast<int>(index);
    }
    return table;
}

/// 构造 Boyer-Moore suffix size 表。
template<typename Pattern>
dsa::container::Vector<int> buildBoyerMooreSuffixes(const Pattern& pattern) {
    const int length = static_cast<int>(sequenceSize(pattern));
    if (length <= 0)
        return dsa::container::Vector<int>();

    dsa::container::Vector<int> suffixes(static_cast<std::size_t>(length), 0);
    suffixes[static_cast<std::size_t>(length - 1)] = length;
    int low = length - 1;
    int high = length - 1;
    for (int index = length - 2; index >= 0; --index) {
        if (low < index &&
            suffixes[static_cast<std::size_t>(length - high + index - 1)] <
                index - low) {
            suffixes[static_cast<std::size_t>(index)] =
                suffixes[static_cast<std::size_t>(length - high + index - 1)];
        } else {
            high = index;
            low = std::min(low, high);
            while (low >= 0 &&
                   sequenceAt(pattern, static_cast<std::size_t>(low)) ==
                       sequenceAt(
                           pattern,
                           static_cast<std::size_t>(length - high + low - 1)
                       )) {
                --low;
            }
            suffixes[static_cast<std::size_t>(index)] = high - low;
        }
    }
    return suffixes;
}

/// 构造 Boyer-Moore 好后缀位移表。
template<typename Pattern>
dsa::container::Vector<int> buildGoodSuffixTable(const Pattern& pattern) {
    const int length = static_cast<int>(sequenceSize(pattern));
    if (length <= 0)
        return dsa::container::Vector<int>();

    const dsa::container::Vector<int> suffixes = buildBoyerMooreSuffixes(pattern);
    dsa::container::Vector<int> shifts(static_cast<std::size_t>(length), length);
    int fill = 0;
    for (int index = length - 1; index >= 0; --index) {
        if (index + 1 == suffixes[static_cast<std::size_t>(index)]) {
            while (fill < length - index - 1) {
                shifts[static_cast<std::size_t>(fill)] = length - index - 1;
                ++fill;
            }
        }
    }
    for (int index = 0; index < length - 1; ++index) {
        shifts[static_cast<std::size_t>(
            length - suffixes[static_cast<std::size_t>(index)] - 1
        )] = length - index - 1;
    }
    return shifts;
}

/// 使用 Boyer-Moore 查找 pattern，未找到返回 stringNpos。
template<typename Pattern, typename Text, typename Observer>
std::size_t boyerMooreSearch(
    const Pattern& pattern,
    const Text& text,
    BoyerMooreStrategy strategy,
    Observer& observer
) {
    const std::size_t patternSize = sequenceSize(pattern);
    const std::size_t textSize = sequenceSize(text);
    if (patternSize == 0)
        return 0;
    if (patternSize > textSize)
        return stringNpos;

    const dsa::container::Vector<int> badCharacter = buildBadCharacterTable(pattern);
    observer.onBadCharacterTable(badCharacter.data(), badCharacter.size());

    dsa::container::Vector<int> goodSuffix;
    if (strategy == BoyerMooreStrategy::Full) {
        goodSuffix = buildGoodSuffixTable(pattern);
        observer.onGoodSuffixTable(goodSuffix.data(), goodSuffix.size());
    }

    std::size_t alignment = 0;
    while (alignment + patternSize <= textSize) {
        std::ptrdiff_t patternIndex =
            static_cast<std::ptrdiff_t>(patternSize) - 1;
        while (patternIndex >= 0 &&
               sequenceAt(pattern, static_cast<std::size_t>(patternIndex)) ==
                   sequenceAt(
                       text,
                       alignment + static_cast<std::size_t>(patternIndex)
                   )) {
            --patternIndex;
        }

        observer.onProgress(
            static_cast<std::ptrdiff_t>(alignment),
            patternIndex,
            strategy == BoyerMooreStrategy::Full
                ? goodSuffix.data()
                : badCharacter.data(),
            strategy == BoyerMooreStrategy::Full
                ? goodSuffix.size()
                : badCharacter.size()
        );
        observer.onPause();
        if (patternIndex < 0)
            return alignment;

        const unsigned char mismatched = static_cast<unsigned char>(
            sequenceAt(text, alignment + static_cast<std::size_t>(patternIndex))
        );
        const int badShift = static_cast<int>(patternIndex) - badCharacter[mismatched];
        int shift = badShift > 1 ? badShift : 1;
        if (strategy == BoyerMooreStrategy::Full) {
            shift = std::max(
                shift,
                goodSuffix[static_cast<std::size_t>(patternIndex)]
            );
        }
        alignment += static_cast<std::size_t>(shift);
    }
    return stringNpos;
}

/// 使用 Boyer-Moore 查找 pattern，默认不产生教学输出。
template<typename Pattern, typename Text>
std::size_t boyerMooreSearch(
    const Pattern& pattern,
    const Text& text,
    BoyerMooreStrategy strategy = BoyerMooreStrategy::BadCharacter
) {
    NullSubstringSearchObserver observer;
    return boyerMooreSearch(pattern, text, strategy, observer);
}

namespace detail {

/// 将单字节字符转换为稳定的非零滚动哈希符号值。
template<typename Sequence>
std::int64_t rollingHashSymbol(const Sequence& sequence, std::size_t index) {
    return static_cast<std::int64_t>(
        static_cast<unsigned char>(sequenceAt(sequence, index))
    ) + 1;
}

} // namespace detail

/// 使用 Rabin-Karp 查找 pattern，未找到返回 stringNpos。
template<typename Pattern, typename Text, typename Observer>
std::size_t rabinKarpSearch(
    const Pattern& pattern,
    const Text& text,
    Observer& observer
) {
    static const std::int64_t base = 257;
    static const std::int64_t modulus = 1000003;

    const std::size_t patternSize = sequenceSize(pattern);
    const std::size_t textSize = sequenceSize(text);
    if (patternSize == 0)
        return 0;
    if (patternSize > textSize)
        return stringNpos;

    std::int64_t highestPower = 1;
    for (std::size_t index = 1; index < patternSize; ++index)
        highestPower = (highestPower * base) % modulus;

    std::int64_t patternHash = 0;
    std::int64_t textHash = 0;
    for (std::size_t index = 0; index < patternSize; ++index) {
        patternHash = (
            patternHash * base + detail::rollingHashSymbol(pattern, index)
        ) % modulus;
        textHash = (
            textHash * base + detail::rollingHashSymbol(text, index)
        ) % modulus;
    }

    for (std::size_t start = 0; start + patternSize <= textSize; ++start) {
        observer.onRabinKarpProgress(start, patternHash, textHash);
        observer.onPause();
        if (patternHash == textHash) {
            std::size_t index = 0;
            while (index < patternSize &&
                   sequenceAt(pattern, index) == sequenceAt(text, start + index)) {
                ++index;
            }
            if (index == patternSize)
                return start;
        }

        if (start + patternSize == textSize)
            break;
        textHash = (
            textHash - detail::rollingHashSymbol(text, start) * highestPower
        ) % modulus;
        if (textHash < 0)
            textHash += modulus;
        textHash = (
            textHash * base +
            detail::rollingHashSymbol(text, start + patternSize)
        ) % modulus;
    }
    return stringNpos;
}

/// 使用 Rabin-Karp 查找 pattern，默认不产生教学输出。
template<typename Pattern, typename Text>
std::size_t rabinKarpSearch(const Pattern& pattern, const Text& text) {
    NullSubstringSearchObserver observer;
    return rabinKarpSearch(pattern, text, observer);
}

} // namespace algorithm
} // namespace dsa

#endif
