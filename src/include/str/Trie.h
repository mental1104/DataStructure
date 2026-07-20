#pragma once

#include "StringST.h"
#include "Vector.h"
#include "dsa_string.h"
#include "string_access.h"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace dsa {
namespace str {

template <typename T>
class Trie : public StringST<T> {
private:
    static constexpr std::size_t kRadix = 256;

    struct Node {
        std::optional<T> value;
        std::array<std::unique_ptr<Node>, kRadix> children{};
    };

public:
    Trie() = default;
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;
    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;
    ~Trie() override = default;

    template <typename Key>
    void put(const Key& key, T value) {
        put_view(as_string_view(key), std::move(value));
    }

    template <typename Key>
    const T* find(const Key& key) const noexcept {
        const Node* node = find_node(as_string_view(key));
        return node != nullptr && node->value ? std::addressof(*node->value) : nullptr;
    }

    template <typename Key>
    T* find(const Key& key) noexcept {
        Node* node = find_node(as_string_view(key));
        return node != nullptr && node->value ? std::addressof(*node->value) : nullptr;
    }

    template <typename Key>
    T get(const Key& key) const {
        const T* value = find(key);
        return value == nullptr ? T{} : *value;
    }

    template <typename Key>
    bool contains(const Key& key) const noexcept {
        return find(key) != nullptr;
    }

    template <typename Key>
    bool remove(const Key& key) {
        const bool removed = remove_node(root_, as_string_view(key), 0);
        if (removed) {
            --this->size_;
        }
        return removed;
    }

    Vector<String> keys() const {
        return keysWithPrefix(std::string_view{});
    }

    template <typename Prefix>
    Vector<String> keysWithPrefix(const Prefix& prefix) const {
        const std::string_view view = as_string_view(prefix);
        Vector<String> result;
        const Node* node = find_node(view);
        if (node == nullptr) {
            return result;
        }

        String current(view);
        collect(node, current, result);
        return result;
    }

    template <typename Pattern>
    Vector<String> keysThatMatch(const Pattern& pattern) const {
        Vector<String> result;
        String current;
        collect_match(root_.get(), as_string_view(pattern), 0, current, result);
        return result;
    }

    template <typename Input>
    String longestPrefixOf(const Input& input) const {
        const std::string_view view = as_string_view(input);
        const Node* node = root_.get();
        std::size_t longest = node != nullptr && node->value ? 0 : std::string_view::npos;

        std::size_t index = 0;
        while (node != nullptr && index < view.size()) {
            node = node->children[to_index(view[index])].get();
            if (node == nullptr) {
                break;
            }
            ++index;
            if (node->value) {
                longest = index;
            }
        }

        return longest == std::string_view::npos
            ? String{}
            : String(view.substr(0, longest));
    }

private:
    static std::size_t to_index(char value) noexcept {
        return static_cast<unsigned char>(value);
    }

    void put_view(std::string_view key, T value) {
        if (!root_) {
            root_ = std::make_unique<Node>();
        }

        Node* node = root_.get();
        for (char value_char : key) {
            std::unique_ptr<Node>& child = node->children[to_index(value_char)];
            if (!child) {
                child = std::make_unique<Node>();
            }
            node = child.get();
        }

        if (!node->value) {
            ++this->size_;
        }
        node->value = std::move(value);
    }

    Node* find_node(std::string_view key) noexcept {
        return const_cast<Node*>(static_cast<const Trie*>(this)->find_node(key));
    }

    const Node* find_node(std::string_view key) const noexcept {
        const Node* node = root_.get();
        for (char value_char : key) {
            if (node == nullptr) {
                return nullptr;
            }
            node = node->children[to_index(value_char)].get();
        }
        return node;
    }

    static bool has_children(const Node& node) noexcept {
        for (const std::unique_ptr<Node>& child : node.children) {
            if (child) {
                return true;
            }
        }
        return false;
    }

    static bool remove_node(std::unique_ptr<Node>& node,
                            std::string_view key,
                            std::size_t depth) {
        if (!node) {
            return false;
        }

        bool removed = false;
        if (depth == key.size()) {
            if (node->value) {
                node->value.reset();
                removed = true;
            }
        } else {
            removed = remove_node(node->children[to_index(key[depth])], key, depth + 1);
        }

        if (!node->value && !has_children(*node)) {
            node.reset();
        }
        return removed;
    }

    static void collect(const Node* node, String& prefix, Vector<String>& result) {
        if (node == nullptr) {
            return;
        }
        if (node->value) {
            result.insert(prefix);
        }

        for (std::size_t index = 0; index < kRadix; ++index) {
            if (!node->children[index]) {
                continue;
            }
            prefix.push_back(static_cast<char>(index));
            collect(node->children[index].get(), prefix, result);
            prefix.pop_back();
        }
    }

    static void collect_match(const Node* node,
                              std::string_view pattern,
                              std::size_t depth,
                              String& prefix,
                              Vector<String>& result) {
        if (node == nullptr) {
            return;
        }
        if (depth == pattern.size()) {
            if (node->value) {
                result.insert(prefix);
            }
            return;
        }

        const char expected = pattern[depth];
        if (expected == '.') {
            for (std::size_t index = 0; index < kRadix; ++index) {
                if (!node->children[index]) {
                    continue;
                }
                prefix.push_back(static_cast<char>(index));
                collect_match(node->children[index].get(), pattern, depth + 1, prefix, result);
                prefix.pop_back();
            }
            return;
        }

        const std::unique_ptr<Node>& child = node->children[to_index(expected)];
        if (child) {
            prefix.push_back(expected);
            collect_match(child.get(), pattern, depth + 1, prefix, result);
            prefix.pop_back();
        }
    }

    std::unique_ptr<Node> root_;
};

}  // namespace str
}  // namespace dsa

template <typename T>
using Trie = dsa::str::Trie<T>;
