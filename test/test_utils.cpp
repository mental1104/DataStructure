#include <gtest/gtest.h>
#include "func.h"
#include "rand.h"
#include "common.h"

TEST(UtilsFuncTest, FactorialAndFunctors) {
    EXPECT_EQ(facI(5), 120);
    int value = 3;
    Double<int> dbl;
    Increment<int> inc;
    Decrement<int> dec;
    dbl(value);
    inc(value);
    dec(value);
    EXPECT_EQ(value, 6);
}

TEST(UtilsRandTest, DiceRanges) {
    int v1 = dice(10);
    EXPECT_GE(v1, 0);
    EXPECT_LT(v1, 10);

    int v2 = dice(5, 10);
    EXPECT_GE(v2, 5);
    EXPECT_LT(v2, 10);

    float f = dice(5.0f);
    EXPECT_GE(f, 0.0f);
    EXPECT_LT(f, 5.0f);

    double d = dice(5.0);
    EXPECT_GE(d, 0.0);
    EXPECT_LT(d, 5.0);

    unsigned char c = static_cast<unsigned char>(dice());
    EXPECT_GE(static_cast<int>(c), 32);
    EXPECT_LE(static_cast<int>(c), 127);
}

TEST(UtilsCommonTest, SwapMinMaxAndSleep) {
    int a = 1;
    int b = 2;
    swap(a, b);
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(::min(3, 4), 3);
    EXPECT_EQ(::max(3, 4), 4);
    sleep_seconds(0);
    clear_screen();
}
