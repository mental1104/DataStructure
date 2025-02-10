#include <gtest/gtest.h>
#include "dsa_string.h"
#include "Trie.h"
#include "TST.h"
#include "print.h"

class TrieTest : public ::testing::Test {
protected:
    Trie<int> trie;
};

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
