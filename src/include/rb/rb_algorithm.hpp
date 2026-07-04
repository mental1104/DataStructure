#ifndef DSA_RB_ALGORITHM_HPP
#define DSA_RB_ALGORITHM_HPP

#include "rb_node_traits.hpp"

namespace dsa {
namespace rb {

template <typename Node, typename Traits>
inline bool is_red(const Node* node) {
    return Traits::get_color(node) == color::red;
}

template <typename Node, typename Traits>
inline bool is_black(const Node* node) {
    return !is_red<Node, Traits>(node);
}

template <typename Node, typename Traits>
inline bool is_root(Node* node) {
    return node && !Traits::parent(node);
}

template <typename Node, typename Traits>
inline bool is_left_child(Node* node) {
    return node &&
           Traits::parent(node) &&
           node == Traits::left(Traits::parent(node));
}

template <typename Node, typename Traits>
inline bool is_right_child(Node* node) {
    return node &&
           Traits::parent(node) &&
           node == Traits::right(Traits::parent(node));
}

template <typename Node, typename Traits>
inline Node* left_of(Node* node) {
    return node ? Traits::left(node) : 0;
}

template <typename Node, typename Traits>
inline Node* right_of(Node* node) {
    return node ? Traits::right(node) : 0;
}

template <typename Node, typename Traits>
Node* minimum(Node* node) {
    while (node && Traits::left(node)) {
        node = Traits::left(node);
    }
    return node;
}

template <typename Node, typename Traits>
Node* maximum(Node* node) {
    while (node && Traits::right(node)) {
        node = Traits::right(node);
    }
    return node;
}

template <typename Node, typename Traits>
Node* successor(Node* node) {
    if (!node) {
        return 0;
    }
    if (Traits::right(node)) {
        return minimum<Node, Traits>(Traits::right(node));
    }

    Node* parent = Traits::parent(node);
    while (parent && node == Traits::right(parent)) {
        node = parent;
        parent = Traits::parent(parent);
    }
    return parent;
}

template <typename Node, typename Traits>
Node* predecessor(Node* node) {
    if (!node) {
        return 0;
    }
    if (Traits::left(node)) {
        return maximum<Node, Traits>(Traits::left(node));
    }

    Node* parent = Traits::parent(node);
    while (parent && node == Traits::left(parent)) {
        node = parent;
        parent = Traits::parent(parent);
    }
    return parent;
}

template <typename Node, typename Traits>
Node* rotate_left(Node*& root, Node* node) {
    Node* pivot = Traits::right(node);
    if (!pivot) {
        return node;
    }

    Traits::right(node) = Traits::left(pivot);
    if (Traits::left(pivot)) {
        Traits::parent(Traits::left(pivot)) = node;
    }

    Traits::parent(pivot) = Traits::parent(node);
    if (!Traits::parent(node)) {
        root = pivot;
    } else if (node == Traits::left(Traits::parent(node))) {
        Traits::left(Traits::parent(node)) = pivot;
    } else {
        Traits::right(Traits::parent(node)) = pivot;
    }

    Traits::left(pivot) = node;
    Traits::parent(node) = pivot;
    Traits::update_height(node);
    Traits::update_height(pivot);
    return pivot;
}

template <typename Node, typename Traits>
Node* rotate_right(Node*& root, Node* node) {
    Node* pivot = Traits::left(node);
    if (!pivot) {
        return node;
    }

    Traits::left(node) = Traits::right(pivot);
    if (Traits::right(pivot)) {
        Traits::parent(Traits::right(pivot)) = node;
    }

    Traits::parent(pivot) = Traits::parent(node);
    if (!Traits::parent(node)) {
        root = pivot;
    } else if (node == Traits::right(Traits::parent(node))) {
        Traits::right(Traits::parent(node)) = pivot;
    } else {
        Traits::left(Traits::parent(node)) = pivot;
    }

    Traits::right(pivot) = node;
    Traits::parent(node) = pivot;
    Traits::update_height(node);
    Traits::update_height(pivot);
    return pivot;
}

template <typename Node, typename Traits>
Node* uncle(Node* node) {
    Node* parent = node ? Traits::parent(node) : 0;
    Node* grand = parent ? Traits::parent(parent) : 0;
    if (!parent || !grand) {
        return 0;
    }
    return parent == Traits::left(grand)
        ? Traits::right(grand)
        : Traits::left(grand);
}

template <typename Node, typename Traits>
Node* rotate_at(Node*& root, Node* node) {
    Node* parent = node ? Traits::parent(node) : 0;
    Node* grand = parent ? Traits::parent(parent) : 0;
    if (!parent || !grand) {
        return node;
    }

    if (is_left_child<Node, Traits>(parent)) {
        if (is_left_child<Node, Traits>(node)) {
            return rotate_right<Node, Traits>(root, grand);
        }
        rotate_left<Node, Traits>(root, parent);
        return rotate_right<Node, Traits>(root, grand);
    }

    if (is_right_child<Node, Traits>(node)) {
        return rotate_left<Node, Traits>(root, grand);
    }
    rotate_right<Node, Traits>(root, parent);
    return rotate_left<Node, Traits>(root, grand);
}

template <typename Node, typename Traits>
void solve_double_red(Node*& root, Node* node) {
    if (!node) {
        return;
    }

    if (node == root || is_root<Node, Traits>(node)) {
        root = node;
        Traits::set_color(root, color::black);
        Traits::increase_height(root);
        Traits::parent(root) = 0;
        return;
    }

    Node* parent = Traits::parent(node);
    if (is_black<Node, Traits>(parent)) {
        return;
    }

    Node* grand = Traits::parent(parent);
    if (!grand) {
        Traits::set_color(parent, color::black);
        root = parent;
        Traits::parent(root) = 0;
        return;
    }

    Node* node_uncle = uncle<Node, Traits>(node);
    if (is_black<Node, Traits>(node_uncle)) {
        if (is_left_child<Node, Traits>(node) ==
            is_left_child<Node, Traits>(parent)) {
            Traits::set_color(parent, color::black);
        } else {
            Traits::set_color(node, color::black);
        }
        Traits::set_color(grand, color::red);
        rotate_at<Node, Traits>(root, node);
        return;
    }

    Traits::set_color(parent, color::black);
    Traits::increase_height(parent);
    Traits::set_color(node_uncle, color::black);
    Traits::increase_height(node_uncle);
    if (!is_root<Node, Traits>(grand)) {
        Traits::set_color(grand, color::red);
    }
    solve_double_red<Node, Traits>(root, grand);
}

template <typename Node, typename Traits>
void insert_fixup(Node*& root, Node* node) {
    Traits::set_color(node, color::red);
    solve_double_red<Node, Traits>(root, node);

    if (root) {
        Traits::set_color(root, color::black);
        Traits::parent(root) = 0;
    }
    Traits::recompute_black_height(root);
}

template <typename Node, typename Traits>
void solve_double_black(Node*& root, Node* node, Node* parent) {
    if (!parent) {
        return;
    }

    Node* sibling = (node == Traits::left(parent))
        ? Traits::right(parent)
        : Traits::left(parent);

    if (!sibling) {
        if (is_red<Node, Traits>(parent)) {
            Traits::set_color(parent, color::black);
        } else {
            Traits::decrease_height(parent);
            solve_double_black<Node, Traits>(root, parent, Traits::parent(parent));
        }
        return;
    }

    if (is_black<Node, Traits>(sibling)) {
        Node* red_child = 0;
        if (is_red<Node, Traits>(Traits::right(sibling))) {
            red_child = Traits::right(sibling);
        }
        if (is_red<Node, Traits>(Traits::left(sibling))) {
            red_child = Traits::left(sibling);
        }

        if (red_child) {
            color old_color = Traits::get_color(parent);
            Node* balanced = rotate_at<Node, Traits>(root, red_child);
            if (Traits::left(balanced)) {
                Traits::set_color(Traits::left(balanced), color::black);
                Traits::update_height(Traits::left(balanced));
            }
            if (Traits::right(balanced)) {
                Traits::set_color(Traits::right(balanced), color::black);
                Traits::update_height(Traits::right(balanced));
            }
            Traits::set_color(balanced, old_color);
            Traits::update_height(balanced);
            return;
        }

        Traits::set_color(sibling, color::red);
        Traits::decrease_height(sibling);
        if (is_red<Node, Traits>(parent)) {
            Traits::set_color(parent, color::black);
        } else {
            Traits::decrease_height(parent);
            solve_double_black<Node, Traits>(root, parent, Traits::parent(parent));
        }
        return;
    }

    Traits::set_color(sibling, color::black);
    Traits::set_color(parent, color::red);
    if (sibling == Traits::left(parent)) {
        rotate_right<Node, Traits>(root, parent);
    } else {
        rotate_left<Node, Traits>(root, parent);
    }
    solve_double_black<Node, Traits>(root, node, parent);
}

template <typename Node, typename Traits>
void erase_fixup(Node*& root, Node* node, Node* parent) {
    if (node == root || is_red<Node, Traits>(node)) {
        Traits::set_color(node, color::black);
    } else {
        solve_double_black<Node, Traits>(root, node, parent);
        if (node) {
            Traits::set_color(node, color::black);
        }
    }

    if (root) {
        Traits::parent(root) = 0;
    }
    Traits::recompute_black_height(root);
}

}  // namespace rb
}  // namespace dsa

#endif  // DSA_RB_ALGORITHM_HPP
