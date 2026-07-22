#ifndef DSA_CONTAINER_TREE_DETAIL_PACKED_B_PLUS_TREE_H
#define DSA_CONTAINER_TREE_DETAIL_PACKED_B_PLUS_TREE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include <dsa/container/vector/Vector.h>

#include <dsa/core/tree/MultiwayTreeAlgorithm.h>

namespace dsa {
namespace container {
namespace detail {

// 事务式打包 B+ 树。查找为 O(log n)，插入/删除通过重建节点层提供 strong guarantee，复杂度 O(n)。
template<
    typename Key,
    typename T,
    typename Compare,
    typename Allocator,
    std::size_t MinimumNumerator,
    std::size_t MinimumDenominator
>
class PackedBPlusTree {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef Compare key_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

private:
    struct Node {
        bool leaf;
        Node* parent;
        Node* next;
        Node* previous;
        dsa::container::Vector<const key_type*> keys;
        dsa::container::Vector<value_type*> values;
        dsa::container::Vector<Node*> children;

        explicit Node(bool isLeaf)
            : leaf(isLeaf), parent(0), next(0), previous(0), keys(), values(), children() {}
    };

    typedef std::allocator_traits<allocator_type> value_allocator_traits;
    typedef typename value_allocator_traits::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;
    typedef typename value_allocator_traits::template rebind_alloc<value_type*> pointer_allocator_type;
    typedef dsa::container::Vector<value_type*, pointer_allocator_type> entry_array_type;

public:
    class const_iterator {
        friend class PackedBPlusTree;
        const Node* leaf_;
        size_type index_;
        const_iterator(const Node* leaf, size_type index) : leaf_(leaf), index_(index) {}
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename PackedBPlusTree::value_type value_type;
        typedef typename PackedBPlusTree::difference_type difference_type;
        typedef const value_type* pointer;
        typedef const value_type& reference;

        const_iterator() : leaf_(0), index_(0) {}
        reference operator*() const { return *leaf_->values[index_]; }
        pointer operator->() const { return leaf_->values[index_]; }
        const_iterator& operator++() {
            ++index_;
            if (leaf_ && index_ >= leaf_->values.size()) {
                leaf_ = leaf_->next;
                index_ = 0;
            }
            return *this;
        }
        const_iterator operator++(int) { const_iterator copy(*this); ++(*this); return copy; }
        friend bool operator==(const const_iterator& left, const const_iterator& right) {
            return left.leaf_ == right.leaf_ && left.index_ == right.index_;
        }
        friend bool operator!=(const const_iterator& left, const const_iterator& right) {
            return !(left == right);
        }
    };

    class iterator {
        friend class PackedBPlusTree;
        Node* leaf_;
        size_type index_;
        iterator(Node* leaf, size_type index) : leaf_(leaf), index_(index) {}
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename PackedBPlusTree::value_type value_type;
        typedef typename PackedBPlusTree::difference_type difference_type;
        typedef value_type* pointer;
        typedef value_type& reference;

        iterator() : leaf_(0), index_(0) {}
        operator const_iterator() const { return const_iterator(leaf_, index_); }
        reference operator*() const { return *leaf_->values[index_]; }
        pointer operator->() const { return leaf_->values[index_]; }
        iterator& operator++() {
            ++index_;
            if (leaf_ && index_ >= leaf_->values.size()) {
                leaf_ = leaf_->next;
                index_ = 0;
            }
            return *this;
        }
        iterator operator++(int) { iterator copy(*this); ++(*this); return copy; }
        friend bool operator==(const iterator& left, const iterator& right) {
            return left.leaf_ == right.leaf_ && left.index_ == right.index_;
        }
        friend bool operator!=(const iterator& left, const iterator& right) { return !(left == right); }
    };

private:
    allocator_type allocator_;
    key_compare compare_;
    size_type order_;
    entry_array_type entries_;
    Node* root_;
    Node* first_leaf_;

    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    Node* createNode(bool leaf) {
        node_allocator_type allocator = nodeAllocator();
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(allocator, node, leaf);
        } catch (...) {
            node_allocator_traits::deallocate(allocator, node, 1);
            throw;
        }
        return node;
    }

    void destroyNode(Node* node) {
        node_allocator_type allocator = nodeAllocator();
        node_allocator_traits::destroy(allocator, node);
        node_allocator_traits::deallocate(allocator, node, 1);
    }

    template<typename M>
    value_type* createValue(const key_type& key, M&& value) {
        value_type* stored = value_allocator_traits::allocate(allocator_, 1);
        try {
            value_allocator_traits::construct(allocator_, stored, key, std::forward<M>(value));
        } catch (...) {
            value_allocator_traits::deallocate(allocator_, stored, 1);
            throw;
        }
        return stored;
    }

    void destroyValue(value_type* value) {
        value_allocator_traits::destroy(allocator_, value);
        value_allocator_traits::deallocate(allocator_, value, 1);
    }

    void destroyTree(Node* node) {
        if (!node)
            return;
        if (!node->leaf) {
            for (size_type index = 0; index < node->children.size(); ++index)
                destroyTree(node->children[index]);
        }
        destroyNode(node);
    }

    bool equivalent(const key_type& left, const key_type& right) const {
        return !compare_(left, right) && !compare_(right, left);
    }

    size_type maxKeys() const { return order_ - 1; }
    size_type minimumCount(size_type maximum) const {
        size_type result = (maximum * MinimumNumerator + MinimumDenominator - 1) / MinimumDenominator;
        return result == 0 ? 1 : result;
    }
    size_type minLeafKeys() const { return minimumCount(maxKeys()); }
    size_type minChildren() const {
        size_type result = minimumCount(order_);
        return result < 2 ? 2 : result;
    }

    size_type entryLowerBound(const key_type& key) const {
        size_type first = 0;
        size_type count = entries_.size();
        while (count > 0) {
            const size_type step = count / 2;
            const size_type middle = first + step;
            if (compare_(entries_[middle]->first, key)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    const key_type* minimumKey(Node* node) const {
        while (node && !node->leaf)
            node = node->children[0];
        return node && !node->values.empty() ? &node->values[0]->first : 0;
    }

    Node* findLeaf(const key_type& key) const {
        Node* node = root_;
        while (node && !node->leaf) {
            size_type first = 0;
            size_type count = node->keys.size();
            while (count > 0) {
                const size_type step = count / 2;
                const size_type middle = first + step;
                if (!compare_(key, *node->keys[middle])) {
                    first = middle + 1;
                    count -= step + 1;
                } else {
                    count = step;
                }
            }
            node = node->children[first];
        }
        return node;
    }

    size_type leafLowerBound(const Node* leaf, const key_type& key) const {
        size_type first = 0;
        size_type count = leaf->values.size();
        while (count > 0) {
            const size_type step = count / 2;
            const size_type middle = first + step;
            if (compare_(leaf->values[middle]->first, key)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    struct BuildResult {
        Node* root;
        Node* firstLeaf;
        BuildResult() : root(0), firstLeaf(0) {}
    };

    BuildResult buildTree() {
        BuildResult result;
        if (entries_.empty())
            return result;

        dsa::container::Vector<Node*> allNodes;
        try {
            dsa::container::Vector<Node*> level;
            const dsa::container::Vector<size_type> leafCounts = dsa::core::MultiwayTreeAlgorithm::partitionCounts(
                entries_.size(), maxKeys(), minLeafKeys()
            );
            size_type offset = 0;
            Node* previous = 0;
            for (size_type group = 0; group < leafCounts.size(); ++group) {
                Node* leaf = createNode(true);
                allNodes.push_back(leaf);
                leaf->values.reserve(leafCounts[group]);
                for (size_type index = 0; index < leafCounts[group]; ++index)
                    leaf->values.push_back(entries_[offset++]);
                leaf->previous = previous;
                if (previous)
                    previous->next = leaf;
                else
                    result.firstLeaf = leaf;
                previous = leaf;
                level.push_back(leaf);
            }

            while (level.size() > 1) {
                const dsa::container::Vector<size_type> parentCounts =
                    level.size() <= order_
                    ? dsa::container::Vector<size_type>(1, level.size())
                    : dsa::core::MultiwayTreeAlgorithm::partitionCounts(
                        level.size(), order_, minChildren()
                    );
                dsa::container::Vector<Node*> parents;
                size_type childOffset = 0;
                for (size_type group = 0; group < parentCounts.size(); ++group) {
                    Node* parent = createNode(false);
                    allNodes.push_back(parent);
                    parent->children.reserve(parentCounts[group]);
                    parent->keys.reserve(parentCounts[group] - 1);
                    for (size_type index = 0; index < parentCounts[group]; ++index) {
                        Node* child = level[childOffset++];
                        child->parent = parent;
                        parent->children.push_back(child);
                        if (index > 0)
                            parent->keys.push_back(minimumKey(child));
                    }
                    parents.push_back(parent);
                }
                level.swap(parents);
            }
            result.root = level[0];
            return result;
        } catch (...) {
            for (size_type index = allNodes.size(); index > 0; --index)
                destroyNode(allNodes[index - 1]);
            throw;
        }
    }

    void rebuildCommit() {
        BuildResult replacement = buildTree();
        Node* oldRoot = root_;
        root_ = replacement.root;
        first_leaf_ = replacement.firstLeaf;
        destroyTree(oldRoot);
    }

    void copyFrom(const PackedBPlusTree& other) {
        try {
            for (size_type index = 0; index < other.entries_.size(); ++index)
                entries_.push_back(createValue(other.entries_[index]->first, other.entries_[index]->second));
            BuildResult built = buildTree();
            root_ = built.root;
            first_leaf_ = built.firstLeaf;
        } catch (...) {
            for (size_type index = 0; index < entries_.size(); ++index)
                destroyValue(entries_[index]);
            entries_.clear();
            throw;
        }
    }

    bool validateNode(const Node* node, bool isRoot, size_type depth, size_type& leafDepth) const {
        if (!node)
            return false;
        if (node->leaf) {
            if (leafDepth == static_cast<size_type>(-1))
                leafDepth = depth;
            if (leafDepth != depth || node->values.empty() || node->values.size() > maxKeys())
                return false;
            if (!isRoot && node->parent && node->parent->parent && node->values.size() < minLeafKeys())
                return false;
            for (size_type i = 1; i < node->values.size(); ++i) {
                if (!compare_(node->values[i - 1]->first, node->values[i]->first))
                    return false;
            }
            return true;
        }
        if (node->children.size() != node->keys.size() + 1 || node->children.size() > order_)
            return false;
        if (!isRoot && node->parent && node->parent->parent && node->children.size() < minChildren())
            return false;
        for (size_type index = 0; index < node->children.size(); ++index) {
            if (node->children[index]->parent != node)
                return false;
            if (index > 0 && node->keys[index - 1] != minimumKey(node->children[index]))
                return false;
            if (!validateNode(node->children[index], false, depth + 1, leafDepth))
                return false;
        }
        return true;
    }

public:
    explicit PackedBPlusTree(
        size_type order = 32,
        const key_compare& compare = key_compare(),
        const allocator_type& allocator = allocator_type()
    ) : allocator_(allocator), compare_(compare), order_(order < 3 ? 3 : order),
        entries_(pointer_allocator_type(allocator)), root_(0), first_leaf_(0) {}

    PackedBPlusTree(const PackedBPlusTree& other)
        : allocator_(value_allocator_traits::select_on_container_copy_construction(other.allocator_)),
          compare_(other.compare_), order_(other.order_), entries_(pointer_allocator_type(allocator_)),
          root_(0), first_leaf_(0) {
        copyFrom(other);
    }

    // 移动构造通过同 allocator 的空树交换所有权，源树保持可复用空状态。
    PackedBPlusTree(PackedBPlusTree&& other)
        : PackedBPlusTree(other.order_, other.compare_, other.allocator_) {
        swap(other);
    }

    ~PackedBPlusTree() {
        destroyTree(root_);
        for (size_type index = 0; index < entries_.size(); ++index)
            destroyValue(entries_[index]);
    }

    PackedBPlusTree& operator=(PackedBPlusTree other) {
        swap(other);
        return *this;
    }

    void swap(PackedBPlusTree& other) {
        using std::swap;
        swap(allocator_, other.allocator_);
        swap(compare_, other.compare_);
        swap(order_, other.order_);
        entries_.swap(other.entries_);
        swap(root_, other.root_);
        swap(first_leaf_, other.first_leaf_);
    }

    allocator_type get_allocator() const { return allocator_; }
    size_type order() const { return order_; }
    size_type size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }
    float target_minimum_occupancy() const {
        return static_cast<float>(MinimumNumerator) / static_cast<float>(MinimumDenominator);
    }

    iterator begin() { return iterator(first_leaf_, 0); }
    const_iterator begin() const { return const_iterator(first_leaf_, 0); }
    iterator end() { return iterator(0, 0); }
    const_iterator end() const { return const_iterator(0, 0); }

    iterator lower_bound(const key_type& key) {
        Node* leaf = findLeaf(key);
        if (!leaf)
            return end();
        const size_type index = leafLowerBound(leaf, key);
        return index == leaf->values.size() ? iterator(leaf->next, 0) : iterator(leaf, index);
    }
    const_iterator lower_bound(const key_type& key) const {
        Node* leaf = findLeaf(key);
        if (!leaf)
            return end();
        const size_type index = leafLowerBound(leaf, key);
        return index == leaf->values.size() ? const_iterator(leaf->next, 0) : const_iterator(leaf, index);
    }

    iterator find(const key_type& key) {
        iterator candidate = lower_bound(key);
        return candidate != end() && equivalent(candidate->first, key) ? candidate : end();
    }
    const_iterator find(const key_type& key) const {
        const_iterator candidate = lower_bound(key);
        return candidate != end() && equivalent(candidate->first, key) ? candidate : end();
    }

    mapped_type* get(const key_type& key) {
        iterator found = find(key);
        return found == end() ? 0 : &found->second;
    }
    const mapped_type* get(const key_type& key) const {
        const_iterator found = find(key);
        return found == end() ? 0 : &found->second;
    }
    bool contains(const key_type& key) const { return find(key) != end(); }

    template<typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& value) {
        const size_type position = entryLowerBound(key);
        if (position < entries_.size() && equivalent(entries_[position]->first, key)) {
            entries_[position]->second = std::forward<M>(value);
            return std::make_pair(find(key), false);
        }
        value_type* created = createValue(key, std::forward<M>(value));
        try {
            entries_.insert(entries_.begin() + static_cast<difference_type>(position), created);
        } catch (...) {
            destroyValue(created);
            throw;
        }
        try {
            rebuildCommit();
        } catch (...) {
            entries_.erase(entries_.begin() + static_cast<difference_type>(position));
            destroyValue(created);
            throw;
        }
        return std::make_pair(find(key), true);
    }

    bool insert(const key_type& key, const mapped_type& value) {
        const size_type position = entryLowerBound(key);
        if (position < entries_.size() && equivalent(entries_[position]->first, key))
            return false;
        insert_or_assign(key, value);
        return true;
    }

    size_type erase(const key_type& key) {
        const size_type position = entryLowerBound(key);
        if (position >= entries_.size() || !equivalent(entries_[position]->first, key))
            return 0;
        value_type* removed = entries_[position];
        entries_.erase(entries_.begin() + static_cast<difference_type>(position));
        try {
            rebuildCommit();
        } catch (...) {
            entries_.insert(entries_.begin() + static_cast<difference_type>(position), removed);
            throw;
        }
        destroyValue(removed);
        return 1;
    }

    mapped_type& at(const key_type& key) {
        mapped_type* value = get(key);
        if (!value)
            throw std::out_of_range("B+ tree key not found");
        return *value;
    }
    const mapped_type& at(const key_type& key) const {
        const mapped_type* value = get(key);
        if (!value)
            throw std::out_of_range("B+ tree key not found");
        return *value;
    }

    void clear() {
        destroyTree(root_);
        root_ = 0;
        first_leaf_ = 0;
        for (size_type index = 0; index < entries_.size(); ++index)
            destroyValue(entries_[index]);
        entries_.clear();
    }

    template<typename Result, typename Aggregate>
    Result range_aggregate(
        const key_type& first,
        const key_type& last,
        Result identity,
        Aggregate aggregate
    ) const {
        const_iterator it = lower_bound(first);
        while (it != end() && !compare_(last, it->first)) {
            identity = aggregate(identity, it->second);
            ++it;
        }
        return identity;
    }

    bool validate() const {
        if (entries_.empty())
            return root_ == 0 && first_leaf_ == 0;
        if (!root_ || root_->parent)
            return false;
        size_type leafDepth = static_cast<size_type>(-1);
        if (!validateNode(root_, true, 0, leafDepth))
            return false;
        size_type count = 0;
        const Node* previousLeaf = 0;
        const key_type* previousKey = 0;
        for (const Node* leaf = first_leaf_; leaf; leaf = leaf->next) {
            if (leaf->previous != previousLeaf)
                return false;
            for (size_type index = 0; index < leaf->values.size(); ++index) {
                const key_type& key = leaf->values[index]->first;
                if (previousKey && !compare_(*previousKey, key))
                    return false;
                previousKey = &key;
                ++count;
            }
            previousLeaf = leaf;
        }
        return count == entries_.size();
    }
};

} // namespace detail
} // namespace container
} // namespace dsa

#endif
