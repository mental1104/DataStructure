#ifndef DSA_CORE_HASH_HASH_TABLE_ALGORITHM_H
#define DSA_CORE_HASH_HASH_TABLE_ALGORITHM_H

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace dsa {
namespace core {
namespace detail {

/// 在不产生无符号溢出的前提下计算 (left + right) % modulus。
inline std::size_t addModulo(
    std::size_t left,
    std::size_t right,
    std::size_t modulus
) {
    left %= modulus;
    right %= modulus;
    return right >= modulus - left ? right - (modulus - left) : left + right;
}

/// 在不产生无符号溢出的前提下计算 (left - right) % modulus。
inline std::size_t subtractModulo(
    std::size_t left,
    std::size_t right,
    std::size_t modulus
) {
    left %= modulus;
    right %= modulus;
    return left >= right ? left - right : modulus - (right - left);
}

/// 使用倍增法计算 (left * right) % modulus，供二次探测生成平方偏移。
inline std::size_t multiplyModulo(
    std::size_t left,
    std::size_t right,
    std::size_t modulus
) {
    std::size_t result = 0;
    left %= modulus;
    while (right != 0) {
        if ((right & 1U) != 0)
            result = addModulo(result, left, modulus);
        right >>= 1U;
        if (right != 0)
            left = addModulo(left, left, modulus);
    }
    return result;
}

/// 判断一个 size_t 整数是否为素数。
inline bool isPrime(std::size_t value) {
    if (value < 2)
        return false;
    if ((value & 1U) == 0)
        return value == 2;
    for (std::size_t divisor = 3; divisor <= value / divisor; divisor += 2) {
        if (value % divisor == 0)
            return false;
    }
    return true;
}

/// 返回不小于 requested 的最小素数；无法继续增长时抛出 length_error。
inline std::size_t nextPrime(std::size_t requested) {
    if (requested <= 2)
        return 2;
    std::size_t candidate = (requested & 1U) == 0 ? requested + 1 : requested;
    while (!isPrime(candidate)) {
        if (candidate > (std::numeric_limits<std::size_t>::max)() - 2)
            throw std::length_error("Hash table bucket count overflow");
        candidate += 2;
    }
    return candidate;
}

/// 返回不小于 requested 且满足 p % 4 == 3 的最小素数。
inline std::size_t nextQuadraticPrime(std::size_t requested) {
    std::size_t candidate = requested < 3 ? 3 : requested;
    if ((candidate & 1U) == 0)
        ++candidate;
    while (!isPrime(candidate) || candidate % 4 != 3) {
        if (candidate > (std::numeric_limits<std::size_t>::max)() - 2)
            throw std::length_error("Quadratic hash table bucket count overflow");
        candidate += 2;
    }
    return candidate;
}

} // namespace detail

/// 线性探测策略：按 base、base+1、base+2 的顺序扫描全部桶。
struct LinearProbing {
    /// 将请求容量规范化为至少 3 的素数桶数。
    static std::size_t bucketCountAtLeast(std::size_t requested) {
        return detail::nextPrime(requested < 3 ? 3 : requested);
    }

    /// 返回第 step 次探测对应的桶下标。
    static std::size_t index(
        std::size_t hash,
        std::size_t step,
        std::size_t bucketCount
    ) {
        return detail::addModulo(hash % bucketCount, step, bucketCount);
    }
};

/// 双向平方探测策略：按 0、+1²、-1²、+2²、-2² 的顺序扫描。
struct QuadraticProbing {
    /// 选择 p % 4 == 3 的素数桶数，使正负平方偏移能够覆盖全部桶。
    static std::size_t bucketCountAtLeast(std::size_t requested) {
        return detail::nextQuadraticPrime(requested);
    }

    /// 返回第 step 次探测对应的桶下标，并避免负数取模与乘法溢出。
    static std::size_t index(
        std::size_t hash,
        std::size_t step,
        std::size_t bucketCount
    ) {
        const std::size_t base = hash % bucketCount;
        if (step == 0)
            return base;

        const std::size_t magnitude = (step + 1) / 2;
        const std::size_t square = detail::multiplyModulo(
            magnitude,
            magnitude,
            bucketCount
        );
        return (step & 1U) != 0
            ? detail::addModulo(base, square, bucketCount)
            : detail::subtractModulo(base, square, bucketCount);
    }
};

/// 记录一次开放寻址探测的命中位置与可插入位置。
template<typename Size>
struct HashProbeResult {
    Size foundIndex;
    Size insertionIndex;
    bool found;

    /// 返回该下标类型使用的无效位置哨兵。
    static Size npos() {
        return (std::numeric_limits<Size>::max)();
    }
};

/// 协调开放寻址哈希表的查找、插入准备、删除与事务式 rehash。
///
/// Storage 负责具体桶表示、对象生命周期、哈希器、相等比较器和资源回滚；
/// 本算法层只维护探测顺序、提交顺序与“不命中时优先复用墓碑”的共同语义。
template<typename Storage, typename ProbePolicy>
class HashTableAlgorithm {
public:
    typedef typename Storage::size_type size_type;
    typedef HashProbeResult<size_type> probe_result;

    /// 查找 key，并同时返回首次可复用墓碑或真正空桶。
    template<typename Key>
    static probe_result locate(const Storage& storage, const Key& key) {
        const size_type bucketCount = storage.bucketCount();
        const size_type invalid = probe_result::npos();
        if (bucketCount == 0)
            return probe_result{invalid, invalid, false};

        const std::size_t hash = storage.hashKey(key);
        size_type firstDeleted = invalid;
        for (size_type step = 0; step < bucketCount; ++step) {
            const size_type index = static_cast<size_type>(
                ProbePolicy::index(hash, step, bucketCount)
            );
            if (storage.isOccupied(index)) {
                if (storage.keysEqual(index, key))
                    return probe_result{index, index, true};
                continue;
            }
            if (storage.isDeleted(index)) {
                if (firstDeleted == invalid)
                    firstDeleted = index;
                continue;
            }
            return probe_result{
                invalid,
                firstDeleted == invalid ? index : firstDeleted,
                false
            };
        }

        return probe_result{invalid, firstDeleted, false};
    }

    /// 在不构造元素的前提下完成重复键检查、扩容/清墓碑与插入位置选择。
    template<typename Key>
    static probe_result prepareInsert(Storage& storage, const Key& key) {
        probe_result result = locate(storage, key);
        if (result.found)
            return result;

        if (storage.needsRehashForInsert() || result.insertionIndex == probe_result::npos()) {
            rehash(storage, storage.growthBucketCount());
            result = locate(storage, key);
        }

        if (result.insertionIndex == probe_result::npos())
            throw std::length_error("Hash table has no insertion slot");
        return result;
    }

    /// 查找并删除 key；具体析构和墓碑提交由 Storage 完成。
    template<typename Key>
    static bool erase(Storage& storage, const Key& key) {
        const probe_result result = locate(storage, key);
        if (!result.found)
            return false;
        storage.eraseAt(result.foundIndex);
        return true;
    }

    /// 以“准备新桶→迁移引用/节点→成功提交→释放旧桶”的顺序执行 rehash。
    static void rehash(Storage& storage, size_type requestedBucketCount) {
        const size_type target = static_cast<size_type>(
            ProbePolicy::bucketCountAtLeast(requestedBucketCount)
        );
        storage.beginRehash(target);
        try {
            const size_type oldBucketCount = storage.bucketCount();
            for (size_type index = 0; index < oldBucketCount; ++index) {
                if (storage.isOccupied(index))
                    storage.transferBucketToPending(index);
            }
        } catch (...) {
            storage.rollbackRehash();
            throw;
        }
        storage.commitRehash();
    }
};

} // namespace core
} // namespace dsa

#endif
