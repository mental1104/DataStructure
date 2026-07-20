#ifndef DSA_CORE_TREE_BIN_TREE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_TREE_ALGORITHM_H

#include <algorithm>
#include <cstddef>
#include <deque>
#include <vector>

namespace dsa {
namespace core {

// 二叉树共享算法层：只通过 Access 读取节点语义，不拥有节点，也不决定 allocator 或容器状态提交。
//
// Access contract:
//   typedef node_type
//   static node_type* left(node_type*)
//   static const node_type* left(const node_type*)
//   static node_type* right(node_type*)
//   static const node_type* right(const node_type*)
//   static node_type* parent(node_type*)
//   static const node_type* parent(const node_type*)
//   static value_type& value(node_type*)
//   static const value_type& value(const node_type*)
//   static int& height(node_type*)
//   static int height(const node_type*)
template<typename Access>
class BinTreeAlgorithm {
public:
    typedef typename Access::node_type node_type;

    // 迭代统计子树节点数，避免深树递归占用调用栈。
    template<typename NodePointer>
    static std::size_t subtreeSize(NodePointer root) {
        if (!root)
            return 0;

        std::size_t count = 0;
        std::vector<NodePointer> stack;
        stack.push_back(root);
        while (!stack.empty()) {
            NodePointer node = stack.back();
            stack.pop_back();
            ++count;

            NodePointer left = Access::left(node);
            NodePointer right = Access::right(node);
            if (right)
                stack.push_back(right);
            if (left)
                stack.push_back(left);
        }
        return count;
    }

    // 返回子树最左节点；空树返回空指针。
    template<typename NodePointer>
    static NodePointer leftmost(NodePointer root) {
        while (root && Access::left(root))
            root = Access::left(root);
        return root;
    }

    // 按中序语义寻找后继，不修改树结构，复杂度为 O(height)。
    template<typename NodePointer>
    static NodePointer successor(NodePointer node) {
        if (!node)
            return NodePointer();

        if (Access::right(node))
            return leftmost(Access::right(node));

        NodePointer parent = Access::parent(node);
        while (parent && node == Access::right(parent)) {
            node = parent;
            parent = Access::parent(parent);
        }
        return parent;
    }

    // 以前序顺序迭代访问节点，额外空间为 O(height) 到 O(n)。
    template<typename NodePointer, typename Visitor>
    static void forEachNodePreOrder(NodePointer root, Visitor&& visitor) {
        if (!root)
            return;

        std::vector<NodePointer> stack;
        stack.push_back(root);
        while (!stack.empty()) {
            NodePointer node = stack.back();
            stack.pop_back();
            visitor(node);

            NodePointer right = Access::right(node);
            NodePointer left = Access::left(node);
            if (right)
                stack.push_back(right);
            if (left)
                stack.push_back(left);
        }
    }

    // 以中序顺序迭代访问节点，额外空间为 O(height)。
    template<typename NodePointer, typename Visitor>
    static void forEachNodeInOrder(NodePointer root, Visitor&& visitor) {
        std::vector<NodePointer> stack;
        NodePointer current = root;

        while (current || !stack.empty()) {
            while (current) {
                stack.push_back(current);
                current = Access::left(current);
            }

            current = stack.back();
            stack.pop_back();
            visitor(current);
            current = Access::right(current);
        }
    }

    // 以后序顺序迭代访问节点；节点访问发生在两个孩子之后，可安全用于子树销毁。
    template<typename NodePointer, typename Visitor>
    static void forEachNodePostOrder(NodePointer root, Visitor&& visitor) {
        std::vector<NodePointer> stack;
        NodePointer current = root;
        NodePointer lastVisited = NodePointer();

        while (current || !stack.empty()) {
            if (current) {
                stack.push_back(current);
                current = Access::left(current);
                continue;
            }

            NodePointer node = stack.back();
            NodePointer right = Access::right(node);
            if (right && lastVisited != right) {
                current = right;
                continue;
            }

            stack.pop_back();
            visitor(node);
            lastVisited = node;
        }
    }

    // 按层序迭代访问节点；队列最多保存一层节点。
    template<typename NodePointer, typename Visitor>
    static void forEachNodeLevelOrder(NodePointer root, Visitor&& visitor) {
        if (!root)
            return;

        std::deque<NodePointer> queue;
        queue.push_back(root);
        while (!queue.empty()) {
            NodePointer node = queue.front();
            queue.pop_front();
            visitor(node);

            NodePointer left = Access::left(node);
            NodePointer right = Access::right(node);
            if (left)
                queue.push_back(left);
            if (right)
                queue.push_back(right);
        }
    }

    // 以前序顺序访问节点值；Visitor 接收可变或只读值引用取决于根指针类型。
    template<typename NodePointer, typename Visitor>
    static void traversePreOrder(NodePointer root, Visitor&& visitor) {
        forEachNodePreOrder(root, [&](NodePointer node) {
            visitor(Access::value(node));
        });
    }

    // 以中序顺序访问节点值；算法不依赖具体容器。
    template<typename NodePointer, typename Visitor>
    static void traverseInOrder(NodePointer root, Visitor&& visitor) {
        forEachNodeInOrder(root, [&](NodePointer node) {
            visitor(Access::value(node));
        });
    }

    // 以后序顺序访问节点值；实现为单栈迭代遍历。
    template<typename NodePointer, typename Visitor>
    static void traversePostOrder(NodePointer root, Visitor&& visitor) {
        forEachNodePostOrder(root, [&](NodePointer node) {
            visitor(Access::value(node));
        });
    }

    // 按层序访问节点值；空树不调用 Visitor。
    template<typename NodePointer, typename Visitor>
    static void traverseLevelOrder(NodePointer root, Visitor&& visitor) {
        forEachNodeLevelOrder(root, [&](NodePointer node) {
            visitor(Access::value(node));
        });
    }

    // 利用 parent 链以 O(1) 额外空间后序销毁子树；Destroy 负责实际对象析构和内存释放。
    template<typename Destroy>
    static std::size_t destroySubtree(node_type* root, Destroy&& destroy) {
        if (!root)
            return 0;

        node_type* const boundary = Access::parent(root);
        node_type* previous = boundary;
        node_type* current = root;
        std::size_t count = 0;

        while (current && current != boundary) {
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
                ++count;
                continue;
            }

            previous = current;
            current = next;
        }
        return count;
    }

    // 对有序二叉树执行区间聚合，保持中序聚合顺序并剪枝区间外子树。
    template<typename NodePointer, typename Value, typename Result, typename Aggregate>
    static Result rangeAggregateOrdered(
        NodePointer root,
        const Value& lower,
        const Value& upper,
        Result identity,
        Aggregate&& aggregate
    ) {
        std::vector<NodePointer> stack;
        NodePointer current = root;
        Result result = identity;

        while (current || !stack.empty()) {
            while (current) {
                if (Access::value(current) < lower) {
                    current = Access::right(current);
                } else {
                    stack.push_back(current);
                    current = Access::left(current);
                }
            }

            if (stack.empty())
                break;

            NodePointer node = stack.back();
            stack.pop_back();
            if (upper < Access::value(node)) {
                current = NodePointer();
                continue;
            }

            result = aggregate(result, Access::value(node));
            current = Access::right(node);
        }
        return result;
    }

    // 根据左右孩子高度更新当前节点高度，并返回更新值。
    static int updateHeight(node_type* node) {
        const int leftHeight = Access::left(node) ? Access::height(Access::left(node)) : -1;
        const int rightHeight = Access::right(node) ? Access::height(Access::right(node)) : -1;
        Access::height(node) = 1 + std::max(leftHeight, rightHeight);
        return Access::height(node);
    }

    // 从指定节点向根方向更新高度，复杂度为 O(height)。
    static void updateHeightAbove(node_type* node) {
        while (node) {
            updateHeight(node);
            node = Access::parent(node);
        }
    }
};

} // namespace core
} // namespace dsa

#endif
