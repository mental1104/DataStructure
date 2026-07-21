#include <gtest/gtest.h>

#include <string>
#include <unordered_map>
#include <vector>

#include <dsa/container/hash/ChainedHashMap.h>
#include <dsa/container/hash/HashMap.h>
#include <dsa/container/string/StringSymbolTable.h>
#include <dsa/container/string/Trie.h>
#include <dsa/container/string/TernarySearchTrie.h>
#include "Dictionary.h"

TEST(HashFacadeTest, OpenAddressingHashMapPreservesDictionarySemantics) {
    dsa::container::HashMap<std::string, int> map;
    EXPECT_TRUE(map.put("alpha", 1));
    EXPECT_FALSE(map.put("alpha", 2));
    ASSERT_NE(map.get("alpha"), nullptr);
    EXPECT_EQ(*map.get("alpha"), 2);
    EXPECT_TRUE(map.remove("alpha"));
    EXPECT_FALSE(map.remove("alpha"));
}

TEST(HashFacadeTest, ChainedMapRehashesWithoutLosingNodes) {
    dsa::container::ChainedHashMap<int, std::string> map(2);
    map.max_load_factor(0.5f);
    for (int i = 0; i < 100; ++i)
        EXPECT_TRUE(map.put(i, std::to_string(i)));
    EXPECT_GE(map.bucket_count(), 200u);
    for (int i = 0; i < 100; ++i) {
        ASSERT_NE(map.get(i), nullptr);
        EXPECT_EQ(*map.get(i), std::to_string(i));
    }
    std::size_t visited = 0;
    for (auto it = map.begin(); it != map.end(); ++it)
        ++visited;
    EXPECT_EQ(visited, map.size());
}

TEST(HashFacadeTest, TeachingHashCodeHasNoOutputSideEffectAndIsStable) {
    EXPECT_EQ(hashCode("hello"), hashCode("hello"));
    EXPECT_NE(hashCode("hello"), hashCode("world"));
}

TEST(StringSymbolTableTest, UnifiesTrieImplementations) {
    typedef dsa::container::Trie<int> Trie;
    dsa::container::StringSymbolTable<Trie> trie;
    EXPECT_TRUE(trie.put(std::string("shell"), 1));
    EXPECT_TRUE(trie.put(std::string("she"), 2));
    ASSERT_NE(trie.get(std::string("she")), nullptr);
    EXPECT_EQ(*trie.get(std::string("she")), 2);
    EXPECT_EQ(trie.longestPrefixOf(std::string("shelter")), "she");

    typedef dsa::container::TernarySearchTrie<int> Tst;
    dsa::container::StringSymbolTable<Tst> tst;
    EXPECT_TRUE(tst.put(std::string("cat"), 3));
    EXPECT_TRUE(tst.contains(std::string("cat")));
    EXPECT_TRUE(tst.remove(std::string("cat")));
}


TEST(HashFacadeTest, ChainedMapSupportsCopyMoveAndDifferentialOperations) {
    dsa::container::ChainedHashMap<int, int> actual(3);
    std::unordered_map<int, int> expected;
    for (int step = 0; step < 1000; ++step) {
        const int key = (step * 37) % 211;
        if (step % 5 == 0) {
            EXPECT_EQ(actual.erase(key), expected.erase(key));
        } else {
            actual.insert_or_assign(key, step);
            expected[key] = step;
        }
        EXPECT_EQ(actual.size(), expected.size());
        for (int probe = 0; probe < 211; probe += 17) {
            const auto found = expected.find(probe);
            const int* value = actual.get(probe);
            EXPECT_EQ(value != nullptr, found != expected.end());
            if (value) {
                EXPECT_EQ(*value, found->second);
            }
        }
    }

    dsa::container::ChainedHashMap<int, int> copy(actual);
    EXPECT_EQ(copy.size(), actual.size());
    dsa::container::ChainedHashMap<int, int> moved(std::move(copy));
    EXPECT_EQ(moved.size(), actual.size());
    EXPECT_TRUE(copy.empty());
    EXPECT_TRUE(copy.put(999, 1));
    EXPECT_EQ(*copy.get(999), 1);

    dsa::container::ChainedHashMap<int, int> assigned;
    assigned = std::move(moved);
    EXPECT_EQ(assigned.size(), actual.size());
    EXPECT_TRUE(moved.empty());
    EXPECT_TRUE(moved.put(1000, 2));
}
