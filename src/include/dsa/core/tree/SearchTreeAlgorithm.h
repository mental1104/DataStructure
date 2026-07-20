#ifndef DSA_CORE_TREE_SEARCH_TREE_ALGORITHM_H
#define DSA_CORE_TREE_SEARCH_TREE_ALGORITHM_H

#include <algorithm>
#include <cstddef>

namespace dsa {
namespace core {

// 二叉搜索树共享结构算法：只依赖 Access 暴露的父子链接和值语义。
// 节点申请、析构、size 提交、平衡元数据与异常回滚均由容器负责。
template<typename Access>
class SearchTreeAlgorithm {
public:
    typedef typename Access::node_type node_type;

    struct EraseResult {
        node_type* removed;
        node_type* fix_node;
        node_type* fix_parent;
        node_type* rebalance_from;
        node_type* moved_node;

        EraseResult()
            : removed(nullptr), fix_node(nullptr), fix_parent(nullptr),
              rebalance_from(nullptr), moved_node(nullptr) {}
    };

    // 使用比较器判断两个键是否等价，避免要求 value_type 提供 operator==。
    template<typename L, typename R, typename Compare>
    static bool equivalent(const L& left, const R& right, const Compare& compare) {
        return !compare(left, right) && !compare(right, left);
    }

    // 返回命中节点；未命中返回 nullptr。复杂度为 O(height)。
    template<typename NodePointer, typename Value, typename Compare>
    static NodePointer find(NodePointer root, const Value& value, const Compare& compare) {
        while (root) {
            if (compare(value, Access::value(root))) {
                root = Access::left(root);
            } else if (compare(Access::value(root), value)) {
                root = Access::right(root);
            } else {
                return root;
            }
        }
        return NodePointer();
    }

    // 返回命中节点链接或待插入空链接，并通过 hot 返回最后访问节点。
    template<typename Value, typename Compare>
    static node_type*& searchSlot(
        node_type*& root,
        node_type*& hot,
        const Value& value,
        const Compare& compare
    ) {
        node_type** slot = &root;
        hot = nullptr;
        while (*slot) {
            node_type* current = *slot;
            if (compare(value, Access::value(current))) {
                hot = current;
                slot = &Access::leftRef(current);
            } else if (compare(Access::value(current), value)) {
                hot = current;
                slot = &Access::rightRef(current);
            } else {
                return *slot;
            }
        }
        return *slot;
    }

    template<typename NodePointer>
    static NodePointer minimum(NodePointer node) {
        while (node && Access::left(node))
            node = Access::left(node);
        return node;
    }

    template<typename NodePointer>
    static NodePointer maximum(NodePointer node) {
        while (node && Access::right(node))
            node = Access::right(node);
        return node;
    }

    template<typename NodePointer>
    static NodePointer successor(NodePointer node) {
        if (!node)
            return NodePointer();
        if (Access::right(node))
            return minimum(Access::right(node));
        NodePointer parent = Access::parent(node);
        while (parent && node == Access::right(parent)) {
            node = parent;
            parent = Access::parent(parent);
        }
        return parent;
    }

    template<typename NodePointer>
    static NodePointer predecessor(NodePointer node) {
        if (!node)
            return NodePointer();
        if (Access::left(node))
            return maximum(Access::left(node));
        NodePointer parent = Access::parent(node);
        while (parent && node == Access::left(parent)) {
            node = parent;
            parent = Access::parent(parent);
        }
        return parent;
    }

    template<typename NodePointer, typename Value, typename Compare>
    static NodePointer lowerBound(NodePointer root, const Value& value, const Compare& compare) {
        NodePointer candidate = NodePointer();
        while (root) {
            if (!compare(Access::value(root), value)) {
                candidate = root;
                root = Access::left(root);
            } else {
                root = Access::right(root);
            }
        }
        return candidate;
    }

    template<typename NodePointer, typename Value, typename Compare>
    static NodePointer upperBound(NodePointer root, const Value& value, const Compare& compare) {
        NodePointer candidate = NodePointer();
        while (root) {
            if (compare(value, Access::value(root))) {
                candidate = root;
                root = Access::left(root);
            } else {
                root = Access::right(root);
            }
        }
        return candidate;
    }

    // 将 parent 的左孩子替换为 child，并同步 child 的 parent 链接。
    static void setLeft(node_type* parent, node_type* child) {
        Access::leftRef(parent) = child;
        if (child)
            Access::parentRef(child) = parent;
    }

    // 将 parent 的右孩子替换为 child，并同步 child 的 parent 链接。
    static void setRight(node_type* parent, node_type* child) {
        Access::rightRef(parent) = child;
        if (child)
            Access::parentRef(child) = parent;
    }

    // 用 replacement 替换 current 在父节点或根链接中的位置。
    static void replace(node_type*& root, node_type* current, node_type* replacement) {
        node_type* parent = Access::parent(current);
        if (!parent) {
            root = replacement;
        } else if (Access::left(parent) == current) {
            Access::leftRef(parent) = replacement;
        } else {
            Access::rightRef(parent) = replacement;
        }
        if (replacement)
            Access::parentRef(replacement) = parent;
    }

    // 围绕 pivot 左旋，返回旋转后的子树根。节点所有权不变。
    static node_type* rotateLeft(node_type*& root, node_type* pivot) {
        node_type* child = Access::right(pivot);
        if (!child)
            return pivot;

        node_type* middle = Access::left(child);
        replace(root, pivot, child);
        setRight(pivot, middle);
        setLeft(child, pivot);
        return child;
    }

    // 围绕 pivot 右旋，返回旋转后的子树根。节点所有权不变。
    static node_type* rotateRight(node_type*& root, node_type* pivot) {
        node_type* child = Access::left(pivot);
        if (!child)
            return pivot;

        node_type* middle = Access::right(child);
        replace(root, pivot, child);
        setLeft(pivot, middle);
        setRight(child, pivot);
        return child;
    }

    // 对 v、parent(v)、grandparent(v) 执行 AVL/RB 共用的四类三节点重构。
    static node_type* restructure(node_type*& root, node_type* value_node) {
        if (!value_node || !Access::parent(value_node) ||
            !Access::parent(Access::parent(value_node)))
            return value_node;

        node_type* parent = Access::parent(value_node);
        node_type* grand = Access::parent(parent);
        if (parent == Access::left(grand)) {
            if (value_node == Access::left(parent)) {
                return rotateRight(root, grand);
            }
            rotateLeft(root, parent);
            return rotateRight(root, grand);
        }

        if (value_node == Access::right(parent)) {
            return rotateLeft(root, grand);
        }
        rotateRight(root, parent);
        return rotateLeft(root, grand);
    }

    // 从已知父链接 slot 物理摘除 target。slot 必须正好引用 target 所在的根/孩子链接。
    // 该版本适合保留“search 返回 Node*&”契约的教学实现。
    static EraseResult detachFromSlot(node_type*& slot, node_type* target) {
        EraseResult result;
        result.removed = target;
        if (!target)
            return result;

        if (!Access::left(target)) {
            result.fix_node = Access::right(target);
            result.fix_parent = Access::parent(target);
            result.rebalance_from = Access::parent(target);
            slot = result.fix_node;
            if (result.fix_node)
                Access::parentRef(result.fix_node) = result.fix_parent;
        } else if (!Access::right(target)) {
            result.fix_node = Access::left(target);
            result.fix_parent = Access::parent(target);
            result.rebalance_from = Access::parent(target);
            slot = result.fix_node;
            Access::parentRef(result.fix_node) = result.fix_parent;
        } else {
            node_type* successor_node = minimum(Access::right(target));
            result.moved_node = successor_node;
            result.fix_node = Access::right(successor_node);

            if (Access::parent(successor_node) != target) {
                node_type* old_parent = Access::parent(successor_node);
                result.fix_parent = old_parent;
                result.rebalance_from = old_parent;
                Access::leftRef(old_parent) = result.fix_node;
                if (result.fix_node)
                    Access::parentRef(result.fix_node) = old_parent;
                setRight(successor_node, Access::right(target));
            } else {
                result.fix_parent = successor_node;
                result.rebalance_from = successor_node;
                if (result.fix_node)
                    Access::parentRef(result.fix_node) = successor_node;
            }

            slot = successor_node;
            Access::parentRef(successor_node) = Access::parent(target);
            setLeft(successor_node, Access::left(target));
        }

        Access::parentRef(target) = nullptr;
        Access::leftRef(target) = nullptr;
        Access::rightRef(target) = nullptr;
        return result;
    }

    // 物理摘除 target，不交换或赋值键值，因而支持不可赋值键和稳定节点身份。
    // 调用方必须在成功提交 size 后析构 result.removed。
    static EraseResult detach(node_type*& root, node_type* target) {
        EraseResult result;
        result.removed = target;
        if (!target)
            return result;

        if (!Access::left(target)) {
            result.fix_node = Access::right(target);
            result.fix_parent = Access::parent(target);
            result.rebalance_from = Access::parent(target);
            replace(root, target, Access::right(target));
        } else if (!Access::right(target)) {
            result.fix_node = Access::left(target);
            result.fix_parent = Access::parent(target);
            result.rebalance_from = Access::parent(target);
            replace(root, target, Access::left(target));
        } else {
            node_type* successor_node = minimum(Access::right(target));
            result.moved_node = successor_node;
            result.fix_node = Access::right(successor_node);

            if (Access::parent(successor_node) != target) {
                node_type* old_parent = Access::parent(successor_node);
                result.fix_parent = old_parent;
                result.rebalance_from = old_parent;
                replace(root, successor_node, Access::right(successor_node));
                setRight(successor_node, Access::right(target));
            } else {
                result.fix_parent = successor_node;
                result.rebalance_from = successor_node;
                if (result.fix_node)
                    Access::parentRef(result.fix_node) = successor_node;
            }

            replace(root, target, successor_node);
            setLeft(successor_node, Access::left(target));
        }

        Access::parentRef(target) = nullptr;
        Access::leftRef(target) = nullptr;
        Access::rightRef(target) = nullptr;
        return result;
    }

    static int nodeHeight(const node_type* node) {
        return node ? Access::height(node) : -1;
    }

    // 按结构高度刷新节点；红黑树的颜色不参与该字段含义。
    static int updateHeight(node_type* node) {
        if (!node)
            return -1;
        Access::heightRef(node) = 1 + std::max(
            nodeHeight(Access::left(node)), nodeHeight(Access::right(node))
        );
        return Access::height(node);
    }

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
