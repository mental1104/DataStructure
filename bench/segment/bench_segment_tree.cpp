#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "SegmentTree.h"

struct SumOpLL {
    long long operator()(long long a, long long b) const { return a + b; }
};

struct Query {
    int l;
    int r;
};

int main() {
    const int N = 200000;
    const int Q = 40000;
    const int U = 10000;

    std::mt19937 rng(2025);
    std::uniform_int_distribution<int> val_dist(-10, 10);
    std::uniform_int_distribution<int> pos_dist(0, N - 1);

    Vector<long long> data(N, N, static_cast<long long>(0));
    for (int i = 0; i < N; ++i) data[i] = val_dist(rng);

    // build queries
    std::vector<Query> queries;
    queries.reserve(Q);
    for (int i = 0; i < Q; ++i) {
        int l = pos_dist(rng);
        int r = pos_dist(rng);
        if (l > r) std::swap(l, r);
        if (r == l) r = std::min(N, l + 1);
        queries.push_back({l, r});
    }

    // build updates
    std::vector<std::pair<int, long long>> updates;
    updates.reserve(U);
    for (int i = 0; i < U; ++i) {
        int p = pos_dist(rng);
        long long v = val_dist(rng);
        updates.push_back({p, v});
    }

    // prefix baseline (static)
    auto t0 = std::chrono::steady_clock::now();
    std::vector<long long> prefix(N + 1, 0);
    for (int i = 0; i < N; ++i) prefix[i + 1] = prefix[i] + data[i];
    auto t1 = std::chrono::steady_clock::now();
    double prefix_build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();

    // segment tree build
    auto b0 = std::chrono::steady_clock::now();
    SegmentTree<long long, SumOpLL> st(data, SumOpLL(), 0);
    auto b1 = std::chrono::steady_clock::now();
    double st_build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(b1 - b0).count();

    // static queries: prefix
    auto q0 = std::chrono::steady_clock::now();
    long long chk1 = 0;
    for (const auto& q : queries) {
        chk1 += prefix[q.r] - prefix[q.l];
    }
    auto q1 = std::chrono::steady_clock::now();
    double prefix_query_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(q1 - q0).count();

    // static queries: segment tree
    auto s0 = std::chrono::steady_clock::now();
    long long chk2 = 0;
    for (const auto& q : queries) {
        chk2 += st.query(q.l, q.r);
    }
    auto s1 = std::chrono::steady_clock::now();
    double st_query_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(s1 - s0).count();

    // mixed updates + queries (segment tree only)
    auto m0 = std::chrono::steady_clock::now();
    long long chk3 = 0;
    for (int i = 0; i < U; ++i) {
        st.update(updates[i].first, updates[i].second);
        // simple query after each update
        const auto& q = queries[i % queries.size()];
        chk3 += st.query(q.l, q.r);
    }
    auto m1 = std::chrono::steady_clock::now();
    double st_mixed_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m1 - m0).count();

    std::cout << "Segment tree benchmark (N=" << N
              << ", Q=" << Q << ", U=" << U << ")\n";
    std::cout << std::left << std::setw(22) << "Phase"
              << std::right << std::setw(12) << "ms"
              << "\n";
    std::cout << std::string(34, '-') << "\n";
    std::cout << std::left << std::setw(22) << "Prefix build"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << prefix_build_ms << "\n";
    std::cout << std::left << std::setw(22) << "Segment build"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << st_build_ms << "\n";
    std::cout << std::left << std::setw(22) << "Prefix queries"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << prefix_query_ms << "\n";
    std::cout << std::left << std::setw(22) << "Segment queries"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << st_query_ms << "\n";
    std::cout << std::left << std::setw(22) << "Segment mixed (U+Q)"
              << std::right << std::setw(12) << std::fixed << std::setprecision(3) << st_mixed_ms << "\n";

    // checksum to avoid dead-code elimination
    std::cout << "Checksums: " << chk1 << ", " << chk2 << ", " << chk3 << "\n";
    return 0;
}
