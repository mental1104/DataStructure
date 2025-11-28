# 平衡二叉搜索树性能测试

对比 `AVL`、`RedBlack`、`Splay`、`BTree` 的性能，场景集中输出：
- **Random Access**：随机查找，RedBlack/BTree 常数因子通常更小。
- **Random Insert / Random Erase**：常规写入/删除表现。
- **Locality Access**：热集合 + burst 查询，Splay 适合高局部性；也可观察树高差异。
- **Update Heavy**：交替插入/删除，验证红黑树在写密集场景下旋转更少。
- **Read Heavy**：构建后高命中率的放大查询，验证 AVL 较低树高的查询优势。

所有运行参数已内置，不需要在命令行或 CMake 中传递（默认：size=400000、access=800000、locality=1200000、insert=400000、erase=400000、seed=42）。

## 构建

```bash
cd build
cmake -DENABLE_BENCHMARKS=ON ..
cmake --build . --target bst_bench
```

可执行文件位于 `build/bench/bst/bst_bench`。

## 通过 ctest 运行

```bash
cd build
ctest -R bst_bench -N   # 查看用例
ctest -R bst_bench      # 运行用例
```

## 运行

```bash
./bench/bst/bst_bench
```

27: Test timeout computed to be: 1500
27: BST benchmark (AVL / RedBlack / Splay / BTree)
27: Initial size: 400000, access ops: 800000, locality ops: 1200000, insert ops: 400000, erase ops: 400000, seed: 42
27: 
27: AVL       | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.2864   ns/op: 357.97     checksum: 127769855710
27: RedBlack  | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.2879   ns/op: 359.93     checksum: 127769855710
27: Splay     | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.6154   ns/op: 769.21     checksum: 127769855710
27: BTree     | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.0810   ns/op: 101.24     checksum: 127769855710
27: 
27: AVL       | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0329   ns/op: 82.22      checksum: 800247964979
27: RedBlack  | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0620   ns/op: 155.09     checksum: 800247964979
27: Splay     | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0484   ns/op: 121.06     checksum: 800247964979
27: BTree     | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0317   ns/op: 79.37      checksum: 800247964979
27: 
27: AVL       | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.3723   ns/op: 930.84     checksum: 159925782211
27: RedBlack  | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.3267   ns/op: 816.73     checksum: 159925782211
27: Splay     | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.6426   ns/op: 1606.50    checksum: 159925782211
27: BTree     | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.1347   ns/op: 336.74     checksum: 159925782211
27: 
27: AVL       | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0048   ns/op: 4.04       checksum: 252105736683
27: RedBlack  | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0042   ns/op: 3.51       checksum: 252105736683
27: Splay     | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0027   ns/op: 2.26       checksum: 252105736683
27: BTree     | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0249   ns/op: 20.77      checksum: 252105736683
27: 
27: AVL       | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.3944   ns/op: 492.99     checksum: 880247764979
27: RedBlack  | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.2995   ns/op: 374.34     checksum: 880247764979
27: Splay     | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.4089   ns/op: 511.11     checksum: 880247764979
27: BTree     | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.1389   ns/op: 173.68     checksum: 880247764979
27: 
27: AVL       | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 1.1264   ns/op: 352.00     checksum: 576065276898
27: RedBlack  | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 1.3989   ns/op: 437.15     checksum: 576065276898
27: Splay     | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 2.6856   ns/op: 839.24     checksum: 576065276898
27: BTree     | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 0.3727   ns/op: 116.48     checksum: 576065276898
1/1 Test #27: bst_bench ........................   Passed   18.55 sec


基准会输出每个场景下 AVL / RedBlack / Splay / BTree 的耗时、ns/op、初始与结束规模及校验和（用于防止核心循环被优化掉）。

## 结果解读（默认参数：size=400000、access=800000、locality=1200000、insert=erase=400000、seed=42）

- **Random Access**：BTree 最快（~101 ns/op）；AVL≈RB（358~360 ns/op），Splay 最慢。
- **Random Insert**：BTree 和 AVL 领先（~79~82 ns/op），Splay 次之，RB 最慢。
- **Random Erase**：BTree 仍领先（~337 ns/op），RB 优于 AVL，Splay 最慢。
- **Locality Access**：Splay 在高局部性下显著最快（~2.26 ns/op）；RB 略快于 AVL；BTree 在此场景落后。
- **Update Heavy（交替插/删）**：RB 优于 AVL（~374 vs ~493 ns/op）；Splay 接近 AVL；BTree 以块结构优势最优。
- **Read Heavy（高命中放大量查询）**：BTree 明显最快（~116 ns/op）；AVL 优于 RB（352 vs 437 ns/op），Splay 最慢。

结论小结：
- 写密集（Update Heavy/Erase）：红黑树比 AVL 更省旋转，BTree 由于块存储常数因子最小。
- 读密集（Read Heavy/Locality）：AVL 树高更低优于 RB；Splay 在强局部性场景独占优势，但全局随机场景最慢。
- 随机查找/插入：BTree 常数因子最小；AVL 与 RB 接近，Splay 仍有额外伸展开销。

### BTree 在本内存基准中为何最快
- **扇出极大、树高极低**：默认 `_order=512`，40 万元素高度约 3～4 层；红黑/AVL 高度约 19 层，指针跳转次数减少一个数量级。
- **节点内连续存储**：`BTNode` 的 `key/child` 放在连续数组，随机访问基本落在 1～2 个 cache line；二叉树每层一次指针解引用，cache miss 和分支失败更多。
- **批量分裂/合并替代频繁旋转**：插入/删除在节点内搬移后才分裂/合并，大扇出让平衡操作触发概率低；相对二叉树每次更新都可能旋转，BTree 常数更小。
- **局部性场景天然契合**：Locality Access 的热集合往往集中在少量节点，命中可以单节点完成扫描；二叉树即使命中也要多层跳转。

### 为何通用容器仍偏好红黑树而非 BTree
- **实现与维护复杂度**：多关键字节点需要管理容量、分裂/合并策略、对齐和异常安全，远比红黑树的简单旋转复杂。
- **迭代器/指针稳定性**：BTree 分裂/合并会搬移大量元素，难以提供 `std::map` 那样的强迭代器稳定性；红黑树节点位置稳定，易于满足标准要求。
- **常规模模态下的常数**：中小规模、通用负载下，节点内线性扫描/搬移的成本未必优于小节点红黑树；后者已经足够快且代码路径短。
- **通用性 vs 定制优化**：BTree/B+Tree 的长板在页/缓存/NUMA/外存场景（数据库、KV 引擎、文件系统）。标准库追求可移植和简单实现，因此采用红黑树；面向缓存优化的 BTree 容器通常以专用库存在（如 Abseil btree、Boost flat_*）。***
