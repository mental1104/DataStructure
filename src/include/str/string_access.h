#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

class String;

namespace dsa {
namespace str {

inline std::string_view as_string_view(std::string_view value) noexcept {
    return value;
}

inline std::string_view as_string_view(const std::string& value) noexcept {
    return std::string_view(value.data(), value.size());
}

inline std::string_view as_string_view(const char* value) noexcept {
    return value == nullptr ? std::string_view{} : std::string_view(value);
}

inline std::string_view as_string_view(char* value) noexcept {
    return as_string_view(static_cast<const char*>(value));
}

template <std::size_t N>
inline std::string_view as_string_view(const char (&value)[N]) noexcept {
    const std::size_t size = N > 0 && value[N - 1] == '\0' ? N - 1 : N;
    return std::string_view(value, size);
}

template <typename Sequence>
inline auto as_string_view(const Sequence& value) noexcept
    -> decltype(std::string_view(value.data(), static_cast<std::size_t>(value.size()))) {
    using pointer_type = decltype(value.data());
    static_assert(std::is_convertible<pointer_type, const char*>::value,
                  "string-like sequence data() must be convertible to const char*");
    return std::string_view(value.data(), static_cast<std::size_t>(value.size()));
}

std::string_view as_string_view(const String& value) noexcept;

}  // namespace str
}  // namespace dsa
