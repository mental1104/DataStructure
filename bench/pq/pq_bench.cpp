#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cstdlib>

#include "Heap.h"
#include "LeftHeap.h"
#include "SkewHeap.h"
#include "PairingHeap.h"
#include "FibonacciHeap.h"

struct BenchConfig {
    std::size_t initial_size = 200000;
    std::size_t mixed_ops = 400000;
    std::size_t meld_heaps = 800;      // 小堆数量
    std::size_t meld_heap_size = 400;  // 每个小堆大小
    std::uint32_t seed = 42;
};

struct Workload {
    std::vector<int> prefill;
    std::vector<int> mixed_stream;
    std::vector<int> meld_keys; // 为每个小堆提供的键，分段使用
};

struct BenchResult {
    std::string heap;
    std::string scenario;
    std::size_t operations;
    std::size_t final_size;
    double seconds;
    double ns_per_op;
};

namespace {

double per_op(std::size_t ops, double seconds) {
    return ops ? seconds * 1e9 / static_cast<double>(ops) : 0.0;
}

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig cfg;
    for (int i = 1; i + 1 < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--init") cfg.initial_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--mixed-ops") cfg.mixed_ops = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--meld-heaps") cfg.meld_heaps = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--meld-heap-size") cfg.meld_heap_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--seed") cfg.seed = static_cast<std::uint32_t>(std::strtoul(argv[++i], nullptr, 10));
    }
    return cfg;
}

Workload make_workload(const BenchConfig& cfg) {
    Workload w;
    std::mt19937 rng(cfg.seed);
    std::uniform_int_distribution<int> dist(1, static_cast<int>(cfg.initial_size * 4 + cfg.mixed_ops));

    w.prefill.resize(cfg.initial_size);
    for (auto& v : w.prefill) v = dist(rng);

    w.mixed_stream.resize(cfg.mixed_ops);
    for (auto& v : w.mixed_stream) v = dist(rng);

    w.meld_keys.resize(cfg.meld_heaps * cfg.meld_heap_size);
    for (auto& v : w.meld_keys) v = dist(rng);

    return w;
}

template <typename Heap>
BenchResult run_mixed(const std::string& name, const Workload& w) {
    Heap heap;
    for (int v : w.prefill) heap.insert(v);

    auto start = std::chrono::steady_clock::now();
    std::size_t ops = 0;
    std::size_t idx = 0;
    for (; idx < w.mixed_stream.size(); ++idx) {
        heap.insert(w.mixed_stream[idx]); ++ops;
        if ((idx % 3 == 0) && !heap.empty()) { heap.delMax(); ++ops; }
    }
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    return {name, "mixed insert/delete", ops, static_cast<std::size_t>(heap.size()), sec, per_op(ops, sec)};
}

template <typename Heap>
BenchResult run_delete_heavy(const std::string& name, const Workload& w) {
    Heap heap;
    for (int v : w.prefill) heap.insert(v);
    auto start = std::chrono::steady_clock::now();
    std::size_t ops = 0;
    while (!heap.empty()) { heap.delMax(); ++ops; }
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    return {name, "drain heap", ops, static_cast<std::size_t>(heap.size()), sec, per_op(ops, sec)};
}

template <typename Heap>
BenchResult run_meldable(const std::string& name, const BenchConfig& cfg, const Workload& w) {
    Heap main_heap;
    std::size_t offset = 0;
    // 先构建一个非空主堆
    for (std::size_t i = 0; i < cfg.meld_heap_size && i < w.meld_keys.size(); ++i) {
        main_heap.insert(w.meld_keys[i]);
    }
    offset += cfg.meld_heap_size;

    auto start = std::chrono::steady_clock::now();
    std::size_t ops = 0;
    for (std::size_t h = 0; h < cfg.meld_heaps && offset < w.meld_keys.size(); ++h) {
        Heap other;
        for (std::size_t k = 0; k < cfg.meld_heap_size && offset < w.meld_keys.size(); ++k, ++offset) {
            other.insert(w.meld_keys[offset]);
        }
        main_heap.merge(other);
        ops += cfg.meld_heap_size;
    }
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    return {name, "meld-heavy (merge)", ops, static_cast<std::size_t>(main_heap.size()), sec, per_op(ops, sec)};
}

template <typename Heap>
BenchResult run_non_meldable(const std::string& name, const BenchConfig& cfg, const Workload& w) {
    Heap heap;
    std::size_t offset = 0;
    for (std::size_t i = 0; i < cfg.meld_heap_size && i < w.meld_keys.size(); ++i) {
        heap.insert(w.meld_keys[i]);
    }
    offset += cfg.meld_heap_size;

    auto start = std::chrono::steady_clock::now();
    std::size_t ops = 0;
    for (std::size_t h = 0; h < cfg.meld_heaps && offset < w.meld_keys.size(); ++h) {
        for (std::size_t k = 0; k < cfg.meld_heap_size && offset < w.meld_keys.size(); ++k, ++offset) {
            heap.insert(w.meld_keys[offset]);
            ++ops;
        }
    }
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();
    return {name, "meld-heavy (bulk insert)", ops, static_cast<std::size_t>(heap.size()), sec, per_op(ops, sec)};
}

void print_results(const std::vector<BenchResult>& results) {
    std::cout << std::left << std::setw(14) << "Heap"
              << std::setw(26) << "Scenario"
              << std::right << std::setw(12) << "Ops"
              << std::setw(12) << "Final"
              << std::setw(14) << "Seconds"
              << std::setw(14) << "ns/op"
              << "\n";
    for (std::size_t i = 0; i < 92; ++i) std::cout << "-";
    std::cout << "\n";
    for (const auto& r : results) {
        std::cout << std::left << std::setw(14) << r.heap
                  << std::setw(26) << r.scenario
                  << std::right << std::setw(12) << r.operations
                  << std::setw(12) << r.final_size
                  << std::setw(14) << std::fixed << std::setprecision(6) << r.seconds
                  << std::setw(14) << std::fixed << std::setprecision(1) << r.ns_per_op
                  << "\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    BenchConfig cfg = parse_args(argc, argv);
    Workload w = make_workload(cfg);

    std::vector<BenchResult> results;
    // 常规混合读写
    results.push_back(run_mixed<Heap<int>>( "BinHeap", w));
    results.push_back(run_mixed<LeftHeap<int>>( "LeftHeap", w));
    results.push_back(run_mixed<SkewHeap<int>>( "SkewHeap", w));
    results.push_back(run_mixed<PairingHeap<int>>( "Pairing", w));
    results.push_back(run_mixed<FibonacciHeap<int>>( "FibHeap", w));

    // 清空
    results.push_back(run_delete_heavy<Heap<int>>( "BinHeap", w));
    results.push_back(run_delete_heavy<LeftHeap<int>>( "LeftHeap", w));
    results.push_back(run_delete_heavy<SkewHeap<int>>( "SkewHeap", w));
    results.push_back(run_delete_heavy<PairingHeap<int>>( "Pairing", w));
    results.push_back(run_delete_heavy<FibonacciHeap<int>>( "FibHeap", w));

    // 融合场景：展示可合并堆的优势
    results.push_back(run_non_meldable<Heap<int>>( "BinHeap", cfg, w));
    results.push_back(run_meldable<LeftHeap<int>>( "LeftHeap", cfg, w));
    results.push_back(run_meldable<SkewHeap<int>>( "SkewHeap", cfg, w));
    results.push_back(run_meldable<PairingHeap<int>>( "Pairing", cfg, w));
    results.push_back(run_meldable<FibonacciHeap<int>>( "FibHeap", cfg, w));

    print_results(results);
    return 0;
}
