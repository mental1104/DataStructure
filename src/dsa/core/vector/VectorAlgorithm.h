#ifndef DSA_CORE_VECTOR_VECTOR_ALGORITHM_H
#define DSA_CORE_VECTOR_VECTOR_ALGORITHM_H

#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace dsa {
namespace core {
namespace detail {

template<typename...>
struct MakeVoid {
    typedef void type;
};

template<typename... Types>
using VoidT = typename MakeVoid<Types...>::type;

template<typename Storage, typename = void>
struct HasVectorStorageSizeType : std::false_type {
};

template<typename Storage>
struct HasVectorStorageSizeType<
    Storage,
    VoidT<typename Storage::size_type>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageSize : std::false_type {
};

template<typename Storage>
struct HasVectorStorageSize<
    Storage,
    VoidT<decltype(std::declval<const Storage&>().size())>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageMaxSize : std::false_type {
};

template<typename Storage>
struct HasVectorStorageMaxSize<
    Storage,
    VoidT<decltype(std::declval<const Storage&>().maxSize())>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageEnsureCapacity : std::false_type {
};

template<typename Storage>
struct HasVectorStorageEnsureCapacity<
    Storage,
    VoidT<decltype(std::declval<Storage&>().ensureCapacity(
        std::declval<typename Storage::size_type>()
    ))>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageOpenGap : std::false_type {
};

template<typename Storage>
struct HasVectorStorageOpenGap<
    Storage,
    VoidT<decltype(std::declval<Storage&>().openGap(
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>()
    ))>
> : std::true_type {
};

template<typename Storage, typename Value, typename = void>
struct HasVectorStorageWriteGap : std::false_type {
};

template<typename Storage, typename Value>
struct HasVectorStorageWriteGap<
    Storage,
    Value,
    VoidT<decltype(std::declval<Storage&>().writeGap(
        std::declval<typename Storage::size_type>(),
        std::declval<Value>()
    ))>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageRollbackGap : std::false_type {
};

template<typename Storage>
struct HasVectorStorageRollbackGap<
    Storage,
    VoidT<decltype(std::declval<Storage&>().rollbackGap(
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>()
    ))>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageCloseGap : std::false_type {
};

template<typename Storage>
struct HasVectorStorageCloseGap<
    Storage,
    VoidT<decltype(std::declval<Storage&>().closeGap(
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>(),
        std::declval<typename Storage::size_type>()
    ))>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageCommitSize : std::false_type {
};

template<typename Storage>
struct HasVectorStorageCommitSize<
    Storage,
    VoidT<decltype(std::declval<Storage&>().commitSize(
        std::declval<typename Storage::size_type>()
    ))>
> : std::true_type {
};

template<typename Storage, typename = void>
struct HasVectorStorageAfterErase : std::false_type {
};

template<typename Storage>
struct HasVectorStorageAfterErase<
    Storage,
    VoidT<decltype(std::declval<Storage&>().afterErase())>
> : std::true_type {
};

/// 判断 Storage 是否具备 VectorAlgorithm 与值写入无关的完整操作合同。
template<typename Storage>
struct HasVectorStorageContract : std::integral_constant<
    bool,
    HasVectorStorageSizeType<Storage>::value &&
    HasVectorStorageSize<Storage>::value &&
    HasVectorStorageMaxSize<Storage>::value &&
    HasVectorStorageEnsureCapacity<Storage>::value &&
    HasVectorStorageOpenGap<Storage>::value &&
    HasVectorStorageRollbackGap<Storage>::value &&
    HasVectorStorageCloseGap<Storage>::value &&
    HasVectorStorageCommitSize<Storage>::value &&
    HasVectorStorageAfterErase<Storage>::value
> {
};

} // namespace detail

/**
 * 编排连续容器教学版与工业版共用的增删状态机。
 *
 * Storage 负责分配、对象生命周期、元素搬移和具体异常保证；
 * VectorAlgorithm 只规定公共操作顺序与提交点。
 *
 * Storage 必须提供：
 *   typedef size_type
 *   size_type size() const
 *   size_type maxSize() const
 *   void ensureCapacity(size_type required)
 *   void openGap(size_type position, size_type count, size_type oldSize)
 *   template<typename Value> void writeGap(size_type position, Value&& value)
 *   void rollbackGap(size_type position, size_type count, size_type oldSize)
 *   void closeGap(size_type first, size_type last, size_type oldSize)
 *   void commitSize(size_type newSize)
 *   void afterErase()
 *
 * 这些要求通过编译期检测表达，不需要为静态策略引入虚基类。
 */
template<typename Storage>
class VectorAlgorithm {
    static_assert(
        detail::HasVectorStorageSizeType<Storage>::value,
        "VectorAlgorithm Storage 必须定义 size_type"
    );
    static_assert(
        detail::HasVectorStorageSize<Storage>::value,
        "VectorAlgorithm Storage 必须实现 size() const"
    );
    static_assert(
        detail::HasVectorStorageMaxSize<Storage>::value,
        "VectorAlgorithm Storage 必须实现 maxSize() const"
    );
    static_assert(
        detail::HasVectorStorageEnsureCapacity<Storage>::value,
        "VectorAlgorithm Storage 必须实现 ensureCapacity(required)"
    );
    static_assert(
        detail::HasVectorStorageOpenGap<Storage>::value,
        "VectorAlgorithm Storage 必须实现 openGap(position, count, oldSize)"
    );
    static_assert(
        detail::HasVectorStorageRollbackGap<Storage>::value,
        "VectorAlgorithm Storage 必须实现 rollbackGap(position, count, oldSize)"
    );
    static_assert(
        detail::HasVectorStorageCloseGap<Storage>::value,
        "VectorAlgorithm Storage 必须实现 closeGap(first, last, oldSize)"
    );
    static_assert(
        detail::HasVectorStorageCommitSize<Storage>::value,
        "VectorAlgorithm Storage 必须实现 commitSize(newSize)"
    );
    static_assert(
        detail::HasVectorStorageAfterErase<Storage>::value,
        "VectorAlgorithm Storage 必须实现 afterErase()"
    );

public:
    typedef typename Storage::size_type size_type;

    /**
     * 计算能够容纳 required 个元素的推荐容量。
     *
     * current 是当前容量，minimum 是增长后的最低容量，maximum 是允许
     * 达到的容量上限。默认上限为 size_type 能表达的最大值；
     * allocator-aware 容器应显式传入自身的 max_size()。
     */
    static size_type recommendCapacity(
        size_type current,
        size_type required,
        size_type minimum,
        size_type maximum = std::numeric_limits<size_type>::max()
    ) {
        if (required > maximum)
            throw std::length_error("Vector capacity exceeds max_size");

        // 扩容基线至少取当前容量与最小容量中的较大值。
        const size_type base = current < minimum ? minimum : current;

        // base 已满足需求时，无需为追求倍增而继续扩大容量。
        if (base >= required)
            return base;

        // base + base 可能溢出或超过 maximum，此时直接钳制到上限。
        const size_type grown =
            base > maximum - base ? maximum : base + base;

        // 一次大批量操作可能超过倍增结果，最终容量仍必须覆盖 required。
        return grown < required ? required : grown;
    }

    /**
     * 在 position 插入一个值。
     *
     * 顺序为：检查上限 → 准备容量 → 打开槽位 → 写入值 →
     * 写入失败时回滚 → 成功后提交 size。
     */
    template<typename Value>
    static size_type insert(
        Storage& storage,
        size_type position,
        Value&& value
    ) {
        static_assert(
            detail::HasVectorStorageWriteGap<Storage, Value&&>::value,
            "VectorAlgorithm Storage 必须实现 writeGap(position, value)"
        );

        const size_type oldSize = storage.size();
        if (oldSize == storage.maxSize())
            throw std::length_error("Vector size exceeds max_size");

        storage.ensureCapacity(oldSize + 1);
        storage.openGap(position, 1, oldSize);

        try {
            storage.writeGap(position, std::forward<Value>(value));
        } catch (...) {
            storage.rollbackGap(position, 1, oldSize);
            throw;
        }

        // size 是整个插入事务的提交点；此前异常不得改变逻辑大小。
        storage.commitSize(oldSize + 1);
        return position;
    }

    /**
     * 删除半开区间 [first, last)，返回删除的元素数量。
     *
     * Storage 先关闭空隙并处理退出逻辑区间的对象，随后提交新 size，
     * 最后执行教学版自动缩容或工业版保持容量等实现专属策略。
     */
    static size_type erase(
        Storage& storage,
        size_type first,
        size_type last
    ) {
        if (first == last)
            return 0;

        const size_type oldSize = storage.size();
        const size_type removed = last - first;
        storage.closeGap(first, last, oldSize);
        storage.commitSize(oldSize - removed);
        storage.afterErase();
        return removed;
    }
};

} // namespace core
} // namespace dsa

#endif
