#include <gtest/gtest.h>

#include <deque>
#include <list>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "dsa/container/list/List.h"
#include "dsa/container/queue/Queue.h"
#include "dsa/container/stack/Stack.h"
#include "dsa/container/vector/Vector.h"

namespace {

template<typename T>
using IndustrialStack = dsa::container::Stack<T>;

template<typename T>
using IndustrialQueue = dsa::container::Queue<T>;

/// 记录 allocator 是否真正参与底层容器的资源申请和释放。
struct AllocationState {
    int allocations;
    int deallocations;

    AllocationState()
        : allocations(0), deallocations(0) {
    }
};

/// 为标准容器提供可观察的 stateful allocator，验证适配器 allocator 构造转发。
template<typename T>
class CountingAllocator {
public:
    typedef T value_type;

    AllocationState* state;

    CountingAllocator() noexcept
        : state(nullptr) {
    }

    explicit CountingAllocator(AllocationState* allocationState) noexcept
        : state(allocationState) {
    }

    template<typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : state(other.state) {
    }

    T* allocate(std::size_t count) {
        if (state != nullptr)
            ++state->allocations;
        return std::allocator<T>().allocate(count);
    }

    void deallocate(T* memory, std::size_t count) noexcept {
        if (state != nullptr)
            ++state->deallocations;
        std::allocator<T>().deallocate(memory, count);
    }

    template<typename U>
    struct rebind {
        typedef CountingAllocator<U> other;
    };
};

template<typename T, typename U>
bool operator==(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return left.state == right.state;
}

template<typename T, typename U>
bool operator!=(
    const CountingAllocator<T>& left,
    const CountingAllocator<U>& right
) {
    return !(left == right);
}

} // namespace

/// 验证默认工业 Stack 组合工业 Vector，而不是继承并暴露连续容器完整 API。
TEST(IndustrialStackTest, DefaultAdapterUsesCompositionAndLifoSemantics) {
    static_assert(
        !std::is_base_of<
            dsa::container::Vector<int>,
            IndustrialStack<int>
        >::value,
        "industrial Stack must be a container adapter, not a Vector subclass"
    );

    IndustrialStack<int> stack;
    EXPECT_TRUE(stack.empty());

    stack.push(1);
    stack.emplace(2);
    stack.push(3);

    EXPECT_EQ(stack.size(), 3U);
    EXPECT_EQ(stack.top(), 3);
    stack.pop();
    EXPECT_EQ(stack.top(), 2);
}

/// 验证默认工业 Queue 组合工业 List，并按 FIFO 顺序操作链表首尾。
TEST(IndustrialQueueTest, DefaultAdapterUsesCompositionAndFifoSemantics) {
    static_assert(
        !std::is_base_of<
            dsa::container::List<int>,
            IndustrialQueue<int>
        >::value,
        "industrial Queue must be a container adapter, not a List subclass"
    );

    IndustrialQueue<int> queue;
    EXPECT_TRUE(queue.empty());

    queue.push(1);
    queue.emplace(2);
    queue.push(3);

    EXPECT_EQ(queue.size(), 3U);
    EXPECT_EQ(queue.front(), 1);
    EXPECT_EQ(queue.back(), 3);
    queue.pop();
    EXPECT_EQ(queue.front(), 2);
}

/// 验证适配器只依赖所需容器契约，可替换为标准库容器而无需复制栈算法。
TEST(IndustrialStackTest, CustomUnderlyingContainerIsSupported) {
    typedef std::deque<std::string> Storage;
    Storage initial;
    initial.push_back("first");
    initial.push_back("second");

    dsa::container::Stack<std::string, Storage> stack(std::move(initial));
    EXPECT_EQ(stack.top(), "second");

    stack.push("third");
    EXPECT_EQ(stack.top(), "third");
    stack.pop();
    EXPECT_EQ(stack.top(), "second");
}

/// 验证 Queue 可替换为 std::deque，适配器不依赖工业 List 的节点表示。
TEST(IndustrialQueueTest, CustomUnderlyingContainerIsSupported) {
    typedef std::deque<int> Storage;
    Storage initial;
    initial.push_back(4);
    initial.push_back(5);

    dsa::container::Queue<int, Storage> queue(initial);
    EXPECT_EQ(queue.front(), 4);
    EXPECT_EQ(queue.back(), 5);

    queue.push(6);
    queue.pop();
    EXPECT_EQ(queue.front(), 5);
    EXPECT_EQ(queue.back(), 6);
}

/// 验证 push/emplace 直接转发移动语义，底层容器可保存 move-only 元素。
TEST(IndustrialAdapterTest, MoveOnlyValuesAreSupported) {
    IndustrialStack<std::unique_ptr<int> > stack;
    stack.push(std::unique_ptr<int>(new int(7)));
    stack.emplace(new int(8));
    ASSERT_NE(stack.top(), nullptr);
    EXPECT_EQ(*stack.top(), 8);
    stack.pop();
    EXPECT_EQ(*stack.top(), 7);

    IndustrialQueue<std::unique_ptr<int> > queue;
    queue.push(std::unique_ptr<int>(new int(9)));
    queue.emplace(new int(10));
    EXPECT_EQ(*queue.front(), 9);
    EXPECT_EQ(*queue.back(), 10);
    queue.pop();
    EXPECT_EQ(*queue.front(), 10);
}

/// 验证 const 适配器只能取得只读元素引用。
TEST(IndustrialAdapterTest, ConstAccessorsAreReadOnly) {
    static_assert(
        std::is_same<
            decltype(std::declval<const IndustrialStack<int>&>().top()),
            const int&
        >::value,
        "const Stack::top must return const_reference"
    );
    static_assert(
        std::is_same<
            decltype(std::declval<const IndustrialQueue<int>&>().front()),
            const int&
        >::value,
        "const Queue::front must return const_reference"
    );
    static_assert(
        std::is_same<
            decltype(std::declval<const IndustrialQueue<int>&>().back()),
            const int&
        >::value,
        "const Queue::back must return const_reference"
    );

    IndustrialStack<int> mutableStack;
    mutableStack.push(11);
    const IndustrialStack<int>& stack = mutableStack;
    EXPECT_EQ(stack.top(), 11);

    IndustrialQueue<int> mutableQueue;
    mutableQueue.push(12);
    const IndustrialQueue<int>& queue = mutableQueue;
    EXPECT_EQ(queue.front(), 12);
    EXPECT_EQ(queue.back(), 12);
}

/// 验证复制、移动、比较和 swap 均委托给底层容器语义。
TEST(IndustrialAdapterTest, ValueSemanticsAndSwapFollowUnderlyingContainer) {
    IndustrialStack<int> leftStack;
    leftStack.push(1);
    leftStack.push(2);

    IndustrialStack<int> copiedStack(leftStack);
    EXPECT_EQ(copiedStack, leftStack);
    copiedStack.pop();
    EXPECT_NE(copiedStack, leftStack);

    IndustrialStack<int> movedStack(std::move(copiedStack));
    EXPECT_EQ(movedStack.top(), 1);
    swap(leftStack, movedStack);
    EXPECT_EQ(leftStack.top(), 1);
    EXPECT_EQ(movedStack.top(), 2);

    IndustrialQueue<int> leftQueue;
    leftQueue.push(3);
    leftQueue.push(4);

    IndustrialQueue<int> copiedQueue(leftQueue);
    EXPECT_EQ(copiedQueue, leftQueue);
    copiedQueue.pop();
    EXPECT_NE(copiedQueue, leftQueue);

    IndustrialQueue<int> movedQueue(std::move(copiedQueue));
    EXPECT_EQ(movedQueue.front(), 4);
    swap(leftQueue, movedQueue);
    EXPECT_EQ(leftQueue.front(), 4);
    EXPECT_EQ(movedQueue.front(), 3);
}

/// 验证 allocator-aware 构造参数被转发到底层标准容器，并保持申请释放配对。
TEST(IndustrialAdapterTest, AllocatorAwareConstructorsForwardToUnderlyingContainer) {
    AllocationState stackState;
    typedef CountingAllocator<int> StackAllocator;
    typedef std::vector<int, StackAllocator> StackStorage;

    {
        dsa::container::Stack<int, StackStorage> stack{StackAllocator(&stackState)};
        stack.push(1);
        stack.push(2);
        EXPECT_EQ(stack.top(), 2);

        dsa::container::Stack<int, StackStorage> copied(
            stack,
            StackAllocator(&stackState)
        );
        EXPECT_EQ(copied, stack);
    }

    EXPECT_GT(stackState.allocations, 0);
    EXPECT_EQ(stackState.allocations, stackState.deallocations);

    AllocationState queueState;
    typedef CountingAllocator<int> QueueAllocator;
    typedef std::list<int, QueueAllocator> QueueStorage;

    {
        dsa::container::Queue<int, QueueStorage> queue{QueueAllocator(&queueState)};
        queue.push(3);
        queue.push(4);
        EXPECT_EQ(queue.front(), 3);

        dsa::container::Queue<int, QueueStorage> copied(
            queue,
            QueueAllocator(&queueState)
        );
        EXPECT_EQ(copied, queue);
    }

    EXPECT_GT(queueState.allocations, 0);
    EXPECT_EQ(queueState.allocations, queueState.deallocations);
}

/// 与标准适配器对拍一组长序列，验证共同的 LIFO/FIFO 行为。
TEST(IndustrialAdapterTest, DifferentialSequenceMatchesStandardAdapters) {
    IndustrialStack<int> actualStack;
    std::stack<int, std::vector<int> > expectedStack;

    for (int value = 0; value < 1000; ++value) {
        actualStack.push(value);
        expectedStack.push(value);
        ASSERT_EQ(actualStack.top(), expectedStack.top());
    }
    while (!expectedStack.empty()) {
        ASSERT_EQ(actualStack.top(), expectedStack.top());
        actualStack.pop();
        expectedStack.pop();
    }
    EXPECT_TRUE(actualStack.empty());

    IndustrialQueue<int> actualQueue;
    std::queue<int, std::list<int> > expectedQueue;

    for (int value = 0; value < 1000; ++value) {
        actualQueue.push(value);
        expectedQueue.push(value);
        ASSERT_EQ(actualQueue.front(), expectedQueue.front());
        ASSERT_EQ(actualQueue.back(), expectedQueue.back());
    }
    while (!expectedQueue.empty()) {
        ASSERT_EQ(actualQueue.front(), expectedQueue.front());
        actualQueue.pop();
        expectedQueue.pop();
    }
    EXPECT_TRUE(actualQueue.empty());
}
