#ifndef DSA_CORE_TREE_SEARCH_TREE_ALGORITHM_H
#define DSA_CORE_TREE_SEARCH_TREE_ALGORITHM_H

#include <algorithm>
#include <cstddef>

namespace dsa {
namespace core {

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

    template<typename L, typename R, typename Compare>
    static bool equivalent(const L& left, const R& right, const Compare& compare) {
        return !compare(left, right) && !compare(right, left);
    }

    template<typename NodePointer, typename Value, typename Compare>
    static NodePointer find(NodePointer root, const Value& value, const Compare& compare) {
        while (root) {
            if (compare(value, Access::value(root)))
                root = Access::left(root);
            else if (compare(Access::value(root), value))
                root = Access::right(root);
            else
                return root;
        }
        return NodePointer();
    }

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

    static void setLeft(node_type* parent, node_type* child) {
        Access::leftRef(parent) = child;
        if (child)
            Access::parentRef(child) = parent;
    }

    static void setRight(node_type* parent, node_type* child) {
        Access::rightRef(parent) = child;
        if (child)
            Access::parentRef(child) = parent;
    }

    static void replace(node_type*& root, node_type* current, node_type* replacement) {
        node_type* parent = Access::parent(current);
        if (!parent)
            root = replacement;
        else if (Access::left(parent) == current)
            Access::leftRef(parent) = replacement;
        else
            Access::rightRef(parent) = replacement;
        if (replacement)
            Access::parentRef(replacement) = parent;
    }

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

    static node_type* restructure(node_type*& root, node_type* value_node) {
        if (!value_node || !Access::parent(value_node) ||
            !Access::parent(Access::parent(value_node)))
            return value_node;

        node_type* parent = Access::parent(value_node);
        node_type* grand = Access::parent(parent);
        if (parent == Access::left(grand)) {
            if (value_node == Access::left(parent))
                return rotateRight(root, grand);
            rotateLeft(root, parent);
            return rotateRight(root, grand);
        }
        if (value_node == Access::right(parent))
            return rotateLeft(root, grand);
        rotateRight(root, parent);
        return rotateLeft(root, grand);
    }

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
