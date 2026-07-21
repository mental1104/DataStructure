#ifndef DSA_CORE_TREE_B_TREE_ALGORITHM_H
#define DSA_CORE_TREE_B_TREE_ALGORITHM_H

#include <cstddef>
#include <vector>

namespace dsa {
namespace core {

// 多路搜索树共享算法：只负责节点内定位、沿孩子下降和结构遍历。
// Access contract:
//   node_type, key_type
//   keyCount(node), key(node, index), childCount(node), child(node, index), parent(node)
template<typename Access>
class BTreeAlgorithm {
public:
    typedef typename Access::node_type node_type;
    typedef typename Access::key_type key_type;

    struct SearchResult {
        node_type* node;
        std::size_t index;
        bool found;

        SearchResult(node_type* value = nullptr, std::size_t position = 0, bool hit = false)
            : node(value), index(position), found(hit) {}
    };

    // 返回第一个不小于 value 的节点内位置。
    template<typename NodePointer, typename Value, typename Compare>
    static std::size_t lowerBound(NodePointer node, const Value& value, const Compare& compare) {
        std::size_t first = 0;
        std::size_t count = Access::keyCount(node);
        while (count) {
            const std::size_t step = count / 2;
            const std::size_t middle = first + step;
            if (compare(Access::key(node, middle), value)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    template<typename Left, typename Right, typename Compare>
    static bool equivalent(const Left& left, const Right& right, const Compare& compare) {
        return !compare(left, right) && !compare(right, left);
    }

    // 沿多路搜索树下降。未命中时返回最终叶节点及其插入位置。
    template<typename Value, typename Compare>
    static SearchResult search(node_type* root, const Value& value, const Compare& compare) {
        node_type* current = root;
        node_type* last = nullptr;
        std::size_t index = 0;
        while (current) {
            last = current;
            index = lowerBound(current, value, compare);
            if (index < Access::keyCount(current) &&
                equivalent(Access::key(current, index), value, compare))
                return SearchResult(current, index, true);
            if (Access::childCount(current) == 0)
                return SearchResult(current, index, false);
            current = Access::child(current, index);
        }
        return SearchResult(last, index, false);
    }

    template<typename NodePointer>
    static NodePointer leftmostLeaf(NodePointer node) {
        while (node && Access::childCount(node) != 0)
            node = Access::child(node, 0);
        return node;
    }

    template<typename NodePointer>
    static NodePointer rightmostLeaf(NodePointer node) {
        while (node && Access::childCount(node) != 0)
            node = Access::child(node, Access::childCount(node) - 1);
        return node;
    }

    // 返回 child 在 parent 孩子数组中的位置；未找到返回 childCount(parent)。
    static std::size_t childIndex(const node_type* parent, const node_type* child) {
        const std::size_t count = Access::childCount(parent);
        for (std::size_t index = 0; index < count; ++index) {
            if (Access::child(parent, index) == child)
                return index;
        }
        return count;
    }

    // 以后序遍历释放整棵多路树；Destroy 负责实际析构和释放。
    template<typename Destroy>
    static std::size_t destroySubtree(node_type* root, Destroy&& destroy) {
        if (!root)
            return 0;

        struct Frame {
            node_type* node;
            std::size_t next_child;
            Frame(node_type* value, std::size_t index) : node(value), next_child(index) {}
        };

        std::vector<Frame> stack;
        stack.push_back(Frame(root, 0));
        std::size_t count = 0;
        while (!stack.empty()) {
            Frame& frame = stack.back();
            const std::size_t child_count = Access::childCount(frame.node);
            if (frame.next_child < child_count) {
                node_type* child = Access::child(frame.node, frame.next_child++);
                if (child)
                    stack.push_back(Frame(child, 0));
                continue;
            }

            node_type* completed = frame.node;
            stack.pop_back();
            destroy(completed);
            ++count;
        }
        return count;
    }
};

} // namespace core
} // namespace dsa

#endif
