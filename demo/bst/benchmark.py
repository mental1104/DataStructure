'''
Date: 2025-02-16 11:30:00
Author: mental1104 mental1104@gmail.com
LastEditors: mental1104 mental1104@gmail.com
LastEditTime: 2025-02-16 11:44:45
'''
import ctypes
import numpy as np
import matplotlib.pyplot as plt

# 加载共享库
lib = ctypes.CDLL("../../lib/libbstapi.so")

# 设置 benchmark 函数的返回值类型
lib.benchmark.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
lib.benchmark.restype = ctypes.c_double

# 枚举操作类型
operations = {"INSERT": 0, "SEARCH": 1, "REMOVE": 2, "LOCALITY": 3}

def run_benchmark(op_name, scales, method):
    op_code = operations[op_name]
    times = []
    for scale in scales:
        time_taken = lib.benchmark(op_code, scale, method)
        times.append(time_taken)
    return times

def plot_results(scales, results, op_name):
    plt.figure()
    for method, times in results.items():
        plt.plot(scales, times, marker='o', label=method)
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Scale")
    plt.ylabel("Time (s)")
    plt.title(f"{op_name} Performance Comparison")
    plt.legend()
    plt.grid(True, which="both", linestyle="--")
    plt.savefig(f"{op_name.lower()}_benchmark.png")
    plt.close()

# 设定测试规模
scales = [1000000, 2000000, 3000000, 4000000]
methods = {0: "AVL", 1: "RedBlack", 2: "Splay", 3: "BTree"}

for op_name in operations.keys():
    results = {}
    for method_code, method_name in methods.items():
        print(op_name, method_name)
        results[method_name] = run_benchmark(op_name, scales, method_code)
        
    plot_results(scales, results, op_name)
