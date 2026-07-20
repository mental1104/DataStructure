#ifndef DSA_CONTAINER_HEAP_DETAIL_HEAP_POLICY_H
#define DSA_CONTAINER_HEAP_DETAIL_HEAP_POLICY_H

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dsa/core/heap/HeapAlgorithm.h>

namespace dsa {
namespace container {
namespace detail {

// 将 std 风格 Compare 适配为“first 是否应位于堆顶方向”。
template<typename T, typename Compare>
class HeapHigher {
public:
    explicit HeapHigher(const Compare& compare) : compare_(&compare) {}
    bool operator()(const T& first, const T& second) const {
        return (*compare_)(second, first);
    }
private:
    const Compare* compare_;
};

// 节点堆统一节点表示；不同 Policy 只解释 left/right 元数据的语义。
template<typename T>
struct HeapNode {
    template<typename... Args>
    explicit HeapNode(Args&&... args)
        : value(std::forward<Args>(args)...), parent(NULL), left(NULL), right(NULL),
          npl(1), degree(0), marked(false) {}

    T value;
    HeapNode* parent;
    HeapNode* left;
    HeapNode* right;
    std::size_t npl;
    std::size_t degree;
    bool marked;
};

// 普通左右孩子节点访问器，供左式堆和斜堆共用。
template<typename Node>
struct BinaryNodeAccess {
    typedef Node node_type;
    typedef typename std::remove_reference<decltype(std::declval<Node>().value)>::type value_type;

    static node_type*& parent(node_type* node) { return node->parent; }
    static node_type*& left(node_type* node) { return node->left; }
    static node_type*& right(node_type* node) { return node->right; }
    static value_type& value(node_type* node) { return node->value; }
    static const value_type& value(const node_type* node) { return node->value; }
    static std::size_t nplValue(const node_type* node) { return node ? node->npl : 0; }
    static std::size_t& nplRef(node_type* node) { return node->npl; }
};

// 最左孩子/右兄弟节点访问器，供配对堆和斐波那契堆共用。
template<typename Node>
struct ChildSiblingNodeAccess {
    typedef Node node_type;
    typedef std::size_t size_type;
    typedef typename std::remove_reference<decltype(std::declval<Node>().value)>::type value_type;

    static node_type*& parent(node_type* node) { return node->parent; }
    static node_type*& child(node_type* node) { return node->left; }
    static node_type*& sibling(node_type* node) { return node->right; }
    static value_type& value(node_type* node) { return node->value; }
    static const value_type& value(const node_type* node) { return node->value; }
    static std::size_t degreeValue(const node_type* node) { return node->degree; }
    static void incrementDegree(node_type* node) { ++node->degree; }
    static void resetDegree(node_type* node) { node->degree = 0; }
    static void setMarked(node_type* node, bool marked) { node->marked = marked; }
};

// 左式堆结构策略：State 只保存根，节点所有权由外层 NodeHeap 管理。
template<typename Access, typename Higher>
struct LeftistPolicy {
    typedef typename Access::node_type node_type;
    struct State {
        State() : root(NULL) {}
        node_type* root;
    };

    static bool empty(const State& state) { return state.root == NULL; }
    static node_type* best(const State& state) { return state.root; }
    static void insert(State& state, node_type* node, const Higher& higher) {
        state.root = dsa::core::LeftistHeapAlgorithm<Access, Higher>::merge(
            state.root, node, higher
        );
    }
    static node_type* detachBest(State& state, std::size_t, const Higher& higher) {
        node_type* removed = state.root;
        node_type* merged = dsa::core::LeftistHeapAlgorithm<Access, Higher>::merge(
            Access::left(removed), Access::right(removed), higher
        );
        Access::left(removed) = NULL;
        Access::right(removed) = NULL;
        Access::parent(removed) = NULL;
        state.root = merged;
        return removed;
    }
    static void meld(State& first, State& second, const Higher& higher) {
        node_type* merged = dsa::core::LeftistHeapAlgorithm<Access, Higher>::merge(
            first.root, second.root, higher
        );
        first.root = merged;
        second.root = NULL;
    }
    template<typename Visitor>
    static void forEach(State& state, Visitor visitor) {
        dsa::core::forEachBinaryHeapNode<Access>(state.root, visitor);
    }
    template<typename Destroy>
    static void destroy(State& state, Destroy destroy) {
        dsa::core::destroyBinaryHeapTree<Access>(state.root, destroy);
        state.root = NULL;
    }
    static void swapState(State& first, State& second) {
        using std::swap;
        swap(first.root, second.root);
    }
};

// 斜堆结构策略：复用同一所有权容器，仅替换 meld 算法。
template<typename Access, typename Higher>
struct SkewPolicy {
    typedef typename Access::node_type node_type;
    struct State {
        State() : root(NULL) {}
        node_type* root;
    };

    static bool empty(const State& state) { return state.root == NULL; }
    static node_type* best(const State& state) { return state.root; }
    static void insert(State& state, node_type* node, const Higher& higher) {
        state.root = dsa::core::SkewHeapAlgorithm<Access, Higher>::merge(
            state.root, node, higher
        );
    }
    static node_type* detachBest(State& state, std::size_t, const Higher& higher) {
        node_type* removed = state.root;
        node_type* merged = dsa::core::SkewHeapAlgorithm<Access, Higher>::merge(
            Access::left(removed), Access::right(removed), higher
        );
        Access::left(removed) = NULL;
        Access::right(removed) = NULL;
        Access::parent(removed) = NULL;
        state.root = merged;
        return removed;
    }
    static void meld(State& first, State& second, const Higher& higher) {
        node_type* merged = dsa::core::SkewHeapAlgorithm<Access, Higher>::merge(
            first.root, second.root, higher
        );
        first.root = merged;
        second.root = NULL;
    }
    template<typename Visitor>
    static void forEach(State& state, Visitor visitor) {
        dsa::core::forEachBinaryHeapNode<Access>(state.root, visitor);
    }
    template<typename Destroy>
    static void destroy(State& state, Destroy destroy) {
        dsa::core::destroyBinaryHeapTree<Access>(state.root, destroy);
        state.root = NULL;
    }
    static void swapState(State& first, State& second) {
        using std::swap;
        swap(first.root, second.root);
    }
};

// 配对堆结构策略：删除堆顶时执行标准两遍配对合并。
template<typename Access, typename Higher>
struct PairingPolicy {
    typedef typename Access::node_type node_type;
    struct State {
        State() : root(NULL) {}
        node_type* root;
    };

    static bool empty(const State& state) { return state.root == NULL; }
    static node_type* best(const State& state) { return state.root; }
    static void insert(State& state, node_type* node, const Higher& higher) {
        state.root = dsa::core::PairingHeapAlgorithm<Access, Higher>::merge(
            state.root, node, higher
        );
    }
    static node_type* detachBest(State& state, std::size_t, const Higher& higher) {
        node_type* removed = state.root;
        node_type* merged = dsa::core::PairingHeapAlgorithm<Access, Higher>::mergePairs(
            Access::child(removed), higher
        );
        Access::child(removed) = NULL;
        Access::sibling(removed) = NULL;
        Access::parent(removed) = NULL;
        state.root = merged;
        return removed;
    }
    static void meld(State& first, State& second, const Higher& higher) {
        node_type* merged = dsa::core::PairingHeapAlgorithm<Access, Higher>::merge(
            first.root, second.root, higher
        );
        first.root = merged;
        second.root = NULL;
    }
    template<typename Visitor>
    static void forEach(State& state, Visitor visitor) {
        dsa::core::forEachChildSiblingHeapNode<Access>(state.root, visitor);
    }
    template<typename Destroy>
    static void destroy(State& state, Destroy destroy) {
        dsa::core::destroyChildSiblingHeapForest<Access>(state.root, destroy);
        state.root = NULL;
    }
    static void swapState(State& first, State& second) {
        using std::swap;
        swap(first.root, second.root);
    }
};

// 斐波那契堆结构策略：维护根表头尾和最优根，删除时延迟 consolidate。
template<typename Access, typename Higher>
struct FibonacciPolicy {
    typedef typename Access::node_type node_type;
    struct State {
        State() : head(NULL), tail(NULL), best(NULL) {}
        node_type* head;
        node_type* tail;
        node_type* best;
    };

    static bool empty(const State& state) { return state.best == NULL; }
    static node_type* best(const State& state) { return state.best; }
    static void insert(State& state, node_type* node, const Higher& higher) {
        const bool becomesBest = !state.best || higher(Access::value(node), Access::value(state.best));
        Access::parent(node) = NULL;
        Access::sibling(node) = NULL;
        Access::setMarked(node, false);
        if (!state.head)
            state.head = node;
        else
            Access::sibling(state.tail) = node;
        state.tail = node;
        if (becomesBest)
            state.best = node;
    }
    static node_type* detachBest(State& state, std::size_t size, const Higher& higher) {
        return dsa::core::FibonacciHeapAlgorithm<Access, Higher>::removeBest(
            state.head, state.tail, state.best, size, higher
        );
    }
    static void meld(State& first, State& second, const Higher& higher) {
        const bool secondWins = !first.best || higher(
            Access::value(second.best), Access::value(first.best)
        );
        if (!first.head) {
            first.head = second.head;
            first.tail = second.tail;
        } else {
            Access::sibling(first.tail) = second.head;
            first.tail = second.tail;
        }
        if (secondWins)
            first.best = second.best;
        second.head = NULL;
        second.tail = NULL;
        second.best = NULL;
    }
    template<typename Visitor>
    static void forEach(State& state, Visitor visitor) {
        dsa::core::forEachChildSiblingHeapNode<Access>(state.head, visitor);
    }
    template<typename Destroy>
    static void destroy(State& state, Destroy destroy) {
        dsa::core::destroyChildSiblingHeapForest<Access>(state.head, destroy);
        state.head = NULL;
        state.tail = NULL;
        state.best = NULL;
    }
    static void swapState(State& first, State& second) {
        using std::swap;
        swap(first.head, second.head);
        swap(first.tail, second.tail);
        swap(first.best, second.best);
    }
};

} // namespace detail
} // namespace container
} // namespace dsa

#endif
