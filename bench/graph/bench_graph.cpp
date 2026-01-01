#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <limits>
#include <map>
#include <unordered_set>
#include <vector>
#include "GraphMatrix.h"
#include "GraphList.h"

struct BenchCase {
    std::string name;
    int n;
    int m;
    std::uint32_t seed;
};

struct BenchResult {
    std::string impl;
    std::string algo;
    double ms;
    long rss_kb;
    bool available{true};
};

long get_rss_kb() {
    std::ifstream status("/proc/self/status");
    std::string key;
    long value = 0;
    std::string unit;
    if (status) {
        while (status >> key >> value >> unit) {
            if (key == "VmRSS:") return value; // already in kB
        }
    }
    return 0;
}

using EdgeKey = std::uint64_t;
EdgeKey make_key(int u, int v) { return (static_cast<EdgeKey>(u) << 32) ^ static_cast<EdgeKey>(v); }

std::vector<std::pair<int, int>> gen_edges(int n, int m, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, n - 1);
    std::unordered_set<EdgeKey> seen;
    seen.reserve(static_cast<size_t>(m) * 2);
    std::vector<std::pair<int, int>> edges;
    edges.reserve(m);
    int target = std::max(m, n - 1); // 至少覆盖生成树
    // ensure weak connectivity with a bidirectional chain
    for (int i = 0; i + 1 < n && (int)edges.size() < target; ++i) {
        int u = i, v = i + 1;
        EdgeKey k1 = make_key(u, v);
        EdgeKey k2 = make_key(v, u);
        if (seen.insert(k1).second) edges.emplace_back(u, v);
        if ((int)edges.size() < target && seen.insert(k2).second) edges.emplace_back(v, u);
    }
    // add remaining random edges (bidirectional)
    while ((int)edges.size() < target) {
        int u = dist(rng);
        int v = dist(rng);
        if (u == v) continue;
        EdgeKey k1 = make_key(u, v);
        EdgeKey k2 = make_key(v, u);
        if (seen.insert(k1).second) edges.emplace_back(u, v);
        if ((int)edges.size() < target && seen.insert(k2).second) edges.emplace_back(v, u);
    }
    return edges;
}

template <typename GraphT>
GraphT build_graph(const BenchCase& c, const std::vector<std::pair<int, int>>& edges) {
    GraphT g;
    for (int i = 0; i < c.n; ++i) g.insert(i);
    std::mt19937 rng(c.seed + 1);
    std::uniform_real_distribution<double> wdist(1.0, 100.0);
    for (int idx = 0; idx < (int)edges.size(); ++idx) {
        int u = edges[idx].first;
        int v = edges[idx].second;
        double w = wdist(rng);
        g.insert(static_cast<int>(w), u, v, w);
    }
    return g;
}

template <typename GraphT, typename Fn>
double time_it(GraphT& g, Fn&& fn) {
    (void)g;
    auto start = std::chrono::steady_clock::now();
    fn();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
}

template <typename GraphT>
std::vector<BenchResult> run_algos(const std::string& impl, GraphT& g) {
    std::vector<BenchResult> res;
    auto record = [&](const std::string& algo, double ms, bool available = true) {
        res.push_back({impl, algo, ms, get_rss_kb(), available});
    };
    struct PFSUpdate {
        void operator()(Graph<int,int>* g, int v, int u) {
            if (g->status(u) == VStatus::UNDISCOVERED && g->priority(u) > g->priority(v) + g->weight(v, u)) {
                g->priority(u) = g->priority(v) + g->weight(v, u);
                g->parent(u) = v;
            }
        }
    };
    record("build", 0.0); // placeholder updated by caller if needed
    record("bfs", time_it(g, [&]() { g.bfs(0); }));
    record("dfs", time_it(g, [&]() { g.dfs(0); }));
    record("bcc", time_it(g, [&]() { g.bcc(0); }));
    record("tSort", time_it(g, [&]() { auto s = g.tSort(0); delete s; }));
    record("dijkstra", time_it(g, [&]() { g.dijkstra(0); }));
    record("pfs", time_it(g, [&]() { PFSUpdate upd; g.pfs(0, upd); }));
    record("prim", time_it(g, [&]() { g.prim(0); }));
    record("kruskal", time_it(g, [&]() { g.kruskal(false); }));
    record("connectedComponents", time_it(g, [&]() { g.connectedComponents(false); }));
    record("connected(v,w)", time_it(g, [&]() { g.connectedComponents(0, g.n > 1 ? g.n - 1 : 0); }));
    record("reachableComponents", time_it(g, [&]() { if (g.n > 0) g.reachableComponents(0); }));
    record("kosarajuSCC", time_it(g, [&]() { g.kosarajuSCC(false); }));
    return res;
}

void print_results(const BenchCase& c, const std::vector<BenchResult>& results) {
    std::cout << "\n[Case] " << c.name << " n=" << c.n << " m=" << c.m << "\n";
    // index by algo then impl
    std::map<std::string, std::map<std::string, BenchResult>> table;
    for (const auto& r : results) table[r.algo][r.impl] = r;

    struct CellInfo {
        std::string text;
        long mb;
        double ms;
    };
    auto format = [](const BenchResult* r) {
        if (!r) return CellInfo{"-", 0, 0.0};
        if (!r->available) return CellInfo{"n/a", 0, -1.0};
        std::ostringstream os;
        long mb = static_cast<long>((r->rss_kb + 1023) / 1024); // round up
        os << std::fixed << std::setprecision(3) << r->ms << "ms/" << mb << "MB";
        return CellInfo{os.str(), mb, r->ms};
    };
    auto pad_paint = [](const CellInfo& c, bool ms_win, bool mem_win, int width) {
        std::string ms_part = c.text.substr(0, c.text.find("ms") + 2); // includes "ms"
        std::string mb_part = (c.text == "n/a" || c.text == "-") ? "" : c.text.substr(c.text.find('/') + 1);
        auto paint = [](const std::string& s) { return "\033[1;32m" + s + "\033[0m"; };
        std::string decorated;
        int visible_len = 0;
        if (c.text == "n/a" || c.text == "-") {
            decorated = c.text;
            visible_len = static_cast<int>(decorated.size());
        } else {
            decorated = (ms_win ? paint(ms_part) : ms_part) + "/" + (mem_win ? paint(mb_part) : mb_part);
            visible_len = static_cast<int>(ms_part.size() + 1 + mb_part.size());
        }
        int pad = width - visible_len;
        if (pad < 1) pad = 1;
        return decorated + std::string(pad, ' ');
    };

    const int algo_w = 22;
    const int cell_w = 24;
    std::cout << std::left << std::setw(algo_w) << "Algo"
              << std::setw(cell_w) << "Matrix"
              << std::setw(cell_w) << "List"
              << "\n";
    std::cout << std::string(algo_w + cell_w * 2, '-') << "\n";

    for (const auto& row : table) {
        const BenchResult* m = nullptr;
        const BenchResult* l = nullptr;
        auto itM = row.second.find("Matrix");
        if (itM != row.second.end()) m = &itM->second;
        auto itL = row.second.find("List");
        if (itL != row.second.end()) l = &itL->second;
        CellInfo cm = format(m);
        CellInfo cl = format(l);
        double min_ms = std::numeric_limits<double>::max();
        long min_mb = std::numeric_limits<long>::max();
        if (m && m->available) { min_ms = std::min(min_ms, cm.ms); min_mb = std::min(min_mb, cm.mb); }
        if (l && l->available) { min_ms = std::min(min_ms, cl.ms); min_mb = std::min(min_mb, cl.mb); }
        bool m_ms_win = m && m->available && (cm.ms <= min_ms + 1e-9);
        bool l_ms_win = l && l->available && (cl.ms <= min_ms + 1e-9);
        bool m_mb_win = m && m->available && (cm.mb <= min_mb);
        bool l_mb_win = l && l->available && (cl.mb <= min_mb);
        std::cout << std::left << std::setw(algo_w) << row.first
                  << pad_paint(cm, m_ms_win, m_mb_win, cell_w)
                  << pad_paint(cl, l_ms_win, l_mb_win, cell_w)
                  << "\n";
    }
}

int main() {
    std::vector<BenchCase> cases = {
        {"sparse", 3000, 15000, 42},
        {"dense", 3000, 150000, 43},
        {"medium", 5000, 50000, 44}
    };

    for (const auto& c : cases) {
        auto edges = gen_edges(c.n, c.m, c.seed);

        // GraphMatrix
        auto start = std::chrono::steady_clock::now();
        GraphMatrix<int, int> gm = build_graph<GraphMatrix<int, int>>(c, edges);
        auto end = std::chrono::steady_clock::now();
        double build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
        auto res_matrix = run_algos("Matrix", gm);
        res_matrix[0].ms = build_ms;
        res_matrix[0].rss_kb = get_rss_kb();

        // GraphList
        start = std::chrono::steady_clock::now();
        GraphList<int, int> gl = build_graph<GraphList<int, int>>(c, edges);
        end = std::chrono::steady_clock::now();
        build_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
        auto res_list = run_algos("List", gl);
        res_list[0].ms = build_ms;
        res_list[0].rss_kb = get_rss_kb();

        res_matrix.insert(res_matrix.end(), res_list.begin(), res_list.end());
        print_results(c, res_matrix);
    }
    return 0;
}
