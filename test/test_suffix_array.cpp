#include <gtest/gtest.h>

#include "dsa_string.h"
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#include "suffix_array.h"
#undef private
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace {
struct FlakyLess {
    static bool flaky;
    static int calls;
    static void setFlaky(bool enable) {
        flaky = enable;
        calls = 0;
    }
    bool operator()(char a, char b) const {
        if (!flaky) return a < b;
        ++calls;
        return calls == 1;
    }
};

bool FlakyLess::flaky = false;
int FlakyLess::calls = 0;
}

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

TEST(SuffixArrayTest, IndexChecksAndCharRefOps) {
    String text("abc");
    SuffixArray<String> sa(text);

    EXPECT_THROW(sa.select_view(-1), std::out_of_range);
    EXPECT_THROW(sa.lcp(0), std::out_of_range);
    EXPECT_EQ(sa.lcp(1), 0);

    SuffixArray<String>::CharRef a{0, &sa};
    SuffixArray<String>::CharRef b{1, &sa};
    EXPECT_EQ(a > b, b < a);
}

TEST(SuffixArrayTest, CompareKeySuffixFallbackReturn) {
    FlakyLess::setFlaky(false);
    String text("abc");
    SuffixArray<String, FlakyLess> sa(text);
    FlakyLess::setFlaky(true);

    int idx = sa.rank(String("z"));
    EXPECT_GE(idx, 0);
}
