import ctypes
import matplotlib.pyplot as plt

# 加载编译后的 C++ 共享库
lib = ctypes.CDLL("../../lib/libsort.so")
lib.sort_random_vector.argtypes = [ctypes.c_int, ctypes.c_int]
lib.sort_random_vector.restype = ctypes.c_double

def benchmark_sorting(size, algorithm):
    """调用 C++ 排序函数进行排序测试，并返回耗时（秒）"""
    return lib.sort_random_vector(size, algorithm)

def run_scenario(scenario):
    """
    运行一个测试场景：
    scenario 是一个字典，包含：
      - name: 场景名称（字符串）
      - title: 图表标题（字符串，可选，默认使用 name）
      - sizes: 数据规模列表
      - algorithms: 算法字典，key 为算法名称，value 为传给底层的策略码
      - output_file: 生成图表保存的文件名
    """
    print(f"开始测试场景: {scenario['name']}")
    sizes = scenario['sizes']
    algorithms = scenario['algorithms']
    results = {name: [] for name in algorithms}

    # 对每个数据规模和每个算法进行测试
    for size in sizes:
        print(f"\n测试数据规模：{size}")
        for name, algo in algorithms.items():
            duration = benchmark_sorting(size, algo)
            results[name].append(duration)
            print(f"  {name} ({size}个元素) 耗时：{duration:.6f}秒")
    
    # 绘制折线图
    plt.figure(figsize=(10, 6))
    for name, times in results.items():
        plt.plot(sizes, times, marker='o', label=name)
    
    plt.xlabel("数据规模（元素个数）")
    plt.ylabel("执行时间（秒）")
    plt.title(scenario.get("title", scenario["name"]))
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.6)

    # 保存图表到持久化文件
    output_file = scenario.get("output_file", f"{scenario['name']}.png")
    plt.savefig(output_file)
    print(f"\n图表已保存到文件：{output_file}\n")
    plt.close()

if __name__ == '__main__':
    # 定义各个测试场景

    # 1. 场景：大数据量下非 O(n^2) 算法测试（排除 BubbleSort、SelectionSort 和 InsertionSort）
    scenario1 = {
        "name": "LargeScale_NonQuadratic",
        "title": "大数据量下非 O(n^2) 算法性能对比",
        # 数据规模从 1000 万到 1 亿，可根据需求调整
        "sizes": [10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000,
                  60_000_000, 70_000_000, 80_000_000, 90_000_000, 100_000_000],
        "algorithms": {
            "ShellSort": 3,
            "MergeSort": 4,
            "MergeSortB": 5,
            "QuickSort": 6,
            "Quick3way": 7,
            "QuickSortB": 8,
            "HeapSort": 9,
        },
        "output_file": "benchmark_group1_large_scale_non_quadratic.png"
    }

    # 2. 场景：小数据量下全算法对决（期望 InsertionSort 能脱颖而出）
    scenario2 = {
        "name": "SmallScale_AllAlgorithms",
        "title": "小数据量下所有排序算法性能对比",
        # 小数据量，适合展示 InsertionSort 的优势
        "sizes": [10, 50, 100, 200, 500, 1000],
        "algorithms": {
            "BubbleSort": 0,
            "SelectionSort": 1,
            "InsertionSort": 2,
            "ShellSort": 3,
            "MergeSort": 4,
            "MergeSortB": 5,
            "QuickSort": 6,
            "Quick3way": 7,
            "QuickSortB": 8,
            "HeapSort": 9,
        },
        "output_file": "benchmark_group2_small_scale_all.png"
    }

    # 3. 场景：对比三个 O(n^2) 算法（BubbleSort、SelectionSort、InsertionSort）
    scenario3 = {
        "name": "SmallScale_Quadratic_Algorithms",
        "title": "小数据量下 O(n^2) 算法性能对比",
        # 为避免长时间运行，数据规模选取较小的数值
        "sizes": [100, 200, 300, 400, 500],
        "algorithms": {
            "BubbleSort": 0,
            "SelectionSort": 1,
            "InsertionSort": 2,
        },
        "output_file": "benchmark_group3_quadratic.png"
    }

    # 4. 场景：大数据量下对比 QuickSort、QuickSortB 和 Quick3way 的区别
    scenario4 = {
        "name": "LargeScale_QuickSorts",
        "title": "大数据量下 QuickSort 变体性能对比",
        # 数据规模采用 1000 万到 5000 万
        "sizes": [10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000],
        "algorithms": {
            "QuickSort": 6,
            "Quick3way": 7,
            "QuickSortB": 8,
        },
        "output_file": "benchmark_group4_quicksorts.png"
    }

    # 将所有场景集中到一个列表，方便后续扩展
    scenarios = [scenario1, scenario2, scenario3, scenario4]

    # 逐个运行各个测试场景
    for scenario in scenarios:
        run_scenario(scenario)
