import ctypes
import numpy as np
import matplotlib.pyplot as plt

# 加载共享库
libbst = ctypes.CDLL("../../lib/libbst.so")

# 设置benchmark函数的参数和返回值类型
libbst.benchmark.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
libbst.benchmark.restype = ctypes.c_double

# 定义操作类型
class Operation:
    INSERT = 0
    SEARCH = 1
    REMOVE = 2
    LOCALITY = 3

def run_benchmark(operation, scales, method):
    times = []
    for scale in scales:
        exec_time = libbst.benchmark(operation, scale, method)
        times.append(exec_time)
        print(f"Scale: {scale}, Time: {exec_time:.6f} sec")
    return times

# 设定测试参数
scales = np.logspace(3, 6, num=10, dtype=int)  # 1000 到 1000000
methods = [0, 1, 2, 3]
names = ["AVL", "RedBlack", "Splay", "BTree"]
colors = ['r', 'g', 'b', 'm']

plt.figure(figsize=(10, 6))

# 对不同方法进行基准测试
for method, name, color in zip(methods, names, colors):
    times = run_benchmark(Operation.INSERT, scales, method)
    plt.plot(scales, times, label=f"{name} Insert", color=color, marker='o')

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Scale")
plt.ylabel("Time (s)")
plt.title("BST Insert Benchmark")
plt.legend()
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.show()
