#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "Bitmap.h"
#include "Eratosthenes.h"

TEST(BitmapTest, SetClearAndExpandPreservesBits) {
    Bitmap bm(8);

    bm.set(0);
    bm.set(7);
    EXPECT_TRUE(bm.test(0));
    EXPECT_TRUE(bm.test(7));

    bm.clear(7);
    EXPECT_FALSE(bm.test(7));

    bm.set(8);
    EXPECT_TRUE(bm.test(8));
    EXPECT_TRUE(bm.test(0));
}

TEST(BitmapTest, DumpAndLoadFromFile) {
    const std::string path = "bitmap_test.bin";
    {
        Bitmap bm(16);
        bm.set(0);
        bm.set(2);
        bm.set(5);
        bm.dump(path.c_str());
    }

    Bitmap loaded(path.c_str(), 16);
    EXPECT_TRUE(loaded.test(0));
    EXPECT_FALSE(loaded.test(1));
    EXPECT_TRUE(loaded.test(2));
    EXPECT_FALSE(loaded.test(3));
    EXPECT_FALSE(loaded.test(4));
    EXPECT_TRUE(loaded.test(5));
    EXPECT_FALSE(loaded.test(6));
    EXPECT_FALSE(loaded.test(7));

    std::remove(path.c_str());
}

TEST(BitmapTest, EratosthenesToFileWritesCompositeBits) {
    const std::string path = "eratosthenes_test.bin";

    eratosthenes_to_file(32, path.c_str());

    Bitmap loaded(path.c_str(), 32);
    EXPECT_TRUE(loaded.test(0));
    EXPECT_TRUE(loaded.test(1));
    EXPECT_FALSE(loaded.test(2));
    EXPECT_TRUE(loaded.test(4));
    EXPECT_FALSE(loaded.test(5));

    std::remove(path.c_str());
}
