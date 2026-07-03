#include <gtest/gtest.h>

#include "rb_algorithm.hpp"

namespace {

struct RawNode {
    explicit RawNode(int value)
        : value(value),
          parent(0),
          left(0),
          right(0),
          color(dsa::rb::color::black) {}

    int value;
    RawNode* parent;
    RawNode* left;
    RawNode* right;
    dsa::rb::color color;
};

typedef dsa::rb::intrusive_node_traits<RawNode> RawNodeTraits;

void link_left(RawNode* parent, RawNode* child) {
    parent->left = child;
    if (child) {
        child->parent = parent;
    }
}

void link_right(RawNode* parent, RawNode* child) {
    parent->right = child;
    if (child) {
        child->parent = parent;
    }
}

}  // namespace

TEST(RBAlgorithmTest, RotateLeftRelinksRootAndParents) {
    RawNode root(10);
    RawNode pivot(20);
    RawNode middle(15);
    RawNode* root_ptr = &root;

    link_right(&root, &pivot);
    link_left(&pivot, &middle);

    RawNode* new_root = dsa::rb::rotate_left<RawNode, RawNodeTraits>(root_ptr, &root);

    ASSERT_EQ(new_root, &pivot);
    EXPECT_EQ(root_ptr, &pivot);
    EXPECT_EQ(pivot.parent, nullptr);
    EXPECT_EQ(pivot.left, &root);
    EXPECT_EQ(root.parent, &pivot);
    EXPECT_EQ(root.right, &middle);
    EXPECT_EQ(middle.parent, &root);
}

TEST(RBAlgorithmTest, MinimumMaximumSuccessorPredecessor) {
    RawNode n10(10);
    RawNode n5(5);
    RawNode n15(15);
    RawNode n12(12);
    RawNode n20(20);

    link_left(&n10, &n5);
    link_right(&n10, &n15);
    link_left(&n15, &n12);
    link_right(&n15, &n20);

    EXPECT_EQ(dsa::rb::minimum<RawNode, RawNodeTraits>(&n10), &n5);
    EXPECT_EQ(dsa::rb::maximum<RawNode, RawNodeTraits>(&n10), &n20);
    EXPECT_EQ(dsa::rb::successor<RawNode, RawNodeTraits>(&n10), &n12);
    EXPECT_EQ(dsa::rb::successor<RawNode, RawNodeTraits>(&n12), &n15);
    EXPECT_EQ(dsa::rb::predecessor<RawNode, RawNodeTraits>(&n15), &n12);
    EXPECT_EQ(dsa::rb::predecessor<RawNode, RawNodeTraits>(&n5), nullptr);
}
