#include <gtest/gtest.h>

#include <string>

#include "container/hash_map.hpp"

namespace {

struct CustomKey {
    explicit CustomKey(int id) : id(id) {}

    int id;
};

struct ConstantHash {
    std::size_t operator()(const CustomKey&) const { return 0; }
};

struct CustomKeyEqual {
    bool operator()(const CustomKey& lhs, const CustomKey& rhs) const {
        return lhs.id == rhs.id;
    }
};

struct CustomKeyCompare {
    bool operator()(const CustomKey& lhs, const CustomKey& rhs) const {
        return lhs.id < rhs.id;
    }
};

}  // namespace

TEST(IndustrialHashMapTest, BasicApiOperations) {
    dsa::industrial::hash_map<int, std::string> map(4);

    EXPECT_TRUE(map.emplace(1, "one"));
    EXPECT_FALSE(map.emplace(1, "uno"));
    ASSERT_NE(map.find(1), nullptr);
    EXPECT_EQ(*map.find(1), "one");
    EXPECT_TRUE(map.contains(1));
    EXPECT_FALSE(map.contains(2));

    map[2] = "two";
    EXPECT_EQ(map.at(2), "two");
    EXPECT_THROW(map.at(9), std::out_of_range);

    EXPECT_EQ(map.erase(1), static_cast<std::size_t>(1));
    EXPECT_EQ(map.erase(1), static_cast<std::size_t>(0));
    EXPECT_FALSE(map.contains(1));

    map.clear();
    EXPECT_TRUE(map.empty());
}

TEST(IndustrialHashMapTest, TreeBucketUsesComparatorWithoutOperatorLess) {
    typedef dsa::industrial::hash_map<
        CustomKey,
        int,
        ConstantHash,
        CustomKeyEqual,
        CustomKeyCompare
    > Map;

    Map map(32);
    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(map.emplace(CustomKey(i), i * 10));
    }

    ASSERT_EQ(map.bucket_count(), static_cast<std::size_t>(32));
    EXPECT_TRUE(map.bucket_is_tree(0));
    EXPECT_EQ(map.bucket_size(0), static_cast<std::size_t>(8));
    ASSERT_NE(map.find(CustomKey(3)), nullptr);
    EXPECT_EQ(*map.find(CustomKey(3)), 30);

    EXPECT_FALSE(map.emplace(CustomKey(3), 300));
    EXPECT_EQ(map.erase(CustomKey(3)), static_cast<std::size_t>(1));
    EXPECT_FALSE(map.contains(CustomKey(3)));
}

TEST(IndustrialHashMapTest, ReserveAndRehashPreserveTreeBucketEntries) {
    typedef dsa::industrial::hash_map<
        CustomKey,
        int,
        ConstantHash,
        CustomKeyEqual,
        CustomKeyCompare
    > Map;

    Map map(4);
    map.reserve(64);
    EXPECT_GE(map.bucket_count(), static_cast<std::size_t>(64));

    for (int i = 0; i < 12; ++i) {
        EXPECT_TRUE(map.emplace(CustomKey(i), i + 100));
    }
    EXPECT_TRUE(map.bucket_is_tree(0));

    map.rehash(128);
    EXPECT_GE(map.bucket_count(), static_cast<std::size_t>(128));
    EXPECT_TRUE(map.bucket_is_tree(0));
    EXPECT_EQ(map.size(), static_cast<std::size_t>(12));

    for (int i = 0; i < 12; ++i) {
        const int* value = map.find(CustomKey(i));
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(*value, i + 100);
    }
}
