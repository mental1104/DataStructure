#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "AVL.h"
#include "RedBlack.h"
#include "Splay.h"

struct BenchConfig {
    std::size_t initial_size = 30000;  // number of elements to prefill
    std::size_t access_ops = 60000;    // random search operations
    std::size_t locality_ops = 80000;  // locality-heavy search operations
    std::size_t insert_ops = 30000;    // random insertion operations
    std::size_t erase_ops = 30000;     // random erase operations
    std::uint32_t seed = 42;           // RNG seed for repeatability
};

struct Workload {
    std::vector<int> base_keys;           // prefill keys for access/insert phases
    std::vector<int> access_queries;      // keys to search (hit/miss mixed)
    std::vector<int> locality_queries;    // locality-prone searches
    std::vector<int> insert_keys;         // new keys to insert (no overlap)
    std::vector<int> erase_initial_keys;  // prefill keys for erase phase
    std::vector<int> erase_keys;          // keys that will be removed
};

struct BenchResult {
    std::string tree;
    std::string scenario;
    std::size_t initial_size;
    std::size_t operations;
    std::size_t final_size;
    double seconds;
    double ns_per_op;
    std::size_t checksum;
};

namespace {

double per_op(std::size_t ops, double seconds) {
    return ops ? seconds * 1e9 / static_cast<double>(ops) : 0.0;
}

void print_usage(const char* exe) {
    std::cout << "Usage: " << exe
              << " [--size <initial_elements>] [--access-ops <count>] [--insert-ops <count>]"
              << " [--erase-ops <count>] [--locality-ops <count>] [--seed <value>]\n";
}

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig config;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto require_value = [&](std::size_t& target) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                throw std::invalid_argument("Missing value for " + arg);
            }
            target = static_cast<std::size_t>(std::stoull(argv[++i]));
        };

        if (arg == "--size") {
            require_value(config.initial_size);
        } else if (arg == "--access-ops") {
            require_value(config.access_ops);
        } else if (arg == "--locality-ops") {
            require_value(config.locality_ops);
        } else if (arg == "--insert-ops") {
            require_value(config.insert_ops);
        } else if (arg == "--erase-ops") {
            require_value(config.erase_ops);
        } else if (arg == "--seed") {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                throw std::invalid_argument("Missing value for --seed");
            }
            config.seed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        } else {
            print_usage(argv[0]);
            throw std::invalid_argument("Unknown argument: " + arg);
        }
    }

    if (config.initial_size == 0) {
        throw std::invalid_argument("Initial size must be greater than 0");
    }

    return config;
}

Workload make_workload(const BenchConfig& config) {
    Workload w;
    std::mt19937 rng(config.seed);

    // 基础预填充键（打乱插入顺序以避免偏向）
    w.base_keys.resize(config.initial_size);
    std::iota(w.base_keys.begin(), w.base_keys.end(), 0);
    std::shuffle(w.base_keys.begin(), w.base_keys.end(), rng);

    // 随机访问查询：80% 命中，20% 未命中
    std::bernoulli_distribution hit_chance(0.8);
    std::uniform_int_distribution<std::size_t> hit_idx(0, config.initial_size - 1);
    std::uniform_int_distribution<int> miss_val(
        static_cast<int>(config.initial_size * 2),
        static_cast<int>(config.initial_size * 4 + config.insert_ops));
    w.access_queries.reserve(config.access_ops);
    for (std::size_t i = 0; i < config.access_ops; ++i) {
        int key = hit_chance(rng) ? w.base_keys[hit_idx(rng)] : miss_val(rng);
        w.access_queries.push_back(key);
    }

    // 局部性访问序列：小热集合 + burst 模式，突出 Splay 的自适应优势
    const std::size_t hot_count = std::max<std::size_t>(4, std::min<std::size_t>(32, w.base_keys.size()));
    std::vector<int> hot_keys(w.base_keys.begin(), w.base_keys.begin() + static_cast<std::ptrdiff_t>(hot_count));

    std::geometric_distribution<int> burst_len(0.08);     // 平均 ~12 次的连续访问
    std::bernoulli_distribution switch_hot(0.2);          // 低概率切换热 key
    std::bernoulli_distribution cold_jump(0.02);          // 罕见跳出热集合
    std::uniform_int_distribution<int> jump_val(
        static_cast<int>(config.initial_size * 2),
        static_cast<int>(config.initial_size * 4 + config.insert_ops));
    std::uniform_int_distribution<std::size_t> hot_start(0, hot_count - 1);

    std::size_t hot_idx = hot_start(rng);
    int remaining = burst_len(rng) + 1;
    w.locality_queries.reserve(config.locality_ops);
    for (std::size_t i = 0; i < config.locality_ops; ++i) {
        if (cold_jump(rng)) {
            w.locality_queries.push_back(jump_val(rng)); // 小概率未命中
        } else {
            if (remaining == 0) {
                if (switch_hot(rng)) {
                    hot_idx = hot_start(rng);
                }
                remaining = burst_len(rng) + 1;
            }
            w.locality_queries.push_back(hot_keys[hot_idx]);
            --remaining;
        }
    }

    // 插入键：与 base_keys 不重叠，保持唯一
    w.insert_keys.reserve(config.insert_ops);
    int next_key = static_cast<int>(config.initial_size * 4 + 7);
    std::uniform_int_distribution<int> gap(1, 3);
    for (std::size_t i = 0; i < config.insert_ops; ++i) {
        next_key += gap(rng);
        w.insert_keys.push_back(next_key);
    }

    // 删除阶段的初始键与删除序列
    const std::size_t erase_initial_size = config.initial_size + config.erase_ops;
    w.erase_initial_keys.resize(erase_initial_size);
    std::iota(w.erase_initial_keys.begin(), w.erase_initial_keys.end(), 0);
    std::shuffle(w.erase_initial_keys.begin(), w.erase_initial_keys.end(), rng);
    w.erase_keys.assign(w.erase_initial_keys.begin(),
                        w.erase_initial_keys.begin() + static_cast<std::ptrdiff_t>(config.erase_ops));

    return w;
}

template <typename Tree>
void bulk_insert(Tree& tree, const std::vector<int>& keys) {
    for (int key : keys) {
        tree.insert(key);
    }
}

template <typename Tree>
BenchResult bench_random_access(const std::string& tree_name, const BenchConfig& config,
                                const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.base_keys);

    std::size_t checksum = 0;
    std::size_t hits = 0;

    auto start = std::chrono::steady_clock::now();
    for (int key : data.access_queries) {
        BinNode<int>* node = tree.search(key);
        if (node && node->data == key) {
            ++hits;
            checksum += static_cast<std::size_t>(node->data);
        } else {
            checksum ^= static_cast<std::size_t>(key);
        }
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Random Access", data.base_keys.size(), config.access_ops,
            static_cast<std::size_t>(tree.size()), seconds, per_op(config.access_ops, seconds),
            checksum ^ hits};
}

template <typename Tree>
BenchResult bench_random_insert(const std::string& tree_name, const BenchConfig& config,
                                const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.base_keys);

    std::size_t checksum = tree.size();
    auto start = std::chrono::steady_clock::now();
    for (int key : data.insert_keys) {
        tree.insert(key);
        checksum += static_cast<std::size_t>(key);
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = data.insert_keys.size();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Random Insert", data.base_keys.size(), ops,
            static_cast<std::size_t>(tree.size()), seconds, per_op(ops, seconds), checksum};
}

template <typename Tree>
BenchResult bench_random_erase(const std::string& tree_name, const BenchConfig& config,
                               const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.erase_initial_keys);

    std::size_t checksum = tree.size();
    std::size_t removed = 0;

    auto start = std::chrono::steady_clock::now();
    for (int key : data.erase_keys) {
        if (tree.remove(key)) {
            ++removed;
            checksum += static_cast<std::size_t>(key);
        } else {
            checksum ^= static_cast<std::size_t>(key);
        }
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Random Erase", data.erase_initial_keys.size(), removed,
            static_cast<std::size_t>(tree.size()), seconds, per_op(removed, seconds), checksum};
}

template <typename Tree>
BenchResult bench_locality_access(const std::string& tree_name, const BenchConfig& config,
                                  const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.base_keys);

    std::size_t checksum = 0;
    std::size_t hits = 0;

    auto start = std::chrono::steady_clock::now();
    for (int key : data.locality_queries) {
        BinNode<int>* node = tree.search(key);
        if (node && node->data == key) {
            ++hits;
            checksum += static_cast<std::size_t>(node->data);
        } else {
            checksum ^= static_cast<std::size_t>(key);
        }
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = data.locality_queries.size();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Locality Access", data.base_keys.size(), ops,
            static_cast<std::size_t>(tree.size()), seconds, per_op(ops, seconds),
            checksum ^ hits};
}

void print_result(const BenchResult& result) {
    std::cout << std::left << std::setw(9) << result.tree << " | "
              << std::setw(13) << result.scenario << " | "
              << "init: " << std::setw(7) << result.initial_size
              << " ops: " << std::setw(7) << result.operations
              << " final: " << std::setw(7) << result.final_size
              << " time(s): " << std::setw(8) << std::fixed << std::setprecision(4) << result.seconds
              << " ns/op: " << std::setw(10) << std::fixed << std::setprecision(2) << result.ns_per_op
              << " checksum: " << result.checksum << '\n';
}

} // namespace

int main(int argc, char** argv) {
    BenchConfig config;
    try {
        config = parse_args(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    Workload workload = make_workload(config);

    std::cout << "BST family benchmark (AVL / RedBlack / Splay)\n"
              << "Initial size: " << config.initial_size
              << ", access ops: " << config.access_ops
              << ", locality ops: " << config.locality_ops
              << ", insert ops: " << config.insert_ops
              << ", erase ops: " << config.erase_ops
              << ", seed: " << config.seed << "\n\n";

    print_result(bench_random_access<AVL<int>>("AVL", config, workload));
    print_result(bench_random_access<RedBlack<int>>("RedBlack", config, workload));
    print_result(bench_random_access<Splay<int>>("Splay", config, workload));
    std::cout << '\n';

    print_result(bench_locality_access<AVL<int>>("AVL", config, workload));
    print_result(bench_locality_access<RedBlack<int>>("RedBlack", config, workload));
    print_result(bench_locality_access<Splay<int>>("Splay", config, workload));
    std::cout << '\n';

    print_result(bench_random_insert<AVL<int>>("AVL", config, workload));
    print_result(bench_random_insert<RedBlack<int>>("RedBlack", config, workload));
    print_result(bench_random_insert<Splay<int>>("Splay", config, workload));
    std::cout << '\n';

    print_result(bench_random_erase<AVL<int>>("AVL", config, workload));
    print_result(bench_random_erase<RedBlack<int>>("RedBlack", config, workload));
    print_result(bench_random_erase<Splay<int>>("Splay", config, workload));

    return 0;
}
