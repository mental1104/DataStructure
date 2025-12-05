# SegmentTree Benchmark

基于 `SegmentTree`（区间求和，点更新）与前缀和/朴素基线的性能测试。

## 场景
- N=200000 个元素（[-10,10] 随机整数），Q=40000 次随机区间查询，U=10000 次点更新。
- 对比对象：
  - Prefix：一次构建前缀和，静态查询 O(1)。
  - Segment：线段树，查询 O(log n)，点更新 O(log n)。
  - Segment mixed：线段树执行 U 次更新，每次后跟一次查询。

## 结果（示例运行）

| Phase               | ms    |
|---------------------|-------|
| Prefix build        | 1.313 |
| Segment build       | 5.667 |
| Prefix queries      | 0.993 |
| Segment queries     | 5.522 |
| Segment mixed (U+Q) | 2.291 |

Checksums：74284539 / 74284539 / 18332824，确保结果未被编译器消除。

## 说明
- RedBlack/Splay/AVL/B-Tree/B+Tree/B*Tree/SkipList 等未列入对比：当前实现不维护区间聚合或幂等查询接口，若以中序遍历计算区间和，时间复杂度为 O(n) 且与线段树场景不具可比性，故未纳入本次基准。若后续为这些结构添加有界时间的区间聚合接口，再补充对比。

## 运行
```bash
cmake --build build --target bench_segment_tree
./bench/segment/bench_segment_tree
```
