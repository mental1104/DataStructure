#pragma once

#include "MatchObserver.h"
#include "dsa_string.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

namespace dsa {
namespace str {
namespace match {

enum class BMStrategy { BadCharacter, Full };

inline std::array<int, 256> build_bad_character(std::string_view pattern,
                                                 MatchObserver* observer_ptr = nullptr) {
    std::array<int, 256> table{};
    table.fill(-1);
    for (std::size_t index = 0; index < pattern.size(); ++index) {
        table[static_cast<unsigned char>(pattern[index])] = static_cast<int>(index);
    }
    if (observer_ptr != nullptr) {
        observer_ptr->onBCTable(table.data(), static_cast<int>(table.size()));
    }
    return table;
}

template <typename Pattern>
std::array<int, 256> build_bad_character(const Pattern& pattern,
                                          MatchObserver* observer = nullptr) {
    return build_bad_character(as_string_view(pattern), observer);
}

inline std::vector<int> build_suffix_sizes(std::string_view pattern) {
    const int length = static_cast<int>(pattern.size());
    std::vector<int> suffix(static_cast<std::size_t>(length), 0);
    if (length == 0) {
        return suffix;
    }

    suffix[static_cast<std::size_t>(length - 1)] = length;
    int lower = length - 1;
    int upper = length - 1;
    for (int index = length - 2; index >= 0; --index) {
        if (lower < index && suffix[static_cast<std::size_t>(length - upper + index - 1)] < index - lower) {
            suffix[static_cast<std::size_t>(index)] =
                suffix[static_cast<std::size_t>(length - upper + index - 1)];
        } else {
            upper = index;
            lower = std::min(lower, upper);
            while (lower >= 0 &&
                   pattern[static_cast<std::size_t>(lower)] ==
                       pattern[static_cast<std::size_t>(length - upper + lower - 1)]) {
                --lower;
            }
            suffix[static_cast<std::size_t>(index)] = upper - lower;
        }
    }
    return suffix;
}

template <typename Pattern>
std::vector<int> build_suffix_sizes(const Pattern& pattern) {
    return build_suffix_sizes(as_string_view(pattern));
}

inline std::vector<int> build_good_suffix(std::string_view pattern,
                                           MatchObserver* observer_ptr = nullptr) {
    const int length = static_cast<int>(pattern.size());
    std::vector<int> good_suffix(static_cast<std::size_t>(length), length);
    if (length == 0) {
        return good_suffix;
    }

    const std::vector<int> suffix = build_suffix_sizes(pattern);
    int fill_index = 0;
    for (int index = length - 1; index >= 0; --index) {
        if (index + 1 == suffix[static_cast<std::size_t>(index)]) {
            while (fill_index < length - index - 1) {
                good_suffix[static_cast<std::size_t>(fill_index++)] = length - index - 1;
            }
        }
    }
    for (int index = 0; index < length - 1; ++index) {
        good_suffix[static_cast<std::size_t>(length - suffix[static_cast<std::size_t>(index)] - 1)] =
            length - index - 1;
    }

    if (observer_ptr != nullptr) {
        observer_ptr->onGSTable(good_suffix.data(), length, pattern);
    }
    return good_suffix;
}

template <typename Pattern>
std::vector<int> build_good_suffix(const Pattern& pattern,
                                    MatchObserver* observer = nullptr) {
    return build_good_suffix(as_string_view(pattern), observer);
}

inline std::size_t boyer_moore_search(std::string_view pattern,
                                      std::string_view text,
                                      BMStrategy strategy = BMStrategy::BadCharacter,
                                      MatchObserver* observer_ptr = nullptr) {
    if (pattern.empty()) {
        return 0;
    }
    if (text.empty() || pattern.size() > text.size()) {
        return std::string_view::npos;
    }

    NoopMatchObserver noop;
    MatchObserver& observer = observer_ptr == nullptr ? static_cast<MatchObserver&>(noop) : *observer_ptr;
    const std::array<int, 256> bad_character = build_bad_character(pattern, &observer);
    const std::vector<int> good_suffix = strategy == BMStrategy::Full
        ? build_good_suffix(pattern, &observer)
        : std::vector<int>{};

    std::size_t alignment = 0;
    while (alignment + pattern.size() <= text.size()) {
        int pattern_index = static_cast<int>(pattern.size()) - 1;
        while (pattern_index >= 0 &&
               pattern[static_cast<std::size_t>(pattern_index)] ==
                   text[alignment + static_cast<std::size_t>(pattern_index)]) {
            --pattern_index;
        }

        observer.onProgress(text,
                            pattern,
                            static_cast<int>(alignment),
                            pattern_index,
                            strategy == BMStrategy::Full ? good_suffix.data() : bad_character.data(),
                            strategy == BMStrategy::Full
                                ? static_cast<int>(good_suffix.size())
                                : static_cast<int>(bad_character.size()));
        observer.onPause();

        if (pattern_index < 0) {
            return alignment;
        }

        const int bad_character_shift = pattern_index -
            bad_character[static_cast<unsigned char>(text[alignment + static_cast<std::size_t>(pattern_index)])];
        int shift = std::max(1, bad_character_shift);
        if (strategy == BMStrategy::Full) {
            shift = std::max(shift, good_suffix[static_cast<std::size_t>(pattern_index)]);
        }
        alignment += static_cast<std::size_t>(shift);
    }
    return std::string_view::npos;
}

template <typename Pattern, typename Text>
std::size_t boyer_moore_search(const Pattern& pattern,
                               const Text& text,
                               BMStrategy strategy = BMStrategy::BadCharacter,
                               MatchObserver* observer = nullptr) {
    return boyer_moore_search(as_string_view(pattern), as_string_view(text), strategy, observer);
}

}  // namespace match
}  // namespace str
}  // namespace dsa

using BMStrategy = dsa::str::match::BMStrategy;

template <typename Pattern>
inline int* buildBC(const Pattern& pattern, MatchObserver* observer = nullptr) {
    const std::array<int, 256> table = dsa::str::match::build_bad_character(pattern, observer);
    int* result = new int[table.size()];
    std::copy(table.begin(), table.end(), result);
    return result;
}

template <typename Pattern>
inline int* buildSS(const Pattern& pattern, int) {
    const std::vector<int> suffix = dsa::str::match::build_suffix_sizes(pattern);
    int* result = new int[suffix.size()];
    std::copy(suffix.begin(), suffix.end(), result);
    return result;
}

template <typename Pattern>
inline int* buildGS(const Pattern& pattern, int, MatchObserver* observer = nullptr) {
    const std::vector<int> good_suffix = dsa::str::match::build_good_suffix(pattern, observer);
    int* result = new int[good_suffix.size()];
    std::copy(good_suffix.begin(), good_suffix.end(), result);
    return result;
}

template <typename Pattern, typename Text>
inline int matchBM(const Pattern& pattern,
                   const Text& text,
                   BMStrategy strategy = BMStrategy::BadCharacter,
                   MatchObserver* observer = nullptr) {
    const std::size_t position = dsa::str::match::boyer_moore_search(pattern, text, strategy, observer);
    return position == std::string_view::npos ? -1 : static_cast<int>(position);
}

template <typename Pattern, typename Text>
inline int matchBMBadCharacter(const Pattern& pattern, const Text& text) {
    return matchBM(pattern, text, BMStrategy::BadCharacter, nullptr);
}

template <typename Pattern, typename Text>
inline int matchBMFull(const Pattern& pattern, const Text& text) {
    return matchBM(pattern, text, BMStrategy::Full, nullptr);
}

template <typename Pattern, typename Text>
inline int matchBMVerbose(const Pattern& pattern,
                          const Text& text,
                          BMStrategy strategy = BMStrategy::BadCharacter) {
    StdoutMatchObserver observer;
    return matchBM(pattern, text, strategy, &observer);
}
