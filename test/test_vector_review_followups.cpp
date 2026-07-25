#include <gtest/gtest.h>

#include <stdexcept>

#include "Vector.h"

/// 验证空教学 Vector 不会解引用 majorityCandidate 返回的尾后迭代器。
TEST(VectorReviewFollowupTest, EmptyMajorityCandidateThrows) {
    Vector<int> values;

    EXPECT_THROW(values.majEleCandidate(), std::logic_error);
}

/// 验证多数候选筛选保留 Boyer-Moore 的候选语义。
TEST(VectorReviewFollowupTest, MajorityCandidateReturnsExistingMajority) {
    int source[] = {1, 2, 1, 1, 3, 1, 1};
    Vector<int> values(source, 7);

    EXPECT_EQ(values.majEleCandidate(), 1);
}

/// 验证前置递增返回当前迭代器，后置递增返回推进前的副本。
TEST(VectorReviewFollowupTest, IteratorPrefixAndPostfixKeepDistinctSemantics) {
    int source[] = {4, 5, 6};
    Vector<int> values(source, 3);

    Vector<int>::iterator current = values.begin();
    Vector<int>::iterator previous = current++;

    EXPECT_EQ(*previous, 4);
    EXPECT_EQ(*current, 5);

    Vector<int>::iterator& advanced = ++current;
    EXPECT_EQ(*current, 6);
    EXPECT_TRUE(&advanced == &current);
}

/// 验证教材查找门面把 upper-bound 结果转换为最后一个不大于目标值的位置。
TEST(VectorReviewFollowupTest, SearchFacadesReturnLastNotGreaterPosition) {
    int source[] = {1, 3, 3, 5};
    Vector<int> values(source, 4);

    EXPECT_EQ(values.search(3, 0, values.size()), 2);
    EXPECT_EQ(values.binSearch(source, 3, 0, 4), 2);
    EXPECT_EQ(values.fibSearch(source, 3, 0, 4), 2);
}

/// 验证无序去重保持首次出现顺序。
TEST(VectorReviewFollowupTest, DeduplicateKeepsFirstOccurrenceOrder) {
    int source[] = {3, 1, 3, 2, 1};
    Vector<int> values(source, 5);

    EXPECT_EQ(values.deduplicate(), 2);
    ASSERT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 3);
    EXPECT_EQ(values[1], 1);
    EXPECT_EQ(values[2], 2);
}
