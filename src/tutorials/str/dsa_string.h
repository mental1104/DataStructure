#ifndef __DSA_STRING
#define __DSA_STRING

#include <cstring>
#include <initializer_list>
#include <utility>

#include "utils.h"
#include "../dsa/algorithm/String.h"

/// 教学字符串容器：保留原有 API，并将可复用序列算法转发到 dsa::algorithm。
class String {
public:
    using value_type = char;
    static constexpr size_type npos = static_cast<size_type>(-1);

    /// 构造空字符串。
    String();

    /// 从零结尾 C 字符串复制构造；空指针按空字符串处理。
    String(const char* text);

    /// 构造只包含一个字符的字符串。
    String(char value);

    /// 深拷贝另一个 String。
    String(const String& other);

    /// 移动接管另一个 String 的缓冲区，并将源对象恢复为空字符串。
    String(String&& other);

    /// 从字符指针复制 count 个字符，允许内容包含零字符。
    String(const char* text, size_type count);

    /// 从 initializer_list 复制构造字符序列。
    String(std::initializer_list<char> values);

    /// 释放独占字符缓冲区。
    ~String();

    /// 使用 copy-and-swap 提供强异常保证。
    String& operator=(const String& other);

    /// 移动赋值后源对象保持合法空状态。
    String& operator=(String&& other);

    /// 从零结尾 C 字符串替换当前内容。
    String& operator=(const char* text);

    /// 返回首字符引用；空字符串调用行为未定义。
    char& front();
    const char& front() const;

    /// 返回尾字符引用；空字符串调用行为未定义。
    char& back();
    const char& back() const;

    /// 检查 index 是否落在有效字符范围内。
    bool check(size_type index) const;

    /// 返回以零结尾的连续字符缓冲区。
    const char* c_str() const;
    char* data();
    const char* data() const;

    /// 返回有效字符数量，不包含末尾零字符。
    size_type size() const;
    bool empty() const;

    /// 返回指定字符；越界时保留旧 API 行为并返回零字符。
    char charAt(size_type index);
    char charAt(size_type index) const;

    /// 通过通用序列算法复制子串。
    String substr(size_type position, size_type count = npos);
    String substr(size_type position, size_type count = npos) const;

    /// 通过通用序列算法复制前缀。
    String prefix(size_type count);
    String prefix(size_type count) const;

    /// 通过通用序列算法复制后缀。
    String suffix(size_type count);
    String suffix(size_type count) const;

    /// 通过通用序列算法判等。
    bool equal(const String& other);
    bool equal(const String& other) const;

    /// 原地拼接另一个 String；内部先构造临时结果再提交。
    String& concat(const String& other);

    /// 在尾部追加单个字符。
    void push_back(char value);

    /// 清空内容但保留已申请容量。
    void clear() noexcept;

    /// 至少预留 requested 个有效字符的容量。
    void reserve(size_type requested);

    /// 返回当前无需重新分配可容纳的有效字符数量。
    size_type capacity() const noexcept;

    char& operator[](size_type index);
    char operator[](size_type index) const;

    bool operator==(const String& other);
    bool operator==(const String& other) const;
    bool operator!=(const String& other) const;
    bool operator<(const String& other) const;

    String operator+(const String& other);
    String operator+(const String& other) const;
    String operator+(char value);
    String operator+(char value) const;

    /// 依次访问每个可写字符，不包含末尾零字符。
    template<typename Visitor>
    void traverse(Visitor&& visit);

    /// 保留函数指针版本的教学 API。
    void traverse(void (*visit)(char&));

    char* begin();
    const char* begin() const;
    char* end();
    const char* end() const;

    /// 交换缓冲区所有权，复杂度 O(1)。
    void swap(String& other) noexcept;

private:
    char* data_;
    size_type size_;
    size_type capacity_;

    /// 复制 count 个字符并一次性提交新缓冲区。
    void assign(const char* text, size_type count);
};

constexpr size_type String::npos;

inline String::String()
    : data_(new char[1]), size_(0), capacity_(0) {
    data_[0] = '\0';
}

inline String::String(const char* text)
    : data_(new char[1]), size_(0), capacity_(0) {
    data_[0] = '\0';
    if (text != nullptr)
        assign(text, static_cast<size_type>(std::strlen(text)));
}

inline String::String(char value)
    : data_(new char[2]), size_(1), capacity_(1) {
    data_[0] = value;
    data_[1] = '\0';
}

inline String::String(const String& other)
    : data_(new char[other.size_ + 1]),
      size_(other.size_),
      capacity_(other.size_) {
    std::memcpy(data_, other.data_, static_cast<std::size_t>(size_) + 1);
}

inline String::String(String&& other)
    : data_(nullptr), size_(0), capacity_(0) {
    char* empty = new char[1];
    empty[0] = '\0';
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = empty;
    other.size_ = 0;
    other.capacity_ = 0;
}

inline String::String(const char* text, size_type count)
    : data_(new char[count + 1]), size_(count), capacity_(count) {
    if (text != nullptr && count > 0)
        std::memcpy(data_, text, count);
    else
        size_ = 0;
    data_[size_] = '\0';
}

inline String::String(std::initializer_list<char> values)
    : data_(new char[values.size() + 1]),
      size_(static_cast<size_type>(values.size())),
      capacity_(static_cast<size_type>(values.size())) {
    size_type index = 0;
    for (char value : values)
        data_[index++] = value;
    data_[size_] = '\0';
}

inline String::~String() {
    delete [] data_;
}

inline String& String::operator=(const String& other) {
    if (this != &other) {
        String copy(other);
        swap(copy);
    }
    return *this;
}

inline String& String::operator=(String&& other) {
    if (this != &other) {
        String moved(std::move(other));
        swap(moved);
    }
    return *this;
}

inline String& String::operator=(const char* text) {
    String replacement(text);
    swap(replacement);
    return *this;
}

inline char& String::front() {
    return data_[0];
}

inline const char& String::front() const {
    return data_[0];
}

inline char& String::back() {
    return data_[size_ - 1];
}

inline const char& String::back() const {
    return data_[size_ - 1];
}

inline bool String::check(size_type index) const {
    return index < size_;
}

inline const char* String::c_str() const {
    return data_;
}

inline char* String::data() {
    return data_;
}

inline const char* String::data() const {
    return data_;
}

inline size_type String::size() const {
    return size_;
}

inline bool String::empty() const {
    return size_ == 0;
}

inline char String::charAt(size_type index) {
    return static_cast<const String&>(*this).charAt(index);
}

inline char String::charAt(size_type index) const {
    return check(index) ? data_[index] : '\0';
}

inline String String::substr(size_type position, size_type count) {
    return static_cast<const String&>(*this).substr(position, count);
}

inline String String::substr(size_type position, size_type count) const {
    return dsa::algorithm::substring<String>(*this, position, count);
}

inline String String::prefix(size_type count) {
    return static_cast<const String&>(*this).prefix(count);
}

inline String String::prefix(size_type count) const {
    return dsa::algorithm::prefix<String>(*this, count);
}

inline String String::suffix(size_type count) {
    return static_cast<const String&>(*this).suffix(count);
}

inline String String::suffix(size_type count) const {
    return dsa::algorithm::suffix<String>(*this, count);
}

inline bool String::equal(const String& other) {
    return static_cast<const String&>(*this).equal(other);
}

inline bool String::equal(const String& other) const {
    return dsa::algorithm::sequenceEqual(*this, other);
}

inline String& String::concat(const String& other) {
    String combined = dsa::algorithm::concat<String>(*this, other);
    swap(combined);
    return *this;
}

inline void String::push_back(char value) {
    if (size_ == capacity_) {
        const size_type grown = capacity_ == 0 ? 1 : capacity_ * 2;
        reserve(grown);
    }
    data_[size_++] = value;
    data_[size_] = '\0';
}

inline void String::clear() noexcept {
    size_ = 0;
    data_[0] = '\0';
}

inline void String::reserve(size_type requested) {
    if (requested <= capacity_)
        return;

    char* replacement = new char[requested + 1];
    if (size_ > 0)
        std::memcpy(replacement, data_, size_);
    replacement[size_] = '\0';
    delete [] data_;
    data_ = replacement;
    capacity_ = requested;
}

inline size_type String::capacity() const noexcept {
    return capacity_;
}

inline char& String::operator[](size_type index) {
    return data_[index];
}

inline char String::operator[](size_type index) const {
    return data_[index];
}

inline bool String::operator==(const String& other) {
    return static_cast<const String&>(*this) == other;
}

inline bool String::operator==(const String& other) const {
    return dsa::algorithm::sequenceEqual(*this, other);
}

inline bool String::operator!=(const String& other) const {
    return !(*this == other);
}

inline bool String::operator<(const String& other) const {
    return dsa::algorithm::sequenceLess(*this, other);
}

inline String String::operator+(const String& other) {
    return static_cast<const String&>(*this) + other;
}

inline String String::operator+(const String& other) const {
    return dsa::algorithm::concat<String>(*this, other);
}

inline String String::operator+(char value) {
    return static_cast<const String&>(*this) + value;
}

inline String String::operator+(char value) const {
    return dsa::algorithm::concat<String>(*this, String(value));
}

template<typename Visitor>
void String::traverse(Visitor&& visit) {
    for (char* current = begin(); current != end(); ++current)
        visit(*current);
}

inline void String::traverse(void (*visit)(char&)) {
    for (char* current = begin(); current != end(); ++current)
        visit(*current);
}

inline char* String::begin() {
    return data_;
}

inline const char* String::begin() const {
    return data_;
}

inline char* String::end() {
    return data_ + size_;
}

inline const char* String::end() const {
    return data_ + size_;
}

inline void String::swap(String& other) noexcept {
    using std::swap;
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
}

inline void String::assign(const char* text, size_type count) {
    char* replacement = new char[count + 1];
    if (count > 0)
        std::memcpy(replacement, text, count);
    replacement[count] = '\0';
    delete [] data_;
    data_ = replacement;
    size_ = count;
    capacity_ = count;
}

inline void swap(String& left, String& right) noexcept {
    left.swap(right);
}

#endif
