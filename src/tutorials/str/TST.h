#ifndef __DSA_TST
#define __DSA_TST

#include "dsa_string.h"
#include "StringST.h"
#include "Vector.h"
#include <dsa/core/string/TrieAlgorithm.h>

/// 教学 Ternary Search Trie 节点。
template<typename T>
struct TSTNode {
    char c;
    TSTNode* left;
    TSTNode* mid;
    TSTNode* right;
    T val;

    explicit TSTNode(char character)
        : c(character),
          left(nullptr),
          mid(nullptr),
          right(nullptr),
          val(T()) {
    }
};

/// 教学 TST；保留 T() 哨兵语义并复用共享查找流程。
template<typename T>
class TST : public StringST<T> {
private:
    typedef TSTNode<T> node_type;

    struct Access {
        static char symbol(node_type* node) {
            return node->c;
        }

        static node_type* left(node_type* node) {
            return node->left;
        }

        static node_type* middle(node_type* node) {
            return node->mid;
        }

        static node_type* right(node_type* node) {
            return node->right;
        }

        static bool terminal(node_type* node) {
            return !(node->val == T());
        }
    };

    typedef dsa::core::TernarySearchTrieAlgorithm<Access> ReadAlgorithm;

    node_type* root_;

    /// 递归插入键，并只在首次写入末端节点时报告 inserted。
    node_type* putNode(
        node_type* node,
        const String& key,
        T value,
        size_type depth,
        bool& inserted
    );

    /// 删除键并剪除无值叶节点。
    node_type* removeNode(
        node_type* node,
        const String& key,
        size_type depth,
        bool& removed
    );

    /// 按字典序收集当前子树中的键。
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

    /// 递归释放整棵 TST。
    void destroy(node_type* node) noexcept;

public:
    TST();
    ~TST();

    /// 空键和 T() 值按教学兼容规则忽略。
    void put(const String& key, T value) override;

    T get(String& key) override;
    T get(const char* key);

    void remove(const String& key) override;

    String longestPrefixOf(String input) override;
    Vector<String> keysWithPrefix(String prefix) override;
    Vector<String> keysThatMatch(String pattern) override;
};

template<typename T>
TST<T>::TST() : root_(nullptr) {
}

template<typename T>
TST<T>::~TST() {
    destroy(root_);
    root_ = nullptr;
    this->s = 0;
}

template<typename T>
void TST<T>::put(const String& key, T value) {
    if (key.empty() || value == T())
        return;

    bool inserted = false;
    root_ = putNode(root_, key, value, 0, inserted);
    if (inserted)
        ++this->s;
}

template<typename T>
typename TST<T>::node_type* TST<T>::putNode(
    node_type* node,
    const String& key,
    T value,
    size_type depth,
    bool& inserted
) {
    const char currentSymbol = key[depth];
    if (node == nullptr)
        node = new node_type(currentSymbol);

    if (currentSymbol < node->c) {
        node->left = putNode(node->left, key, value, depth, inserted);
    } else if (node->c < currentSymbol) {
        node->right = putNode(node->right, key, value, depth, inserted);
    } else if (depth + 1 < key.size()) {
        node->mid = putNode(node->mid, key, value, depth + 1, inserted);
    } else {
        inserted = node->val == T();
        node->val = value;
    }
    return node;
}

template<typename T>
T TST<T>::get(String& key) {
    node_type* node = ReadAlgorithm::find(root_, key);
    return node == nullptr ? T() : node->val;
}

template<typename T>
T TST<T>::get(const char* key) {
    String converted(key);
    return get(converted);
}

template<typename T>
void TST<T>::remove(const String& key) {
    if (key.empty())
        return;

    bool removed = false;
    root_ = removeNode(root_, key, 0, removed);
    if (removed)
        --this->s;
}

template<typename T>
typename TST<T>::node_type* TST<T>::removeNode(
    node_type* node,
    const String& key,
    size_type depth,
    bool& removed
) {
    if (node == nullptr)
        return nullptr;

    const char currentSymbol = key[depth];
    if (currentSymbol < node->c) {
        node->left = removeNode(node->left, key, depth, removed);
    } else if (node->c < currentSymbol) {
        node->right = removeNode(node->right, key, depth, removed);
    } else if (depth + 1 < key.size()) {
        node->mid = removeNode(
            node->mid,
            key,
            depth + 1,
            removed
        );
    } else if (!(node->val == T())) {
        node->val = T();
        removed = true;
    }

    if (!(node->val == T()) || node->left != nullptr ||
        node->mid != nullptr || node->right != nullptr) {
        return node;
    }
    delete node;
    return nullptr;
}

template<typename T>
String TST<T>::longestPrefixOf(String input) {
    return dsa::algorithm::substring<String>(
        input,
        0,
        ReadAlgorithm::longestPrefixLength(root_, input)
    );
}

template<typename T>
Vector<String> TST<T>::keysWithPrefix(String prefix) {
    Vector<String> output;
    if (prefix.empty()) {
        collect(root_, prefix, output);
        return output;
    }

    node_type* node = ReadAlgorithm::find(root_, prefix);
    if (node == nullptr)
        return output;
    if (!(node->val == T()))
        output.insert(prefix);
    collect(node->mid, prefix, output);
    return output;
}

template<typename T>
void TST<T>::collect(
    node_type* node,
    const String& prefix,
    Vector<String>& output
) const {
    if (node == nullptr)
        return;

    collect(node->left, prefix, output);
    const String connected = prefix + node->c;
    if (!(node->val == T()))
        output.insert(connected);
    collect(node->mid, connected, output);
    collect(node->right, prefix, output);
}

template<typename T>
Vector<String> TST<T>::keysThatMatch(String pattern) {
    Vector<String> output;
    if (!pattern.empty())
        collectMatch(root_, String(), pattern, 0, output);
    return output;
}

template<typename T>
void TST<T>::collectMatch(
    node_type* node,
    const String& prefix,
    const String& pattern,
    size_type depth,
    Vector<String>& output
) const {
    if (node == nullptr)
        return;

    const char expected = pattern[depth];
    if (expected == '.' || expected < node->c)
        collectMatch(node->left, prefix, pattern, depth, output);

    if (expected == '.' || expected == node->c) {
        const String connected = prefix + node->c;
        if (depth + 1 == pattern.size()) {
            if (!(node->val == T()))
                output.insert(connected);
        } else {
            collectMatch(
                node->mid,
                connected,
                pattern,
                depth + 1,
                output
            );
        }
    }

    if (expected == '.' || node->c < expected)
        collectMatch(node->right, prefix, pattern, depth, output);
}

template<typename T>
void TST<T>::destroy(node_type* node) noexcept {
    if (node == nullptr)
        return;
    destroy(node->left);
    destroy(node->mid);
    destroy(node->right);
    delete node;
}

#endif
