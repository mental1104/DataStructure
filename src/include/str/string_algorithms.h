#pragma once

#include "dsa_string.h"

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace dsa {
namespace str {

template <typename Sequence>
String substr(const Sequence& input, size_type pos, size_type count) {
    const std::string_view view = as_string_view(input);
    const std::size_t start = static_cast<std::size_t>(pos);
    if (start >= view.size()) {
        return String{};
    }

    const std::size_t requested = count == String::npos
        ? std::string_view::npos
        : static_cast<std::size_t>(count);
    return String(view.substr(start, requested));
}

template <typename Sequence>
String sub_str(const Sequence& input, size_type pos, size_type count) {
    return substr(input, pos, count);
}

template <typename Sequence>
String prefix(const Sequence& input, size_type count) {
    return substr(input, 0, count);
}

template <typename Sequence>
String suffix(const Sequence& input, size_type count) {
    const std::string_view view = as_string_view(input);
    const std::size_t requested = static_cast<std::size_t>(count);
    const std::size_t start = requested >= view.size() ? 0 : view.size() - requested;
    return String(view.substr(start));
}

template <typename Left, typename Right>
String concat(const Left& left, const Right& right) {
    const std::string_view lhs = as_string_view(left);
    const std::string_view rhs = as_string_view(right);

    std::string result;
    result.reserve(lhs.size() + rhs.size());
    if (!lhs.empty()) {
        result.append(lhs.data(), lhs.size());
    }
    if (!rhs.empty()) {
        result.append(rhs.data(), rhs.size());
    }
    return String(std::move(result));
}

template <typename Sequence, typename Prefix>
bool starts_with(const Sequence& input, const Prefix& expected_prefix) noexcept {
    const std::string_view view = as_string_view(input);
    const std::string_view expected = as_string_view(expected_prefix);
    return view.size() >= expected.size() && view.compare(0, expected.size(), expected) == 0;
}

template <typename Sequence, typename Suffix>
bool ends_with(const Sequence& input, const Suffix& expected_suffix) noexcept {
    const std::string_view view = as_string_view(input);
    const std::string_view expected = as_string_view(expected_suffix);
    return view.size() >= expected.size() &&
           view.compare(view.size() - expected.size(), expected.size(), expected) == 0;
}

}  // namespace str
}  // namespace dsa
