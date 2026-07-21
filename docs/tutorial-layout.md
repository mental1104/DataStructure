# 源码布局

教学实现统一存放在 `src/tutorials/**`，共享流程、通用算法和工业实现统一存放在
`src/dsa/**`。原 `src/include/**` 兼容转发层已移除。

## Include 策略

- 工业与算法代码使用 `#include <dsa/...>`；
- 新教学代码使用 `#include <tutorials/<family>/...>`；
- 构建系统继续把 `src/tutorials` 及其一级子目录加入搜索路径，兼容仓库内既有
  `<vector/Vector.h>`、`<Vector.h>` 等教学 include；
- 安装时分别发布 `include/DSA/dsa/**` 与 `include/DSA/tutorials/**`。

教学目录覆盖：

- `bitmap`、`bst`、`bt`、`btree`、`dp`、`entry`；
- `graph`、`ht`、`list`、`pq`、`queue`、`segment`；
- `sort`、`stack`、`str`、`uf`、`utils`、`vector`、`print`。

## 目录职责

- `src/tutorials/**`：保留教材式数据表示、旧公开 API 和演示友好的实现；
- `src/dsa/core/**`：教学版与工业版可共享的结构流程；
- `src/dsa/algorithm/**`：iterator/value-first 通用算法；
- `src/dsa/container/**`：allocator-aware、RAII 的工业实现。
