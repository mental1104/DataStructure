#ifndef DSA_UF_WEIGHTED_QUICK_UNION_WITH_COMPRESSION_H
#define DSA_UF_WEIGHTED_QUICK_UNION_WITH_COMPRESSION_H

#include <fstream>

#include "WeightedQuickUnion.h"

/// 教学版按大小合并并带完整路径压缩的并查集。
class WeightedQuickUnionwithCompression : public WeightedQuickUnion {
public:
    WeightedQuickUnionwithCompression() = delete;

    /// 构造 N 棵单节点树。
    explicit WeightedQuickUnionwithCompression(int N);

    /// 从文件流读取 N 和连接对，保留原教学构造入口。
    explicit WeightedQuickUnionwithCompression(std::ifstream& input);

    /// 复用父类深拷贝语义。
    WeightedQuickUnionwithCompression(
        const WeightedQuickUnionwithCompression& other
    ) = default;

    /// 复用父类深拷贝赋值语义。
    WeightedQuickUnionwithCompression& operator=(
        const WeightedQuickUnionwithCompression& other
    ) = default;

    /// 复用父类移动语义。
    WeightedQuickUnionwithCompression(
        WeightedQuickUnionwithCompression&& other
    ) noexcept = default;

    /// 复用父类移动赋值语义。
    WeightedQuickUnionwithCompression& operator=(
        WeightedQuickUnionwithCompression&& other
    ) noexcept = default;

    /// 由父类 RAII 成员统一释放存储。
    ~WeightedQuickUnionwithCompression() override = default;

    /// 查找根节点并把访问路径上的节点直接链接到根。
    int find(int p) override;
};

inline WeightedQuickUnionwithCompression::WeightedQuickUnionwithCompression(int N)
    : WeightedQuickUnion(N) {}

inline WeightedQuickUnionwithCompression::WeightedQuickUnionwithCompression(
    std::ifstream& input
) : WeightedQuickUnion(readElementCount(input)) {
    loadConnections(input);
    std::printf("%d components\n", count());
}

inline int WeightedQuickUnionwithCompression::find(int p) {
    Storage storage(*this);
    return Algorithm::findRootWithCompression(storage, p);
}

/// 提供大小写更自然的新名称，同时保留原类名兼容旧代码。
using WeightedQuickUnionWithCompression = WeightedQuickUnionwithCompression;

#endif
