#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "Hashtable.h"
#include "HashtableB.h"
#include "dsa/algorithm/Sequence.h"
#include "dsa/container/hash/HashTable.h"

namespace {

struct CollisionHash {
    std::size_t operator()(int value) const {
        return static_cast<std::size_t>(value % 3);
    }
};

struct ThrowState {
    int remaining;

    ThrowState() : remaining(-1) {}
};

struct ThrowingHash {
    std::shared_ptr<ThrowState> state;

    ThrowingHash() : state(new ThrowState) {}

    explicit ThrowingHash(const std::shared_ptr<ThrowState>& sharedState)
        : state(sharedState) {}

    std::size_t operator()(int value) const {
        if (state->remaining == 0)
            throw std::runtime_error("hash failure");
        if (state->remaining > 0)
            --state->remaining;
        return static_cast<std::size_t>(value);
    }
};

struct AllocationStats {
    int allocations;
    int deallocations;

    AllocationStats() : allocations(0), deallocations(0) {}
};

template<typename T>
struct CountingAllocator {
    typedef T value_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;

    std::shared_ptr<AllocationStats> stats;
    int id;

    CountingAllocator() : stats(new AllocationStats), id(0) {}

    CountingAllocator(const std::shared_ptr<AllocationStats>& sharedStats, int allocatorId)
        : stats(sharedStats), id(allocatorId) {}

    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other)
        : stats(other.stats), id(other.id) {}

    T* allocate(std::size_t count) {
        stats->allocations += static_cast<int>(count);
        return std::allocator<T>().allocate(count);
    }

    void deallocate(T* pointer, std::size_t count) {
        stats->deallocations += static_cast<int>(count);
        std::allocator<T>().deallocate(pointer, count);
    }

    template<typename U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };

    template<typename U>
    bool operator==(const CountingAllocator<U>& other) const {
        return id == other.id && stats == other.stats;
    }

    template<typename U>
    bool operator!=(const CountingAllocator<U>& other) const {
        return !(*this == other);
    }
};

} // namespace

TEST(TeachingHashtableRefactorTest, PreservesApiAndProvidesDeepCopy) {
    Hashtable<int, int> table(3);
    const int initialCapacity = table.capacity();

    EXPECT_TRUE(table.put(0, 10));
    EXPECT_TRUE(table.put(initialCapacity, 20));
    EXPECT_FALSE(table.put(0, 99));
    ASSERT_NE(table.get(initialCapacity), nullptr);
    EXPECT_EQ(*table.get(initialCapacity), 20);

    EXPECT_TRUE(table.remove(0));
    EXPECT_TRUE(table._lazyRemoval()->test(0));
    EXPECT_TRUE(table.put(0, 30));
    EXPECT_FALSE(table._lazyRemoval()->test(0));

    Hashtable<int, int> copy(table);
    EXPECT_TRUE(copy.remove(initialCapacity));
    EXPECT_NE(table.get(initialCapacity), nullptr);
    EXPECT_EQ(copy.get(initialCapacity), nullptr);
}

TEST(TeachingHashtableRefactorTest, QuadraticProbeCoversCollisionsAndNegativeKeys) {
    QuadraticHT<int, int> table(5);
    const int capacity = table._M();

    EXPECT_TRUE(table.put(0, 1));
    EXPECT_TRUE(table.put(capacity, 2));
    EXPECT_TRUE(table.put(2 * capacity, 3));
    EXPECT_TRUE(table.put(-1, 4));

    EXPECT_EQ(*table.get(0), 1);
    EXPECT_EQ(*table.get(capacity), 2);
    EXPECT_EQ(*table.get(2 * capacity), 3);
    EXPECT_EQ(*table.get(-1), 4);

    for (int value = 0; value < 200; ++value)
        EXPECT_TRUE(table.put(1000 + value, value));
    for (int value = 0; value < 200; ++value)
        EXPECT_EQ(*table.get(1000 + value), value);
}

TEST(ContainerHashTableTest, SupportsMapStyleInsertionLookupAndErase) {
    typedef dsa::container::HashTable<int, int, CollisionHash> Table;
    Table table;

    for (int key = 0; key < 500; ++key)
        EXPECT_TRUE(table.emplace(key, key * 10).second);

    EXPECT_EQ(table.size(), 500U);
    EXPECT_FALSE(table.emplace(1, 999).second);
    EXPECT_EQ(table.at(1), 10);
    EXPECT_THROW(table.at(9999), std::out_of_range);

    EXPECT_EQ(table.erase(1), 1U);
    EXPECT_EQ(table.erase(1), 0U);
    table[1] = 123;
    EXPECT_EQ(table.at(1), 123);

    std::pair<Table::iterator, bool> assigned = table.insert_or_assign(1, 456);
    EXPECT_FALSE(assigned.second);
    EXPECT_EQ(assigned.first->second, 456);
}

TEST(ContainerHashTableTest, IteratorsIntegrateWithSequenceForEach) {
    dsa::container::HashTable<int, int> table;
    for (int key = 1; key <= 100; ++key)
        table.emplace(key, key);

    int sum = 0;
    dsa::algorithm::forEach(
        table.cbegin(),
        table.cend(),
        [&sum](const std::pair<const int, int>& entry) {
            sum += entry.second;
        }
    );
    EXPECT_EQ(sum, 5050);

    dsa::container::HashTable<int, int>::iterator iterator = table.begin();
    ASSERT_NE(iterator, table.end());
    const int erasedKey = iterator->first;
    iterator = table.erase(iterator);
    EXPECT_FALSE(table.contains(erasedKey));
    EXPECT_TRUE(iterator == table.end() || table.contains(iterator->first));
}

TEST(ContainerHashTableTest, SupportsMoveOnlyMappedTypeAndStableReferencesAcrossRehash) {
    dsa::container::HashTable<int, std::unique_ptr<int> > table;
    table.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(1),
        std::forward_as_tuple(new int(7))
    );

    std::unique_ptr<int>* mappedAddress = &table.at(1);
    int* valueAddress = table.at(1).get();
    table.reserve(1000);

    EXPECT_EQ(&table.at(1), mappedAddress);
    EXPECT_EQ(table.at(1).get(), valueAddress);
    EXPECT_EQ(*table.at(1), 7);
}

TEST(ContainerHashTableTest, QuadraticPolicyHandlesDenseCollisionGroups) {
    dsa::container::QuadraticHashTable<int, int, CollisionHash> table;
    for (int key = 0; key < 300; ++key)
        EXPECT_TRUE(table.emplace(key, key + 1).second);

    for (int key = 0; key < 300; ++key)
        EXPECT_EQ(table.at(key), key + 1);

    for (int key = 0; key < 150; ++key)
        EXPECT_EQ(table.erase(key), 1U);
    for (int key = 0; key < 150; ++key)
        EXPECT_TRUE(table.emplace(key, key + 2).second);
}

TEST(ContainerHashTableTest, RehashRollsBackWhenHasherThrows) {
    std::shared_ptr<ThrowState> state(new ThrowState);
    dsa::container::HashTable<int, int, ThrowingHash> table(
        3,
        ThrowingHash(state)
    );
    for (int key = 0; key < 20; ++key)
        table.emplace(key, key * 10);

    const std::size_t oldBucketCount = table.bucket_count();
    state->remaining = 3;
    EXPECT_THROW(table.rehash(oldBucketCount * 4), std::runtime_error);

    state->remaining = -1;
    EXPECT_EQ(table.bucket_count(), oldBucketCount);
    EXPECT_EQ(table.size(), 20U);
    for (int key = 0; key < 20; ++key)
        EXPECT_EQ(table.at(key), key * 10);
}

TEST(ContainerHashTableTest, HonorsStatefulAllocatorOwnership) {
    typedef std::pair<const int, int> Pair;
    typedef CountingAllocator<Pair> Allocator;
    typedef dsa::container::HashTable<
        int,
        int,
        std::hash<int>,
        std::equal_to<int>,
        Allocator
    > Table;

    std::shared_ptr<AllocationStats> firstStats(new AllocationStats);
    std::shared_ptr<AllocationStats> secondStats(new AllocationStats);
    {
        Table first(
            3,
            std::hash<int>(),
            std::equal_to<int>(),
            Allocator(firstStats, 1)
        );
        for (int key = 0; key < 100; ++key)
            first.emplace(key, key);

        Table second(
            3,
            std::hash<int>(),
            std::equal_to<int>(),
            Allocator(secondStats, 2)
        );
        second = first;
        EXPECT_EQ(second.get_allocator().id, 2);
        EXPECT_EQ(second.size(), first.size());

        Table moved(
            3,
            std::hash<int>(),
            std::equal_to<int>(),
            Allocator(secondStats, 2)
        );
        moved = std::move(second);
        EXPECT_EQ(moved.size(), first.size());
    }

    EXPECT_EQ(firstStats->allocations, firstStats->deallocations);
    EXPECT_EQ(secondStats->allocations, secondStats->deallocations);
}

TEST(ContainerHashTableTest, MatchesStdUnorderedMapForCommonOperations) {
    dsa::container::HashTable<int, int> table;
    std::unordered_map<int, int> reference;
    unsigned int seed = 1;

    for (int step = 0; step < 5000; ++step) {
        seed = seed * 1103515245U + 12345U;
        const int key = static_cast<int>((seed >> 8) % 200);
        const int operation = static_cast<int>(seed % 3);

        if (operation == 0) {
            const bool actual = table.emplace(key, step).second;
            const bool expected = reference.emplace(key, step).second;
            EXPECT_EQ(actual, expected);
        } else if (operation == 1) {
            EXPECT_EQ(table.erase(key), reference.erase(key));
        } else {
            dsa::container::HashTable<int, int>::iterator actual = table.find(key);
            std::unordered_map<int, int>::iterator expected = reference.find(key);
            ASSERT_EQ(actual == table.end(), expected == reference.end());
            if (actual != table.end()) {
                EXPECT_EQ(actual->second, expected->second);
            }
        }
    }

    EXPECT_EQ(table.size(), reference.size());
}
