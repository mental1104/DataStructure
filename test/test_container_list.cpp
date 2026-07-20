#include <gtest/gtest.h>

#include <list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "List.h"
#include "Queue.h"
#include "StackB.h"
#include "dsa/algorithm/Search.h"
#include "dsa/algorithm/Sequence.h"
#include "dsa/container/list/List.h"
#include "Sort.h"

namespace {

template<typename T>
using IndustrialList = dsa::container::List<T>;

/// 验证链表节点不要求元素可默认构造或复制。
struct MoveOnlyValue {
    int value;

    explicit MoveOnlyValue(int current)
        : value(current) {
    }

    MoveOnlyValue(MoveOnlyValue&& other) noexcept
        : value(other.value) {
        other.value = -1;
    }

    MoveOnlyValue& operator=(MoveOnlyValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    MoveOnlyValue(const MoveOnlyValue&) = delete;
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

    bool operator<(const MoveOnlyValue& other) const {
        return value < other.value;
    }

    bool operator==(const MoveOnlyValue& other) const {
        return value == other.value;
    }
};

/// 验证 erase、clear 与析构会立即结束节点中元素的生命周期。
struct TrackedValue {
    static int live;
    int value;

    explicit TrackedValue(int current = 0)
        : value(current) {
        ++live;
    }

    TrackedValue(const TrackedValue& other)
        : value(other.value) {
        ++live;
    }

    TrackedValue(TrackedValue&& other) noexcept
        : value(other.value) {
        other.value = -1;
        ++live;
    }

    TrackedValue& operator=(const TrackedValue& other) {
        value = other.value;
        return *this;
    }

    TrackedValue& operator=(TrackedValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    ~TrackedValue() {
        --live;
    }

    bool operator==(const TrackedValue& other) const {
        return value == other.value;
    }
};

int TrackedValue::live = 0;

/// 验证节点值构造抛异常时，链表链接和 size 不会提前提交。
struct ThrowingValue {
    static int live;
    static bool failConstruction;
    int value;

    explicit ThrowingValue(int current = 0)
        : value(current) {
        if (failConstruction)
            throw std::runtime_error("value construction failed");
        ++live;
    }

    ThrowingValue(const ThrowingValue& other)
        : value(other.value) {
        if (failConstruction)
            throw std::runtime_error("value copy failed");
        ++live;
    }

    ThrowingValue(ThrowingValue&& other)
        : value(other.value) {
        if (failConstruction)
            throw std::runtime_error("value move failed");
        other.value = -1;
        ++live;
    }

    ThrowingValue& operator=(const ThrowingValue& other) {
        value = other.value;
        return *this;
    }

    ThrowingValue& operator=(ThrowingValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    ~ThrowingValue() {
        --live;
    }
};

int ThrowingValue::live = 0;
bool ThrowingValue::failConstruction = false;

/// 记录节点 allocator 的申请和释放次数。
struct AllocationState {
    int allocations;
    int deallocations;

    AllocationState()
        : allocations(0), deallocations(0) {
    }
};

/// 验证 allocator rebind、传播和节点释放配对。
template<typename T>
class CountingAllocator {
public:
    typedef T value_type;
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

    AllocationState* state;

    CountingAllocator() noexcept
        : state(nullptr) {
    }

    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {
    }

    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : state(other.state) {
    }

    T* allocate(std::size_t count) {
        if (state != nullptr)
            ++state->allocations;
        return std::allocator<T>().allocate(count);
    }

    void deallocate(T* memory, std::size_t count) noexcept {
        if (state != nullptr)
            ++state->deallocations;
        std::allocator<T>().deallocate(memory, count);
    }

    template<typename U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };
};

template<typename T, typename U>
bool operator==(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return left.state == right.state;
}

template<typename T, typename U>
bool operator!=(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return !(left == right);
}

/// 返回双向链表中第 index 个迭代器，供差分测试定位。
template<typename Container>
typename Container::iterator iteratorAt(Container& container, std::size_t index) {
    typename Container::iterator current = container.begin();
    while (index-- > 0)
        ++current;
    return current;
}

/// 逐项比较工业 List 与 std::list 的共同序列语义。
void expectSameSequence(
    const IndustrialList<int>& actual,
    const std::list<int>& expected
) {
    ASSERT_EQ(actual.size(), expected.size());
    IndustrialList<int>::const_iterator actualIt = actual.begin();
    std::list<int>::const_iterator expectedIt = expected.begin();
    while (expectedIt != expected.end()) {
        ASSERT_NE(actualIt, actual.end());
        EXPECT_EQ(*actualIt, *expectedIt);
        ++actualIt;
        ++expectedIt;
    }
    EXPECT_EQ(actualIt, actual.end());
}

} // namespace

/// 验证教学 List 的公开节点 API、派生 Queue/Stack 与切片构造保持兼容。
TEST(TeachingListRefactorTest, PublicNodeAndDerivedContainerContractsRemainAvailable) {
    ::List<int> values;
    values.insertAsLast(3);
    values.insertAsLast(1);
    values.insertAsLast(1);

    EXPECT_EQ(values.disordered(), 1);
    ::List<int> slice(values, 1, 2);
    ASSERT_EQ(slice.size(), 2);
    EXPECT_EQ(slice[0], 1);
    EXPECT_EQ(slice[1], 1);

    Queue<int> queue;
    queue.enqueue(7);
    EXPECT_EQ(queue.dequeue(), 7);

    Stack<int> stack;
    stack.push(9);
    EXPECT_EQ(stack.pop(), 9);
}

/// 验证无值哨兵、主要构造方式、const-correct 双向迭代器与反向遍历。
TEST(IndustrialListTest, ConstructorsAndBidirectionalIteratorsMatchStdListShape) {
    IndustrialList<int> empty;
    EXPECT_TRUE(empty.empty());

    IndustrialList<int> defaultValues(3);
    EXPECT_EQ(defaultValues.size(), 3U);
    EXPECT_EQ(defaultValues.front(), 0);

    IndustrialList<int> filled(3, 7);
    int source[] = {1, 2, 3};
    IndustrialList<int> ranged(source, source + 3);
    IndustrialList<int> listed{4, 5, 6};

    EXPECT_EQ(filled.back(), 7);
    EXPECT_EQ(ranged.front(), 1);
    EXPECT_EQ(listed.back(), 6);

    static_assert(
        std::is_same<
            decltype(*std::declval<const IndustrialList<int>&>().begin()),
            const int&
        >::value,
        "const List must expose read-only iteration"
    );

    IndustrialList<int>::iterator tail = listed.end();
    --tail;
    EXPECT_EQ(*tail, 6);
    EXPECT_EQ(*listed.rbegin(), 6);
}

/// 验证复制拥有独立节点链，移动操作正确转移所有权并清空源链表。
TEST(IndustrialListTest, CopyAndMoveSemanticsOwnIndependentNodeChains) {
    IndustrialList<int> original{1, 2, 3};
    IndustrialList<int> copied(original);
    copied.front() = 9;

    EXPECT_EQ(original.front(), 1);
    EXPECT_EQ(copied.front(), 9);

    IndustrialList<int> copyAssigned;
    copyAssigned = original;
    EXPECT_EQ(copyAssigned, original);

    IndustrialList<int> moved(std::move(copied));
    EXPECT_EQ(moved.size(), 3U);
    EXPECT_TRUE(copied.empty());

    IndustrialList<int> moveAssigned;
    moveAssigned = std::move(copyAssigned);
    EXPECT_EQ(moveAssigned, original);
    EXPECT_TRUE(copyAssigned.empty());
}

/// 验证节点插入不会使既有元素迭代器失效，erase 只失效被删节点。
TEST(IndustrialListTest, InsertAndErasePreserveUnrelatedIterators) {
    IndustrialList<int> values{1, 3};
    IndustrialList<int>::iterator stable = values.begin();
    IndustrialList<int>::iterator position = values.end();
    --position;

    IndustrialList<int>::iterator inserted = values.insert(position, 2);
    EXPECT_EQ(*stable, 1);
    EXPECT_EQ(*inserted, 2);

    IndustrialList<int>::iterator successor = values.erase(inserted);
    EXPECT_EQ(*stable, 1);
    EXPECT_EQ(*successor, 3);
    EXPECT_EQ(values.size(), 2U);
}

/// 验证 move-only 且不可默认构造元素可参与原地构造、排序和删除。
TEST(IndustrialListTest, MoveOnlyAndNonDefaultConstructibleValuesAreSupported) {
    IndustrialList<MoveOnlyValue> values;
    values.emplace_back(3);
    values.emplace_front(1);
    values.emplace(values.end(), 2);

    values.sort([](const MoveOnlyValue& left, const MoveOnlyValue& right) {
        return left.value < right.value;
    });

    EXPECT_EQ(values.front().value, 1);
    EXPECT_EQ(values.back().value, 3);
    values.pop_front();
    EXPECT_EQ(values.front().value, 2);
}

/// 验证构造失败不会提交 size 或链接，erase 与 clear 会及时析构元素。
TEST(IndustrialListTest, NodeConstructionRollsBackAndEraseDestroysImmediately) {
    ThrowingValue::live = 0;
    ThrowingValue::failConstruction = false;

    {
        IndustrialList<ThrowingValue> values;
        values.emplace_back(1);
        const int liveBeforeFailure = ThrowingValue::live;

        ThrowingValue::failConstruction = true;
        EXPECT_THROW(values.emplace_back(2), std::runtime_error);
        ThrowingValue::failConstruction = false;

        EXPECT_EQ(values.size(), 1U);
        EXPECT_EQ(values.front().value, 1);
        EXPECT_EQ(ThrowingValue::live, liveBeforeFailure);

        values.erase(values.begin());
        EXPECT_EQ(ThrowingValue::live, 0);
        values.emplace_back(3);
        values.clear();
        EXPECT_EQ(ThrowingValue::live, 0);
    }

    EXPECT_EQ(ThrowingValue::live, 0);
}

/// 验证每个业务节点均通过 rebind allocator 成对申请释放，复制赋值传播 allocator。
TEST(IndustrialListTest, StatefulAllocatorRebindAndPropagationAreRespected) {
    AllocationState firstState;
    AllocationState secondState;
    typedef CountingAllocator<int> Allocator;

    {
        dsa::container::List<int, Allocator> source{Allocator(&firstState)};
        source.push_back(1);
        source.push_back(2);

        dsa::container::List<int, Allocator> target{Allocator(&secondState)};
        target.push_back(9);
        target = source;

        EXPECT_EQ(target.get_allocator().state, &firstState);
        EXPECT_EQ(target, source);
    }

    EXPECT_EQ(firstState.allocations, firstState.deallocations);
    EXPECT_EQ(secondState.allocations, secondState.deallocations);
}

/// 验证 splice 常数时间保留节点地址，并显式拒绝不同 allocator 的节点所有权转移。
TEST(IndustrialListTest, SplicePreservesNodeIdentityAndChecksAllocatorBoundary) {
    IndustrialList<int> left{1, 4};
    IndustrialList<int> right{2, 3};
    IndustrialList<int>::iterator moved = right.begin();
    int* movedAddress = std::addressof(*moved);

    IndustrialList<int>::iterator position = left.end();
    --position;
    left.splice(position, right, moved);

    EXPECT_EQ(std::addressof(*iteratorAt(left, 1)), movedAddress);
    EXPECT_EQ(left.size(), 3U);
    EXPECT_EQ(right.size(), 1U);

    AllocationState firstState;
    AllocationState secondState;
    typedef CountingAllocator<int> Allocator;
    dsa::container::List<int, Allocator> first{Allocator(&firstState)};
    dsa::container::List<int, Allocator> second{Allocator(&secondState)};
    first.push_back(1);
    second.push_back(2);

    EXPECT_THROW(first.splice(first.end(), second), std::logic_error);
    EXPECT_EQ(first.size(), 1U);
    EXPECT_EQ(second.size(), 1U);
}

/// 验证 unique、remove、reverse、merge 和比较异常后的节点保全。
TEST(IndustrialListTest, LinkedListSpecificAlgorithmsRelinkWithoutLosingNodes) {
    IndustrialList<int> values{3, 3, 1, 2, 2};
    values.unique();
    values.remove(1);
    values.reverse();
    values.sort();

    const int expected[] = {2, 3};
    int index = 0;
    for (IndustrialList<int>::const_iterator current = values.begin();
         current != values.end();
         ++current) {
        EXPECT_EQ(*current, expected[index++]);
    }

    IndustrialList<int> odds{1, 3, 5};
    IndustrialList<int> evens{0, 2, 4, 6};
    odds.merge(evens);
    EXPECT_TRUE(evens.empty());
    index = 0;
    for (IndustrialList<int>::const_iterator current = odds.begin();
         current != odds.end();
         ++current) {
        EXPECT_EQ(*current, index++);
    }

    IndustrialList<int> throwingSort{5, 1, 4, 2, 3};
    int remainingComparisons = 2;
    EXPECT_THROW(
        throwingSort.sort([&remainingComparisons](int left, int right) {
            if (remainingComparisons-- == 0)
                throw std::runtime_error("comparison failed");
            return left < right;
        }),
        std::runtime_error
    );
    EXPECT_EQ(throwingSort.size(), 5U);
    throwingSort.sort();
    EXPECT_EQ(throwingSort.front(), 1);
    EXPECT_EQ(throwingSort.back(), 5);
}

/// 验证工业 List 可直接接入 Search、Sequence 和 Sort 算法族。
TEST(IndustrialListTest, RepositoryAlgorithmsAcceptBidirectionalIterators) {
    IndustrialList<int> values{3, 1, 2, 2};

    IndustrialList<int>::iterator found = dsa::algorithm::findLast(
        values.begin(),
        values.end(),
        2
    );
    ASSERT_NE(found, values.end());
    EXPECT_EQ(*found, 2);
    EXPECT_EQ(
        dsa::algorithm::disorderCount(values.begin(), values.end()),
        1
    );

    int sum = 0;
    dsa::algorithm::forEach(values.begin(), values.end(), [&sum](int value) {
        sum += value;
    });
    EXPECT_EQ(sum, 8);

    IndustrialList<int>::iterator logicalEnd = dsa::algorithm::uniqueAdjacent(
        values.begin(),
        values.end()
    );
    values.erase(logicalEnd, values.end());
    EXPECT_EQ(values.size(), 3U);

    Sort(values, SortStrategy::InsertionSort);
    EXPECT_EQ(values.front(), 1);
    EXPECT_EQ(values.back(), 3);
    EXPECT_THROW(Sort(values, SortStrategy::QuickSort), std::invalid_argument);
}

/// 通过长操作序列与 std::list 对拍共同语义，覆盖首尾插删、中间插删、remove、reverse 和 sort。
TEST(IndustrialListTest, DifferentialOperationsMatchStdList) {
    IndustrialList<int> actual;
    std::list<int> expected;

    for (int step = 0; step < 2000; ++step) {
        switch (step % 9) {
        case 0:
            actual.push_back(step);
            expected.push_back(step);
            break;
        case 1:
            actual.push_front(-step);
            expected.push_front(-step);
            break;
        case 2:
            if (!expected.empty()) {
                actual.pop_back();
                expected.pop_back();
            }
            break;
        case 3:
            if (!expected.empty()) {
                actual.pop_front();
                expected.pop_front();
            }
            break;
        case 4: {
            const std::size_t index = expected.empty() ? 0 : expected.size() / 2;
            actual.insert(iteratorAt(actual, index), step * 2);
            expected.insert(iteratorAt(expected, index), step * 2);
            break;
        }
        case 5:
            if (!expected.empty()) {
                const std::size_t index = expected.size() / 2;
                actual.erase(iteratorAt(actual, index));
                expected.erase(iteratorAt(expected, index));
            }
            break;
        case 6:
            actual.remove(step - 20);
            expected.remove(step - 20);
            break;
        case 7:
            actual.reverse();
            expected.reverse();
            break;
        case 8:
            actual.sort();
            expected.sort();
            break;
        }

        expectSameSequence(actual, expected);
    }
}
