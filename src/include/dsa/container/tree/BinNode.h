#ifndef DSA_CONTAINER_TREE_BIN_NODE_H
#define DSA_CONTAINER_TREE_BIN_NODE_H

#include <utility>

namespace dsa {
namespace container {

struct BinTreeNodeMetadata {
    int height;

    BinTreeNodeMetadata() noexcept
        : height(0) {}
};

struct CopyBinNodeMetadataTag {};

template<typename T, typename Metadata = BinTreeNodeMetadata>
struct BinNode : Metadata {
    typedef T value_type;
    typedef Metadata metadata_type;
    typedef BinNode<T, Metadata> node_type;

    T data;
    node_type* parent;
    node_type* lc;
    node_type* rc;

    template<typename... Args>
    explicit BinNode(node_type* p, Args&&... args)
        : Metadata(),
          data(std::forward<Args>(args)...),
          parent(p),
          lc(nullptr),
          rc(nullptr) {}

    template<typename... Args>
    BinNode(
        node_type* p,
        CopyBinNodeMetadataTag,
        const Metadata& metadata,
        Args&&... args
    )
        : Metadata(metadata),
          data(std::forward<Args>(args)...),
          parent(p),
          lc(nullptr),
          rc(nullptr) {}

    BinNode(const BinNode&) = delete;
    BinNode& operator=(const BinNode&) = delete;
    BinNode(BinNode&&) = delete;
    BinNode& operator=(BinNode&&) = delete;
};

} // namespace container
} // namespace dsa

#endif
