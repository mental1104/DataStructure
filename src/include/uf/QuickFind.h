#ifndef DSA_UF_QUICK_FIND_H
#define DSA_UF_QUICK_FIND_H

#include <cstddef>
#include <fstream>

#include "UnionFind.h"
#include "../dsa/core/uf/UnionFindAlgorithm.h"

/// 教学版 Quick-Find：查询 O(1)，合并需要扫描全部元素。
class QuickFind : public UnionFind {
private:
    /// 把教学版标签数组适配到共享 UnionFindAlgorithm。
    class Storage {
    public:
        typedef int index_type;
        typedef std::size_t size_type;

        /// 绑定当前 QuickFind 实例。
        explicit Storage(QuickFind& owner) noexcept;

        /// 检查教学索引合法性。
        void validate(index_type index) const;

        /// 返回元素总数。
        size_type elementCount() const noexcept;

        /// 读取元素当前的组件标签。
        index_type representativeAt(index_type index) const noexcept;

        /// 替换元素的组件标签。
        void replaceRepresentative(index_type index, index_type representative) noexcept;

        /// 在标签改写完成后提交组件数量变化。
        void commitMerge() noexcept;

    private:
        QuickFind& owner_;
    };

    typedef dsa::core::UnionFindAlgorithm<Storage> Algorithm;

public:
    QuickFind() = delete;

    /// 构造 N 个互不连通的元素。
    explicit QuickFind(int N);

    /// 从文件流读取 N 和连接对，保留原教学构造入口。
    explicit QuickFind(std::ifstream& input);

    /// 复用基类深拷贝语义。
    QuickFind(const QuickFind& other) = default;

    /// 复用基类深拷贝赋值语义。
    QuickFind& operator=(const QuickFind& other) = default;

    /// 复用基类移动语义。
    QuickFind(QuickFind&& other) noexcept = default;

    /// 复用基类移动赋值语义。
    QuickFind& operator=(QuickFind&& other) noexcept = default;

    /// 由基类统一释放标签数组。
    ~QuickFind() override = default;

    /// O(1) 返回组件标签。
    int find(int p) override;

    /// O(N) 将 p 所在标签全部替换为 q 所在标签。
    void unite(int p, int q) override;
};

inline QuickFind::Storage::Storage(QuickFind& owner) noexcept
    : owner_(owner) {}

inline void QuickFind::Storage::validate(index_type index) const {
    owner_.validateIndex(index);
}

inline QuickFind::Storage::size_type
QuickFind::Storage::elementCount() const noexcept {
    return static_cast<size_type>(owner_._N);
}

inline QuickFind::Storage::index_type
QuickFind::Storage::representativeAt(index_type index) const noexcept {
    return owner_._id[index];
}

inline void QuickFind::Storage::replaceRepresentative(
    index_type index,
    index_type representative
) noexcept {
    owner_._id[index] = representative;
}

inline void QuickFind::Storage::commitMerge() noexcept {
    --owner_._count;
}

inline QuickFind::QuickFind(int N)
    : UnionFind(N) {}

inline QuickFind::QuickFind(std::ifstream& input)
    : UnionFind(readElementCount(input)) {
    loadConnections(input);
    std::printf("%d components\n", count());
}

inline int QuickFind::find(int p) {
    Storage storage(*this);
    return Algorithm::findRepresentative(storage, p);
}

inline void QuickFind::unite(int p, int q) {
    const int pId = find(p);
    const int qId = find(q);
    Storage storage(*this);
    (void)Algorithm::uniteRepresentatives(storage, pId, qId);
}

#endif
