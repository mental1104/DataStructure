#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "dsa/algorithm/Search.h"
#include "dsa/container/bitmap/Bitmap.h"

namespace {

/// 记录测试 allocator 的申请和释放次数。
struct AllocationState {
    int allocations;
    int deallocations;

    /// 将申请与释放计数初始化为零。
    AllocationState()
        : allocations(0), deallocations(0) {
    }
};

/// 用于验证 allocator propagation 和不等 allocator 交换路径的测试 allocator。
template<typename T, bool Propagate>
class CountingAllocator {
public:
    typedef T value_type;
    typedef std::integral_constant<bool, Propagate> propagate_on_container_copy_assignment;
    typedef std::integral_constant<bool, Propagate> propagate_on_container_move_assignment;
    typedef std::integral_constant<bool, Propagate> propagate_on_container_swap;

    AllocationState* state;

    /// 创建未绑定统计状态的 allocator。
    CountingAllocator() noexcept
        : state(nullptr) {
    }

    /// 创建绑定指定统计状态的 allocator。
    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {
    }

    /// 从其他 value_type 的同策略 allocator 转换构造。
    template<typename U>
    CountingAllocator(const CountingAllocator<U, Propagate>& other) noexcept
        : state(other.state) {
    }

    /// 申请 count 个 T 的存储并记录次数。
    T* allocate(std::size_t count) {
        if (state != nullptr)
            ++state->allocations;
        return std::allocator<T>().allocate(count);
    }

    /// 释放存储并记录次数。
    void deallocate(T* memory, std::size_t count) noexcept {
        if (state != nullptr)
            ++state->deallocations;
        std::allocator<T>().deallocate(memory, count);
    }

    /// 为 allocator_traits 提供其他 value_type 的 rebinding 类型。
    template<typename U>
    struct rebind {
        typedef CountingAllocator<U, Propagate> other;
    };
};

/// 判断两个测试 allocator 是否绑定同一统计状态。
template<typename T, typename U, bool Propagate>
bool operator==(
    const CountingAllocator<T, Propagate>& left,
    const CountingAllocator<U, Propagate>& right
) noexcept {
    return left.state == right.state;
}

/// 判断两个测试 allocator 是否绑定不同统计状态。
template<typename T, typename U, bool Propagate>
bool operator!=(
    const CountingAllocator<T, Propagate>& left,
    const CountingAllocator<U, Propagate>& right
) noexcept {
    return !(left == right);
}

/// 对比工业位图和 vector<bool> 的全部公共逻辑状态。
void expectEquivalent(
    const dsa::container::Bitmap<>& bitmap,
    const std::vector<bool>& model
) {
    ASSERT_EQ(bitmap.size(), model.size());

    std::size_t expectedCount = 0;
    for (std::size_t index = 0; index < model.size(); ++index) {
        EXPECT_EQ(bitmap[index], model[index]);
        if (model[index])
            ++expectedCount;
    }

    EXPECT_EQ(bitmap.count(), expectedCount);
    EXPECT_EQ(bitmap.any(), expectedCount != 0);
    EXPECT_EQ(bitmap.none(), expectedCount == 0);
    EXPECT_EQ(bitmap.all(), expectedCount == model.size());
}

} // namespace

TEST(ContainerBitmapTest, HandlesWordBoundariesAndFindOperations) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap bitmap(130, false);

    bitmap.set(0).set(63).set(64).set(129);

    EXPECT_EQ(bitmap.count(), 4u);
    EXPECT_EQ(bitmap.find_first(), 0u);
    EXPECT_EQ(bitmap.find_next(0), 63u);
    EXPECT_EQ(bitmap.find_next(63), 64u);
    EXPECT_EQ(bitmap.find_next(64), 129u);
    EXPECT_EQ(bitmap.find_next(129), Bitmap::npos);
}

TEST(ContainerBitmapTest, ProxyAndIteratorsWorkWithStandardAlgorithms) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap bitmap(130, false);
    bitmap.set(0).set(63).set(64).set(129);

    bitmap[1] = true;
    swap(bitmap[1], bitmap[2]);
    EXPECT_FALSE(bitmap[1]);
    EXPECT_TRUE(bitmap[2]);

    std::reverse(bitmap.begin(), bitmap.end());
    EXPECT_TRUE(bitmap[0]);
    EXPECT_TRUE(bitmap[65]);
    EXPECT_TRUE(bitmap[66]);
    EXPECT_TRUE(bitmap[127]);
    EXPECT_TRUE(bitmap[129]);

    Bitmap::iterator lastTrue = dsa::algorithm::findLast(
        bitmap.begin(), bitmap.end(), true
    );
    ASSERT_NE(lastTrue, bitmap.end());
    EXPECT_EQ(lastTrue - bitmap.begin(), 129);

    const Bitmap& view = bitmap;
    EXPECT_EQ(
        static_cast<std::size_t>(std::count(view.cbegin(), view.cend(), true)),
        view.count()
    );
}

TEST(ContainerBitmapTest, SupportsCopyMoveRangeAndInitializerListConstruction) {
    typedef dsa::container::Bitmap<> Bitmap;
    const bool sourceValues[] = {true, false, true, true};

    Bitmap ranged(sourceValues, sourceValues + 4);
    Bitmap listed({true, false, true, true});
    EXPECT_EQ(ranged, listed);

    Bitmap copied = ranged;
    copied.reset(0);
    EXPECT_TRUE(ranged[0]);
    EXPECT_FALSE(copied[0]);

    Bitmap moved = std::move(copied);
    EXPECT_FALSE(moved[0]);
    EXPECT_TRUE(copied.empty());

    Bitmap assigned;
    assigned = {false, true, false};
    EXPECT_EQ(assigned.to_string(), "010");
}

TEST(ContainerBitmapTest, ResizeAndCapacityPreserveExistingBits) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap bitmap(65, false);
    bitmap.set(0).set(64);

    bitmap.reserve(1024);
    EXPECT_GE(bitmap.capacity(), 1024u);
    EXPECT_TRUE(bitmap[0]);
    EXPECT_TRUE(bitmap[64]);

    bitmap.resize(130, true);
    EXPECT_TRUE(bitmap[0]);
    EXPECT_TRUE(bitmap[64]);
    for (std::size_t index = 65; index < 130; ++index)
        EXPECT_TRUE(bitmap[index]);

    bitmap.resize(1);
    EXPECT_TRUE(bitmap[0]);
    bitmap.resize(130, false);
    for (std::size_t index = 1; index < 130; ++index)
        EXPECT_FALSE(bitmap[index]);

    bitmap.shrink_to_fit();
    EXPECT_GE(bitmap.capacity(), bitmap.size());
}

TEST(ContainerBitmapTest, BitwiseOperationsRequireEqualSizes) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap left(130, false);
    Bitmap right(130, false);
    left.set(1).set(64);
    right.set(64).set(129);

    EXPECT_EQ((left & right).count(), 1u);
    EXPECT_EQ((left | right).count(), 3u);
    EXPECT_EQ((left ^ right).count(), 2u);
    EXPECT_EQ((~left).count(), 128u);

    Bitmap mismatched(1, false);
    EXPECT_THROW(left &= mismatched, std::invalid_argument);
    EXPECT_THROW(left |= mismatched, std::invalid_argument);
    EXPECT_THROW(left ^= mismatched, std::invalid_argument);
}

TEST(ContainerBitmapTest, CheckedOperationsRejectInvalidPositions) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap bitmap(8, false);

    EXPECT_THROW(bitmap.test(8), std::out_of_range);
    EXPECT_THROW(bitmap.set(8), std::out_of_range);
    EXPECT_THROW(bitmap.reset(8), std::out_of_range);
    EXPECT_THROW(bitmap.flip(8), std::out_of_range);

    Bitmap empty;
    EXPECT_THROW(empty.pop_back(), std::out_of_range);
}

TEST(ContainerBitmapTest, MatchesVectorBoolAcrossRandomMutationSequence) {
    typedef dsa::container::Bitmap<> Bitmap;
    Bitmap bitmap;
    std::vector<bool> model;
    std::mt19937 random(20260721u);

    for (int step = 0; step < 10000; ++step) {
        const int operation = static_cast<int>(random() % 8u);
        if (operation == 0) {
            const bool value = (random() & 1u) != 0;
            bitmap.push_back(value);
            model.push_back(value);
        } else if (operation == 1 && !model.empty()) {
            bitmap.pop_back();
            model.pop_back();
        } else if (operation == 2) {
            const std::size_t size = random() % 257u;
            const bool value = (random() & 1u) != 0;
            bitmap.resize(size, value);
            model.resize(size, value);
        } else if (operation == 3 && !model.empty()) {
            const std::size_t index = random() % model.size();
            bitmap.set(index);
            model[index] = true;
        } else if (operation == 4 && !model.empty()) {
            const std::size_t index = random() % model.size();
            bitmap.reset(index);
            model[index] = false;
        } else if (operation == 5 && !model.empty()) {
            const std::size_t index = random() % model.size();
            bitmap.flip(index);
            model[index] = !model[index];
        } else if (operation == 6) {
            bitmap.flip();
            for (std::size_t index = 0; index < model.size(); ++index)
                model[index] = !model[index];
        } else if (operation == 7) {
            bitmap.reset();
            std::fill(model.begin(), model.end(), false);
        }
        expectEquivalent(bitmap, model);
    }
}

TEST(ContainerBitmapTest, StatefulAllocatorBalancesAllocations) {
    typedef CountingAllocator<bool, true> Allocator;
    typedef dsa::container::Bitmap<Allocator> Bitmap;
    AllocationState firstState;
    AllocationState secondState;

    {
        Bitmap first(130, true, Allocator(&firstState));
        Bitmap second(65, false, Allocator(&secondState));
        second.set(64);

        first = second;
        EXPECT_EQ(first.get_allocator().state, &secondState);

        Bitmap moved(1, false, Allocator(&firstState));
        moved = std::move(first);
        EXPECT_EQ(moved.get_allocator().state, &secondState);
        EXPECT_TRUE(first.empty());
    }

    EXPECT_EQ(firstState.allocations, firstState.deallocations);
    EXPECT_EQ(secondState.allocations, secondState.deallocations);
}

TEST(ContainerBitmapTest, UnequalNonPropagatingAllocatorsKeepOwnershipDuringSwap) {
    typedef CountingAllocator<bool, false> Allocator;
    typedef dsa::container::Bitmap<Allocator> Bitmap;
    AllocationState leftState;
    AllocationState rightState;

    {
        Bitmap left(65, true, Allocator(&leftState));
        Bitmap right(2, false, Allocator(&rightState));
        right.set(1);

        left.swap(right);

        EXPECT_EQ(left.get_allocator().state, &leftState);
        EXPECT_EQ(right.get_allocator().state, &rightState);
        EXPECT_EQ(left.size(), 2u);
        EXPECT_FALSE(left[0]);
        EXPECT_TRUE(left[1]);
        EXPECT_EQ(right.size(), 65u);
        EXPECT_TRUE(right.all());
    }

    EXPECT_EQ(leftState.allocations, leftState.deallocations);
    EXPECT_EQ(rightState.allocations, rightState.deallocations);
}
