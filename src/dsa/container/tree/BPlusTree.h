#ifndef DSA_CONTAINER_TREE_B_PLUS_TREE_H
#define DSA_CONTAINER_TREE_B_PLUS_TREE_H

#include <functional>
#include <memory>
#include <utility>

#include <dsa/container/tree/detail/PackedBPlusTree.h>

namespace dsa {
namespace container {

// B+ 树映射：叶节点保存全部键值，内部节点只保存分隔键；非根目标占用率为 1/2。
template<
    typename Key,
    typename T,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >
>
using BPlusTree = detail::PackedBPlusTree<Key, T, Compare, Allocator, 1, 2>;

} // namespace container
} // namespace dsa

#endif
