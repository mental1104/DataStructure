#ifndef __DSA_AHO_CORASICK
#define __DSA_AHO_CORASICK

#include <cstddef>
#include <cstring>
#include <vector>

#include "AhoCorasickImpl.h"
#include "Vector.h"
#include "dsa_string.h"

/*
 * 工程门面层：
 * - 管理业务 pattern ID、构建状态和结果对象；
 * - 将仓库 String / Vector 接口适配到独立算法核心；
 * - Build 完成后查询只读，可被多个调用方并发复用。
 */
class AhoCorasick {
public:
    struct Pattern {
        int id;
        String text;

        Pattern() : id(-1), text() {}
        Pattern(int pattern_id, const String& pattern_text)
            : id(pattern_id), text(pattern_text) {}
    };

    struct Match {
        int pattern_id;
        Rank pattern_index;
        size_type start;
        size_type end;

        Match()
            : pattern_id(-1), pattern_index(-1), start(0), end(0) {}

        Match(int id, Rank index, size_type match_start, size_type match_end)
            : pattern_id(id),
              pattern_index(index),
              start(match_start),
              end(match_end) {}
    };

private:
    Vector<Pattern> patterns_;
    ac_algorithm::AhoCorasickImpl algorithm_;
    bool built_;
    int next_pattern_id_;

    bool idExists(int pattern_id) const {
        for (Rank i = 0; i < patterns_.size(); ++i) {
            if (patterns_[i].id == pattern_id) {
                return true;
            }
        }
        return false;
    }

    Match adapt(const ac_algorithm::AhoCorasickIndexMatch& match) const {
        const Rank index = static_cast<Rank>(match.pattern_index);
        return Match(patterns_[index].id,
                     index,
                     static_cast<size_type>(match.start),
                     static_cast<size_type>(match.end));
    }

public:
    AhoCorasick() : built_(false), next_pattern_id_(0) {}

    int add(const String& pattern) {
        if (pattern.empty()) {
            return -1;
        }

        while (idExists(next_pattern_id_)) {
            ++next_pattern_id_;
        }

        const int pattern_id = next_pattern_id_++;
        patterns_.insert(Pattern(pattern_id, pattern));
        built_ = false;
        return pattern_id;
    }

    int add(const char* pattern) {
        if (pattern == nullptr) {
            return -1;
        }
        return add(String(pattern));
    }

    bool add(const String& pattern, int pattern_id) {
        if (pattern.empty() || idExists(pattern_id)) {
            return false;
        }

        patterns_.insert(Pattern(pattern_id, pattern));
        if (pattern_id >= next_pattern_id_) {
            next_pattern_id_ = pattern_id + 1;
        }
        built_ = false;
        return true;
    }

    bool add(const char* pattern, int pattern_id) {
        if (pattern == nullptr) {
            return false;
        }
        return add(String(pattern), pattern_id);
    }

    void build() {
        algorithm_.clear();
        for (Rank i = 0; i < patterns_.size(); ++i) {
            const Pattern& pattern = patterns_[i];
            algorithm_.addPattern(pattern.text.c_str(), pattern.text.size());
        }
        algorithm_.build();
        built_ = true;
    }

    void clear() {
        while (!patterns_.empty()) {
            patterns_.remove(patterns_.size() - 1);
        }
        algorithm_.clear();
        built_ = false;
        next_pattern_id_ = 0;
    }

    bool isBuilt() const {
        return built_;
    }

    Rank patternCount() const {
        return patterns_.size();
    }

    const Pattern* pattern(Rank index) const {
        if (index < 0 || index >= patterns_.size()) {
            return nullptr;
        }
        return &patterns_[index];
    }

    const Pattern* patternById(int pattern_id) const {
        for (Rank i = 0; i < patterns_.size(); ++i) {
            if (patterns_[i].id == pattern_id) {
                return &patterns_[i];
            }
        }
        return nullptr;
    }

    bool contains(const String& text) const {
        return built_ && algorithm_.contains(text.c_str(), text.size());
    }

    bool contains(const char* text) const {
        return text != nullptr && contains(String(text));
    }

    bool findFirst(const String& text, Match& result) const {
        if (!built_) {
            return false;
        }

        ac_algorithm::AhoCorasickIndexMatch raw_match;
        if (!algorithm_.findFirst(text.c_str(), text.size(), raw_match)) {
            return false;
        }

        result = adapt(raw_match);
        return true;
    }

    bool findFirst(const char* text, Match& result) const {
        return text != nullptr && findFirst(String(text), result);
    }

    Vector<Match> findAll(const String& text) const {
        Vector<Match> matches;
        if (!built_) {
            return matches;
        }

        const std::vector<ac_algorithm::AhoCorasickIndexMatch> raw_matches =
            algorithm_.findAll(text.c_str(), text.size());
        for (std::size_t i = 0; i < raw_matches.size(); ++i) {
            matches.insert(adapt(raw_matches[i]));
        }
        return matches;
    }

    Vector<Match> findAll(const char* text) const {
        if (text == nullptr) {
            return Vector<Match>();
        }
        return findAll(String(text));
    }
};

#endif
