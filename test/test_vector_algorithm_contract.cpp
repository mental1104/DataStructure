#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

#include "dsa/core/vector/VectorAlgorithm.h"

namespace {

struct CompleteStorage {
    typedef std::size_t size_type;

    size_type size() const;
    size_type maxSize() const;
    void ensureCapacity(size_type required);
    void openGap(size_type position, size_type count, size_type oldSize);

    template<typename Value>
    void writeGap(size_type position, Value&& value);

    void rollbackGap(size_type position, size_type count, size_type oldSize);
    void closeGap(size_type first, size_type last, size_type oldSize);
    void commitSize(size_type newSize);
    void afterErase();
};

struct MissingRollbackStorage {
    typedef std::size_t size_type;

    size_type size() const;
    size_type maxSize() const;
    void ensureCapacity(size_type required);
    void openGap(size_type position, size_type count, size_type oldSize);

    template<typename Value>
    void writeGap(size_type position, Value&& value);

    void closeGap(size_type first, size_type last, size_type oldSize);
    void commitSize(size_type newSize);
    void afterErase();
};

static_assert(
    dsa::core::detail::HasVectorStorageContract<CompleteStorage>::value,
    "完整 Storage 应满足 VectorAlgorithm 合同"
);

static_assert(
    !dsa::core::detail::HasVectorStorageContract<MissingRollbackStorage>::value,
    "缺少 rollbackGap 的 Storage 不应通过合同检测"
);

static_assert(
    dsa::core::detail::HasVectorStorageWriteGap<CompleteStorage, int&&>::value,
    "完整 Storage 应支持 writeGap"
);

} // namespace

/// 验证默认 maximum 使用 size_type 理论上限，并保持几何增长。
TEST(VectorAlgorithmContractTest, RecommendCapacityProvidesDefaultMaximum) {
    typedef dsa::core::VectorAlgorithm<CompleteStorage> Algorithm;

    EXPECT_EQ(Algorithm::recommendCapacity(4U, 5U, 1U), 8U);
    EXPECT_EQ(Algorithm::recommendCapacity(4U, 20U, 1U), 20U);
}

/// 验证最小容量已覆盖需求时不会被无意义地继续扩大到上限。
TEST(VectorAlgorithmContractTest, RecommendCapacityKeepsSatisfiedMinimum) {
    typedef dsa::core::VectorAlgorithm<CompleteStorage> Algorithm;

    EXPECT_EQ(Algorithm::recommendCapacity(0U, 1U, 8U, 10U), 8U);
}

/// 验证 required 超过显式 maximum 时抛出 length_error。
TEST(VectorAlgorithmContractTest, RecommendCapacityRejectsMaximumOverflow) {
    typedef dsa::core::VectorAlgorithm<CompleteStorage> Algorithm;

    EXPECT_THROW(
        Algorithm::recommendCapacity(8U, 11U, 1U, 10U),
        std::length_error
    );
}
