# Graph Benchmark (Adjacency Matrix vs List)

数据来源：`bench_graph_compare`（连通图，权重随机 1~100），三组规模：
- sparse: n=3000, m=15000
- dense: n=3000, m=150000
- medium: n=5000, m=50000

记法：时间/内存（例如 59.750ms/111MB）。

## 纯耗时胜出（✔ 更快）

| 算法 | 矩阵 | 列表 |
| --- | --- | --- |
| build |  | ✔ |
| bfs | ✔ |  |
| dfs | ✔ |  |
| bcc | ✔ |  |
| tSort | ✔ |  |
| connected(v,w) |  | ✔ |
| connectedComponents |  | ✔ |
| reachableComponents |  | ✔ |
| dijkstra | ✔ |  |
| prim | ✔ |  |
| pfs | ✔ |  |
| kruskal |  | ✔ |
| kosarajuSCC |  | ✔ |

## 纯内存胜出（✔ 更少）

| 算法 | 矩阵 | 列表 |
| --- | --- | --- |
| build | ✔ |  |
| bfs | ✔ |  |
| dfs | ✔ |  |
| bcc | ✔ |  |
| tSort | ✔ |  |
| connected(v,w) | ✔(dense≈相当) |  |
| connectedComponents | ✔(dense≈相当) |  |
| reachableComponents | ✔ |  |
| dijkstra | ✔ |  |
| prim | ✔ |  |
| pfs | ✔ |  |
| kruskal | ✔(dense略低) |  |
| kosarajuSCC | ✔ |  |

## 详细结论与原因
以下时间/内存来自最新跑分（时间/内存，单位 ms/MB）。矩阵(M) 与邻接表(L) 的表现均用数据结构特性解释。

- 构建：M=86/112,113/126,263/359；L=2/112,148/132,10/359。表在稀疏/中等几乎 O(n+e) 推入链表，常数极小；矩阵需要初始化 n² 位/指针，dense 时还要填充 n² 空位，初始化成本高。
- BFS/DFS/BCC/tSort：稀疏 M≈57~60ms vs L≈165~175ms；dense M≈67~72ms vs L≈1.0s；medium M≈160~167ms vs L≈813~820ms。矩阵按行顺序访问连续内存，邻接检查是 O(1) 访问数组，CPU cache 命中高；表需遍历链表节点，存在指针跳转和 cache miss，邻居多时开销放大，dense 场景尤甚。
- Reachable/Connected/connected(v,w)：稀疏 L=0.43~0.63ms << M=30~32ms；dense L≈30~38ms ~ M≈31~32ms；medium L≈2.7~4.1ms << M≈81~85ms。连通性/可达性逻辑主要扫“有边”邻居，表的度遍历成本与度数成正比，稀疏/中等时链表短，常数小；矩阵需要扫描整行（长度 n），稀疏图中多数空，浪费时间。
- Dijkstra/Prim/PFS：稀疏 M≈95~102ms vs L≈205~211ms；dense M≈108~119ms vs L≈1071~1086ms；medium M≈276~285ms vs L≈924~939ms。三者都频繁取“下一最小优先级”顶点并多次松弛。矩阵邻接检查是数组索引，松弛循环 cache 友好；表在高出度场景下遍历链表耗时，导致总松弛次数相同但指针跳转开销显著，dense 时差距接近 10×。
- Kruskal：稀疏/medium L=3.6/15.8ms << M=40.4/93.2ms；dense L=56.0ms < M=62.0ms。Kruskal 首要成本是枚举边并入堆/排序，表直接遍历已有边集合，枚举 O(e)；矩阵需要扫描 n² 找非空边，稀疏图中大量空检查，导致耗时高。
- KosarajuSCC：稀疏 L=5.0ms << M=193.8ms；dense L=217.1ms < M=253.3ms；medium L=23.9ms << M=513.9ms。原因同 DFS：矩阵逆图/正图遍历扫描整行，稀疏时大量空位；表只走实际出边，回溯与反转遍历开销低。
- 内存：稀疏/medium 两者接近（M=110~112MB/358~359MB，L=112MB/359MB），dense 时表多出 3~16MB（M=113~126MB，L=126~132MB），源于每条边链表节点的指针/对象开销；矩阵为 n² 指针或标记，dense 时指针数组与对象占用集中但开销可控。

## 适用建议
- 遍历/最短路/Prim 类（频繁邻接扫描）：矩阵在时间上占优，尤其在度数高时。
- 连通性/可达性、Kruskal、SCC（只需真实边集）：稀疏图优先用表；dense 图 SCC/Kruskal 仍有小幅优势，但遍历类可能成为瓶颈。
- 构建阶段关注：稀疏/中等图表构建极快；若频繁重建图可优先表。稠密图两者成本接近。

## 数据来源

espeon@Blue-Espeon:~/code/common/cpp/lib/DataStructure/build$ ./bench/graph/bench_graph_compare 

[Case] sparse n=3000 m=15000
Algo                  Matrix                  List                    
----------------------------------------------------------------------
bcc                   59.750ms/111MB          174.387ms/112MB         
bfs                   57.483ms/110MB          172.169ms/112MB         
build                 86.841ms/112MB          2.165ms/112MB           
connected(v,w)        30.021ms/111MB          0.443ms/112MB           
connectedComponents   31.156ms/111MB          0.627ms/112MB           
dfs                   57.735ms/110MB          170.109ms/112MB         
dijkstra              96.949ms/111MB          204.931ms/112MB         
kosarajuSCC           193.816ms/112MB         4.967ms/112MB           
kruskal               40.395ms/111MB          3.573ms/112MB           
pfs                   101.584ms/111MB         204.762ms/112MB         
prim                  95.476ms/111MB          211.003ms/112MB         
reachableComponents   32.468ms/111MB          0.430ms/112MB           
tSort                 33.788ms/111MB          165.794ms/112MB         

[Case] dense n=3000 m=150000
Algo                  Matrix                  List                    
----------------------------------------------------------------------
bcc                   72.117ms/113MB          1077.591ms/126MB        
bfs                   70.989ms/113MB          1059.507ms/126MB        
build                 113.294ms/126MB         148.055ms/132MB         
connected(v,w)        32.947ms/113MB          30.763ms/129MB          
connectedComponents   32.048ms/113MB          32.546ms/129MB          
dfs                   72.669ms/113MB          1036.888ms/126MB        
dijkstra              108.249ms/113MB         1072.492ms/126MB        
kosarajuSCC           253.304ms/126MB         217.146ms/132MB         
kruskal               61.995ms/113MB          56.022ms/129MB          
pfs                   119.079ms/113MB         1071.794ms/126MB        
prim                  109.736ms/113MB         1085.876ms/126MB        
reachableComponents   32.228ms/113MB          29.660ms/129MB          
tSort                 34.846ms/113MB          1007.146ms/126MB        

[Case] medium n=5000 m=50000
Algo                  Matrix                  List                    
----------------------------------------------------------------------
bcc                   164.980ms/359MB         819.844ms/359MB         
bfs                   161.701ms/359MB         819.265ms/359MB         
build                 262.941ms/359MB         9.677ms/359MB           
connected(v,w)        81.318ms/359MB          4.110ms/359MB           
connectedComponents   84.560ms/359MB          3.803ms/359MB           
dfs                   167.417ms/359MB         813.901ms/359MB         
dijkstra              275.557ms/359MB         938.776ms/359MB         
kosarajuSCC           513.875ms/359MB         23.908ms/359MB          
kruskal               93.207ms/359MB          15.765ms/359MB          
pfs                   285.497ms/359MB         924.171ms/359MB         
prim                  275.610ms/359MB         928.126ms/359MB         
reachableComponents   85.526ms/359MB          2.669ms/359MB           
tSort                 87.961ms/359MB          812.868ms/359MB
