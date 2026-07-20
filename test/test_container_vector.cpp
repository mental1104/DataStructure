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

struct MoveOnlyValue {
    int value;

    explicit MoveOnlyValue(int current)
        : value(current) {}

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
};

int TrackedValue::live = 0;

struct ThrowingValue {
    static int live;
    static int copies;
    static int throwAfter;

    int value;

    explicit ThrowingValue(int current = 0)
        : value(current) {
        ++live;
    }

    ThrowingValue(const ThrowingValue& other)
        : value(other.value) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("copy failed");
        ++live;
    }

    ThrowingValue(ThrowingValue&& other) noexcept(false)
        : value(other.value) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("move failed");
        other.value = -1;
        ++live;
    }

    ThrowingValue& operator=(const ThrowingValue& other) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("copy assignment failed");
        value = other.value;
        return *this;
    }

    ThrowingValue& operator=(ThrowingValue&& other) noexcept(false) {
        if (throwAfter >= 0 && copies++ >= throwAfter)
            throw std::runtime_error("move assignment failed");
        value = other.value;
        other.value = -1;
        return *this;
    }

    ~ThrowingValue() {
        --live;
    }
};

int ThrowingValue::live = 0;
int ThrowingValue::copies = 0;
int ThrowingValue::throwAfter = -1;

struct AllocationState {
    int allocations;
    int deallocations;

    AllocationState()
        : allocations(0), deallocations(0) {}
};

template<typename T>
class CountingAllocator {
public:
    typedef T value_type;
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

    AllocationState* state;

    CountingAllocator() noexcept
        : state(nullptr) {}

    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {}

    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : state(other.state) {}

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

} // namespace

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

TEST(IndustrialVectorTest, InputIteratorConstructorConsumesSinglePassRange) {
    std::istringstream stream("1 2 3 4");
    std::istream_iterator<int> first(stream);
    std::istream_iterator<int> last;
    IndustrialVector<int> values(first, last);

    ASSERT_EQ(values.size(), 4U);
    EXPECT_EQ(values.front(), 1);
    EXPECT_EQ(values.back(), 4);
}

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

TEST(IndustrialVectorTest, ExistingSortFacadeAcceptsIndustrialVector) {
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
