#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <random>
#include <string>
#include <vector>
#if __cplusplus >= 201703L
#include <string_view>
#endif

#include "dsa_string.h"
#include "LSD.h"
#include "MSD.h"
#include "Quick3String.h"
#include "Trie.h"
#include "TST.h"
#include <dsa/algorithm/String.h>
#include <dsa/algorithm/StringSort.h>
#include <dsa/algorithm/SubstringSearch.h>
#include <dsa/container/string/String.h>
#include <dsa/container/string/Trie.h>
#include <dsa/container/string/TernarySearchTrie.h>

namespace {

std::string randomString(std::mt19937& engine, int maxLength) {
    std::uniform_int_distribution<int> lengthDistribution(0, maxLength);
    std::uniform_int_distribution<int> symbolDistribution(0, 4);
    std::string result;
    const int length = lengthDistribution(engine);
    for (int index = 0; index < length; ++index)
        result.push_back(static_cast<char>('a' + symbolDistribution(engine)));
    return result;
}

} // namespace

TEST(StringAlgorithmRefactor, AcceptsTeachingStandardAndCStringSequences) {
    String teaching("substring");
    std::string standard("substring");
    const char* cString = "substring";

    EXPECT_EQ(dsa::algorithm::sequenceSize(teaching), 9U);
    EXPECT_EQ(dsa::algorithm::sequenceSize(standard), 9U);
    EXPECT_EQ(dsa::algorithm::sequenceSize(cString), 9U);
    EXPECT_EQ(
        dsa::algorithm::substring<std::string>(teaching, 3, 3),
        "str"
    );
    EXPECT_EQ(
        dsa::algorithm::prefix<std::string>(cString, 3),
        "sub"
    );
    EXPECT_EQ(
        dsa::algorithm::suffix<std::string>(standard, 3),
        "ing"
    );
    EXPECT_TRUE(dsa::algorithm::startsWith(teaching, "sub"));
    EXPECT_TRUE(dsa::algorithm::endsWith(standard, String("ing")));
    EXPECT_TRUE(dsa::algorithm::sequenceEqual(teaching, cString));

#if __cplusplus >= 201703L
    const std::string_view view("substring");
    EXPECT_TRUE(dsa::algorithm::sequenceEqual(view, teaching));
    EXPECT_EQ(dsa::algorithm::substring<std::string>(view, 3), "string");
#endif
}

TEST(StringAlgorithmRefactor, SubstringSearchAcceptsMixedSequenceTypes) {
    const String teachingText("ababcababd");
    const std::string standardPattern("ababd");
    EXPECT_EQ(
        dsa::algorithm::kmpSearch(
            standardPattern,
            teachingText,
            dsa::algorithm::KmpStrategy::Basic
        ),
        5U
    );
    EXPECT_EQ(
        dsa::algorithm::kmpSearch(
            "ababd",
            teachingText,
            dsa::algorithm::KmpStrategy::Improved
        ),
        5U
    );
    EXPECT_EQ(
        dsa::algorithm::boyerMooreSearch(
            "needle",
            std::string("find the needle in haystack"),
            dsa::algorithm::BoyerMooreStrategy::Full
        ),
        9U
    );
    EXPECT_EQ(
        dsa::algorithm::rabinKarpSearch(
            String("159"),
            std::string("31415926")
        ),
        3U
    );
    EXPECT_EQ(
        dsa::algorithm::kmpSearch("missing", std::string("text")),
        dsa::algorithm::stringNpos
    );
}

TEST(StringAlgorithmRefactor, MatchAlgorithmsDifferentialTest) {
    std::mt19937 engine(20260721U);
    for (int round = 0; round < 1000; ++round) {
        const std::string text = randomString(engine, 40);
        const std::string pattern = randomString(engine, 10);
        const std::size_t expected = text.find(pattern);
        EXPECT_EQ(dsa::algorithm::findNaive(pattern, text), expected);
        EXPECT_EQ(
            dsa::algorithm::kmpSearch(
                pattern,
                text,
                dsa::algorithm::KmpStrategy::Basic
            ),
            expected
        );
        EXPECT_EQ(
            dsa::algorithm::kmpSearch(
                pattern,
                text,
                dsa::algorithm::KmpStrategy::Improved
            ),
            expected
        );
        EXPECT_EQ(
            dsa::algorithm::boyerMooreSearch(
                pattern,
                text,
                dsa::algorithm::BoyerMooreStrategy::BadCharacter
            ),
            expected
        );
        EXPECT_EQ(
            dsa::algorithm::boyerMooreSearch(
                pattern,
                text,
                dsa::algorithm::BoyerMooreStrategy::Full
            ),
            expected
        );
        EXPECT_EQ(dsa::algorithm::rabinKarpSearch(pattern, text), expected);
    }
}

TEST(StringAlgorithmRefactor, StringSortAlgorithmsAreContainerAgnostic) {
    std::vector<std::string> fixed = {
        "dab", "cab", "abc", "bac", "cba"
    };
    dsa::algorithm::lsdStringSort(fixed, 3);
    EXPECT_TRUE(std::is_sorted(fixed.begin(), fixed.end()));

    std::vector<std::string> variable = {
        "", "b", "a", "aa", "ab", "aba"
    };
    std::vector<std::string> expected = variable;
    std::sort(expected.begin(), expected.end());
    std::vector<std::string> msd = variable;
    std::vector<std::string> quick3 = variable;
    dsa::algorithm::msdStringSort(msd);
    dsa::algorithm::quick3StringSort(quick3);
    EXPECT_EQ(msd, expected);
    EXPECT_EQ(quick3, expected);

    std::vector<std::string> invalid = {"a", "bbb"};
    EXPECT_THROW(dsa::algorithm::lsdStringSort(invalid, 3), std::invalid_argument);
}

TEST(StringAlgorithmRefactor, TeachingStringKeepsOldFacadeAndAddsMoveSemantics) {
    String value("substring");
    EXPECT_STREQ(value.substr(3).c_str(), "string");
    EXPECT_STREQ(value.prefix(3).c_str(), "sub");
    EXPECT_STREQ(value.suffix(3).c_str(), "ing");

    String copy(value);
    String moved(std::move(copy));
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(moved, value);

    value.concat(String("!"));
    EXPECT_STREQ(value.c_str(), "substring!");
    EXPECT_GE(value.capacity(), value.size());
}

TEST(StringAlgorithmRefactor, TeachingSortFacadesUseSharedAlgorithms) {
    Vector<String> values;
    values.insert(String("dab"));
    values.insert(String("cab"));
    values.insert(String("abc"));
    LSD(values, 3);
    EXPECT_STREQ(values[0].c_str(), "abc");
    EXPECT_STREQ(values[1].c_str(), "cab");
    EXPECT_STREQ(values[2].c_str(), "dab");

    MSD::sort(values);
    Quick3String::sort(values);
    EXPECT_STREQ(values[0].c_str(), "abc");
}

TEST(StringAlgorithmRefactor, TeachingTrieMaintainsSizeInvariants) {
    Trie<int> trie;
    trie.put("app", 1);
    trie.put("apple", 2);
    trie.put("app", 3);
    EXPECT_EQ(trie.size(), 2);
    EXPECT_EQ(trie.get("app"), 3);

    trie.remove("missing");
    EXPECT_EQ(trie.size(), 2);
    EXPECT_STREQ(trie.longestPrefixOf("apples").c_str(), "apple");

    trie.remove("apple");
    EXPECT_EQ(trie.size(), 1);
    EXPECT_EQ(trie.get("apple"), 0);
}

TEST(StringAlgorithmRefactor, TeachingTstMaintainsSizeInvariants) {
    TST<int> trie;
    trie.put("app", 1);
    trie.put("apple", 2);
    trie.put("app", 3);
    EXPECT_EQ(trie.size(), 2);
    EXPECT_EQ(trie.get("app"), 3);

    trie.remove("missing");
    EXPECT_EQ(trie.size(), 2);
    EXPECT_STREQ(trie.longestPrefixOf("apples").c_str(), "apple");

    trie.put("", 9);
    EXPECT_EQ(trie.size(), 2);
}

TEST(StringAlgorithmRefactor, IndustrialStringMaintainsTerminatorAndMovedFromState) {
    dsa::container::String value("hello");
    value += " world";
    EXPECT_EQ(value.size(), 11U);
    EXPECT_STREQ(value.c_str(), "hello world");
    EXPECT_EQ(value.substr(6), dsa::container::String("world"));

    dsa::container::String moved(std::move(value));
    EXPECT_TRUE(value.empty());
    EXPECT_STREQ(value.c_str(), "");
    EXPECT_STREQ(moved.c_str(), "hello world");
}

TEST(StringAlgorithmRefactor, IndustrialTrieSupportsZeroAndMoveOnlyValues) {
    dsa::container::Trie<int> trie;
    EXPECT_TRUE(trie.insert("zero", 0));
    ASSERT_NE(trie.find(std::string("zero")), nullptr);
    EXPECT_EQ(*trie.find("zero"), 0);
    EXPECT_FALSE(trie.insert(String("zero"), 7));
    EXPECT_EQ(*trie.find("zero"), 7);

    dsa::container::Trie<int> moved(std::move(trie));
    EXPECT_EQ(trie.size(), 0U);
    EXPECT_EQ(moved.size(), 1U);

    dsa::container::Trie<std::unique_ptr<int> > moveOnly;
    moveOnly.insert("key", std::unique_ptr<int>(new int(42)));
    ASSERT_NE(moveOnly.find("key"), nullptr);
    EXPECT_EQ(**moveOnly.find("key"), 42);
}

TEST(StringAlgorithmRefactor, IndustrialTstSupportsEmptyZeroAndMoveOnlyValues) {
    dsa::container::TernarySearchTrie<int> trie;
    EXPECT_TRUE(trie.insert("", 0));
    EXPECT_TRUE(trie.insert("cat", 3));
    EXPECT_TRUE(trie.insert(std::string("car"), 4));
    EXPECT_EQ(trie.size(), 3U);
    ASSERT_NE(trie.find(""), nullptr);
    EXPECT_EQ(*trie.find(""), 0);
    EXPECT_EQ(trie.longestPrefixOf("carton"), "car");

    dsa::container::TernarySearchTrie<int> moved(std::move(trie));
    EXPECT_EQ(trie.size(), 0U);
    EXPECT_EQ(moved.size(), 3U);

    dsa::container::TernarySearchTrie<std::unique_ptr<int> > moveOnly;
    moveOnly.insert("x", std::unique_ptr<int>(new int(9)));
    ASSERT_NE(moveOnly.find("x"), nullptr);
    EXPECT_EQ(**moveOnly.find("x"), 9);
}
