# 教学版源码布局

教学实现统一存放在 `src/tutorials/**`，工业实现继续存放在
`src/include/dsa/**`。

## 兼容策略

- 原 `src/include/<family>/**` 路径保留为轻量转发头；
- 旧代码无需立即修改 include；
- 新教学代码应显式使用 `#include <tutorials/<family>/...>`；
- 安装时同时发布兼容头与 `tutorials/**` 真实实现。

本次迁移覆盖 73 个教学头文件，涉及：

- `bitmap`、`bst`、`bt`、`btree`、`dp`、`entry`；
- `graph`、`ht`、`list`、`pq`、`queue`、`segment`；
- `sort`、`stack`、`str`、`uf`、`utils`、`vector`、`print`。

## 目录职责

- `src/tutorials/**`：保留教材式数据表示、旧公开 API 和演示友好的实现；
- `src/include/dsa/core/**`：教学版与工业版可共享的结构流程；
- `src/include/dsa/algorithm/**`：iterator/value-first 通用算法；
- `src/include/dsa/container/**`：allocator-aware、RAII 的工业实现。
