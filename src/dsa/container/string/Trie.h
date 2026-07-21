#ifndef DSA_CONTAINER_STRING_TRIE_H
#define DSA_CONTAINER_STRING_TRIE_H

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../algorithm/String.h"
#include "../../core/string/TrieAlgorithm.h"

namespace dsa {
namespace container {

/// 工业 R-way Trie 映射：节点和值使用独占所有权，允许存储零值和 move-only 值。
template<typename T, std::size_t Alphabet = 256, typename CharT = char>
class Trie {
public:
    typedef T mapped_type;
    typedef CharT char_type;
    typedef std::basic_string<char_type> string_type;
    typedef std::size_t size_type;

private:
    struct Node {
        std::unique_ptr<mapped_type> value;
        std::array<std::unique_ptr<Node>, Alphabet> children;
    };

    struct Access {
        static std::size_t index(char_type symbol) {
            return dsa::algorithm::alphabetIndex(symbol);
        }

        static bool validIndex(std::size_t indexValue) {
            return indexValue < Alphabet;
        }

        static Node* child(Node* node, std::size_t indexValue) {
            return node->children[indexValue].get();
        }

        static bool terminal(Node* node) {
            return node->value.get() != nullptr;
        }
    };

    typedef dsa::core::TrieAlgorithm<Access> ReadAlgorithm;

    std::unique_ptr<Node> root_;
    size_type size_;

    /// 深拷贝子树；值不可拷贝时仅在实际使用拷贝操作时产生编译错误。
    static std::unique_ptr<Node> clone(const std::unique_ptr<Node>& node);

    /// 判断节点是否不含值和任何子节点。
    static bool emptyNode(const Node& node);

    /// 递归删除并剪枝。
    template<typename Key>
    static bool eraseNode(
        std::unique_ptr<Node>& node,
        const Key& key,
        size_type depth,
        bool& removed
    );

    /// 收集子树中的全部键。
    static void collect(
        Node* node,
        const string_type& prefix,
        std::vector<string_type>& output
    );

    /// 按点号通配模式收集等长键。
    template<typename Pattern>
    static void collectMatch(
        Node* node,
        const string_type& prefix,
        const Pattern& pattern,
        size_type depth,
        std::vector<string_type>& output
    );

    /// 将任意键序列复制为 string_type。
    template<typename Key>
    static string_type makeKey(const Key& key);

public:
    Trie();
    Trie(const Trie& other);
    Trie(Trie&& other) noexcept;
    ~Trie() = default;

    Trie& operator=(const Trie& other);
    Trie& operator=(Trie&& other) noexcept;

    /// 插入或替换键值；返回 true 表示新增键，false 表示覆盖。
    template<typename Key, typename Value>
    bool insert(const Key& key, Value&& value);

    /// 返回键对应值指针；未命中返回 nullptr。
    template<typename Key>
    mapped_type* find(const Key& key);

    template<typename Key>
    const mapped_type* find(const Key& key) const;

    template<typename Key>
    bool contains(const Key& key) const;

    /// 删除键；只有真实删除时返回 true。
    template<typename Key>
    bool erase(const Key& key);

    void clear();
    bool empty() const noexcept;
    size_type size() const noexcept;

    template<typename Key>
    std::vector<string_type> keysWithPrefix(const Key& prefix) const;

    template<typename Pattern>
    std::vector<string_type> keysThatMatch(const Pattern& pattern) const;

    template<typename Key>
    string_type longestPrefixOf(const Key& input) const;

    void swap(Trie& other) noexcept;
};

template<typename T, std::size_t Alphabet, typename CharT>
Trie<T, Alphabet, CharT>::Trie()
    : root_(new Node()), size_(0) {
}

template<typename T, std::size_t Alphabet, typename CharT>
Trie<T, Alphabet, CharT>::Trie(const Trie& other)
    : root_(clone(other.root_)), size_(other.size_) {
}

template<typename T, std::size_t Alphabet, typename CharT>
Trie<T, Alphabet, CharT>::Trie(Trie&& other) noexcept
    : root_(std::move(other.root_)), size_(other.size_) {
    other.size_ = 0;
}

template<typename T, std::size_t Alphabet, typename CharT>
Trie<T, Alphabet, CharT>&
Trie<T, Alphabet, CharT>::operator=(const Trie& other) {
    if (this != &other) {
        Trie copy(other);
        swap(copy);
    }
    return *this;
}

template<typename T, std::size_t Alphabet, typename CharT>
Trie<T, Alphabet, CharT>&
Trie<T, Alphabet, CharT>::operator=(Trie&& other) noexcept {
    if (this != &other) {
        root_ = std::move(other.root_);
        size_ = other.size_;
        other.size_ = 0;
    }
    return *this;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key, typename Value>
bool Trie<T, Alphabet, CharT>::insert(const Key& key, Value&& value) {
    if (!root_)
        root_.reset(new Node());

    Node* current = root_.get();
    const size_type length = dsa::algorithm::sequenceSize(key);
    for (size_type depth = 0; depth < length; ++depth) {
        const size_type indexValue = Access::index(
            static_cast<char_type>(dsa::algorithm::sequenceAt(key, depth))
        );
        if (!Access::validIndex(indexValue))
            throw std::out_of_range("dsa::container::Trie alphabet index");
        if (!current->children[indexValue])
            current->children[indexValue].reset(new Node());
        current = current->children[indexValue].get();
    }

    const bool inserted = !current->value;
    current->value.reset(new mapped_type(std::forward<Value>(value)));
    if (inserted)
        ++size_;
    return inserted;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
typename Trie<T, Alphabet, CharT>::mapped_type*
Trie<T, Alphabet, CharT>::find(const Key& key) {
    Node* node = ReadAlgorithm::find(root_.get(), key);
    return node == nullptr ? nullptr : node->value.get();
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
const typename Trie<T, Alphabet, CharT>::mapped_type*
Trie<T, Alphabet, CharT>::find(const Key& key) const {
    Node* node = ReadAlgorithm::find(root_.get(), key);
    return node == nullptr ? nullptr : node->value.get();
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
bool Trie<T, Alphabet, CharT>::contains(const Key& key) const {
    return find(key) != nullptr;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
bool Trie<T, Alphabet, CharT>::erase(const Key& key) {
    bool removed = false;
    eraseNode(root_, key, 0, removed);
    if (removed)
        --size_;
    return removed;
}

template<typename T, std::size_t Alphabet, typename CharT>
void Trie<T, Alphabet, CharT>::clear() {
    root_.reset(new Node());
    size_ = 0;
}

template<typename T, std::size_t Alphabet, typename CharT>
bool Trie<T, Alphabet, CharT>::empty() const noexcept {
    return size_ == 0;
}

template<typename T, std::size_t Alphabet, typename CharT>
typename Trie<T, Alphabet, CharT>::size_type
Trie<T, Alphabet, CharT>::size() const noexcept {
    return size_;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
std::vector<typename Trie<T, Alphabet, CharT>::string_type>
Trie<T, Alphabet, CharT>::keysWithPrefix(const Key& prefix) const {
    std::vector<string_type> output;
    Node* node = ReadAlgorithm::find(root_.get(), prefix);
    collect(node, makeKey(prefix), output);
    return output;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Pattern>
std::vector<typename Trie<T, Alphabet, CharT>::string_type>
Trie<T, Alphabet, CharT>::keysThatMatch(const Pattern& pattern) const {
    std::vector<string_type> output;
    collectMatch(root_.get(), string_type(), pattern, 0, output);
    return output;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
typename Trie<T, Alphabet, CharT>::string_type
Trie<T, Alphabet, CharT>::longestPrefixOf(const Key& input) const {
    return dsa::algorithm::substring<string_type>(
        input,
        0,
        ReadAlgorithm::longestPrefixLength(root_.get(), input)
    );
}

template<typename T, std::size_t Alphabet, typename CharT>
void Trie<T, Alphabet, CharT>::swap(Trie& other) noexcept {
    root_.swap(other.root_);
    std::swap(size_, other.size_);
}

template<typename T, std::size_t Alphabet, typename CharT>
std::unique_ptr<typename Trie<T, Alphabet, CharT>::Node>
Trie<T, Alphabet, CharT>::clone(const std::unique_ptr<Node>& node) {
    if (!node)
        return std::unique_ptr<Node>();

    std::unique_ptr<Node> copy(new Node());
    if (node->value)
        copy->value.reset(new mapped_type(*node->value));
    for (size_type indexValue = 0; indexValue < Alphabet; ++indexValue)
        copy->children[indexValue] = clone(node->children[indexValue]);
    return copy;
}

template<typename T, std::size_t Alphabet, typename CharT>
bool Trie<T, Alphabet, CharT>::emptyNode(const Node& node) {
    if (node.value)
        return false;
    for (size_type indexValue = 0; indexValue < Alphabet; ++indexValue) {
        if (node.children[indexValue])
            return false;
    }
    return true;
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
bool Trie<T, Alphabet, CharT>::eraseNode(
    std::unique_ptr<Node>& node,
    const Key& key,
    size_type depth,
    bool& removed
) {
    if (!node)
        return true;

    const size_type length = dsa::algorithm::sequenceSize(key);
    if (depth == length) {
        if (node->value) {
            node->value.reset();
            removed = true;
        }
    } else {
        const size_type indexValue = Access::index(
            static_cast<char_type>(dsa::algorithm::sequenceAt(key, depth))
        );
        if (!Access::validIndex(indexValue))
            return false;
        eraseNode(node->children[indexValue], key, depth + 1, removed);
    }

    if (emptyNode(*node)) {
        node.reset();
        return true;
    }
    return false;
}

template<typename T, std::size_t Alphabet, typename CharT>
void Trie<T, Alphabet, CharT>::collect(
    Node* node,
    const string_type& prefix,
    std::vector<string_type>& output
) {
    if (node == nullptr)
        return;
    if (node->value)
        output.push_back(prefix);

    for (size_type indexValue = 0; indexValue < Alphabet; ++indexValue) {
        if (node->children[indexValue]) {
            string_type next(prefix);
            next.push_back(static_cast<char_type>(indexValue));
            collect(node->children[indexValue].get(), next, output);
        }
    }
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Pattern>
void Trie<T, Alphabet, CharT>::collectMatch(
    Node* node,
    const string_type& prefix,
    const Pattern& pattern,
    size_type depth,
    std::vector<string_type>& output
) {
    if (node == nullptr)
        return;

    const size_type length = dsa::algorithm::sequenceSize(pattern);
    if (depth == length) {
        if (node->value)
            output.push_back(prefix);
        return;
    }

    const char_type expected = static_cast<char_type>(
        dsa::algorithm::sequenceAt(pattern, depth)
    );
    if (expected == static_cast<char_type>('.')) {
        for (size_type indexValue = 0; indexValue < Alphabet; ++indexValue) {
            if (node->children[indexValue]) {
                string_type next(prefix);
                next.push_back(static_cast<char_type>(indexValue));
                collectMatch(
                    node->children[indexValue].get(),
                    next,
                    pattern,
                    depth + 1,
                    output
                );
            }
        }
        return;
    }

    const size_type indexValue = Access::index(expected);
    if (!Access::validIndex(indexValue))
        return;
    string_type next(prefix);
    next.push_back(expected);
    collectMatch(
        node->children[indexValue].get(),
        next,
        pattern,
        depth + 1,
        output
    );
}

template<typename T, std::size_t Alphabet, typename CharT>
template<typename Key>
typename Trie<T, Alphabet, CharT>::string_type
Trie<T, Alphabet, CharT>::makeKey(const Key& key) {
    string_type result;
    dsa::algorithm::appendSequence(result, key);
    return result;
}

template<typename T, std::size_t Alphabet, typename CharT>
void swap(
    Trie<T, Alphabet, CharT>& left,
    Trie<T, Alphabet, CharT>& right
) noexcept {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
