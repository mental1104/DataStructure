# Trie vs TST benchmark

Benchmarks live in `bench_trie_compare` and focus on where each structure shines.

- `ascii-short-lookup`: 120k unique keys, length 8, alphabet `[a-zA-Z0-9]`. Heavy hit/miss lookups with a few short prefixes. Direct array indexing in `Trie` should beat the ternary search tree's per-character comparisons.
- `binary-deep-prefix`: 10k unique keys, length 48, alphabet `{0,1}`. Repeated prefix enumerations (short and long) to stress `keysWithPrefix`. `TST` avoids scanning 128 child slots per node, so it should win here. (Count was reduced to keep runtime reasonable.)

Latest run (ms):

- ascii-short-lookup: Trie build 491.6 / hit 82.0 / miss 1.1 / prefix 1.5 vs TST build 368.5 / hit 209.0 / miss 3.0 / prefix 3.1. Trie wins lookups and prefixes; TST builds faster but slower to query.
- binary-deep-prefix: Trie build 173.1 / hit 15.4 / miss 0.10 / prefix 22121.2 vs TST build 8.1 / hit 4.9 / miss 0.08 / prefix 311.2 (prefix_out=60015 for both). TST massively faster on deep prefix enumeration.

Run from the build directory:

```bash
cmake .. -DENABLE_BENCHMARKS=ON
cmake --build . --target bench_trie_compare
./bench/trie/bench_trie_compare
```

Columns show build/lookup/prefix timings in milliseconds and how many strings each prefix query returned. Missed keys are forced outside the test alphabet to keep the workloads deterministic.
