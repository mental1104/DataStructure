#include <chrono>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "match/BM.h"
#include "match/KMP.h"
#include "match/KR.h"

struct Case {
    std::string name;
    String text;
    String pattern;
    long long expected; // -1 means no match expected
};

struct Result {
    std::string algo;
    double ms;
    int index;
};

char random_digit(std::mt19937& rng) {
    static const char digits[] = "0123456789";
    std::uniform_int_distribution<int> dist(0, 9);
    return digits[dist(rng)];
}

char random_ascii(std::mt19937& rng) {
    // printable ASCII 32-126
    std::uniform_int_distribution<int> dist(32, 126);
    return static_cast<char>(dist(rng));
}

std::string make_digits(std::size_t len, std::mt19937& rng) {
    std::string s(len, '0');
    for (std::size_t i = 0; i < len; ++i) s[i] = random_digit(rng);
    return s;
}

std::string make_ascii(std::size_t len, std::mt19937& rng) {
    std::string s(len, ' ');
    for (std::size_t i = 0; i < len; ++i) s[i] = random_ascii(rng);
    return s;
}

Case build_case(const std::string& name, std::size_t text_len, std::size_t pat_len, std::uint32_t seed, bool repetitive) {
    std::mt19937 rng(seed);
    std::string text = repetitive ? std::string(text_len, '8') : make_digits(text_len, rng);
    std::string pattern = repetitive ? std::string(pat_len, '7') : make_digits(pat_len, rng);

    if (text_len >= pat_len) {
        std::size_t pos = text_len / 3;
        if (pos + pat_len > text_len) pos = text_len - pat_len;
        std::copy(pattern.begin(), pattern.end(), text.begin() + static_cast<std::ptrdiff_t>(pos));
        return {name, String(text.c_str()), String(pattern.c_str()), static_cast<long long>(pos)};
    }
    return {name, String(text.c_str()), String(pattern.c_str()), 0};
}

Case build_custom_case(const std::string& name, const std::string& text, const std::string& pattern, long long pos) {
    return {name, String(text.c_str()), String(pattern.c_str()), pos};
}

Result bench(const std::string& algo, const Case& c, const std::function<int()>& fn) {
    (void)c;
    auto start = std::chrono::steady_clock::now();
    int idx = fn();
    auto end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
    return {algo, ms, idx};
}

void run_case(const Case& c) {
    std::vector<Result> results;
    results.reserve(5);

    results.push_back(bench("KMP (basic)", c, [&]() { return matchKMP(c.pattern, c.text, KMPStrategy::Basic, nullptr); }));
    results.push_back(bench("KMP (improved)", c, [&]() { return matchKMP(c.pattern, c.text, KMPStrategy::Improved, nullptr); }));
    results.push_back(bench("BM (bad-char)", c, [&]() { return matchBM(c.pattern, c.text, BMStrategy::BadCharacter, nullptr); }));
    results.push_back(bench("BM (full)", c, [&]() { return matchBM(c.pattern, c.text, BMStrategy::Full, nullptr); }));
    results.push_back(bench("KR", c, [&]() { return matchKR(c.pattern, c.text, nullptr); }));

    std::cout << "\n[Case] " << c.name
              << " text_len=" << c.text.size()
              << " pattern_len=" << c.pattern.size()
              << " expected=";
    if (c.expected < 0) std::cout << "n/a";
    else std::cout << c.expected;
    std::cout << "\n";
    std::cout << std::left << std::setw(18) << "Algorithm"
              << std::right << std::setw(12) << "ms"
              << std::setw(10) << "index"
              << std::setw(12) << "correct"
              << "\n";
    std::cout << std::string(52, '-') << "\n";
    for (const auto& r : results) {
        bool ok = (c.expected >= 0)
            ? (r.index == static_cast<int>(c.expected))
            : (r.index < 0 || r.index >= static_cast<int>(c.text.size() - c.pattern.size() + 1));
        std::cout << std::left << std::setw(18) << r.algo
                  << std::right << std::setw(12) << std::fixed << std::setprecision(3) << r.ms
                  << std::setw(10) << r.index
                  << std::setw(12) << (ok ? "yes" : "no")
                  << "\n";
    }
}

int main() {
    std::vector<Case> cases;
    // baseline random
    cases.push_back(build_case("random-200k", 200000, 64, 42, false));
    cases.push_back(build_case("random-400k", 400000, 128, 43, false));
    // repetitive text but distinct pattern chars
    cases.push_back(build_case("repetitive-200k", 200000, 80, 44, true));
    // BM near-worst-case: long run of the same char in text, pattern shares prefix but last char differs until the planted match
    {
        std::size_t text_len = 200000;
        std::size_t pat_len = 64;
        std::string text(text_len, '7');
        std::string pattern(pat_len - 1, '7');
        pattern.push_back('8');
        std::size_t pos = text_len / 2;
        std::copy(pattern.begin(), pattern.end(), text.begin() + static_cast<std::ptrdiff_t>(pos));
        cases.push_back(build_custom_case("bm-worstish-200k", text, pattern, pos));
    }
    // long pattern to increase preprocessing weight and reduce BM advantage
    cases.push_back(build_case("long-pattern-300k", 300000, 5000, 45, false));
    // large alphabet random ASCII
    {
        std::mt19937 rng(46);
        std::string text = make_ascii(200000, rng);
        std::string pattern = make_ascii(128, rng);
        std::size_t pos = 150000;
        std::copy(pattern.begin(), pattern.end(), text.begin() + static_cast<std::ptrdiff_t>(pos));
        cases.push_back(build_custom_case("ascii-200k", text, pattern, pos));
    }
    // match only at the very end
    {
        std::mt19937 rng(47);
        std::string text = make_digits(200000, rng);
        std::string pattern = make_digits(64, rng);
        std::size_t pos = text.size() - pattern.size();
        std::copy(pattern.begin(), pattern.end(), text.begin() + static_cast<std::ptrdiff_t>(pos));
        cases.push_back(build_custom_case("match-at-end-200k", text, pattern, pos));
    }
    // guaranteed no match: text is digits, pattern is letters
    {
        std::string text(200000, '0');
        std::mt19937 rng(48);
        for (char& c : text) c = random_digit(rng);
        std::string pattern(128, 'A'); // not present in digits
        cases.push_back(build_custom_case("no-match-200k", text, pattern, -1));
    }

    for (const auto& c : cases) run_case(c);
    return 0;
}
