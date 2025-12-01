# ds_bench 性能测试

对比 `AVL`、`RedBlack`、`Splay`、`BTree`、`BPlusTree`、`BStarTree`、`QuadraticHT`、`Skiplist` 的性能（HashLinear 目前禁用）。场景：
- **Random Access**：随机查找，命中率 80%。
- **Random Insert / Random Erase**：常规写入/删除。
- **Locality Access**：热集合 + burst 查询，考察局部性。
- **Update Heavy**：交替插入/删除。
- **Read Heavy**：高命中率放大量查询。

参数内置（默认：size=400000、access=800000、locality=1200000、insert=erase=400000、seed=42）。

## 构建

```bash
cd build
cmake -DENABLE_BENCHMARKS=ON ..
cmake --build . --target ds_bench
```

可执行位于 `build/bench/bst/ds_bench`。

## 运行

```bash
./bench/bst/ds_bench
```

## 结果摘要（最新一轮）

- Random Access：BTree/BPlusTree 快（~160–200 ns/op），AVL/RB 次之；Splay/BStarTree/Skiplist/HashQuad 落后（600–1000+ ns/op）。
- Random Insert：Splay 最快（~98 ns/op），AVL/RB/BPlusTree/Skiplist 稳定（170–296 ns/op）；BStarTree 慢（~1.8 µs）；HashQuad 很慢（~5.4 µs）。
- Random Erase：HashQuad 极快（~67 ns/op，删除只清桶）；BPlusTree 次之（~573 ns/op）；AVL/RB ~800–980 ns/op；BStarTree/Skiplist 1400–2000+ ns/op。
- Locality Access：Splay 12 ns/op 最优；AVL/RB 14–15 ns/op；B+/B*/BTree 55–64 ns/op；Skiplist/HashQuad 100–133 ns/op。
- Update Heavy：BPlusTree ~285 ns/op 最优；AVL/RB 350–430 ns/op；Skiplist ~498 ns/op；BTree/BStarTree/HashQuad/Splay 更慢。
- Read Heavy：BTree ~168 ns/op，BPlusTree ~210 ns/op；AVL/RB ~340–360 ns/op；HashQuad ~393 ns/op；BStarTree/Skiplist 680–860 ns/op；Splay 1100+ ns/op。
- Range Scan（顺序写+滑窗点查）：HashQuad最快（~7.8 ns/op，连续桶），RB/BTree/AVL ~28–50 ns/op，BPlus/BStar/Skiplist/Splay 更慢。
- Range Scan/Seq Insert 用例使用全量顺序写，突出写放大/扫描模式差异；新增 Seq Bulk Insert（全量顺序写耗时）。
- Seq Insert + Light Read（顺序写后随机读）：BTree/BPlusTree/HashQuad/AVL/RB 280–520 ns/op，Skiplist/BStar/Splay 900–2600 ns/op（当前实现常数仍高）。
- Seq Bulk Insert（全量顺序写）：Splay 29 ns/op；RB/AVL/HashQuad 47–237 ns/op；BTree/BPlus ~370–430 ns/op；Skiplist ~264 ns/op；BStar ~2.8 µs/op。BStar/Skiplist 仍未展现优势，反映实现常数较大。
- MT Random Access（粗锁，规模小）：HashQuad 164 ns/op 最快；AVL/RB/BPlus ~440–520 ns/op；BStar/Skiplist ~1 µs。
- HashLinear：仍存在极端聚簇导致访问超慢，保持禁用。

## 关键结论
- BTree/BPlusTree：大扇出+连续存储带来低常数，随机访问/读重/混合写中表现突出。
- AVL vs RedBlack：AVL 查询略优，RB 在写密集/更新场景旋转更少；Splay 只在强局部性下具优势。
- BStarTree：当前实现插删常数较高，在本基准中整体落后。
- Hashing/Skiplist：HashQuad 插入/随机访问常数大，删除最快；Skiplist 插入竞争力尚可，但全局随机读写慢。HashLinear 已禁用等待后续优化。

## FAQ
1) HashQuad 随机删除为何最快？  
   线性/二次探测删除只需标记或清空桶，无旋转/重分裂；此基准删除量大（40 万）且表容量较大，探测长度短，所以删除常数极低。

2) 哈希表号称近似 O(1)，为何 HashQuad 随机访问不如 AVL/RB？  
   a) 探测表装填因子高时平均探测步长增长，且二次探测步长不友好缓存。  
   b) 这个实现的 hashCode(k)=k，键分布（打乱后重复探测模式）造成簇，常数被放大。  
   c) 树结构遍历是少量分支跳转且高度 ~19，对现代 CPU 分支预测和缓存更友好。

3) Skiplist/BStarTree“全败”的意义？  
- Skiplist：简单、易并发，适合有序链表增删/按层扩容。建议补充 **顺序写+范围扫描** 场景（例如插入有序键后连续 range query），跳表的链式结构在顺序扫描时 cache 命中稳定，迭代简单。  
- BStarTree：用 2/3 满载策略提升空间利用率，适合对节点分裂成本敏感的存储介质（外存/闪存）。可补一个 **大批量顺序插入后随机少量查询** 的场景，考察其更低分裂频率对写放大的改善。

## 额外场景建议（单线程）
- Skiplist：顺序插入 0..N-1，再执行多次相邻区间的 range 查询（例如长度 100 的滑动窗口），对比树/哈希在顺序扫描时的 cache 行利用与常数开销。
- BStarTree：先顺序批量插入（避免频繁分裂），随后少量随机读查询；观察 2/3 满载策略在“写放大敏感”场景的潜在优势。

新增场景已在 `ds_bench` 中实现：Skiplist Range Scan、BStarTree Seq Insert + Light Read，以及小规模多线程粗锁随机访问（AVL/RB/BPlus/BStar/Skiplist/HashQuad）。

## 原始数据

espeon@Blue-Espeon:~/code/common/cpp/lib/DataStructure/build/bench/bst$ ./ds_bench
Data structure benchmark (AVL / RedBlack / Splay / BTree / BPlusTree / BStarTree / Hash / Skiplist)
Initial size: 400000, access ops: 800000, locality ops: 1200000, insert ops: 400000, erase ops: 400000, seed: 42

AVL       | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.2090   ns/op: 261.22     checksum: 127769855710
RedBlack  | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.2861   ns/op: 357.64     checksum: 127769855710
Splay     | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.9136   ns/op: 1142.06    checksum: 127769855710
BTree     | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.1265   ns/op: 158.18     checksum: 127769855710
BPlusTree | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.1500   ns/op: 187.50     checksum: 127769855710
BStarTree | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.4793   ns/op: 599.14     checksum: 127769855710
HashQuad  | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.6123   ns/op: 765.40     checksum: 127769855710
Skiplist  | Random Access | init: 400000  ops: 800000  final: 400000  time(s): 0.6201   ns/op: 775.12     checksum: 127769855710

AVL       | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0719   ns/op: 179.81     checksum: 800247964979
RedBlack  | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0845   ns/op: 211.17     checksum: 800247964979
Splay     | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.0496   ns/op: 123.92     checksum: 800247964979
BTree     | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.1390   ns/op: 347.57     checksum: 800247964979
BPlusTree | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.1165   ns/op: 291.36     checksum: 800247964979
BStarTree | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.7188   ns/op: 1796.96    checksum: 800247964979
HashQuad  | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 2.5232   ns/op: 6308.11    checksum: 800247964979
Skiplist  | Random Insert | init: 400000  ops: 400000  final: 800000  time(s): 0.1148   ns/op: 287.08     checksum: 800247964979

AVL       | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.3631   ns/op: 907.79     checksum: 159925782211
RedBlack  | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.3085   ns/op: 771.29     checksum: 159925782211
Splay     | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.7832   ns/op: 1958.12    checksum: 159925782211
BTree     | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.4098   ns/op: 1024.44    checksum: 159925782211
BPlusTree | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.2667   ns/op: 666.63     checksum: 159925782211
BStarTree | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 1.0158   ns/op: 2539.52    checksum: 159925782211
HashQuad  | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.0355   ns/op: 88.67      checksum: 159925782211
Skiplist  | Random Erase  | init: 800000  ops: 400000  final: 400000  time(s): 0.6968   ns/op: 1742.09    checksum: 159925782211

AVL       | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0179   ns/op: 14.96      checksum: 252105736683
RedBlack  | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0163   ns/op: 13.57      checksum: 252105736683
Splay     | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0130   ns/op: 10.85      checksum: 252105736683
BTree     | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0680   ns/op: 56.66      checksum: 252105736683
BPlusTree | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0761   ns/op: 63.39      checksum: 252105736683
BStarTree | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.0735   ns/op: 61.24      checksum: 252105736683
HashQuad  | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.1126   ns/op: 93.83      checksum: 252105736683
Skiplist  | Locality Access | init: 400000  ops: 1200000 final: 400000  time(s): 0.1494   ns/op: 124.51     checksum: 252105736683

AVL       | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.4016   ns/op: 501.98     checksum: 880247764979
RedBlack  | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.3068   ns/op: 383.54     checksum: 880247764979
Splay     | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.5643   ns/op: 705.32     checksum: 880247764979
BTree     | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.5317   ns/op: 664.58     checksum: 880247764979
BPlusTree | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.2428   ns/op: 303.47     checksum: 880247764979
BStarTree | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.8722   ns/op: 1090.25    checksum: 880247764979
HashQuad  | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 1.1725   ns/op: 1465.65    checksum: 880247764979
Skiplist  | Update Heavy  | init: 400000  ops: 800000  final: 400000  time(s): 0.4886   ns/op: 610.75     checksum: 880247764979

AVL       | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 1.2600   ns/op: 393.74     checksum: 576065276898
RedBlack  | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 1.2397   ns/op: 387.42     checksum: 576065276898
Splay     | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 3.6675   ns/op: 1146.08    checksum: 576065276898
BTree     | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 0.5480   ns/op: 171.24     checksum: 576065276898
BPlusTree | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 0.6640   ns/op: 207.50     checksum: 576065276898
BStarTree | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 2.3228   ns/op: 725.88     checksum: 576065276898
HashQuad  | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 1.1642   ns/op: 363.81     checksum: 576065276898
Skiplist  | Read Heavy    | init: 400000  ops: 3200000 final: 400000  time(s): 2.7450   ns/op: 857.82     checksum: 576065276898
