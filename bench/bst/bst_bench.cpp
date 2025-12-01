#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "AVL.h"
#include "RedBlack.h"
#include "Splay.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "BStarTree.h"

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
    std::vector<int> read_heavy_queries;  // read-heavy lookups to highlight AVL height advantage
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

template <typename NodePtr>
bool is_hit(NodePtr node, int key) {
    return node && node->data == key;
}

inline bool is_hit(BTNode<int>* node, int key) {
    if (!node) return false;
    Rank r = node->key.search(key);
    return r >= 0 && r < node->key.size() && node->key[r] == key;
}

inline bool is_hit(const int* value_ptr, int key) {
    return value_ptr && *value_ptr == key;
}

// 轻量包装，使 BPlusTree 适配当前基准的 insert/search/remove 接口
struct BPlusTreeAdapter {
    explicit BPlusTreeAdapter(int order = 64) : tree(order) {}

    bool insert(int key) { return tree.insert(key, key); }
    bool remove(int key) { return tree.remove(key); }
    const int* search(int key) const { return tree.search(key); }
    int size() const { return tree.size(); }

    BPlusTree<int, int> tree;
};

// 轻量包装，使 BStarTree 参与基准
struct BStarTreeAdapter {
    explicit BStarTreeAdapter(int order = 5) : tree(order) {}

    bool insert(int key) { return tree.insert(key); }
    bool remove(int key) { return tree.remove(key); }
    BTNode<int>* search(int key) { return tree.search(key); }
    int size() const { return tree.size(); }

    BStarTree<int> tree;
};

BenchConfig default_config() {
    BenchConfig config;
    config.initial_size = 400000;
    config.access_ops = 800000;
    config.locality_ops = 1200000;
    config.insert_ops = 400000;
    config.erase_ops = 400000;
    config.seed = 42;
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

    // 读密集访问序列：高命中率 + 更长的查询流，凸显 AVL 的较低树高优势
    const std::size_t read_heavy_ops = config.access_ops * 4; // 放大查询次数
    std::bernoulli_distribution read_hit(0.9);
    w.read_heavy_queries.reserve(read_heavy_ops);
    for (std::size_t i = 0; i < read_heavy_ops; ++i) {
        int key = read_hit(rng) ? w.base_keys[hit_idx(rng)] : miss_val(rng);
        w.read_heavy_queries.push_back(key);
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
        auto node = tree.search(key);
        if (is_hit(node, key)) { ++hits; checksum += static_cast<std::size_t>(key); }
        else { checksum ^= static_cast<std::size_t>(key); }
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
        auto node = tree.search(key);
        if (is_hit(node, key)) { ++hits; checksum += static_cast<std::size_t>(key); }
        else { checksum ^= static_cast<std::size_t>(key); }
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = data.locality_queries.size();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Locality Access", data.base_keys.size(), ops,
            static_cast<std::size_t>(tree.size()), seconds, per_op(ops, seconds),
            checksum ^ hits};
}

template <typename Tree>
BenchResult bench_update_heavy(const std::string& tree_name, const BenchConfig& config,
                               const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.base_keys);

    const std::size_t pairs = std::min(data.insert_keys.size(), data.base_keys.size());
    const std::size_t ops = pairs * 2; // insert + erase

    std::size_t checksum = tree.size();
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < pairs; ++i) {
        tree.insert(data.insert_keys[i]);
        checksum += static_cast<std::size_t>(data.insert_keys[i]);
        tree.remove(data.base_keys[i]);
        checksum += static_cast<std::size_t>(data.base_keys[i]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Update Heavy", data.base_keys.size(), ops,
            static_cast<std::size_t>(tree.size()), seconds, per_op(ops, seconds), checksum};
}

template <typename Tree>
BenchResult bench_read_heavy(const std::string& tree_name, const BenchConfig& config,
                             const Workload& data) {
    Tree tree;
    bulk_insert(tree, data.base_keys);

    std::size_t checksum = 0;
    std::size_t hits = 0;

    auto start = std::chrono::steady_clock::now();
    for (int key : data.read_heavy_queries) {
        auto node = tree.search(key);
        if (is_hit(node, key)) { ++hits; checksum += static_cast<std::size_t>(key); }
        else { checksum ^= static_cast<std::size_t>(key); }
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = data.read_heavy_queries.size();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {tree_name, "Read Heavy", data.base_keys.size(), ops,
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

int main() {
    BenchConfig config = default_config();
    Workload workload = make_workload(config);

    std::cout << "BST benchmark (AVL / RedBlack / Splay / BTree)\n"
              << "Initial size: " << config.initial_size
              << ", access ops: " << config.access_ops
              << ", locality ops: " << config.locality_ops
              << ", insert ops: " << config.insert_ops
              << ", erase ops: " << config.erase_ops
              << ", seed: " << config.seed << "\n\n";

    using BenchFn = BenchResult (*)(const std::string&, const BenchConfig&, const Workload&);
    struct Scenario {
        const char* group;
        BenchFn fn;
        const char* tree;
    };

    const std::vector<Scenario> scenarios = {
        // 基础场景先跑，便于观察总体表现
        {"Random Access", bench_random_access<AVL<int>>, "AVL"},
        {"Random Access", bench_random_access<RedBlack<int>>, "RedBlack"},
        {"Random Access", bench_random_access<Splay<int>>, "Splay"},
        {"Random Access", bench_random_access<BTree<int>>, "BTree"},
        {"Random Access", bench_random_access<BPlusTreeAdapter>, "BPlusTree"},
        {"Random Access", bench_random_access<BStarTreeAdapter>, "BStarTree"},

        {"Random Insert", bench_random_insert<AVL<int>>, "AVL"},
        {"Random Insert", bench_random_insert<RedBlack<int>>, "RedBlack"},
        {"Random Insert", bench_random_insert<Splay<int>>, "Splay"},
        {"Random Insert", bench_random_insert<BTree<int>>, "BTree"},
        {"Random Insert", bench_random_insert<BPlusTreeAdapter>, "BPlusTree"},
        {"Random Insert", bench_random_insert<BStarTreeAdapter>, "BStarTree"},

        {"Random Erase", bench_random_erase<AVL<int>>, "AVL"},
        {"Random Erase", bench_random_erase<RedBlack<int>>, "RedBlack"},
        {"Random Erase", bench_random_erase<Splay<int>>, "Splay"},
        {"Random Erase", bench_random_erase<BTree<int>>, "BTree"},
        {"Random Erase", bench_random_erase<BPlusTreeAdapter>, "BPlusTree"},
        {"Random Erase", bench_random_erase<BStarTreeAdapter>, "BStarTree"},

        // 场景用例集中放在后面
        {"Locality Access", bench_locality_access<AVL<int>>, "AVL"},
        {"Locality Access", bench_locality_access<RedBlack<int>>, "RedBlack"},
        {"Locality Access", bench_locality_access<Splay<int>>, "Splay"},
        {"Locality Access", bench_locality_access<BTree<int>>, "BTree"},
        {"Locality Access", bench_locality_access<BPlusTreeAdapter>, "BPlusTree"},
        {"Locality Access", bench_locality_access<BStarTreeAdapter>, "BStarTree"},

        {"Update Heavy", bench_update_heavy<AVL<int>>, "AVL"},
        {"Update Heavy", bench_update_heavy<RedBlack<int>>, "RedBlack"},
        {"Update Heavy", bench_update_heavy<Splay<int>>, "Splay"},
        {"Update Heavy", bench_update_heavy<BTree<int>>, "BTree"},
        {"Update Heavy", bench_update_heavy<BPlusTreeAdapter>, "BPlusTree"},
        {"Update Heavy", bench_update_heavy<BStarTreeAdapter>, "BStarTree"},

        {"Read Heavy", bench_read_heavy<AVL<int>>, "AVL"},
        {"Read Heavy", bench_read_heavy<RedBlack<int>>, "RedBlack"},
        {"Read Heavy", bench_read_heavy<Splay<int>>, "Splay"},
        {"Read Heavy", bench_read_heavy<BTree<int>>, "BTree"},
        {"Read Heavy", bench_read_heavy<BPlusTreeAdapter>, "BPlusTree"},
        {"Read Heavy", bench_read_heavy<BStarTreeAdapter>, "BStarTree"},
    };

    const char* last_group = "";
    for (const auto& s : scenarios) {
        if (std::string(s.group) != last_group) {
            if (*last_group) std::cout << '\n';
            last_group = s.group;
        }
        print_result(s.fn(s.tree, config, workload));
    }

    return 0;
}
