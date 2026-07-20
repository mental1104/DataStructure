#include <gtest/gtest.h>

#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "dsa/algorithm/Search.h"
#include "dsa/algorithm/Sequence.h"
#include "dsa/container/vector/Vector.h"
#include "Sort.h"

namespace {

template<typename T>
using IndustrialVector = dsa::container::Vector<T>;

/// 用于验证容器支持不可拷贝、不可默认构造元素的测试类型。
struct MoveOnlyValue {
    int value;

    /// 使用整数初始化测试值。
    explicit MoveOnlyValue(int current)
        : value(current) {
    }

    /// 移动构造测试值，并将源对象标记为已移动。
    MoveOnlyValue(MoveOnlyValue&& other) noexcept
        : value(other.value) {
        other.value = -1;
    }

    /// 移动赋值测试值，并将源对象标记为已移动。
    MoveOnlyValue& operator=(MoveOnlyValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    /// 禁止拷贝构造，确保测试覆盖 move-only 元素。
    MoveOnlyValue(const MoveOnlyValue&) = delete;

    /// 禁止拷贝赋值，确保测试覆盖 move-only 元素。
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

    /// 提供排序算法需要的小于比较。
    bool operator<(const MoveOnlyValue& other) const {
        return value < other.value;
    }

    /// 提供序列算法和断言需要的相等比较。
    bool operator==(const MoveOnlyValue& other) const {
        return value == other.value;
    }
};

/// 用于验证 erase、clear 和析构是否及时结束对象生命周期的测试类型。
struct TrackedValue {
    static int live;
    int value;

    /// 构造对象并增加存活计数。
    explicit TrackedValue(int current = 0)
        : value(current) {
        ++live;
    }

    /// 拷贝构造对象并增加存活计数。
    TrackedValue(const TrackedValue& other)
        : value(other.value) {
        ++live;
    }

    /// 移动构造对象并增加存活计数。
    TrackedValue(TrackedValue&& other) noexcept
        : value(other.value) {
        other.value = -1;
        ++live;
    }

    /// 拷贝赋值对象值，不改变存活计数。
    TrackedValue& operator=(const TrackedValue& other) {
        value = other.value;
        return *this;
    }

    /// 移动赋值对象值，不改变存活计数。
    TrackedValue& operator=(TrackedValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    /// 析构对象并减少存活计数。
    ~TrackedValue() {
        --live;
    }
};

int TrackedValue::live = 0;

/// 用于验证扩容和插入异常回滚的可注入异常测试类型。
struct ThrowingValue {
    static int live;
    static int copies;
    static int throwAfter;

    int value;

    /// 构造对象并增加存活计数。
    explicit ThrowingValue(int current = 0)
        : value(current) {
        ++live;
    }

    /// 在达到指定次数后抛出异常的拷贝构造函数。
    ThrowingValue(const ThrowingValue& other)
        : value(other.value) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("copy failed");
        ++live;
    }

    /// 在达到指定次数后抛出异常的移动构造函数。
    ThrowingValue(ThrowingValue&& other) noexcept(false)
        : value(other.value) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("move failed");
        other.value = -1;
        ++live;
    }

    /// 在达到指定次数后抛出异常的拷贝赋值运算符。
    ThrowingValue& operator=(const ThrowingValue& other) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("copy assignment failed");
        value = other.value;
        return *this;
    }

    /// 在达到指定次数后抛出异常的移动赋值运算符。
    ThrowingValue& operator=(ThrowingValue&& other) noexcept(false) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("move assignment failed");
        value = other.value;
        other.value = -1;
        return *this;
    }

    /// 析构对象并减少存活计数。
    ~ThrowingValue() {
        --live;
    }
};

int ThrowingValue::live = 0;
int ThrowingValue::copies = 0;
int ThrowingValue::throwAfter = -1;

/// 记录测试 allocator 的申请和释放次数。
struct AllocationState {
    int allocations;
    int deallocations;

    /// 将申请与释放计数初始化为零。
    AllocationState()
        : allocations(0), deallocations(0) {
    }
};

/// 用于验证 stateful allocator propagation 和存储释放配对的测试 allocator。
template<typename T>
class CountingAllocator {
public:
    typedef T value_type;
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

    AllocationState* state;

    /// 构造未绑定统计状态的 allocator。
    CountingAllocator() noexcept
        : state(nullptr) {
    }

    /// 构造绑定指定统计状态的 allocator。
    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {
    }

    /// 从其他 value_type 的 CountingAllocator 转换构造。
    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : state(other.state) {
    }

    /// 申请 count 个 T 的存储并记录申请次数。
    T* allocate(std::size_t count) {
        if (state != nullptr)
            ++state->allocations;
        return std::allocator<T>().allocate(count);
    }

    /// 释放存储并记录释放次数。
    void deallocate(T* memory, std::size_t count) noexcept {
        if (state != nullptr)
            ++state->deallocations;
        std::allocator<T>().deallocate(memory, count);
    }

    /// 为 allocator_traits 提供其他 value_type 的 rebinding 类型。
    template<typename U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };
};

/// 判断两个 CountingAllocator 是否绑定同一个统计状态。
template<typename T, typename U>
bool operator==(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return left.state == right.state;
}

/// 判断两个 CountingAllocator 是否绑定不同统计状态。
template<typename T, typename U>
bool operator!=(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return !(left == right);
}

} // namespace

/// 验证主要构造方式、随机访问和 at 越界检查。
TEST(IndustrialVectorTest, ConstructorsAndElementAccessMatchStdVectorShape) {
    IndustrialVector<int> empty;
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(empty.size(), 0U);

    IndustrialVector<int> defaultValues(3);
    EXPECT_EQ(defaultValues.size(), 3U);
    EXPECT_EQ(defaultValues[0], 0);

    IndustrialVector<int> filled(4, 7);
    EXPECT_EQ(filled.size(), 4U);
    EXPECT_EQ(filled.front(), 7);
    EXPECT_EQ(filled.back(), 7);

    int source[] = {1, 2, 3};
    IndustrialVector<int> ranged(source, source + 3);
    EXPECT_EQ(ranged.size(), 3U);
    EXPECT_EQ(ranged[1], 2);

    IndustrialVector<int> listed{4, 5, 6};
    listed[1] = 50;
    EXPECT_EQ(listed.at(1), 50);
    EXPECT_THROW(listed.at(8), std::out_of_range);
    EXPECT_EQ(listed.data(), listed.begin());
}

/// 验证单遍输入迭代器区间构造不会重复遍历输入源。
TEST(IndustrialVectorTest, InputIteratorConstructorConsumesSinglePassRange) {
    std::istringstream stream("1 2 3 4");
    std::istream_iterator<int> first(stream);
    std::istream_iterator<int> last;
    IndustrialVector<int> values(first, last);

    ASSERT_EQ(values.size(), 4U);
    EXPECT_EQ(values.front(), 1);
    EXPECT_EQ(values.back(), 4);
}

/// 验证拷贝拥有独立存储，移动操作正确转移所有权。
TEST(IndustrialVectorTest, CopyAndMoveSemanticsOwnIndependentStorage) {
    IndustrialVector<int> original{1, 2, 3};
    IndustrialVector<int> copied(original);
    copied[0] = 9;

    EXPECT_EQ(original[0], 1);
    EXPECT_EQ(copied[0], 9);

    IndustrialVector<int> copyAssigned;
    copyAssigned = original;
    EXPECT_EQ(copyAssigned, original);

    IndustrialVector<int> moved(std::move(copied));
    EXPECT_EQ(moved.size(), 3U);
    EXPECT_TRUE(copied.empty());

    IndustrialVector<int> moveAssigned;
    moveAssigned = std::move(copyAssigned);
    EXPECT_EQ(moveAssigned, original);
    EXPECT_TRUE(copyAssigned.empty());
}

/// 验证 push_back 使用几何增长并具备均摊 O(1) 扩容次数。
TEST(IndustrialVectorTest, PushBackUsesGeometricAmortizedGrowth) {
    AllocationState state;
    typedef CountingAllocator<int> Allocator;
    dsa::container::Vector<int, Allocator> values{Allocator(&state)};

    for (int value = 0; value < 1000; ++value)
        values.push_back(value);

    EXPECT_EQ(values.size(), 1000U);
    EXPECT_GE(values.capacity(), values.size());
    EXPECT_LT(state.allocations, 20);
    EXPECT_EQ(values[512], 512);
}

/// 验证首尾插入和删除均经过共享 VectorAlgorithm 流程。
TEST(IndustrialVectorTest, FrontAndBackInsertionReuseVectorMutationAlgorithm) {
    IndustrialVector<int> values;
    values.push_back(2);
    values.push_front(1);
    values.emplace_back(3);
    values.emplace_front(0);

    ASSERT_EQ(values.size(), 4U);
    EXPECT_EQ(values[0], 0);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 2);
    EXPECT_EQ(values[3], 3);

    values.pop_front();
    values.pop_back();
    ASSERT_EQ(values.size(), 2U);
    EXPECT_EQ(values.front(), 1);
    EXPECT_EQ(values.back(), 2);
}

/// 验证 erase 不自动缩容，而 shrink_to_fit 显式压缩容量。
TEST(IndustrialVectorTest, EraseKeepsCapacityUntilShrinkToFit) {
    IndustrialVector<int> values;
    for (int value = 0; value < 32; ++value)
        values.push_back(value);

    const IndustrialVector<int>::size_type oldCapacity = values.capacity();
    IndustrialVector<int>::iterator next = values.erase(
        values.begin() + 4,
        values.begin() + 28
    );

    EXPECT_EQ(values.size(), 8U);
    EXPECT_EQ(values.capacity(), oldCapacity);
    EXPECT_EQ(*next, 28);

    values.shrink_to_fit();
    EXPECT_EQ(values.capacity(), values.size());
}

/// 验证 move-only 且不可默认构造类型可参与插入和删除。
TEST(IndustrialVectorTest, MoveOnlyAndNonDefaultConstructibleValuesAreSupported) {
    IndustrialVector<MoveOnlyValue> values;
    values.emplace_back(2);
    values.emplace_front(1);
    values.push_back(MoveOnlyValue(3));

    ASSERT_EQ(values.size(), 3U);
    EXPECT_EQ(values[0].value, 1);
    EXPECT_EQ(values[2].value, 3);

    values.erase(values.begin() + 1);
    ASSERT_EQ(values.size(), 2U);
    EXPECT_EQ(values[1].value, 3);
}

/// 验证 erase 和 clear 会立即析构退出逻辑区间的对象。
TEST(IndustrialVectorTest, EraseAndClearDestroyElementsImmediately) {
    TrackedValue::live = 0;
    {
        IndustrialVector<TrackedValue> values;
        values.emplace_back(1);
        values.emplace_back(2);
        values.emplace_front(0);
        EXPECT_EQ(TrackedValue::live, 3);

        values.pop_front();
        EXPECT_EQ(TrackedValue::live, 2);

        values.clear();
        EXPECT_EQ(TrackedValue::live, 0);
    }
    EXPECT_EQ(TrackedValue::live, 0);
}

/// 验证重新分配过程中构造失败会清理临时存储并保留原容器。
TEST(IndustrialVectorTest, ReallocationFailureRollsBackPendingStorage) {
    ThrowingValue::live = 0;
    ThrowingValue::copies = 0;
    ThrowingValue::throwAfter = -1;

    {
        IndustrialVector<ThrowingValue> values;
        values.reserve(3);
        values.emplace_back(1);
        values.emplace_back(2);
        values.emplace_back(3);

        const IndustrialVector<ThrowingValue>::size_type oldCapacity =
            values.capacity();

        ThrowingValue::copies = 0;
        ThrowingValue::throwAfter = 1;
        EXPECT_THROW(values.emplace_front(0), std::runtime_error);

        EXPECT_EQ(values.size(), 3U);
        EXPECT_EQ(values.capacity(), oldCapacity);
        EXPECT_EQ(values[0].value, 1);
        EXPECT_EQ(values[1].value, 2);
        EXPECT_EQ(values[2].value, 3);
        EXPECT_EQ(ThrowingValue::live, 3);

        ThrowingValue::throwAfter = -1;
    }

    EXPECT_EQ(ThrowingValue::live, 0);
}

/// 验证 reserve、resize、assign 和 swap 的基本容器语义。
TEST(IndustrialVectorTest, ReserveResizeAssignAndSwapFollowContainerSemantics) {
    IndustrialVector<int> values;
    values.reserve(20);
    EXPECT_GE(values.capacity(), 20U);

    values.resize(3, 8);
    EXPECT_EQ(values.size(), 3U);
    EXPECT_EQ(values[2], 8);

    values.resize(1);
    EXPECT_EQ(values.size(), 1U);

    values.assign({1, 2, 3});
    IndustrialVector<int> other{9, 8};
    values.swap(other);

    EXPECT_EQ(values.size(), 2U);
    EXPECT_EQ(values.front(), 9);
    EXPECT_EQ(other.size(), 3U);
    EXPECT_EQ(other.back(), 3);
}

/// 验证 Search.h 和 Sequence.h 可以直接操作工业 Vector 的连续迭代器。
TEST(IndustrialVectorTest, SequenceAlgorithmsOperateOnIndustrialIterators) {
    IndustrialVector<int> values{1, 1, 2, 2, 3};

    IndustrialVector<int>::iterator newEnd = dsa::algorithm::uniqueAdjacent(
        values.begin(),
        values.end()
    );
    values.erase(newEnd, values.end());

    ASSERT_EQ(values.size(), 3U);
    EXPECT_EQ(
        dsa::algorithm::disorderCount(values.begin(), values.end()),
        0
    );

    IndustrialVector<int>::iterator upper = dsa::algorithm::upperBound(
        values.begin(),
        values.end(),
        2
    );
    EXPECT_EQ(upper - values.begin(), 2);
}

/// 验证 Sort.h 的工业 Vector 重载支持普通类型和 move-only 类型。
TEST(IndustrialVectorTest, SortFacadeSupportsIndustrialVector) {
    IndustrialVector<int> values{5, 1, 4, 2, 3};
    Sort(values, SortStrategy::QuickSort);

    for (int index = 0; index < 5; ++index)
        EXPECT_EQ(values[static_cast<std::size_t>(index)], index + 1);

    IndustrialVector<MoveOnlyValue> moveOnly;
    moveOnly.emplace_back(3);
    moveOnly.emplace_back(1);
    moveOnly.emplace_back(2);
    Sort(moveOnly, SortStrategy::HeapSort);

    EXPECT_EQ(moveOnly[0].value, 1);
    EXPECT_EQ(moveOnly[2].value, 3);
    EXPECT_THROW(
        Sort(values, SortStrategy::RadixSort),
        std::invalid_argument
    );
}

/// 验证 stateful allocator 的传播规则和申请释放次数保持配对。
TEST(IndustrialVectorTest, StatefulAllocatorPropagationDoesNotLeakStorage) {
    AllocationState firstState;
    AllocationState secondState;
    typedef CountingAllocator<int> Allocator;
    typedef dsa::container::Vector<int, Allocator> AllocatedVector;

    {
        AllocatedVector first{Allocator(&firstState)};
        first.push_back(1);
        first.push_back(2);

        AllocatedVector second{Allocator(&secondState)};
        second.push_back(9);
        second = first;

        EXPECT_EQ(second.get_allocator().state, &firstState);
        EXPECT_EQ(second, first);

        AllocatedVector moved{Allocator(&secondState)};
        moved = std::move(second);
        EXPECT_EQ(moved.get_allocator().state, &firstState);
        EXPECT_EQ(moved.size(), 2U);
    }

    EXPECT_EQ(firstState.allocations, firstState.deallocations);
    EXPECT_EQ(secondState.allocations, secondState.deallocations);
}
