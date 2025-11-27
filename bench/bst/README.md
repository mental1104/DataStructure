# 平衡二叉搜索树性能测试

对比 `AVL`、`RedBlack` 与 `Splay` 三种自平衡 BST 的随机访问（search）、局部性访问、插入与删除性能。基准会预生成一致的工作负载，避免不同树类型之间的随机性偏差；其中“Locality Access” 场景使用小热集合 + burst 模式的高重复访问序列，便于体现 Splay 的自适应优势。

## 构建

```bash
cd build
cmake -DENABLE_BENCHMARKS=ON ..
cmake --build . --target bst_bench
```

可执行文件位于 `build/bench/bst/bst_bench`。

## 通过 ctest 运行

确保配置时开启了 `ENABLE_BENCHMARKS`（默认 ON，且需要 `BUILD_TESTING` 为 ON），然后在构建目录下：

```bash
ctest -R bst_bench -N   # 查看用例
ctest -R bst_bench      # 运行用例
```

## 运行

```bash
./bench/bst/bst_bench \
  --size 30000 \
  --access-ops 60000 \
  --locality-ops 80000 \
  --insert-ops 30000 \
  --erase-ops 30000 \
  --seed 42
```

参数说明：
- `--size`：预填充的元素数量（默认 30000）
- `--access-ops`：随机 search 操作次数（默认 60000，80% 命中、20% 未命中）
- `--locality-ops`：局部性访问次数（默认 80000，小概率跳出工作集制造未命中）
- `--insert-ops`：插入操作次数（默认 30000）
- `--erase-ops`：删除操作次数（默认 30000，预填充量会额外加上该值以避免删空）
- `--seed`：随机数种子（默认 42），保证多次运行对不同树类型复用相同的工作负载

基准输出每个树类型在四种场景下的耗时、ns/op、初始与结束规模及校验和（用于避免编译器优化掉核心循环）。其中局部性访问场景通常能突出 Splay 在高命中/局部访问模式下的伸展优势。

## 样例结果解读（i7-12700H, size=300000, access=1e6, locality=1e6, insert=5e5, erase=5e5, seed=42）

- 随机访问：RedBlack ≈238ns/op 略优于 AVL ≈268ns/op；Splay ≈822ns/op 最慢，常数开销大。
- 局部性访问：Splay ≈11ns/op 最快，AVL/RedBlack ≈13.7ns/op，热集合+burst 场景下伸展收益显现。
- 随机插入：Splay ≈104ns/op 最快；AVL ≈191ns/op，RedBlack ≈197ns/op，Splay 的旋转在追加/插入时摊销成本更低。
- 随机删除：RedBlack ≈893ns/op 最快，其次 AVL ≈970ns/op；Splay ≈2124ns/op 最慢，删除时的双旋与再平衡开销更高。

以上仅为该机器/参数下的观察，实际差异会受硬件、编译器、规模及访问模式影响。***
