#include <gtest/gtest.h>
#include "WeightedQuickUnion.h"

TEST(WeightedQuickUnionTest, ConstructorWithSize) {
    WeightedQuickUnion wqu(10);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(wqu.find(i), i);
    }
    EXPECT_EQ(wqu.count(), 10);
}

TEST(WeightedQuickUnionTest, UnionAndFind) {
    WeightedQuickUnion wqu(10);
    wqu.unite(1, 2);
    wqu.unite(2, 3);
    EXPECT_EQ(wqu.find(1), wqu.find(2));
    EXPECT_EQ(wqu.find(2), wqu.find(3));
    EXPECT_NE(wqu.find(1), wqu.find(4));
    EXPECT_EQ(wqu.count(), 8);
}

TEST(WeightedQuickUnionTest, Connected) {
    WeightedQuickUnion wqu(10);
    EXPECT_FALSE(wqu.connected(1, 2));
    wqu.unite(1, 2);
    EXPECT_TRUE(wqu.connected(1, 2));
}

TEST(WeightedQuickUnionTest, ConstructorWithFile) {
    std::ofstream testFile("test_data_wqu.txt");
    testFile << "10\n1 2\n2 3\n4 5\n";
    testFile.close();

    std::ifstream inputFile("test_data_wqu.txt");
    WeightedQuickUnion wqu(inputFile);
    inputFile.close();

    EXPECT_TRUE(wqu.connected(1, 2));
    EXPECT_TRUE(wqu.connected(2, 3));
    EXPECT_FALSE(wqu.connected(1, 4));
    EXPECT_EQ(wqu.count(), 7);
}
