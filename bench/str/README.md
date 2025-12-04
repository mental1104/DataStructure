# Substring Match Benchmarks

基于 `bench_substring_match.cpp` 的几类场景，数据均为整数种子下可复现的随机/构造输入，指标为单次匹配耗时（毫秒）与首个匹配位置。全部算法返回结果正确。

## 场景与结论
- random-200k / random-400k（小字母表随机，模式 64/128）：BM（bad-char/full）常数优势显著，KMP 改进版略优于 basic，KR 常数最大。
- repetitive-200k（文本全 8，模式全 7，植入唯一匹配）：BM 坏字符一跳到位几乎零耗时；KMP/KR 线性推进。
- bm-worstish-200k（文本全 7，模式末尾 8 才分叉）：BM 跳距被压缩，耗时接近 KMP/KR，展示 BM 的退化趋势。
- long-pattern-300k（模式长 5000）：BM 仍最快但差距收窄；KMP 改进版优于 basic；BM full 在该规模下常数略高于 bad-char。
- ascii-200k（大字母表随机 ASCII）：字符分布稀疏使坏字符表发挥更大跳距，BM 依旧最快；KMP/KR 常数偏高。
- match-at-end-200k（匹配只在文本末尾）：KMP/KR 必扫到底，耗时最长；BM 仍受益于跳跃。
- no-match-200k（文本为数字，模式为字母，无匹配）：BM 通过坏字符快速滑过，耗时最低；KMP/KR 线性扫完文本。

## 核心 takeaway
- BM 平均/常见场景常数小、跳距大，表现最佳；但在高度重复且末尾才分叉或极长模式下优势收敛。
- KMP/KR 稳定 O(n) 最坏时间，适合需要确定性上界的场合；KMP 改进版在长模式上优于 basic。
- 字母表越大、模式越短，BM 的坏字符/好后缀跳跃收益越明显；字母表越小且模式末尾才差异，BM 退化更容易显现。
