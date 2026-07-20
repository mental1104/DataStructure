#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "QuickFind.h"
#include "QuickUnion.h"
#include "WeightedQuickUnion.h"
#include "WeightedQuickUnionwithCompression.h"
#include <dsa/container/uf/UnionFind.h>

namespace {

template<typename UF>
void verifyTeachingValueSemantics() {
    UF original(8);
    original.unite(0, 1);
    original.unite(1, 2);

    UF copied(original);
    copied.unite(2, 3);
    EXPECT_TRUE(copied.connected(0, 3));
    EXPECT_FALSE(original.connected(0, 3));

    UF assigned(2);
    assigned = original;
    assigned.unite(2, 4);
    EXPECT_TRUE(assigned.connected(0, 4));
    EXPECT_FALSE(original.connected(0, 4));

    UF moved(std::move(copied));
    EXPECT_TRUE(moved.connected(0, 3));
    EXPECT_EQ(copied.size(), 0);
    EXPECT_EQ(copied.count(), 0);

    UF moveAssigned(1);
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(moveAssigned.connected(0, 4));
    EXPECT_EQ(assigned.size(), 0);
    EXPECT_EQ(assigned.count(), 0);
}

template<typename UF>
void verifyTeachingBounds() {
    UF uf(4);
    EXPECT_THROW((void)uf.find(-1), std::out_of_range);
    EXPECT_THROW((void)uf.find(4), std::out_of_range);
    EXPECT_THROW(uf.unite(0, 4), std::out_of_range);
}

template<typename UF>
void verifyTeachingFileConstructor(const std::string& path) {
    {
        std::ofstream output(path.c_str());
        ASSERT_TRUE(output.is_open());
        output << "6\n0 1\n1 2\n3 4\n5";
    }

    std::ifstream input(path.c_str());
    ASSERT_TRUE(input.is_open());
    UF uf(input);

    EXPECT_TRUE(uf.connected(0, 2));
    EXPECT_TRUE(uf.connected(3, 4));
    EXPECT_FALSE(uf.connected(0, 3));
    EXPECT_EQ(uf.count(), 3);
    std::remove(path.c_str());
}

class InspectableCompression : public WeightedQuickUnionwithCompression {
public:
    explicit InspectableCompression(int count)
        : WeightedQuickUnionwithCompression(count) {}

    int parentOf(int index) const {
        return _id[index];
    }
};

} // namespace

TEST(UnionFindFamilyTest, TeachingBaseHasVirtualDestructor) {
    static_assert(
        std::has_virtual_destructor<UnionFind>::value,
        "UnionFind must support polymorphic destruction"
    );

    std::unique_ptr<UnionFind> uf(new WeightedQuickUnionwithCompression(4));
    uf->unite(0, 1);
    EXPECT_TRUE(uf->connected(0, 1));
}

TEST(UnionFindFamilyTest, TeachingImplementationsHaveIndependentValueSemantics) {
    verifyTeachingValueSemantics<QuickFind>();
    verifyTeachingValueSemantics<QuickUnion>();
    verifyTeachingValueSemantics<WeightedQuickUnion>();
    verifyTeachingValueSemantics<WeightedQuickUnionwithCompression>();
}

TEST(UnionFindFamilyTest, TeachingImplementationsRejectInvalidIndices) {
    verifyTeachingBounds<QuickFind>();
    verifyTeachingBounds<QuickUnion>();
    verifyTeachingBounds<WeightedQuickUnion>();
    verifyTeachingBounds<WeightedQuickUnionwithCompression>();

    EXPECT_THROW(QuickFind(-1), std::invalid_argument);
    EXPECT_THROW(QuickUnion(-1), std::invalid_argument);
    EXPECT_THROW(WeightedQuickUnion(-1), std::invalid_argument);
    EXPECT_THROW(WeightedQuickUnionwithCompression(-1), std::invalid_argument);
}

TEST(UnionFindFamilyTest, FileConstructorsStopAtIncompletePair) {
    verifyTeachingFileConstructor<QuickFind>("test_union_find_quick_find.txt");
    verifyTeachingFileConstructor<QuickUnion>("test_union_find_quick_union.txt");
    verifyTeachingFileConstructor<WeightedQuickUnion>("test_union_find_weighted.txt");
    verifyTeachingFileConstructor<WeightedQuickUnionwithCompression>(
        "test_union_find_compressed.txt"
    );
}

TEST(UnionFindFamilyTest, PathCompressionFlattensVisitedPath) {
    InspectableCompression uf(8);
    uf.unite(0, 1);
    uf.unite(2, 3);
    uf.unite(0, 2);

    ASSERT_EQ(uf.parentOf(3), 2);
    EXPECT_EQ(uf.find(3), 0);
    EXPECT_EQ(uf.parentOf(3), 0);
}

TEST(UnionFindFamilyTest, TeachingImplementationsAgreeOnRandomSequence) {
    const int elementCount = 64;
    QuickFind quickFind(elementCount);
    QuickUnion quickUnion(elementCount);
    WeightedQuickUnion weighted(elementCount);
    WeightedQuickUnionwithCompression compressed(elementCount);

    std::mt19937 random(20260721U);
    std::uniform_int_distribution<int> index(0, elementCount - 1);

    for (int step = 0; step < 1000; ++step) {
        const int left = index(random);
        const int right = index(random);
        if (step % 3 != 0) {
            quickFind.unite(left, right);
            quickUnion.unite(left, right);
            weighted.unite(left, right);
            compressed.unite(left, right);
        } else {
            const bool expected = quickFind.connected(left, right);
            EXPECT_EQ(quickUnion.connected(left, right), expected);
            EXPECT_EQ(weighted.connected(left, right), expected);
            EXPECT_EQ(compressed.connected(left, right), expected);
        }

        EXPECT_EQ(quickUnion.count(), quickFind.count());
        EXPECT_EQ(weighted.count(), quickFind.count());
        EXPECT_EQ(compressed.count(), quickFind.count());
    }
}

TEST(IndustrialUnionFindTest, SupportsCoreApiAndComponentSizes) {
    dsa::container::UnionFind<> uf(8);

    EXPECT_EQ(uf.size(), 8U);
    EXPECT_EQ(uf.count(), 8U);
    EXPECT_TRUE(uf.unite(0, 1));
    EXPECT_FALSE(uf.unite(1, 0));
    EXPECT_TRUE(uf.unite(2, 3));
    EXPECT_TRUE(uf.unite(0, 2));
    EXPECT_TRUE(uf.connected(1, 3));
    EXPECT_EQ(uf.componentSize(3), 4U);
    EXPECT_EQ(uf.componentCount(), 5U);

    const dsa::container::UnionFind<>& readOnly = uf;
    EXPECT_EQ(readOnly.find(3), uf.find(3));
    EXPECT_TRUE(readOnly.connected(0, 2));
    EXPECT_EQ(readOnly.componentSize(1), 4U);
}

TEST(IndustrialUnionFindTest, CopyAndMoveKeepOwnershipIndependent) {
    dsa::container::UnionFind<> original(6);
    original.unite(0, 1);
    original.unite(1, 2);

    dsa::container::UnionFind<> copied(original);
    copied.unite(2, 3);
    EXPECT_TRUE(copied.connected(0, 3));
    EXPECT_FALSE(original.connected(0, 3));

    dsa::container::UnionFind<> moved(std::move(copied));
    EXPECT_TRUE(moved.connected(0, 3));
    EXPECT_TRUE(copied.empty());
    EXPECT_EQ(copied.count(), 0U);

    dsa::container::UnionFind<> assigned(1);
    assigned = original;
    EXPECT_TRUE(assigned.connected(0, 2));

    dsa::container::UnionFind<> moveAssigned(1);
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(moveAssigned.connected(0, 2));
    EXPECT_TRUE(assigned.empty());
    EXPECT_EQ(assigned.count(), 0U);
}

TEST(IndustrialUnionFindTest, ResetAndBoundsMaintainInvariants) {
    dsa::container::UnionFind<> uf(5);
    uf.unite(0, 1);
    uf.unite(1, 2);

    uf.reset();
    EXPECT_EQ(uf.count(), uf.size());
    EXPECT_FALSE(uf.connected(0, 1));
    EXPECT_EQ(uf.componentSize(2), 1U);

    EXPECT_THROW((void)uf.find(5), std::out_of_range);
    EXPECT_THROW(uf.unite(0, 5), std::out_of_range);
}

TEST(IndustrialUnionFindTest, DifferentialTestMatchesReferenceLabels) {
    const std::size_t elementCount = 96;
    dsa::container::UnionFind<> uf(elementCount);
    std::vector<std::size_t> labels(elementCount);
    std::iota(labels.begin(), labels.end(), std::size_t(0));
    std::size_t componentCount = elementCount;

    std::mt19937 random(1104U);
    std::uniform_int_distribution<std::size_t> index(0, elementCount - 1);

    for (int step = 0; step < 3000; ++step) {
        const std::size_t left = index(random);
        const std::size_t right = index(random);

        if (step % 4 != 0) {
            const std::size_t leftLabel = labels[left];
            const std::size_t rightLabel = labels[right];
            const bool shouldMerge = leftLabel != rightLabel;
            EXPECT_EQ(uf.unite(left, right), shouldMerge);

            if (shouldMerge) {
                for (std::size_t& label : labels) {
                    if (label == leftLabel)
                        label = rightLabel;
                }
                --componentCount;
            }
        } else {
            EXPECT_EQ(uf.connected(left, right), labels[left] == labels[right]);
        }

        EXPECT_EQ(uf.count(), componentCount);
    }
}
