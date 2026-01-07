#include <gtest/gtest.h>
#include <cstring>
#include "dsa_string.h"
#include "Trie.h"
#include "TST.h"
#include "print.h"

class TrieTest : public ::testing::Test {
protected:
    Trie<int> trie;
};

static bool containsString(const Vector<String>& values, const char* target) {
    for (Rank i = 0; i < values.size(); ++i) {
        if (std::strcmp(values[i].c_str(), target) == 0) {
            return true;
        }
    }
    return false;
}

TEST_F(TrieTest, PutAndGet) {
    trie.put("apple", 10);
    trie.put("app", 5);
    EXPECT_EQ(trie.get("apple"), 10);
    EXPECT_EQ(trie.get("app"), 5);
    EXPECT_EQ(trie.get("appl"), 0); // 不存在的键
}

TEST_F(TrieTest, Remove) {
    trie.put("apple", 10);
    trie.put("app", 5);
    trie.remove("apple");
    EXPECT_EQ(trie.get("apple"), 0);
    EXPECT_EQ(trie.get("app"), 5);
}

TEST_F(TrieTest, PrefixMatching) {
    trie.put("apple", 10);
    trie.put("app", 5);
    trie.put("apply", 15);
    Vector<String> keys = trie.keysWithPrefix("app");
    EXPECT_NE(keys.search(String("apple")), -1);
    EXPECT_NE(keys.search(String("app")), -1);
    EXPECT_NE(keys.search(String("apply")), -1);
}

TEST_F(TrieTest, PutZeroIgnoredAndContains) {
    trie.put("zero", 0);
    EXPECT_TRUE(trie.empty());
    EXPECT_EQ(trie.size(), 0);
    EXPECT_FALSE(trie.contains(String("zero")));
    trie.keys();
}

TEST_F(TrieTest, KeysThatMatchAndLongestPrefix) {
    trie.put("cat", 1);
    trie.put("car", 2);
    trie.put("cart", 3);
    Vector<String> matches = trie.keysThatMatch("c..");
    EXPECT_TRUE(containsString(matches, "cat"));
    EXPECT_TRUE(containsString(matches, "car"));
    EXPECT_FALSE(containsString(matches, "cart"));

    String prefix = trie.longestPrefixOf("carton");
    EXPECT_STREQ(prefix.c_str(), "cart");
    EXPECT_STREQ(trie.longestPrefixOf("").c_str(), "");
}

TEST_F(TrieTest, RemovePrunesLeaf) {
    trie.put("solo", 42);
    EXPECT_EQ(trie.get("solo"), 42);
    trie.remove("solo");
    EXPECT_EQ(trie.get("solo"), 0);
    EXPECT_EQ(trie.size(), 0);
    Vector<String> keys = trie.keysWithPrefix("s");
    EXPECT_EQ(keys.size(), 0);
}

TEST_F(TrieTest, PrefixWithNoMatch) {
    trie.put("apple", 10);
    Vector<String> keys = trie.keysWithPrefix("zzz");
    EXPECT_EQ(keys.size(), 0);
}

class TSTTest : public ::testing::Test {
protected:
    TST<int> tst;
};

TEST_F(TSTTest, PutAndGet) {
    tst.put("banana", 20);
    tst.put("band", 15);
    EXPECT_EQ(tst.get("banana"), 20);
    EXPECT_EQ(tst.get("band"), 15);
    EXPECT_EQ(tst.get("ban"), 0);
}

TEST_F(TSTTest, Remove) {
    tst.put("banana", 20);
    tst.put("band", 15);
    tst.remove("banana");
    EXPECT_EQ(tst.get("banana"), 0);
    EXPECT_EQ(tst.get("band"), 15);
}

TEST_F(TSTTest, PrefixMatching) {
    tst.put("banana", 20);
    tst.put("band", 15);
    tst.put("banner", 25);
    Vector<String> keys = tst.keysWithPrefix("ban");
    EXPECT_NE(keys.search(String("banana")), -1);
    EXPECT_NE(keys.search(String("band")), -1);
    EXPECT_NE(keys.search(String("banner")), -1);
}

TEST_F(TSTTest, ContainsAndKeys) {
    tst.put("dog", 3);
    EXPECT_TRUE(tst.contains(String("dog")));
    EXPECT_FALSE(tst.contains(String("do")));
    tst.keys();
}

TEST_F(TSTTest, GetCStrAndLongestPrefix) {
    tst.put("ship", 7);
    tst.put("shore", 9);
    EXPECT_EQ(tst.get("ship"), 7);
    EXPECT_STREQ(tst.longestPrefixOf("shoreline").c_str(), "shore");
    EXPECT_STREQ(tst.longestPrefixOf("").c_str(), "");
}

TEST_F(TSTTest, KeysThatMatchAndEmptyPrefix) {
    tst.put("tap", 1);
    tst.put("tip", 2);
    Vector<String> all = tst.keysWithPrefix("");
    EXPECT_TRUE(containsString(all, "tap"));
    EXPECT_TRUE(containsString(all, "tip"));

    Vector<String> matches = tst.keysThatMatch("t.p");
    EXPECT_TRUE(containsString(matches, "tap"));
    EXPECT_TRUE(containsString(matches, "tip"));
}

TEST_F(TSTTest, RemovePrunesLeaf) {
    tst.put("solo", 1);
    tst.remove("solo");
    EXPECT_EQ(tst.size(), 0);
    Vector<String> keys = tst.keysWithPrefix("s");
    EXPECT_EQ(keys.size(), 0);
}

TEST_F(TSTTest, RemoveMissingFromEmpty) {
    tst.remove("ghost");
    EXPECT_EQ(tst.size(), 0);
}
