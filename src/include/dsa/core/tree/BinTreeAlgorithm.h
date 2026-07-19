#ifndef DSA_CORE_TREE_BIN_TREE_ALGORITHM_H
#define DSA_CORE_TREE_BIN_TREE_ALGORITHM_H

#include <algorithm>

namespace dsa {
namespace core {

template<typename Derived, typename T, typename Node>
class BinTreeAlgorithm {
protected:
    int updateHeightImpl(Node* node) {
        node->height = 1 + std::max(stature(node->lc), stature(node->rc));
        return node->height;
    }

    void updateHeightAboveImpl(Node* node) {
        Derived& tree = static_cast<Derived&>(*this);
        while(node){
            tree.updateHeight(node);
            node = node->parent;
        }
    }

    Node*& fromParentToImpl(const Node& node) {
        Derived& tree = static_cast<Derived&>(*this);
        if(IsRoot(node))
            return tree.rootRef();
        else if(IsLChild(node))
            return node.parent->lc;
        else
            return node.parent->rc;
    }

    template<typename Res, typename Agg>
    Res rangeAggregateRecImpl(Node* node, const T& lo, const T& hi,
                              Res acc, Agg& agg) const {
        if (!node) return acc;
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
