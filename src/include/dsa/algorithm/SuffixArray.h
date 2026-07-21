#ifndef DSA_ALGORITHM_SUFFIX_ARRAY_H
#define DSA_ALGORITHM_SUFFIX_ARRAY_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dsa {
namespace algorithm {

// 倍增法构建后缀下标；返回按字典序排列的起点，复杂度 O(n log n)。
template<typename Text, typename Less>
std::vector<int> buildSuffixIndices(const Text& text, Less less) {
    const int count = static_cast<int>(text.size());
    std::vector<int> suffixes(static_cast<std::size_t>(count));
    if (count == 0)
        return suffixes;

    std::vector<int> ranks(static_cast<std::size_t>(count));
    std::vector<int> nextRanks(static_cast<std::size_t>(count));
    std::vector<int> temporary(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
        suffixes[static_cast<std::size_t>(i)] = i;

    std::stable_sort(
        suffixes.begin(), suffixes.end(),
        [&text, &less](int left, int right) {
            return less(text[static_cast<std::size_t>(left)], text[static_cast<std::size_t>(right)]);
        }
    );

    int rankCount = 0;
    ranks[static_cast<std::size_t>(suffixes[0])] = 0;
    for (int i = 1; i < count; ++i) {
        const int previous = suffixes[static_cast<std::size_t>(i - 1)];
        const int current = suffixes[static_cast<std::size_t>(i)];
        const bool different =
            less(text[static_cast<std::size_t>(previous)], text[static_cast<std::size_t>(current)]) ||
            less(text[static_cast<std::size_t>(current)], text[static_cast<std::size_t>(previous)]);
        if (different)
            ++rankCount;
        ranks[static_cast<std::size_t>(current)] = rankCount;
    }

    std::vector<int> counts(static_cast<std::size_t>(count + 1));
    const auto countingSort = [&](int offset) {
        std::fill(counts.begin(), counts.end(), 0);
        for (int i = 0; i < count; ++i) {
            const int position = suffixes[static_cast<std::size_t>(i)];
            const int bucket = position + offset < count
                ? ranks[static_cast<std::size_t>(position + offset)] + 1
                : 0;
            ++counts[static_cast<std::size_t>(bucket)];
        }
        for (std::size_t i = 1; i < counts.size(); ++i)
            counts[i] += counts[i - 1];
        for (int i = count - 1; i >= 0; --i) {
            const int position = suffixes[static_cast<std::size_t>(i)];
            const int bucket = position + offset < count
                ? ranks[static_cast<std::size_t>(position + offset)] + 1
                : 0;
            temporary[static_cast<std::size_t>(--counts[static_cast<std::size_t>(bucket)])] = position;
        }
        suffixes.swap(temporary);
    };

    for (int offset = 1; rankCount < count - 1; offset <<= 1) {
        countingSort(offset);
        countingSort(0);
        rankCount = 0;
        nextRanks[static_cast<std::size_t>(suffixes[0])] = 0;
        for (int i = 1; i < count; ++i) {
            const int current = suffixes[static_cast<std::size_t>(i)];
            const int previous = suffixes[static_cast<std::size_t>(i - 1)];
            const int currentSecond = current + offset < count
                ? ranks[static_cast<std::size_t>(current + offset)] : -1;
            const int previousSecond = previous + offset < count
                ? ranks[static_cast<std::size_t>(previous + offset)] : -1;
            if (ranks[static_cast<std::size_t>(current)] != ranks[static_cast<std::size_t>(previous)] ||
                currentSecond != previousSecond)
                ++rankCount;
            nextRanks[static_cast<std::size_t>(current)] = rankCount;
        }
        ranks.swap(nextRanks);
        if (offset > count / 2)
            break;
    }
    return suffixes;
}

template<typename Text>
std::vector<int> buildSuffixIndices(const Text& text) {
    typedef typename Text::value_type value_type;
    return buildSuffixIndices(text, std::less<value_type>());
}

// 只拥有文本副本和后缀下标的工业算法对象；不依赖仓库容器。
template<typename Text, typename Less = std::less<typename Text::value_type> >
class SuffixArray {
public:
    typedef typename Text::value_type value_type;

    explicit SuffixArray(const Text& text, const Less& less = Less())
        : text_(text), suffixes_(buildSuffixIndices(text_, less)), less_(less) {}

    std::size_t size() const { return suffixes_.size(); }
    bool empty() const { return suffixes_.empty(); }
    int length() const { return static_cast<int>(suffixes_.size()); }

    int index(int position) const {
        checkIndex(position);
        return suffixes_[static_cast<std::size_t>(position)];
    }

    std::pair<int, int> select_view(int position) const {
        return std::make_pair(index(position), length());
    }

    Text select(int position) const {
        const int start = index(position);
        return Text(text_.begin() + start, text_.end());
    }

    int lcp(int position) const {
        if (position <= 0 || position >= length())
            throw std::out_of_range("SuffixArray::lcp index out of range");
        return lcpSuffix(index(position), index(position - 1));
    }

    int rank(const Text& key) const {
        int first = 0;
        int last = length();
        while (first < last) {
            const int middle = first + (last - first) / 2;
            const int comparison = compareKeySuffix(key, suffixes_[static_cast<std::size_t>(middle)]);
            if (comparison > 0)
                first = middle + 1;
            else
                last = middle;
        }
        return first;
    }

    const Text& text() const { return text_; }
    const std::vector<int>& indices() const { return suffixes_; }

private:
    Text text_;
    std::vector<int> suffixes_;
    Less less_;

    void checkIndex(int position) const {
        if (position < 0 || position >= length())
            throw std::out_of_range("SuffixArray index out of range");
    }

    bool equalValue(const value_type& left, const value_type& right) const {
        return !less_(left, right) && !less_(right, left);
    }

    int lcpSuffix(int left, int right) const {
        int count = 0;
        while (left < length() && right < length() &&
               equalValue(text_[static_cast<std::size_t>(left)], text_[static_cast<std::size_t>(right)])) {
            ++left;
            ++right;
            ++count;
        }
        return count;
    }

    int compareKeySuffix(const Text& key, int suffixStart) const {
        int keyIndex = 0;
        int textIndex = suffixStart;
        const int keySize = static_cast<int>(key.size());
        while (keyIndex < keySize && textIndex < length() &&
               equalValue(key[static_cast<std::size_t>(keyIndex)], text_[static_cast<std::size_t>(textIndex)])) {
            ++keyIndex;
            ++textIndex;
        }
        if (keyIndex == keySize && textIndex == length())
            return 0;
        if (keyIndex == keySize)
            return -1;
        if (textIndex == length())
            return 1;
        if (less_(key[static_cast<std::size_t>(keyIndex)], text_[static_cast<std::size_t>(textIndex)]))
            return -1;
        if (less_(text_[static_cast<std::size_t>(textIndex)], key[static_cast<std::size_t>(keyIndex)]))
            return 1;
        return 0;
    }
};

} // namespace algorithm
} // namespace dsa

#endif
