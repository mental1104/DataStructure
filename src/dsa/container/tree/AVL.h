#ifndef DSA_CONTAINER_TREE_AVL_H
#define DSA_CONTAINER_TREE_AVL_H

#include <dsa/container/tree/BST.h>

namespace dsa {
namespace container {

template<
    typename T,
    typename Compare = std::less<T>,
    typename Allocator = std::allocator<T>
>
using AVL = detail::BasicSearchTree<T, Compare, Allocator, detail::AvlTreeTag>;

} // namespace container
} // namespace dsa

#endif
