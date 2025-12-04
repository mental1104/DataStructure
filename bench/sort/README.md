# 排序基准说明

## 运行方式
- 构建后执行 `./sort_bench`（默认在 `build/bench/sort/`），会依次跑三块数据：
  - **随机数据块**：包含所有算法，冒泡/选择/插入用小规模，其他用大规模。
  - **近乎有序数据块**：插入排序规模提升到大规模，其余保持不变；为节省时间，不再运行冒泡/选择。
  - **大量重复值数据块**：值域压缩到 10 个不同值，大量重复，凸显 Quick3way 的优势；从这一块开始，插入排序退出不再参与。
- 可用 `--shape random|almost|dups|both|all|random,almost,dups` 覆盖默认形态；`--perturb r` 调整近乎有序的扰动比例（默认 0.1% 相邻交换）；`--seed` 控制随机种子。
- 输出顺序：先跑完所有 `Vector` 基准，再跑所有 `List` 基准。列表用例规模更小（默认 list_n2=2k, list_nlogn=50k，可用 `--ln2`/`--lnlogn` 调整，避免大规模 List/Radix 拉长时间），算法包含 L-Selection、L-Insertion（第三块起退出）、L-MergeTD、L-Radix。

## 关键实现点
- 近乎有序输入构造：先生成递增数组，再随机做少量相邻交换，体现插入排序接近线性时间的优势。
- 快速排序族添加随机 pivot，避免在近乎有序数据上退化成 O(n²) 导致“卡死”。
- 冒泡/选择只跑第一块，避免大规模数据拖慢全局测试。
- 堆排序在构建堆阶段修正为从最后一个父节点开始下滤，避免第二块（近乎有序）出现未排序的 `no` 结果。
- 第三块起插入排序不再参与；大量重复值场景有利于三路快排（Quick3way）。

## 观察结论（表格化）

### Vector（n2=20k, nlogn=2,000,000, seed=42）
**随机**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| QuickBF   | 0.2345  | yes |
| Quick     | 0.2520  | yes |
| MergeBU   | 0.2930  | yes |
| MergeTD   | 0.2999  | yes |
| Shell     | 0.7773  | yes |
| Heap      | 0.5011  | yes |
| Quick3way | 0.3533  | yes |
| Insertion | 0.3838  | yes |
| Selection | 0.3274  | yes |
| Bubble    | 0.9723  | yes |

**近乎有序**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| Insertion | 0.0037  | yes |
| QuickBF   | 0.0680  | yes |
| Shell     | 0.0493  | yes |
| Quick     | 0.1107  | yes |
| MergeBU   | 0.1267  | yes |
| MergeTD   | 0.1347  | yes |
| Quick3way | 0.2532  | yes |
| Heap      | 0.3801  | yes |

**大量重复值**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| Quick3way | 0.0412  | yes |
| QuickBF   | 0.1167  | yes |
| Quick     | 0.1447  | yes |
| Shell     | 0.1367  | yes |
| MergeBU   | 0.1768  | yes |
| MergeTD   | 0.1853  | yes |
| Heap      | 0.3542  | yes |

### List（list_n2=2k, list_nlogn=50k）
**随机**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| L-MergeTD | 0.0100  | yes |
| L-Radix   | 0.0155  | yes |
| L-Insertion | 0.0013 | yes |
| L-Selection | 0.0030 | yes |

**近乎有序**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| L-Insertion | 0.0007 | yes |
| L-MergeTD | 0.0011  | yes |
| L-Radix   | 0.0062  | yes |
| L-Selection | 0.0022 | yes |

**大量重复值**
| Algorithm | Seconds | Sorted |
|-----------|---------|--------|
| L-MergeTD | 0.0070  | yes |
| L-Radix   | 0.0076  | yes |
| L-Selection | 0.0031 | yes |

### 结论要点
- MergeTD vs MergeBU：自底向上略快且更稳定（无递归开销、固定步长更利缓存）。
- Heap 慢的原因：sink 比较/交换多、缓存不友好、常数因子大；难以赶上 quick/merge，微调收益有限。
- 快排族随机 pivot 避免近乎有序/重复值退化；三路快排在重复值场景最佳。
- Shell 在近乎有序和重复键上快过两路快排（未切换到插入的版本）：间隔插入能快速消除稀疏逆序/重复带来的交换。
- List 复杂度：L-Selection/Insertion O(n²) 原地；L-Insertion 近乎有序时接近线性；L-MergeTD O(n log n) 稳定，栈 O(log n)；L-Radix O(w·n)，空间 O(k)。
