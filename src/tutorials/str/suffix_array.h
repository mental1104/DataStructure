#pragma once

#include <functional>   // std::less
#include <stdexcept>
#include <utility>

#include "Sort.h"
#include "Vector.h"
#include "dsa_string.h"

// Text must support:
//   - text.size() -> size_t
//   - text[i]     -> value_type
//   - copy-constructible
// 构建：O(n log n) 倍增（按秩排序二元组）
template <typename Text,
          typename Less = std::less<typename Text::value_type>>
class SuffixArray {
public:
    using value_type = typename Text::value_type;

    explicit SuffixArray(const Text& text)
        : text_(text),
          n_(static_cast<int>(text_.size())),
          sa_(n_, n_, 0),
          less_() {

        if (n_ == 0) return;

        Vector<int> rank(n_, n_, 0);
        Vector<int> tmp_rank(n_, n_, 0);
        Vector<int> tmp_sa(n_, n_, 0);

        // 初始按首字符排序并压缩秩
        Vector<CharRef> chars(n_, n_, CharRef{0, this});
        for (int i = 0; i < n_; ++i) chars[i].pos = i;
        Sort(chars);
        int r = 0;
        sa_[0] = chars[0].pos;
        rank[sa_[0]] = 0;
        for (int i = 1; i < n_; ++i) {
            sa_[i] = chars[i].pos;
            if (!char_equal(chars[i - 1].pos, chars[i].pos)) ++r;
            rank[sa_[i]] = r;
        }

        // 计数排序专用桶，0 保留给越界(-1)，其余为秩+1
        int max_bucket = (r + 2 > n_) ? (r + 2) : n_;
        Vector<int> cnt(max_bucket, max_bucket, 0);

        auto counting_sort = [&](int k) {
            for (int i = 0; i < max_bucket; ++i) cnt[i] = 0;
            for (int i = 0; i < n_; ++i) {
                int idx = (sa_[i] + k < n_) ? (rank[sa_[i] + k] + 1) : 0;
                ++cnt[idx];
            }
            for (int i = 1; i < max_bucket; ++i) cnt[i] += cnt[i - 1];
            for (int i = n_ - 1; i >= 0; --i) {
                int s = sa_[i];
                int idx = (s + k < n_) ? (rank[s + k] + 1) : 0;
                tmp_sa[--cnt[idx]] = s;
            }
            for (int i = 0; i < n_; ++i) sa_[i] = tmp_sa[i];
        };

        for (int k = 1; r < n_ - 1; k <<= 1) {
            counting_sort(k);
            counting_sort(0);

            tmp_rank[sa_[0]] = 0;
            r = 0;
            for (int i = 1; i < n_; ++i) {
                int cur = sa_[i];
                int prev = sa_[i - 1];
                bool diff_first = rank[cur] != rank[prev];
                int cur_second = (cur + k < n_) ? rank[cur + k] : -1;
                int prev_second = (prev + k < n_) ? rank[prev + k] : -1;
                if (diff_first || cur_second != prev_second) ++r;
                tmp_rank[cur] = r;
            }
            for (int i = 0; i < n_; ++i) rank[i] = tmp_rank[i];

            // 更新桶大小以减少无用遍历
            int needed = r + 2;
            if (needed > max_bucket) {
                max_bucket = needed;
                cnt = Vector<int>(max_bucket, max_bucket, 0);
            }
        }
    }

    int length() const { return n_; }

    // Return the i-th suffix as a "view" [begin, end) in the original text.
    // You can adapt this to return an actual container if you prefer.
    std::pair<int, int> select_view(int i) const {
        check_index(i, "select_view");
        return { sa_[i], n_ };  // [start, n_)
    }

    // For convenience: extract suffix as a Text (requires Text(begin,end) ctor)
    Text select(int i) const {
        check_index(i, "select");
        return Text(text_.begin() + sa_[i], text_.end());
    }

    int index(int i) const {
        check_index(i, "index");
        return sa_[i];
    }

    int lcp(int i) const {
        if (i <= 0 || i >= n_) {
            throw std::out_of_range("lcp: i must be in [1, N-1]");
        }
        return lcp_suffix(sa_[i], sa_[i - 1]);
    }

    // Rank for a key sequence with same value_type (e.g. another string or vector)
    int rank(const Text& key) const {
        int lo = 0;
        int hi = n_ - 1;

        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            int cmp = compare_key_suffix(key, sa_[mid]);
            if (cmp < 0) {
                hi = mid - 1;
            } else if (cmp > 0) {
                lo = mid + 1;
            } else {
                return mid;
            }
        }
        return lo;
    }

private:
    struct CharRef {
        int pos;
        const SuffixArray* owner;

        bool operator<(const CharRef& other) const {
            return owner->char_less(pos, other.pos);
        }
        bool operator>(const CharRef& other) const {
            return other < *this;
        }
    };

    struct Key {
        int pos;
        int first;
        int second;

        bool operator<(const Key& other) const {
            if (first != other.first) return first < other.first;
            return second < other.second;
        }
        bool operator>(const Key& other) const {
            return other < *this;
        }
    };

    Text text_;
    int n_;
    Vector<int> sa_;
    Less less_;

    void check_index(int i, const char* func) const {
        if (i < 0 || i >= n_) {
            String msg(func);
            msg = msg + ": index out of range";
            throw std::out_of_range(msg.c_str());
        }
    }

    bool suffix_less(int i, int j) const {
        int a = i;
        int b = j;
        while (a < n_ && b < n_ &&
               !less_(text_[b], text_[a]) &&
               !less_(text_[a], text_[b])) {
            ++a;
            ++b;
        }
        if (a == n_ && b == n_) return false;  // equal: i is not < j
        if (a == n_) return true;              // suffix at i is shorter
        if (b == n_) return false;
        return less_(text_[a], text_[b]);
    }

    bool char_less(int i, int j) const {
        return less_(text_[i], text_[j]);
    }

    bool char_equal(int i, int j) const {
        return !less_(text_[i], text_[j]) && !less_(text_[j], text_[i]);
    }

    int lcp_suffix(int i, int j) const {
        int len = 0;
        while (i < n_ && j < n_ &&
               !less_(text_[j], text_[i]) &&
               !less_(text_[i], text_[j])) {
            ++i;
            ++j;
            ++len;
        }
        return len;
    }

    // return <0 if key < suffix, 0 if equal, >0 if key > suffix
    int compare_key_suffix(const Text& key, int start) const {
        int i = 0;
        int j = start;
        int key_n = static_cast<int>(key.size());

        while (i < key_n && j < n_ &&
               !less_(text_[j], key[i]) &&
               !less_(key[i], text_[j])) {
            ++i;
            ++j;
        }

        if (i == key_n && j == n_) return 0;
        if (i == key_n) return -1;   // key shorter
        if (j == n_) return 1;       // suffix shorter

        // compare differing elements
        if (less_(key[i], text_[j])) return -1;
        if (less_(text_[j], key[i])) return 1;
        return 0; // theoretically unreachable (we already checked !=)
    }
};
