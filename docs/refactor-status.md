# 数据结构与算法重构状态

> 审计基线：`refactor-container@56fdd50`。判断标准是仓库中是否已有
> `src/include/dsa/**` 工业实现、共享流程或 iterator-first 算法，而不是仅看旧教学测试是否存在。

## 已完成主要家族

### 数据结构

- Vector、List；
- Stack、Queue 容器适配器；
- Bitmap；
- Hashtable / HashtableB 对应的工业 HashTable；
- BinTree、BST、AVL、RedBlack、BTree；
- Graph、GraphMatrix、GraphList；
- String、Trie、Ternary Search Trie；
- UnionFind 家族；
- 完全二叉堆、左式堆、斜堆、配对堆、斐波那契堆。

### 算法

- Search、Sequence；
- String 基础算法与子串搜索；
- LSD、MSD、Quick3String 字符串排序；
- KMP、BM、KR；
- 已重构容器对应的共享 mutation、遍历、平衡、rehash、heap workflow。

## 尚未完成完整重构

### 数据结构

1. **Splay Tree**
   - 教学实现：`src/tutorials/bst/Splay.h`
   - 欠缺：工业容器、共享伸展流程、生命周期与异常测试。

2. **B+ Tree**
   - 教学实现：`src/tutorials/btree/BPlusTree.h`
   - 欠缺：工业容器、节点所有权、分裂/合并事务和 iterator。

3. **B* Tree**
   - 教学实现：`src/tutorials/btree/BStarTree.h`
   - 当前实现仍偏占位，尚未形成完整可验证实现。

4. **Skiplist / Quadlist**
   - 教学实现：`src/tutorials/ht/Skiplist.h`、`Quadlist.h`、`QuadlistNode.h`
   - 欠缺：工业容器、allocator、随机层级策略抽离和异常安全。

5. **Segment Tree**
   - 教学实现：`src/tutorials/segment/SegmentTree.h`
   - 欠缺：泛型 monoid / operation contract、工业接口和测试矩阵。

6. **HashMap / HashtableChain 旧门面**
   - 教学实现：`src/tutorials/ht/HashMap.h`、`HashtableChain.h`
   - 现有工业 `HashTable` 尚未覆盖这些旧 API 的完整兼容契约。

7. **StringST / Dictionary 抽象门面**
   - 教学实现：`src/tutorials/str/StringST.h`、`src/tutorials/ht/Dictionary.h`
   - 尚未收敛成统一的工业符号表接口。

### 算法

1. **通用排序家族**
   - 教学实现：`src/tutorials/sort/**`
   - 欠缺：`src/include/dsa/algorithm/Sort.h` 的 iterator-first 统一门面，以及多 iterator category 验证。

2. **Suffix Array**
   - 教学实现：`src/tutorials/str/suffix_array.h`
   - 尚未迁入通用字符串算法层。

3. **Fibonacci 动态规划示例**
   - 教学实现：`src/tutorials/dp/Fib.h`
   - 尚未整理为明确的算法 API 与复杂度/边界测试。

4. **Eratosthenes / primeNLT**
   - 教学实现：`src/tutorials/bitmap/Eratosthenes.h`、`src/tutorials/print/primeNLT.h`
   - 尚未迁入通用数论算法层，且 `primeNLT` 仍与打印/工具目录耦合。

## 不计入本轮重构欠账

`demo/**` 中的皇后、迷宫、表达式求值、括号匹配等属于示例程序。
除非后续明确要求抽成库算法，否则不计入容器生命周期重构清单。
