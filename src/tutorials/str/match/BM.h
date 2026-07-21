#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include "MatchObserver.h"
#include "dsa_string.h"
#include <dsa/algorithm/SubstringSearch.h>

/// 旧教学 API 的 Boyer-Moore 策略枚举。
enum class BMStrategy {
    BadCharacter,
    Full
};

namespace dsa_string_match_detail {

/// 将通用 Boyer-Moore 事件适配到原 MatchObserver。
class LegacyBmObserver {
public:
    LegacyBmObserver(
        const String& pattern,
        const String& text,
        MatchObserver& observer
    ) : pattern_(pattern), text_(text), observer_(observer) {
    }

    void onBadCharacterTable(const int* table, std::size_t length) {
        observer_.onBCTable(table, static_cast<int>(length));
    }

    void onGoodSuffixTable(const int* table, std::size_t length) {
        observer_.onGSTable(
            table,
            static_cast<int>(length),
            pattern_
        );
    }

    void onProgress(
        std::ptrdiff_t alignment,
        std::ptrdiff_t patternIndex,
        const int* auxiliary,
        std::size_t length
    ) {
        observer_.onProgress(
            text_,
            pattern_,
            static_cast<int>(alignment),
            static_cast<int>(patternIndex),
            auxiliary,
            static_cast<int>(length)
        );
    }

    void onPause() {
        observer_.onPause();
    }

private:
    const String& pattern_;
    const String& text_;
    MatchObserver& observer_;
};

} // namespace dsa_string_match_detail

/// 构造旧 API 所需的 256 项坏字符数组，由调用方 delete[]。
inline int* buildBC(const String& pattern, MatchObserver* observer = nullptr) {
    const dsa::container::Vector<int> table =
        dsa::algorithm::buildBadCharacterTable(pattern);
    int* result = new int[table.size()];
    std::copy(table.begin(), table.end(), result);
    if (observer != nullptr)
        observer->onBCTable(result, static_cast<int>(table.size()));
    return result;
}

/// 构造旧 API 所需的 suffix size 数组，由调用方 delete[]。
inline int* buildSS(const String& pattern, int length) {
    const dsa::container::Vector<int> table =
        dsa::algorithm::buildBoyerMooreSuffixes(pattern);
    const int selected = std::min(length, static_cast<int>(table.size()));
    int* result = new int[length > 0 ? static_cast<std::size_t>(length) : 0];
    for (int index = 0; index < selected; ++index)
        result[index] = table[static_cast<std::size_t>(index)];
    for (int index = selected; index < length; ++index)
        result[index] = 0;
    return result;
}

/// 构造旧 API 所需的好后缀数组，由调用方 delete[]。
inline int* buildGS(
    const String& pattern,
    int length,
    MatchObserver* observer = nullptr
) {
    const dsa::container::Vector<int> table = dsa::algorithm::buildGoodSuffixTable(pattern);
    const int selected = std::min(length, static_cast<int>(table.size()));
    int* result = new int[length > 0 ? static_cast<std::size_t>(length) : 0];
    for (int index = 0; index < selected; ++index)
        result[index] = table[static_cast<std::size_t>(index)];
    for (int index = selected; index < length; ++index)
        result[index] = length;
    if (observer != nullptr)
        observer->onGSTable(result, length, pattern);
    return result;
}

/// 旧 String 专用入口仅作为 facade，核心匹配由泛型算法完成。
inline int matchBM(
    const String& pattern,
    const String& text,
    BMStrategy strategy = BMStrategy::BadCharacter,
    MatchObserver* observer = nullptr
) {
    NoopMatchObserver noop;
    MatchObserver& selected = observer == nullptr ? static_cast<MatchObserver&>(noop)
                                                 : *observer;
    dsa_string_match_detail::LegacyBmObserver adapter(
        pattern,
        text,
        selected
    );
    const std::size_t result = dsa::algorithm::boyerMooreSearch(
        pattern,
        text,
        strategy == BMStrategy::Full
            ? dsa::algorithm::BoyerMooreStrategy::Full
            : dsa::algorithm::BoyerMooreStrategy::BadCharacter,
        adapter
    );
    return result == dsa::algorithm::stringNpos
        ? static_cast<int>(text.size())
        : static_cast<int>(result);
}

inline int matchBMBadCharacter(const String& pattern, const String& text) {
    return matchBM(pattern, text, BMStrategy::BadCharacter, nullptr);
}

inline int matchBMFull(const String& pattern, const String& text) {
    return matchBM(pattern, text, BMStrategy::Full, nullptr);
}

inline int matchBMVerbose(
    const String& pattern,
    const String& text,
    BMStrategy strategy = BMStrategy::BadCharacter
) {
    StdoutMatchObserver observer;
    return matchBM(pattern, text, strategy, &observer);
}
