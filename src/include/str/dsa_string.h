#pragma once

#include "utils.h"
#include "string_access.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

class String;

namespace dsa {
namespace str {

template <typename Sequence>
String substr(const Sequence& input, size_type pos, size_type count = static_cast<size_type>(-1));

template <typename Sequence>
String sub_str(const Sequence& input, size_type pos, size_type count = static_cast<size_type>(-1));

template <typename Sequence>
String prefix(const Sequence& input, size_type count);

template <typename Sequence>
String suffix(const Sequence& input, size_type count);

template <typename Left, typename Right>
String concat(const Left& left, const Right& right);

}  // namespace str
}  // namespace dsa

class String {
public:
    using value_type = char;
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;
    static constexpr size_type npos = static_cast<size_type>(-1);

    String() = default;
    String(const char* value) : data_(value == nullptr ? "" : value) {}
    String(char value) : data_(1, value) {}
    String(const char* value, size_type count)
        : data_(value == nullptr ? std::string{} : std::string(value, static_cast<std::size_t>(count))) {}
    String(std::string_view value)
        : data_(value.empty() ? std::string{} : std::string(value.data(), value.size())) {}
    String(const std::string& value) : data_(value) {}
    String(std::string&& value) noexcept : data_(std::move(value)) {}

    String(const String&) = default;
    String(String&&) noexcept = default;
    String& operator=(const String&) = default;
    String& operator=(String&&) noexcept = default;
    ~String() = default;

    char& front() { return data_.front(); }
    const char& front() const { return data_.front(); }
    char& back() { return data_.back(); }
    const char& back() const { return data_.back(); }

    bool check(size_type index) const noexcept {
        return static_cast<std::size_t>(index) < data_.size();
    }

    char charAt(size_type index) const noexcept {
        return check(index) ? data_[static_cast<std::size_t>(index)] : '\0';
    }

    const char* c_str() const noexcept { return data_.c_str(); }
    char* data() noexcept { return data_.data(); }
    const char* data() const noexcept { return data_.data(); }

    size_type size() const noexcept { return static_cast<size_type>(data_.size()); }
    bool empty() const noexcept { return data_.empty(); }
    size_type capacity() const noexcept { return static_cast<size_type>(data_.capacity()); }

    void clear() noexcept { data_.clear(); }
    void reserve(size_type capacity) { data_.reserve(static_cast<std::size_t>(capacity)); }
    void push_back(char value) { data_.push_back(value); }
    void pop_back() { data_.pop_back(); }

    String substr(size_type pos, size_type count = npos) const;
    String prefix(size_type count) const;
    String suffix(size_type count) const;

    bool equal(const String& rhs) const noexcept { return *this == rhs; }
    String& concat(const String& rhs);
    String& append(std::string_view rhs) {
        if (!rhs.empty()) {
            data_.append(rhs.data(), rhs.size());
        }
        return *this;
    }

    char& operator[](size_type index) noexcept { return data_[static_cast<std::size_t>(index)]; }
    const char& operator[](size_type index) const noexcept { return data_[static_cast<std::size_t>(index)]; }

    bool operator==(const String& rhs) const noexcept { return data_ == rhs.data_; }
    bool operator!=(const String& rhs) const noexcept { return !(*this == rhs); }
    bool operator<(const String& rhs) const noexcept { return data_ < rhs.data_; }

    String& operator+=(const String& rhs) { return concat(rhs); }
    String& operator+=(char rhs) {
        push_back(rhs);
        return *this;
    }

    String operator+(const String& rhs) const;
    String operator+(char rhs) const;

    template <typename Visitor>
    void traverse(Visitor&& visit) {
        for (char& value : data_) {
            visit(value);
        }
    }

    void traverse(void (*visit)(char&)) {
        for (char& value : data_) {
            visit(value);
        }
    }

    iterator begin() noexcept { return data_.begin(); }
    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator end() const noexcept { return data_.end(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    std::string_view view() const noexcept { return std::string_view(data_.data(), data_.size()); }
    operator std::string_view() const noexcept { return view(); }

private:
    std::string data_;
};

inline String operator+(char lhs, const String& rhs) {
    String result(lhs);
    result += rhs;
    return result;
}

namespace dsa {
namespace str {

inline std::string_view as_string_view(const String& value) noexcept {
    return value.view();
}

}  // namespace str
}  // namespace dsa

#include "string_algorithms.h"

inline String String::substr(size_type pos, size_type count) const {
    return dsa::str::substr(*this, pos, count);
}

inline String String::prefix(size_type count) const {
    return dsa::str::prefix(*this, count);
}

inline String String::suffix(size_type count) const {
    return dsa::str::suffix(*this, count);
}

inline String& String::concat(const String& rhs) {
    data_.append(rhs.data_.data(), rhs.data_.size());
    return *this;
}

inline String String::operator+(const String& rhs) const {
    return dsa::str::concat(*this, rhs);
}

inline String String::operator+(char rhs) const {
    String result(*this);
    result.push_back(rhs);
    return result;
}
