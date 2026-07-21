#ifndef DSA_CORE_HEAP_HEAP_ALGORITHM_H
#define DSA_CORE_HEAP_HEAP_ALGORITHM_H

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace dsa {
namespace core {

// 完全二叉堆共享算法：只依赖随机访问语义和位置交换，不管理容器容量与对象生命周期。
template<typename Access, typename Higher>
class BinaryHeapAlgorithm {
public:
    typedef typename Access::size_type size_type;

    // 返回节点 i 的父节点下标；调用方保证 i > 0。
    static size_type parent(size_type index) {
        return (index - 1) / 2;
    }

    // 返回节点 i 的左孩子下标。
    static size_type leftChild(size_type index) {
        return index * 2 + 1;
    }

    // 返回节点 i 的右孩子下标。
    static size_type rightChild(size_type index) {
        return index * 2 + 2;
    }

    // 将指定位置元素向上移动到满足堆序的位置，返回最终下标。
    static size_type siftUp(Access& access, size_type index, const Higher& higher) {
        while (index > 0) {
            const size_type parentIndex = parent(index);
            if (!higher(access.value(index), access.value(parentIndex)))
                break;
            access.swapAt(index, parentIndex);
            index = parentIndex;
        }
        return index;
    }

    // 将指定位置元素向下移动到满足堆序的位置，返回最终下标。
    static size_type siftDown(
        Access& access,
        size_type count,
        size_type index,
        const Higher& higher
    ) {
        while (leftChild(index) < count) {
            size_type preferred = leftChild(index);
            const size_type right = rightChild(index);
            if (right < count && higher(access.value(right), access.value(preferred)))
                preferred = right;
            if (!higher(access.value(preferred), access.value(index)))
                break;
            access.swapAt(index, preferred);
            index = preferred;
        }
        return index;
    }

    // 使用 Floyd 自底向上建堆，时间复杂度为 O(n)。
    static void heapify(Access& access, size_type count, const Higher& higher) {
        if (count < 2)
            return;
        for (size_type parentCount = count / 2; parentCount > 0; --parentCount)
            siftDown(access, count, parentCount - 1, higher);
    }

    // 验证随机访问区间是否满足堆序，主要供测试与调试使用。
    static bool isHeap(const Access& access, size_type count, const Higher& higher) {
        for (size_type index = 1; index < count; ++index) {
            if (higher(access.value(index), access.value(parent(index))))
                return false;
        }
        return true;
    }
};

// 左式堆共享算法：Access 负责节点字段，算法只编排右脊合并、左右交换与 npl 更新。
template<typename Access, typename Higher>
class LeftistHeapAlgorithm {
public:
    typedef typename Access::node_type node_type;

    // 合并两棵左式堆；比较和临时路径申请完成前不修改输入结构。
    static node_type* merge(node_type* first, node_type* second, const Higher& higher) {
        if (!first) {
            if (second)
                Access::parent(second) = nullptr;
            return second;
        }
        if (!second) {
            Access::parent(first) = nullptr;
            return first;
        }

        std::vector<node_type*> path;
        while (first && second) {
            if (higher(Access::value(second), Access::value(first)))
                std::swap(first, second);
            path.push_back(first);
            first = Access::right(first);
        }

        node_type* merged = first ? first : second;
        while (!path.empty()) {
            node_type* root = path.back();
            path.pop_back();

            Access::right(root) = merged;
            if (merged)
                Access::parent(merged) = root;

            if (!Access::left(root) ||
                Access::nplValue(Access::left(root)) < Access::nplValue(Access::right(root))) {
                std::swap(Access::left(root), Access::right(root));
            }

            if (Access::left(root))
                Access::parent(Access::left(root)) = root;
            if (Access::right(root))
                Access::parent(Access::right(root)) = root;
            Access::nplRef(root) = Access::right(root) ? Access::nplValue(Access::right(root)) + 1 : 1;
            merged = root;
        }

        if (merged)
            Access::parent(merged) = nullptr;
        return merged;
    }
};

// 斜堆共享算法：沿右脊收集路径后一次性回接，避免递归深度随退化结构增长。
template<typename Access, typename Higher>
class SkewHeapAlgorithm {
public:
    typedef typename Access::node_type node_type;

    // 合并两棵斜堆；成功后每个路径节点都交换左右子树。
    static node_type* merge(node_type* first, node_type* second, const Higher& higher) {
        if (!first) {
            if (second)
                Access::parent(second) = nullptr;
            return second;
        }
        if (!second) {
            Access::parent(first) = nullptr;
            return first;
        }

        std::vector<node_type*> path;
        while (first && second) {
            if (higher(Access::value(second), Access::value(first)))
                std::swap(first, second);
            path.push_back(first);
            first = Access::right(first);
        }

        node_type* merged = first ? first : second;
        while (!path.empty()) {
            node_type* root = path.back();
            path.pop_back();

            Access::right(root) = merged;
            std::swap(Access::left(root), Access::right(root));
            if (Access::left(root))
                Access::parent(Access::left(root)) = root;
            if (Access::right(root))
                Access::parent(Access::right(root)) = root;
            merged = root;
        }

        if (merged)
            Access::parent(merged) = nullptr;
        return merged;
    }
};

// 配对堆共享算法：节点使用 child/sibling 语义，所有比较完成后再提交链接变化。
template<typename Access, typename Higher>
class PairingHeapAlgorithm {
public:
    typedef typename Access::node_type node_type;

private:
    struct LinkPlan {
        node_type* winner;
        node_type* loser;
    };

    // 按已确定的优先关系把 loser 挂为 winner 的最左孩子。
    static void linkKnown(node_type* winner, node_type* loser) {
        Access::parent(winner) = nullptr;
        Access::sibling(winner) = nullptr;
        Access::parent(loser) = winner;
        Access::sibling(loser) = Access::child(winner);
        if (Access::child(winner))
            Access::parent(Access::child(winner)) = winner;
        Access::child(winner) = loser;
    }

public:
    // O(1) 合并两个配对堆根；比较器抛异常时输入保持不变。
    static node_type* merge(node_type* first, node_type* second, const Higher& higher) {
        if (!first) {
            if (second) {
                Access::parent(second) = nullptr;
                Access::sibling(second) = nullptr;
            }
            return second;
        }
        if (!second) {
            Access::parent(first) = nullptr;
            Access::sibling(first) = nullptr;
            return first;
        }

        node_type* winner = first;
        node_type* loser = second;
        if (higher(Access::value(second), Access::value(first))) {
            winner = second;
            loser = first;
        }
        linkKnown(winner, loser);
        return winner;
    }

    // 对兄弟链执行“两两合并，再从右向左合并”的标准配对流程。
    static node_type* mergePairs(node_type* firstSibling, const Higher& higher) {
        if (!firstSibling)
            return nullptr;

        std::vector<node_type*> roots;
        for (node_type* node = firstSibling; node; node = Access::sibling(node))
            roots.push_back(node);
        if (roots.size() == 1) {
            Access::parent(roots[0]) = nullptr;
            Access::sibling(roots[0]) = nullptr;
            return roots[0];
        }

        std::vector<node_type*> paired;
        paired.reserve((roots.size() + 1) / 2);
        std::vector<LinkPlan> plans;
        plans.reserve(roots.size() - 1);

        std::size_t index = 0;
        for (; index + 1 < roots.size(); index += 2) {
            node_type* first = roots[index];
            node_type* second = roots[index + 1];
            const bool secondWins = higher(Access::value(second), Access::value(first));
            node_type* winner = secondWins ? second : first;
            node_type* loser = secondWins ? first : second;
            plans.push_back(LinkPlan{winner, loser});
            paired.push_back(winner);
        }
        if (index < roots.size())
            paired.push_back(roots[index]);

        node_type* result = paired.back();
        for (std::size_t remaining = paired.size() - 1; remaining > 0; --remaining) {
            node_type* leftRoot = paired[remaining - 1];
            const bool resultWins = higher(Access::value(result), Access::value(leftRoot));
            node_type* winner = resultWins ? result : leftRoot;
            node_type* loser = resultWins ? leftRoot : result;
            plans.push_back(LinkPlan{winner, loser});
            result = winner;
        }

        for (std::size_t i = 0; i < roots.size(); ++i) {
            Access::parent(roots[i]) = nullptr;
            Access::sibling(roots[i]) = nullptr;
        }
        for (std::size_t i = 0; i < plans.size(); ++i)
            linkKnown(plans[i].winner, plans[i].loser);

        Access::parent(result) = nullptr;
        Access::sibling(result) = nullptr;
        return result;
    }
};

// 斐波那契堆共享算法：根表和孩子表使用单向 sibling 链，度数合并在提交前完成规划。
template<typename Access, typename Higher>
class FibonacciHeapAlgorithm {
public:
    typedef typename Access::node_type node_type;
    typedef typename Access::size_type size_type;

private:
    struct PlannedTree {
        node_type* root;
        size_type degree;

        PlannedTree() : root(nullptr), degree(0) {}
        PlannedTree(node_type* value, size_type nodeDegree) : root(value), degree(nodeDegree) {}
    };

    struct LinkPlan {
        node_type* winner;
        node_type* loser;
    };

    // 按规划结果把 loser 链接为 winner 的孩子，并更新度数与标记。
    static void linkKnown(node_type* winner, node_type* loser) {
        Access::parent(loser) = winner;
        Access::setMarked(loser, false);
        Access::sibling(loser) = Access::child(winner);
        if (Access::child(winner))
            Access::parent(Access::child(winner)) = winner;
        Access::child(winner) = loser;
        Access::incrementDegree(winner);
    }

    // 对候选根集合规划度数合并；比较和内存申请失败时尚未修改节点。
    static node_type* consolidateCandidates(
        const std::vector<node_type*>& candidates,
        node_type*& head,
        node_type*& tail,
        const Higher& higher
    ) {
        if (candidates.empty()) {
            head = nullptr;
            tail = nullptr;
            return nullptr;
        }

        const std::size_t initialBound =
            static_cast<std::size_t>(std::numeric_limits<size_type>::digits) * 2U + 4U;
        std::vector<PlannedTree> table(initialBound);
        std::vector<LinkPlan> plans;
        plans.reserve(candidates.size());

        for (std::size_t i = 0; i < candidates.size(); ++i) {
            PlannedTree current(candidates[i], Access::degreeValue(candidates[i]));
            while (true) {
                if (current.degree >= table.size())
                    table.resize(static_cast<std::size_t>(current.degree) + 2U);
                PlannedTree& slot = table[static_cast<std::size_t>(current.degree)];
                if (!slot.root) {
                    slot = current;
                    break;
                }

                PlannedTree other = slot;
                slot = PlannedTree();
                const bool currentWins = higher(
                    Access::value(current.root),
                    Access::value(other.root)
                );
                node_type* winner = currentWins ? current.root : other.root;
                node_type* loser = currentWins ? other.root : current.root;
                plans.push_back(LinkPlan{winner, loser});
                current = PlannedTree(winner, current.degree + 1);
            }
        }

        std::vector<node_type*> finalRoots;
        finalRoots.reserve(candidates.size());
        node_type* best = nullptr;
        for (std::size_t i = 0; i < table.size(); ++i) {
            if (!table[i].root)
                continue;
            finalRoots.push_back(table[i].root);
            if (!best || higher(Access::value(table[i].root), Access::value(best)))
                best = table[i].root;
        }

        for (std::size_t i = 0; i < candidates.size(); ++i) {
            Access::parent(candidates[i]) = nullptr;
            Access::sibling(candidates[i]) = nullptr;
            Access::setMarked(candidates[i], false);
        }
        for (std::size_t i = 0; i < plans.size(); ++i)
            linkKnown(plans[i].winner, plans[i].loser);

        head = nullptr;
        tail = nullptr;
        for (std::size_t i = 0; i < finalRoots.size(); ++i) {
            node_type* root = finalRoots[i];
            Access::parent(root) = nullptr;
            Access::setMarked(root, false);
            Access::sibling(root) = nullptr;
            if (!head)
                head = root;
            else
                Access::sibling(tail) = root;
            tail = root;
        }
        return best;
    }

public:
    // 将度数相同的两棵树链接为一棵；调用方保证二者都是独立根。
    static node_type* linkTrees(node_type* first, node_type* second, const Higher& higher) {
        if (!first)
            return second;
        if (!second)
            return first;
        node_type* winner = first;
        node_type* loser = second;
        if (higher(Access::value(second), Access::value(first))) {
            winner = second;
            loser = first;
        }
        linkKnown(winner, loser);
        Access::parent(winner) = nullptr;
        Access::sibling(winner) = nullptr;
        return winner;
    }

    // 整理当前根表，使每个度数至多保留一棵树，并刷新最优根与尾指针。
    static void consolidate(
        node_type*& head,
        node_type*& tail,
        node_type*& best,
        const Higher& higher
    ) {
        std::vector<node_type*> candidates;
        for (node_type* root = head; root; root = Access::sibling(root))
            candidates.push_back(root);
        best = consolidateCandidates(candidates, head, tail, higher);
    }

    // 删除最优根前先规划完整 consolidate，成功后一次性提交新根表并返回被摘除节点。
    static node_type* removeBest(
        node_type*& head,
        node_type*& tail,
        node_type*& best,
        size_type size,
        const Higher& higher
    ) {
        if (!best)
            return nullptr;

        node_type* removed = best;
        std::vector<node_type*> candidates;
        if (size > 0)
            candidates.reserve(static_cast<std::size_t>(size - 1));

        for (node_type* root = head; root; root = Access::sibling(root)) {
            if (root != removed)
                candidates.push_back(root);
        }
        for (node_type* child = Access::child(removed); child; child = Access::sibling(child))
            candidates.push_back(child);

        node_type* newHead = nullptr;
        node_type* newTail = nullptr;
        node_type* newBest = consolidateCandidates(candidates, newHead, newTail, higher);

        Access::child(removed) = nullptr;
        Access::sibling(removed) = nullptr;
        Access::parent(removed) = nullptr;
        Access::resetDegree(removed);
        Access::setMarked(removed, false);
        head = newHead;
        tail = newTail;
        best = newBest;
        return removed;
    }
};

// 以前序方式遍历普通二叉节点，仅供复制、验证等非析构路径使用。
template<typename Access, typename Visitor>
void forEachBinaryHeapNode(typename Access::node_type* root, Visitor visitor) {
    typedef typename Access::node_type node_type;
    if (!root)
        return;
    std::vector<node_type*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        node_type* node = stack.back();
        stack.pop_back();
        visitor(node);
        if (Access::right(node))
            stack.push_back(Access::right(node));
        if (Access::left(node))
            stack.push_back(Access::left(node));
    }
}

// 依赖 parent 链以 O(1) 额外空间销毁普通二叉堆节点，Destroy 负责实际释放。
template<typename Access, typename Destroy>
void destroyBinaryHeapTree(typename Access::node_type* root, Destroy destroy) {
    typedef typename Access::node_type node_type;
    node_type* previous = nullptr;
    node_type* current = root;
    while (current) {
        node_type* next = nullptr;
        const bool descending = previous == Access::parent(current);
        const bool returningFromLeft = previous == Access::left(current);
        if (descending && Access::left(current)) {
            next = Access::left(current);
        } else if ((descending || returningFromLeft) && Access::right(current)) {
            next = Access::right(current);
        } else {
            next = Access::parent(current);
            node_type* completed = current;
            previous = completed;
            current = next;
            destroy(completed);
            continue;
        }
        previous = current;
        current = next;
    }
}

// 遍历 child/sibling 森林中的全部节点，仅供复制与验证等允许分配临时空间的路径使用。
template<typename Access, typename Visitor>
void forEachChildSiblingHeapNode(typename Access::node_type* head, Visitor visitor) {
    typedef typename Access::node_type node_type;
    if (!head)
        return;
    std::vector<node_type*> stack;
    stack.push_back(head);
    while (!stack.empty()) {
        node_type* node = stack.back();
        stack.pop_back();
        visitor(node);
        if (Access::sibling(node))
            stack.push_back(Access::sibling(node));
        if (Access::child(node))
            stack.push_back(Access::child(node));
    }
}

// 不申请临时内存地销毁 child/sibling 森林，适用于配对堆和斐波那契堆析构。
template<typename Access, typename Destroy>
void destroyChildSiblingHeapForest(typename Access::node_type* head, Destroy destroy) {
    typedef typename Access::node_type node_type;
    node_type* current = head;
    while (current) {
        if (Access::child(current)) {
            node_type* child = Access::child(current);
            Access::child(current) = nullptr;
            current = child;
            continue;
        }

        node_type* sibling = Access::sibling(current);
        node_type* parent = Access::parent(current);
        node_type* completed = current;
        current = sibling ? sibling : parent;
        destroy(completed);
    }
}

} // namespace core
} // namespace dsa

#endif
