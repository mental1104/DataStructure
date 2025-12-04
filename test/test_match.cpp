#include <gtest/gtest.h>
#include "match/KMP.h"
#include "match/BM.h"
#include "match/KR.h"
#include "match/MatchObserver.h"

TEST(MatchKMP, BasicAndImprovedMatch) {
    String text("ababcababd");
    String pattern("ababd");
    int idx_basic = matchKMP(pattern, text, KMPStrategy::Basic, nullptr);
    int idx_improved = matchKMP(pattern, text, KMPStrategy::Improved, nullptr);
    EXPECT_EQ(idx_basic, 5);
    EXPECT_EQ(idx_improved, 5);
}

TEST(MatchBM, BadCharacterAndFull) {
    String text("find the needle in haystack");
    String pattern("needle");
    int idx_bc = matchBM(pattern, text, BMStrategy::BadCharacter, nullptr);
    int idx_full = matchBM(pattern, text, BMStrategy::Full, nullptr);
    EXPECT_EQ(idx_bc, 9);
    EXPECT_EQ(idx_full, 9);
}

TEST(MatchKR, FindsNumericSubstring) {
    String text("31415926");
    String pattern("159");
    int idx = matchKR(pattern, text, nullptr);
    EXPECT_EQ(idx, 3);
}
