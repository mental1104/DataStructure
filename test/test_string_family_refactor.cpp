#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>

#include "match/BM.h"
#include "match/KMP.h"
#include "match/KR.h"
#include "TST.h"
#include "Trie.h"
#include "dsa_string.h"
#include "string_algorithms.h"

TEST(StringFamilyRefactor, StringUsesValueSemanticsAndDelegatesAlgorithms) {
    std::string source = "hello";
    String value(source);
    String copied = value;
    String moved = std::move(copied);

    EXPECT_EQ(moved, String("hello"));
    EXPECT_EQ(dsa::str::substr(value, 1, 3), String("ell"));
    EXPECT_EQ(dsa::str::sub_str(std::string_view("abcdef"), 2, 2), String("cd"));
    EXPECT_EQ(dsa::str::prefix("prefix", 3), String("pre"));
    EXPECT_EQ(dsa::str::suffix(source, 2), String("lo"));
    EXPECT_EQ(dsa::str::concat(value, std::string_view(" world")), String("hello world"));
    EXPECT_TRUE(dsa::str::starts_with(value, "he"));
    EXPECT_TRUE(dsa::str::ends_with(value, std::string("lo")));

    // 旧成员 API 只保留为兼容入口，实际委托给自由算法。
    EXPECT_EQ(value.substr(1, 3), String("ell"));
    EXPECT_EQ(value.prefix(2), String("he"));
    EXPECT_EQ(value.suffix(2), String("lo"));
}

TEST(StringFamilyRefactor, MatchAlgorithmsAcceptMixedStringLikeInputs) {
    const String text("ababcababd");
    const std::string pattern = "ababd";

    EXPECT_EQ(matchKMP(pattern, text, KMPStrategy::Basic), 5);
    EXPECT_EQ(matchKMP(std::string_view(pattern), text, KMPStrategy::Improved), 5);
    EXPECT_EQ(matchBM("needle", std::string_view("find the needle in haystack"), BMStrategy::BadCharacter), 9);
    EXPECT_EQ(matchBM(std::string("needle"), "find the needle in haystack", BMStrategy::Full), 9);
    EXPECT_EQ(matchKR(std::string_view("cab"), String("abcabc")), 2);

    EXPECT_EQ(matchKMP("missing", text), -1);
    EXPECT_EQ(matchBM("missing", text), -1);
    EXPECT_EQ(matchKR("missing", text), -1);
    EXPECT_EQ(matchKMP("", text), 0);
    EXPECT_EQ(matchBM("", text), 0);
    EXPECT_EQ(matchKR("", text), 0);
}

TEST(StringFamilyRefactor, TrieStoresZeroAndAcceptsGenericKeys) {
    Trie<int> trie;
    trie.put(std::string("app"), 0);
    trie.put(std::string_view("apple"), 2);
    trie.put("apply", 3);

    EXPECT_TRUE(trie.contains("app"));
    EXPECT_EQ(trie.get(std::string_view("app")), 0);
    EXPECT_EQ(trie.size(), 3U);

    trie.put("apple", 20);
    EXPECT_EQ(trie.size(), 3U);
    EXPECT_EQ(trie.get("apple"), 20);
    EXPECT_EQ(trie.longestPrefixOf(std::string("applejack")), String("apple"));
    EXPECT_EQ(trie.keysWithPrefix("app").size(), 3);
    EXPECT_EQ(trie.keysThatMatch("app..").size(), 2);

    EXPECT_TRUE(trie.remove(std::string_view("app")));
    EXPECT_FALSE(trie.remove("app"));
    EXPECT_FALSE(trie.contains("app"));
}

TEST(StringFamilyRefactor, TernarySearchTrieStoresZeroAndSupportsEmptyKey) {
    TST<int> trie;
    trie.put("", 7);
    trie.put(std::string("cat"), 0);
    trie.put(std::string_view("car"), 2);
    trie.put("cart", 3);

    EXPECT_TRUE(trie.contains(""));
    EXPECT_TRUE(trie.contains("cat"));
    EXPECT_EQ(trie.get("cat"), 0);
    EXPECT_EQ(trie.size(), 4U);
    EXPECT_EQ(trie.longestPrefixOf("carton"), String("cart"));
    EXPECT_EQ(trie.keysWithPrefix("ca").size(), 3);
    EXPECT_EQ(trie.keysThatMatch("ca.").size(), 2);

    trie.put("car", 20);
    EXPECT_EQ(trie.size(), 4U);
    EXPECT_EQ(trie.get("car"), 20);
    EXPECT_TRUE(trie.remove(""));
    EXPECT_FALSE(trie.contains(""));
}
