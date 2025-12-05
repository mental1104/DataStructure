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

        Vector<SuffixRef> suffixes(n_, n_, SuffixRef{0, this});
        for (int i = 0; i < n_; ++i) {
            suffixes[i].pos = i;
        }

        Sort(suffixes);

        for (int i = 0; i < n_; ++i) {
            sa_[i] = suffixes[i].pos;
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
    struct SuffixRef {
        int pos;
        const SuffixArray* owner;

        bool operator<(const SuffixRef& other) const {
            return owner->suffix_less(pos, other.pos);
        }
        bool operator>(const SuffixRef& other) const {
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
