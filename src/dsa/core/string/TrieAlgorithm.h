#ifndef DSA_CORE_STRING_TRIE_ALGORITHM_H
#define DSA_CORE_STRING_TRIE_ALGORITHM_H

#include <cstddef>

#include "../../algorithm/String.h"

namespace dsa {
namespace core {

/// 编排 R-way Trie 的只读查找流程，节点表示由 Access policy 提供。
template<typename Access>
class TrieAlgorithm {
public:
    template<typename Node, typename Key>
    static Node* find(Node* root, const Key& key) {
        Node* current = root;
        const std::size_t length = dsa::algorithm::sequenceSize(key);
        for (std::size_t depth = 0; current != nullptr && depth < length; ++depth) {
            const std::size_t index = Access::index(
                dsa::algorithm::sequenceAt(key, depth)
            );
            if (!Access::validIndex(index))
                return nullptr;
            current = Access::child(current, index);
        }
        return current;
    }

    template<typename Node, typename Key>
    static std::size_t longestPrefixLength(Node* root, const Key& key) {
        Node* current = root;
        std::size_t best = 0;
        const std::size_t length = dsa::algorithm::sequenceSize(key);
        for (std::size_t depth = 0; current != nullptr && depth < length; ++depth) {
            const std::size_t index = Access::index(
                dsa::algorithm::sequenceAt(key, depth)
            );
            if (!Access::validIndex(index))
                break;
            current = Access::child(current, index);
            if (current != nullptr && Access::terminal(current))
                best = depth + 1;
        }
        return best;
    }
};

/// 编排 Ternary Search Trie 的只读查找流程，节点表示由 Access policy 提供。
template<typename Access>
class TernarySearchTrieAlgorithm {
public:
    template<typename Node, typename Key>
    static Node* find(Node* root, const Key& key) {
        const std::size_t length = dsa::algorithm::sequenceSize(key);
        if (length == 0)
            return nullptr;

        Node* current = root;
        std::size_t depth = 0;
        while (current != nullptr) {
            const auto symbol = dsa::algorithm::sequenceAt(key, depth);
            if (symbol < Access::symbol(current)) {
                current = Access::left(current);
            } else if (Access::symbol(current) < symbol) {
                current = Access::right(current);
            } else if (depth + 1 < length) {
                ++depth;
                current = Access::middle(current);
            } else {
                return current;
            }
        }
        return nullptr;
    }

    template<typename Node, typename Key>
    static std::size_t longestPrefixLength(Node* root, const Key& key) {
        const std::size_t length = dsa::algorithm::sequenceSize(key);
        if (length == 0)
            return 0;

        Node* current = root;
        std::size_t depth = 0;
        std::size_t best = 0;
        while (current != nullptr && depth < length) {
            const auto symbol = dsa::algorithm::sequenceAt(key, depth);
            if (symbol < Access::symbol(current)) {
                current = Access::left(current);
            } else if (Access::symbol(current) < symbol) {
                current = Access::right(current);
            } else {
                ++depth;
                if (Access::terminal(current))
                    best = depth;
                current = Access::middle(current);
            }
        }
        return best;
    }
};

} // namespace core
} // namespace dsa

#endif
