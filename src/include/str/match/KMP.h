#pragma once

#include "MatchObserver.h"
#include "dsa_string.h"

#include <cstddef>
#include <string_view>
#include <vector>

namespace dsa {
namespace str {
namespace match {

enum class KMPStrategy { Basic, Improved };

inline std::vector<int> build_next(std::string_view pattern,
                                   KMPStrategy strategy = KMPStrategy::Basic) {
    std::vector<int> next(pattern.size(), -1);
    if (pattern.empty()) {
        return next;
    }

    int pattern_index = 0;
    int candidate = -1;
    while (pattern_index < static_cast<int>(pattern.size()) - 1) {
        if (candidate < 0 || pattern[static_cast<std::size_t>(pattern_index)] ==
                                 pattern[static_cast<std::size_t>(candidate)]) {
            ++pattern_index;
            ++candidate;
            if (strategy == KMPStrategy::Improved &&
                pattern[static_cast<std::size_t>(pattern_index)] ==
                    pattern[static_cast<std::size_t>(candidate)]) {
                next[static_cast<std::size_t>(pattern_index)] =
                    next[static_cast<std::size_t>(candidate)];
            } else {
                next[static_cast<std::size_t>(pattern_index)] = candidate;
            }
        } else {
            candidate = next[static_cast<std::size_t>(candidate)];
        }
    }
    return next;
}

template <typename Pattern>
std::vector<int> build_next(const Pattern& pattern,
                            KMPStrategy strategy = KMPStrategy::Basic) {
    return build_next(as_string_view(pattern), strategy);
}

inline std::size_t kmp_search(std::string_view pattern,
                              std::string_view text,
                              KMPStrategy strategy = KMPStrategy::Basic,
                              MatchObserver* observer_ptr = nullptr) {
    if (pattern.empty()) {
        return 0;
    }
    if (text.empty() || pattern.size() > text.size()) {
        return std::string_view::npos;
    }

    NoopMatchObserver noop;
    MatchObserver& observer = observer_ptr == nullptr ? static_cast<MatchObserver&>(noop) : *observer_ptr;
    const std::vector<int> next = build_next(pattern, strategy);
    observer.onNextTable(pattern, next.data(), static_cast<int>(next.size()));

    std::size_t text_index = 0;
    int pattern_index = 0;
    while (text_index < text.size() && pattern_index < static_cast<int>(pattern.size())) {
        observer.onProgress(text,
                            pattern,
                            static_cast<int>(text_index) - pattern_index,
                            pattern_index,
                            next.data(),
                            static_cast<int>(next.size()));
        observer.onPause();

        if (pattern_index < 0 ||
            text[text_index] == pattern[static_cast<std::size_t>(pattern_index)]) {
            ++text_index;
            ++pattern_index;
        } else {
            pattern_index = next[static_cast<std::size_t>(pattern_index)];
        }
    }

    return pattern_index == static_cast<int>(pattern.size())
        ? text_index - pattern.size()
        : std::string_view::npos;
}

template <typename Pattern, typename Text>
std::size_t kmp_search(const Pattern& pattern,
                       const Text& text,
                       KMPStrategy strategy = KMPStrategy::Basic,
                       MatchObserver* observer = nullptr) {
    return kmp_search(as_string_view(pattern), as_string_view(text), strategy, observer);
}

}  // namespace match
}  // namespace str
}  // namespace dsa

using KMPStrategy = dsa::str::match::KMPStrategy;

template <typename Pattern>
inline int* buildNext(const Pattern& pattern) {
    const std::vector<int> next = dsa::str::match::build_next(pattern, KMPStrategy::Basic);
    int* result = new int[next.size()];
    for (std::size_t index = 0; index < next.size(); ++index) {
        result[index] = next[index];
    }
    return result;
}

template <typename Pattern>
inline int* buildNextImproved(const Pattern& pattern) {
    const std::vector<int> next = dsa::str::match::build_next(pattern, KMPStrategy::Improved);
    int* result = new int[next.size()];
    for (std::size_t index = 0; index < next.size(); ++index) {
        result[index] = next[index];
    }
    return result;
}

template <typename Pattern, typename Text>
inline int matchKMP(const Pattern& pattern,
                    const Text& text,
                    KMPStrategy strategy = KMPStrategy::Basic,
                    MatchObserver* observer = nullptr) {
    const std::size_t position = dsa::str::match::kmp_search(pattern, text, strategy, observer);
    return position == std::string_view::npos ? -1 : static_cast<int>(position);
}

template <typename Pattern, typename Text>
inline int matchKMPBasic(const Pattern& pattern, const Text& text) {
    return matchKMP(pattern, text, KMPStrategy::Basic, nullptr);
}

template <typename Pattern, typename Text>
inline int matchKMPImproved(const Pattern& pattern, const Text& text) {
    return matchKMP(pattern, text, KMPStrategy::Improved, nullptr);
}
