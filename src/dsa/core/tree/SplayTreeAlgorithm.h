#ifndef DSA_CORE_TREE_SPLAY_TREE_ALGORITHM_H
#define DSA_CORE_TREE_SPLAY_TREE_ALGORITHM_H

#include <dsa/core/tree/SearchTreeAlgorithm.h>

namespace dsa {
namespace core {

// 伸展树共享结构流程：仅编排旋转顺序，不管理节点分配、析构和 size。
template<typename Access>
class SplayTreeAlgorithm {
public:
    typedef typename Access::node_type node_type;
    typedef SearchTreeAlgorithm<Access> search_algorithm_type;

    // 将 node 通过 zig / zig-zig / zig-zag 旋转到根，并返回新根。
    static node_type* splay(node_type*& root, node_type* node) {
        if (!node)
            return node;

        while (Access::parent(node)) {
            node_type* parent = Access::parent(node);
            node_type* grand = Access::parent(parent);
            if (!grand) {
                if (node == Access::left(parent))
                    rotateRight(root, parent);
                else
                    rotateLeft(root, parent);
                continue;
            }

            if (node == Access::left(parent) && parent == Access::left(grand)) {
                rotateRight(root, grand);
                rotateRight(root, parent);
            } else if (node == Access::right(parent) && parent == Access::right(grand)) {
                rotateLeft(root, grand);
                rotateLeft(root, parent);
            } else if (node == Access::right(parent) && parent == Access::left(grand)) {
                rotateLeft(root, parent);
                rotateRight(root, grand);
            } else {
                rotateRight(root, parent);
                rotateLeft(root, grand);
            }
        }
        root = node;
        search_algorithm_type::updateHeightAbove(node);
        return node;
    }

    // 围绕 pivot 左旋并立即修复受影响节点的高度。
    static node_type* rotateLeft(node_type*& root, node_type* pivot) {
        node_type* child = search_algorithm_type::rotateLeft(root, pivot);
        search_algorithm_type::updateHeight(pivot);
        search_algorithm_type::updateHeight(child);
        return child;
    }

    // 围绕 pivot 右旋并立即修复受影响节点的高度。
    static node_type* rotateRight(node_type*& root, node_type* pivot) {
        node_type* child = search_algorithm_type::rotateRight(root, pivot);
        search_algorithm_type::updateHeight(pivot);
        search_algorithm_type::updateHeight(child);
        return child;
    }

    // 查找 value；命中时伸展命中节点，未命中时伸展最后访问节点。
    template<typename Value, typename Compare>
    static node_type* searchAndSplay(
        node_type*& root,
        const Value& value,
        const Compare& compare
    ) {
        node_type* hot = 0;
        node_type*& slot = search_algorithm_type::searchSlot(root, hot, value, compare);
        return splay(root, slot ? slot : hot);
    }
};

} // namespace core
} // namespace dsa

#endif
