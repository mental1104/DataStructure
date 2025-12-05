#include <gtest/gtest.h>

#include "SegmentTree.h"

struct SumOp {
    int operator()(int a, int b) const { return a + b; }
};

TEST(SegmentTreeTest, BuildAndQuery) {
    Vector<int> data(8, 8, 0);
    for (int i = 0; i < 8; ++i) data[i] = i + 1; // 1..8

    SegmentTree<int, SumOp> st(data, SumOp(), 0);

    EXPECT_EQ(st.query(0, 8), 36);
    EXPECT_EQ(st.query(0, 3), 6);
    EXPECT_EQ(st.query(2, 5), 12);
    EXPECT_EQ(st.query(4, 8), 26);
}

TEST(SegmentTreeTest, PointUpdate) {
    Vector<int> data(5, 5, 0);
    for (int i = 0; i < 5; ++i) data[i] = 1;

    SegmentTree<int, SumOp> st(data, SumOp(), 0);
    EXPECT_EQ(st.query(0, 5), 5);

    st.update(2, 10);
    EXPECT_EQ(st.query(0, 5), 14);
    EXPECT_EQ(st.query(2, 3), 10);

    st.update(0, -4);
    EXPECT_EQ(st.query(0, 2), -3);
    EXPECT_EQ(st.query(0, 5), 9);
}
