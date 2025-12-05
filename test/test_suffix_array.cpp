#include <gtest/gtest.h>

#include "dsa_string.h"
#include "suffix_array.h"

TEST(SuffixArrayTest, BananaOrderAndQueries) {
    String text("banana");
    SuffixArray<String> sa(text);

    int expected_indices[] = {5, 3, 1, 0, 4, 2};
    for (int i = 0; i < sa.length(); ++i) {
        EXPECT_EQ(sa.index(i), expected_indices[i]);
    }

    EXPECT_EQ(sa.length(), 6);
    EXPECT_EQ(sa.lcp(1), 1);
    EXPECT_EQ(sa.lcp(2), 3);
    EXPECT_EQ(sa.lcp(3), 0);
    EXPECT_EQ(sa.lcp(4), 0);
    EXPECT_EQ(sa.lcp(5), 2);

    auto view = sa.select_view(2);
    EXPECT_EQ(view.first, 1);
    EXPECT_EQ(view.second, 6);

    EXPECT_EQ(sa.rank(String("ana")), 1);
    EXPECT_EQ(sa.rank(String("band")), 4);
}

TEST(SuffixArrayTest, RepeatedCharacters) {
    String text("aaaa");
    SuffixArray<String> sa(text);

    int expected_indices[] = {3, 2, 1, 0};
    for (int i = 0; i < sa.length(); ++i) {
        EXPECT_EQ(sa.index(i), expected_indices[i]);
    }

    EXPECT_EQ(sa.rank(String("aaa")), 2);
    EXPECT_EQ(sa.rank(String("aaaaa")), 4);
    EXPECT_EQ(sa.lcp(1), 1);
    EXPECT_EQ(sa.lcp(2), 2);
    EXPECT_EQ(sa.lcp(3), 3);
}
