#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Bitmap.h"
#include "Eratosthenes.h"

TEST(BitmapTest, SetClearFlipAndExpandPreservesBits) {
    Bitmap bitmap(8);

    bitmap.set(0);
    bitmap.set(7);
    bitmap.set(8);
    EXPECT_TRUE(bitmap.test(0));
    EXPECT_TRUE(bitmap.test(7));
    EXPECT_TRUE(bitmap.test(8));
    EXPECT_EQ(bitmap.size(), 9u);
    EXPECT_EQ(bitmap.count(), 3u);

    bitmap.clear(7);
    bitmap.flip(8);
    EXPECT_FALSE(bitmap.test(7));
    EXPECT_FALSE(bitmap.test(8));
    EXPECT_TRUE(bitmap.test(0));
}

TEST(BitmapTest, CopyAndMoveOwnStorageIndependently) {
    Bitmap source(16);
    source.set(1);
    source.set(15);

    Bitmap copied(source);
    copied.clear(1);
    EXPECT_TRUE(source.test(1));
    EXPECT_FALSE(copied.test(1));

    Bitmap assigned(1);
    assigned = source;
    assigned.flip(15);
    EXPECT_TRUE(source.test(15));
    EXPECT_FALSE(assigned.test(15));

    Bitmap moved(std::move(copied));
    EXPECT_TRUE(moved.test(15));
    EXPECT_TRUE(copied.empty());

    Bitmap moveAssigned(8);
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(moveAssigned.test(1));
    EXPECT_TRUE(assigned.empty());
}

TEST(BitmapTest, ConstTestDoesNotExpand) {
    Bitmap bitmap(8);
    bitmap.set(0);

    const Bitmap& view = bitmap;
    const std::size_t originalSize = view.size();
    const std::size_t originalCapacity = view.capacity();

    EXPECT_FALSE(view.test(100));
    EXPECT_EQ(view.size(), originalSize);
    EXPECT_EQ(view.capacity(), originalCapacity);
}

TEST(BitmapTest, RejectsNegativeSizeAndIndex) {
    EXPECT_THROW(Bitmap(-1), std::invalid_argument);

    Bitmap bitmap(8);
    EXPECT_THROW(bitmap.set(-1), std::out_of_range);
    EXPECT_THROW(bitmap.clear(-1), std::out_of_range);
    EXPECT_THROW(bitmap.flip(-1), std::out_of_range);
    EXPECT_THROW(bitmap.test(-1), std::out_of_range);
}

TEST(BitmapTest, BulkOperationsAndBitsToStringRespectLogicalSize) {
    Bitmap bitmap(10);
    bitmap.set();
    EXPECT_EQ(bitmap.count(), 10u);
    EXPECT_TRUE(bitmap.all());

    bitmap.flip();
    EXPECT_TRUE(bitmap.none());

    bitmap.set(0);
    bitmap.set(9);
    char* text = bitmap.bits2string(10);
    EXPECT_EQ(std::string(text), "1000000001");
    delete[] text;

    bitmap.clear();
    EXPECT_TRUE(bitmap.none());
    EXPECT_EQ(bitmap.size(), 10u);
}

TEST(BitmapTest, DumpUsesLegacyMsbFirstByteOrder) {
    const std::string path = "bitmap_test.bin";
    {
        Bitmap bitmap(9);
        bitmap.set(0);
        bitmap.set(7);
        bitmap.set(8);
        bitmap.dump(path.c_str());
    }

    std::ifstream input(path.c_str(), std::ios::binary);
    unsigned char bytes[2] = {0, 0};
    input.read(reinterpret_cast<char*>(bytes), 2);
    EXPECT_EQ(bytes[0], static_cast<unsigned char>(0x81));
    EXPECT_EQ(bytes[1], static_cast<unsigned char>(0x80));

    Bitmap loaded(path.c_str(), 9);
    EXPECT_TRUE(loaded.test(0));
    EXPECT_TRUE(loaded.test(7));
    EXPECT_TRUE(loaded.test(8));
    EXPECT_EQ(loaded.count(), 3u);

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
    EXPECT_TRUE(loaded.test(6));
    EXPECT_FALSE(loaded.test(31));

    std::remove(path.c_str());
}
