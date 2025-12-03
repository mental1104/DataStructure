#include <iostream>
#include <cassert>
#include "Heap.h"  // 包含 Heap 的实现

using namespace std;

int main() {
    cout << "开始运行 Heap 单元测试..." << endl;

    // ===== 测试1：插入元素后检查 getMax() =====
    Heap<int> heap;
    heap.insert(15);
    assert(heap.getMax() == 15);  // 只有一个元素时，自然最大就是它
    heap.insert(10);
    assert(heap.getMax() == 15);  // 插入比 15 小的元素，不影响堆顶
    heap.insert(20);
    assert(heap.getMax() == 20);  // 插入更大的元素后，堆顶应更新为 20
    heap.insert(5);
    assert(heap.getMax() == 20);
    cout << "测试1通过：insert 与 getMax() 正常工作。" << endl;

    // ===== 测试2：删除最大元素 delMax() =====
    int max_val = heap.delMax();
    assert(max_val == 20);
    assert(heap.getMax() == 15);
    max_val = heap.delMax();
    assert(max_val == 15);
    assert(heap.getMax() == 10);
    max_val = heap.delMax();
    assert(max_val == 10);
    max_val = heap.delMax();
    assert(max_val == 5);
    // 注意：堆为空时，不应调用 getMax()（此处仅做删除测试）
    cout << "测试2通过：delMax 删除最大元素顺序正确。" << endl;

    // ===== 测试3：数组批量构造堆（heapify） =====
    int arr[] = {12, 7, 9, 20, 5};
    int n = sizeof(arr) / sizeof(arr[0]);
    Heap<int> heap2(arr, n);
    // 经过 heapify 后，堆顶应为 20
    assert(heap2.getMax() == 20);
    // 依次删除，确保删除序列为降序（堆排序验证）
    int prev = heap2.delMax();
    while (heap2.size() > 0) {
        int curr = heap2.delMax();
        assert(prev >= curr);
        prev = curr;
    }
    cout << "测试3通过：批量构造与堆排序验证正常。" << endl;

    // ===== 测试4：遍历底层容器（Vector）的迭代器 =====
    Heap<int> heap3;
    heap3.insert(100);
    heap3.insert(50);
    heap3.insert(75);
    heap3.insert(25);
    int sum = 0;
    for (auto it = heap3.begin(); it != heap3.end(); ++it)
        sum += *it;
    // 计算累加和进行简单验证
    assert(sum == (100 + 50 + 75 + 25));
    cout << "测试4通过：迭代器遍历正常工作。" << endl;

    // ===== 测试5：小顶堆模式 =====
    Heap<int, false> minHeap;
    minHeap.insert(9);
    minHeap.insert(4);
    minHeap.insert(7);
    assert(minHeap.getMax() == 4); // 此时为最小元素
    assert(minHeap.delMax() == 4);
    assert(minHeap.getMax() == 7);
    cout << "测试5通过：小顶堆模式正常工作。" << endl;

    cout << "所有 Heap 单元测试均已通过！" << endl;
    return 0;
}
