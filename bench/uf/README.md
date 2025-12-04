# Union-Find Benchmark

对 `QuickFind`、`QuickUnion`、`WeightedQuickUnion`、`WeightedQuickUnionwithCompression` 的性能做对比，二进制：`bench_uf_compare`。

## 场景设计（各自擅长领域）
- `find-heavy`：n=50000，union=0，query=3000000。查询远多于合并，节点访问集中在一小段区间，偏向 QuickFind 的 O(1) 查询优势。
- `union-heavy-random`：n=15000，union=30000，query=5000。随机合并 + 少量查询，考察树形平衡，WeightedQuickUnion（无压缩）预期领先。
- `adversarial-chain`：n=30000，union=29999，query=2000000。顺序链式合并后反复查询叶子与根，压缩版本应明显优于非压缩。
- `star-batch-unions`：n=20000，union=19999，query=200。批量星型合并、查询极少，低常数开销的 QuickUnion 更占便宜。

## 本轮跑分结论
- find-heavy：0 union + 3M queries让 QuickFind 以最低常数拿下（16.9ms）。QuickUnion/Weighted 都是树高≈0或1，find 也是 O(1) 但多了指针追溯/尺寸数组，故略慢；带压缩版多了一次压缩循环，常数最高。若想进一步放大 QuickFind 优势，可以拉大 n/查询量或在同一热点区重复访问，以增加指针追溯的相对开销。
- union-heavy-random：30k union + 少量查询，压缩版最稳（0.598ms），纯按尺寸的 Weighted 紧随其后（0.793ms）。随机合并下树形较平衡，压缩收益有限但开销小；QuickFind/QuickUnion 因 union 代价高（扫描或长链）落后两个数量级。
- adversarial-chain：按链合并使 QuickUnion 跑到 39s 级别（最坏 O(N) find）；按尺寸合并就把树高压到 logN，15ms 级别；压缩版稍慢（23ms）因为链合并时 union 次数多，压缩在每次 find 都做，而 logN 树已足够浅，额外写操作成了开销。如果想让压缩胜出，可增加“先按顺序合并成链，再大量跨远端查询”的比例（当前已如此，但 logN 树的基线太好），或禁用按尺寸平衡。
- star-batch-unions：几乎无查询，合并成星，最优的是常数最小的 QuickUnion（0.147ms），Weighted 版本因维护 size/压缩稍慢，QuickFind 因 union O(N) 最慢。该场景验证了“批量浅层合并 + 少量查询”时低开销实现更优。
- 总体：场景成功区分了四种实现的优势/瓶颈：QuickFind 在纯查询下最好；QuickUnion 易被链化，需浅树才能胜出；按权重合并在随机/对抗 union 下表现稳健；压缩在深树 + 高频查询时才值得，若树本就平衡则会因写放大反而慢。若想进一步细化，可新增“先链化再纯查询（禁 size 平衡）”或“多次重复查询同一路径”的 case 来凸显压缩的收益。

## 运行
在 `build` 目录下执行：

```bash
./bench/uf/bench_uf_compare
```
