#ifndef __DSA_TRIE
#define __DSA_TRIE

#include <cstddef>

#include "dsa_string.h"
#include "StringST.h"
#include "Vector.h"
#include <dsa/core/string/TrieAlgorithm.h>

/// 教学 R-way Trie 节点；next 保留 Vector 表示以便观察固定字母表结构。
template<typename T>
struct TrieNode {
    T val;
    Vector<TrieNode<T>*> next;

    TrieNode() : val(T()), next(R, R, nullptr) {}
};

/// 保留旧节点类型名，避免下游教学代码因重命名失效。
template<typename T>
using Node = TrieNode<T>;

/// 教学 R-way Trie；保留 T() 哨兵语义，同时复用通用查找流程。
template<typename T>
class Trie : public StringST<T> {
private:
    typedef TrieNode<T> node_type;

    struct Access {
        static std::size_t index(char symbol) {
            return dsa::algorithm::alphabetIndex(symbol);
        }

        static bool validIndex(std::size_t indexValue) {
            return indexValue < R;
        }

        static node_type* child(node_type* node, std::size_t indexValue) {
            return node->next[static_cast<Rank>(indexValue)];
        }

        static bool terminal(node_type* node) {
            return !(node->val == T());
        }
    };

    typedef dsa::core::TrieAlgorithm<Access> ReadAlgorithm;

    node_type* root_;

    /// 递归创建缺失节点，并只在首次写入键时报告 inserted。
    node_type* putNode(
        node_type* node,
        const String& key,
        T value,
        size_type depth,
        bool& inserted
    );

    /// 删除键并剪枝；只有真实存在的键才报告 removed。
    node_type* removeNode(
        node_type* node,
        const String& key,
        size_type depth,
        bool& removed
    );

    /// 收集指定节点下的全部键。
    void collect(
        node_type* node,
        const String& prefix,
        Vector<String>& output
    ) const;

    /// 按点号通配模式收集等长键。
    void collectMatch(
        node_type* node,
        const String& prefix,
        const String& pattern,
        size_type depth,
        Vector<String>& output
    ) const;

    /// 判断当前节点是否仍含子节点。
    bool hasChildren(node_type* node) const;

    /// 递归释放整棵 Trie，节点值由正常析构负责。
    void destroy(node_type* node) noexcept;

public:
    Trie();
    ~Trie();

    /// 教学版保持 T() 为无值哨兵；写入 T() 会被忽略。
    void put(const String& key, T value) override;

    T get(String& key) override;
    T get(const char* key);

    void remove(const String& key) override;

    Vector<String> keysWithPrefix(String prefix) override;
    Vector<String> keysThatMatch(String pattern) override;
    String longestPrefixOf(String input) override;
};

template<typename T>
Trie<T>::Trie() : root_(nullptr) {
}

template<typename T>
Trie<T>::~Trie() {
    destroy(root_);
    root_ = nullptr;
    this->s = 0;
}

template<typename T>
void Trie<T>::put(const String& key, T value) {
    if (value == T())
        return;

    bool inserted = false;
    root_ = putNode(root_, key, value, 0, inserted);
    if (inserted)
        ++this->s;
}

template<typename T>
typename Trie<T>::node_type* Trie<T>::putNode(
    node_type* node,
    const String& key,
    T value,
    size_type depth,
    bool& inserted
) {
    if (node == nullptr)
        node = new node_type();

    if (depth == key.size()) {
        inserted = node->val == T();
        node->val = value;
        return node;
    }

    const std::size_t indexValue = Access::index(key[depth]);
    node->next[static_cast<Rank>(indexValue)] = putNode(
        node->next[static_cast<Rank>(indexValue)],
        key,
        value,
        depth + 1,
        inserted
    );
    return node;
}

template<typename T>
T Trie<T>::get(String& key) {
    node_type* node = ReadAlgorithm::find(root_, key);
    return node == nullptr ? T() : node->val;
}

template<typename T>
T Trie<T>::get(const char* key) {
    String converted(key);
    return get(converted);
}

template<typename T>
void Trie<T>::remove(const String& key) {
    bool removed = false;
    root_ = removeNode(root_, key, 0, removed);
    if (removed)
        --this->s;
}

template<typename T>
typename Trie<T>::node_type* Trie<T>::removeNode(
    node_type* node,
    const String& key,
    size_type depth,
    bool& removed
) {
    if (node == nullptr)
        return nullptr;

    if (depth == key.size()) {
        if (!(node->val == T())) {
            node->val = T();
            removed = true;
        }
    } else {
        const std::size_t indexValue = Access::index(key[depth]);
        node->next[static_cast<Rank>(indexValue)] = removeNode(
            node->next[static_cast<Rank>(indexValue)],
            key,
            depth + 1,
            removed
        );
    }

    if (!(node->val == T()) || hasChildren(node))
        return node;
    delete node;
    return nullptr;
}

template<typename T>
Vector<String> Trie<T>::keysWithPrefix(String prefix) {
    Vector<String> output;
    node_type* node = ReadAlgorithm::find(root_, prefix);
    collect(node, prefix, output);
    return output;
}

template<typename T>
void Trie<T>::collect(
    node_type* node,
    const String& prefix,
    Vector<String>& output
) const {
    if (node == nullptr)
        return;
    if (!(node->val == T()))
        output.insert(prefix);

    for (size_type indexValue = 0; indexValue < R; ++indexValue) {
        node_type* child = node->next[static_cast<Rank>(indexValue)];
        if (child != nullptr) {
            collect(
                child,
                prefix + static_cast<char>(indexValue),
                output
            );
        }
    }
}

template<typename T>
Vector<String> Trie<T>::keysThatMatch(String pattern) {
    Vector<String> output;
    collectMatch(root_, String(), pattern, 0, output);
    return output;
}

template<typename T>
void Trie<T>::collectMatch(
    node_type* node,
    const String& prefix,
    const String& pattern,
    size_type depth,
    Vector<String>& output
) const {
    if (node == nullptr)
        return;
    if (depth == pattern.size()) {
        if (!(node->val == T()))
            output.insert(prefix);
        return;
    }

    const char expected = pattern[depth];
    if (expected == '.') {
        for (size_type indexValue = 0; indexValue < R; ++indexValue) {
            node_type* child = node->next[static_cast<Rank>(indexValue)];
            if (child != nullptr) {
                collectMatch(
                    child,
                    prefix + static_cast<char>(indexValue),
                    pattern,
                    depth + 1,
                    output
                );
            }
        }
        return;
    }

    const std::size_t indexValue = Access::index(expected);
    collectMatch(
        node->next[static_cast<Rank>(indexValue)],
        prefix + expected,
        pattern,
        depth + 1,
        output
    );
}

template<typename T>
String Trie<T>::longestPrefixOf(String input) {
    return dsa::algorithm::substring<String>(
        input,
        0,
        ReadAlgorithm::longestPrefixLength(root_, input)
    );
}

template<typename T>
bool Trie<T>::hasChildren(node_type* node) const {
    for (size_type indexValue = 0; indexValue < R; ++indexValue) {
        if (node->next[static_cast<Rank>(indexValue)] != nullptr)
            return true;
    }
    return false;
}

template<typename T>
void Trie<T>::destroy(node_type* node) noexcept {
    if (node == nullptr)
        return;
    for (size_type indexValue = 0; indexValue < R; ++indexValue)
        destroy(node->next[static_cast<Rank>(indexValue)]);
    delete node;
}

#endif
