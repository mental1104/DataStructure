#pragma once

#include "MatchObserver.h"
#include "dsa_string.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dsa {
namespace str {
namespace match {

using HashCode = std::int64_t;
constexpr HashCode kKRMod = 1000000007LL;
constexpr HashCode kKRBase = 256LL;

inline HashCode prepare_high_order(std::size_t pattern_size) noexcept {
    HashCode high_order = 1;
    for (std::size_t index = 1; index < pattern_size; ++index) {
        high_order = (high_order * kKRBase) % kKRMod;
    }
    return high_order;
}

inline HashCode byte_value(char character) noexcept {
    return static_cast<unsigned char>(character);
}

inline bool equal_at(std::string_view pattern,
                     std::string_view text,
                     std::size_t offset) noexcept {
    return text.compare(offset, pattern.size(), pattern) == 0;
}

inline std::size_t rabin_karp_search(std::string_view pattern,
                                     std::string_view text,
                                     MatchObserver* observer_ptr = nullptr) {
    if (pattern.empty()) {
        return 0;
    }
    if (text.empty() || pattern.size() > text.size()) {
        return std::string_view::npos;
    }

    NoopMatchObserver noop;
    MatchObserver& observer = observer_ptr == nullptr ? static_cast<MatchObserver&>(noop) : *observer_ptr;
    const HashCode high_order = prepare_high_order(pattern.size());
    HashCode pattern_hash = 0;
    HashCode text_hash = 0;

    for (std::size_t index = 0; index < pattern.size(); ++index) {
        pattern_hash = (pattern_hash * kKRBase + byte_value(pattern[index])) % kKRMod;
        text_hash = (text_hash * kKRBase + byte_value(text[index])) % kKRMod;
    }

    for (std::size_t offset = 0; offset + pattern.size() <= text.size(); ++offset) {
        observer.onKRProgress(text, pattern, offset, pattern_hash, text_hash);
        observer.onPause();
        if (pattern_hash == text_hash && equal_at(pattern, text, offset)) {
            return offset;
        }

        if (offset + pattern.size() == text.size()) {
            break;
        }

        text_hash = (text_hash - byte_value(text[offset]) * high_order) % kKRMod;
        if (text_hash < 0) {
            text_hash += kKRMod;
        }
        text_hash = (text_hash * kKRBase + byte_value(text[offset + pattern.size()])) % kKRMod;
    }

    return std::string_view::npos;
}

template <typename Pattern, typename Text>
std::size_t rabin_karp_search(const Pattern& pattern,
                              const Text& text,
                              MatchObserver* observer = nullptr) {
    return rabin_karp_search(as_string_view(pattern), as_string_view(text), observer);
}

}  // namespace match
}  // namespace str
}  // namespace dsa

using HashCode = dsa::str::match::HashCode;
constexpr HashCode kKRMod = dsa::str::match::kKRMod;
constexpr HashCode kKRBase = dsa::str::match::kKRBase;

inline HashCode prepareDm(size_type pattern_size) {
    return dsa::str::match::prepare_high_order(static_cast<std::size_t>(pattern_size));
}

template <typename Pattern, typename Text>
inline int matchKR(const Pattern& pattern,
                   const Text& text,
                   MatchObserver* observer = nullptr) {
    const std::size_t position = dsa::str::match::rabin_karp_search(pattern, text, observer);
    return position == std::string_view::npos ? -1 : static_cast<int>(position);
}
