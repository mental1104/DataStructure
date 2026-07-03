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
    return pivot;
}

template <typename Node, typename Traits>
void insert_fixup(Node*& root, Node* node) {
    Traits::set_color(node, color::red);

    while (node != root && is_red<Node, Traits>(Traits::parent(node))) {
        Node* parent = Traits::parent(node);
        Node* grand = Traits::parent(parent);
        if (!grand) {
            break;
        }

        if (parent == Traits::left(grand)) {
            Node* uncle = Traits::right(grand);
            if (is_red<Node, Traits>(uncle)) {
                Traits::set_color(parent, color::black);
                Traits::set_color(uncle, color::black);
                Traits::set_color(grand, color::red);
                node = grand;
            } else {
                if (node == Traits::right(parent)) {
                    node = parent;
                    rotate_left<Node, Traits>(root, node);
                    parent = Traits::parent(node);
                    grand = parent ? Traits::parent(parent) : 0;
                }
                Traits::set_color(parent, color::black);
                Traits::set_color(grand, color::red);
                if (grand) {
                    rotate_right<Node, Traits>(root, grand);
                }
            }
        } else {
            Node* uncle = Traits::left(grand);
            if (is_red<Node, Traits>(uncle)) {
                Traits::set_color(parent, color::black);
                Traits::set_color(uncle, color::black);
                Traits::set_color(grand, color::red);
                node = grand;
            } else {
                if (node == Traits::left(parent)) {
                    node = parent;
                    rotate_right<Node, Traits>(root, node);
                    parent = Traits::parent(node);
                    grand = parent ? Traits::parent(parent) : 0;
                }
                Traits::set_color(parent, color::black);
                Traits::set_color(grand, color::red);
                if (grand) {
                    rotate_left<Node, Traits>(root, grand);
                }
            }
        }
    }

    if (root) {
        Traits::set_color(root, color::black);
        Traits::parent(root) = 0;
    }
}

template <typename Node, typename Traits>
void erase_fixup(Node*& root, Node* node, Node* parent) {
    while (node != root && is_black<Node, Traits>(node)) {
        if (!parent) {
            break;
        }

        if (node == Traits::left(parent)) {
            Node* sibling = Traits::right(parent);
            if (is_red<Node, Traits>(sibling)) {
                Traits::set_color(sibling, color::black);
                Traits::set_color(parent, color::red);
                rotate_left<Node, Traits>(root, parent);
                sibling = Traits::right(parent);
            }

            if (is_black<Node, Traits>(left_of<Node, Traits>(sibling)) &&
                is_black<Node, Traits>(right_of<Node, Traits>(sibling))) {
                Traits::set_color(sibling, color::red);
                node = parent;
                parent = Traits::parent(node);
            } else {
                if (is_black<Node, Traits>(right_of<Node, Traits>(sibling))) {
                    Traits::set_color(left_of<Node, Traits>(sibling), color::black);
                    Traits::set_color(sibling, color::red);
                    rotate_right<Node, Traits>(root, sibling);
                    sibling = Traits::right(parent);
                }
                Traits::set_color(sibling, Traits::get_color(parent));
                Traits::set_color(parent, color::black);
                Traits::set_color(right_of<Node, Traits>(sibling), color::black);
                rotate_left<Node, Traits>(root, parent);
                node = root;
                parent = 0;
            }
        } else {
            Node* sibling = Traits::left(parent);
            if (is_red<Node, Traits>(sibling)) {
                Traits::set_color(sibling, color::black);
                Traits::set_color(parent, color::red);
                rotate_right<Node, Traits>(root, parent);
                sibling = Traits::left(parent);
            }

            if (is_black<Node, Traits>(right_of<Node, Traits>(sibling)) &&
                is_black<Node, Traits>(left_of<Node, Traits>(sibling))) {
                Traits::set_color(sibling, color::red);
                node = parent;
                parent = Traits::parent(node);
            } else {
                if (is_black<Node, Traits>(left_of<Node, Traits>(sibling))) {
                    Traits::set_color(right_of<Node, Traits>(sibling), color::black);
                    Traits::set_color(sibling, color::red);
                    rotate_left<Node, Traits>(root, sibling);
                    sibling = Traits::left(parent);
                }
                Traits::set_color(sibling, Traits::get_color(parent));
                Traits::set_color(parent, color::black);
                Traits::set_color(left_of<Node, Traits>(sibling), color::black);
                rotate_right<Node, Traits>(root, parent);
                node = root;
                parent = 0;
            }
        }
    }

    Traits::set_color(node, color::black);
    if (root) {
        Traits::parent(root) = 0;
    }
}

}  // namespace rb
}  // namespace dsa

#endif  // DSA_RB_ALGORITHM_HPP
