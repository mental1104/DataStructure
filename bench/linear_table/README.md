# 线性表性能测试

对比 `src/include/vector/Vector.h` 与 `src/include/list/List.h` 的静态（遍历、随机访问）和动态（尾插、随机插入、随机删除、前端插删）性能，并展示 List 在“已知位置 O(1) 插删”场景下的优势。

## 构建

```bash
cd build
cmake -DENABLE_BENCHMARKS=ON ..
cmake --build . --target linear_table_bench
```

可执行文件位于 `build/bench/linear_table/linear_table_bench`。

## 通过 ctest 运行

配置时开启 `ENABLE_BENCHMARKS`（默认 ON，且需要 `BUILD_TESTING` 处于 ON），然后在构建目录下：

```bash
ctest -R linear_table_bench -N   # 查看用例
ctest -R linear_table_bench      # 运行用例
```

如需调整规模，可直接运行可执行文件并传参（见下）。

## 运行

```bash
./bench/linear_table/linear_table_bench \
  --size 20000 \
  --access-ops 50000 \
  --insert-ops 20000 \
  --erase-ops 20000 \
  --append-ops 20000 \
  --front-ops 20000 \
  --traverse-rounds 3 \
  --seed 42
```

参数说明：
- `--size`：预填充的元素数量（默认 20000）
- `--access-ops`：随机访问操作次数（默认 50000）
- `--insert-ops`：随机插入操作次数（默认 20000）
- `--erase-ops`：随机删除操作次数（默认 20000，预填充量会额外加上该值以避免删空）
- `--append-ops`：尾部插入（push back）次数（默认 20000）
- `--front-ops`：前端插入/删除次数（默认 20000，会在单独的基准中测试 List 的 O(1) 前端插删优势）
- `--traverse-rounds`：完整遍历的轮数（默认 3）
- `--seed`：随机数种子（默认 42）

## 结果解读（默认参数）

- 遍历与随机访问：Vector 连续内存，随机访问 O(1) 显著快于 List 逐节点走查；遍历差距很小。
- 尾插、随机插入、随机删除：两者都是 O(n)（随机位置）时 Vector 受益于缓存局部性，通常更快。
- 前端插删：List 的 `insertAsFirst`/`remove(first)` 是 O(1)，而 Vector 在头部插删需要整体搬移 O(n)；在 `Front Insert/Front Erase` 场景下 List 应优于 Vector（这是 List 的典型优势场景）。***
