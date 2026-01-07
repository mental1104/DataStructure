#include <gtest/gtest.h>
#include <array>
#include <cstdio>
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

TEST(MatchBM, BuildSSTrickyPattern) {
    String pattern("abab");
    int* ss = buildSS(pattern, static_cast<int>(pattern.size()));
    EXPECT_EQ(ss[1], 2);
    delete[] ss;
}

TEST(MatchKR, FindsNumericSubstring) {
    String text("31415926");
    String pattern("159");
    int idx = matchKR(pattern, text, nullptr);
    EXPECT_EQ(idx, 3);
}

TEST(MatchObserverTest, StdoutObserverCallbacks) {
    StdoutMatchObserver observer;
    String pattern("aba");
    int next[] = {-1, 0, 1};
    observer.onNextTable(pattern, next, 3);
    observer.onNextTable(pattern, nullptr, 3);

    String text("ababa");
    observer.onProgress(text, pattern, 1, 1, nullptr, 0);

    std::array<int, 128> bc;
    bc.fill(-1);
    bc[static_cast<unsigned char>('a')] = 0;
    bc[static_cast<unsigned char>('b')] = 1;
    observer.onBCTable(bc.data(), static_cast<int>(bc.size()));

    int gs[] = {1, 2, 3};
    observer.onGSTable(gs, 3, pattern);

    observer.onKRProgress(text, pattern, 2, 123, 456);

    ungetc('\n', stdin);
    observer.onPause();

    MatchObserver* base = new MatchObserver();
    delete base;

    NoopMatchObserver noop;
    noop.onNextTable(pattern, next, 3);
    noop.onProgress(text, pattern, 0, 0, nullptr, 0);
    noop.onBCTable(next, 3);
    noop.onGSTable(gs, 3, pattern);
    noop.onKRProgress(text, pattern, 0, 0, 0);
}
