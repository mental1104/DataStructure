#ifndef DSA_CORE_TREE_BIN_NODE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_NODE_ALGORITHM_H

#include <deque>
#include <vector>

namespace dsa {
namespace core {

template<typename Node>
class BinNodeAlgorithm {
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

    static Node* leftChild(const Node& node) {
        return node.lc;
    }

    static Node* rightChild(const Node& node) {
        return node.rc;
    }

    static bool hasChild(const Node& node) {
        return leftChild(node) || rightChild(node);
    }

    static bool hasBothChildren(const Node& node) {
        return leftChild(node) && rightChild(node);
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

    static int subtreeSize(Node* root) {
        if (!root)
            return 0;
        return 1 + subtreeSize(root->lc) + subtreeSize(root->rc);
    }

    static Node* successor(Node* node) {
        if (!node)
            return nullptr;

        if (node->rc) {
            node = node->rc;
            while (node->lc)
                node = node->lc;
            return node;
        }

        while (node->parent && isRightChild(*node))
            node = node->parent;
        return node->parent;
    }

    template<typename Visitor>
    static void traversePre(Node* root, Visitor& visit) {
        if (!root)
            return;

        std::vector<Node*> stack;
        stack.push_back(root);

        while (!stack.empty()) {
            Node* node = stack.back();
            stack.pop_back();
            visit(node->data);

            if (node->rc)
                stack.push_back(node->rc);
            if (node->lc)
                stack.push_back(node->lc);
        }
    }

    template<typename Visitor>
    static void traverseIn(Node* root, Visitor& visit) {
        std::vector<Node*> stack;
        Node* node = root;

        while (node || !stack.empty()) {
            while (node) {
                stack.push_back(node);
                node = node->lc;
            }

            node = stack.back();
            stack.pop_back();
            visit(node->data);
            node = node->rc;
        }
    }

    template<typename Visitor>
    static void traversePost(Node* root, Visitor& visit) {
        if (!root)
            return;

        std::vector<Node*> pending;
        std::vector<Node*> reversed;
        pending.push_back(root);

        while (!pending.empty()) {
            Node* node = pending.back();
            pending.pop_back();
            reversed.push_back(node);

            if (node->lc)
                pending.push_back(node->lc);
            if (node->rc)
                pending.push_back(node->rc);
        }

        while (!reversed.empty()) {
            visit(reversed.back()->data);
            reversed.pop_back();
        }
    }

    template<typename Visitor>
    static void traverseLevel(Node* root, Visitor& visit) {
        if (!root)
            return;

        std::deque<Node*> queue;
        queue.push_back(root);

        while (!queue.empty()) {
            Node* node = queue.front();
            queue.pop_front();
            visit(node->data);

            if (node->lc)
                queue.push_back(node->lc);
            if (node->rc)
                queue.push_back(node->rc);
        }
    }

    template<typename DestroyNode>
    static int destroySubtree(Node* root, DestroyNode& destroyNode) {
        if (!root)
            return 0;

        Node* left = root->lc;
        Node* right = root->rc;
        const int removed =
            1 + destroySubtree(left, destroyNode)
              + destroySubtree(right, destroyNode);
        destroyNode(root);
        return removed;
    }
};

} // namespace core
} // namespace dsa

#endif
