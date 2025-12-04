#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "QuickFind.h"
#include "QuickUnion.h"
#include "WeightedQuickUnion.h"
#include "WeightedQuickUnionwithCompression.h"

enum class OpKind { Union, Connected };

struct Operation {
    OpKind kind;
    int a;
    int b;
};

enum class Pattern {
    RandomMix,
    FindHeavy,
    ChainAdversarial,
    StarUnion
};

struct Scenario {
    std::string name;
    std::string focus;
    int n;
    int unions;
    int queries;
    Pattern pattern;
    std::uint32_t seed;
};

std::vector<Operation> build_ops(const Scenario& s) {
    std::vector<Operation> ops;
    ops.reserve(static_cast<std::size_t>(s.unions + s.queries));
    std::mt19937 rng(s.seed);
    std::uniform_int_distribution<int> dist(0, s.n - 1);

    auto push_random_union = [&]() {
        int a = dist(rng);
        int b = dist(rng);
        while (b == a) b = dist(rng);
        ops.push_back({OpKind::Union, a, b});
    };

    if (s.pattern == Pattern::RandomMix) {
        for (int i = 0; i < s.unions; ++i) push_random_union();
        for (int i = 0; i < s.queries; ++i) {
            ops.push_back({OpKind::Connected, dist(rng), dist(rng)});
        }
    } else if (s.pattern == Pattern::FindHeavy) {
        for (int i = 0; i < s.unions; ++i) push_random_union();
        int hot_span = std::max(2, s.n / 50);
        std::uniform_int_distribution<int> hot_dist(0, hot_span - 1);
        for (int i = 0; i < s.queries; ++i) {
            int a = (i % 5 == 0) ? dist(rng) : hot_dist(rng);
            int b = (i % 7 == 0) ? dist(rng) : hot_dist(rng);
            if (a == b) b = (b + 1) % s.n;
            ops.push_back({OpKind::Connected, a, b});
        }
    } else if (s.pattern == Pattern::ChainAdversarial) {
        int chain_edges = std::min(s.n - 1, s.unions);
        for (int i = 0; i < chain_edges; ++i) {
            ops.push_back({OpKind::Union, i, i + 1});
        }
        while (static_cast<int>(ops.size()) < s.unions) push_random_union();
        int leaf = s.n - 1;
        for (int i = 0; i < s.queries; ++i) {
            int other = (i % 3 == 0) ? 0 : std::max(0, leaf - (i % 1024));
            ops.push_back({OpKind::Connected, leaf, other});
        }
    } else if (s.pattern == Pattern::StarUnion) {
        int limit = std::min(s.n - 1, s.unions);
        for (int i = 1; i <= limit; ++i) {
            ops.push_back({OpKind::Union, i, 0});
        }
        while (static_cast<int>(ops.size()) < s.unions) push_random_union();
        for (int i = 0; i < s.queries; ++i) {
            int a = (i % 2 == 0) ? dist(rng) : 0;
            int b = dist(rng);
            if (a == b) b = (b + 1) % s.n;
            ops.push_back({OpKind::Connected, a, b});
        }
    }
    return ops;
}

template <typename UF>
double run_impl(const Scenario& s, const std::vector<Operation>& ops, volatile std::uint64_t& guard) {
    UF uf(s.n);
    std::uint64_t checksum = 0;
    auto start = std::chrono::steady_clock::now();
    for (const auto& op : ops) {
        if (op.kind == OpKind::Union) {
            uf.unite(op.a, op.b);
        } else {
            checksum ^= static_cast<std::uint64_t>(uf.connected(op.a, op.b));
        }
    }
    auto end = std::chrono::steady_clock::now();
    guard ^= checksum;
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
}

struct BenchResult {
    std::string impl;
    double ms;
    std::uint64_t checksum;
};

std::vector<BenchResult> run_case(const Scenario& s, const std::vector<Operation>& ops, volatile std::uint64_t& guard) {
    std::vector<BenchResult> res;
    res.reserve(4);
    auto add = [&](const std::string& impl, double ms, std::uint64_t checksum) {
        res.push_back({impl, ms, checksum});
    };

    {
        volatile std::uint64_t local_guard = 0;
        double ms = run_impl<QuickFind>(s, ops, local_guard);
        add("QuickFind (find O(1))", ms, local_guard);
        guard ^= local_guard;
    }
    {
        volatile std::uint64_t local_guard = 0;
        double ms = run_impl<QuickUnion>(s, ops, local_guard);
        add("QuickUnion (naive trees)", ms, local_guard);
        guard ^= local_guard;
    }
    {
        volatile std::uint64_t local_guard = 0;
        double ms = run_impl<WeightedQuickUnion>(s, ops, local_guard);
        add("WeightedQuickUnion (by size)", ms, local_guard);
        guard ^= local_guard;
    }
    {
        volatile std::uint64_t local_guard = 0;
        double ms = run_impl<WeightedQuickUnionwithCompression>(s, ops, local_guard);
        add("WeightedQuickUnion+Compression", ms, local_guard);
        guard ^= local_guard;
    }

    return res;
}

void print_case(const Scenario& s, const std::vector<BenchResult>& res) {
    double best = res.empty() ? 0.0 : std::min_element(res.begin(), res.end(), [](const BenchResult& a, const BenchResult& b) {
        return a.ms < b.ms;
    })->ms;

    std::cout << "\n[Case] " << s.name << " | n=" << s.n
              << " unions=" << s.unions << " queries=" << s.queries
              << " | focus: " << s.focus << "\n";
    std::cout << std::left << std::setw(38) << "Implementation"
              << std::setw(14) << "Time(ms)"
              << "checksum\n";
    std::cout << std::string(38 + 14 + 12, '-') << "\n";
    for (const auto& r : res) {
        bool fastest = r.ms <= best + 1e-6;
        std::cout << std::left << std::setw(38) << (fastest ? (r.impl + " *") : r.impl)
                  << std::setw(14) << std::fixed << std::setprecision(3) << r.ms
                  << r.checksum << "\n";
    }
}

int main() {
    std::vector<Scenario> cases = {
        {"find-heavy", "Favor QuickFind style O(1) queries with ultra-light unions", 50000, 0, 3000000, Pattern::FindHeavy, 42},
        {"union-heavy-random", "Stress balancing; WeightedQuickUnion without compression should shine", 15000, 30000, 5000, Pattern::RandomMix, 43},
        {"adversarial-chain", "Tall trees plus repeated queries to showcase compression", 30000, 29999, 2000000, Pattern::ChainAdversarial, 44},
        {"star-batch-unions", "Low-depth unions with almost no queries favoring low-overhead QuickUnion", 20000, 19999, 200, Pattern::StarUnion, 45}
    };

    volatile std::uint64_t guard = 0;
    for (const auto& c : cases) {
        auto ops = build_ops(c);
        auto res = run_case(c, ops, guard);
        print_case(c, res);
    }
    // prevent optimizer from discarding results
    if (guard == 0xFFFFFFFFFFFFFFFFULL) std::cerr << "guard: " << guard << "\n";
    return 0;
}
