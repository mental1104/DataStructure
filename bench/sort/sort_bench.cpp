#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Sort.h"

struct BenchConfig {
    std::size_t n2_size = 5000;     // 冒泡/选择/插入等 O(n^2) 算法的数据量
    std::size_t nlogn_size = 200000; // O(n log n) 算法的数据量
    std::uint32_t seed = 42;
};

struct BenchCase {
    SortStrategy strategy;
    std::string name;
    std::size_t size;
};

struct BenchResult {
    std::string name;
    std::size_t size;
    double seconds;
    bool sorted;
};

namespace {

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig cfg;
    for (int i = 1; i + 1 < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--n2") cfg.n2_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--nlogn") cfg.nlogn_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--seed") cfg.seed = static_cast<std::uint32_t>(std::strtoul(argv[++i], nullptr, 10));
    }
    return cfg;
}

std::vector<int> make_data(std::size_t n, std::uint32_t seed) {
    std::vector<int> data;
    data.reserve(n);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(n * 4 + 100));
    for (std::size_t i = 0; i < n; ++i) data.push_back(dist(rng));
    return data;
}

bool is_sorted_vec(const Vector<int>& vec) {
    for (int i = 1; i < vec.size(); ++i) {
        if (vec[i - 1] > vec[i]) return false;
    }
    return true;
}

BenchResult run_case(const BenchCase& bc, std::uint32_t seed) {
    std::vector<int> base = make_data(bc.size, seed);
    Vector<int> vec(base.data(), static_cast<Rank>(base.size()));

    auto start = std::chrono::steady_clock::now();
    Sort(vec, bc.strategy);
    auto end = std::chrono::steady_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();
    return {bc.name, bc.size, sec, is_sorted_vec(vec)};
}

void print_results(const std::vector<BenchResult>& results) {
    std::cout << std::left << std::setw(14) << "Algorithm"
              << std::right << std::setw(12) << "N"
              << std::setw(14) << "Seconds"
              << std::setw(10) << "Sorted"
              << "\n";
    for (int i = 0; i < 50; ++i) std::cout << "-";
    std::cout << "\n";
    for (const auto& r : results) {
        std::cout << std::left << std::setw(14) << r.name
                  << std::right << std::setw(12) << r.size
                  << std::setw(14) << std::fixed << std::setprecision(6) << r.seconds
                  << std::setw(10) << (r.sorted ? "yes" : "no")
                  << "\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    BenchConfig cfg = parse_args(argc, argv);

    std::vector<BenchCase> cases = {
        {SortStrategy::BubbleSort,    "Bubble",   cfg.n2_size},
        {SortStrategy::SelectionSort, "Selection",cfg.n2_size},
        {SortStrategy::InsertionSort, "Insertion",cfg.n2_size},
        {SortStrategy::ShellSort,     "Shell",    cfg.nlogn_size},
        {SortStrategy::MergeSort,     "MergeTD",  cfg.nlogn_size},
        {SortStrategy::MergeSortB,    "MergeBU",  cfg.nlogn_size},
        {SortStrategy::QuickSort,     "Quick",    cfg.nlogn_size},
        {SortStrategy::Quick3way,     "Quick3way",cfg.nlogn_size},
        {SortStrategy::QuickSortB,    "QuickBF",  cfg.nlogn_size},
        {SortStrategy::HeapSort,      "Heap",     cfg.nlogn_size},
    };

    std::vector<BenchResult> results;
    results.reserve(cases.size());
    for (const auto& c : cases) {
        results.push_back(run_case(c, cfg.seed));
    }

    print_results(results);
    return 0;
}
