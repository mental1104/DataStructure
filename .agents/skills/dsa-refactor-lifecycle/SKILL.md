---
name: dsa-refactor-lifecycle
description: DataStructure 项目专用的数据结构与算法完整重构流程。用于重构教学版容器、树、图、哈希表或算法，抽离可复用算法层，保持教学 API 兼容，新增 STL 风格工业实现，接入现有 Search、Sequence、Sort 等算法，补齐测试并通过 Draft PR 交付。用户提到“按照 Vector 的流程重构”“教学版和工业版复用”“实现仿 STL 数据结构”“抽离 Algorithm.h”时必须使用。
---

# DataStructure 重构生命周期

## 目标

把一个教学数据结构或算法完整推进到以下状态：

```text
现状审计
→ 兼容边界冻结
→ 算法与存储分层
→ 通用算法抽离
→ 教学版接入共享流程
→ 工业版实现
→ 算法生态适配
→ 测试证明
→ 可读性收口
→ Draft PR 交付
```

最终结果必须同时满足：

1. 教学版仍适合阅读和学习；
2. 工业版遵循标准容器或现代 C++ 的生命周期、所有权和复杂度语义；
3. 能够共用的算法只实现一次；
4. 具体存储、allocator、节点所有权和异常回滚由各自实现负责；
5. 所有完成结论均有仓库代码或测试证据。

# 项目固定规则

本 Skill 仅用于：

```text
https://github.com/mental1104/DataStructure
```

默认基线分支：

```text
refactor-container
```

必须遵守：

- 不自动修改或合入 `main`；
- 不自动合并任何 PR；
- 最终由用户手动审核和合并；
- 未获明确授权时，不 force-push、不 squash、不删除已有 commit；
- 已经存在的教学实现不得为了新增工业版而直接删除；
- 默认创建 Draft PR；
- commit 级跳过 CI 时使用标准 `[skip ci]`、`[ci skip]` 等格式；
- 仓库代码和真实测试是事实来源，聊天记录只用于理解目标。

## 工业实现的仓库内依赖闭环

以下规则适用于生产实现目录：

```text
src/dsa/container/
src/dsa/core/
src/dsa/algorithm/
```

### 容器依赖规则

- 生产实现禁止使用 STL 容器作为成员存储、返回结果、算法临时缓冲或内部工作队列；
- 禁止包含 `<vector>`、`<list>`、`<map>`、`<queue>`、`<set>`、`<array>` 等 STL 容器头文件；
- 数据结构依赖线性容器时，必须优先使用仓库中的 `dsa::container::Vector`、`List`、`Queue`、`Stack`、`BinaryHeap` 等实现；
- 不得为了替换 STL 容器制造循环依赖。基础容器只能依赖更底层的 allocator、迭代器和共享流程，高层结构与算法可以依赖基础容器；
- `std::allocator`、`allocator_traits`、智能指针、`pair`、`tuple`、迭代器 traits、类型 traits、函数对象、异常、`move`、`forward` 和 `swap` 不属于本条禁止范围；
- `std::basic_string` 只允许作为文本值或外部兼容输入，不得作为已有仓库字符串容器能够承担的内部工作存储。

### 泛型算法依赖规则

使用任何 `std::<algorithm>` 前，必须先搜索：

```text
src/dsa/algorithm/
src/dsa/core/
```

处理顺序：

1. 仓库已有对等算法时，必须复用仓库实现；
2. 仓库缺少、但该算法可脱离具体容器复用时，应先新增 iterator-first 的仓库算法，再由调用方复用；
3. 不得让仓库的算法策略枚举表面存在，实际却委托给 `std::sort`、`std::stable_sort`、`std::make_heap` 等标准算法；
4. 只有仓库不存在对等能力、且新增通用实现没有明确教学或工程价值时，才可保留标准算法，并在代码审阅中说明原因。

### 测试与 Benchmark 边界

- `test/` 和 `bench/` 可以使用 STL 容器或算法作为输入构造、差分 oracle、性能基线和预期结果；
- 测试中的 STL 使用不得渗透为生产 API 或生产实现依赖；
- 新增或修改生产代码后必须执行：

```bash
python3 tools/check_dsa_stl_dependencies.py
cmake --build build --target check_dsa_stl_dependencies
```

静态门禁失败时，不得创建或更新交付 PR。

# 什么时候触发

以下请求必须使用本 Skill：

- “重构这个数据结构”；
- “按照 Vector 的流程重构 List / Tree / Graph”；
- “实现工业版仿 STL 容器”；
- “教学版和工业版共用算法”；
- “抽离 `<Component>Algorithm.h`”；
- “把算法改成 iterator-first”；
- “保持原 API 不变，新增工业实现”；
- “完成一轮数据结构工业化”；
- “给这个结构补齐 allocator、移动语义、异常安全和测试”。

用户只要求某一个阶段时，可以只执行该阶段，但仍要遵守项目固定规则和证据要求。

# 架构分层

每次重构必须先把代码分到以下四层。

## 第一层：跨数据结构通用算法

只依赖迭代器、值、比较器或函数对象的算法，应放在：

```text
src/dsa/algorithm/
```

典型形态：

```cpp
template<typename Iterator, typename Compare>
Result algorithm(Iterator first, Iterator last, Compare compare);
```

要求：

- 原则上不包含具体业务容器头文件；必须拥有动态结果或临时缓冲时，只能依赖仓库基础容器，并保证不存在反向循环依赖；
- 不直接访问 `_elem`、`_root`、`_size` 等字段；
- 不负责修改容器的 size、capacity 或节点所有权；
- 返回迭代器、计数或值，让容器自己提交状态；
- 明确迭代器类别、稳定性和复杂度要求。

适用示例：

- 查找与边界搜索；
- 去重、遍历和聚合；
- 基于迭代器的排序；
- 不依赖容器表示的序列算法。

## 第二层：数据结构专属共享流程

教学版和工业版步骤相同，但存储操作不同的流程，应放到单一文件：

```text
src/dsa/core/<component>/<Component>Algorithm.h
```

例如：

- Vector 的打开空隙、写入、回滚、提交 size；
- Tree 的节点链接、断开和元数据更新；
- Hash 的探测、插入、删除和 rehash 状态机；
- Heap 的 sift-up / sift-down；
- Graph 的遍历状态推进。

共享层只能依赖语义操作，不得依赖具体表示。

推荐 policy contract：

```text
查询当前状态
准备资源
执行结构变化
写入值或节点
异常回滚
提交新状态
执行实现专属后处理
```

不要为了泛化暴露：

```text
dataPtr()
sizeRef()
capacityRef()
rootRef()
```

优先提供语义明确的操作：

```text
ensureCapacity
openGap
commitSize
linkChild
replaceRoot
rollbackNode
updateMetadata
```

一个数据结构通常只维护一个主 `<Component>Algorithm.h`。不要无意义拆成大量 `Component*.h`。

## 第三层：教学版适配

教学版负责：

- 保留既有类名、文件路径和公开 API；
- 保留下游继承所依赖的 protected 成员和 helper；
- 保留教学友好的数据表示；
- 用一个轻量 adapter 接入共享流程；
- 继续承担教学版特有策略。

例如 Vector 教学版可以继续：

- 使用 `new T[]`；
- 让 capacity 区间内对象全部默认构造；
- 删除后按原策略自动 shrink。

这些行为不能被错误地强加给工业版。

## 第四层：工业版实现

工业版负责：

- RAII 和 allocator；
- 对象构造、析构和未初始化存储；
- 拷贝与移动语义；
- 迭代器和失效规则；
- 异常保证；
- 时间与空间复杂度；
- 标准风格 API；
- move-only、非默认构造和 throwing 类型。

工业版复用前两层算法，但不得继承教学版来复用不兼容的存储模型。

正确依赖方向：

```text
通用算法层
      ↑
组件共享流程层
      ↑          ↑
教学版      工业版
```

禁止：

```text
共享算法层 → include 具体工业容器
共享算法层 → 直接访问教学版内部字段
工业版     → 复制一整份教学版 mutation 流程
工业版     → 继承教学版只为复用代码
```

# 完整执行流程

## 阶段 0：确认版本、分支和范围

开始前确认：

- 目标数据结构或算法；
- 当前基线分支；
- 是否已有相关 PR；
- 是否为堆叠 PR；
- 当前 head 和 commit 关系；
- 用户是否允许改写历史；
- 本次是完整生命周期还是单阶段任务。

退出门禁：

- 目标文件、基线和交付分支已确定；
- 不会误改 `main`；
- 不会误删或重写现有 commit。

## 阶段 1：审计教学实现

必须阅读：

- 类声明和完整实现；
- 构造与析构；
- public / protected / private API；
- 主要成员状态；
- 插入、删除、查找、遍历等核心流程；
- 扩容、缩容、平衡、rehash 或元数据维护；
- 下游继承类和调用方；
- 测试文件及其 CMake 注册方式。

形成内部审计表：

| 维度 | 当前行为 | 必须兼容 | 工业目标 |
|---|---|---|---|
| API | 当前签名 | 是/否 | 标准风格接口 |
| 表示 | 教学存储 | 通常保留 | RAII/allocator |
| 生命周期 | 当前模型 | 教学版保留 | 精确构造析构 |
| 核心流程 | 当前实现 | 抽离候选 | 共享 workflow |
| 算法 | 容器耦合程度 | 改造候选 | iterator-first |
| 复杂度 | 当前特征 | 保持或改进 | 明确保证 |
| 测试 | 当前覆盖 | 回归契约 | 扩展矩阵 |

退出门禁：

- 已明确哪些是通用算法；
- 哪些是组件共享流程；
- 哪些必须留在教学版或工业版。

## 阶段 2：冻结兼容边界和不变量

记录：

- 公开 API；
- protected 成员和继承依赖；
- 返回值和迭代器行为；
- 空状态表示；
- size/capacity 或父子关系不变量；
- 删除和清理行为；
- moved-from 状态；
- 自动 shrink、平衡或 rehash 策略；
- 复杂度目标；
- 异常后的最低保证。

每个核心操作都要能回答：

```text
前置条件是什么？
读取什么状态？
修改什么状态？
何时提交？
失败时清理什么？
成功后建立什么不变量？
复杂度是多少？
```

退出门禁：

- 所有有意改变的行为已明确列出；
- 未经批准，不修改教学版外部契约。

## 阶段 3：分类每个重要函数

每个函数必须归入一类：

| 类型 | 去向 |
|---|---|
| iterator/value-only 算法 | `dsa/algorithm` |
| 组件专属共享状态机 | `<Component>Algorithm.h` |
| 教学存储与生命周期 | 教学实现 |
| 工业存储与生命周期 | 工业实现 |
| API 转发 | 对应 facade |
| 测试 instrumentation | 测试文件 |

发现教学版和工业版重复逻辑时，依次判断：

1. 两边步骤是否相同？
2. 是否只有存储原语不同？
3. 能否用小型 policy contract 表达？
4. 抽离后是否仍不依赖任何具体容器？
5. 是否不会破坏异常和生命周期保证？

前四项为是且第五项安全时，不允许保留两份完整实现。

## 阶段 4：抽离共享算法

### 跨组件算法

优先 iterator-first，并至少用两种不同迭代器或容器验证。

### 组件共享流程

只抽离稳定步骤和提交顺序。

对于多阶段 mutation，明确：

- 已申请但未提交的资源；
- 已构造对象数量；
- 哪一步可能抛异常；
- 回滚责任归谁；
- commit point 在哪里；
- 失败后提供 strong 还是 basic guarantee。

退出门禁：

- 共享层可用至少两个真实或 mock adapter 编译；
- 共享层没有泄漏具体存储表示。

## 阶段 5：教学版接入

要求：

- 不改变原 include 路径；
- 不改变原 public/protected 契约；
- 用 adapter 调用共享流程；
- 教学版特有策略保留在 adapter；
- 原有测试先保持不变；
- 编译所有下游继承结构。

退出门禁：

- 教学版回归测试通过；
- 下游结构编译通过；
- 教学版未引入 allocator 等工业细节。

## 阶段 6：实现工业版

对应标准容器存在时，以 `std::*` 语义为参考。

按适用情况实现：

- 默认、count/value、range 和 initializer-list 构造；
- 析构；
- 深拷贝构造与赋值；
- 移动构造与赋值；
- allocator-aware 构造和 propagation；
- begin/end、const 和 reverse iterator；
- 元素访问；
- reserve/resize/shrink 等容量接口；
- insert/emplace/erase/push/pop；
- comparison 和 swap；
- 明确标注非标准扩展。

必须考虑：

- move-only；
- 非默认构造；
- throwing copy/move construction；
- throwing assignment；
- stateful allocator；
- erase/clear 后立即析构；
- moved-from 合法空状态；
- iterator invalidation；
- 均摊复杂度和最坏复杂度。

扩容、克隆或 rehash 必须采用 transaction-like 流程：

```text
申请新资源
→ 构造或迁移
→ 失败时清理新资源
→ 全部成功后提交
→ 清理旧资源
```

退出门禁：

- 工业版真实调用共享算法层；
- 没有复制教学版完整流程；
- 生命周期和异常边界有测试计划。

## 阶段 7：接入仓库算法生态

检查新结构能否适配：

- `Search.h`；
- `Sequence.h`；
- `Sort.h`；
- traversal 或其他现有算法门面。

优先新增泛型 iterator overload，而不是为工业容器复制一套算法。

检查：

- const 和非 const iterator；
- iterator category；
- 算法是否意外要求可复制或默认构造；
- 返回 iterator 是否能继续交给 erase/insert；
- 原教学 facade 是否仍可用。

退出门禁：

- 至少一个真实仓库算法族已在工业版上验证；
- 没有新增容器专属算法副本。

## 阶段 8：验证矩阵

### 教学版回归

- 原测试；
- 下游继承结构；
- 原 demo 或聚合目标。

### 工业版核心行为

- 构造与析构；
- 拷贝独立性；
- 移动所有权转移；
- 随机访问和越界；
- 插入、删除、clear、resize、swap；
- iterator 遍历；
- 空和单元素边界。

### 类型矩阵

- 基础类型；
- 普通可拷贝类型；
- move-only、非默认构造类型；
- 生命周期计数类型；
- 抛异常类型；
- stateful allocator 或所有权策略。

### 算法集成

- 查找；
- Sequence 变换；
- 排序或遍历；
- 算法返回 iterator 后继续容器 mutation。

### 复杂度和资源

- 扩容次数；
- 是否发生不应有的自动缩容；
- 单操作是否出现意外分配；
- 随机长序列。

### Differential Testing

存在标准对应物时，与以下之一对拍：

```text
std::vector
std::list
std::map
std::unordered_map
```

只比较共同语义，不把项目扩展误当标准 API。

### 工具矩阵

按环境能力运行：

- CMake / CTest / GTest；
- GCC 和 Clang；
- 项目支持的多个 C++ 标准；
- `-Wall -Wextra -Wpedantic -Werror`；
- AddressSanitizer；
- UndefinedBehaviorSanitizer；
- leak detection；
- 满足触发条件时的 GitHub Actions。

禁止把“编译通过”表述为“行为测试通过”。

退出门禁：

- 失败测试已解决或明确记录；
- 未运行的重要验证必须说明原因；
- 不得声称未经执行的检查已经通过。

## 阶段 9：可读性收口

大型模板类默认采用：

```text
类定义：只保留类型、成员和函数声明
→ 类定义结束
→ 按职责依次编写类外模板实现
→ 非成员函数实现
```

默认把实现保留在同一个主 `.h` 文件中。

只有仓库已有明确规范或用户指定时，才拆 `.inl` / `.tpp`。

实现顺序建议：

1. adapter / storage policy；
2. 构造、析构、拷贝与移动；
3. 元素访问和 iterator；
4. 容量和 modifiers；
5. private helpers；
6. non-member operators。

所有本次新增或实质修改的类和函数都要有中文职责注释。

注释重点：

- 所有权；
- 生命周期；
- 异常回滚；
- iterator 失效；
- 复杂度；
- 与标准行为的差异。

不要写只复述语法的注释。

退出门禁：

- 类内没有大段实现；
- 没有无意义的多文件碎片；
- 注释覆盖所有新增或实质修改函数；
- dependency direction 仍然单向。

## 阶段 10：Draft PR 交付

发布前检查完整 `base...head`，不要只看最新 commit。

PR 必须说明：

- 实现内容；
- 分层结构；
- 教学版保留内容；
- 工业版新增语义；
- 共享了哪些算法；
- 测试和实际结果；
- 兼容性；
- 风险与非目标；
- stacked PR 的合并顺序。

交付规则：

- 默认 Draft；
- 不自动合并；
- 不自动关闭独立工作线的 PR；
- 后续 PR 完整包含前置 PR 时，明确哪些旧 PR 可在合并后关闭；
- 报告 base、head、commit、changed files 和 CI 状态。

# 结构专项检查

## 连续容器

重点审计：

- raw storage 和 constructed range；
- 几何扩容；
- gap opening/closing；
- 插入值与容器元素 aliasing；
- iterator invalidation；
- 显式或自动 shrink。

## 链式容器

重点审计：

- sentinel；
- node allocator rebind；
- 链接前构造、失败清理、成功提交；
- iterator stability；
- splice/merge 所有权边界。

## 树

重点审计：

- parent/child 一致性；
- 节点所有权和 allocator；
- subtree clone 回滚；
- subtree destruction；
- height、color、rank 等元数据；
- 通用结构流程与平衡策略边界；
- CRTP 只有在确需静态扩展时使用。

## 哈希表

重点审计：

- bucket 状态；
- probing 或 chaining；
- tombstone；
- rehash transaction；
- load factor；
- iterator 和 rehash 失效行为。

## 图

重点审计：

- vertex/edge 所有权；
- adjacency 表示；
- 稳定 ID；
- directed/undirected 不变量；
- 遍历状态尽可能外置；
- duplicate edge 和 removal 语义。

## 纯算法

不需要工业容器时：

- 抽成 iterator-first 或 range-first；
- 保留教学 facade；
- 用多种 iterator 类型验证；
- 明确稳定性、比较器和复杂度；
- 不依赖某一个仓库容器。

# 必须避免的反模式

## 教学版和工业版各复制一套核心算法

当高层步骤一致时，应使用共享 workflow 和两个 adapter。

## 伪泛型接口

```text
algorithm(dataPtr, sizeRef, capacityRef)
```

这只是把 private 表示暴露给算法层，应改成语义操作。

## 共享算法直接管理 allocator

共享层可以编排 construct/destroy，但实际 allocator 和清理责任属于工业 adapter。

## 工业版继承教学版

共享算法，不共享不兼容的数据表示。

## 过度拆分头文件

函数多不代表必须拆很多 `.inl` 或 `Component*.h`。先使用“类内声明、类外实现”的单主头文件布局。

## 测试文件未注册

新增测试文件不等于测试可执行。必须检查 CMake/CTest/GTest 注册并运行准确目标。

## 未编译下游却声称兼容

涉及 protected 字段、继承或模板实例时，必须编译下游结构。

## 使用 STL 风格名称但不提供对应语义

若接口命名对齐 `std::*`，必须同步审计生命周期、iterator、allocator、异常和复杂度；所有有意偏差必须记录。

# 进度更新要求

长任务在以下节点向用户报告实质进展：

1. 完成教学版和兼容面审计后；
2. 确定共享层边界后；
3. 教学版和工业版能同时编译后；
4. 测试发现关键缺陷或证明关键保证后；
5. 创建 PR 前。

不要只报告低层工具调用。

# 最终报告要求

最终回复必须包含：

- 仓库和 PR；
- base/head；
- 是否保留 commit 历史；
- 最终文件和分层结构；
- 共用算法；
- 教学版专属行为；
- 工业版专属行为；
- 已实现 API 和非目标；
- 实际执行的测试及结果；
- CI 状态；
- stacked PR 合并顺序；
- 剩余风险和未验证边界。

# 完成清单

只有所有适用项满足时，才可以称为完成：

- [ ] 已确认仓库版本、基线和目标分支；
- [ ] 已审计教学 API 和下游依赖；
- [ ] 已冻结兼容边界和不变量；
- [ ] 可泛化算法已 iterator/value 化；
- [ ] 必要时只存在一个组件共享 Algorithm 文件；
- [ ] 教学版已通过 adapter 使用共享流程；
- [ ] 工业版已通过 adapter 使用共享流程；
- [ ] 存储和对象生命周期仍由各实现负责；
- [ ] 已测试工业版构造、析构、拷贝和移动；
- [ ] 已测试 move-only 和非默认构造类型；
- [ ] 已测试适用的异常回滚路径；
- [ ] 已适配至少一个真实仓库算法族；
- [ ] 原教学测试保持有效；
- [ ] 下游继承结构已编译；
- [ ] 类内声明、类外实现布局已收口；
- [ ] 所有新增或实质修改函数有中文注释；
- [ ] 已检查完整 PR diff；
- [ ] 所有测试结论都有证据；
- [ ] PR 保持 Draft，除非用户明确要求 Ready；
- [ ] 未自动合并任何分支。

不适用的项目必须说明原因，不能静默跳过。