#ifndef __DSA_SEGMENT_TREE
#define __DSA_SEGMENT_TREE

#include "Vector.h"
#include "common.h"
#include <dsa/core/segment/SegmentTreeAlgorithm.h>

// 示例运算：区间求和。
template<typename T>
struct SegmentSum {
    T operator()(const T& a, const T& b) const { return a + b; }
};

// 教学版线段树：保留 Vector 存储和 length() 的历史语义，结构流程复用共享算法。
template<typename T, typename Op = SegmentSum<T> >
class SegmentTree {
private:
    class Access {
    public:
        typedef T value_type;
        typedef std::size_t size_type;

        explicit Access(SegmentTree& tree) : tree_(tree) {}
        explicit Access(const SegmentTree& tree)
            : tree_(const_cast<SegmentTree&>(tree)) {}

        void reset(size_type count, size_type leaves, const value_type& identity) {
            tree_._size = static_cast<int>(count);
            tree_._n = static_cast<int>(leaves);
            tree_._tree = Vector<T>(
                static_cast<int>(leaves << 1),
                static_cast<int>(leaves << 1),
                identity
            );
        }

        void assign(size_type index, const value_type& value) {
            tree_._tree[static_cast<int>(index)] = value;
        }

        const value_type& value(size_type index) const {
            return tree_._tree[static_cast<int>(index)];
        }

        size_type valueCount() const { return static_cast<size_type>(tree_._size); }
        size_type leafCount() const { return static_cast<size_type>(tree_._n); }

    private:
        SegmentTree& tree_;
    };

    typedef dsa::core::SegmentTreeAlgorithm<Access> Algorithm;

public:
    SegmentTree() : _n(1), _size(0), _op(Op()), _identity(T()), _tree(2, 2, T()) {}

    SegmentTree(const Vector<T>& data, Op op, T identity)
        : _n(1), _size(0), _op(op), _identity(identity), _tree() {
        build(data);
    }

    void build(const Vector<T>& data) {
        Access access(*this);
        Algorithm::build(access, data.begin(), data.end(), _op, _identity);
    }

    void update(int pos, const T& val) {
        Access access(*this);
        Algorithm::update(access, static_cast<std::size_t>(pos), val, _op);
    }

    T query(int l, int r) const {
        if (l < 0 || r < 0)
            throw std::out_of_range("SegmentTree::query range out of range");
        const Access access(*this);
        return Algorithm::query(
            access,
            static_cast<std::size_t>(l),
            static_cast<std::size_t>(r),
            _op,
            _identity
        );
    }

    int length() const { return _n; }
    int size() const { return _size; }

private:
    int _n;
    int _size;
    Op _op;
    T _identity;
    Vector<T> _tree;
};

#endif
