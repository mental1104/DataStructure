#ifndef DSA_CONTAINER_SKIPLIST_SKIP_LIST_H
#define DSA_CONTAINER_SKIPLIST_SKIP_LIST_H

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <new>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <dsa/core/skiplist/SkipListAlgorithm.h>

namespace dsa {
namespace container {

// allocator-aware 有序映射跳表；节点高度元数据与 value 均遵循用户 allocator 的 rebind 规则。
template<
    typename Key,
    typename T,
    typename Compare = std::less<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T> >,
    typename Engine = std::mt19937
>
class SkipList {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<const key_type, mapped_type> value_type;
    typedef Compare key_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

private:
    struct Node;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<Node*> pointer_allocator_type;
    typedef std::vector<Node*, pointer_allocator_type> forward_array_type;

    struct Node {
        forward_array_type forward;
        bool engaged;
        typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type storage;

        Node(size_type level, const pointer_allocator_type& allocator)
            : forward(level, static_cast<Node*>(0), allocator), engaged(false) {}

        value_type* valuePtr() { return reinterpret_cast<value_type*>(&storage); }
        const value_type* valuePtr() const { return reinterpret_cast<const value_type*>(&storage); }
    };

    typedef std::allocator_traits<allocator_type> value_allocator_traits;
    typedef typename value_allocator_traits::template rebind_alloc<Node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

    struct KeyAccess {
        const key_type& operator()(const Node* node) const { return node->valuePtr()->first; }
    };

    struct ForwardAccess {
        Node* operator()(Node* node, size_type level) const {
            return level < node->forward.size() ? node->forward[level] : 0;
        }
    };

public:
    class const_iterator {
        friend class SkipList;
        const Node* node_;
        explicit const_iterator(const Node* node) : node_(node) {}
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename SkipList::value_type value_type;
        typedef typename SkipList::difference_type difference_type;
        typedef const value_type* pointer;
        typedef const value_type& reference;

        const_iterator() : node_(0) {}
        reference operator*() const { return *node_->valuePtr(); }
        pointer operator->() const { return node_->valuePtr(); }
        const_iterator& operator++() { node_ = node_->forward[0]; return *this; }
        const_iterator operator++(int) { const_iterator copy(*this); ++(*this); return copy; }
        friend bool operator==(const const_iterator& left, const const_iterator& right) {
            return left.node_ == right.node_;
        }
        friend bool operator!=(const const_iterator& left, const const_iterator& right) {
            return !(left == right);
        }
    };

    class iterator {
        friend class SkipList;
        Node* node_;
        explicit iterator(Node* node) : node_(node) {}
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename SkipList::value_type value_type;
        typedef typename SkipList::difference_type difference_type;
        typedef value_type* pointer;
        typedef value_type& reference;

        iterator() : node_(0) {}
        operator const_iterator() const { return const_iterator(node_); }
        reference operator*() const { return *node_->valuePtr(); }
        pointer operator->() const { return node_->valuePtr(); }
        iterator& operator++() { node_ = node_->forward[0]; return *this; }
        iterator operator++(int) { iterator copy(*this); ++(*this); return copy; }
        friend bool operator==(const iterator& left, const iterator& right) { return left.node_ == right.node_; }
        friend bool operator!=(const iterator& left, const iterator& right) { return !(left == right); }
    };

private:
    allocator_type allocator_;
    key_compare compare_;
    Engine engine_;
    double probability_;
    size_type maximum_level_;
    size_type level_count_;
    size_type size_;
    Node* head_;

    pointer_allocator_type pointerAllocator() const { return pointer_allocator_type(allocator_); }
    node_allocator_type nodeAllocator() const { return node_allocator_type(allocator_); }

    Node* createSentinel(size_type level) {
        node_allocator_type allocator = nodeAllocator();
        Node* node = node_allocator_traits::allocate(allocator, 1);
        try {
            node_allocator_traits::construct(allocator, node, level, pointerAllocator());
        } catch (...) {
            node_allocator_traits::deallocate(allocator, node, 1);
            throw;
        }
        return node;
    }

    template<typename M>
    Node* createValueNode(size_type level, const key_type& key, M&& value) {
        Node* node = createSentinel(level);
        try {
            value_allocator_traits::construct(
                allocator_, node->valuePtr(), key, std::forward<M>(value)
            );
            node->engaged = true;
        } catch (...) {
            destroyNode(node);
            throw;
        }
        return node;
    }

    void destroyNode(Node* node) {
        if (!node)
            return;
        if (node->engaged) {
            value_allocator_traits::destroy(allocator_, node->valuePtr());
            node->engaged = false;
        }
        node_allocator_type allocator = nodeAllocator();
        node_allocator_traits::destroy(allocator, node);
        node_allocator_traits::deallocate(allocator, node, 1);
    }

    bool equivalent(const key_type& left, const key_type& right) const {
        return !compare_(left, right) && !compare_(right, left);
    }

    Node* lowerBoundNode(const key_type& key, std::vector<Node*>* path = 0) const {
        return dsa::core::SkipListAlgorithm::lowerBoundPath(
            head_, level_count_, key, compare_, KeyAccess(), ForwardAccess(), path
        );
    }

    void copyFrom(const SkipList& other) {
        try {
            for (const_iterator it = other.begin(); it != other.end(); ++it)
                insert_or_assign(it->first, it->second);
        } catch (...) {
            clear();
            throw;
        }
    }

public:
    explicit SkipList(
        const key_compare& compare = key_compare(),
        const allocator_type& allocator = allocator_type(),
        size_type maximumLevel = 32,
        double probability = 0.5,
        typename Engine::result_type seed = Engine::default_seed
    ) : allocator_(allocator), compare_(compare), engine_(seed), probability_(probability),
        maximum_level_(maximumLevel), level_count_(1), size_(0), head_(0) {
        if (maximum_level_ == 0)
            throw std::invalid_argument("SkipList maximum level must be positive");
        if (!(probability_ > 0.0 && probability_ < 1.0))
            throw std::invalid_argument("SkipList probability must be in (0, 1)");
        head_ = createSentinel(maximum_level_);
    }

    SkipList(const SkipList& other)
        : allocator_(value_allocator_traits::select_on_container_copy_construction(other.allocator_)),
          compare_(other.compare_), engine_(other.engine_), probability_(other.probability_),
          maximum_level_(other.maximum_level_), level_count_(1), size_(0), head_(0) {
        head_ = createSentinel(maximum_level_);
        copyFrom(other);
    }

    // 移动构造先为源对象建立空哨兵，再交换所有权；源对象可继续插入和查询。
    SkipList(SkipList&& other)
        : SkipList(
            other.compare_, other.allocator_, other.maximum_level_,
            other.probability_, Engine::default_seed
        ) {
        swap(other);
    }

    ~SkipList() {
        clear();
        destroyNode(head_);
    }

    SkipList& operator=(const SkipList& other) {
        if (this != &other) {
            SkipList copy(other);
            swap(copy);
        }
        return *this;
    }

    // 移动赋值遵守 allocator 传播边界；不可传播且 allocator 不等时逐元素迁移。
    SkipList& operator=(SkipList&& other) {
        if (this == &other)
            return *this;

        typedef typename value_allocator_traits::propagate_on_container_move_assignment propagate;
        if (propagate::value || allocator_ == other.allocator_) {
            SkipList replacement(std::move(other));
            clear();
            destroyNode(head_);
            if (propagate::value)
                allocator_ = replacement.allocator_;
            compare_ = std::move(replacement.compare_);
            engine_ = std::move(replacement.engine_);
            probability_ = replacement.probability_;
            maximum_level_ = replacement.maximum_level_;
            level_count_ = replacement.level_count_;
            size_ = replacement.size_;
            head_ = replacement.head_;
            replacement.head_ = 0;
            replacement.level_count_ = 1;
            replacement.size_ = 0;
            return *this;
        }

        SkipList replacement(
            other.compare_, allocator_, other.maximum_level_,
            other.probability_, Engine::default_seed
        );
        replacement.engine_ = other.engine_;
        for (iterator it = other.begin(); it != other.end(); ++it)
            replacement.insert_or_assign(it->first, std::move(it->second));
        swap(replacement);
        other.clear();
        return *this;
    }

    iterator begin() { return iterator(head_ ? head_->forward[0] : 0); }
    const_iterator begin() const { return const_iterator(head_ ? head_->forward[0] : 0); }
    iterator end() { return iterator(0); }
    const_iterator end() const { return const_iterator(0); }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    size_type level() const { return level_count_; }
    allocator_type get_allocator() const { return allocator_; }

    iterator lower_bound(const key_type& key) { return iterator(lowerBoundNode(key)); }
    const_iterator lower_bound(const key_type& key) const { return const_iterator(lowerBoundNode(key)); }

    iterator find(const key_type& key) {
        Node* node = lowerBoundNode(key);
        return node && equivalent(node->valuePtr()->first, key) ? iterator(node) : end();
    }
    const_iterator find(const key_type& key) const {
        Node* node = lowerBoundNode(key);
        return node && equivalent(node->valuePtr()->first, key) ? const_iterator(node) : end();
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
        std::vector<Node*> path(maximum_level_, static_cast<Node*>(0));
        Node* candidate = lowerBoundNode(key, &path);
        if (candidate && equivalent(candidate->valuePtr()->first, key)) {
            candidate->valuePtr()->second = std::forward<M>(value);
            return std::make_pair(iterator(candidate), false);
        }

        const size_type nodeLevel = dsa::core::SkipListAlgorithm::randomLevel(
            engine_, probability_, maximum_level_
        );
        Node* node = createValueNode(nodeLevel, key, std::forward<M>(value));
        if (nodeLevel > level_count_) {
            for (size_type level = level_count_; level < nodeLevel; ++level)
                path[level] = head_;
        }
        for (size_type level = 0; level < nodeLevel; ++level) {
            node->forward[level] = path[level]->forward[level];
            path[level]->forward[level] = node;
        }
        if (nodeLevel > level_count_)
            level_count_ = nodeLevel;
        ++size_;
        return std::make_pair(iterator(node), true);
    }

    bool put(const key_type& key, const mapped_type& value) {
        return insert_or_assign(key, value).second;
    }

    size_type erase(const key_type& key) {
        std::vector<Node*> path(maximum_level_, static_cast<Node*>(0));
        Node* candidate = lowerBoundNode(key, &path);
        if (!candidate || !equivalent(candidate->valuePtr()->first, key))
            return 0;
        for (size_type level = 0; level < candidate->forward.size(); ++level) {
            if (path[level]->forward[level] == candidate)
                path[level]->forward[level] = candidate->forward[level];
        }
        destroyNode(candidate);
        --size_;
        while (level_count_ > 1 && !head_->forward[level_count_ - 1])
            --level_count_;
        return 1;
    }

    bool remove(const key_type& key) { return erase(key) != 0; }

    void clear() {
        if (!head_)
            return;
        Node* node = head_->forward[0];
        while (node) {
            Node* next = node->forward[0];
            destroyNode(node);
            node = next;
        }
        for (size_type level = 0; level < head_->forward.size(); ++level)
            head_->forward[level] = 0;
        size_ = 0;
        level_count_ = 1;
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
        size_type count = 0;
        const Node* previous = 0;
        for (const Node* node = head_->forward[0]; node; node = node->forward[0]) {
            if (!node->engaged || node->forward.empty())
                return false;
            if (previous && !compare_(previous->valuePtr()->first, node->valuePtr()->first))
                return false;
            previous = node;
            ++count;
        }
        if (count != size_)
            return false;
        for (size_type level = 1; level < level_count_; ++level) {
            const Node* lastNode = 0;
            for (const Node* node = head_->forward[level]; node; node = node->forward[level]) {
                if (node->forward.size() <= level)
                    return false;
                if (lastNode && !compare_(lastNode->valuePtr()->first, node->valuePtr()->first))
                    return false;
                lastNode = node;
            }
        }
        return true;
    }

    void swap(SkipList& other) {
        using std::swap;
        swap(allocator_, other.allocator_);
        swap(compare_, other.compare_);
        swap(engine_, other.engine_);
        swap(probability_, other.probability_);
        swap(maximum_level_, other.maximum_level_);
        swap(level_count_, other.level_count_);
        swap(size_, other.size_);
        swap(head_, other.head_);
    }
};

} // namespace container
} // namespace dsa

#endif
