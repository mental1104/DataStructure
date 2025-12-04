#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Trie.h"
#include "TST.h"

struct Workload {
    std::string name;
    std::string note;
    std::size_t key_len;
    std::vector<String> keys;
    std::vector<String> hits;
    std::vector<String> misses;
    std::vector<String> prefixes;
};

struct BenchResult {
    std::string impl;
    std::string case_name;
    double build_ms{0.0};
    double hit_ms{0.0};
    double miss_ms{0.0};
    double prefix_ms{0.0};
    std::size_t prefix_output{0};
};

volatile std::size_t g_sink = 0;

std::vector<std::string> make_unique_words(std::size_t count, std::size_t len, const std::string& alphabet, std::uint32_t seed) {
    std::vector<std::string> words;
    words.reserve(count);
    std::unordered_set<std::string> seen;
    seen.reserve(count * 2);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(alphabet.size() - 1));
    while (words.size() < count) {
        std::string s(len, ' ');
        for (std::size_t i = 0; i < len; ++i) s[i] = alphabet[dist(rng)];
        if (seen.insert(s).second) words.push_back(std::move(s));
    }
    return words;
}

std::vector<String> to_dsa_strings(const std::vector<std::string>& src) {
    std::vector<String> out;
    out.reserve(src.size());
    for (const auto& s : src) out.emplace_back(s.c_str());
    return out;
}

std::vector<String> make_prefixes(const std::vector<std::string>& base, std::size_t prefix_len, std::size_t limit) {
    std::vector<String> res;
    res.reserve(limit);
    for (std::size_t i = 0; i < base.size() && res.size() < limit; ++i) {
        if (base[i].size() > prefix_len) res.emplace_back(base[i].substr(0, prefix_len).c_str());
    }
    return res;
}

std::vector<String> repeat_prefixes(const std::vector<String>& base, std::size_t repeat) {
    if (repeat <= 1) return base;
    std::vector<String> res;
    res.reserve(base.size() * repeat);
    for (std::size_t i = 0; i < repeat; ++i) {
        res.insert(res.end(), base.begin(), base.end());
    }
    return res;
}

Workload make_ascii_lookup_case() {
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const std::size_t key_len = 8;
    const std::size_t key_count = 120000;
    Workload w;
    w.name = "ascii-short-lookup";
    w.note = "Large alphabet, short keys, hit/miss lookup heavy to favor array-indexed Trie.";
    w.key_len = key_len;

    auto words = make_unique_words(key_count, key_len, alphabet, 42);
    w.keys = to_dsa_strings(words);
    w.hits = w.keys;

    std::vector<std::string> miss_words;
    miss_words.reserve(words.size());
    for (const auto& s : words) {
        std::string miss = s;
        miss[0] = '!'; // outside the alphabet used above
        miss_words.push_back(std::move(miss));
    }
    w.misses = to_dsa_strings(miss_words);

    auto prefixes = make_prefixes(words, 3, 24);
    prefixes.emplace_back("!");
    w.prefixes = std::move(prefixes);
    return w;
}

Workload make_binary_prefix_case() {
    const std::string alphabet = "01";
    const std::size_t key_len = 48;
    const std::size_t key_count = 10000; // keep deep chains but lighter to avoid long Trie scans
    Workload w;
    w.name = "binary-deep-prefix";
    w.note = "Tiny alphabet, deep keys, prefix enumeration repeated to favor TST's 3-way branching.";
    w.key_len = key_len;

    auto words = make_unique_words(key_count, key_len, alphabet, 314159);
    w.keys = to_dsa_strings(words);
    w.hits = w.keys;

    std::vector<std::string> miss_words;
    miss_words.reserve(words.size());
    for (const auto& s : words) {
        std::string miss = s;
        miss[0] = '2'; // ensure misses are absent from the binary alphabet data set
        miss_words.push_back(std::move(miss));
    }
    w.misses = to_dsa_strings(miss_words);

    std::vector<String> base_prefixes = {String("0"), String("1"), String("00"), String("01"), String("10"), String("11")};
    if (!words.empty()) {
        base_prefixes.emplace_back(words[0].substr(0, 12).c_str());
        base_prefixes.emplace_back(words[0].substr(0, 24).c_str());
    }
    w.prefixes = repeat_prefixes(base_prefixes, 3); // still prefix-heavy but bounded
    return w;
}

template <typename Dict>
BenchResult run_dict(Workload& w, const std::string& impl) {
    Dict dict;
    BenchResult res;
    res.impl = impl;
    res.case_name = w.name;
    auto now = [] { return std::chrono::steady_clock::now(); };

    auto t0 = now();
    for (auto& k : w.keys) dict.put(k, 1);
    auto t1 = now();
    res.build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();

    std::size_t digest = 0;

    t0 = now();
    for (auto& q : w.hits) digest += static_cast<std::size_t>(dict.get(q));
    t1 = now();
    res.hit_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();

    t0 = now();
    for (auto& q : w.misses) digest += static_cast<std::size_t>(dict.get(q));
    t1 = now();
    res.miss_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();

    t0 = now();
    for (auto& pre : w.prefixes) {
        auto v = dict.keysWithPrefix(pre);
        auto sz = static_cast<std::size_t>(v.size());
        res.prefix_output += sz;
        digest += sz;
    }
    t1 = now();
    res.prefix_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();

    g_sink ^= digest ^ res.prefix_output;
    return res;
}

void print_results(const Workload& w, const std::vector<BenchResult>& results) {
    std::cout << "\n[Case] " << w.name
              << " keys=" << w.keys.size()
              << " key_len=" << w.key_len
              << "\nNote: " << w.note << "\n";

    const int impl_w = 14;
    const int cell_w = 12;
    std::cout << std::left << std::setw(impl_w) << "impl"
              << std::right << std::setw(cell_w) << "build"
              << std::setw(cell_w) << "hit"
              << std::setw(cell_w) << "miss"
              << std::setw(cell_w + 2) << "prefix"
              << std::setw(cell_w + 4) << "prefix_out"
              << "\n";
    std::cout << std::string(impl_w + cell_w * 4 + 6, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(impl_w) << r.impl
                  << std::right << std::setw(cell_w) << std::fixed << std::setprecision(3) << r.build_ms
                  << std::setw(cell_w) << std::fixed << std::setprecision(3) << r.hit_ms
                  << std::setw(cell_w) << std::fixed << std::setprecision(3) << r.miss_ms
                  << std::setw(cell_w + 2) << std::fixed << std::setprecision(3) << r.prefix_ms
                  << std::setw(cell_w + 4) << static_cast<long long>(r.prefix_output)
                  << "\n";
    }
}

int main() {
    std::vector<Workload> cases;
    cases.push_back(make_ascii_lookup_case());
    cases.push_back(make_binary_prefix_case());

    for (auto& c : cases) {
        std::vector<BenchResult> results;
        results.reserve(2);
        results.push_back(run_dict<Trie<int>>(c, "Trie (R-way)"));
        results.push_back(run_dict<TST<int>>(c, "TST (ternary)"));
        print_results(c, results);
    }
    return 0;
}
