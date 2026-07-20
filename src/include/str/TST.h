#pragma once

#include "StringST.h"
#include "Vector.h"
#include "dsa_string.h"
#include "string_access.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace dsa {
namespace str {

template <typename T>
class TernarySearchTrie : public StringST<T> {
private:
    struct Node {
        explicit Node(unsigned char character) : character(character) {}

        unsigned char character;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> middle;
        std::unique_ptr<Node> right;
        std::optional<T> value;
    };

public:
    TernarySearchTrie() = default;
    TernarySearchTrie(const TernarySearchTrie&) = delete;
    TernarySearchTrie& operator=(const TernarySearchTrie&) = delete;
    TernarySearchTrie(TernarySearchTrie&&) noexcept = default;
    TernarySearchTrie& operator=(TernarySearchTrie&&) noexcept = default;
    ~TernarySearchTrie() override = default;

    template <typename Key>
    void put(const Key& key, T value) {
        const std::string_view view = as_string_view(key);
        if (view.empty()) {
            if (!empty_value_) {
                ++this->size_;
            }
            empty_value_ = std::move(value);
            return;
        }

        bool inserted = false;
        root_ = put_node(std::move(root_), view, 0, std::move(value), inserted);
        if (inserted) {
            ++this->size_;
        }
    }

    template <typename Key>
    const T* find(const Key& key) const noexcept {
        const std::string_view view = as_string_view(key);
        if (view.empty()) {
            return empty_value_ ? std::addressof(*empty_value_) : nullptr;
        }
        const Node* node = find_node(view);
        return node != nullptr && node->value ? std::addressof(*node->value) : nullptr;
    }

    template <typename Key>
    T* find(const Key& key) noexcept {
        return const_cast<T*>(static_cast<const TernarySearchTrie*>(this)->find(key));
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
        const std::string_view view = as_string_view(key);
        if (view.empty()) {
            if (!empty_value_) {
                return false;
            }
            empty_value_.reset();
            --this->size_;
            return true;
        }

        bool removed = false;
        root_ = remove_node(std::move(root_), view, 0, removed);
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

        if (view.empty()) {
            if (empty_value_) {
                result.insert(String{});
            }
            String current;
            collect(root_.get(), current, result);
            return result;
        }

        const Node* node = find_node(view);
        if (node == nullptr) {
            return result;
        }

        String current(view);
        if (node->value) {
            result.insert(current);
        }
        collect(node->middle.get(), current, result);
        return result;
    }

    template <typename Pattern>
    Vector<String> keysThatMatch(const Pattern& pattern) const {
        const std::string_view view = as_string_view(pattern);
        Vector<String> result;
        if (view.empty()) {
            if (empty_value_) {
                result.insert(String{});
            }
            return result;
        }

        String current;
        collect_match(root_.get(), view, 0, current, result);
        return result;
    }

    template <typename Input>
    String longestPrefixOf(const Input& input) const {
        const std::string_view view = as_string_view(input);
        if (view.empty()) {
            return String{};
        }

        const Node* node = root_.get();
        std::size_t index = 0;
        std::size_t longest = empty_value_ ? 0 : std::string_view::npos;

        while (node != nullptr && index < view.size()) {
            const unsigned char character = to_character(view[index]);
            if (character < node->character) {
                node = node->left.get();
            } else if (character > node->character) {
                node = node->right.get();
            } else {
                ++index;
                if (node->value) {
                    longest = index;
                }
                node = node->middle.get();
            }
        }

        return longest == std::string_view::npos
            ? String{}
            : String(view.substr(0, longest));
    }

private:
    static unsigned char to_character(char value) noexcept {
        return static_cast<unsigned char>(value);
    }

    static std::unique_ptr<Node> put_node(std::unique_ptr<Node> node,
                                          std::string_view key,
                                          std::size_t depth,
                                          T value,
                                          bool& inserted) {
        const unsigned char character = to_character(key[depth]);
        if (!node) {
            node = std::make_unique<Node>(character);
        }

        if (character < node->character) {
            node->left = put_node(std::move(node->left), key, depth, std::move(value), inserted);
        } else if (character > node->character) {
            node->right = put_node(std::move(node->right), key, depth, std::move(value), inserted);
        } else if (depth + 1 < key.size()) {
            node->middle = put_node(std::move(node->middle), key, depth + 1, std::move(value), inserted);
        } else {
            inserted = !node->value;
            node->value = std::move(value);
        }
        return node;
    }

    const Node* find_node(std::string_view key) const noexcept {
        const Node* node = root_.get();
        std::size_t depth = 0;
        while (node != nullptr) {
            const unsigned char character = to_character(key[depth]);
            if (character < node->character) {
                node = node->left.get();
            } else if (character > node->character) {
                node = node->right.get();
            } else if (depth + 1 < key.size()) {
                ++depth;
                node = node->middle.get();
            } else {
                return node;
            }
        }
        return nullptr;
    }

    static std::unique_ptr<Node> remove_node(std::unique_ptr<Node> node,
                                             std::string_view key,
                                             std::size_t depth,
                                             bool& removed) {
        if (!node) {
            return nullptr;
        }

        const unsigned char character = to_character(key[depth]);
        if (character < node->character) {
            node->left = remove_node(std::move(node->left), key, depth, removed);
        } else if (character > node->character) {
            node->right = remove_node(std::move(node->right), key, depth, removed);
        } else if (depth + 1 < key.size()) {
            node->middle = remove_node(std::move(node->middle), key, depth + 1, removed);
        } else if (node->value) {
            node->value.reset();
            removed = true;
        }

        if (!node->value && !node->left && !node->middle && !node->right) {
            return nullptr;
        }
        return node;
    }

    static void collect(const Node* node, String& prefix, Vector<String>& result) {
        if (node == nullptr) {
            return;
        }

        collect(node->left.get(), prefix, result);

        prefix.push_back(static_cast<char>(node->character));
        if (node->value) {
            result.insert(prefix);
        }
        collect(node->middle.get(), prefix, result);
        prefix.pop_back();

        collect(node->right.get(), prefix, result);
    }

    static void collect_match(const Node* node,
                              std::string_view pattern,
                              std::size_t depth,
                              String& prefix,
                              Vector<String>& result) {
        if (node == nullptr) {
            return;
        }

        const char expected = pattern[depth];
        const unsigned char expected_character = to_character(expected);
        if (expected == '.' || expected_character < node->character) {
            collect_match(node->left.get(), pattern, depth, prefix, result);
        }

        if (expected == '.' || expected_character == node->character) {
            prefix.push_back(static_cast<char>(node->character));
            if (depth + 1 == pattern.size()) {
                if (node->value) {
                    result.insert(prefix);
                }
            } else {
                collect_match(node->middle.get(), pattern, depth + 1, prefix, result);
            }
            prefix.pop_back();
        }

        if (expected == '.' || expected_character > node->character) {
            collect_match(node->right.get(), pattern, depth, prefix, result);
        }
    }

    std::unique_ptr<Node> root_;
    std::optional<T> empty_value_;
};

}  // namespace str
}  // namespace dsa

template <typename T>
using TST = dsa::str::TernarySearchTrie<T>;

template <typename T>
using TernarySearchTrie = dsa::str::TernarySearchTrie<T>;
