#ifndef DSA_CORE_TREE_BIN_TREE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_TREE_ALGORITHM_H

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include "dsa/core/tree/BinNodeAlgorithm.h"

namespace dsa {
namespace core {

template<typename Derived, typename T, typename Node>
class BinTreeAlgorithm {
protected:
    typedef BinNodeAlgorithm<Node> NodeAlgorithm;

    Derived& derived() {
        return static_cast<Derived&>(*this);
    }

    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }

    int updateHeightImpl(Node* node) {
        node->height = 1 + std::max(
            NodeAlgorithm::stature(node->lc),
            NodeAlgorithm::stature(node->rc)
        );
        return node->height;
    }

    void updateHeightAboveImpl(Node* node) {
        Derived& tree = derived();
        while (node) {
            tree.updateHeight(node);
            node = node->parent;
        }
    }

    Node*& fromParentToImpl(const Node& node) {
        Derived& tree = derived();
        if (NodeAlgorithm::isRoot(node))
            return tree.rootRef();
        if (NodeAlgorithm::isLeftChild(node))
            return node.parent->lc;
        return node.parent->rc;
    }

    int subtreeSizeImpl(Node* root) const {
        return NodeAlgorithm::subtreeSize(root);
    }

    std::size_t subtreeSizeWideImpl(const Node* root) const {
        return NodeAlgorithm::subtreeSizeWide(root);
    }

    template<typename Visitor>
    void traverseLevelImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseLevel(root, visit);
    }

    template<typename Visitor>
    void traverseLevelImpl(const Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseLevel(root, visit);
    }

    template<typename Visitor>
    void traversePreImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePre(root, visit);
    }

    template<typename Visitor>
    void traversePreImpl(const Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePre(root, visit);
    }

    template<typename Visitor>
    void traverseInImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseIn(root, visit);
    }

    template<typename Visitor>
    void traverseInImpl(const Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseIn(root, visit);
    }

    template<typename Visitor>
    void traversePostImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePost(root, visit);
    }

    template<typename Visitor>
    void traversePostImpl(const Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePost(root, visit);
    }

    template<typename DestroyNode>
    std::size_t destroySubtreeImpl(Node* root, DestroyNode& destroyNode) {
        return NodeAlgorithm::destroySubtree(root, destroyNode);
    }

    template<typename... Args>
    Node* emplaceRootImpl(Args&&... args) {
        Derived& tree = derived();
        if (tree.rootRef())
            throw std::logic_error("BinTree root already exists");

        Node* node = tree.createNode(nullptr, std::forward<Args>(args)...);
        tree.rootRef() = node;
        tree.sizeRef() = 1;
        return node;
    }

    template<typename... Args>
    Node* emplaceLeftImpl(Node* parent, Args&&... args) {
        Derived& tree = derived();
        if (!parent)
            throw std::invalid_argument("BinTree parent is null");
        if (parent->lc)
            throw std::logic_error("BinTree left child already exists");

        Node* node = tree.createNode(
            parent,
            std::forward<Args>(args)...
        );
        parent->lc = node;
        ++tree.sizeRef();
        updateHeightAboveImpl(parent);
        return node;
    }

    template<typename... Args>
    Node* emplaceRightImpl(Node* parent, Args&&... args) {
        Derived& tree = derived();
        if (!parent)
            throw std::invalid_argument("BinTree parent is null");
        if (parent->rc)
            throw std::logic_error("BinTree right child already exists");

        Node* node = tree.createNode(
            parent,
            std::forward<Args>(args)...
        );
        parent->rc = node;
        ++tree.sizeRef();
        updateHeightAboveImpl(parent);
        return node;
    }

    template<typename... Args>
    Node* resetRootImpl(Args&&... args) {
        Derived& tree = derived();
        Node* node = tree.createNode(nullptr, std::forward<Args>(args)...);
        Node* oldRoot = tree.rootRef();

        tree.rootRef() = node;
        tree.sizeRef() = 1;
        tree.destroyOwnedSubtree(oldRoot);
        return node;
    }

    template<typename... Args>
    Node* resetLeftImpl(Node* parent, Args&&... args) {
        Derived& tree = derived();
        if (!parent)
            throw std::invalid_argument("BinTree parent is null");

        Node* node = tree.createNode(
            parent,
            std::forward<Args>(args)...
        );
        Node* oldRoot = parent->lc;
        const std::size_t removed =
            NodeAlgorithm::subtreeSizeWide(oldRoot);

        parent->lc = node;
        tree.sizeRef() = tree.sizeRef() + 1 - removed;
        tree.destroyOwnedSubtree(oldRoot);
        updateHeightAboveImpl(parent);
        return node;
    }

    template<typename... Args>
    Node* resetRightImpl(Node* parent, Args&&... args) {
        Derived& tree = derived();
        if (!parent)
            throw std::invalid_argument("BinTree parent is null");

        Node* node = tree.createNode(
            parent,
            std::forward<Args>(args)...
        );
        Node* oldRoot = parent->rc;
        const std::size_t removed =
            NodeAlgorithm::subtreeSizeWide(oldRoot);

        parent->rc = node;
        tree.sizeRef() = tree.sizeRef() + 1 - removed;
        tree.destroyOwnedSubtree(oldRoot);
        updateHeightAboveImpl(parent);
        return node;
    }

    std::size_t eraseSubtreeImpl(Node* root) {
        Derived& tree = derived();
        if (!root)
            return 0;

        Node* parent = root->parent;
        if (!parent)
            tree.rootRef() = nullptr;
        else if (parent->lc == root)
            parent->lc = nullptr;
        else if (parent->rc == root)
            parent->rc = nullptr;
        else
            throw std::invalid_argument(
                "BinTree node is not linked to its parent"
            );

        root->parent = nullptr;
        const std::size_t removed = tree.destroyOwnedSubtree(root);
        tree.sizeRef() -= removed;
        updateHeightAboveImpl(parent);
        return removed;
    }

    void clearImpl() {
        Derived& tree = derived();
        Node* root = tree.rootRef();
        tree.rootRef() = nullptr;
        tree.sizeRef() = 0;
        tree.destroyOwnedSubtree(root);
    }

    template<typename Res, typename Agg>
    Res rangeAggregateRecImpl(
        Node* node,
        const T& lo,
        const T& hi,
        Res acc,
        Agg& agg
    ) const {
        if (!node)
            return acc;
        if (node->data < lo)
            return rangeAggregateRecImpl(node->rc, lo, hi, acc, agg);
        if (hi < node->data)
            return rangeAggregateRecImpl(node->lc, lo, hi, acc, agg);

        acc = rangeAggregateRecImpl(node->lc, lo, hi, acc, agg);
        acc = agg(acc, node->data);
        return rangeAggregateRecImpl(node->rc, lo, hi, acc, agg);
    }
};

} // namespace core
} // namespace dsa

#endif
