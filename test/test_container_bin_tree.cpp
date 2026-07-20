#include <gtest/gtest.h>

#include <dsa/container/tree/BinTree.h>
#include "BinTree.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

struct LifetimeValue {
    static int alive;
    int value;

    explicit LifetimeValue(int v) : value(v) { ++alive; }
    LifetimeValue(const LifetimeValue& other) : value(other.value) { ++alive; }
    LifetimeValue(LifetimeValue&& other) noexcept : value(other.value) {
        ++alive;
        other.value = -1;
    }
    LifetimeValue& operator=(const LifetimeValue&) = default;
    ~LifetimeValue() { --alive; }
};

int LifetimeValue::alive = 0;

struct ThrowOnCopy {
    static int alive;
    static int copiesBeforeThrow;
    int value;

    explicit ThrowOnCopy(int v) : value(v) { ++alive; }
    ThrowOnCopy(const ThrowOnCopy& other) : value(other.value) {
        if (copiesBeforeThrow-- == 0)
            throw std::runtime_error("copy failed");
        ++alive;
    }
    ThrowOnCopy(ThrowOnCopy&& other) noexcept : value(other.value) {
        ++alive;
        other.value = -1;
    }
    ~ThrowOnCopy() { --alive; }
};

int ThrowOnCopy::alive = 0;
int ThrowOnCopy::copiesBeforeThrow = 100;

struct AllocatorState {
    int allocations{0};
    int deallocations{0};
};

template<typename T>
struct StatefulAllocator {
    typedef T value_type;

    AllocatorState* state;
    int id;

    explicit StatefulAllocator(AllocatorState* s = nullptr, int allocatorId = 0)
        : state(s), id(allocatorId) {}

    template<typename U>
    StatefulAllocator(const StatefulAllocator<U>& other)
        : state(other.state), id(other.id) {}

    T* allocate(std::size_t count) {
        if (state)
            state->allocations += static_cast<int>(count);
        return std::allocator<T>().allocate(count);
    }

    void deallocate(T* pointer, std::size_t count) {
        if (state)
            state->deallocations += static_cast<int>(count);
        std::allocator<T>().deallocate(pointer, count);
    }

    template<typename U>
    struct rebind {
        typedef StatefulAllocator<U> other;
    };

    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;
};

template<typename T, typename U>
bool operator==(const StatefulAllocator<T>& lhs, const StatefulAllocator<U>& rhs) {
    return lhs.state == rhs.state && lhs.id == rhs.id;
}

template<typename T, typename U>
bool operator!=(const StatefulAllocator<T>& lhs, const StatefulAllocator<U>& rhs) {
    return !(lhs == rhs);
}

} // namespace

TEST(TeachingBinTreeRefactorTest, ReplacingChildKeepsSizeAndHeightConsistent) {
    BinTree<int> tree;
    BinNode<int>* root = tree.insertAsRoot(10);
    BinNode<int>* left = tree.insertAsLC(root, 5);
    tree.insertAsLC(left, 2);
    tree.insertAsRC(left, 7);

    ASSERT_EQ(tree.size(), 4);
    tree.insertAsLC(root, 6);

    ASSERT_EQ(tree.size(), 2);
    ASSERT_NE(root->lc, nullptr);
    EXPECT_EQ(root->lc->data, 6);
    EXPECT_EQ(root->height, 1);
}

TEST(TeachingBinTreeRefactorTest, AttachLeftReplacesOnlyLeftSubtree) {
    BinTree<int> tree;
    BinNode<int>* root = tree.insertAsRoot(10);
    tree.insertAsLC(root, 5);
    tree.insertAsRC(root, 15);

    BinTree<int>* source = new BinTree<int>();
    BinNode<int>* sourceRoot = source->insertAsRoot(20);
    source->insertAsLC(sourceRoot, 18);

    tree.attachAsLC(root, source);

    EXPECT_EQ(source, nullptr);
    ASSERT_NE(root->lc, nullptr);
    ASSERT_NE(root->rc, nullptr);
    EXPECT_EQ(root->lc->data, 20);
    EXPECT_EQ(root->lc->lc->data, 18);
    EXPECT_EQ(root->lc->parent, root);
    EXPECT_EQ(root->rc->data, 15);
    EXPECT_EQ(tree.size(), 4);
}

TEST(TeachingBinTreeRefactorTest, SharedTraversalAlgorithmsKeepHistoricalOrder) {
    BinTree<int> tree;
    BinNode<int>* root = tree.insertAsRoot(10);
    BinNode<int>* left = tree.insertAsLC(root, 5);
    tree.insertAsRC(root, 15);
    tree.insertAsLC(left, 2);
    tree.insertAsRC(left, 7);

    std::vector<int> preorder;
    tree.travPre([&](int& value) { preorder.push_back(value); });
    EXPECT_EQ(preorder, std::vector<int>({10, 5, 2, 7, 15}));

    std::vector<int> inorder;
    tree.travIn([&](int& value) { inorder.push_back(value); });
    EXPECT_EQ(inorder, std::vector<int>({2, 5, 7, 10, 15}));

    std::vector<int> postorder;
    tree.travPost([&](int& value) { postorder.push_back(value); });
    EXPECT_EQ(postorder, std::vector<int>({2, 7, 5, 15, 10}));

    std::vector<int> levelorder;
    tree.travLevel([&](int& value) { levelorder.push_back(value); });
    EXPECT_EQ(levelorder, std::vector<int>({10, 5, 15, 2, 7}));
}

TEST(TeachingBinTreeRefactorTest, DeepTreeTraversalAndDestructionAreIterative) {
    BinTree<int> tree;
    BinNode<int>* node = tree.insertAsRoot(0);
    const int nodeCount = 50000;
    for (int value = 1; value < nodeCount; ++value) {
        node->lc = new BinNode<int>(value, node);
        node = node->lc;
    }

    EXPECT_EQ(tree.root()->size(), nodeCount);
    std::size_t visited = 0;
    tree.travPost([&](int&) { ++visited; });
    EXPECT_EQ(visited, static_cast<std::size_t>(nodeCount));
}

TEST(IndustrialBinTreeTest, SupportsInsertionTraversalIteratorAndErase) {
    typedef dsa::container::BinTree<int> Tree;
    Tree tree;
    Tree::Node* root = tree.emplace_root(10);
    Tree::Node* left = tree.emplace_left(root, 5);
    tree.emplace_right(root, 15);
    tree.emplace_left(left, 2);
    tree.emplace_right(left, 7);

    EXPECT_EQ(tree.size(), 5U);
    EXPECT_EQ(tree.height(), 2);

    std::vector<int> inorder;
    for (Tree::iterator it = tree.begin(); it != tree.end(); ++it)
        inorder.push_back(*it);
    EXPECT_EQ(inorder, std::vector<int>({2, 5, 7, 10, 15}));

    Tree::iterator mutableIterator = tree.begin();
    Tree::const_iterator constIterator = mutableIterator;
    EXPECT_EQ(*constIterator, 2);

    const Tree& constTree = tree;
    std::vector<int> levelorder;
    constTree.traverse_levelorder([&](const int& value) { levelorder.push_back(value); });
    EXPECT_EQ(levelorder, std::vector<int>({10, 5, 15, 2, 7}));

    EXPECT_EQ(tree.erase_subtree(left), 3U);
    EXPECT_EQ(tree.size(), 2U);
    EXPECT_EQ(tree.height(), 1);
    EXPECT_EQ(root->left(), nullptr);
}

TEST(IndustrialBinTreeTest, CopyIsIndependentAndMoveTransfersOwnership) {
    typedef dsa::container::BinTree<int> Tree;
    Tree source;
    Tree::Node* root = source.emplace_root(10);
    source.emplace_left(root, 5);
    source.emplace_right(root, 15);

    Tree copy(source);
    ASSERT_EQ(copy.size(), source.size());
    copy.root()->value() = 99;
    EXPECT_EQ(source.root()->value(), 10);

    Tree moved(std::move(copy));
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(moved.size(), 3U);
    EXPECT_EQ(moved.root()->value(), 99);
}

TEST(IndustrialBinTreeTest, SupportsMoveOnlyNonDefaultConstructibleValues) {
    typedef dsa::container::BinTree<std::unique_ptr<int> > Tree;
    Tree source;
    Tree::Node* root = source.emplace_root(new int(10));
    source.emplace_left(root, new int(5));

    Tree moved(std::move(source));
    EXPECT_TRUE(source.empty());
    ASSERT_EQ(moved.size(), 2U);
    EXPECT_EQ(**moved.begin(), 5);
}

TEST(IndustrialBinTreeTest, ClearImmediatelyDestroysValues) {
    LifetimeValue::alive = 0;
    {
        dsa::container::BinTree<LifetimeValue> tree;
        dsa::container::BinTree<LifetimeValue>::Node* root = tree.emplace_root(1);
        tree.emplace_left(root, 2);
        tree.emplace_right(root, 3);
        EXPECT_EQ(LifetimeValue::alive, 3);

        tree.clear();
        EXPECT_EQ(LifetimeValue::alive, 0);
        EXPECT_TRUE(tree.empty());
    }
    EXPECT_EQ(LifetimeValue::alive, 0);
}

TEST(IndustrialBinTreeTest, CopyConstructionRollsBackOnValueException) {
    ThrowOnCopy::alive = 0;
    {
        dsa::container::BinTree<ThrowOnCopy> source;
        dsa::container::BinTree<ThrowOnCopy>::Node* root = source.emplace_root(1);
        source.emplace_left(root, 2);
        source.emplace_right(root, 3);
        ASSERT_EQ(ThrowOnCopy::alive, 3);

        ThrowOnCopy::copiesBeforeThrow = 1;
        EXPECT_THROW((dsa::container::BinTree<ThrowOnCopy>(source)), std::runtime_error);
        EXPECT_EQ(ThrowOnCopy::alive, 3);
        EXPECT_EQ(source.size(), 3U);
    }
    EXPECT_EQ(ThrowOnCopy::alive, 0);
}

TEST(IndustrialBinTreeTest, UnequalAllocatorMoveRebuildsAndSwapRejectsOwnershipMismatch) {
    typedef dsa::container::BinTree<int, StatefulAllocator<int> > Tree;
    AllocatorState firstState;
    AllocatorState secondState;

    {
        Tree source(StatefulAllocator<int>(&firstState, 1));
        Tree::Node* sourceRoot = source.emplace_root(10);
        source.emplace_left(sourceRoot, 5);

        Tree target(StatefulAllocator<int>(&secondState, 2));
        target.emplace_root(99);
        target = std::move(source);

        EXPECT_TRUE(source.empty());
        EXPECT_EQ(target.size(), 2U);
        EXPECT_EQ(target.root()->value(), 10);
        EXPECT_EQ(target.get_allocator().id, 2);

        Tree other(StatefulAllocator<int>(&firstState, 1));
        other.emplace_root(7);
        EXPECT_THROW(target.swap(other), std::logic_error);
    }

    EXPECT_EQ(firstState.allocations, firstState.deallocations);
    EXPECT_EQ(secondState.allocations, secondState.deallocations);
}

TEST(IndustrialBinTreeTest, RejectsOccupiedAndForeignNodeMutations) {
    typedef dsa::container::BinTree<int> Tree;
    Tree first;
    Tree::Node* firstRoot = first.emplace_root(1);
    first.emplace_left(firstRoot, 2);

    Tree second;
    Tree::Node* secondRoot = second.emplace_root(3);

    EXPECT_THROW(first.emplace_left(firstRoot, 4), std::logic_error);
    EXPECT_THROW(first.emplace_right(secondRoot, 4), std::invalid_argument);
    EXPECT_THROW(first.erase_subtree(secondRoot), std::invalid_argument);
    EXPECT_EQ(first.size(), 2U);
}
