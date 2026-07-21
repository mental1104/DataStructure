#ifndef DSA_UF_QUICK_UNION_H
#define DSA_UF_QUICK_UNION_H

#include <cstddef>
#include <fstream>

#include "UnionFind.h"
#include "../dsa/core/uf/UnionFindAlgorithm.h"

/// 教学版 Quick-Union：使用父指针森林表示连通分量。
class QuickUnion : public UnionFind {
protected:
    /// 把教学版父指针数组适配到共享 UnionFindAlgorithm。
    class Storage {
    public:
        typedef int index_type;
        typedef std::size_t size_type;

        /// 绑定当前 QuickUnion 实例。
        explicit Storage(QuickUnion& owner) noexcept;

        /// 检查教学索引合法性。
        void validate(index_type index) const;

        /// 返回节点的父节点。
        index_type parentOf(index_type index) const noexcept;

        /// 将子根或路径节点链接到新的父节点。
        void linkRoot(index_type child, index_type parent) noexcept;

        /// 在父指针修改完成后提交组件数量变化。
        void commitMerge() noexcept;

    private:
        QuickUnion& owner_;
    };

    typedef dsa::core::UnionFindAlgorithm<Storage> Algorithm;

public:
    QuickUnion() = delete;

    /// 构造 N 棵单节点树。
    QuickUnion(int N);

    /// 从文件流读取 N 和连接对，保留原教学构造入口。
    QuickUnion(std::ifstream& input);

    /// 复用基类深拷贝语义。
    QuickUnion(const QuickUnion& other) = default;

    /// 复用基类深拷贝赋值语义。
    QuickUnion& operator=(const QuickUnion& other) = default;

    /// 复用基类移动语义。
    QuickUnion(QuickUnion&& other) noexcept = default;

    /// 复用基类移动赋值语义。
    QuickUnion& operator=(QuickUnion&& other) noexcept = default;

    /// 由基类统一释放父指针数组。
    ~QuickUnion() override = default;

    /// 沿父指针查找根节点，复杂度为 O(tree height)。
    int find(int p) override;

    /// 将 p 的根直接链接到 q 的根。
    void unite(int p, int q) override;
};

inline QuickUnion::Storage::Storage(QuickUnion& owner) noexcept
    : owner_(owner) {}

inline void QuickUnion::Storage::validate(index_type index) const {
    owner_.validateIndex(index);
}

inline QuickUnion::Storage::index_type
QuickUnion::Storage::parentOf(index_type index) const noexcept {
    return owner_._id[index];
}

inline void QuickUnion::Storage::linkRoot(
    index_type child,
    index_type parent
) noexcept {
    owner_._id[child] = parent;
}

inline void QuickUnion::Storage::commitMerge() noexcept {
    --owner_._count;
}

inline QuickUnion::QuickUnion(int N)
    : UnionFind(N) {}

inline QuickUnion::QuickUnion(std::ifstream& input)
    : UnionFind(readElementCount(input)) {
    loadConnections(input);
    std::printf("%d components\n", count());
}

inline int QuickUnion::find(int p) {
    Storage storage(*this);
    return Algorithm::findRoot(storage, p);
}

inline void QuickUnion::unite(int p, int q) {
    const int pRoot = find(p);
    const int qRoot = find(q);
    Storage storage(*this);
    (void)Algorithm::linkRoots(storage, pRoot, qRoot);
}

#endif
