#ifndef DSA_UF_WEIGHTED_QUICK_UNION_H
#define DSA_UF_WEIGHTED_QUICK_UNION_H

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <memory>
#include <utility>

#include "UnionFind.h"
#include "../dsa/core/uf/UnionFindAlgorithm.h"

/// 教学版按大小合并 Quick-Union，树高保持在 O(log N)。
class WeightedQuickUnion : public UnionFind {
protected:
    std::unique_ptr<int[]> _sizeStorage;
    int* _sz;

    /// 把教学版父指针与树大小元数据适配到共享 UnionFindAlgorithm。
    class Storage {
    public:
        typedef int index_type;
        typedef std::size_t size_type;

        /// 绑定当前 WeightedQuickUnion 实例。
        explicit Storage(WeightedQuickUnion& owner) noexcept;

        /// 检查教学索引合法性。
        void validate(index_type index) const;

        /// 返回节点的父节点。
        index_type parentOf(index_type index) const noexcept;

        /// 将子根或路径节点链接到新的父节点。
        void linkRoot(index_type child, index_type parent) noexcept;

        /// 返回根节点维护的组件大小。
        size_type componentSize(index_type root) const noexcept;

        /// 把 childRoot 的组件大小合并到 parentRoot。
        void absorbComponent(index_type parentRoot, index_type childRoot) noexcept;

        /// 在父指针和大小元数据全部更新后提交组件数量变化。
        void commitMerge() noexcept;

    private:
        WeightedQuickUnion& owner_;
    };

    typedef dsa::core::UnionFindAlgorithm<Storage> Algorithm;

    /// 初始化每棵单节点树的大小为 1。
    void resetSizes(int N);

public:
    WeightedQuickUnion() = delete;

    /// 构造 N 棵带大小元数据的单节点树。
    WeightedQuickUnion(int N);

    /// 从文件流读取 N 和连接对，保留原教学构造入口。
    WeightedQuickUnion(std::ifstream& input);

    /// 深拷贝父指针和树大小数组。
    WeightedQuickUnion(const WeightedQuickUnion& other);

    /// 深拷贝赋值父指针和树大小数组。
    WeightedQuickUnion& operator=(const WeightedQuickUnion& other);

    /// 转移父指针和树大小数组所有权。
    WeightedQuickUnion(WeightedQuickUnion&& other) noexcept;

    /// 转移赋值父指针和树大小数组所有权。
    WeightedQuickUnion& operator=(WeightedQuickUnion&& other) noexcept;

    /// 由 RAII 成员统一释放两组数组。
    ~WeightedQuickUnion() override = default;

    /// 沿父指针查找根节点，复杂度为 O(log N)。
    int find(int p) override;

    /// 按组件大小把较小树链接到较大树。
    void unite(int p, int q) override;
};

inline WeightedQuickUnion::Storage::Storage(WeightedQuickUnion& owner) noexcept
    : owner_(owner) {}

inline void WeightedQuickUnion::Storage::validate(index_type index) const {
    owner_.validateIndex(index);
}

inline WeightedQuickUnion::Storage::index_type
WeightedQuickUnion::Storage::parentOf(index_type index) const noexcept {
    return owner_._id[index];
}

inline void WeightedQuickUnion::Storage::linkRoot(
    index_type child,
    index_type parent
) noexcept {
    owner_._id[child] = parent;
}

inline WeightedQuickUnion::Storage::size_type
WeightedQuickUnion::Storage::componentSize(index_type root) const noexcept {
    return static_cast<size_type>(owner_._sz[root]);
}

inline void WeightedQuickUnion::Storage::absorbComponent(
    index_type parentRoot,
    index_type childRoot
) noexcept {
    owner_._sz[parentRoot] += owner_._sz[childRoot];
}

inline void WeightedQuickUnion::Storage::commitMerge() noexcept {
    --owner_._count;
}

inline void WeightedQuickUnion::resetSizes(int N) {
    std::unique_ptr<int[]> replacement;
    if (N > 0) {
        replacement.reset(new int[static_cast<std::size_t>(N)]);
        std::fill(replacement.get(), replacement.get() + N, 1);
    }

    _sizeStorage = std::move(replacement);
    _sz = _sizeStorage.get();
}

inline WeightedQuickUnion::WeightedQuickUnion(int N)
    : UnionFind(N), _sizeStorage(), _sz(nullptr) {
    resetSizes(N);
}

inline WeightedQuickUnion::WeightedQuickUnion(std::ifstream& input)
    : WeightedQuickUnion(readElementCount(input)) {
    loadConnections(input);
    std::printf("%d components\n", count());
}

inline WeightedQuickUnion::WeightedQuickUnion(const WeightedQuickUnion& other)
    : UnionFind(other), _sizeStorage(), _sz(nullptr) {
    if (_N > 0) {
        _sizeStorage.reset(new int[static_cast<std::size_t>(_N)]);
        std::copy(other._sz, other._sz + _N, _sizeStorage.get());
        _sz = _sizeStorage.get();
    }
}

inline WeightedQuickUnion& WeightedQuickUnion::operator=(
    const WeightedQuickUnion& other
) {
    if (this == &other)
        return *this;

    std::unique_ptr<int[]> replacement;
    if (other._N > 0) {
        replacement.reset(new int[static_cast<std::size_t>(other._N)]);
        std::copy(other._sz, other._sz + other._N, replacement.get());
    }

    UnionFind::operator=(other);
    _sizeStorage = std::move(replacement);
    _sz = _sizeStorage.get();
    return *this;
}

inline WeightedQuickUnion::WeightedQuickUnion(
    WeightedQuickUnion&& other
) noexcept
    : UnionFind(std::move(other)),
      _sizeStorage(std::move(other._sizeStorage)),
      _sz(_sizeStorage.get()) {
    other._sz = nullptr;
}

inline WeightedQuickUnion& WeightedQuickUnion::operator=(
    WeightedQuickUnion&& other
) noexcept {
    if (this == &other)
        return *this;

    UnionFind::operator=(std::move(other));
    _sizeStorage = std::move(other._sizeStorage);
    _sz = _sizeStorage.get();
    other._sz = nullptr;
    return *this;
}

inline int WeightedQuickUnion::find(int p) {
    Storage storage(*this);
    return Algorithm::findRoot(storage, p);
}

inline void WeightedQuickUnion::unite(int p, int q) {
    const int pRoot = find(p);
    const int qRoot = find(q);
    Storage storage(*this);
    (void)Algorithm::linkRootsBySize(storage, pRoot, qRoot);
}

#endif
