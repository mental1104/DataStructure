#ifndef DSA_RB_NODE_TRAITS_HPP
#define DSA_RB_NODE_TRAITS_HPP

#include "rb_color.hpp"

namespace dsa {
namespace rb {

template <typename Node>
struct rb_node_traits;

template <typename Node>
struct intrusive_node_traits {
    typedef Node node_type;

    static node_type*& parent(node_type* node) { return node->parent; }
    static node_type*& left(node_type* node) { return node->left; }
    static node_type*& right(node_type* node) { return node->right; }

    static color get_color(const node_type* node) {
        return node ? node->color : color::black;
    }

    static void set_color(node_type* node, color value) {
        if (node) {
            node->color = value;
        }
    }
};

}  // namespace rb
}  // namespace dsa

#endif  // DSA_RB_NODE_TRAITS_HPP
