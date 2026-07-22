#ifndef DSA_CORE_SKIPLIST_SKIP_LIST_ALGORITHM_H
#define DSA_CORE_SKIPLIST_SKIP_LIST_ALGORITHM_H

#include <cstddef>

#include <dsa/container/vector/Vector.h>

namespace dsa {
namespace core {

// 跳表共享流程：随机层高与基于 forward(level) 的查找路径，不管理节点生命周期。
class SkipListAlgorithm {
public:
    template<typename Engine>
    static std::size_t randomLevel(
        Engine& engine,
        double probability,
        std::size_t maximumLevel
    ) {
        std::size_t level = 1;
        const typename Engine::result_type threshold = static_cast<typename Engine::result_type>(
            probability * static_cast<double>(Engine::max())
        );
        while (level < maximumLevel && engine() <= threshold)
            ++level;
        return level;
    }

    template<typename Node, typename Key, typename Compare, typename KeyAccess, typename ForwardAccess>
    static Node* lowerBoundPath(
        Node* head,
        std::size_t levelCount,
        const Key& key,
        const Compare& compare,
        const KeyAccess& keyAccess,
        const ForwardAccess& forwardAccess,
        dsa::container::Vector<Node*>* path
    ) {
        Node* current = head;
        for (std::size_t level = levelCount; level > 0; --level) {
            const std::size_t index = level - 1;
            Node* next = forwardAccess(current, index);
            while (next && compare(keyAccess(next), key)) {
                current = next;
                next = forwardAccess(current, index);
            }
            if (path)
                (*path)[index] = current;
        }
        return forwardAccess(current, 0);
    }
};

} // namespace core
} // namespace dsa

#endif
