#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "suffix_array.h"
#include "match/BM.h"
#include "match/KMP.h"
#include "match/KR.h"

struct Query {
    String pattern;
    int expected; // -1 if not present
};

std::string make_random_digits(std::size_t len, std::mt19937& rng) {
    static const char digits[] = "0123456789";
    std::uniform_int_distribution<int> dist(0, 9);
    std::string s(len, '0');
    for (std::size_t i = 0; i < len; ++i) s[i] = digits[dist(rng)];
    return s;
}

std::vector<Query> build_queries(const std::string& base, std::size_t qlen, int num_present, int num_absent, std::mt19937& rng) {
    std::vector<Query> qs;
    qs.reserve(static_cast<std::size_t>(num_present + num_absent));

    // present queries sampled from the base text
    std::uniform_int_distribution<int> dist(0, static_cast<int>(base.size() - qlen));
    for (int i = 0; i < num_present; ++i) {
        int pos = dist(rng);
        qs.push_back({ String(base.substr(static_cast<std::size_t>(pos), qlen).c_str()), pos });
    }

    // absent queries: choose characters outside digit set to avoid accidental hits
    for (int i = 0; i < num_absent; ++i) {
        std::string pat(qlen, 'a');
        qs.push_back({ String(pat.c_str()), -1 });
    }
    return qs;
}

int brute_search(const String& text, const String& pat) {
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pat.size());
    if (m == 0) return 0;
    if (m > n) return -1;
    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m && text[i + j] == pat[j]) ++j;
        if (j == m) return i;
    }
    return -1;
}

int suffix_search(const SuffixArray<String>& sa, const String& text, const String& pat) {
    int pos = sa.rank(pat);
    if (pos >= sa.length()) return -1;
    int start = sa.index(pos);
    if (start + static_cast<int>(pat.size()) > static_cast<int>(text.size())) return -1;
    for (int i = 0; i < static_cast<int>(pat.size()); ++i) {
        if (text[start + i] != pat[i]) return -1;
    }
    return start;
}

template <typename Fn>
std::pair<double, int> bench_queries(const std::vector<Query>& qs, int text_len, Fn&& fn) {
    auto t0 = std::chrono::steady_clock::now();
    int mismatches = 0;
    for (const auto& q : qs) {
        int idx = fn(q.pattern);
        bool ok = (q.expected >= 0)
            ? (idx == q.expected)
            : (idx < 0 || idx > text_len - static_cast<int>(q.pattern.size()));
        if (!ok) ++mismatches;
    }
    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();
    return { ms, mismatches };
}

int main() {
    const std::size_t text_len = 200000;
    const std::size_t query_len = 32;
    const int present = 200;
    const int absent = 200;

    std::mt19937 rng(2024);
    std::string base_text = make_random_digits(text_len, rng);
    String text(base_text.c_str());

    auto queries = build_queries(base_text, query_len, present, absent, rng);

    // Build suffix array (measure separately)
    auto build_start = std::chrono::steady_clock::now();
    SuffixArray<String> sa(text);
    auto build_end = std::chrono::steady_clock::now();
    double build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(build_end - build_start).count();

    auto brute = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return brute_search(text, pat);
    });

    auto kmp_basic = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return matchKMP(pat, text, KMPStrategy::Basic, nullptr);
    });

    auto kmp_improved = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return matchKMP(pat, text, KMPStrategy::Improved, nullptr);
    });

    auto bm_bad = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return matchBM(pat, text, BMStrategy::BadCharacter, nullptr);
    });

    auto bm_full = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return matchBM(pat, text, BMStrategy::Full, nullptr);
    });

    auto kr = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return matchKR(pat, text, nullptr);
    });

    auto sa_res = bench_queries(queries, static_cast<int>(text.size()), [&](const String& pat) {
        return suffix_search(sa, text, pat);
    });

    std::cout << "Suffix array benchmark (text len=" << text_len
              << ", query len=" << query_len
              << ", total queries=" << queries.size()
              << ")\n";
    std::cout << std::left << std::setw(20) << "Phase"
              << std::right << std::setw(12) << "ms"
              << std::setw(14) << "mismatches"
              << "\n";
    std::cout << std::string(46, '-') << "\n";
    std::cout << std::left << std::setw(20) << "SA build"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << build_ms
              << std::setw(14) << 0
              << "\n";
    std::cout << std::left << std::setw(20) << "Brute-force"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << brute.first
              << std::setw(14) << brute.second
              << "\n";
    std::cout << std::left << std::setw(20) << "KMP basic"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << kmp_basic.first
              << std::setw(14) << kmp_basic.second
              << "\n";
    std::cout << std::left << std::setw(20) << "KMP improved"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << kmp_improved.first
              << std::setw(14) << kmp_improved.second
              << "\n";
    std::cout << std::left << std::setw(20) << "BM bad-char"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << bm_bad.first
              << std::setw(14) << bm_bad.second
              << "\n";
    std::cout << std::left << std::setw(20) << "BM full"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << bm_full.first
              << std::setw(14) << bm_full.second
              << "\n";
    std::cout << std::left << std::setw(20) << "KR"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << kr.first
              << std::setw(14) << kr.second
              << "\n";
    std::cout << std::left << std::setw(20) << "Suffix array"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << sa_res.first
              << std::setw(14) << sa_res.second
              << "\n";

    return 0;
}
