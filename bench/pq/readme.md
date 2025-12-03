# 优先队列（堆）基准说明

基准程序：`pq_bench`，可通过命令行参数调整规模：

- `--init N`：初始预填元素数
- `--mixed-ops M`：混合场景的插入/删除操作数（约 2/3 插入，1/3 删除）
- `--meld-heaps K` / `--meld-heap-size S`：大量小堆合并场景的堆数量与单堆大小
- `--heavy-meld-heaps H` / `--heavy-meld-heap-size L`：大型合并场景的堆数量与单堆大小
- `--seed SEED`：随机种子 

输出场景：

- `mixed insert/delete`：普通插入/删除混合
- `drain heap`：纯删除直至清空
- `meld-heavy (merge)`：可合并堆用 `merge`，二叉堆对照用逐元素插入
- `merge latency (per heap)`：以合并次数为操作单位，比较每次合并的延迟
- `large merge (per elem)`：合并大堆，按元素成本统计

## 当前结论

- **BinHeap 全面领先**：在现有实现和工作负载下，完全二叉堆的常数和局部性最好，几乎所有场景都更快。
- **可合并堆未展现理论优势**：Left/Skew/Pairing/Fibonacci 堆的 `merge` 虽然摊还 O(1)，但指针跳转+分配的常数成本抵消了优势；按元素统计时也未反超。
- **缺失 `decrease-key`/`delete` 优化**：Fib/配对堆的理论优势在大量 `decrease-key`，当前代码未实现，基准也未覆盖，因此无优势可见。
- **实现未做性能调优**：未使用内存池、未优化递归/字段，常数开销偏高。

## 后续改进方向

1. 实现 `decrease-key`/`delete`，设计对应基准（大量减键/更新）。
2. 为合并场景做实现优化（减少分配、迭代代替递归）并用更极端的合并规模测试。
3. 内存池/缓存友好布局以降低指针结构的常数开销。
