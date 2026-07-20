#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <queue>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "FibonacciHeap.h"
#include "Heap.h"
#include "LeftHeap.h"
#include "PairingHeap.h"
#include "SkewHeap.h"
#include <dsa/container/heap/Heap.h>

namespace {

// 将任意工业堆与 std::priority_queue 执行固定种子的随机差分测试。
template<typename HeapType>
void expectPriorityQueueEquivalent() {
    HeapType actual;
    std::priority_queue<int> expected;
    std::mt19937 random(42);

    for (int operation = 0; operation < 5000; ++operation) {
        if (expected.empty() || random() % 3 != 0) {
            const int value = static_cast<int>(random() % 10000);
            actual.push(value);
            expected.push(value);
        } else {
            ASSERT_EQ(expected.top(), actual.top());
            actual.pop();
            expected.pop();
        }
        ASSERT_EQ(expected.size(), actual.size());
        if (!expected.empty())
            ASSERT_EQ(expected.top(), actual.top());
    }

    while (!expected.empty()) {
        ASSERT_EQ(expected.top(), actual.extract_top());
        expected.pop();
    }
    EXPECT_TRUE(actual.empty());
}

// 验证教学节点堆的复制、移动和破坏性 merge 不再共享裸指针所有权。
template<typename HeapType>
void expectTeachingOwnershipSemantics() {
    HeapType original;
    original.insert(3);
    original.insert(9);
    original.insert(1);

    HeapType copied(original);
    EXPECT_EQ(9, copied.delMax());
    EXPECT_EQ(9, original.getMax());

    HeapType assigned;
    assigned.insert(100);
    assigned = original;
    EXPECT_EQ(9, assigned.getMax());

    HeapType moved(std::move(copied));
    EXPECT_TRUE(copied.empty());
    EXPECT_EQ(3, moved.getMax());

    HeapType source;
    source.insert(8);
    source.insert(12);
    original.merge(source);
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(12, original.getMax());
}

// 记录 allocator 分配与释放数量，用于验证节点所有权闭环。
struct AllocationCounts {
    std::size_t allocations;
    std::size_t deallocations;

    AllocationCounts() : allocations(0), deallocations(0) {}
};

// 提供带身份的非传播 allocator，覆盖复制、移动和不兼容 meld 分支。
template<typename T>
class CountingAllocator {
public:
    typedef T value_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;

    AllocationCounts* counts;
    int identity;

    // 构造无统计目标的默认 allocator。
    CountingAllocator() : counts(NULL), identity(0) {}

    // 绑定统计对象和 allocator 身份。
    CountingAllocator(AllocationCounts* valueCounts, int valueIdentity)
        : counts(valueCounts), identity(valueIdentity) {}

    // 从其他 value_type 的 rebound allocator 复制状态。
    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other)
        : counts(other.counts), identity(other.identity) {}

    // 分配 n 个对象并累计统计。
    T* allocate(std::size_t count) {
        if (counts)
            counts->allocations += count;
        return std::allocator<T>().allocate(count);
    }

    // 释放 n 个对象并累计统计。
    void deallocate(T* pointer, std::size_t count) {
        if (counts)
            counts->deallocations += count;
        std::allocator<T>().deallocate(pointer, count);
    }

    template<typename U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };
};

// 判断两个 CountingAllocator 是否拥有相同资源身份。
template<typename T, typename U>
bool operator==(const CountingAllocator<T>& first, const CountingAllocator<U>& second) {
    return first.counts == second.counts && first.identity == second.identity;
}

// 判断两个 CountingAllocator 是否不兼容。
template<typename T, typename U>
bool operator!=(const CountingAllocator<T>& first, const CountingAllocator<U>& second) {
    return !(first == second);
}

// 验证 allocator-aware 节点堆释放数量与分配数量一致。
template<typename HeapType>
void expectAllocatorLifecycleClosed() {
    AllocationCounts counts;
    typedef CountingAllocator<int> Allocator;
    {
        HeapType heap(typename HeapType::value_compare(), Allocator(&counts, 1));
        for (int value = 0; value < 200; ++value)
            heap.push(value);

        HeapType copied(heap);
        HeapType moved(std::move(copied), Allocator(&counts, 1));
        while (!moved.empty())
            moved.pop();
        heap.clear();
    }
    EXPECT_EQ(counts.allocations, counts.deallocations);
}

// 保存比较器剩余成功次数，用于稳定触发异常路径。
struct ThrowState {
    explicit ThrowState(int value) : remaining(value) {}
    int remaining;
};

// 在指定比较次数后抛异常，验证 meld 的事务提交边界。
class ThrowingLess {
public:
    ThrowingLess() : state_(new ThrowState(1000000)) {}
    explicit ThrowingLess(const std::shared_ptr<ThrowState>& state) : state_(state) {}

    // 比较前递减预算，预算为零时抛出 runtime_error。
    bool operator()(int first, int second) const {
        if (state_->remaining-- == 0)
            throw std::runtime_error("comparison failure");
        return first < second;
    }

private:
    std::shared_ptr<ThrowState> state_;
};

// 验证比较器异常时 meld 不会让两个堆共享节点或改变 size/top。
template<typename HeapType>
void expectMeldRollbackOnComparisonFailure() {
    std::shared_ptr<ThrowState> state(new ThrowState(1000));
    HeapType first{ThrowingLess(state)};
    HeapType second{ThrowingLess(state)};
    for (int value : std::vector<int>{9, 7, 5})
        first.push(value);
    for (int value : std::vector<int>{8, 6, 4})
        second.push(value);

    const std::size_t firstSize = first.size();
    const std::size_t secondSize = second.size();
    state->remaining = 0;
    EXPECT_THROW(first.meld(second), std::runtime_error);
    EXPECT_EQ(firstSize, first.size());
    EXPECT_EQ(secondSize, second.size());
    EXPECT_EQ(9, first.top());
    EXPECT_EQ(8, second.top());
}

// 仅支持移动，用于验证工业堆不要求 value_type 可复制或默认构造。
class MoveOnlyValue {
public:
    explicit MoveOnlyValue(int value) : value_(value) {}
    MoveOnlyValue(MoveOnlyValue&& other) noexcept : value_(other.value_) {}
    MoveOnlyValue& operator=(MoveOnlyValue&& other) noexcept {
        value_ = other.value_;
        return *this;
    }

    MoveOnlyValue(const MoveOnlyValue&) = delete;
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

    // 返回测试值。
    int value() const { return value_; }

private:
    int value_;
};

// 按 MoveOnlyValue 内部整数建立标准最大堆语义。
struct MoveOnlyLess {
    bool operator()(const MoveOnlyValue& first, const MoveOnlyValue& second) const {
        return first.value() < second.value();
    }
};

TEST(TeachingHeapRefactorTest, CompleteBinaryHeapKeepsMaxAndMinModes) {
    Heap<int> maxHeap;
    Heap<int, false> minHeap;
    for (int value : std::vector<int>{3, 9, 1, 7, 5}) {
        maxHeap.insert(value);
        minHeap.insert(value);
    }

    for (int expected : std::vector<int>{9, 7, 5, 3, 1})
        EXPECT_EQ(expected, maxHeap.delMax());
    for (int expected : std::vector<int>{1, 3, 5, 7, 9})
        EXPECT_EQ(expected, minHeap.delMax());
    EXPECT_THROW(maxHeap.getMax(), std::runtime_error);
    EXPECT_THROW(maxHeap.delMax(), std::runtime_error);
}

TEST(TeachingHeapRefactorTest, NodeHeapsOwnCopiesIndependently) {
    expectTeachingOwnershipSemantics<LeftHeap<int> >();
    expectTeachingOwnershipSemantics<SkewHeap<int> >();
    expectTeachingOwnershipSemantics<PairingHeap<int> >();
    expectTeachingOwnershipSemantics<FibonacciHeap<int> >();
}

TEST(IndustrialHeapRefactorTest, AllFamiliesMatchPriorityQueue) {
    expectPriorityQueueEquivalent<dsa::container::BinaryHeap<int> >();
    expectPriorityQueueEquivalent<dsa::container::LeftistHeap<int> >();
    expectPriorityQueueEquivalent<dsa::container::SkewHeap<int> >();
    expectPriorityQueueEquivalent<dsa::container::PairingHeap<int> >();
    expectPriorityQueueEquivalent<dsa::container::FibonacciHeap<int> >();
}

TEST(IndustrialHeapRefactorTest, CompleteBinaryHeapSupportsRangeAndMinComparator) {
    const std::vector<int> values{3, 9, 1, 7, 5};
    dsa::container::BinaryHeap<int, std::greater<int> > heap(
        values.begin(),
        values.end(),
        std::greater<int>()
    );
    for (int expected : std::vector<int>{1, 3, 5, 7, 9})
        EXPECT_EQ(expected, heap.extract_top());
}

TEST(IndustrialHeapRefactorTest, MeldConsumesSourceAndPreservesOrder) {
    dsa::container::LeftistHeap<int> leftist;
    dsa::container::LeftistHeap<int> leftistSource;
    leftist.push(5);
    leftistSource.push(12);
    leftist.meld(leftistSource);
    EXPECT_TRUE(leftistSource.empty());
    EXPECT_EQ(12, leftist.top());

    dsa::container::FibonacciHeap<int> fibonacci;
    dsa::container::FibonacciHeap<int> fibonacciSource;
    fibonacci.push(8);
    fibonacciSource.push(20);
    fibonacci.meld(fibonacciSource);
    EXPECT_TRUE(fibonacciSource.empty());
    EXPECT_EQ(20, fibonacci.top());
}

TEST(IndustrialHeapRefactorTest, NodeFamiliesCloseAllocatorLifecycle) {
    typedef CountingAllocator<int> Allocator;
    expectAllocatorLifecycleClosed<
        dsa::container::LeftistHeap<int, std::less<int>, Allocator>
    >();
    expectAllocatorLifecycleClosed<
        dsa::container::SkewHeap<int, std::less<int>, Allocator>
    >();
    expectAllocatorLifecycleClosed<
        dsa::container::PairingHeap<int, std::less<int>, Allocator>
    >();
    expectAllocatorLifecycleClosed<
        dsa::container::FibonacciHeap<int, std::less<int>, Allocator>
    >();
}

TEST(IndustrialHeapRefactorTest, MeldRejectsIncompatibleAllocators) {
    AllocationCounts counts;
    typedef CountingAllocator<int> Allocator;
    dsa::container::PairingHeap<int, std::less<int>, Allocator> first(
        std::less<int>(),
        Allocator(&counts, 1)
    );
    dsa::container::PairingHeap<int, std::less<int>, Allocator> second(
        std::less<int>(),
        Allocator(&counts, 2)
    );
    first.push(9);
    second.push(8);

    EXPECT_THROW(first.meld(second), std::logic_error);
    EXPECT_EQ(1u, first.size());
    EXPECT_EQ(1u, second.size());
}

TEST(IndustrialHeapRefactorTest, MeldRollsBackWhenComparatorThrows) {
    expectMeldRollbackOnComparisonFailure<
        dsa::container::LeftistHeap<int, ThrowingLess>
    >();
    expectMeldRollbackOnComparisonFailure<
        dsa::container::SkewHeap<int, ThrowingLess>
    >();
    expectMeldRollbackOnComparisonFailure<
        dsa::container::PairingHeap<int, ThrowingLess>
    >();
    expectMeldRollbackOnComparisonFailure<
        dsa::container::FibonacciHeap<int, ThrowingLess>
    >();
}

TEST(IndustrialHeapRefactorTest, NodeFamiliesSupportMoveOnlyValues) {
    dsa::container::LeftistHeap<MoveOnlyValue, MoveOnlyLess> leftist;
    leftist.emplace(4);
    leftist.emplace(9);
    EXPECT_EQ(9, leftist.extract_top().value());

    dsa::container::SkewHeap<MoveOnlyValue, MoveOnlyLess> skew;
    skew.emplace(3);
    skew.emplace(8);
    EXPECT_EQ(8, skew.extract_top().value());

    dsa::container::PairingHeap<MoveOnlyValue, MoveOnlyLess> pairing;
    pairing.emplace(2);
    pairing.emplace(7);
    EXPECT_EQ(7, pairing.extract_top().value());

    dsa::container::FibonacciHeap<MoveOnlyValue, MoveOnlyLess> fibonacci;
    fibonacci.emplace(1);
    fibonacci.emplace(6);
    EXPECT_EQ(6, fibonacci.extract_top().value());
}

} // namespace
