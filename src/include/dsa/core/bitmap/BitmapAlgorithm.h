#ifndef DSA_CORE_BITMAP_BITMAP_ALGORITHM_H
#define DSA_CORE_BITMAP_BITMAP_ALGORITHM_H

#include <cstddef>
#include <limits>
#include <type_traits>

namespace dsa {
namespace core {

/// 为定长无符号块定义位图的块索引、掩码与尾块规范化规则。
template<typename Block>
struct BitmapLayout {
    static_assert(std::is_integral<Block>::value, "Bitmap block must be an integer type");
    static_assert(std::is_unsigned<Block>::value, "Bitmap block must be unsigned");

    typedef Block block_type;
    typedef std::size_t size_type;

    /// 返回单个存储块可容纳的位数。
    static size_type bitsPerBlock() noexcept {
        return static_cast<size_type>((std::numeric_limits<block_type>::digits));
    }

    /// 计算容纳 bitCount 个逻辑位所需的存储块数量。
    static size_type blocksFor(size_type bitCount) noexcept {
        if (bitCount == 0)
            return 0;
        return 1 + (bitCount - 1) / bitsPerBlock();
    }

    /// 返回逻辑位所在的存储块下标。
    static size_type blockIndex(size_type bitIndex) noexcept {
        return bitIndex / bitsPerBlock();
    }

    /// 返回逻辑位在块内对应的掩码；索引 0 映射到最高位以兼容教学版文件格式。
    static block_type mask(size_type bitIndex) noexcept {
        const size_type offset = bitIndex % bitsPerBlock();
        return static_cast<block_type>(
            static_cast<block_type>(1) << (bitsPerBlock() - 1 - offset)
        );
    }

    /// 返回低层存储块的全 1 值。
    static block_type fullBlock() noexcept {
        return (std::numeric_limits<block_type>::max)();
    }

    /// 返回前 validBits 个逻辑位有效的掩码；有效位位于块的高位侧。
    static block_type validMask(size_type validBits) noexcept {
        if (validBits == 0)
            return static_cast<block_type>(0);
        if (validBits >= bitsPerBlock())
            return fullBlock();
        return static_cast<block_type>(
            fullBlock() << (bitsPerBlock() - validBits)
        );
    }
};

/// 编排教学位图和工业位图共用的块级操作，不接管具体存储与分配器。
///
/// Access 需要提供：
/// - block_type / size_type；
/// - size_type blockCount() const；
/// - block_type readBlock(size_type) const；
/// - void writeBlock(size_type, block_type)。
template<typename Access>
class BitmapAlgorithm {
public:
    typedef typename Access::block_type block_type;
    typedef typename Access::size_type size_type;
    typedef BitmapLayout<block_type> Layout;

    /// 将指定逻辑位设为 value；调用方负责保证索引已落入已分配范围。
    static void set(Access& access, size_type bitIndex, bool value = true) {
        const size_type block = Layout::blockIndex(bitIndex);
        const block_type bitMask = Layout::mask(bitIndex);
        const block_type current = access.readBlock(block);
        access.writeBlock(
            block,
            value
                ? static_cast<block_type>(current | bitMask)
                : static_cast<block_type>(current & static_cast<block_type>(~bitMask))
        );
    }

    /// 清除指定逻辑位；调用方负责保证索引已落入已分配范围。
    static void reset(Access& access, size_type bitIndex) {
        set(access, bitIndex, false);
    }

    /// 翻转指定逻辑位；调用方负责保证索引已落入已分配范围。
    static void flip(Access& access, size_type bitIndex) {
        const size_type block = Layout::blockIndex(bitIndex);
        access.writeBlock(
            block,
            static_cast<block_type>(access.readBlock(block) ^ Layout::mask(bitIndex))
        );
    }

    /// 查询指定逻辑位；调用方负责保证索引已落入已分配范围。
    static bool test(const Access& access, size_type bitIndex) {
        return (
            access.readBlock(Layout::blockIndex(bitIndex)) & Layout::mask(bitIndex)
        ) != 0;
    }

    /// 将所有已分配块清零。
    static void resetAll(Access& access) {
        for (size_type block = 0; block < access.blockCount(); ++block)
            access.writeBlock(block, static_cast<block_type>(0));
    }

    /// 将前 bitCount 个逻辑位置 1，并保证尾块未使用位保持为 0。
    static void setAll(Access& access, size_type bitCount) {
        for (size_type block = 0; block < access.blockCount(); ++block)
            access.writeBlock(block, Layout::fullBlock());
        normalizeTail(access, bitCount);
    }

    /// 翻转前 bitCount 个逻辑位，并保证尾块未使用位保持为 0。
    static void flipAll(Access& access, size_type bitCount) {
        for (size_type block = 0; block < access.blockCount(); ++block) {
            access.writeBlock(
                block,
                static_cast<block_type>(~access.readBlock(block))
            );
        }
        normalizeTail(access, bitCount);
    }

    /// 统计前 bitCount 个逻辑位中置位的数量。
    static size_type count(const Access& access, size_type bitCount) {
        const size_type blocks = Layout::blocksFor(bitCount);
        size_type result = 0;
        for (size_type block = 0; block < blocks; ++block) {
            block_type value = access.readBlock(block);
            if (block + 1 == blocks) {
                const size_type tailBits = bitCount % Layout::bitsPerBlock();
                if (tailBits != 0)
                    value = static_cast<block_type>(value & Layout::validMask(tailBits));
            }
            result += popcount(value);
        }
        return result;
    }

    /// 判断前 bitCount 个逻辑位是否至少有一个置位，按块跳过全零区间。
    static bool any(const Access& access, size_type bitCount) {
        const size_type blocks = Layout::blocksFor(bitCount);
        for (size_type block = 0; block < blocks; ++block) {
            block_type value = access.readBlock(block);
            if (block + 1 == blocks) {
                const size_type tailBits = bitCount % Layout::bitsPerBlock();
                if (tailBits != 0)
                    value = static_cast<block_type>(value & Layout::validMask(tailBits));
            }
            if (value != 0)
                return true;
        }
        return false;
    }

    /// 判断前 bitCount 个逻辑位是否全部置位；空位图按数学约定返回 true。
    static bool all(const Access& access, size_type bitCount) {
        const size_type blocks = Layout::blocksFor(bitCount);
        for (size_type block = 0; block < blocks; ++block) {
            block_type expected = Layout::fullBlock();
            if (block + 1 == blocks) {
                const size_type tailBits = bitCount % Layout::bitsPerBlock();
                if (tailBits != 0)
                    expected = Layout::validMask(tailBits);
            }
            if (static_cast<block_type>(access.readBlock(block) & expected) != expected)
                return false;
        }
        return true;
    }

    /// 查找第一个等于 value 的逻辑位，未找到时返回 bitCount。
    static size_type findFirst(
        const Access& access,
        size_type bitCount,
        bool value
    ) {
        return findFrom(access, bitCount, 0, value);
    }

    /// 从 previous 后一个位置查找等于 value 的逻辑位，未找到时返回 bitCount。
    static size_type findNext(
        const Access& access,
        size_type bitCount,
        size_type previous,
        bool value
    ) {
        if (
            previous == (std::numeric_limits<size_type>::max)() ||
            previous >= bitCount ||
            previous + 1 >= bitCount
        ) {
            return bitCount;
        }
        return findFrom(access, bitCount, previous + 1, value);
    }

    /// 对等长位图执行按位与；块数和逻辑长度由上层容器校验。
    template<typename RightAccess>
    static void bitAnd(
        Access& destination,
        const RightAccess& right,
        size_type bitCount
    ) {
        combine(destination, right, bitCount, AndOperation());
    }

    /// 对等长位图执行按位或；块数和逻辑长度由上层容器校验。
    template<typename RightAccess>
    static void bitOr(
        Access& destination,
        const RightAccess& right,
        size_type bitCount
    ) {
        combine(destination, right, bitCount, OrOperation());
    }

    /// 对等长位图执行按位异或；块数和逻辑长度由上层容器校验。
    template<typename RightAccess>
    static void bitXor(
        Access& destination,
        const RightAccess& right,
        size_type bitCount
    ) {
        combine(destination, right, bitCount, XorOperation());
    }

    /// 清除尾块中不属于逻辑位图的未使用位。
    static void normalizeTail(Access& access, size_type bitCount) {
        if (bitCount == 0 || access.blockCount() == 0)
            return;
        const size_type validBits = bitCount % Layout::bitsPerBlock();
        if (validBits == 0)
            return;
        const size_type last = Layout::blocksFor(bitCount) - 1;
        access.writeBlock(
            last,
            static_cast<block_type>(access.readBlock(last) & Layout::validMask(validBits))
        );
    }

private:
    /// 将两个存储块执行按位与。
    struct AndOperation {
        /// 返回 left 与 right 的按位与结果。
        block_type operator()(block_type left, block_type right) const {
            return static_cast<block_type>(left & right);
        }
    };

    /// 将两个存储块执行按位或。
    struct OrOperation {
        /// 返回 left 与 right 的按位或结果。
        block_type operator()(block_type left, block_type right) const {
            return static_cast<block_type>(left | right);
        }
    };

    /// 将两个存储块执行按位异或。
    struct XorOperation {
        /// 返回 left 与 right 的按位异或结果。
        block_type operator()(block_type left, block_type right) const {
            return static_cast<block_type>(left ^ right);
        }
    };

    /// 从 start 开始按块查找目标位值，仅在命中块内逐位定位。
    static size_type findFrom(
        const Access& access,
        size_type bitCount,
        size_type start,
        bool value
    ) {
        if (start >= bitCount)
            return bitCount;

        const size_type bitsPerBlock = Layout::bitsPerBlock();
        const size_type blocks = Layout::blocksFor(bitCount);
        size_type block = Layout::blockIndex(start);
        size_type offset = start % bitsPerBlock;

        for (; block < blocks; ++block) {
            block_type valid = Layout::fullBlock();
            if (block + 1 == blocks) {
                const size_type tailBits = bitCount % bitsPerBlock;
                if (tailBits != 0)
                    valid = Layout::validMask(tailBits);
            }

            block_type candidate = access.readBlock(block);
            candidate = value
                ? static_cast<block_type>(candidate & valid)
                : static_cast<block_type>(static_cast<block_type>(~candidate) & valid);
            if (offset != 0) {
                candidate = static_cast<block_type>(
                    candidate & static_cast<block_type>(Layout::fullBlock() >> offset)
                );
            }

            if (candidate != 0) {
                for (size_type bit = offset; bit < bitsPerBlock; ++bit) {
                    if ((candidate & Layout::mask(bit)) != 0) {
                        const size_type result = block * bitsPerBlock + bit;
                        return result < bitCount ? result : bitCount;
                    }
                }
            }
            offset = 0;
        }
        return bitCount;
    }

    /// 使用 Kernighan 算法统计单个块中的置位数量。
    static size_type popcount(block_type value) noexcept {
        size_type result = 0;
        while (value != 0) {
            value = static_cast<block_type>(value & static_cast<block_type>(value - 1));
            ++result;
        }
        return result;
    }

    /// 对每个有效存储块执行二元按位操作，并在提交后规范化尾块。
    template<typename RightAccess, typename Operation>
    static void combine(
        Access& destination,
        const RightAccess& right,
        size_type bitCount,
        Operation operation
    ) {
        const size_type blocks = Layout::blocksFor(bitCount);
        for (size_type block = 0; block < blocks; ++block) {
            destination.writeBlock(
                block,
                operation(destination.readBlock(block), right.readBlock(block))
            );
        }
        normalizeTail(destination, bitCount);
    }
};

} // namespace core
} // namespace dsa

#endif
