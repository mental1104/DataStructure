#ifndef DSA_CORE_TREE_BIN_NODE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_NODE_ALGORITHM_H

#include <cstddef>
#include <deque>
#include <utility>
#include <vector>

namespace dsa {
namespace core {

template<typename Node>
class BinNodeAlgorithm {
private:
    template<typename NodePtr, typename Visitor>
    static void traversePreGeneric(NodePtr root, Visitor& visit) {
        if (!root)
            return;

        NodePtr boundary = root->parent;
        NodePtr previous = boundary;
        NodePtr current = root;

        while (current && current != boundary) {
            NodePtr next = nullptr;

            if (previous == current->parent) {
                visit(current->data);
                if (current->lc)
                    next = current->lc;
                else if (current->rc)
                    next = current->rc;
                else
                    next = current->parent;
            } else if (previous == current->lc) {
                next = current->rc ? current->rc : current->parent;
            } else {
                next = current->parent;
            }

            previous = current;
            current = next;
        }
    }

    template<typename NodePtr, typename Visitor>
    static void traverseInGeneric(NodePtr root, Visitor& visit) {
        if (!root)
            return;

        NodePtr end = successor(maximum(root));
        NodePtr node = minimum(root);
        while (node != end) {
            visit(node->data);
            node = successor(node);
        }
    }

    template<typename NodePtr, typename Visitor>
    static void traversePostGeneric(NodePtr root, Visitor& visit) {
        if (!root)
            return;

        NodePtr boundary = root->parent;
        NodePtr previous = boundary;
        NodePtr current = root;

        while (current && current != boundary) {
            NodePtr next = nullptr;

            if (previous == current->parent) {
                if (current->lc)
                    next = current->lc;
                else if (current->rc)
                    next = current->rc;
                else {
                    visit(current->data);
                    next = current->parent;
                }
            } else if (previous == current->lc) {
                if (current->rc)
                    next = current->rc;
                else {
                    visit(current->data);
                    next = current->parent;
                }
            } else {
                visit(current->data);
                next = current->parent;
            }

            previous = current;
            current = next;
        }
    }

    template<typename NodePtr, typename Visitor>
    static void traverseLevelGeneric(NodePtr root, Visitor& visit) {
        if (!root)
            return;

        std::deque<NodePtr> queue;
        queue.push_back(root);

        while (!queue.empty()) {
            NodePtr node = queue.front();
            queue.pop_front();
            visit(node->data);

            if (node->lc)
                queue.push_back(node->lc);
            if (node->rc)
                queue.push_back(node->rc);
        }
    }

public:
    static int stature(const Node* node) {
        return node ? node->height : -1;
    }

    static bool isRoot(const Node& node) {
        return node.parent == nullptr;
    }

    static bool isLeftChild(const Node& node) {
        return !isRoot(node) && node.parent->lc == &node;
    }

    static bool isRightChild(const Node& node) {
        return !isRoot(node) && node.parent->rc == &node;
    }

    static bool hasParent(const Node& node) {
        return !isRoot(node);
    }

    // Compatibility: the teaching API historically returns a mutable pointer
    // even when the node reference itself is const.
    static Node* leftChild(const Node& node) {
        return node.lc;
    }

    static Node* rightChild(const Node& node) {
        return node.rc;
    }

    static bool hasChild(const Node& node) {
        return node.lc || node.rc;
    }

    static bool hasBothChildren(const Node& node) {
        return node.lc && node.rc;
    }

    static bool isLeaf(const Node& node) {
        return !hasChild(node);
    }

    static bool balanced(const Node& node) {
        return stature(node.lc) == stature(node.rc);
    }

    static int balanceFactor(const Node& node) {
        return stature(node.lc) - stature(node.rc);
    }

    static bool avlBalanced(const Node& node) {
        const int factor = balanceFactor(node);
        return -2 < factor && factor < 2;
    }

    static Node* tallerChild(const Node* node) {
        if (stature(node->lc) > stature(node->rc))
            return node->lc;
        if (stature(node->lc) < stature(node->rc))
            return node->rc;
        return isLeftChild(*node) ? node->lc : node->rc;
    }

    static Node* sibling(const Node* node) {
        if (!node || isRoot(*node))
            return nullptr;
        return isLeftChild(*node) ? node->parent->rc : node->parent->lc;
    }

    static Node* uncle(Node* node) {
        if (!node || !node->parent || !node->parent->parent)
            return nullptr;
        return isLeftChild(*node->parent)
            ? node->parent->parent->rc
            : node->parent->parent->lc;
    }

    static Node* minimum(Node* node) {
        if (!node)
            return nullptr;
        while (node->lc)
            node = node->lc;
        return node;
    }

    static const Node* minimum(const Node* node) {
        if (!node)
            return nullptr;
        while (node->lc)
            node = node->lc;
        return node;
    }

    static Node* maximum(Node* node) {
        if (!node)
            return nullptr;
        while (node->rc)
            node = node->rc;
        return node;
    }

    static const Node* maximum(const Node* node) {
        if (!node)
            return nullptr;
        while (node->rc)
            node = node->rc;
        return node;
    }

    static Node* successor(Node* node) {
        if (!node)
            return nullptr;
        if (node->rc)
            return minimum(node->rc);

        while (node->parent && isRightChild(*node))
            node = node->parent;
        return node->parent;
    }

    static const Node* successor(const Node* node) {
        if (!node)
            return nullptr;
        if (node->rc)
            return minimum(static_cast<const Node*>(node->rc));

        while (node->parent && isRightChild(*node))
            node = node->parent;
        return node->parent;
    }

    static Node* predecessor(Node* node) {
        if (!node)
            return nullptr;
        if (node->lc)
            return maximum(node->lc);

        while (node->parent && isLeftChild(*node))
            node = node->parent;
        return node->parent;
    }

    static const Node* predecessor(const Node* node) {
        if (!node)
            return nullptr;
        if (node->lc)
            return maximum(static_cast<const Node*>(node->lc));

        while (node->parent && isLeftChild(*node))
            node = node->parent;
        return node->parent;
    }

    static std::size_t subtreeSizeWide(const Node* root) {
        if (!root)
            return 0;

        const Node* end = successor(maximum(root));
        const Node* node = minimum(root);
        std::size_t count = 0;

        while (node != end) {
            ++count;
            node = successor(node);
        }
        return count;
    }

    static int subtreeSize(Node* root) {
        return static_cast<int>(subtreeSizeWide(root));
    }

    template<typename Visitor>
    static void traversePre(Node* root, Visitor& visit) {
        traversePreGeneric(root, visit);
    }

    template<typename Visitor>
    static void traversePre(const Node* root, Visitor& visit) {
        traversePreGeneric(root, visit);
    }

    template<typename Visitor>
    static void traverseIn(Node* root, Visitor& visit) {
        traverseInGeneric(root, visit);
    }

    template<typename Visitor>
    static void traverseIn(const Node* root, Visitor& visit) {
        traverseInGeneric(root, visit);
    }

    template<typename Visitor>
    static void traversePost(Node* root, Visitor& visit) {
        traversePostGeneric(root, visit);
    }

    template<typename Visitor>
    static void traversePost(const Node* root, Visitor& visit) {
        traversePostGeneric(root, visit);
    }

    template<typename Visitor>
    static void traverseLevel(Node* root, Visitor& visit) {
        traverseLevelGeneric(root, visit);
    }

    template<typename Visitor>
    static void traverseLevel(const Node* root, Visitor& visit) {
        traverseLevelGeneric(root, visit);
    }

    // O(n) time, O(1) auxiliary memory. The caller must unlink root first.
    template<typename DestroyNode>
    static std::size_t destroySubtree(Node* root, DestroyNode& destroyNode) {
        std::size_t removed = 0;

        while (root) {
            if (root->lc) {
                Node* promoted = root->lc;
                root->lc = promoted->rc;
                if (root->lc)
                    root->lc->parent = root;

                promoted->rc = root;
                promoted->parent = root->parent;
                root->parent = promoted;
                root = promoted;
            } else {
                Node* next = root->rc;
                if (next)
                    next->parent = root->parent;

                destroyNode(root);
                root = next;
                ++removed;
            }
        }
        return removed;
    }

    template<typename CreateNode, typename DestroyNode>
    static Node* cloneSubtree(
        const Node* source,
        Node* parent,
        CreateNode& createNode,
        DestroyNode& destroyNode
    ) {
        if (!source)
            return nullptr;

        Node* result = nullptr;
        try {
            result = createNode(parent, *source);
            typedef std::pair<const Node*, Node*> WorkItem;
            std::vector<WorkItem> pending;
            pending.push_back(WorkItem(source, result));

            while (!pending.empty()) {
                const WorkItem item = pending.back();
                pending.pop_back();
                const Node* src = item.first;
                Node* dst = item.second;

                if (src->lc) {
                    dst->lc = createNode(dst, *src->lc);
                    pending.push_back(WorkItem(src->lc, dst->lc));
                }
                if (src->rc) {
                    dst->rc = createNode(dst, *src->rc);
                    pending.push_back(WorkItem(src->rc, dst->rc));
                }
            }
        } catch (...) {
            if (result)
                destroySubtree(result, destroyNode);
            throw;
        }
        return result;
    }

    template<typename CreateNode, typename DestroyNode>
    static Node* moveCloneSubtree(
        Node* source,
        Node* parent,
        CreateNode& createNode,
        DestroyNode& destroyNode
    ) {
        if (!source)
            return nullptr;

        Node* result = nullptr;
        try {
            result = createNode(parent, *source);
            typedef std::pair<Node*, Node*> WorkItem;
            std::vector<WorkItem> pending;
            pending.push_back(WorkItem(source, result));

            while (!pending.empty()) {
                const WorkItem item = pending.back();
                pending.pop_back();
                Node* src = item.first;
                Node* dst = item.second;

                if (src->lc) {
                    dst->lc = createNode(dst, *src->lc);
                    pending.push_back(WorkItem(src->lc, dst->lc));
                }
                if (src->rc) {
                    dst->rc = createNode(dst, *src->rc);
                    pending.push_back(WorkItem(src->rc, dst->rc));
                }
            }
        } catch (...) {
            if (result)
                destroySubtree(result, destroyNode);
            throw;
        }
        return result;
    }
};

} // namespace core
} // namespace dsa

#endif
