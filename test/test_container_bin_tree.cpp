#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "dsa/container/tree/BinTree.h"

namespace {

struct AllocationState {
    int outstanding;
    int allocations;
    int deallocations;

    AllocationState()
        : outstanding(0), allocations(0), deallocations(0) {}
};

template<typename T>
class CountingAllocator {
public:
    typedef T value_type;

    AllocationState* state;

    CountingAllocator() noexcept
        : state(nullptr) {}

    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {}

    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : state(other.state) {}

    T* allocate(std::size_t count) {
        if (state) {
            state->outstanding += static_cast<int>(count);
            state->allocations += static_cast<int>(count);
        }
        return std::allocator<T>().allocate(count);
    }

    void deallocate(T* pointer, std::size_t count) noexcept {
        if (state) {
            state->outstanding -= static_cast<int>(count);
            state->deallocations += static_cast<int>(count);
        }
        std::allocator<T>().deallocate(pointer, count);
    }

    template<typename U>
    bool operator==(const CountingAllocator<U>& other) const noexcept {
        return state == other.state;
    }

    template<typename U>
    bool operator!=(const CountingAllocator<U>& other) const noexcept {
        return !(*this == other);
    }
};

struct MoveOnlyValue {
    int value;

    explicit MoveOnlyValue(int input)
        : value(input) {}

    MoveOnlyValue(MoveOnlyValue&& other) noexcept
        : value(other.value) {
        other.value = -1;
    }

    MoveOnlyValue& operator=(MoveOnlyValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }

    MoveOnlyValue(const MoveOnlyValue&) = delete;
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;
};

struct ThrowingValue {
    static int liveCount;
    static bool throwOnConstruction;

    int value;

    explicit ThrowingValue(int input)
        : value(input) {
        if (throwOnConstruction)
            throw std::runtime_error("construction failed");
        ++liveCount;
    }

    ThrowingValue(const ThrowingValue& other)
        : value(other.value) {
        if (throwOnConstruction)
            throw std::runtime_error("copy failed");
        ++liveCount;
    }

    ThrowingValue(ThrowingValue&& other)
        : value(other.value) {
        if (throwOnConstruction)
            throw std::runtime_error("move failed");
        other.value = -1;
        ++liveCount;
    }

    ~ThrowingValue() {
        --liveCount;
    }
};

int ThrowingValue::liveCount = 0;
bool ThrowingValue::throwOnConstruction = false;

struct RedBlackMetadata : dsa::container::BinTreeNodeMetadata {
    bool red;

    RedBlackMetadata() noexcept
        : red(true) {}
};

template<typename T, typename Allocator = std::allocator<T> >
class RedBlackExtensionSmoke final
    : public dsa::container::BasicBinTree<
          RedBlackExtensionSmoke<T, Allocator>,
          T,
          RedBlackMetadata,
          Allocator
      > {
private:
    typedef dsa::container::BasicBinTree<
        RedBlackExtensionSmoke<T, Allocator>,
        T,
        RedBlackMetadata,
        Allocator
    > Base;
    typedef typename Base::node_type Node;

    friend class dsa::core::BinTreeAlgorithm<
        RedBlackExtensionSmoke<T, Allocator>,
        T,
        Node
    >;

protected:
    int updateHeight(Node* node) noexcept {
        const int childHeight = node->lc ? node->lc->height : 0;
        node->height = childHeight + (node->red ? 0 : 1);
        return node->height;
    }

public:
    RedBlackExtensionSmoke()
        : Base() {}
};

} // namespace

TEST(ContainerBinTreeTest, UsesCustomAllocatorForEveryOwnedNode) {
    AllocationState state;
    typedef CountingAllocator<std::string> Allocator;
    typedef dsa::container::BinTree<std::string, Allocator> Tree;

    {
        Tree tree{Allocator(&state)};
        Tree::node_pointer root = tree.emplace_root("root");
        tree.emplace_left(root, "left");
        tree.emplace_right(root, "right");

        EXPECT_EQ(tree.size(), 3U);
        EXPECT_EQ(state.outstanding, 3);

        Tree copied(tree);
        EXPECT_EQ(copied.size(), 3U);
        EXPECT_EQ(state.outstanding, 6);

        tree.clear();
        EXPECT_EQ(state.outstanding, 3);
    }

    EXPECT_EQ(state.outstanding, 0);
    EXPECT_EQ(state.allocations, state.deallocations);
}

TEST(ContainerBinTreeTest, SupportsMoveOnlyAndNonDefaultConstructibleValues) {
    dsa::container::BinTree<MoveOnlyValue> tree;
    dsa::container::BinTree<MoveOnlyValue>::node_pointer root =
        tree.emplace_root(10);
    tree.emplace_left(root, 5);
    tree.emplace_right(root, 15);

    std::vector<int> values;
    tree.traverse_in([&values](MoveOnlyValue& value) {
        values.push_back(value.value);
    });

    EXPECT_EQ(values, std::vector<int>({5, 10, 15}));
}

TEST(ContainerBinTreeTest, FailedReplacementPreservesExistingSubtree) {
    ThrowingValue::throwOnConstruction = false;

    {
        dsa::container::BinTree<ThrowingValue> tree;
        dsa::container::BinTree<ThrowingValue>::node_pointer root =
            tree.emplace_root(10);
        tree.emplace_left(root, 5);

        ASSERT_EQ(ThrowingValue::liveCount, 2);
        ThrowingValue::throwOnConstruction = true;

        EXPECT_THROW(tree.reset_left(root, 7), std::runtime_error);

        ThrowingValue::throwOnConstruction = false;
        ASSERT_NE(root->lc, nullptr);
        EXPECT_EQ(root->lc->data.value, 5);
        EXPECT_EQ(tree.size(), 2U);
        EXPECT_EQ(ThrowingValue::liveCount, 2);
    }

    ThrowingValue::throwOnConstruction = false;
    EXPECT_EQ(ThrowingValue::liveCount, 0);
}

TEST(ContainerBinTreeTest, ProvidesConstCorrectBidirectionalIteration) {
    dsa::container::BinTree<int> tree;
    dsa::container::BinTree<int>::node_pointer root = tree.emplace_root(10);
    dsa::container::BinTree<int>::node_pointer left = tree.emplace_left(root, 5);
    tree.emplace_right(root, 15);
    tree.emplace_left(left, 2);
    tree.emplace_right(left, 7);

    const dsa::container::BinTree<int>& constTree = tree;
    dsa::container::BinTree<int>::const_iterator iterator = constTree.begin();

    static_assert(
        std::is_same<decltype(*iterator), const int&>::value,
        "const_iterator must expose const value references"
    );

    std::vector<int> values;
    for (; iterator != constTree.end(); ++iterator)
        values.push_back(*iterator);

    EXPECT_EQ(values, std::vector<int>({2, 5, 7, 10, 15}));

    iterator = constTree.end();
    --iterator;
    EXPECT_EQ(*iterator, 15);
}

TEST(ContainerBinTreeTest, CrtpExtensionHasNoVirtualDispatch) {
    typedef dsa::container::BinTree<int> Tree;
    typedef RedBlackExtensionSmoke<int> RedBlackTree;

    static_assert(!std::is_polymorphic<Tree>::value,
                  "industrial BinTree must not contain a vtable");
    static_assert(!std::is_polymorphic<RedBlackTree>::value,
                  "derived balanced trees must remain statically dispatched");

    RedBlackTree tree;
    RedBlackTree::node_pointer root = tree.emplace_root(10);
    tree.emplace_left(root, 5);

    EXPECT_EQ(tree.size(), 2U);
    EXPECT_EQ(root->height, 0);
}
