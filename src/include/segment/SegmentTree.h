#ifndef __DSA_SEGMENT_TREE
#define __DSA_SEGMENT_TREE

#include "Vector.h"
#include "common.h"

// 示例运算：区间求和
template<typename T>
struct SegmentSum {
    T operator()(const T& a, const T& b) const { return a + b; }
};

// 基础线段树：支持区间查询/单点更新，运算由 Op 提供（需满足结合律）
// 构建: O(n)，查询/更新: O(log n)
template<typename T, typename Op = SegmentSum<T>>
class SegmentTree {
public:
    SegmentTree() : _n(0), _op(Op()), _identity(T()) {}

    SegmentTree(const Vector<T>& data, Op op, T identity)
        : _n(0), _op(op), _identity(identity) {
        build(data);
    }

    void build(const Vector<T>& data) {
        _n = 1;
        while (_n < data.size()) _n <<= 1;
        _tree = Vector<T>(_n << 1, _n << 1, _identity);
        for (int i = 0; i < data.size(); ++i) {
            _tree[_n + i] = data[i];
        }
        for (int i = _n - 1; i > 0; --i) {
            _tree[i] = _op(_tree[i << 1], _tree[i << 1 | 1]);
        }
    }

    // 0-indexed 单点赋值更新
    void update(int pos, const T& val) {
        int p = _n + pos;
        _tree[p] = val;
        while (p > 1) {
            p >>= 1;
            _tree[p] = _op(_tree[p << 1], _tree[p << 1 | 1]);
        }
    }

    // 区间查询 [l, r)
    T query(int l, int r) const {
        T res_left = _identity;
        T res_right = _identity;
        int L = _n + l;
        int R = _n + r;
        while (L < R) {
            if (L & 1) res_left = _op(res_left, _tree[L++]);
            if (R & 1) res_right = _op(_tree[--R], res_right);
            L >>= 1;
            R >>= 1;
        }
        return _op(res_left, res_right);
    }

    int length() const { return _n; }

private:
    int _n;          // 叶子数量（为原始长度向上取整的 2 次幂）
    Op _op;          // 结合运算
    T _identity;     // 幺元
    Vector<T> _tree; // 1-based 存储
};

#endif
