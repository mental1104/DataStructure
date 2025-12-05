# SegmentTree Benchmark

基于 `SegmentTree`（区间求和，点更新）与前缀和/朴素基线，以及具备区间聚合接口的 RedBlack / AVL / Splay / Skiplist / BTree / BStarTree / BPlusTree 的性能测试。

## 场景
- N=50000 个元素（[-10,10] 随机整数），Q=10000 次随机区间查询，U=2000 次点更新。
- 对比对象：
  - Prefix：一次构建前缀和，静态查询 O(1)。
  - Segment：线段树，查询 O(log n)，点更新 O(log n)。
  - Segment mixed：线段树执行 U 次更新，每次后跟一次查询。

## 结果（示例运行）

| Phase               | ms       |
|---------------------|----------|
| Prefix build        | 0.289    |
| Segment build       | 1.301    |
| Prefix queries      | 0.195    |
| Segment queries     | 1.193    |
| Segment mixed (U+Q) | 0.571    |
| RedBlack queries    | 4568.406 |
| AVL queries         | 4528.145 |
| Splay queries       | 17268.382|
| Skiplist queries    | 12000.084|
| BTree queries       | 2219.934 |
| BStarTree queries   | 20898.429|
| BPlusTree queries   | 4319.798 |

Checksums：5985581 / 5985581 / 1139619，trees=5985581（一致），确保结果未被编译器消除。

## 说明
- 说明：BST 系列（RedBlack/AVL/Splay/BTree/BStar/BPlus）和 Skiplist 通过新增区间聚合接口实现，依赖有序遍历累加，时间复杂度 O(k+log n) 但常数巨大；在本规模下查询耗时比线段树慢 3–4 个数量级。
- 线段树：建树 ~1.3ms，纯查询 ~1.2ms，更新+查询 ~0.57ms，适合频繁区间查询与点更新；前缀和在无更新场景最快。
- BTree/BPlusTree 在 B 家族中相对较快，但仍远慢于线段树；BStarTree 和 Skiplist 因额外链接/层数开销最慢。Splay 自调节导致常数更高。

## 运行
```bash
cmake --build build --target bench_segment_tree
./bench/segment/bench_segment_tree
```
