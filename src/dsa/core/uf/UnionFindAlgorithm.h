#ifndef DSA_CORE_UF_UNION_FIND_ALGORITHM_H
#define DSA_CORE_UF_UNION_FIND_ALGORITHM_H

#include <cstddef>

namespace dsa {
namespace core {

/// 编排并查集家族共有的查找与合并流程，具体存储和组件计数由 Storage 负责。
///
/// 各操作按需使用以下语义接口：
/// - validate(index)：检查索引合法性；
/// - elementCount()：返回元素总数；
/// - representativeAt(index) / replaceRepresentative(index, representative)：Quick-Find 标签访问；
/// - parentOf(index) / linkRoot(child, parent)：树形父节点访问；
/// - componentSize(root) / absorbComponent(parent, child)：按大小合并所需元数据；
/// - commitMerge()：在结构修改完成后提交组件数量变化。
template<typename Storage>
class UnionFindAlgorithm {
public:
    typedef typename Storage::index_type index_type;
    typedef typename Storage::size_type size_type;

    /// 返回 Quick-Find 表示中的组件标签，复杂度为 O(1)。
    static index_type findRepresentative(Storage& storage, index_type index);

    /// 扫描全部标签并把 source 组件改写为 target，复杂度为 O(N)。
    static bool uniteRepresentatives(
        Storage& storage,
        index_type source,
        index_type target
    );

    /// 沿父指针查找根节点，不修改路径，复杂度为 O(tree height)。
    static index_type findRoot(Storage& storage, index_type index);

    /// 两阶段查根并压缩整条访问路径，均摊复杂度接近 O(1)。
    static index_type findRootWithCompression(Storage& storage, index_type index);

    /// 将一个根直接链接到另一个根，适用于朴素 Quick-Union。
    static bool linkRoots(
        Storage& storage,
        index_type sourceRoot,
        index_type targetRoot
    );

    /// 将较小树链接到较大树，并在链接完成后提交组件数量。
    static bool linkRootsBySize(
        Storage& storage,
        index_type leftRoot,
        index_type rightRoot
    );
};

template<typename Storage>
typename UnionFindAlgorithm<Storage>::index_type
UnionFindAlgorithm<Storage>::findRepresentative(Storage& storage, index_type index) {
    storage.validate(index);
    return storage.representativeAt(index);
}

template<typename Storage>
bool UnionFindAlgorithm<Storage>::uniteRepresentatives(
    Storage& storage,
    index_type source,
    index_type target
) {
    if (source == target)
        return false;

    const size_type count = storage.elementCount();
    for (size_type offset = 0; offset < count; ++offset) {
        const index_type index = static_cast<index_type>(offset);
        if (storage.representativeAt(index) == source)
            storage.replaceRepresentative(index, target);
    }

    storage.commitMerge();
    return true;
}

template<typename Storage>
typename UnionFindAlgorithm<Storage>::index_type
UnionFindAlgorithm<Storage>::findRoot(Storage& storage, index_type index) {
    storage.validate(index);
    while (index != storage.parentOf(index))
        index = storage.parentOf(index);
    return index;
}

template<typename Storage>
typename UnionFindAlgorithm<Storage>::index_type
UnionFindAlgorithm<Storage>::findRootWithCompression(
    Storage& storage,
    index_type index
) {
    storage.validate(index);

    index_type root = index;
    while (root != storage.parentOf(root))
        root = storage.parentOf(root);

    while (index != storage.parentOf(index)) {
        const index_type next = storage.parentOf(index);
        storage.linkRoot(index, root);
        index = next;
    }

    return root;
}

template<typename Storage>
bool UnionFindAlgorithm<Storage>::linkRoots(
    Storage& storage,
    index_type sourceRoot,
    index_type targetRoot
) {
    if (sourceRoot == targetRoot)
        return false;

    storage.linkRoot(sourceRoot, targetRoot);
    storage.commitMerge();
    return true;
}

template<typename Storage>
bool UnionFindAlgorithm<Storage>::linkRootsBySize(
    Storage& storage,
    index_type leftRoot,
    index_type rightRoot
) {
    if (leftRoot == rightRoot)
        return false;

    if (storage.componentSize(leftRoot) < storage.componentSize(rightRoot)) {
        storage.linkRoot(leftRoot, rightRoot);
        storage.absorbComponent(rightRoot, leftRoot);
    } else {
        storage.linkRoot(rightRoot, leftRoot);
        storage.absorbComponent(leftRoot, rightRoot);
    }

    storage.commitMerge();
    return true;
}

} // namespace core
} // namespace dsa

#endif
