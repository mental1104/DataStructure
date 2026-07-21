# 数据结构与算法重构状态

> 当前审计基线：`agent/consolidate-tutorial-implementations` 之上的剩余家族重构分支。
> “完成”表示教学 API 仍可回归、可共享流程已下沉、工业实现或通用算法已存在，并有实际测试证据；不以“文件存在”代替完成结论。

## 总体结论

原仓库中已识别的数据结构与算法家族均已完成本轮分层重构：

```text
src/tutorials/**
    教学表示、历史 API、兼容 facade

src/dsa/core/**
    教学版与工业版可共享的结构流程

src/dsa/algorithm/**
    iterator-first / value-first 通用算法

src/dsa/container/**
    RAII、allocator-aware 的工业实现
```

`demo/**` 中的皇后、迷宫、表达式求值和括号匹配仍按示例程序维护，不属于容器生命周期重构范围。

## 已完成的数据结构

| 家族 | 教学版 | 共享层 / 算法层 | 工业版 | 关键语义 |
|---|---|---|---|---|
| Vector / List | `src/tutorials/vector`、`list` | `dsa/core/vector`、`list` | `dsa/container/vector`、`list` | allocator、迭代器、异常回滚 |
| Stack / Queue | `src/tutorials/stack`、`queue` | 复用底层容器 | `dsa/container/stack`、`queue` | 容器适配器语义 |
| Bitmap | `src/tutorials/bitmap` | `dsa/core/bitmap` | `dsa/container/bitmap` | 位存储生命周期与扩容 |
| HashTable | `src/tutorials/ht` | `dsa/core/hash`、`dsa/algorithm/Hash.h` | `dsa/container/hash/HashTable.h` | probing、tombstone、rehash transaction |
| HashMap / HashtableChain | `src/tutorials/ht/HashMap.h`、`HashtableChain.h` | `dsa/algorithm/Hash.h` | `HashMap.h`、`ChainedHashMap.h` | 开放寻址与独立链式门面 |
| Dictionary / StringST | 历史抽象保留 | 通用 hash 与字符串容器 API | `Dictionary.h`、`StringSymbolTable.h` | 统一 `put/get/remove` 门面 |
| BinTree / BST / AVL / RedBlack / BTree | `src/tutorials/bt`、`bst`、`btree` | `dsa/core/tree` | `dsa/container/tree` | 节点所有权、平衡、遍历 |
| Splay Tree | `src/tutorials/bst/Splay.h` | `SplayTreeAlgorithm.h` | `dsa/container/tree/Splay.h` | zig / zig-zig / zig-zag、moved-from 可复用 |
| B+ Tree | `src/tutorials/btree/BPlusTree.h` | `MultiwayTreeAlgorithm.h` | `dsa/container/tree/BPlusTree.h` | 叶链、iterator、事务重建 |
| B* Tree | 教学兼容 facade 保留 | `MultiwayTreeAlgorithm.h` | `dsa/container/tree/BStarTree.h` | 2/3 目标占用率的多路结构 |
| Graph / GraphMatrix / GraphList | `src/tutorials/graph` | `dsa/core/graph` | `dsa/container/graph` | 外置遍历状态、稳定顶点语义 |
| String / Trie / TST | `src/tutorials/str` | `dsa/core/string`、字符串算法层 | `dsa/container/string` | 字符串生命周期和符号表 |
| UnionFind 家族 | `src/tutorials/uf` | `dsa/core/uf` | `dsa/container/uf` | quick-find / union / weighting / compression |
| 完全二叉堆与节点堆家族 | `src/tutorials/pq` | `dsa/core/heap` | `dsa/container/heap` | sift、meld、节点所有权 |
| Skiplist / Quadlist | `src/tutorials/ht` | `dsa/core/skiplist` | `dsa/container/skiplist` | 随机层级、allocator、四向链接 |
| Segment Tree | `src/tutorials/segment` | `dsa/core/segment` | `dsa/container/segment` | 泛型 operation / identity、区间顺序 |

## 已完成的算法

| 算法族 | 通用实现 | 教学兼容 |
|---|---|---|
| Search / Sequence | `dsa/algorithm/Search.h`、`Sequence.h` | 原容器 facade 保留 |
| 通用排序 | `dsa/algorithm/Sort.h` | `src/tutorials/sort/Sort.h` / `SortImpl.h` |
| 字符串排序 | `dsa/algorithm/StringSort.h` | LSD / MSD / Quick3String facade |
| 子串搜索 | `dsa/algorithm/SubstringSearch.h` | KMP / BM / KR facade |
| Suffix Array | `dsa/algorithm/SuffixArray.h` | `src/tutorials/str/suffix_array.h` |
| Fibonacci | `dsa/algorithm/Fibonacci.h` | `src/tutorials/dp/Fib.h` |
| Eratosthenes / primeNLT | `dsa/algorithm/Prime.h` | 原 bitmap / print API 保留 |

## 复杂度与实现边界

- `SplayTree`：查找、插入和删除保持伸展树的均摊 `O(log n)` 目标；单次最坏 `O(n)`。
- `SkipList`：查找、插入和删除为期望 `O(log n)`，最坏 `O(n)`。
- `SegmentTree`：构建 `O(n)`，单点更新和区间查询 `O(log n)`。
- `ChainedHashMap`：平均查找和修改 `O(1)`；rehash 先计算目标桶，再以节点 `splice` 提交。
- 当前工业 `BPlusTree` / `BStarTree` 使用**事务式打包重建**：查找 `O(log n)`，插入和删除 `O(n)`。这是为了先提供稳定节点层次、叶链、iterator、allocator 与 strong guarantee；增量 split / merge 可作为后续性能优化，但不影响本轮 API 与生命周期重构完成度。
- 通用排序算法接受标准迭代器；历史容器 facade 仍保留原有策略限制，避免无意扩大旧 API 行为。

## 验证状态

- GCC 14 / C++11：完整构建，61 个非 benchmark 测试通过。
- GCC 14 / C++17：完整构建，61 个非 benchmark 测试通过；新增对拍后测试数量会由 CI 重新确认。
- Clang 17 / C++20：新增与教学相关目标在 `-Wall -Wextra -Wpedantic -Werror` 下编译通过。
- GCC ASan + UBSan：新增数论、Suffix Array、哈希门面、多路树、SkipList、Splay / Segment / Sort 核心测试通过。
- Clang ASan 在当前容器中无法初始化 libc interceptors，所有最小测试均在进入测试逻辑前崩溃，因此不作为代码失败结论。

最终以 PR 的 GitHub Actions 编译器 × C++ 标准矩阵结果为准。
