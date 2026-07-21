#include "gtest/gtest.h"

#include <dsa/algorithm/Sort.h>

#include <algorithm>
#include <forward_list>
#include <functional>
#include <list>
#include <random>
#include <vector>

namespace {

/// 使用相邻元素检查任意前向区间是否满足指定顺序。
template<typename Iterator, typename Compare>
bool isSortedRange(Iterator first, Iterator last, Compare compare) {
    if (first == last)
        return true;

    Iterator current = first;
    Iterator next = current;
    ++next;
    while (next != last) {
        if (compare(*next, *current))
            return false;
        ++current;
        ++next;
    }
    return true;
}

/// 验证 iterator-first 入口为全部随机访问策略提供仓库自实现。
TEST(SortAlgorithmTest, RandomAccessStrategiesSortStdVector) {
    const dsa::algorithm::SortStrategy strategies[] = {
        dsa::algorithm::SortStrategy::BubbleSort,
        dsa::algorithm::SortStrategy::SelectionSort,
        dsa::algorithm::SortStrategy::InsertionSort,
        dsa::algorithm::SortStrategy::ShellSort,
        dsa::algorithm::SortStrategy::MergeSort,
        dsa::algorithm::SortStrategy::MergeSortB,
        dsa::algorithm::SortStrategy::QuickSort,
        dsa::algorithm::SortStrategy::Quick3way,
        dsa::algorithm::SortStrategy::QuickSortB,
        dsa::algorithm::SortStrategy::HeapSort
    };

    for (const dsa::algorithm::SortStrategy strategy : strategies) {
        std::vector<int> values{5, 3, 8, 1, 9, 2, 2, 5, 0, -1};
        EXPECT_TRUE(dsa::algorithm::sort(values.begin(), values.end(), strategy));
        EXPECT_TRUE(isSortedRange(values.begin(), values.end(), std::less<int>()))
            << "排序策略失败: " << static_cast<int>(strategy);
    }
}

/// 用固定随机种子和 std::sort 对拍，覆盖空区间、重复值和不同长度。
TEST(SortAlgorithmTest, RandomAccessStrategiesMatchStdSort) {
    const dsa::algorithm::SortStrategy strategies[] = {
        dsa::algorithm::SortStrategy::BubbleSort,
        dsa::algorithm::SortStrategy::SelectionSort,
        dsa::algorithm::SortStrategy::InsertionSort,
        dsa::algorithm::SortStrategy::ShellSort,
        dsa::algorithm::SortStrategy::MergeSort,
        dsa::algorithm::SortStrategy::MergeSortB,
        dsa::algorithm::SortStrategy::QuickSort,
        dsa::algorithm::SortStrategy::Quick3way,
        dsa::algorithm::SortStrategy::QuickSortB,
        dsa::algorithm::SortStrategy::HeapSort
    };
    std::mt19937 engine(20260721U);
    std::uniform_int_distribution<int> distribution(-20, 20);

    for (int size = 0; size <= 40; ++size) {
        std::vector<int> source(static_cast<std::size_t>(size));
        for (int& value : source)
            value = distribution(engine);

        std::vector<int> expected = source;
        std::sort(expected.begin(), expected.end());
        for (const dsa::algorithm::SortStrategy strategy : strategies) {
            std::vector<int> actual = source;
            dsa::algorithm::sort(actual.begin(), actual.end(), strategy);
            EXPECT_EQ(actual, expected)
                << "对拍失败，size=" << size
                << ", strategy=" << static_cast<int>(strategy);
        }
    }
}

struct StableRecord {
    int key;
    int originalOrder;
};

struct StableRecordLess {
    /// 只比较 key，用 originalOrder 验证相等元素是否保持输入顺序。
    bool operator()(const StableRecord& left, const StableRecord& right) const {
        return left.key < right.key;
    }
};

/// 验证两种归并策略都保持相等键的原始顺序。
TEST(SortAlgorithmTest, MergeStrategiesAreStable) {
    const dsa::algorithm::SortStrategy strategies[] = {
        dsa::algorithm::SortStrategy::MergeSort,
        dsa::algorithm::SortStrategy::MergeSortB
    };

    for (const dsa::algorithm::SortStrategy strategy : strategies) {
        std::vector<StableRecord> values{
            {2, 0}, {1, 1}, {2, 2}, {1, 3}, {2, 4}
        };
        dsa::algorithm::sort(
            values.begin(),
            values.end(),
            strategy,
            StableRecordLess()
        );

        ASSERT_EQ(values.size(), 5U);
        EXPECT_EQ(values[0].originalOrder, 1);
        EXPECT_EQ(values[1].originalOrder, 3);
        EXPECT_EQ(values[2].originalOrder, 0);
        EXPECT_EQ(values[3].originalOrder, 2);
        EXPECT_EQ(values[4].originalOrder, 4);
    }
}

struct MoveOnlyRecord {
    int key;

    explicit MoveOnlyRecord(int value)
        : key(value) {
    }

    MoveOnlyRecord(const MoveOnlyRecord&) = delete;
    MoveOnlyRecord& operator=(const MoveOnlyRecord&) = delete;

    MoveOnlyRecord(MoveOnlyRecord&& other) noexcept
        : key(other.key) {
        other.key = -1;
    }

    MoveOnlyRecord& operator=(MoveOnlyRecord&& other) noexcept {
        key = other.key;
        other.key = -1;
        return *this;
    }
};

struct MoveOnlyRecordLess {
    /// 仅通过 key 比较 move-only 元素。
    bool operator()(const MoveOnlyRecord& left, const MoveOnlyRecord& right) const {
        return left.key < right.key;
    }
};

/// 验证两种归并实现不要求元素可复制或默认构造。
TEST(SortAlgorithmTest, MergeStrategiesSupportMoveOnlyValues) {
    const dsa::algorithm::SortStrategy strategies[] = {
        dsa::algorithm::SortStrategy::MergeSort,
        dsa::algorithm::SortStrategy::MergeSortB
    };

    for (const dsa::algorithm::SortStrategy strategy : strategies) {
        std::vector<MoveOnlyRecord> values;
        values.emplace_back(3);
        values.emplace_back(1);
        values.emplace_back(2);
        dsa::algorithm::sort(
            values.begin(),
            values.end(),
            strategy,
            MoveOnlyRecordLess()
        );

        ASSERT_EQ(values.size(), 3U);
        EXPECT_EQ(values[0].key, 1);
        EXPECT_EQ(values[1].key, 2);
        EXPECT_EQ(values[2].key, 3);
    }
}

/// 验证全部随机访问策略都只要求元素可移动，不隐式复制枢轴或临时值。
TEST(SortAlgorithmTest, RandomAccessStrategiesSupportMoveOnlyValues) {
    const dsa::algorithm::SortStrategy strategies[] = {
        dsa::algorithm::SortStrategy::BubbleSort,
        dsa::algorithm::SortStrategy::SelectionSort,
        dsa::algorithm::SortStrategy::InsertionSort,
        dsa::algorithm::SortStrategy::ShellSort,
        dsa::algorithm::SortStrategy::MergeSort,
        dsa::algorithm::SortStrategy::MergeSortB,
        dsa::algorithm::SortStrategy::QuickSort,
        dsa::algorithm::SortStrategy::Quick3way,
        dsa::algorithm::SortStrategy::QuickSortB,
        dsa::algorithm::SortStrategy::HeapSort
    };

    for (const dsa::algorithm::SortStrategy strategy : strategies) {
        std::vector<MoveOnlyRecord> values;
        values.emplace_back(4);
        values.emplace_back(1);
        values.emplace_back(3);
        values.emplace_back(2);

        EXPECT_TRUE(dsa::algorithm::sort(
            values.begin(),
            values.end(),
            strategy,
            MoveOnlyRecordLess()
        ));
        ASSERT_EQ(values.size(), 4U);
        EXPECT_EQ(values[0].key, 1);
        EXPECT_EQ(values[1].key, 2);
        EXPECT_EQ(values[2].key, 3);
        EXPECT_EQ(values[3].key, 4);
    }
}

/// 验证比较器由调用方控制最终顺序，而不是算法内部固定使用 operator<。
TEST(SortAlgorithmTest, ComparatorControlsOrdering) {
    std::vector<int> values{1, 5, 2, 4, 3, 3};

    dsa::algorithm::sort(
        values.begin(),
        values.end(),
        dsa::algorithm::SortStrategy::Quick3way,
        std::greater<int>()
    );

    EXPECT_TRUE(isSortedRange(values.begin(), values.end(), std::greater<int>()));
    EXPECT_EQ(values.front(), 5);
    EXPECT_EQ(values.back(), 1);
}

/// 验证前向迭代器可使用冒泡和选择排序，并拒绝要求回退能力的策略。
TEST(SortAlgorithmTest, ForwardIteratorsUseCapabilityBasedDispatch) {
    std::forward_list<int> values{4, 1, 3, 2};

    dsa::algorithm::sort(
        values.begin(),
        values.end(),
        dsa::algorithm::SortStrategy::SelectionSort
    );
    EXPECT_TRUE(isSortedRange(values.begin(), values.end(), std::less<int>()));

    EXPECT_THROW(
        dsa::algorithm::sort(
            values.begin(),
            values.end(),
            dsa::algorithm::SortStrategy::InsertionSort
        ),
        std::invalid_argument
    );
}

/// 验证双向迭代器支持适用策略，并明确拒绝需要随机访问能力的策略。
TEST(SortAlgorithmTest, BidirectionalIteratorsUseCapabilityBasedDispatch) {
    std::list<int> values{4, 1, 3, 2};

    dsa::algorithm::sort(
        values.begin(),
        values.end(),
        dsa::algorithm::SortStrategy::InsertionSort
    );
    EXPECT_TRUE(isSortedRange(values.begin(), values.end(), std::less<int>()));

    EXPECT_THROW(
        dsa::algorithm::sort(
            values.begin(),
            values.end(),
            dsa::algorithm::SortStrategy::QuickSort
        ),
        std::invalid_argument
    );
}

/// 验证非法枚举继续保持历史 facade 的空操作兼容语义。
TEST(SortAlgorithmTest, UnknownStrategyIsNoOp) {
    std::vector<int> values{3, 1, 2};

    EXPECT_FALSE(dsa::algorithm::sort(
        values.begin(),
        values.end(),
        static_cast<dsa::algorithm::SortStrategy>(999)
    ));

    EXPECT_EQ(values[0], 3);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 2);
}

} // namespace
