#ifndef DSA_ALGORITHM_STRING_H
#define DSA_ALGORITHM_STRING_H

#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

namespace dsa {
namespace algorithm {

static const std::size_t stringNpos = (std::numeric_limits<std::size_t>::max)();

namespace detail {

/// 返回具有 size() 成员的普通序列长度。
template<typename Sequence>
std::size_t sequenceSizeDispatch(
    const Sequence& sequence,
    std::false_type,
    std::false_type
) {
    return static_cast<std::size_t>(sequence.size());
}

/// 返回以零结尾的字符指针长度；空指针视为空序列。
template<typename Sequence>
std::size_t sequenceSizeDispatch(
    const Sequence& sequence,
    std::false_type,
    std::true_type
) {
    typedef typename std::remove_cv<
        typename std::remove_pointer<Sequence>::type
    >::type char_type;
    return sequence == nullptr
        ? 0
        : std::char_traits<char_type>::length(sequence);
}

/// 字符数组末尾存在零字符时不把终止符计入序列长度。
template<typename Sequence>
std::size_t sequenceSizeDispatch(
    const Sequence& sequence,
    std::true_type,
    std::false_type
) {
    static const std::size_t length = std::extent<Sequence>::value;
    return length > 0 && sequence[length - 1] == typename std::remove_extent<Sequence>::type()
        ? length - 1
        : length;
}

/// 在支持 reserve() 时预留容量，避免构造结果时反复扩容。
template<typename Result>
auto reserveIfSupported(Result& result, std::size_t count, int)
    -> decltype(result.reserve(static_cast<decltype(result.size())>(count)), void()) {
    result.reserve(static_cast<decltype(result.size())>(count));
}

/// 不支持 reserve() 的结果类型保持原行为。
template<typename Result>
void reserveIfSupported(Result&, std::size_t, long) {
}

} // namespace detail

/// 返回 String、std::string、std::string_view、C 字符串等序列的长度。
template<typename Sequence>
std::size_t sequenceSize(const Sequence& sequence) {
    return detail::sequenceSizeDispatch(
        sequence,
        typename std::is_array<Sequence>::type(),
        typename std::is_pointer<Sequence>::type()
    );
}

namespace detail {

/// 对具有 size() 的序列使用其原生下标类型，避免窄化告警。
template<typename Sequence>
auto sequenceAtDispatch(const Sequence& sequence, std::size_t index, std::false_type)
    -> decltype(sequence[static_cast<decltype(sequence.size())>(index)]) {
    return sequence[static_cast<decltype(sequence.size())>(index)];
}

/// 数组和指针保持 std::size_t 下标。
template<typename Sequence>
auto sequenceAtDispatch(const Sequence& sequence, std::size_t index, std::true_type)
    -> decltype(sequence[index]) {
    return sequence[index];
}

} // namespace detail

/// 返回序列指定位置的只读元素，不负责越界检查。
template<typename Sequence>
auto sequenceAt(const Sequence& sequence, std::size_t index)
    -> decltype(detail::sequenceAtDispatch(
        sequence, index,
        typename std::integral_constant<bool,
            std::is_array<Sequence>::value || std::is_pointer<Sequence>::value
        >::type()
    )) {
    typedef typename std::integral_constant<bool,
        std::is_array<Sequence>::value || std::is_pointer<Sequence>::value
    >::type raw_sequence_tag;
    return detail::sequenceAtDispatch(sequence, index, raw_sequence_tag());
}

/// 将输入序列追加到提供 push_back() 的结果对象中。
template<typename Result, typename Sequence>
void appendSequence(Result& result, const Sequence& sequence) {
    const std::size_t count = sequenceSize(sequence);
    for (std::size_t index = 0; index < count; ++index)
        result.push_back(sequenceAt(sequence, index));
}

/// 从任意序列复制 [position, position + count) 并构造指定结果字符串类型。
template<typename Result, typename Sequence>
Result substring(
    const Sequence& sequence,
    std::size_t position,
    std::size_t count = stringNpos
) {
    const std::size_t length = sequenceSize(sequence);
    if (position >= length)
        return Result();

    const std::size_t available = length - position;
    const std::size_t selected = count < available ? count : available;
    Result result;
    detail::reserveIfSupported(result, selected, 0);
    for (std::size_t offset = 0; offset < selected; ++offset)
        result.push_back(sequenceAt(sequence, position + offset));
    return result;
}

/// 返回输入序列长度不超过 count 的前缀副本。
template<typename Result, typename Sequence>
Result prefix(const Sequence& sequence, std::size_t count) {
    return substring<Result>(sequence, 0, count);
}

/// 返回输入序列长度不超过 count 的后缀副本。
template<typename Result, typename Sequence>
Result suffix(const Sequence& sequence, std::size_t count) {
    const std::size_t length = sequenceSize(sequence);
    const std::size_t selected = count < length ? count : length;
    return substring<Result>(sequence, length - selected, selected);
}

/// 将两个任意序列拼接为指定结果字符串类型。
template<typename Result, typename Left, typename Right>
Result concat(const Left& left, const Right& right) {
    Result result;
    detail::reserveIfSupported(
        result,
        sequenceSize(left) + sequenceSize(right),
        0
    );
    appendSequence(result, left);
    appendSequence(result, right);
    return result;
}

/// 按元素逐一比较两个序列是否相等。
template<typename Left, typename Right>
bool sequenceEqual(const Left& left, const Right& right) {
    const std::size_t leftSize = sequenceSize(left);
    if (leftSize != sequenceSize(right))
        return false;

    for (std::size_t index = 0; index < leftSize; ++index) {
        if (!(sequenceAt(left, index) == sequenceAt(right, index)))
            return false;
    }
    return true;
}

/// 执行与 std::lexicographical_compare 相同的字典序比较。
template<typename Left, typename Right>
bool sequenceLess(const Left& left, const Right& right) {
    const std::size_t leftSize = sequenceSize(left);
    const std::size_t rightSize = sequenceSize(right);
    const std::size_t common = leftSize < rightSize ? leftSize : rightSize;

    for (std::size_t index = 0; index < common; ++index) {
        if (sequenceAt(left, index) < sequenceAt(right, index))
            return true;
        if (sequenceAt(right, index) < sequenceAt(left, index))
            return false;
    }
    return leftSize < rightSize;
}

/// 判断 sequence 是否以 candidate 开头。
template<typename Sequence, typename Candidate>
bool startsWith(const Sequence& sequence, const Candidate& candidate) {
    const std::size_t candidateSize = sequenceSize(candidate);
    if (candidateSize > sequenceSize(sequence))
        return false;

    for (std::size_t index = 0; index < candidateSize; ++index) {
        if (!(sequenceAt(sequence, index) == sequenceAt(candidate, index)))
            return false;
    }
    return true;
}

/// 判断 sequence 是否以 candidate 结尾。
template<typename Sequence, typename Candidate>
bool endsWith(const Sequence& sequence, const Candidate& candidate) {
    const std::size_t sequenceLength = sequenceSize(sequence);
    const std::size_t candidateSize = sequenceSize(candidate);
    if (candidateSize > sequenceLength)
        return false;

    const std::size_t offset = sequenceLength - candidateSize;
    for (std::size_t index = 0; index < candidateSize; ++index) {
        if (!(sequenceAt(sequence, offset + index) == sequenceAt(candidate, index)))
            return false;
    }
    return true;
}

/// 使用朴素算法查找 pattern 首次出现位置，未找到返回 stringNpos。
template<typename Pattern, typename Text>
std::size_t findNaive(const Pattern& pattern, const Text& text) {
    const std::size_t patternSize = sequenceSize(pattern);
    const std::size_t textSize = sequenceSize(text);
    if (patternSize == 0)
        return 0;
    if (patternSize > textSize)
        return stringNpos;

    for (std::size_t start = 0; start + patternSize <= textSize; ++start) {
        std::size_t index = 0;
        while (index < patternSize &&
               sequenceAt(pattern, index) == sequenceAt(text, start + index)) {
            ++index;
        }
        if (index == patternSize)
            return start;
    }
    return stringNpos;
}

/// 将字符安全转换为 R-way Trie 的非负字母表下标。
template<typename Symbol>
std::size_t alphabetIndex(Symbol symbol) {
    typedef typename std::decay<Symbol>::type symbol_type;
    typedef typename std::make_unsigned<symbol_type>::type unsigned_symbol_type;
    return static_cast<std::size_t>(static_cast<unsigned_symbol_type>(symbol));
}

} // namespace algorithm
} // namespace dsa

#endif
