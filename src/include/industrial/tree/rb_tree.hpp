#ifndef DSA_INDUSTRIAL_TREE_RB_TREE_HPP
#define DSA_INDUSTRIAL_TREE_RB_TREE_HPP

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include "../../rb/rb_algorithm.hpp"

namespace dsa {
namespace industrial {

template<
    class T,
    class Compare = std::less<T>,
    class Allocator = std::allocator<T>
>
class rb_tree {
public:
    typedef T value_type;
    typedef Compare value_compare;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

private:
    struct node {
        template <class... Args>
        explicit node(Args&&... args)
            : value(std::forward<Args>(args)...),
              parent(0),
              left(0),
              right(0),
              color(dsa::rb::color::red) {}

        value_type value;
        node* parent;
        node* left;
        node* right;
        dsa::rb::color color;
    };

    struct node_traits {
        typedef node node_type;

        static node_type*& parent(node_type* n) { return n->parent; }
        static node_type*& left(node_type* n) { return n->left; }
        static node_type*& right(node_type* n) { return n->right; }

        static dsa::rb::color get_color(const node_type* n) {
            return n ? n->color : dsa::rb::color::black;
        }

        static void set_color(node_type* n, dsa::rb::color color) {
            if (n) {
                n->color = color;
            }
        }
    };

    typedef typename std::allocator_traits<allocator_type>
        ::template rebind_alloc<node> node_allocator_type;
    typedef std::allocator_traits<node_allocator_type> node_allocator_traits;

public:
    class const_iterator;

    class iterator {
        friend class rb_tree;
        friend class const_iterator;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef typename rb_tree::value_type value_type;
        typedef typename rb_tree::difference_type difference_type;
        typedef typename rb_tree::pointer pointer;
        typedef typename rb_tree::reference reference;

        iterator() : node_(0), tree_(0) {}

        reference operator*() const { return node_->value; }
        pointer operator->() const { return &node_->value; }

        iterator& operator++() {
            node_ = dsa::rb::successor<node, node_traits>(node_);
            return *this;
        }

        iterator operator++(int) {
            iterator old(*this);
            ++(*this);
            return old;
        }

        iterator& operator--() {
            node_ = node_
                ? dsa::rb::predecessor<node, node_traits>(node_)
                : dsa::rb::maximum<node, node_traits>(tree_->root_);
            return *this;
        }

        iterator operator--(int) {
            iterator old(*this);
            --(*this);
            return old;
        }

        bool operator==(const iterator& rhs) const { return node_ == rhs.node_; }
        bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

    private:
        iterator(node* n, rb_tree* tree) : node_(n), tree_(tree) {}

        node* node_;
        rb_tree* tree_;
    };

    class const_iterator {
        friend class rb_tree;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef typename rb_tree::value_type value_type;
        typedef typename rb_tree::difference_type difference_type;
        typedef typename rb_tree::const_pointer pointer;
        typedef typename rb_tree::const_reference reference;

        const_iterator() : node_(0), tree_(0) {}
        const_iterator(const iterator& it) : node_(it.node_), tree_(it.tree_) {}

        reference operator*() const { return node_->value; }
        pointer operator->() const { return &node_->value; }

        const_iterator& operator++() {
            node_ = dsa::rb::successor<node, node_traits>(node_);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator old(*this);
            ++(*this);
            return old;
        }

        const_iterator& operator--() {
            node_ = node_
                ? dsa::rb::predecessor<node, node_traits>(node_)
                : dsa::rb::maximum<node, node_traits>(tree_->root_);
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator old(*this);
            --(*this);
            return old;
        }

        bool operator==(const const_iterator& rhs) const { return node_ == rhs.node_; }
        bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

    private:
        const_iterator(node* n, const rb_tree* tree) : node_(n), tree_(tree) {}

        node* node_;
        const rb_tree* tree_;
    };

    explicit rb_tree(
        const value_compare& comp = value_compare(),
        const allocator_type& alloc = allocator_type())
        : root_(0),
          size_(0),
          comp_(comp),
          alloc_(alloc) {}

    rb_tree(rb_tree&& other)
        : root_(other.root_),
          size_(other.size_),
          comp_(std::move(other.comp_)),
          alloc_(std::move(other.alloc_)) {
        other.root_ = 0;
        other.size_ = 0;
    }

    rb_tree& operator=(rb_tree&& other) {
        if (this != &other) {
            clear();
            root_ = other.root_;
            size_ = other.size_;
            comp_ = std::move(other.comp_);
            alloc_ = std::move(other.alloc_);
            other.root_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    rb_tree(const rb_tree&) = delete;
    rb_tree& operator=(const rb_tree&) = delete;

    ~rb_tree() { clear(); }

    allocator_type get_allocator() const { return allocator_type(alloc_); }
    value_compare value_comp() const { return comp_; }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }

    iterator begin() {
        return iterator(dsa::rb::minimum<node, node_traits>(root_), this);
    }

    const_iterator begin() const {
        return const_iterator(dsa::rb::minimum<node, node_traits>(root_), this);
    }

    const_iterator cbegin() const { return begin(); }

    iterator end() { return iterator(0, this); }
    const_iterator end() const { return const_iterator(0, this); }
    const_iterator cend() const { return end(); }

    std::pair<iterator, bool> insert(const value_type& value) {
        return insert_constructed_node(create_node(value));
    }

    std::pair<iterator, bool> insert(value_type&& value) {
        return insert_constructed_node(create_node(std::move(value)));
    }

    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return insert_constructed_node(create_node(std::forward<Args>(args)...));
    }

    iterator find(const value_type& value) {
        return iterator(find_node(value), this);
    }

    const_iterator find(const value_type& value) const {
        return const_iterator(find_node(value), this);
    }

    template <class Key, class KeyCompare>
    iterator find_as(const Key& key, KeyCompare key_compare) {
        return iterator(find_node_as(key, key_compare), this);
    }

    template <class Key, class KeyCompare>
    const_iterator find_as(const Key& key, KeyCompare key_compare) const {
        return const_iterator(find_node_as(key, key_compare), this);
    }

    size_type erase(const value_type& value) {
        node* found = find_node(value);
        if (!found) {
            return 0;
        }
        erase_node(found);
        return 1;
    }

    iterator erase(iterator pos) {
        node* current = pos.node_;
        if (!current) {
            return end();
        }
        node* next = dsa::rb::successor<node, node_traits>(current);
        erase_node(current);
        return iterator(next, this);
    }

    void clear() {
        destroy_subtree(root_);
        root_ = 0;
        size_ = 0;
    }

    void swap(rb_tree& other) {
        using std::swap;
        swap(root_, other.root_);
        swap(size_, other.size_);
        swap(comp_, other.comp_);
        swap(alloc_, other.alloc_);
    }

private:
    template <class... Args>
    node* create_node(Args&&... args) {
        node* allocated = node_allocator_traits::allocate(alloc_, 1);
        try {
            node_allocator_traits::construct(
                alloc_, allocated, std::forward<Args>(args)...);
        } catch (...) {
            node_allocator_traits::deallocate(alloc_, allocated, 1);
            throw;
        }
        return allocated;
    }

    void destroy_node(node* n) {
        node_allocator_traits::destroy(alloc_, n);
        node_allocator_traits::deallocate(alloc_, n, 1);
    }

    void destroy_subtree(node* n) {
        if (!n) {
            return;
        }
        destroy_subtree(n->left);
        destroy_subtree(n->right);
        destroy_node(n);
    }

    std::pair<iterator, bool> insert_constructed_node(node* candidate) {
        node* parent = 0;
        node* current = root_;
        bool attach_left = false;

        try {
            while (current) {
                parent = current;
                if (comp_(candidate->value, current->value)) {
                    attach_left = true;
                    current = current->left;
                } else if (comp_(current->value, candidate->value)) {
                    attach_left = false;
                    current = current->right;
                } else {
                    iterator existing(current, this);
                    destroy_node(candidate);
                    return std::make_pair(existing, false);
                }
            }
        } catch (...) {
            destroy_node(candidate);
            throw;
        }

        candidate->parent = parent;
        candidate->left = 0;
        candidate->right = 0;
        candidate->color = dsa::rb::color::red;

        if (!parent) {
            root_ = candidate;
        } else if (attach_left) {
            parent->left = candidate;
        } else {
            parent->right = candidate;
        }

        dsa::rb::insert_fixup<node, node_traits>(root_, candidate);
        ++size_;
        return std::make_pair(iterator(candidate, this), true);
    }

    node* find_node(const value_type& value) const {
        node* current = root_;
        while (current) {
            if (comp_(current->value, value)) {
                current = current->right;
            } else if (comp_(value, current->value)) {
                current = current->left;
            } else {
                return current;
            }
        }
        return 0;
    }

    template <class Key, class KeyCompare>
    node* find_node_as(const Key& key, KeyCompare key_compare) const {
        node* current = root_;
        while (current) {
            if (key_compare(current->value, key)) {
                current = current->right;
            } else if (key_compare(key, current->value)) {
                current = current->left;
            } else {
                return current;
            }
        }
        return 0;
    }

    void transplant(node* target, node* replacement) {
        if (!target->parent) {
            root_ = replacement;
        } else if (target == target->parent->left) {
            target->parent->left = replacement;
        } else {
            target->parent->right = replacement;
        }
        if (replacement) {
            replacement->parent = target->parent;
        }
    }

    void erase_node(node* target) {
        node* moved = target;
        node* fixup_node = 0;
        node* fixup_parent = 0;
        dsa::rb::color moved_original_color = moved->color;

        if (!target->left) {
            fixup_node = target->right;
            fixup_parent = target->parent;
            transplant(target, target->right);
        } else if (!target->right) {
            fixup_node = target->left;
            fixup_parent = target->parent;
            transplant(target, target->left);
        } else {
            moved = dsa::rb::minimum<node, node_traits>(target->right);
            moved_original_color = moved->color;
            fixup_node = moved->right;

            if (moved->parent == target) {
                fixup_parent = moved;
                if (fixup_node) {
                    fixup_node->parent = moved;
                }
            } else {
                fixup_parent = moved->parent;
                transplant(moved, moved->right);
                moved->right = target->right;
                moved->right->parent = moved;
            }

            transplant(target, moved);
            moved->left = target->left;
            moved->left->parent = moved;
            moved->color = target->color;
        }

        destroy_node(target);
        --size_;

        if (moved_original_color == dsa::rb::color::black) {
            dsa::rb::erase_fixup<node, node_traits>(root_, fixup_node, fixup_parent);
        }
    }

    node* root_;
    size_type size_;
    value_compare comp_;
    node_allocator_type alloc_;
};

template <class T, class Compare, class Allocator>
void swap(rb_tree<T, Compare, Allocator>& lhs, rb_tree<T, Compare, Allocator>& rhs) {
    lhs.swap(rhs);
}

}  // namespace industrial
}  // namespace dsa

#endif  // DSA_INDUSTRIAL_TREE_RB_TREE_HPP
