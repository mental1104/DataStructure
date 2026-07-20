#ifndef DSA_CONTAINER_STRING_STRING_H
#define DSA_CONTAINER_STRING_STRING_H

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "../vector/Vector.h"
#include "../../algorithm/String.h"

namespace dsa {
namespace container {

/// 基于工业 Vector 的连续字符串容器，保持末尾零字符并复用 allocator/lifecycle 语义。
template<
    typename CharT,
    typename Traits = std::char_traits<CharT>,
    typename Allocator = std::allocator<CharT>
>
class BasicString {
public:
    typedef CharT value_type;
    typedef Traits traits_type;
    typedef Allocator allocator_type;
    typedef dsa::container::Vector<value_type, allocator_type> storage_type;
    typedef typename storage_type::size_type size_type;
    typedef typename storage_type::difference_type difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = static_cast<size_type>(-1);

    /// 构造仅含终止符的空字符串。
    BasicString();

    /// 使用指定 allocator 构造空字符串。
    explicit BasicString(const allocator_type& allocator);

    /// 从零结尾字符串复制构造。
    BasicString(
        const value_type* text,
        const allocator_type& allocator = allocator_type()
    );

    /// 从指针复制 count 个字符，允许内容包含零字符。
    BasicString(
        const value_type* text,
        size_type count,
        const allocator_type& allocator = allocator_type()
    );

    /// 构造 count 个 value 字符。
    BasicString(
        size_type count,
        value_type value,
        const allocator_type& allocator = allocator_type()
    );

    /// 从迭代器区间复制构造。
    template<typename InputIt>
    BasicString(
        InputIt first,
        InputIt last,
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIt>::value>::type* = nullptr
    );

    /// 从 initializer_list 复制构造。
    BasicString(
        std::initializer_list<value_type> values,
        const allocator_type& allocator = allocator_type()
    );

    /// 深拷贝字符和 allocator 语义。
    BasicString(const BasicString& other) = default;

    /// 使用指定 allocator 深拷贝字符。
    BasicString(const BasicString& other, const allocator_type& allocator);

    /// 接管底层 Vector，并将源对象恢复为空字符串。
    BasicString(BasicString&& other);

    /// 使用指定 allocator 移动构造。
    BasicString(BasicString&& other, const allocator_type& allocator);

    ~BasicString() = default;

    BasicString& operator=(const BasicString& other) = default;
    BasicString& operator=(BasicString&& other);
    BasicString& operator=(const value_type* text);
    BasicString& operator=(std::initializer_list<value_type> values);

    allocator_type get_allocator() const noexcept;

    reference at(size_type position);
    const_reference at(size_type position) const;
    reference operator[](size_type position) noexcept;
    const_reference operator[](size_type position) const noexcept;
    reference front() noexcept;
    const_reference front() const noexcept;
    reference back() noexcept;
    const_reference back() const noexcept;

    pointer data() noexcept;
    const_pointer data() const noexcept;
    const_pointer c_str() const noexcept;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type length() const noexcept;
    size_type capacity() const noexcept;
    size_type max_size() const noexcept;

    /// 预留 requested 个有效字符以及一个终止符的空间。
    void reserve(size_type requested);

    /// 尝试压缩底层 Vector 容量，同时保留终止符。
    void shrink_to_fit();

    /// 清空有效字符但保留终止符。
    void clear();

    /// 在终止符之前插入一个字符。
    void push_back(value_type value);

    /// 删除最后一个有效字符；空字符串调用行为未定义。
    void pop_back();

    /// 将任意序列追加到当前字符串，先构造临时结果再提交。
    template<typename Sequence>
    BasicString& append(const Sequence& sequence);

    BasicString& operator+=(const BasicString& other);
    BasicString& operator+=(value_type value);
    BasicString& operator+=(const value_type* text);

    /// 返回独立子串，越界位置返回空字符串。
    BasicString substr(size_type position = 0, size_type count = npos) const;

    /// 按字典序比较，返回负数、零或正数。
    int compare(const BasicString& other) const;

    /// 按 allocator 规则交换底层 Vector。
    void swap(BasicString& other);

private:
    storage_type storage_;

    /// 确保 storage_ 至少包含一个末尾零字符，用于修复 moved-from 状态。
    void ensureTerminator();

    /// 从任意序列构造临时字符串并提交。
    template<typename Sequence>
    void assignSequence(const Sequence& sequence);
};

template<typename CharT, typename Traits, typename Allocator>
const typename BasicString<CharT, Traits, Allocator>::size_type
    BasicString<CharT, Traits, Allocator>::npos;

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString()
    : storage_(1, value_type()) {
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    const allocator_type& allocator
) : storage_(1, value_type(), allocator) {
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    const value_type* text,
    const allocator_type& allocator
) : storage_(1, value_type(), allocator) {
    if (text != nullptr)
        append(text);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    const value_type* text,
    size_type count,
    const allocator_type& allocator
) : storage_(1, value_type(), allocator) {
    reserve(count);
    for (size_type index = 0; text != nullptr && index < count; ++index)
        push_back(text[index]);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    size_type count,
    value_type value,
    const allocator_type& allocator
) : storage_(1, value_type(), allocator) {
    reserve(count);
    for (size_type index = 0; index < count; ++index)
        push_back(value);
}

template<typename CharT, typename Traits, typename Allocator>
template<typename InputIt>
BasicString<CharT, Traits, Allocator>::BasicString(
    InputIt first,
    InputIt last,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIt>::value>::type*
) : storage_(1, value_type(), allocator) {
    for (; first != last; ++first)
        push_back(*first);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    std::initializer_list<value_type> values,
    const allocator_type& allocator
) : BasicString(values.begin(), values.end(), allocator) {
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    const BasicString& other,
    const allocator_type& allocator
) : storage_(other.storage_, allocator) {
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(BasicString&& other)
    : storage_(std::move(other.storage_)) {
    ensureTerminator();
    other.ensureTerminator();
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>::BasicString(
    BasicString&& other,
    const allocator_type& allocator
) : storage_(std::move(other.storage_), allocator) {
    ensureTerminator();
    other.ensureTerminator();
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator=(BasicString&& other) {
    if (this != &other) {
        storage_ = std::move(other.storage_);
        ensureTerminator();
        other.ensureTerminator();
    }
    return *this;
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator=(const value_type* text) {
    assignSequence(text);
    return *this;
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator=(
    std::initializer_list<value_type> values
) {
    BasicString replacement(values, get_allocator());
    swap(replacement);
    return *this;
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::allocator_type
BasicString<CharT, Traits, Allocator>::get_allocator() const noexcept {
    return storage_.get_allocator();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reference
BasicString<CharT, Traits, Allocator>::at(size_type position) {
    if (position >= size())
        throw std::out_of_range("dsa::container::BasicString::at");
    return storage_[position];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reference
BasicString<CharT, Traits, Allocator>::at(size_type position) const {
    if (position >= size())
        throw std::out_of_range("dsa::container::BasicString::at");
    return storage_[position];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reference
BasicString<CharT, Traits, Allocator>::operator[](size_type position) noexcept {
    return storage_[position];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reference
BasicString<CharT, Traits, Allocator>::operator[](size_type position) const noexcept {
    return storage_[position];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reference
BasicString<CharT, Traits, Allocator>::front() noexcept {
    return storage_.front();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reference
BasicString<CharT, Traits, Allocator>::front() const noexcept {
    return storage_.front();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reference
BasicString<CharT, Traits, Allocator>::back() noexcept {
    return storage_[size() - 1];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reference
BasicString<CharT, Traits, Allocator>::back() const noexcept {
    return storage_[size() - 1];
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::pointer
BasicString<CharT, Traits, Allocator>::data() noexcept {
    return storage_.data();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_pointer
BasicString<CharT, Traits, Allocator>::data() const noexcept {
    return storage_.data();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_pointer
BasicString<CharT, Traits, Allocator>::c_str() const noexcept {
    return storage_.data();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::iterator
BasicString<CharT, Traits, Allocator>::begin() noexcept {
    return storage_.data();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_iterator
BasicString<CharT, Traits, Allocator>::begin() const noexcept {
    return storage_.data();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_iterator
BasicString<CharT, Traits, Allocator>::cbegin() const noexcept {
    return begin();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::iterator
BasicString<CharT, Traits, Allocator>::end() noexcept {
    return storage_.data() + size();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_iterator
BasicString<CharT, Traits, Allocator>::end() const noexcept {
    return storage_.data() + size();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_iterator
BasicString<CharT, Traits, Allocator>::cend() const noexcept {
    return end();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reverse_iterator
BasicString<CharT, Traits, Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reverse_iterator
BasicString<CharT, Traits, Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reverse_iterator
BasicString<CharT, Traits, Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(cend());
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::reverse_iterator
BasicString<CharT, Traits, Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reverse_iterator
BasicString<CharT, Traits, Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::const_reverse_iterator
BasicString<CharT, Traits, Allocator>::crend() const noexcept {
    return const_reverse_iterator(cbegin());
}

template<typename CharT, typename Traits, typename Allocator>
bool BasicString<CharT, Traits, Allocator>::empty() const noexcept {
    return size() == 0;
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::size_type
BasicString<CharT, Traits, Allocator>::size() const noexcept {
    return storage_.size() == 0 ? 0 : storage_.size() - 1;
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::size_type
BasicString<CharT, Traits, Allocator>::length() const noexcept {
    return size();
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::size_type
BasicString<CharT, Traits, Allocator>::capacity() const noexcept {
    return storage_.capacity() == 0 ? 0 : storage_.capacity() - 1;
}

template<typename CharT, typename Traits, typename Allocator>
typename BasicString<CharT, Traits, Allocator>::size_type
BasicString<CharT, Traits, Allocator>::max_size() const noexcept {
    return storage_.max_size() == 0 ? 0 : storage_.max_size() - 1;
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::reserve(size_type requested) {
    storage_.reserve(requested + 1);
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::shrink_to_fit() {
    storage_.shrink_to_fit();
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::clear() {
    storage_.resize(1);
    storage_[0] = value_type();
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::push_back(value_type value) {
    storage_.insert(storage_.end() - 1, std::move(value));
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::pop_back() {
    storage_.erase(storage_.end() - 2);
}

template<typename CharT, typename Traits, typename Allocator>
template<typename Sequence>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::append(const Sequence& sequence) {
    BasicString combined(get_allocator());
    combined.reserve(size() + static_cast<size_type>(dsa::algorithm::sequenceSize(sequence)));
    dsa::algorithm::appendSequence(combined, *this);
    dsa::algorithm::appendSequence(combined, sequence);
    swap(combined);
    return *this;
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator+=(const BasicString& other) {
    return append(other);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator+=(value_type value) {
    push_back(value);
    return *this;
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>&
BasicString<CharT, Traits, Allocator>::operator+=(const value_type* text) {
    return append(text);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator>
BasicString<CharT, Traits, Allocator>::substr(
    size_type position,
    size_type count
) const {
    const size_type currentSize = size();
    if (position >= currentSize)
        return BasicString(get_allocator());
    const size_type available = currentSize - position;
    const size_type selected = count < available ? count : available;
    BasicString result(get_allocator());
    result.reserve(selected);
    for (size_type offset = 0; offset < selected; ++offset)
        result.push_back((*this)[position + offset]);
    return result;
}

template<typename CharT, typename Traits, typename Allocator>
int BasicString<CharT, Traits, Allocator>::compare(const BasicString& other) const {
    if (dsa::algorithm::sequenceEqual(*this, other))
        return 0;
    return dsa::algorithm::sequenceLess(*this, other) ? -1 : 1;
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::swap(BasicString& other) {
    storage_.swap(other.storage_);
}

template<typename CharT, typename Traits, typename Allocator>
void BasicString<CharT, Traits, Allocator>::ensureTerminator() {
    if (storage_.empty())
        storage_.push_back(value_type());
    else
        storage_.back() = value_type();
}

template<typename CharT, typename Traits, typename Allocator>
template<typename Sequence>
void BasicString<CharT, Traits, Allocator>::assignSequence(
    const Sequence& sequence
) {
    BasicString replacement(get_allocator());
    replacement.append(sequence);
    swap(replacement);
}

template<typename CharT, typename Traits, typename Allocator>
bool operator==(
    const BasicString<CharT, Traits, Allocator>& left,
    const BasicString<CharT, Traits, Allocator>& right
) {
    return dsa::algorithm::sequenceEqual(left, right);
}

template<typename CharT, typename Traits, typename Allocator>
bool operator!=(
    const BasicString<CharT, Traits, Allocator>& left,
    const BasicString<CharT, Traits, Allocator>& right
) {
    return !(left == right);
}

template<typename CharT, typename Traits, typename Allocator>
bool operator<(
    const BasicString<CharT, Traits, Allocator>& left,
    const BasicString<CharT, Traits, Allocator>& right
) {
    return dsa::algorithm::sequenceLess(left, right);
}

template<typename CharT, typename Traits, typename Allocator>
BasicString<CharT, Traits, Allocator> operator+(
    const BasicString<CharT, Traits, Allocator>& left,
    const BasicString<CharT, Traits, Allocator>& right
) {
    return dsa::algorithm::concat<BasicString<CharT, Traits, Allocator> >(
        left,
        right
    );
}

template<typename CharT, typename Traits, typename Allocator>
void swap(
    BasicString<CharT, Traits, Allocator>& left,
    BasicString<CharT, Traits, Allocator>& right
) {
    left.swap(right);
}

typedef BasicString<char> String;

} // namespace container
} // namespace dsa

#endif
