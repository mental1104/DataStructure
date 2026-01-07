#include <gtest/gtest.h>
#include "WeightedQuickUnionwithCompression.h"

// 测试构造函数和初始状态
TEST(WeightedQuickUnionwithCompressionTest, InitialState) {
    WeightedQuickUnionwithCompression wqu(10);

    // 初始时，每个元素应该是它自己的根
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(wqu.find(i), i);
    }
}

// 测试 unite 和 find 方法
TEST(WeightedQuickUnionwithCompressionTest, UniteAndFind) {
    WeightedQuickUnionwithCompression wqu(10);

    wqu.unite(0, 1);
    wqu.unite(1, 2);
    
    // 在联合了 0-1 和 1-2 后，它们都应该有相同的根
    EXPECT_EQ(wqu.find(0), wqu.find(1));
    EXPECT_EQ(wqu.find(1), wqu.find(2));
    EXPECT_EQ(wqu.find(0), wqu.find(2));
}

// 测试路径压缩
TEST(WeightedQuickUnionwithCompressionTest, PathCompression) {
    WeightedQuickUnionwithCompression wqu(10);

    wqu.unite(0, 1);
    wqu.unite(1, 2);
    wqu.unite(2, 3);

    // 执行 find 操作以触发路径压缩
    wqu.find(3);

    // 路径压缩后，3 的根应该是 0
    EXPECT_EQ(wqu.find(3), 0);
    EXPECT_EQ(wqu.find(2), 0); // 2 应该也直接连接到 0
    EXPECT_EQ(wqu.find(1), 0); // 1 应该也直接连接到 0
}

// 测试 connected 方法
TEST(WeightedQuickUnionwithCompressionTest, Connected) {
    WeightedQuickUnionwithCompression wqu(10);

    wqu.unite(0, 1);
    wqu.unite(1, 2);

    // 0 和 2 应该是连接的
    EXPECT_TRUE(wqu.connected(0, 2));
    // 0 和 3 不应该是连接的
    EXPECT_FALSE(wqu.connected(0, 3));
}

// 测试边界情况：尝试联合已经连接的元素
TEST(WeightedQuickUnionwithCompressionTest, UniteAlreadyConnected) {
    WeightedQuickUnionwithCompression wqu(10);

    wqu.unite(0, 1);
    wqu.unite(1, 2);
    
    // 如果元素已经连接，联合操作不应该有任何变化
    wqu.unite(0, 2);
    EXPECT_EQ(wqu.find(0), wqu.find(2));
}

// 测试边界情况：自我联合
TEST(WeightedQuickUnionwithCompressionTest, SelfUnite) {
    WeightedQuickUnionwithCompression wqu(10);

    // 将一个元素与自己联合，应该不会改变它的根
    wqu.unite(0, 0);
    EXPECT_EQ(wqu.find(0), 0);
}

TEST(WeightedQuickUnionwithCompressionTest, DestructorRuns) {
    WeightedQuickUnionwithCompression* wqu = new WeightedQuickUnionwithCompression(3);
    wqu->unite(0, 1);
    delete wqu;
}
