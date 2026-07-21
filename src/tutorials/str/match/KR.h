#pragma once

#include <cstddef>
#include <cstdint>

#include "MatchObserver.h"
#include "dsa_string.h"
#include <dsa/algorithm/SubstringSearch.h>

constexpr int kKRMod = 97;
constexpr int kKRBase = 10;
using HashCode = std::int64_t;

/// 保留旧教学辅助函数：将十进制字符转换为数值。
inline int krDigit(const String& sequence, size_type index) {
    return sequence[index] - '0';
}

/// 保留旧教学辅助函数：计算 R^(m - 1) mod M。
inline HashCode prepareDm(size_type length) {
    HashCode highestPower = 1;
    for (size_type index = 1; index < length; ++index)
        highestPower = (kKRBase * highestPower) % kKRMod;
    return highestPower;
}

/// 保留旧教学辅助函数：逐字符确认哈希命中的候选位置。
inline bool check1by1(
    const String& pattern,
    const String& text,
    size_type position
) {
    for (size_type index = 0; index < pattern.size(); ++index) {
        if (pattern[index] != text[position + index])
            return false;
    }
    return true;
}

/// 保留旧教学辅助函数：更新十进制字符串的滚动哈希。
inline void updateHash(
    HashCode& textHash,
    const String& text,
    size_type patternLength,
    size_type nextStart,
    HashCode highestPower
) {
    textHash = (
        textHash - krDigit(text, nextStart - 1) * highestPower
    ) % kKRMod;
    textHash = (
        textHash * kKRBase + krDigit(text, nextStart + patternLength - 1)
    ) % kKRMod;
    if (textHash < 0)
        textHash += kKRMod;
}

namespace dsa_string_match_detail {

/// 将通用 Rabin-Karp 进度事件适配到原 MatchObserver。
class LegacyKrObserver {
public:
    LegacyKrObserver(
        const String& pattern,
        const String& text,
        MatchObserver& observer
    ) : pattern_(pattern), text_(text), observer_(observer) {
    }

    void onRabinKarpProgress(
        std::size_t alignment,
        std::int64_t patternHash,
        std::int64_t textHash
    ) {
        observer_.onKRProgress(
            text_,
            pattern_,
            alignment,
            patternHash,
            textHash
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

/// 旧 String 专用入口仅作为 facade，核心匹配由泛型 Rabin-Karp 完成。
inline int matchKR(
    const String& pattern,
    const String& text,
    MatchObserver* observer
) {
    NoopMatchObserver noop;
    MatchObserver& selected = observer == nullptr
        ? static_cast<MatchObserver&>(noop)
        : *observer;
    dsa_string_match_detail::LegacyKrObserver adapter(
        pattern,
        text,
        selected
    );
    const std::size_t result = dsa::algorithm::rabinKarpSearch(
        pattern,
        text,
        adapter
    );
    return result == dsa::algorithm::stringNpos
        ? static_cast<int>(text.size())
        : static_cast<int>(result);
}

/// 保留两参数教学入口，并继续输出逐步哈希过程。
inline int matchKR(const String& pattern, const String& text) {
    StdoutMatchObserver observer;
    return matchKR(pattern, text, &observer);
}
