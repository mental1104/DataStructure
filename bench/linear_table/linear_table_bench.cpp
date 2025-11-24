#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <stdexcept>

#include "List.h"
#include "Vector.h"

struct BenchConfig {
    std::size_t initial_size = 20000;   // number of elements to prefill
    std::size_t access_ops = 50000;     // random access operations
    std::size_t insert_ops = 20000;     // random insertion operations
    std::size_t erase_ops = 20000;      // random erase operations
    std::size_t append_ops = 20000;     // append to tail operations
    std::size_t traverse_rounds = 3;    // full traversals of the container
    std::size_t front_ops = 20000;      // insert/erase at front
    std::uint32_t seed = 42;            // RNG seed so runs are repeatable
};

struct BenchResult {
    std::string container;
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
              << " [--erase-ops <count>] [--append-ops <count>] [--front-ops <count>]"
              << " [--traverse-rounds <count>] [--seed <value>]\n";
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
        } else if (arg == "--insert-ops") {
            require_value(config.insert_ops);
        } else if (arg == "--erase-ops") {
            require_value(config.erase_ops);
        } else if (arg == "--append-ops") {
            require_value(config.append_ops);
        } else if (arg == "--front-ops") {
            require_value(config.front_ops);
        } else if (arg == "--traverse-rounds") {
            require_value(config.traverse_rounds);
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

Vector<int> build_vector(std::size_t count) {
    Vector<int> data;
    for (std::size_t i = 0; i < count; ++i) {
        data.insert(static_cast<int>(i));
    }
    return data;
}

List<int> build_list(std::size_t count) {
    List<int> data;
    for (std::size_t i = 0; i < count; ++i) {
        data.insertAsLast(static_cast<int>(i));
    }
    return data;
}

BenchResult bench_vector_traverse(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size);
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t round = 0; round < config.traverse_rounds; ++round) {
        for (Rank i = 0; i < data.size(); ++i) {
            checksum += static_cast<std::size_t>(data[i]);
        }
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = static_cast<std::size_t>(data.size()) * config.traverse_rounds;
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Traverse", config.initial_size, ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(ops, seconds), checksum};
}

BenchResult bench_list_traverse(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size);
    std::size_t checksum = 0;
    ListNode<int>* tail = data.end().cur;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t round = 0; round < config.traverse_rounds; ++round) {
        for (ListNode<int>* node = data.first(); node != tail; node = node->succ) {
            checksum += static_cast<std::size_t>(node->data);
        }
    }
    auto end = std::chrono::steady_clock::now();

    const std::size_t ops = static_cast<std::size_t>(data.size()) * config.traverse_rounds;
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Traverse", config.initial_size, ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(ops, seconds), checksum};
}

BenchResult bench_vector_random_access(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size);
    std::mt19937 rng(config.seed);
    std::uniform_int_distribution<std::size_t> dist(0, static_cast<std::size_t>(data.size() - 1));
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.access_ops; ++i) {
        std::size_t idx = dist(rng);
        checksum += static_cast<std::size_t>(data[static_cast<Rank>(idx)]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Random Access", config.initial_size, config.access_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.access_ops, seconds), checksum};
}

BenchResult bench_list_random_access(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size);
    std::mt19937 rng(config.seed);
    std::uniform_int_distribution<std::size_t> dist(0, static_cast<std::size_t>(data.size() - 1));
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.access_ops; ++i) {
        std::size_t idx = dist(rng);
        checksum += static_cast<std::size_t>(data[static_cast<Rank>(idx)]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Random Access", config.initial_size, config.access_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.access_ops, seconds), checksum};
}

BenchResult bench_vector_append(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.append_ops; ++i) {
        Rank pos = static_cast<Rank>(data.size());
        data.insert(pos, static_cast<int>(i));
        checksum += static_cast<std::size_t>(data[pos]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Append", config.initial_size, config.append_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.append_ops, seconds), checksum};
}

BenchResult bench_list_append(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.append_ops; ++i) {
        ListNode<int>* inserted = data.insertAsLast(static_cast<int>(i));
        checksum += static_cast<std::size_t>(inserted->data);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Append", config.initial_size, config.append_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.append_ops, seconds), checksum};
}

BenchResult bench_vector_random_insert(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size);
    std::mt19937 rng(config.seed + 1);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.insert_ops; ++i) {
        std::size_t pos = static_cast<std::size_t>(rng()) % (static_cast<std::size_t>(data.size()) + 1);
        Rank rank = static_cast<Rank>(pos);
        data.insert(rank, static_cast<int>(i));
        checksum += static_cast<std::size_t>(data[rank]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Random Insert", config.initial_size, config.insert_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.insert_ops, seconds), checksum};
}

BenchResult bench_list_random_insert(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size);
    std::mt19937 rng(config.seed + 1);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.insert_ops; ++i) {
        std::size_t pos = static_cast<std::size_t>(rng()) % (static_cast<std::size_t>(data.size()) + 1);
        if (pos == static_cast<std::size_t>(data.size())) {
            ListNode<int>* inserted = data.insertAsLast(static_cast<int>(i));
            checksum += static_cast<std::size_t>(inserted->data);
        } else {
            ListNode<int>* node = data.first();
            for (std::size_t step = 0; step < pos; ++step) {
                node = node->succ;
            }
            ListNode<int>* inserted = data.insertB(node, static_cast<int>(i));
            checksum += static_cast<std::size_t>(inserted->data);
        }
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Random Insert", config.initial_size, config.insert_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.insert_ops, seconds), checksum};
}

BenchResult bench_vector_random_erase(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size + config.erase_ops);
    std::mt19937 rng(config.seed + 2);
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.erase_ops; ++i) {
        std::size_t pos = static_cast<std::size_t>(rng()) % static_cast<std::size_t>(data.size());
        Rank rank = static_cast<Rank>(pos);
        checksum += static_cast<std::size_t>(data.remove(rank));
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Random Erase", config.initial_size + config.erase_ops, config.erase_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.erase_ops, seconds), checksum};
}

BenchResult bench_list_random_erase(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size + config.erase_ops);
    std::mt19937 rng(config.seed + 2);
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.erase_ops; ++i) {
        std::size_t pos = static_cast<std::size_t>(rng()) % static_cast<std::size_t>(data.size());
        ListNode<int>* node = data.first();
        for (std::size_t step = 0; step < pos; ++step) {
            node = node->succ;
        }
        checksum += static_cast<std::size_t>(data.remove(node));
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Random Erase", config.initial_size + config.erase_ops, config.erase_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.erase_ops, seconds), checksum};
}

BenchResult bench_vector_front_insert(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.front_ops; ++i) {
        data.insert(0, static_cast<int>(i));
        checksum += static_cast<std::size_t>(data[0]);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Front Insert", config.initial_size, config.front_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.front_ops, seconds), checksum};
}

BenchResult bench_list_front_insert(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size);
    std::size_t checksum = static_cast<std::size_t>(data.size());

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.front_ops; ++i) {
        ListNode<int>* inserted = data.insertAsFirst(static_cast<int>(i));
        checksum += static_cast<std::size_t>(inserted->data);
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Front Insert", config.initial_size, config.front_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.front_ops, seconds), checksum};
}

BenchResult bench_vector_front_erase(const BenchConfig& config) {
    Vector<int> data = build_vector(config.initial_size + config.front_ops);
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.front_ops; ++i) {
        checksum += static_cast<std::size_t>(data.remove(0));
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"Vector", "Front Erase", config.initial_size + config.front_ops, config.front_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.front_ops, seconds), checksum};
}

BenchResult bench_list_front_erase(const BenchConfig& config) {
    List<int> data = build_list(config.initial_size + config.front_ops);
    std::size_t checksum = 0;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.front_ops; ++i) {
        checksum += static_cast<std::size_t>(data.remove(data.first()));
    }
    auto end = std::chrono::steady_clock::now();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    return {"List", "Front Erase", config.initial_size + config.front_ops, config.front_ops,
            static_cast<std::size_t>(data.size()), seconds, per_op(config.front_ops, seconds), checksum};
}

void print_result(const BenchResult& result) {
    std::cout << std::left << std::setw(8) << result.container << " | "
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

    std::cout << "Linear table benchmark (Vector vs List)\n"
              << "Initial size: " << config.initial_size
              << ", access ops: " << config.access_ops
              << ", insert ops: " << config.insert_ops
              << ", erase ops: " << config.erase_ops
              << ", append ops: " << config.append_ops
              << ", front ops: " << config.front_ops
              << ", traverse rounds: " << config.traverse_rounds
              << ", seed: " << config.seed << "\n\n";

    // 静态场景：遍历、随机访问
    print_result(bench_vector_traverse(config));
    print_result(bench_list_traverse(config));
    print_result(bench_vector_random_access(config));
    print_result(bench_list_random_access(config));
    std::cout << '\n';

    // 动态场景：尾插、随机插入、随机删除
    print_result(bench_vector_append(config));
    print_result(bench_list_append(config));
    print_result(bench_vector_random_insert(config));
    print_result(bench_list_random_insert(config));
    print_result(bench_vector_random_erase(config));
    print_result(bench_list_random_erase(config));
    std::cout << '\n';

    // List 优势场景：已知迭代器位置的前端操作
    print_result(bench_vector_front_insert(config));
    print_result(bench_list_front_insert(config));
    print_result(bench_vector_front_erase(config));
    print_result(bench_list_front_erase(config));

    return 0;
}
