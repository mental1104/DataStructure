# DSA

## 简介

本仓库主要涵盖《数据结构-邓俊辉》和 *Algorithm 4th* 中的代码:

参考资料：
+ Algorithms 4th: [Offical Website](https://algs4.cs.princeton.edu/home/)
+ Algorithms 4th: [Java Code](https://algs4.cs.princeton.edu/code/)
+ Algorithms 4th: [sample data](https://algs4.cs.princeton.edu/code/algs4-data.zip)
+ DSACPP: [Deng JunHui](https://dsa.cs.tsinghua.edu.cn/~deng/ds/dsacpp/)

本仓库基于官方的实现，将两套书籍中的代码精选数据结构和算法来实现。

## 如何使用

测试操作系统：macOS/Ubuntu 22.04    
测试编译器：g++/clang++  

```
git clone --recurse-submodules git@github.com:mental1104/DataStructure.git
```

编译：
```sql
cd DataStructure
mkdir build
cd build
cmake ..
make
```

### 运行测试

```sql
ctest
```

## 数据结构-邓俊辉-demo演示

### 1 绪论

### 2 向量

[src](demo/vector/Vector.cpp)

```
./demo/vector/Vector
```

![Vector](docs/pics/demo/Vector.png)

### 3 列表

[src](demo/list/List.cpp)

```
./demo/list/List
```
![List](docs/pics/demo/List.png)

### 4.1 栈

[src](demo/stack/Stack.cpp)

```
./demo/stack/Stack
```

![Stack](docs/pics/demo/Stack.gif)

#### 八皇后

[src](demo/stack/Queen.cpp)

```
./demo/stack/Queen
```
通过回车触发：
![Queen](docs/pics/demo/Queen.gif)

#### 迷宫寻径

[src](demo/stack/labyrinth.cpp)

```
cd demo/stack
./labyrinth
```

通过回车触发寻径：

![labyrinth](docs/pics/demo/labyrinth.gif)

#### 表达式求值

[src](demo/stack/evaluate.cpp)

```
./demo/stack/evaluate
```


![evaluate](docs/pics/demo/evaluate.png)

#### 括号匹配

[src](demo/stack/paren.cpp)

```
./demo/stack/paren
```

### 4.2 队列

[src](demo/queue/Queue.cpp)

```
./demo/queue/Queue
```

![Queue](docs/pics/demo/Queue.gif)

### 5 二叉树

[src](demo/bt/BinTree.cpp)

```
./demo/bt/BinTree
```
![alt text](docs/pics/demo/BinTree.png)

### 6 图

[src](demo/graph/GraphMatrix.cpp)

```
cd demo/graph
./GraphMatrix
```
![GraphMatrix](docs/pics/demo/GraphMatrix.png)

### 7 二叉搜索树

[src](demo/bst/BST.cpp)

```
./demo/bst/BST
```

![BST](docs/pics/demo/BST.png)

#### AVL树

[src](demo/bst/AVL.cpp)

```
./demo/bst/AVL
```

![AVL](docs/pics/demo/AVL.png)

### 8 高级二叉搜索树

#### 性能评测

```
cd demo/bst
python3 benchmark.py
```

#### Splay树

[src](demo/bst/Splay.cpp)

```
./demo/bst/Splay
```

![Splay](docs/pics/demo/Splay.png)

#### B-树

#### 红黑树

[src](demo/bst/RedBlack.cpp)

```
./demo/bst/RedBlack
```

![RedBlack](docs/pics/demo/RedBlack.png)

### 9 散列表

#### 哈希表-线性探测

[src](demo/bst/Hashtable.cpp)

```
./demo/bst/Hashtable
```


#### 哈希表-双向平方探测

[src](demo/bst/HashtableB.cpp)

```
./demo/bst/HashtableB
```

#### 跳跃表

[src](demo/bst/Skiplist.cpp)

```
./demo/bst/Skiplist
```

![Skiplist](docs/pics/demo/Skiplist.gif)

### 10 优先级队列

#### 堆

[src](demo/pq/Heap.cpp)

```
./demo/bst/Heap
```

![Heap](docs/pics/demo/Heap.png)

#### 左式堆

[src](demo/pq/LeftHeap.cpp)

```
./demo/bst/LeftHeap
```


### 11 串

### 12. 排序

#### 性能测试

```
cd demo/sort
python3 benchmark.py
```

## 算法4-demo演示

### 并查集

## 后续TODO
