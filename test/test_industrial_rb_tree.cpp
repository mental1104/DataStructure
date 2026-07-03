#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "tree/rb_tree.hpp"

namespace {

struct CountingStats {
    CountingStats() : allocations(0), deallocations(0) {}

    std::size_t allocations;
    std::size_t deallocations;
};

template <class T>
class CountingAllocator {
public:
    typedef T value_type;

    explicit CountingAllocator(CountingStats* stats = 0) : stats(stats) {}

    template <class U>
    CountingAllocator(const CountingAllocator<U>& other) : stats(other.stats) {}

    T* allocate(std::size_t n) {
        if (stats) {
            stats->allocations += n;
        }
        return std::allocator<T>().allocate(n);
    }

    void deallocate(T* p, std::size_t n) {
        if (stats) {
            stats->deallocations += n;
        }
        std::allocator<T>().deallocate(p, n);
    }

    template <class U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };

    CountingStats* stats;
};

template <class T, class U>
bool operator==(const CountingAllocator<T>& lhs, const CountingAllocator<U>& rhs) {
    return lhs.stats == rhs.stats;
}

template <class T, class U>
bool operator!=(const CountingAllocator<T>& lhs, const CountingAllocator<U>& rhs) {
    return !(lhs == rhs);
}

struct CustomType {
    CustomType(int id, const std::string& name) : id(id), name(name) {}

    int id;
    std::string name;
};

struct CustomTypeCompare {
    bool operator()(const CustomType& lhs, const CustomType& rhs) const {
        return lhs.id < rhs.id;
    }
};

struct NonDefaultConstructible {
    explicit NonDefaultConstructible(int value) : value(value) {}
    NonDefaultConstructible() = delete;

    int value;
};

struct NonDefaultCompare {
    bool operator()(
        const NonDefaultConstructible& lhs,
        const NonDefaultConstructible& rhs) const {
        return lhs.value < rhs.value;
    }
};

}  // namespace

TEST(IndustrialRBTreeTest, InsertFindEraseAndIterateInOrder) {
    dsa::industrial::rb_tree<int> tree;
    const int values[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};

    for (std::size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        EXPECT_TRUE(tree.insert(values[i]).second);
    }
    EXPECT_FALSE(tree.insert(6).second);
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(9));

    const int expected_values[] = {1, 3, 4, 6, 7, 8, 10, 13, 14};
    std::vector<int> actual;
    for (dsa::industrial::rb_tree<int>::iterator it = tree.begin();
         it != tree.end();
         ++it) {
        actual.push_back(*it);
    }
    std::vector<int> expected(
        expected_values,
        expected_values + sizeof(expected_values) / sizeof(expected_values[0]));
    EXPECT_EQ(actual, expected);

    ASSERT_NE(tree.find(10), tree.end());
    EXPECT_EQ(tree.erase(3), static_cast<std::size_t>(1));
    EXPECT_EQ(tree.erase(100), static_cast<std::size_t>(0));
    EXPECT_EQ(tree.find(3), tree.end());
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(8));

    tree.clear();
    EXPECT_TRUE(tree.empty());
}

TEST(IndustrialRBTreeTest, UsesAllocatorTraitsRebindForNodes) {
    CountingStats stats;
    {
        typedef CountingAllocator<int> Alloc;
        std::less<int> less;
        Alloc allocator(&stats);
        dsa::industrial::rb_tree<int, std::less<int>, Alloc> tree(less, allocator);

        EXPECT_TRUE(tree.emplace(2).second);
        EXPECT_TRUE(tree.emplace(1).second);
        EXPECT_TRUE(tree.emplace(3).second);
        EXPECT_EQ(stats.allocations, static_cast<std::size_t>(3));

        EXPECT_EQ(tree.erase(2), static_cast<std::size_t>(1));
        EXPECT_EQ(stats.deallocations, static_cast<std::size_t>(1));
    }
    EXPECT_EQ(stats.allocations, stats.deallocations);
}

TEST(IndustrialRBTreeTest, SupportsCustomTypeWithComparatorOnly) {
    dsa::industrial::rb_tree<CustomType, CustomTypeCompare> tree;

    EXPECT_TRUE(tree.emplace(2, "two").second);
    EXPECT_TRUE(tree.emplace(1, "one").second);
    EXPECT_FALSE(tree.emplace(2, "duplicate").second);

    dsa::industrial::rb_tree<CustomType, CustomTypeCompare>::iterator found =
        tree.find(CustomType(2, "probe"));
    ASSERT_NE(found, tree.end());
    EXPECT_EQ(found->name, "two");
}

TEST(IndustrialRBTreeTest, SupportsNonDefaultConstructibleValues) {
    dsa::industrial::rb_tree<NonDefaultConstructible, NonDefaultCompare> tree;

    EXPECT_TRUE(tree.emplace(4).second);
    EXPECT_TRUE(tree.emplace(2).second);
    EXPECT_TRUE(tree.emplace(6).second);
    EXPECT_FALSE(tree.emplace(4).second);

    EXPECT_NE(tree.find(NonDefaultConstructible(2)), tree.end());
    EXPECT_EQ(tree.erase(NonDefaultConstructible(4)), static_cast<std::size_t>(1));
    EXPECT_EQ(tree.find(NonDefaultConstructible(4)), tree.end());
}
