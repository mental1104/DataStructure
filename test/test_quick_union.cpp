#include <gtest/gtest.h>
#include "QuickUnion.h"

TEST(QuickUnionTest, ConstructorWithSize) {
    QuickUnion qu(10);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(qu.find(i), i);
    }
    EXPECT_EQ(qu.count(), 10);
}

TEST(QuickUnionTest, UnionAndFind) {
    QuickUnion qu(10);
    qu.unite(1, 2);
    qu.unite(2, 3);
    EXPECT_EQ(qu.find(1), qu.find(2));
    EXPECT_EQ(qu.find(2), qu.find(3));
    EXPECT_NE(qu.find(1), qu.find(4));
    EXPECT_EQ(qu.count(), 8);
}

TEST(QuickUnionTest, Connected) {
    QuickUnion qu(10);
    EXPECT_FALSE(qu.connected(1, 2));
    qu.unite(1, 2);
    EXPECT_TRUE(qu.connected(1, 2));
}

TEST(QuickUnionTest, ConstructorWithFile) {
    std::ofstream testFile("test_data_qu.txt");
    testFile << "10\n1 2\n2 3\n4 5\n";
    testFile.close();

    std::ifstream inputFile("test_data_qu.txt");
    QuickUnion qu(inputFile);
    inputFile.close();

    EXPECT_TRUE(qu.connected(1, 2));
    EXPECT_TRUE(qu.connected(2, 3));
    EXPECT_FALSE(qu.connected(1, 4));
    EXPECT_EQ(qu.count(), 7);
}