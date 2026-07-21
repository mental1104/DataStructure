#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <dsa/algorithm/SuffixArray.h>

TEST(IndustrialSuffixArrayTest, SupportsStandardStringAndQueries) {
    dsa::algorithm::SuffixArray<std::string> suffixes(std::string("banana"));
    const int expected[] = {5, 3, 1, 0, 4, 2};
    ASSERT_EQ(suffixes.size(), 6u);
    for (int i = 0; i < 6; ++i)
        EXPECT_EQ(suffixes.index(i), expected[i]);
    EXPECT_EQ(suffixes.lcp(2), 3);
    EXPECT_EQ(suffixes.rank(std::string("ana")), 1);
    EXPECT_EQ(suffixes.select(0), "a");
}

TEST(IndustrialSuffixArrayTest, HandlesEmptyAndRepeatedText) {
    dsa::algorithm::SuffixArray<std::string> empty{std::string()};
    EXPECT_TRUE(empty.empty());

    dsa::algorithm::SuffixArray<std::string> repeated(std::string("aaaa"));
    EXPECT_EQ(repeated.lcp(3), 3);
    EXPECT_EQ(repeated.rank(std::string("aaaaa")), 4);
}
