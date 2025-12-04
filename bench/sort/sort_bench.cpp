#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <numeric>
#include <sstream>
#include <vector>

#include "Sort.h"

enum class DataShape {
    Random,
    AlmostSorted,
    ManyDuplicates
};

struct BenchConfig {
    std::size_t n2_size = 20000;     // 冒泡/选择/插入等 O(n^2) 算法的数据量
    std::size_t nlogn_size = 2000000; // O(n log n) 算法的数据量
    std::size_t list_n2_size = 2000;    // List 排序用例的小规模（控制更小避免过慢）
    std::size_t list_nlogn_size = 50000; // List 排序用例的大规模（控制更小避免过慢）
    std::uint32_t seed = 42;
    DataShape data_shape = DataShape::Random; // 输入数据形态（默认首个形态）
    std::vector<DataShape> shapes = {DataShape::Random, DataShape::AlmostSorted, DataShape::ManyDuplicates}; // 默认跑随机+近乎有序+大量重复
    double perturb_ratio = 0.001;             // 近乎有序数据的扰动比例（相邻交换次数占比）
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

std::vector<DataShape> parse_shape_list(const std::string& arg) {
    std::vector<DataShape> shapes;
    auto push_shape = [&](const std::string& token) {
        if (token == "almost") shapes.push_back(DataShape::AlmostSorted);
        else if (token == "random") shapes.push_back(DataShape::Random);
        else if (token == "dups" || token == "dup" || token == "repeat" || token == "many") shapes.push_back(DataShape::ManyDuplicates);
    };

    if (arg == "both" || arg == "all") {
        shapes = {DataShape::Random, DataShape::AlmostSorted, DataShape::ManyDuplicates};
        return shapes;
    }

    std::stringstream ss(arg);
    std::string token;
    while (std::getline(ss, token, ',')) {
        push_shape(token);
    }

    if (shapes.empty()) shapes.push_back(DataShape::Random);
    return shapes;
}

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig cfg;
    for (int i = 1; i + 1 < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--n2") cfg.n2_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--nlogn") cfg.nlogn_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--ln2") cfg.list_n2_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--lnlogn") cfg.list_nlogn_size = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--seed") cfg.seed = static_cast<std::uint32_t>(std::strtoul(argv[++i], nullptr, 10));
        else if (arg == "--shape") {
            std::string shape = argv[++i];
            cfg.shapes = parse_shape_list(shape);
            cfg.data_shape = cfg.shapes.front();
        }
        else if (arg == "--perturb") {
            cfg.perturb_ratio = std::strtod(argv[++i], nullptr);
            if (cfg.perturb_ratio < 0.0) cfg.perturb_ratio = 0.0;
        }
    }
    return cfg;
}

std::vector<int> make_almost_sorted(std::size_t n, std::uint32_t seed, double perturb_ratio) {
    std::vector<int> data(n);
    if (n == 0) return data;

    std::iota(data.begin(), data.end(), 0);
    if (n < 2) return data;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::size_t> idx_dist(0, n - 2);
    std::size_t swaps = std::max<std::size_t>(1, static_cast<std::size_t>(n * perturb_ratio));
    for (std::size_t i = 0; i < swaps; ++i) {
        std::size_t idx = idx_dist(rng);
        std::swap(data[idx], data[idx + 1]);
    }
    return data;
}

std::vector<int> make_data(std::size_t n, std::uint32_t seed, DataShape shape, double perturb_ratio) {
    if (shape == DataShape::AlmostSorted) return make_almost_sorted(n, seed, perturb_ratio);
    if (shape == DataShape::ManyDuplicates) {
        std::vector<int> data;
        data.reserve(n);
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 9); // 大量重复：值域压缩到 10 个不同值
        for (std::size_t i = 0; i < n; ++i) data.push_back(dist(rng));
        return data;
    }

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

bool is_sorted_list(const List<int>& lst) {
    if (lst.size() < 2) return true;
    int prev = lst[0];
    for (int i = 1; i < lst.size(); ++i) {
        if (prev > lst[i]) return false;
        prev = lst[i];
    }
    return true;
}

const char* shape_name(DataShape s) {
    switch (s) {
    case DataShape::Random: return "random";
    case DataShape::AlmostSorted: return "almost";
    case DataShape::ManyDuplicates: return "dups";
    }
    return "random";
}

void print_header(const BenchConfig& cfg, DataShape shape, const char* domain, std::size_t n2, std::size_t nlogn) {
    std::cout << "[" << domain << "] shape=" << shape_name(shape)
              << ", perturb=" << cfg.perturb_ratio
              << ", seed=" << cfg.seed
              << ", n2=" << n2
              << ", nlogn=" << nlogn
              << "\n";
}

BenchResult run_case(const BenchCase& bc, const BenchConfig& cfg, DataShape shape) {
    std::vector<int> base = make_data(bc.size, cfg.seed, shape, cfg.perturb_ratio);
    Vector<int> vec(base.data(), static_cast<Rank>(base.size()));

    auto start = std::chrono::steady_clock::now();
    Sort(vec, bc.strategy);
    auto end = std::chrono::steady_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();
    return {bc.name, bc.size, sec, is_sorted_vec(vec)};
}

List<int> make_list(const std::vector<int>& data) {
    List<int> lst;
    for (int v : data) lst.insertAsLast(v);
    return lst;
}

BenchResult run_list_case(const BenchCase& bc, const BenchConfig& cfg, DataShape shape) {
    std::vector<int> base = make_data(bc.size, cfg.seed, shape, cfg.perturb_ratio);
    List<int> lst = make_list(base);

    auto start = std::chrono::steady_clock::now();
    Sort(lst, bc.strategy);
    auto end = std::chrono::steady_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();
    return {bc.name, bc.size, sec, is_sorted_list(lst)};
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

    const auto& shapes = cfg.shapes;

    // Vector benchmarks first
    std::cout << "[Vector]\n";
    bool first_block = true;
    for (std::size_t idx = 0; idx < shapes.size(); ++idx) {
        DataShape shape = shapes[idx];
        bool is_first = idx == 0;
        bool allow_insertion = idx < 2; // 第三个场景起不再跑插入排序
        if (!first_block) std::cout << "\n";
        first_block = false;

        std::vector<BenchCase> cases;
        if (is_first) {
            cases.push_back({SortStrategy::BubbleSort,    "Bubble",    cfg.n2_size});
            cases.push_back({SortStrategy::SelectionSort, "Selection", cfg.n2_size});
        }
        if (allow_insertion) cases.push_back({SortStrategy::InsertionSort, "Insertion", cfg.n2_size});
        cases.push_back({SortStrategy::ShellSort,     "Shell",    cfg.nlogn_size});
        cases.push_back({SortStrategy::MergeSort,     "MergeTD",  cfg.nlogn_size});
        cases.push_back({SortStrategy::MergeSortB,    "MergeBU",  cfg.nlogn_size});
        cases.push_back({SortStrategy::QuickSort,     "Quick",    cfg.nlogn_size});
        cases.push_back({SortStrategy::Quick3way,     "Quick3way",cfg.nlogn_size});
        cases.push_back({SortStrategy::QuickSortB,    "QuickBF",  cfg.nlogn_size});
        cases.push_back({SortStrategy::HeapSort,      "Heap",     cfg.nlogn_size});

        if (shape == DataShape::AlmostSorted) {
            for (auto& c : cases) {
                if (c.strategy == SortStrategy::InsertionSort) c.size = cfg.nlogn_size;
            }
        }

        std::vector<BenchResult> results;
        results.reserve(cases.size());
        for (const auto& c : cases) {
            results.push_back(run_case(c, cfg, shape));
        }

        print_header(cfg, shape, "Vector", cfg.n2_size, cfg.nlogn_size);
        print_results(results);
    }

    // List benchmarks after all vector cases
    std::cout << "\n[List]\n";
    first_block = true;
    for (std::size_t idx = 0; idx < shapes.size(); ++idx) {
        DataShape shape = shapes[idx];
        bool allow_insertion = idx < 2;
        if (!first_block) std::cout << "\n";
        first_block = false;

        std::vector<BenchCase> list_cases = {
            {SortStrategy::SelectionSort, "L-Selection", cfg.list_n2_size},
            {SortStrategy::InsertionSort, "L-Insertion", allow_insertion ? cfg.list_n2_size : 0},
            {SortStrategy::MergeSort,     "L-MergeTD",   cfg.list_nlogn_size},
            {SortStrategy::RadixSort,     "L-Radix",     cfg.list_nlogn_size}
        };
        if (shape == DataShape::AlmostSorted) {
            for (auto& c : list_cases) {
                if (c.strategy == SortStrategy::InsertionSort) c.size = allow_insertion ? cfg.list_nlogn_size : 0;
            }
        }

        std::vector<BenchResult> list_results;
        list_results.reserve(list_cases.size());
        for (const auto& c : list_cases) {
            if (c.size == 0) continue;
            list_results.push_back(run_list_case(c, cfg, shape));
        }

        print_header(cfg, shape, "List", cfg.list_n2_size, cfg.list_nlogn_size);
        print_results(list_results);
    }
    return 0;
}
