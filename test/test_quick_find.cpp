#include <gtest/gtest.h>
#include "QuickFind.h"

TEST(QuickFindTest, ConstructorWithSize) {
    QuickFind qf(10);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(qf.find(i), i);
    }
    EXPECT_EQ(qf.count(), 10);
}

TEST(QuickFindTest, UnionAndFind) {
    QuickFind qf(10);
    qf.unite(1, 2);
    qf.unite(2, 3);
    EXPECT_EQ(qf.find(1), qf.find(2));
    EXPECT_EQ(qf.find(2), qf.find(3));
    EXPECT_NE(qf.find(1), qf.find(4));
    EXPECT_EQ(qf.count(), 8);
}

TEST(QuickFindTest, Connected) {
    QuickFind qf(10);
    EXPECT_FALSE(qf.connected(1, 2));
    qf.unite(1, 2);
    EXPECT_TRUE(qf.connected(1, 2));
    qf.traverse();
}

TEST(QuickFindTest, ConstructorWithFile) {
    std::ofstream testFile("test_data.txt");
    testFile << "10\n1 2\n2 3\n4 5\n";
    testFile.close();

    std::ifstream inputFile("test_data.txt");
    QuickFind qf(inputFile);
    inputFile.close();

    EXPECT_TRUE(qf.connected(1, 2));
    EXPECT_TRUE(qf.connected(2, 3));
    EXPECT_FALSE(qf.connected(1, 4));
    EXPECT_EQ(qf.count(), 7);
}
