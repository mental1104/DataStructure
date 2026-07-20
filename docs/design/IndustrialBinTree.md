# Industrial BinTree Design

## Goals

The industrial tree is an owning, allocator-aware foundation for later AVL,
Splay, and red-black containers. It is intentionally separate from the teaching
`BinTree` API.

| Area | Requirement |
| --- | --- |
| Allocation | Accept a standard-compatible `Allocator`; rebind it to the node type through `std::allocator_traits`. |
| Value types | Do not require default construction or copyability for insertion. Support perfect forwarding, move-only values, over-aligned values, and throwing constructors. |
| Ownership | Each tree exclusively owns its nodes. Node construction is commit-after-success; destruction is allocator-correct. |
| Exception safety | `emplace_*` leaves the tree unchanged if construction fails. `reset_*` allocates first and only then replaces the old subtree. Copy operations build a guarded clone before committing it. |
| Extension cost | Use CRTP and compile-time metadata, with no virtual functions or per-operation function-pointer dispatch. |
| Algorithm reuse | Structural behavior lives in `dsa/core/tree/*Algorithm.h`. The container only owns allocator state, node lifetime, and stable public forwarding methods. |
| Iteration | Provide const-correct bidirectional in-order iterators. Insertion does not invalidate existing iterators; erasing/replacing a subtree invalidates iterators into that subtree. |
| Scale | Destruction and the three depth-first traversals use parent links and do not allocate auxiliary storage. Level-order traversal still requires a queue. |

The container follows the standard library allocator propagation traits for
copy assignment, move assignment, and swap. When
`propagate_on_container_swap` is false, swapping unequal allocators throws
instead of invoking undefined behavior.

## Layering

```text
core/tree/BinNodeAlgorithm.h
    Node relationships, successor/predecessor, traversal, clone and destroy

core/tree/BinTreeAlgorithm.h
    Structural insertion/replacement/erase, height propagation and link updates

container/tree/BinNode.h
    Value + links + compile-time metadata storage

container/tree/BinTree.h
    Allocator ownership, exception-safe node lifetime and public API
```

The stable compile-time hook contract between the algorithm and owning layers is:

```text
rootRef()
sizeRef()
createNode(parent, args...)
destroyOwnedSubtree(root)
updateHeight(node)
```

Algorithm implementation may change without editing the industrial container as
long as this hook contract remains stable.

## Red-black extension point

A red-black tree should derive from `BasicBinTree`, not from the concrete plain
`BinTree`. This preserves the final derived type in CRTP and therefore keeps
height/color repair statically dispatched.

```cpp
struct RedBlackMetadata : dsa::container::BinTreeNodeMetadata {
    bool red;

    RedBlackMetadata() noexcept : red(true) {}
};

template<typename Value, typename Compare, typename Allocator>
class RedBlackTree final
    : public dsa::container::BasicBinTree<
          RedBlackTree<Value, Compare, Allocator>,
          Value,
          RedBlackMetadata,
          Allocator
      >,
      private dsa::core::RedBlackTreeAlgorithm<
          RedBlackTree<Value, Compare, Allocator>
      > {
    // Compare and key-extraction policy live here.
    // Node allocation and ownership come from BasicBinTree.
    // Search, rotation and color repair come from RedBlackTreeAlgorithm.
};
```

A future associative wrapper can then supply policies such as `Compare` and
`KeyOfValue`, using the red-black tree as the underlying component without
using `std::map` or `std::set`.

## Deliberate boundaries

- The container is not internally synchronized. Concurrent mutation requires
  external synchronization.
- Public node pointers are non-owning handles. Directly rewriting `parent`,
  `lc`, or `rc` violates container invariants; structural changes must go
  through the tree or an approved algorithm mixin.
- The current plain binary tree does not define ordering. Ordered lookup and
  range aggregation belong in the future BST/red-black algorithm layers.
