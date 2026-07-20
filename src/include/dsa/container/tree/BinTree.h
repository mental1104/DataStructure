#ifndef DSA_CONTAINER_TREE_BIN_TREE_H
#define DSA_CONTAINER_TREE_BIN_TREE_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "dsa/container/tree/BinNode.h"
#include "dsa/core/tree/BinTreeAlgorithm.h"

namespace dsa {
namespace container {

template<typename Derived, typename T, typename Metadata, typename Allocator>
class BasicBinTree
    : protected dsa::core::BinTreeAlgorithm<
          Derived,
          T,
          BinNode<T, Metadata>
      > {
public:
    typedef T value_type;
    typedef Metadata metadata_type;
    typedef Allocator allocator_type;
    typedef BinNode<T, Metadata> node_type;
    typedef node_type* node_pointer;
    typedef const node_type* const_node_pointer;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

protected:
    typedef dsa::core::BinTreeAlgorithm<
        Derived,
        T,
        node_type
    > Algorithm;
    typedef dsa::core::BinNodeAlgorithm<node_type> NodeAlgorithm;
    typedef std::allocator_traits<allocator_type> AllocatorTraits;
    typedef typename AllocatorTraits::template rebind_alloc<node_type>
        NodeAllocator;
    typedef std::allocator_traits<NodeAllocator> NodeAllocatorTraits;

    friend class dsa::core::BinTreeAlgorithm<Derived, T, node_type>;

    allocator_type allocator_;
    node_pointer root_;
    size_type size_;

    BasicBinTree()
        : allocator_(), root_(nullptr), size_(0) {}

    explicit BasicBinTree(const allocator_type& allocator)
        : allocator_(allocator), root_(nullptr), size_(0) {}

    BasicBinTree(const BasicBinTree& other)
        : allocator_(
              AllocatorTraits::select_on_container_copy_construction(
                  other.allocator_
              )
          ),
          root_(nullptr),
          size_(0) {
        cloneCopyFrom(other.root_, other.size_);
    }

    BasicBinTree(
        const BasicBinTree& other,
        const allocator_type& allocator
    )
        : allocator_(allocator), root_(nullptr), size_(0) {
        cloneCopyFrom(other.root_, other.size_);
    }

    BasicBinTree(BasicBinTree&& other)
        noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
        : allocator_(std::move(other.allocator_)),
          root_(other.root_),
          size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    BasicBinTree(
        BasicBinTree&& other,
        const allocator_type& allocator
    )
        : allocator_(allocator), root_(nullptr), size_(0) {
        if (allocator_ == other.allocator_) {
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        } else {
            cloneMoveFrom(other.root_, other.size_);
            other.clearStorage();
        }
    }

    ~BasicBinTree() noexcept {
        clearStorage();
    }

    BasicBinTree& operator=(const BasicBinTree& other) {
        if (this == &other)
            return *this;

        copyAssign(
            other,
            typename AllocatorTraits::
                propagate_on_container_copy_assignment()
        );
        return *this;
    }

    BasicBinTree& operator=(BasicBinTree&& other) {
        if (this == &other)
            return *this;

        moveAssign(
            other,
            typename AllocatorTraits::
                propagate_on_container_move_assignment()
        );
        return *this;
    }

    node_pointer& rootRef() noexcept {
        return root_;
    }

    size_type& sizeRef() noexcept {
        return size_;
    }

    int updateHeight(node_pointer node) noexcept {
        return Algorithm::updateHeightImpl(node);
    }

    template<typename... Args>
    node_pointer createNode(node_pointer parent, Args&&... args) {
        return createNodeWithAllocator(
            allocator_,
            parent,
            std::forward<Args>(args)...
        );
    }

    void destroyNode(node_pointer node) noexcept {
        destroyNodeWithAllocator(allocator_, node);
    }

    size_type destroyOwnedSubtree(node_pointer root) noexcept {
        if (!root)
            return 0;

        DestroyWithAllocator destroy(allocator_);
        return NodeAlgorithm::destroySubtree(root, destroy);
    }

public:
    template<bool IsConst>
    class basic_iterator {
        friend class BasicBinTree;
        template<bool>
        friend class basic_iterator;

        typedef typename std::conditional<
            IsConst,
            const_node_pointer,
            node_pointer
        >::type NodePtr;

        NodePtr node_;
        NodePtr root_;

        basic_iterator(NodePtr node, NodePtr root)
            : node_(node), root_(root) {}

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef typename std::conditional<
            IsConst,
            const T*,
            T*
        >::type pointer;
        typedef typename std::conditional<
            IsConst,
            const T&,
            T&
        >::type reference;

        basic_iterator()
            : node_(nullptr), root_(nullptr) {}

        template<
            bool B,
            typename std::enable_if<IsConst && !B, int>::type = 0
        >
        basic_iterator(const basic_iterator<B>& other)
            : node_(other.node_), root_(other.root_) {}

        reference operator*() const {
            return node_->data;
        }

        pointer operator->() const {
            return std::addressof(node_->data);
        }

        basic_iterator& operator++() {
            node_ = NodeAlgorithm::successor(node_);
            return *this;
        }

        basic_iterator operator++(int) {
            basic_iterator copy(*this);
            ++(*this);
            return copy;
        }

        basic_iterator& operator--() {
            node_ = node_
                ? NodeAlgorithm::predecessor(node_)
                : NodeAlgorithm::maximum(root_);
            return *this;
        }

        basic_iterator operator--(int) {
            basic_iterator copy(*this);
            --(*this);
            return copy;
        }

        template<bool B>
        bool operator==(const basic_iterator<B>& other) const {
            return node_ == other.node_;
        }

        template<bool B>
        bool operator!=(const basic_iterator<B>& other) const {
            return !(*this == other);
        }
    };

    typedef basic_iterator<false> iterator;
    typedef basic_iterator<true> const_iterator;

    allocator_type get_allocator() const {
        return allocator_;
    }

    size_type max_size() const {
        NodeAllocator nodeAllocator(allocator_);
        return NodeAllocatorTraits::max_size(nodeAllocator);
    }

    bool empty() const noexcept {
        return root_ == nullptr;
    }

    size_type size() const noexcept {
        return size_;
    }

    node_pointer root() noexcept {
        return root_;
    }

    const_node_pointer root() const noexcept {
        return root_;
    }

    template<typename... Args>
    node_pointer emplace_root(Args&&... args) {
        return Algorithm::emplaceRootImpl(
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    node_pointer emplace_left(node_pointer parent, Args&&... args) {
        return Algorithm::emplaceLeftImpl(
            parent,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    node_pointer emplace_right(node_pointer parent, Args&&... args) {
        return Algorithm::emplaceRightImpl(
            parent,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    node_pointer reset_root(Args&&... args) {
        return Algorithm::resetRootImpl(
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    node_pointer reset_left(node_pointer parent, Args&&... args) {
        return Algorithm::resetLeftImpl(
            parent,
            std::forward<Args>(args)...
        );
    }

    template<typename... Args>
    node_pointer reset_right(node_pointer parent, Args&&... args) {
        return Algorithm::resetRightImpl(
            parent,
            std::forward<Args>(args)...
        );
    }

    size_type erase_subtree(node_pointer root) {
        return Algorithm::eraseSubtreeImpl(root);
    }

    void clear() noexcept {
        Algorithm::clearImpl();
    }

    iterator begin() noexcept {
        return iterator(NodeAlgorithm::minimum(root_), root_);
    }

    iterator end() noexcept {
        return iterator(nullptr, root_);
    }

    const_iterator begin() const noexcept {
        return cbegin();
    }

    const_iterator end() const noexcept {
        return cend();
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(NodeAlgorithm::minimum(root_), root_);
    }

    const_iterator cend() const noexcept {
        return const_iterator(nullptr, root_);
    }

    template<typename Visitor>
    void traverse_pre(Visitor&& visitor) {
        Algorithm::traversePreImpl(root_, visitor);
    }

    template<typename Visitor>
    void traverse_pre(Visitor&& visitor) const {
        Algorithm::traversePreImpl(
            static_cast<const_node_pointer>(root_),
            visitor
        );
    }

    template<typename Visitor>
    void traverse_in(Visitor&& visitor) {
        Algorithm::traverseInImpl(root_, visitor);
    }

    template<typename Visitor>
    void traverse_in(Visitor&& visitor) const {
        Algorithm::traverseInImpl(
            static_cast<const_node_pointer>(root_),
            visitor
        );
    }

    template<typename Visitor>
    void traverse_post(Visitor&& visitor) {
        Algorithm::traversePostImpl(root_, visitor);
    }

    template<typename Visitor>
    void traverse_post(Visitor&& visitor) const {
        Algorithm::traversePostImpl(
            static_cast<const_node_pointer>(root_),
            visitor
        );
    }

    template<typename Visitor>
    void traverse_level(Visitor&& visitor) {
        Algorithm::traverseLevelImpl(root_, visitor);
    }

    template<typename Visitor>
    void traverse_level(Visitor&& visitor) const {
        Algorithm::traverseLevelImpl(
            static_cast<const_node_pointer>(root_),
            visitor
        );
    }

    void swap(BasicBinTree& other) {
        swapImpl(
            other,
            typename AllocatorTraits::propagate_on_container_swap()
        );
    }

private:
    struct DestroyWithAllocator {
        allocator_type* allocator;

        explicit DestroyWithAllocator(allocator_type& value)
            : allocator(&value) {}

        void operator()(node_pointer node) const noexcept {
            destroyNodeWithAllocator(*allocator, node);
        }
    };

    struct SubtreeGuard {
        allocator_type* allocator;
        node_pointer root;

        SubtreeGuard(allocator_type& value, node_pointer node)
            : allocator(&value), root(node) {}

        ~SubtreeGuard() {
            if (root) {
                DestroyWithAllocator destroy(*allocator);
                NodeAlgorithm::destroySubtree(root, destroy);
            }
        }

        node_pointer release() {
            node_pointer result = root;
            root = nullptr;
            return result;
        }

        SubtreeGuard(const SubtreeGuard&) = delete;
        SubtreeGuard& operator=(const SubtreeGuard&) = delete;
    };

    template<typename... Args>
    static node_pointer createNodeWithAllocator(
        allocator_type& allocator,
        node_pointer parent,
        Args&&... args
    ) {
        NodeAllocator nodeAllocator(allocator);
        typename NodeAllocatorTraits::pointer allocated =
            NodeAllocatorTraits::allocate(nodeAllocator, 1);
        node_pointer node = std::addressof(*allocated);

        try {
            NodeAllocatorTraits::construct(
                nodeAllocator,
                node,
                parent,
                std::forward<Args>(args)...
            );
        } catch (...) {
            NodeAllocatorTraits::deallocate(nodeAllocator, allocated, 1);
            throw;
        }
        return node;
    }

    static void destroyNodeWithAllocator(
        allocator_type& allocator,
        node_pointer node
    ) noexcept {
        NodeAllocator nodeAllocator(allocator);
        typename NodeAllocatorTraits::pointer allocated =
            std::pointer_traits<
                typename NodeAllocatorTraits::pointer
            >::pointer_to(*node);

        NodeAllocatorTraits::destroy(nodeAllocator, node);
        NodeAllocatorTraits::deallocate(nodeAllocator, allocated, 1);
    }

    void clearStorage() noexcept {
        node_pointer oldRoot = root_;
        root_ = nullptr;
        size_ = 0;

        if (oldRoot) {
            DestroyWithAllocator destroy(allocator_);
            NodeAlgorithm::destroySubtree(oldRoot, destroy);
        }
    }

    struct CopyCreateWithAllocator {
        allocator_type* allocator;

        explicit CopyCreateWithAllocator(allocator_type& value)
            : allocator(&value) {}

        node_pointer operator()(
            node_pointer parent,
            const node_type& source
        ) const {
            return createNodeWithAllocator(
                *allocator,
                parent,
                CopyBinNodeMetadataTag(),
                static_cast<const Metadata&>(source),
                source.data
            );
        }
    };

    struct MoveCreateWithAllocator {
        allocator_type* allocator;

        explicit MoveCreateWithAllocator(allocator_type& value)
            : allocator(&value) {}

        node_pointer operator()(
            node_pointer parent,
            node_type& source
        ) const {
            return createNodeWithAllocator(
                *allocator,
                parent,
                CopyBinNodeMetadataTag(),
                static_cast<const Metadata&>(source),
                std::move(source.data)
            );
        }
    };

    static node_pointer cloneCopyRoot(
        const_node_pointer source,
        allocator_type& allocator
    ) {
        if (!source)
            return nullptr;

        CopyCreateWithAllocator create(allocator);
        DestroyWithAllocator destroy(allocator);
        return NodeAlgorithm::cloneSubtree(
            source,
            nullptr,
            create,
            destroy
        );
    }

    static node_pointer cloneMoveRoot(
        node_pointer source,
        allocator_type& allocator
    ) {
        if (!source)
            return nullptr;

        MoveCreateWithAllocator create(allocator);
        DestroyWithAllocator destroy(allocator);
        return NodeAlgorithm::moveCloneSubtree(
            source,
            nullptr,
            create,
            destroy
        );
    }

    void cloneCopyFrom(
        const_node_pointer source,
        size_type sourceSize
    ) {
        node_pointer cloned = cloneCopyRoot(source, allocator_);
        SubtreeGuard guard(allocator_, cloned);
        root_ = guard.release();
        size_ = sourceSize;
    }

    void cloneMoveFrom(node_pointer source, size_type sourceSize) {
        node_pointer cloned = cloneMoveRoot(source, allocator_);
        SubtreeGuard guard(allocator_, cloned);
        root_ = guard.release();
        size_ = sourceSize;
    }

    void copyAssign(const BasicBinTree& other, std::false_type) {
        allocator_type targetAllocator = allocator_;
        node_pointer cloned = cloneCopyRoot(
            other.root_,
            targetAllocator
        );
        SubtreeGuard guard(targetAllocator, cloned);

        clearStorage();
        root_ = guard.release();
        size_ = other.size_;
    }

    void copyAssign(const BasicBinTree& other, std::true_type) {
        allocator_type targetAllocator = other.allocator_;
        node_pointer cloned = cloneCopyRoot(
            other.root_,
            targetAllocator
        );
        SubtreeGuard guard(targetAllocator, cloned);

        clearStorage();
        allocator_ = targetAllocator;
        root_ = guard.release();
        size_ = other.size_;
    }

    void moveAssign(BasicBinTree& other, std::true_type) {
        clearStorage();
        allocator_ = std::move(other.allocator_);
        root_ = other.root_;
        size_ = other.size_;
        other.root_ = nullptr;
        other.size_ = 0;
    }

    void moveAssign(BasicBinTree& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            clearStorage();
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
            return;
        }

        allocator_type targetAllocator = allocator_;
        node_pointer cloned = cloneMoveRoot(
            other.root_,
            targetAllocator
        );
        SubtreeGuard guard(targetAllocator, cloned);

        clearStorage();
        root_ = guard.release();
        size_ = other.size_;
        other.clearStorage();
    }

    void swapData(BasicBinTree& other) noexcept {
        using std::swap;
        swap(root_, other.root_);
        swap(size_, other.size_);
    }

    void swapImpl(BasicBinTree& other, std::true_type) {
        using std::swap;
        swap(allocator_, other.allocator_);
        swapData(other);
    }

    void swapImpl(BasicBinTree& other, std::false_type) {
        if (allocator_ == other.allocator_) {
            swapData(other);
            return;
        }

        throw std::logic_error(
            "BinTree::swap requires equal allocators when "
            "propagate_on_container_swap is false"
        );
    }
};

template<typename T, typename Allocator = std::allocator<T> >
class BinTree final
    : public BasicBinTree<
          BinTree<T, Allocator>,
          T,
          BinTreeNodeMetadata,
          Allocator
      > {
private:
    typedef BasicBinTree<
        BinTree<T, Allocator>,
        T,
        BinTreeNodeMetadata,
        Allocator
    > Base;

public:
    typedef typename Base::allocator_type allocator_type;

    BinTree()
        : Base() {}

    explicit BinTree(const allocator_type& allocator)
        : Base(allocator) {}

    BinTree(const BinTree& other)
        : Base(static_cast<const Base&>(other)) {}

    BinTree(const BinTree& other, const allocator_type& allocator)
        : Base(static_cast<const Base&>(other), allocator) {}

    BinTree(BinTree&& other)
        noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
        : Base(std::move(static_cast<Base&>(other))) {}

    BinTree(BinTree&& other, const allocator_type& allocator)
        : Base(std::move(static_cast<Base&>(other)), allocator) {}

    BinTree& operator=(const BinTree& other) {
        Base::operator=(static_cast<const Base&>(other));
        return *this;
    }

    BinTree& operator=(BinTree&& other) {
        Base::operator=(std::move(static_cast<Base&>(other)));
        return *this;
    }
};

template<typename T, typename Allocator>
void swap(BinTree<T, Allocator>& lhs, BinTree<T, Allocator>& rhs) {
    lhs.swap(rhs);
}

} // namespace container
} // namespace dsa

#endif
