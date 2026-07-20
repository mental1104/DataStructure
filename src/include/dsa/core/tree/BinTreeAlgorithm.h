#ifndef DSA_CORE_TREE_BIN_TREE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_TREE_ALGORITHM_H

#include <algorithm>

#include "dsa/core/tree/BinNodeAlgorithm.h"

namespace dsa {
namespace core {

template<typename Derived, typename T, typename Node>
class BinTreeAlgorithm {
protected:
    using NodeAlgorithm = BinNodeAlgorithm<Node>;

    int updateHeightImpl(Node* node) {
        node->height = 1 + std::max(
            NodeAlgorithm::stature(node->lc),
            NodeAlgorithm::stature(node->rc)
        );
        return node->height;
    }

    void updateHeightAboveImpl(Node* node) {
        Derived& tree = static_cast<Derived&>(*this);
        while (node) {
            tree.updateHeight(node);
            node = node->parent;
        }
    }

    Node*& fromParentToImpl(const Node& node) {
        Derived& tree = static_cast<Derived&>(*this);
        if (NodeAlgorithm::isRoot(node))
            return tree.rootRef();
        if (NodeAlgorithm::isLeftChild(node))
            return node.parent->lc;
        return node.parent->rc;
    }

    int subtreeSizeImpl(Node* root) const {
        return NodeAlgorithm::subtreeSize(root);
    }

    template<typename Visitor>
    void traverseLevelImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseLevel(root, visit);
    }

    template<typename Visitor>
    void traversePreImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePre(root, visit);
    }

    template<typename Visitor>
    void traverseInImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traverseIn(root, visit);
    }

    template<typename Visitor>
    void traversePostImpl(Node* root, Visitor& visit) const {
        NodeAlgorithm::traversePost(root, visit);
    }

    template<typename DestroyNode>
    int destroySubtreeImpl(Node* root, DestroyNode& destroyNode) {
        return NodeAlgorithm::destroySubtree(root, destroyNode);
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
