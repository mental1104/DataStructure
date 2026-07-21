#ifndef DSA_CONTAINER_STRING_TERNARY_SEARCH_TRIE_H
#define DSA_CONTAINER_STRING_TERNARY_SEARCH_TRIE_H

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <dsa/container/vector/Vector.h>

#include "../../algorithm/String.h"
#include "../../core/string/TrieAlgorithm.h"

namespace dsa {
namespace container {

/// 工业 Ternary Search Trie 映射：使用 unique_ptr 管理节点和值，支持 move-only 值。
template<typename T, typename CharT = char>
class TernarySearchTrie {
public:
    typedef T mapped_type;
    typedef CharT char_type;
    typedef std::basic_string<char_type> string_type;
    typedef std::size_t size_type;

private:
    struct Node {
        explicit Node(char_type character)
            : symbol(character) {
        }

        char_type symbol;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> middle;
        std::unique_ptr<Node> right;
        std::unique_ptr<mapped_type> value;
    };

    struct Access {
        static char_type symbol(Node* node) {
            return node->symbol;
        }

        static Node* left(Node* node) {
            return node->left.get();
        }

        static Node* middle(Node* node) {
            return node->middle.get();
        }

        static Node* right(Node* node) {
            return node->right.get();
        }

        static bool terminal(Node* node) {
            return node->value.get() != nullptr;
        }
    };

    typedef dsa::core::TernarySearchTrieAlgorithm<Access> ReadAlgorithm;

    std::unique_ptr<Node> root_;
    std::unique_ptr<mapped_type> emptyValue_;
    size_type size_;

    static std::unique_ptr<Node> clone(const std::unique_ptr<Node>& node);

    template<typename Key, typename Value>
    static Node* insertNode(
        std::unique_ptr<Node>& node,
        const Key& key,
        Value&& value,
        size_type depth,
        bool& inserted
    );

    template<typename Key>
    static bool eraseNode(
        std::unique_ptr<Node>& node,
        const Key& key,
        size_type depth,
        bool& removed
    );

    static bool emptyNode(const Node& node);

    static void collect(
        Node* node,
        const string_type& prefix,
        dsa::container::Vector<string_type>& output
    );

    template<typename Pattern>
    static void collectMatch(
        Node* node,
        const string_type& prefix,
        const Pattern& pattern,
        size_type depth,
        dsa::container::Vector<string_type>& output
    );

    template<typename Key>
    static string_type makeKey(const Key& key);

public:
    TernarySearchTrie();
    TernarySearchTrie(const TernarySearchTrie& other);
    TernarySearchTrie(TernarySearchTrie&& other) noexcept;
    ~TernarySearchTrie() = default;

    TernarySearchTrie& operator=(const TernarySearchTrie& other);
    TernarySearchTrie& operator=(TernarySearchTrie&& other) noexcept;

    /// 插入或替换键值；空键由独立槽位保存。
    template<typename Key, typename Value>
    bool insert(const Key& key, Value&& value);

    template<typename Key>
    mapped_type* find(const Key& key);

    template<typename Key>
    const mapped_type* find(const Key& key) const;

    template<typename Key>
    bool contains(const Key& key) const;

    template<typename Key>
    bool erase(const Key& key);

    void clear() noexcept;
    bool empty() const noexcept;
    size_type size() const noexcept;

    template<typename Key>
    dsa::container::Vector<string_type> keysWithPrefix(const Key& prefix) const;

    template<typename Pattern>
    dsa::container::Vector<string_type> keysThatMatch(const Pattern& pattern) const;

    template<typename Key>
    string_type longestPrefixOf(const Key& input) const;

    void swap(TernarySearchTrie& other) noexcept;
};

template<typename T, typename CharT>
TernarySearchTrie<T, CharT>::TernarySearchTrie()
    : root_(), emptyValue_(), size_(0) {
}

template<typename T, typename CharT>
TernarySearchTrie<T, CharT>::TernarySearchTrie(
    const TernarySearchTrie& other
) : root_(clone(other.root_)), emptyValue_(), size_(other.size_) {
    if (other.emptyValue_)
        emptyValue_.reset(new mapped_type(*other.emptyValue_));
}

template<typename T, typename CharT>
TernarySearchTrie<T, CharT>::TernarySearchTrie(
    TernarySearchTrie&& other
) noexcept
    : root_(std::move(other.root_)),
      emptyValue_(std::move(other.emptyValue_)),
      size_(other.size_) {
    other.size_ = 0;
}

template<typename T, typename CharT>
TernarySearchTrie<T, CharT>&
TernarySearchTrie<T, CharT>::operator=(const TernarySearchTrie& other) {
    if (this != &other) {
        TernarySearchTrie copy(other);
        swap(copy);
    }
    return *this;
}

template<typename T, typename CharT>
TernarySearchTrie<T, CharT>&
TernarySearchTrie<T, CharT>::operator=(
    TernarySearchTrie&& other
) noexcept {
    if (this != &other) {
        root_ = std::move(other.root_);
        emptyValue_ = std::move(other.emptyValue_);
        size_ = other.size_;
        other.size_ = 0;
    }
    return *this;
}

template<typename T, typename CharT>
template<typename Key, typename Value>
bool TernarySearchTrie<T, CharT>::insert(const Key& key, Value&& value) {
    const size_type length = dsa::algorithm::sequenceSize(key);
    if (length == 0) {
        const bool inserted = !emptyValue_;
        emptyValue_.reset(new mapped_type(std::forward<Value>(value)));
        if (inserted)
            ++size_;
        return inserted;
    }

    bool inserted = false;
    insertNode(root_, key, std::forward<Value>(value), 0, inserted);
    if (inserted)
        ++size_;
    return inserted;
}

template<typename T, typename CharT>
template<typename Key>
typename TernarySearchTrie<T, CharT>::mapped_type*
TernarySearchTrie<T, CharT>::find(const Key& key) {
    if (dsa::algorithm::sequenceSize(key) == 0)
        return emptyValue_.get();
    Node* node = ReadAlgorithm::find(root_.get(), key);
    return node == nullptr ? nullptr : node->value.get();
}

template<typename T, typename CharT>
template<typename Key>
const typename TernarySearchTrie<T, CharT>::mapped_type*
TernarySearchTrie<T, CharT>::find(const Key& key) const {
    if (dsa::algorithm::sequenceSize(key) == 0)
        return emptyValue_.get();
    Node* node = ReadAlgorithm::find(root_.get(), key);
    return node == nullptr ? nullptr : node->value.get();
}

template<typename T, typename CharT>
template<typename Key>
bool TernarySearchTrie<T, CharT>::contains(const Key& key) const {
    return find(key) != nullptr;
}

template<typename T, typename CharT>
template<typename Key>
bool TernarySearchTrie<T, CharT>::erase(const Key& key) {
    if (dsa::algorithm::sequenceSize(key) == 0) {
        if (!emptyValue_)
            return false;
        emptyValue_.reset();
        --size_;
        return true;
    }

    bool removed = false;
    eraseNode(root_, key, 0, removed);
    if (removed)
        --size_;
    return removed;
}

template<typename T, typename CharT>
void TernarySearchTrie<T, CharT>::clear() noexcept {
    root_.reset();
    emptyValue_.reset();
    size_ = 0;
}

template<typename T, typename CharT>
bool TernarySearchTrie<T, CharT>::empty() const noexcept {
    return size_ == 0;
}

template<typename T, typename CharT>
typename TernarySearchTrie<T, CharT>::size_type
TernarySearchTrie<T, CharT>::size() const noexcept {
    return size_;
}

template<typename T, typename CharT>
template<typename Key>
dsa::container::Vector<typename TernarySearchTrie<T, CharT>::string_type>
TernarySearchTrie<T, CharT>::keysWithPrefix(const Key& prefix) const {
    dsa::container::Vector<string_type> output;
    const size_type length = dsa::algorithm::sequenceSize(prefix);
    if (length == 0) {
        if (emptyValue_)
            output.push_back(string_type());
        collect(root_.get(), string_type(), output);
        return output;
    }

    Node* node = ReadAlgorithm::find(root_.get(), prefix);
    if (node == nullptr)
        return output;
    const string_type prefixString = makeKey(prefix);
    if (node->value)
        output.push_back(prefixString);
    collect(node->middle.get(), prefixString, output);
    return output;
}

template<typename T, typename CharT>
template<typename Pattern>
dsa::container::Vector<typename TernarySearchTrie<T, CharT>::string_type>
TernarySearchTrie<T, CharT>::keysThatMatch(const Pattern& pattern) const {
    dsa::container::Vector<string_type> output;
    if (dsa::algorithm::sequenceSize(pattern) == 0) {
        if (emptyValue_)
            output.push_back(string_type());
        return output;
    }
    collectMatch(root_.get(), string_type(), pattern, 0, output);
    return output;
}

template<typename T, typename CharT>
template<typename Key>
typename TernarySearchTrie<T, CharT>::string_type
TernarySearchTrie<T, CharT>::longestPrefixOf(const Key& input) const {
    const size_type best = ReadAlgorithm::longestPrefixLength(root_.get(), input);
    return dsa::algorithm::substring<string_type>(input, 0, best);
}

template<typename T, typename CharT>
void TernarySearchTrie<T, CharT>::swap(
    TernarySearchTrie& other
) noexcept {
    root_.swap(other.root_);
    emptyValue_.swap(other.emptyValue_);
    std::swap(size_, other.size_);
}

template<typename T, typename CharT>
std::unique_ptr<typename TernarySearchTrie<T, CharT>::Node>
TernarySearchTrie<T, CharT>::clone(const std::unique_ptr<Node>& node) {
    if (!node)
        return std::unique_ptr<Node>();
    std::unique_ptr<Node> copy(new Node(node->symbol));
    if (node->value)
        copy->value.reset(new mapped_type(*node->value));
    copy->left = clone(node->left);
    copy->middle = clone(node->middle);
    copy->right = clone(node->right);
    return copy;
}

template<typename T, typename CharT>
template<typename Key, typename Value>
typename TernarySearchTrie<T, CharT>::Node*
TernarySearchTrie<T, CharT>::insertNode(
    std::unique_ptr<Node>& node,
    const Key& key,
    Value&& value,
    size_type depth,
    bool& inserted
) {
    const char_type current = static_cast<char_type>(
        dsa::algorithm::sequenceAt(key, depth)
    );
    if (!node)
        node.reset(new Node(current));

    if (current < node->symbol) {
        insertNode(node->left, key, std::forward<Value>(value), depth, inserted);
    } else if (node->symbol < current) {
        insertNode(node->right, key, std::forward<Value>(value), depth, inserted);
    } else if (depth + 1 < dsa::algorithm::sequenceSize(key)) {
        insertNode(
            node->middle,
            key,
            std::forward<Value>(value),
            depth + 1,
            inserted
        );
    } else {
        inserted = !node->value;
        node->value.reset(new mapped_type(std::forward<Value>(value)));
    }
    return node.get();
}

template<typename T, typename CharT>
template<typename Key>
bool TernarySearchTrie<T, CharT>::eraseNode(
    std::unique_ptr<Node>& node,
    const Key& key,
    size_type depth,
    bool& removed
) {
    if (!node)
        return true;

    const char_type current = static_cast<char_type>(
        dsa::algorithm::sequenceAt(key, depth)
    );
    if (current < node->symbol) {
        eraseNode(node->left, key, depth, removed);
    } else if (node->symbol < current) {
        eraseNode(node->right, key, depth, removed);
    } else if (depth + 1 < dsa::algorithm::sequenceSize(key)) {
        eraseNode(node->middle, key, depth + 1, removed);
    } else if (node->value) {
        node->value.reset();
        removed = true;
    }

    if (emptyNode(*node)) {
        node.reset();
        return true;
    }
    return false;
}

template<typename T, typename CharT>
bool TernarySearchTrie<T, CharT>::emptyNode(const Node& node) {
    return !node.value && !node.left && !node.middle && !node.right;
}

template<typename T, typename CharT>
void TernarySearchTrie<T, CharT>::collect(
    Node* node,
    const string_type& prefix,
    dsa::container::Vector<string_type>& output
) {
    if (node == nullptr)
        return;
    collect(node->left.get(), prefix, output);
    string_type connected(prefix);
    connected.push_back(node->symbol);
    if (node->value)
        output.push_back(connected);
    collect(node->middle.get(), connected, output);
    collect(node->right.get(), prefix, output);
}

template<typename T, typename CharT>
template<typename Pattern>
void TernarySearchTrie<T, CharT>::collectMatch(
    Node* node,
    const string_type& prefix,
    const Pattern& pattern,
    size_type depth,
    dsa::container::Vector<string_type>& output
) {
    if (node == nullptr)
        return;

    const char_type expected = static_cast<char_type>(
        dsa::algorithm::sequenceAt(pattern, depth)
    );
    if (expected == static_cast<char_type>('.') || expected < node->symbol)
        collectMatch(node->left.get(), prefix, pattern, depth, output);

    if (expected == static_cast<char_type>('.') || expected == node->symbol) {
        string_type connected(prefix);
        connected.push_back(node->symbol);
        if (depth + 1 == dsa::algorithm::sequenceSize(pattern)) {
            if (node->value)
                output.push_back(connected);
        } else {
            collectMatch(
                node->middle.get(),
                connected,
                pattern,
                depth + 1,
                output
            );
        }
    }

    if (expected == static_cast<char_type>('.') || node->symbol < expected)
        collectMatch(node->right.get(), prefix, pattern, depth, output);
}

template<typename T, typename CharT>
template<typename Key>
typename TernarySearchTrie<T, CharT>::string_type
TernarySearchTrie<T, CharT>::makeKey(const Key& key) {
    string_type result;
    dsa::algorithm::appendSequence(result, key);
    return result;
}

template<typename T, typename CharT>
void swap(
    TernarySearchTrie<T, CharT>& left,
    TernarySearchTrie<T, CharT>& right
) noexcept {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
