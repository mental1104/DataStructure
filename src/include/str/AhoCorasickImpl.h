#ifndef __DSA_AHO_CORASICK_IMPL
#define __DSA_AHO_CORASICK_IMPL

#include <array>
#include <cstddef>
#include <queue>
#include <vector>

namespace ac_algorithm {

struct AhoCorasickIndexMatch {
    std::size_t pattern_index;
    std::size_t start;
    std::size_t end;

    AhoCorasickIndexMatch()
        : pattern_index(0), start(0), end(0) {}

    AhoCorasickIndexMatch(std::size_t index,
                          std::size_t match_start,
                          std::size_t match_end)
        : pattern_index(index), start(match_start), end(match_end) {}
};

/*
 * 教学算法层：
 * - 只处理字节序列、模式下标和 failure 转移；
 * - 不关心业务 ID、分类、生命周期或返回对象包装；
 * - 使用 256 路字节字母表，因此可匹配 UTF-8 文本，但位置是字节偏移。
 */
class AhoCorasickImpl {
private:
    static const std::size_t AlphabetSize = 256;

    struct Node {
        std::array<std::size_t, AlphabetSize> next;
        std::size_t fail;
        std::vector<std::size_t> outputs;

        Node() : fail(0) {
            next.fill(invalidState());
        }

        static std::size_t invalidState() {
            return static_cast<std::size_t>(-1);
        }
    };

    std::vector<Node> nodes_;
    std::vector<std::size_t> pattern_lengths_;
    bool built_;

    static std::size_t invalidState() {
        return static_cast<std::size_t>(-1);
    }

    static std::size_t symbol(char value) {
        return static_cast<unsigned char>(value);
    }

    static void appendOutputs(Node& target, const Node& fallback) {
        target.outputs.insert(target.outputs.end(),
                              fallback.outputs.begin(),
                              fallback.outputs.end());
    }

public:
    AhoCorasickImpl() : built_(false) {
        nodes_.push_back(Node());
    }

    void clear() {
        nodes_.clear();
        nodes_.push_back(Node());
        pattern_lengths_.clear();
        built_ = false;
    }

    std::size_t addPattern(const char* pattern, std::size_t length) {
        if (built_ || pattern == nullptr || length == 0) {
            return invalidState();
        }

        std::size_t state = 0;
        for (std::size_t i = 0; i < length; ++i) {
            const std::size_t c = symbol(pattern[i]);
            std::size_t next_state = nodes_[state].next[c];
            if (next_state == invalidState()) {
                next_state = nodes_.size();
                nodes_[state].next[c] = next_state;
                nodes_.push_back(Node());
            }
            state = next_state;
        }

        const std::size_t pattern_index = pattern_lengths_.size();
        pattern_lengths_.push_back(length);
        nodes_[state].outputs.push_back(pattern_index);
        return pattern_index;
    }

    void build() {
        if (built_) {
            return;
        }

        std::queue<std::size_t> pending;
        for (std::size_t c = 0; c < AlphabetSize; ++c) {
            const std::size_t child = nodes_[0].next[c];
            if (child == invalidState()) {
                nodes_[0].next[c] = 0;
            } else {
                nodes_[child].fail = 0;
                pending.push(child);
            }
        }

        while (!pending.empty()) {
            const std::size_t state = pending.front();
            pending.pop();

            for (std::size_t c = 0; c < AlphabetSize; ++c) {
                const std::size_t child = nodes_[state].next[c];
                if (child == invalidState()) {
                    nodes_[state].next[c] = nodes_[nodes_[state].fail].next[c];
                    continue;
                }

                const std::size_t fallback = nodes_[nodes_[state].fail].next[c];
                nodes_[child].fail = fallback;
                appendOutputs(nodes_[child], nodes_[fallback]);
                pending.push(child);
            }
        }

        built_ = true;
    }

    bool built() const {
        return built_;
    }

    std::size_t patternCount() const {
        return pattern_lengths_.size();
    }

    bool contains(const char* text, std::size_t length) const {
        AhoCorasickIndexMatch ignored;
        return findFirst(text, length, ignored);
    }

    bool findFirst(const char* text,
                   std::size_t length,
                   AhoCorasickIndexMatch& result) const {
        if (!built_ || text == nullptr) {
            return false;
        }

        std::size_t state = 0;
        for (std::size_t i = 0; i < length; ++i) {
            state = nodes_[state].next[symbol(text[i])];
            if (!nodes_[state].outputs.empty()) {
                const std::size_t pattern_index = nodes_[state].outputs.front();
                const std::size_t end = i + 1;
                result = AhoCorasickIndexMatch(
                    pattern_index,
                    end - pattern_lengths_[pattern_index],
                    end);
                return true;
            }
        }
        return false;
    }

    std::vector<AhoCorasickIndexMatch> findAll(const char* text,
                                                std::size_t length) const {
        std::vector<AhoCorasickIndexMatch> matches;
        if (!built_ || text == nullptr) {
            return matches;
        }

        std::size_t state = 0;
        for (std::size_t i = 0; i < length; ++i) {
            state = nodes_[state].next[symbol(text[i])];
            const std::vector<std::size_t>& outputs = nodes_[state].outputs;
            for (std::size_t j = 0; j < outputs.size(); ++j) {
                const std::size_t pattern_index = outputs[j];
                const std::size_t end = i + 1;
                matches.push_back(AhoCorasickIndexMatch(
                    pattern_index,
                    end - pattern_lengths_[pattern_index],
                    end));
            }
        }
        return matches;
    }
};

} // namespace ac_algorithm

#endif
