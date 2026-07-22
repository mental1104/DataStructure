#pragma once

#include <cstddef>
#include <vector>

#include "MatchObserver.h"
#include "dsa_string.h"
#include <dsa/algorithm/SubstringSearch.h>

/// 旧教学 API 的 KMP 策略枚举。
enum class KMPStrategy {
    Basic,
    Improved
};

namespace dsa_string_match_detail {

/// 将通用 KMP 的索引事件适配到原 MatchObserver 可视化接口。
class LegacyKmpObserver {
public:
    LegacyKmpObserver(
        const String& pattern,
        const String& text,
        MatchObserver& observer
    ) : pattern_(pattern), text_(text), observer_(observer) {
    }

    void onKmpTable(const int* table, std::size_t length) {
        observer_.onNextTable(pattern_, table, static_cast<int>(length));
    }

    void onProgress(
        std::ptrdiff_t alignment,
        std::ptrdiff_t patternIndex,
        const int* table,
        std::size_t length
    ) {
        observer_.onProgress(
            text_,
            pattern_,
            static_cast<int>(alignment),
            static_cast<int>(patternIndex),
            table,
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

/// 保留旧返回类型，将通用 next 表复制到调用方负责释放的数组。
inline int* buildNext(const String& pattern) {
    const dsa::container::Vector<int> table = dsa::algorithm::buildKmpNext(pattern);
    int* result = new int[table.size()];
    for (std::size_t index = 0; index < table.size(); ++index)
        result[index] = table[index];
    return result;
}

/// 保留旧返回类型，将改进 next 表复制到调用方负责释放的数组。
inline int* buildNextImproved(const String& pattern) {
    const dsa::container::Vector<int> table = dsa::algorithm::buildKmpNextImproved(pattern);
    int* result = new int[table.size()];
    for (std::size_t index = 0; index < table.size(); ++index)
        result[index] = table[index];
    return result;
}

/// 旧 String 专用入口仅作为 facade，核心匹配由泛型算法完成。
inline int matchKMP(
    const String& pattern,
    const String& text,
    KMPStrategy strategy = KMPStrategy::Basic,
    MatchObserver* observer = nullptr
) {
    NoopMatchObserver noop;
    MatchObserver& selected = observer == nullptr ? static_cast<MatchObserver&>(noop)
                                                 : *observer;
    dsa_string_match_detail::LegacyKmpObserver adapter(
        pattern,
        text,
        selected
    );
    const std::size_t result = dsa::algorithm::kmpSearch(
        pattern,
        text,
        strategy == KMPStrategy::Improved
            ? dsa::algorithm::KmpStrategy::Improved
            : dsa::algorithm::KmpStrategy::Basic,
        adapter
    );
    return result == dsa::algorithm::stringNpos
        ? static_cast<int>(text.size())
        : static_cast<int>(result);
}

inline int matchKMPBasic(const String& pattern, const String& text) {
    StdoutMatchObserver observer;
    return matchKMP(pattern, text, KMPStrategy::Basic, &observer);
}

inline int matchKMPImproved(const String& pattern, const String& text) {
    StdoutMatchObserver observer;
    return matchKMP(pattern, text, KMPStrategy::Improved, &observer);
}
