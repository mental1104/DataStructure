#ifndef DSA_CONTAINER_UF_UNION_FIND_H
#define DSA_CONTAINER_UF_UNION_FIND_H

#include <cstddef>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../core/uf/UnionFindAlgorithm.h"

namespace dsa {
namespace container {

/// 工业版并查集：按大小合并并执行完整路径压缩。
///
/// 元素集合固定为 [0, size())，构造后不发生重新分配；unite 和 find
/// 不使任何外部对象失效。索引越界时抛出 std::out_of_range。
template<typename Allocator = std::allocator<std::size_t> >
class UnionFind {
public:
    typedef std::size_t value_type;
    typedef Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;

private:
    typedef std::vector<value_type, allocator_type> storage_type;

    storage_type parent_;
    storage_type componentSize_;
    size_type componentCount_;

    /// 把工业版父节点和组件大小存储适配到共享 UnionFindAlgorithm。
    class Storage {
    public:
        typedef value_type index_type;
        typedef typename UnionFind::size_type size_type;

        /// 绑定当前工业版 UnionFind 实例。
        explicit Storage(UnionFind& owner) noexcept;

        /// 检查索引合法性。
        void validate(index_type index) const;

        /// 返回节点的父节点。
        index_type parentOf(index_type index) const noexcept;

        /// 将子根或路径节点链接到新的父节点。
        void linkRoot(index_type child, index_type parent) noexcept;

        /// 返回根节点维护的组件大小。
        size_type componentSize(index_type root) const noexcept;

        /// 把 childRoot 的组件大小合并到 parentRoot。
        void absorbComponent(index_type parentRoot, index_type childRoot) noexcept;

        /// 在结构和元数据全部更新后提交组件数量变化。
        void commitMerge() noexcept;

    private:
        UnionFind& owner_;
    };

    /// 为 const 查询提供只读父指针适配，不执行路径压缩。
    class ConstStorage {
    public:
        typedef value_type index_type;
        typedef typename UnionFind::size_type size_type;

        /// 绑定当前只读 UnionFind 实例。
        explicit ConstStorage(const UnionFind& owner) noexcept;

        /// 检查索引合法性。
        void validate(index_type index) const;

        /// 返回节点的父节点。
        index_type parentOf(index_type index) const noexcept;

    private:
        const UnionFind& owner_;
    };

    typedef dsa::core::UnionFindAlgorithm<Storage> Algorithm;
    typedef dsa::core::UnionFindAlgorithm<ConstStorage> ConstAlgorithm;

    /// 检查索引是否落在 [0, size())。
    void validateIndex(value_type index) const;

public:
    /// 构造空并查集。
    UnionFind();

    /// 构造 count 个互不连通的元素，并使用指定 allocator 管理元数据。
    explicit UnionFind(
        size_type count,
        const allocator_type& allocator = allocator_type()
    );

    /// 深拷贝父节点、组件大小和组件数量。
    UnionFind(const UnionFind& other) = default;

    /// 深拷贝赋值父节点、组件大小和组件数量。
    UnionFind& operator=(const UnionFind& other) = default;

    /// 转移内部存储，并把源对象重置为合法空状态。
    UnionFind(UnionFind&& other) noexcept(
        std::is_nothrow_move_constructible<storage_type>::value
    );

    /// 转移赋值内部存储，并把源对象重置为合法空状态。
    UnionFind& operator=(UnionFind&& other) noexcept(
        std::is_nothrow_move_assignable<storage_type>::value
    );

    /// 返回用于父节点存储的 allocator 副本。
    allocator_type get_allocator() const noexcept;

    /// 返回元素总数。
    size_type size() const noexcept;

    /// 判断当前是否不包含元素。
    bool empty() const noexcept;

    /// 返回当前连通分量数量。
    size_type count() const noexcept;

    /// 返回当前连通分量数量的语义化别名。
    size_type componentCount() const noexcept;

    /// 查找根节点并压缩访问路径，均摊复杂度为 O(alpha(N))。
    value_type find(value_type index);

    /// 只读查找根节点，不修改路径，复杂度为 O(log N)。
    value_type find(value_type index) const;

    /// 判断两个元素是否属于同一组件，并压缩两条访问路径。
    bool connected(value_type left, value_type right);

    /// 只读判断两个元素是否属于同一组件。
    bool connected(value_type left, value_type right) const;

    /// 按大小合并两个组件；实际发生合并时返回 true。
    bool unite(value_type left, value_type right);

    /// 返回指定元素所在组件的元素数量，并压缩访问路径。
    size_type componentSize(value_type index);

    /// 返回指定元素所在组件的元素数量，不修改路径。
    size_type componentSize(value_type index) const;

    /// 恢复为每个元素各自独立的初始状态，复杂度为 O(N)。
    void reset() noexcept;
};

template<typename Allocator>
UnionFind<Allocator>::Storage::Storage(UnionFind& owner) noexcept
    : owner_(owner) {}

template<typename Allocator>
void UnionFind<Allocator>::Storage::validate(index_type index) const {
    owner_.validateIndex(index);
}

template<typename Allocator>
typename UnionFind<Allocator>::Storage::index_type
UnionFind<Allocator>::Storage::parentOf(index_type index) const noexcept {
    return owner_.parent_[index];
}

template<typename Allocator>
void UnionFind<Allocator>::Storage::linkRoot(
    index_type child,
    index_type parent
) noexcept {
    owner_.parent_[child] = parent;
}

template<typename Allocator>
typename UnionFind<Allocator>::Storage::size_type
UnionFind<Allocator>::Storage::componentSize(index_type root) const noexcept {
    return owner_.componentSize_[root];
}

template<typename Allocator>
void UnionFind<Allocator>::Storage::absorbComponent(
    index_type parentRoot,
    index_type childRoot
) noexcept {
    owner_.componentSize_[parentRoot] += owner_.componentSize_[childRoot];
}

template<typename Allocator>
void UnionFind<Allocator>::Storage::commitMerge() noexcept {
    --owner_.componentCount_;
}

template<typename Allocator>
UnionFind<Allocator>::ConstStorage::ConstStorage(const UnionFind& owner) noexcept
    : owner_(owner) {}

template<typename Allocator>
void UnionFind<Allocator>::ConstStorage::validate(index_type index) const {
    owner_.validateIndex(index);
}

template<typename Allocator>
typename UnionFind<Allocator>::ConstStorage::index_type
UnionFind<Allocator>::ConstStorage::parentOf(index_type index) const noexcept {
    return owner_.parent_[index];
}

template<typename Allocator>
void UnionFind<Allocator>::validateIndex(value_type index) const {
    if (index >= parent_.size())
        throw std::out_of_range("UnionFind index is out of range");
}

template<typename Allocator>
UnionFind<Allocator>::UnionFind()
    : parent_(), componentSize_(), componentCount_(0) {}

template<typename Allocator>
UnionFind<Allocator>::UnionFind(
    size_type count,
    const allocator_type& allocator
) : parent_(count, value_type(), allocator),
    componentSize_(count, value_type(1), allocator),
    componentCount_(count) {
    std::iota(parent_.begin(), parent_.end(), value_type());
}

template<typename Allocator>
UnionFind<Allocator>::UnionFind(UnionFind&& other) noexcept(
    std::is_nothrow_move_constructible<storage_type>::value
) : parent_(std::move(other.parent_)),
    componentSize_(std::move(other.componentSize_)),
    componentCount_(other.componentCount_) {
    other.parent_.clear();
    other.componentSize_.clear();
    other.componentCount_ = 0;
}

template<typename Allocator>
UnionFind<Allocator>& UnionFind<Allocator>::operator=(UnionFind&& other) noexcept(
    std::is_nothrow_move_assignable<storage_type>::value
) {
    if (this == &other)
        return *this;

    parent_ = std::move(other.parent_);
    componentSize_ = std::move(other.componentSize_);
    componentCount_ = other.componentCount_;

    other.parent_.clear();
    other.componentSize_.clear();
    other.componentCount_ = 0;
    return *this;
}

template<typename Allocator>
typename UnionFind<Allocator>::allocator_type
UnionFind<Allocator>::get_allocator() const noexcept {
    return parent_.get_allocator();
}

template<typename Allocator>
typename UnionFind<Allocator>::size_type
UnionFind<Allocator>::size() const noexcept {
    return parent_.size();
}

template<typename Allocator>
bool UnionFind<Allocator>::empty() const noexcept {
    return parent_.empty();
}

template<typename Allocator>
typename UnionFind<Allocator>::size_type
UnionFind<Allocator>::count() const noexcept {
    return componentCount_;
}

template<typename Allocator>
typename UnionFind<Allocator>::size_type
UnionFind<Allocator>::componentCount() const noexcept {
    return componentCount_;
}

template<typename Allocator>
typename UnionFind<Allocator>::value_type
UnionFind<Allocator>::find(value_type index) {
    Storage storage(*this);
    return Algorithm::findRootWithCompression(storage, index);
}

template<typename Allocator>
typename UnionFind<Allocator>::value_type
UnionFind<Allocator>::find(value_type index) const {
    ConstStorage storage(*this);
    return ConstAlgorithm::findRoot(storage, index);
}

template<typename Allocator>
bool UnionFind<Allocator>::connected(value_type left, value_type right) {
    return find(left) == find(right);
}

template<typename Allocator>
bool UnionFind<Allocator>::connected(value_type left, value_type right) const {
    return find(left) == find(right);
}

template<typename Allocator>
bool UnionFind<Allocator>::unite(value_type left, value_type right) {
    const value_type leftRoot = find(left);
    const value_type rightRoot = find(right);
    Storage storage(*this);
    return Algorithm::linkRootsBySize(storage, leftRoot, rightRoot);
}

template<typename Allocator>
typename UnionFind<Allocator>::size_type
UnionFind<Allocator>::componentSize(value_type index) {
    return componentSize_[find(index)];
}

template<typename Allocator>
typename UnionFind<Allocator>::size_type
UnionFind<Allocator>::componentSize(value_type index) const {
    return componentSize_[find(index)];
}

template<typename Allocator>
void UnionFind<Allocator>::reset() noexcept {
    std::iota(parent_.begin(), parent_.end(), value_type());
    std::fill(componentSize_.begin(), componentSize_.end(), value_type(1));
    componentCount_ = parent_.size();
}

} // namespace container
} // namespace dsa

#endif
