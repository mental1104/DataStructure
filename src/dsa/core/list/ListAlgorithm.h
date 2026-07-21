#ifndef DSA_CORE_LIST_LIST_ALGORITHM_H
#define DSA_CORE_LIST_LIST_ALGORITHM_H

#include <utility>

namespace dsa {
namespace core {

/// 协调双向链表的节点创建、链接、摘除和跨容器转移流程。
///
/// Storage 负责具体节点类型、allocator、对象生命周期和 size 字段；
/// 本类只固定“先构造、后链接、最后提交”的顺序，避免教学版与工业版
/// 分别复制一套易出错的 mutation workflow。
///
/// Storage contract:
///   typedef node_type
///   typedef size_type
///   template<typename... Args> node_type* createNode(Args&&...)
///   void destroyNode(node_type*) noexcept
///   void linkBefore(node_type* position, node_type* node) noexcept
///   void unlink(node_type* node) noexcept
///   node_type* next(node_type*) const noexcept
///   size_type countRange(node_type* first, node_type* last) const
///   bool sameOwner(const Storage&) const noexcept
///   bool contains(node_type* first, node_type* last, node_type* target) const
///   void transferBefore(node_type* position, node_type* first, node_type* last) noexcept
///   void commitInsert(size_type count) noexcept
///   void commitErase(size_type count) noexcept
template<typename Storage>
class ListAlgorithm {
public:
    typedef typename Storage::node_type node_type;
    typedef typename Storage::size_type size_type;

    /// 构造一个脱链节点，成功后将其链接到 position 前并提交 size。
    template<typename... Args>
    static node_type* emplaceBefore(
        Storage& storage,
        node_type* position,
        Args&&... args
    ) {
        node_type* node = storage.createNode(std::forward<Args>(args)...);
        storage.linkBefore(position, node);
        storage.commitInsert(size_type(1));
        return node;
    }

    /// 摘除并销毁 node，返回删除位置原本的后继节点。
    static node_type* erase(Storage& storage, node_type* node) {
        node_type* successor = storage.next(node);
        storage.unlink(node);
        storage.commitErase(size_type(1));
        storage.destroyNode(node);
        return successor;
    }

    /// 将 [first, last) 的节点整体移动到 position 前，不构造、移动或复制元素。
    static void spliceBefore(
        Storage& destination,
        node_type* position,
        Storage& source,
        node_type* first,
        node_type* last
    ) {
        if (first == last || position == first || position == last)
            return;

        if (destination.sameOwner(source)) {
            // position 落在被移动区间内时，保持原链不变，避免破坏链接。
            if (destination.contains(first, last, position))
                return;
            destination.transferBefore(position, first, last);
            return;
        }

        const size_type count = source.countRange(first, last);
        destination.transferBefore(position, first, last);
        source.commitErase(count);
        destination.commitInsert(count);
    }
};

} // namespace core
} // namespace dsa

#endif
