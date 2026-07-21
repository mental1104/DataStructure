#ifndef DSA_CONTAINER_BITMAP_BITMAP_H
#define DSA_CONTAINER_BITMAP_BITMAP_H

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <dsa/container/vector/Vector.h>
#include <dsa/core/bitmap/BitmapAlgorithm.h>

namespace dsa {
namespace container {

/// allocator-aware 动态位图；以 64 位块压缩存储 bool，并提供代理引用与随机访问迭代器。
template<typename Allocator = std::allocator<bool> >
class Bitmap {
public:
    typedef bool value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::uint64_t word_type;

private:
    typedef std::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::template rebind_alloc<word_type> word_allocator_type;
    typedef dsa::container::Vector<word_type, word_allocator_type> storage_type;
    typedef dsa::core::BitmapLayout<word_type> Layout;

    class Storage;
    class ConstStorage;

    typedef dsa::core::BitmapAlgorithm<Storage> Algorithm;
    typedef dsa::core::BitmapAlgorithm<ConstStorage> ConstAlgorithm;

public:
    /// 表示单个位的可修改代理；其生命周期不得超过所属位图或导致重分配的操作。
    class reference {
    public:
        /// 复制代理绑定关系，不复制或修改实际位值。
        reference(const reference& other) noexcept;

        /// 将代理指向的位赋值为 value。
        reference& operator=(bool value) noexcept;

        /// 从另一个位代理读取布尔值后赋值。
        reference& operator=(const reference& other) noexcept;

        /// 将代理转换为普通 bool。
        operator bool() const noexcept;

        /// 返回当前位的逻辑非值。
        bool operator~() const noexcept;

        /// 原地翻转当前位。
        reference& flip() noexcept;

        /// 交换两个代理指向的逻辑位值。
        friend void swap(reference left, reference right) noexcept {
            const bool temporary = static_cast<bool>(left);
            left = static_cast<bool>(right);
            right = temporary;
        }

    private:
        friend class Bitmap;

        /// 绑定一个存储块和位掩码，仅由所属 Bitmap 创建。
        reference(word_type& word, word_type mask) noexcept;

        word_type* word_;
        word_type mask_;
    };

    class const_iterator;

    /// 随机访问可写位迭代器；解引用返回 Bitmap::reference 代理。
    class iterator {
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef bool value_type;
        typedef typename Bitmap::difference_type difference_type;
        typedef typename Bitmap::reference reference;
        typedef void pointer;

        /// 创建未绑定迭代器，仅支持赋值和析构。
        iterator() noexcept;

        /// 解引用当前逻辑位。
        reference operator*() const noexcept;

        /// 随机访问相对偏移位置。
        reference operator[](difference_type offset) const noexcept;

        /// 前置递增到下一位。
        iterator& operator++() noexcept;

        /// 后置递增到下一位。
        iterator operator++(int) noexcept;

        /// 前置递减到前一位。
        iterator& operator--() noexcept;

        /// 后置递减到前一位。
        iterator operator--(int) noexcept;

        /// 向前移动 offset 个位置。
        iterator& operator+=(difference_type offset) noexcept;

        /// 向后移动 offset 个位置。
        iterator& operator-=(difference_type offset) noexcept;

        /// 返回向前移动后的迭代器副本。
        iterator operator+(difference_type offset) const noexcept;

        /// 返回向后移动后的迭代器副本。
        iterator operator-(difference_type offset) const noexcept;

        /// 计算同一位图内两个迭代器的逻辑距离。
        difference_type operator-(const iterator& other) const noexcept;

        /// 判断两个迭代器是否指向同一位图的同一位置。
        bool operator==(const iterator& other) const noexcept;

        /// 判断两个迭代器是否不同。
        bool operator!=(const iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator<(const iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator>(const iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator<=(const iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator>=(const iterator& other) const noexcept;

        /// 支持 offset + iterator 形式的随机访问。
        friend iterator operator+(difference_type offset, iterator current) noexcept {
            current += offset;
            return current;
        }

    private:
        friend class Bitmap;
        friend class const_iterator;

        /// 绑定所属位图和逻辑下标，仅由 Bitmap 创建。
        iterator(Bitmap* owner, size_type index) noexcept;

        Bitmap* owner_;
        size_type index_;
    };

    /// 随机访问只读位迭代器；解引用返回普通 bool。
    class const_iterator {
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef bool value_type;
        typedef typename Bitmap::difference_type difference_type;
        typedef bool reference;
        typedef void pointer;

        /// 创建未绑定迭代器，仅支持赋值和析构。
        const_iterator() noexcept;

        /// 从可写迭代器转换为只读迭代器。
        const_iterator(const iterator& other) noexcept;

        /// 解引用当前逻辑位。
        bool operator*() const noexcept;

        /// 随机访问相对偏移位置。
        bool operator[](difference_type offset) const noexcept;

        /// 前置递增到下一位。
        const_iterator& operator++() noexcept;

        /// 后置递增到下一位。
        const_iterator operator++(int) noexcept;

        /// 前置递减到前一位。
        const_iterator& operator--() noexcept;

        /// 后置递减到前一位。
        const_iterator operator--(int) noexcept;

        /// 向前移动 offset 个位置。
        const_iterator& operator+=(difference_type offset) noexcept;

        /// 向后移动 offset 个位置。
        const_iterator& operator-=(difference_type offset) noexcept;

        /// 返回向前移动后的迭代器副本。
        const_iterator operator+(difference_type offset) const noexcept;

        /// 返回向后移动后的迭代器副本。
        const_iterator operator-(difference_type offset) const noexcept;

        /// 计算同一位图内两个迭代器的逻辑距离。
        difference_type operator-(const const_iterator& other) const noexcept;

        /// 判断两个迭代器是否指向同一位图的同一位置。
        bool operator==(const const_iterator& other) const noexcept;

        /// 判断两个迭代器是否不同。
        bool operator!=(const const_iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator<(const const_iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator>(const const_iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator<=(const const_iterator& other) const noexcept;

        /// 比较同一位图内的逻辑位置顺序。
        bool operator>=(const const_iterator& other) const noexcept;

        /// 支持 offset + const_iterator 形式的随机访问。
        friend const_iterator operator+(
            difference_type offset,
            const_iterator current
        ) noexcept {
            current += offset;
            return current;
        }

    private:
        friend class Bitmap;

        /// 绑定所属位图和逻辑下标，仅由 Bitmap 创建。
        const_iterator(const Bitmap* owner, size_type index) noexcept;

        const Bitmap* owner_;
        size_type index_;
    };

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = static_cast<size_type>(-1);

    /// 创建空位图，并绑定指定 allocator。
    explicit Bitmap(const allocator_type& allocator = allocator_type());

    /// 创建 count 位位图，并将所有逻辑位初始化为 value。
    explicit Bitmap(
        size_type count,
        bool value = false,
        const allocator_type& allocator = allocator_type()
    );

    /// 从输入迭代器逐位构造位图；迭代器值会转换为 bool。
    template<typename InputIterator>
    Bitmap(
        InputIterator first,
        InputIterator last,
        const allocator_type& allocator = allocator_type(),
        typename std::enable_if<!std::is_integral<InputIterator>::value>::type* = nullptr
    );

    /// 从 bool 初始化列表按顺序构造位图。
    Bitmap(
        std::initializer_list<bool> values,
        const allocator_type& allocator = allocator_type()
    );

    /// 深拷贝逻辑大小和压缩存储；allocator 选择遵循 allocator_traits。
    Bitmap(const Bitmap& other);

    /// 使用指定 allocator 深拷贝位图。
    Bitmap(const Bitmap& other, const allocator_type& allocator);

    /// 转移压缩存储所有权，并将源对象收敛为空位图。
    Bitmap(Bitmap&& other) noexcept(noexcept(storage_type(std::move(other.words_))));

    /// 使用指定 allocator 移动构造；allocator 不兼容时由 vector 迁移存储块。
    Bitmap(Bitmap&& other, const allocator_type& allocator);

    /// 由底层 vector 自动释放压缩存储。
    ~Bitmap();

    /// 深拷贝赋值；只有底层块赋值成功后才提交逻辑大小。
    Bitmap& operator=(const Bitmap& other);

    /// 移动赋值并将源对象收敛为空位图；异常规格由底层 allocator 决定。
    Bitmap& operator=(Bitmap&& other) noexcept(noexcept(words_ = std::move(other.words_)));

    /// 使用初始化列表替换当前逻辑位序列。
    Bitmap& operator=(std::initializer_list<bool> values);

    /// 返回当前 allocator。
    allocator_type get_allocator() const;

    /// 返回逻辑位数量。
    size_type size() const noexcept;

    /// 返回底层已保留容量可容纳的逻辑位数量。
    size_type capacity() const noexcept;

    /// 返回 allocator 和 vector 能支持的最大逻辑位数量。
    size_type max_size() const noexcept;

    /// 判断位图是否为空。
    bool empty() const noexcept;

    /// 预留至少 bitCapacity 个逻辑位的存储；重分配会使全部迭代器和代理失效。
    void reserve(size_type bitCapacity);

    /// 释放未使用的块容量；若发生重分配，全部迭代器和代理失效。
    void shrink_to_fit();

    /// 调整逻辑位数量；新增位初始化为 value，原有位保持不变。
    void resize(size_type count, bool value = false);

    /// 在末尾追加一个逻辑位；发生重分配时全部迭代器和代理失效。
    void push_back(bool value);

    /// 删除末尾逻辑位；空位图调用会抛出 out_of_range。
    void pop_back();

    /// 删除全部逻辑位和已构造块；保留 vector 可能持有的容量。
    void clear() noexcept;

    /// 返回可写随机访问迭代器起点。
    iterator begin() noexcept;

    /// 返回可写随机访问迭代器终点。
    iterator end() noexcept;

    /// 返回只读随机访问迭代器起点。
    const_iterator begin() const noexcept;

    /// 返回只读随机访问迭代器终点。
    const_iterator end() const noexcept;

    /// 返回只读随机访问迭代器起点。
    const_iterator cbegin() const noexcept;

    /// 返回只读随机访问迭代器终点。
    const_iterator cend() const noexcept;

    /// 返回反向可写迭代器起点。
    reverse_iterator rbegin() noexcept;

    /// 返回反向可写迭代器终点。
    reverse_iterator rend() noexcept;

    /// 返回反向只读迭代器起点。
    const_reverse_iterator rbegin() const noexcept;

    /// 返回反向只读迭代器终点。
    const_reverse_iterator rend() const noexcept;

    /// 返回反向只读迭代器起点。
    const_reverse_iterator crbegin() const noexcept;

    /// 返回反向只读迭代器终点。
    const_reverse_iterator crend() const noexcept;

    /// 未检查下标访问，返回可写代理；越界行为与标准容器 operator[] 一致为未定义。
    reference operator[](size_type position) noexcept;

    /// 未检查下标访问，返回普通 bool；越界行为与标准容器 operator[] 一致为未定义。
    bool operator[](size_type position) const noexcept;

    /// 检查下标后返回逻辑位值。
    bool test(size_type position) const;

    /// 检查下标后将逻辑位置为 value。
    Bitmap& set(size_type position, bool value = true);

    /// 检查下标后清除逻辑位。
    Bitmap& reset(size_type position);

    /// 检查下标后翻转逻辑位。
    Bitmap& flip(size_type position);

    /// 将全部逻辑位置 1。
    Bitmap& set() noexcept;

    /// 将全部逻辑位清零。
    Bitmap& reset() noexcept;

    /// 翻转全部逻辑位，并清理尾块未使用位。
    Bitmap& flip() noexcept;

    /// 返回置位数量。
    size_type count() const noexcept;

    /// 判断是否至少存在一个置位。
    bool any() const noexcept;

    /// 判断是否不存在置位。
    bool none() const noexcept;

    /// 判断是否所有逻辑位均置位；空位图返回 true。
    bool all() const noexcept;

    /// 返回第一个置位下标；未找到时返回 npos。
    size_type find_first() const noexcept;

    /// 返回 previous 之后的第一个置位下标；未找到时返回 npos。
    size_type find_next(size_type previous) const noexcept;

    /// 与等长位图执行按位与；长度不一致时抛出 invalid_argument。
    Bitmap& operator&=(const Bitmap& other);

    /// 与等长位图执行按位或；长度不一致时抛出 invalid_argument。
    Bitmap& operator|=(const Bitmap& other);

    /// 与等长位图执行按位异或；长度不一致时抛出 invalid_argument。
    Bitmap& operator^=(const Bitmap& other);

    /// 将逻辑位转换为字符串；下标 0 对应字符串首字符。
    std::string to_string(char zero = '0', char one = '1') const;

    /// 交换逻辑大小、存储和 allocator；不传播且 allocator 不等时通过迁移保持所有权正确。
    void swap(Bitmap& other);

private:
    storage_type words_;
    size_type size_;

    /// 返回指定逻辑位的可写代理，不做边界检查。
    reference referenceAt(size_type position) noexcept;

    /// 返回指定逻辑位值，不做边界检查。
    bool valueAt(size_type position) const noexcept;

    /// 校验逻辑下标并抛出标准越界异常。
    void checkPosition(size_type position) const;

    /// 校验二元位运算两侧逻辑长度一致。
    void ensureSameSize(const Bitmap& other) const;
};

/// 将工业版 64 位块数组适配为共享位图算法所需的读写接口。
template<typename Allocator>
class Bitmap<Allocator>::Storage {
public:
    typedef typename Bitmap::word_type block_type;
    typedef typename Bitmap::size_type size_type;

    /// 绑定可写工业位图，不取得其存储所有权。
    explicit Storage(Bitmap& owner) noexcept;

    /// 返回当前 64 位存储块数量。
    size_type blockCount() const noexcept;

    /// 读取指定 64 位块。
    block_type readBlock(size_type index) const noexcept;

    /// 写回指定 64 位块。
    void writeBlock(size_type index, block_type value) noexcept;

private:
    Bitmap& owner_;
};

/// 为 const 操作提供只读 64 位块访问。
template<typename Allocator>
class Bitmap<Allocator>::ConstStorage {
public:
    typedef typename Bitmap::word_type block_type;
    typedef typename Bitmap::size_type size_type;

    /// 绑定只读工业位图，不取得其存储所有权。
    explicit ConstStorage(const Bitmap& owner) noexcept;

    /// 返回当前 64 位存储块数量。
    size_type blockCount() const noexcept;

    /// 读取指定 64 位块。
    block_type readBlock(size_type index) const noexcept;

private:
    const Bitmap& owner_;
};

template<typename Allocator>
Bitmap<Allocator>::Storage::Storage(Bitmap& owner) noexcept
    : owner_(owner) {
}

template<typename Allocator>
typename Bitmap<Allocator>::Storage::size_type
Bitmap<Allocator>::Storage::blockCount() const noexcept {
    return owner_.words_.size();
}

template<typename Allocator>
typename Bitmap<Allocator>::Storage::block_type
Bitmap<Allocator>::Storage::readBlock(size_type index) const noexcept {
    return owner_.words_[index];
}

template<typename Allocator>
void Bitmap<Allocator>::Storage::writeBlock(size_type index, block_type value) noexcept {
    owner_.words_[index] = value;
}

template<typename Allocator>
Bitmap<Allocator>::ConstStorage::ConstStorage(const Bitmap& owner) noexcept
    : owner_(owner) {
}

template<typename Allocator>
typename Bitmap<Allocator>::ConstStorage::size_type
Bitmap<Allocator>::ConstStorage::blockCount() const noexcept {
    return owner_.words_.size();
}

template<typename Allocator>
typename Bitmap<Allocator>::ConstStorage::block_type
Bitmap<Allocator>::ConstStorage::readBlock(size_type index) const noexcept {
    return owner_.words_[index];
}

template<typename Allocator>
Bitmap<Allocator>::reference::reference(word_type& word, word_type mask) noexcept
    : word_(&word), mask_(mask) {
}

template<typename Allocator>
Bitmap<Allocator>::reference::reference(const reference& other) noexcept
    : word_(other.word_), mask_(other.mask_) {
}

template<typename Allocator>
typename Bitmap<Allocator>::reference&
Bitmap<Allocator>::reference::operator=(bool value) noexcept {
    if (value)
        *word_ |= mask_;
    else
        *word_ &= ~mask_;
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::reference&
Bitmap<Allocator>::reference::operator=(const reference& other) noexcept {
    return *this = static_cast<bool>(other);
}

template<typename Allocator>
Bitmap<Allocator>::reference::operator bool() const noexcept {
    return (*word_ & mask_) != 0;
}

template<typename Allocator>
bool Bitmap<Allocator>::reference::operator~() const noexcept {
    return !static_cast<bool>(*this);
}

template<typename Allocator>
typename Bitmap<Allocator>::reference& Bitmap<Allocator>::reference::flip() noexcept {
    *word_ ^= mask_;
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>::iterator::iterator() noexcept
    : owner_(nullptr), index_(0) {
}

template<typename Allocator>
Bitmap<Allocator>::iterator::iterator(Bitmap* owner, size_type index) noexcept
    : owner_(owner), index_(index) {
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator::reference
Bitmap<Allocator>::iterator::operator*() const noexcept {
    return owner_->referenceAt(index_);
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator::reference
Bitmap<Allocator>::iterator::operator[](difference_type offset) const noexcept {
    return owner_->referenceAt(static_cast<size_type>(
        static_cast<difference_type>(index_) + offset
    ));
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator& Bitmap<Allocator>::iterator::operator++() noexcept {
    ++index_;
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator Bitmap<Allocator>::iterator::operator++(int) noexcept {
    iterator copy(*this);
    ++(*this);
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator& Bitmap<Allocator>::iterator::operator--() noexcept {
    --index_;
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator Bitmap<Allocator>::iterator::operator--(int) noexcept {
    iterator copy(*this);
    --(*this);
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator&
Bitmap<Allocator>::iterator::operator+=(difference_type offset) noexcept {
    index_ = static_cast<size_type>(static_cast<difference_type>(index_) + offset);
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator&
Bitmap<Allocator>::iterator::operator-=(difference_type offset) noexcept {
    return *this += -offset;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator
Bitmap<Allocator>::iterator::operator+(difference_type offset) const noexcept {
    iterator copy(*this);
    copy += offset;
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator
Bitmap<Allocator>::iterator::operator-(difference_type offset) const noexcept {
    iterator copy(*this);
    copy -= offset;
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator::difference_type
Bitmap<Allocator>::iterator::operator-(const iterator& other) const noexcept {
    return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator==(const iterator& other) const noexcept {
    return owner_ == other.owner_ && index_ == other.index_;
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator!=(const iterator& other) const noexcept {
    return !(*this == other);
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator<(const iterator& other) const noexcept {
    return index_ < other.index_;
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator>(const iterator& other) const noexcept {
    return other < *this;
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator<=(const iterator& other) const noexcept {
    return !(other < *this);
}

template<typename Allocator>
bool Bitmap<Allocator>::iterator::operator>=(const iterator& other) const noexcept {
    return !(*this < other);
}

template<typename Allocator>
Bitmap<Allocator>::const_iterator::const_iterator() noexcept
    : owner_(nullptr), index_(0) {
}

template<typename Allocator>
Bitmap<Allocator>::const_iterator::const_iterator(const iterator& other) noexcept
    : owner_(other.owner_), index_(other.index_) {
}

template<typename Allocator>
Bitmap<Allocator>::const_iterator::const_iterator(
    const Bitmap* owner,
    size_type index
) noexcept
    : owner_(owner), index_(index) {
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator*() const noexcept {
    return owner_->valueAt(index_);
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator[](difference_type offset) const noexcept {
    return owner_->valueAt(static_cast<size_type>(
        static_cast<difference_type>(index_) + offset
    ));
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator&
Bitmap<Allocator>::const_iterator::operator++() noexcept {
    ++index_;
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator
Bitmap<Allocator>::const_iterator::operator++(int) noexcept {
    const_iterator copy(*this);
    ++(*this);
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator&
Bitmap<Allocator>::const_iterator::operator--() noexcept {
    --index_;
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator
Bitmap<Allocator>::const_iterator::operator--(int) noexcept {
    const_iterator copy(*this);
    --(*this);
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator&
Bitmap<Allocator>::const_iterator::operator+=(difference_type offset) noexcept {
    index_ = static_cast<size_type>(static_cast<difference_type>(index_) + offset);
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator&
Bitmap<Allocator>::const_iterator::operator-=(difference_type offset) noexcept {
    return *this += -offset;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator
Bitmap<Allocator>::const_iterator::operator+(difference_type offset) const noexcept {
    const_iterator copy(*this);
    copy += offset;
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator
Bitmap<Allocator>::const_iterator::operator-(difference_type offset) const noexcept {
    const_iterator copy(*this);
    copy -= offset;
    return copy;
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator::difference_type
Bitmap<Allocator>::const_iterator::operator-(const const_iterator& other) const noexcept {
    return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator==(const const_iterator& other) const noexcept {
    return owner_ == other.owner_ && index_ == other.index_;
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator!=(const const_iterator& other) const noexcept {
    return !(*this == other);
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator<(const const_iterator& other) const noexcept {
    return index_ < other.index_;
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator>(const const_iterator& other) const noexcept {
    return other < *this;
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator<=(const const_iterator& other) const noexcept {
    return !(other < *this);
}

template<typename Allocator>
bool Bitmap<Allocator>::const_iterator::operator>=(const const_iterator& other) const noexcept {
    return !(*this < other);
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(const allocator_type& allocator)
    : words_(word_allocator_type(allocator)), size_(0) {
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(
    size_type count,
    bool value,
    const allocator_type& allocator
)
    : words_(
          Layout::blocksFor(count),
          value ? Layout::fullBlock() : static_cast<word_type>(0),
          word_allocator_type(allocator)
      ),
      size_(count) {
    Storage storage(*this);
    Algorithm::normalizeTail(storage, size_);
}

template<typename Allocator>
template<typename InputIterator>
Bitmap<Allocator>::Bitmap(
    InputIterator first,
    InputIterator last,
    const allocator_type& allocator,
    typename std::enable_if<!std::is_integral<InputIterator>::value>::type*
)
    : words_(word_allocator_type(allocator)), size_(0) {
    for (; first != last; ++first)
        push_back(static_cast<bool>(*first));
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(
    std::initializer_list<bool> values,
    const allocator_type& allocator
)
    : words_(word_allocator_type(allocator)), size_(0) {
    reserve(values.size());
    for (typename std::initializer_list<bool>::const_iterator it = values.begin();
         it != values.end();
         ++it) {
        push_back(*it);
    }
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(const Bitmap& other)
    : words_(other.words_), size_(other.size_) {
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(const Bitmap& other, const allocator_type& allocator)
    : words_(other.words_.begin(), other.words_.end(), word_allocator_type(allocator)),
      size_(other.size_) {
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(Bitmap&& other)
    noexcept(noexcept(storage_type(std::move(other.words_))))
    : words_(std::move(other.words_)), size_(other.size_) {
    other.words_.clear();
    other.size_ = 0;
}

template<typename Allocator>
Bitmap<Allocator>::Bitmap(Bitmap&& other, const allocator_type& allocator)
    : words_(std::move(other.words_), word_allocator_type(allocator)),
      size_(other.size_) {
    other.words_.clear();
    other.size_ = 0;
}

template<typename Allocator>
Bitmap<Allocator>::~Bitmap() = default;

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator=(const Bitmap& other) {
    if (this != &other) {
        words_ = other.words_;
        size_ = other.size_;
    }
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator=(Bitmap&& other)
    noexcept(noexcept(words_ = std::move(other.words_))) {
    if (this != &other) {
        words_ = std::move(other.words_);
        size_ = other.size_;
        other.words_.clear();
        other.size_ = 0;
    }
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator=(std::initializer_list<bool> values) {
    Bitmap replacement(values, get_allocator());
    swap(replacement);
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::allocator_type Bitmap<Allocator>::get_allocator() const {
    return allocator_type(words_.get_allocator());
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type Bitmap<Allocator>::size() const noexcept {
    return size_;
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type Bitmap<Allocator>::capacity() const noexcept {
    const size_type blockCapacity = words_.capacity();
    if (blockCapacity > (std::numeric_limits<size_type>::max)() / Layout::bitsPerBlock())
        return (std::numeric_limits<size_type>::max)();
    return blockCapacity * Layout::bitsPerBlock();
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type Bitmap<Allocator>::max_size() const noexcept {
    const size_type blockMaximum = words_.max_size();
    if (blockMaximum > (std::numeric_limits<size_type>::max)() / Layout::bitsPerBlock())
        return (std::numeric_limits<size_type>::max)();
    return blockMaximum * Layout::bitsPerBlock();
}

template<typename Allocator>
bool Bitmap<Allocator>::empty() const noexcept {
    return size_ == 0;
}

template<typename Allocator>
void Bitmap<Allocator>::reserve(size_type bitCapacity) {
    if (bitCapacity > max_size())
        throw std::length_error("Bitmap capacity exceeds max_size");
    words_.reserve(Layout::blocksFor(bitCapacity));
}

template<typename Allocator>
void Bitmap<Allocator>::shrink_to_fit() {
    words_.shrink_to_fit();
}

template<typename Allocator>
void Bitmap<Allocator>::resize(size_type count, bool value) {
    if (count > max_size())
        throw std::length_error("Bitmap size exceeds max_size");
    if (count == size_)
        return;

    const size_type oldSize = size_;
    words_.resize(Layout::blocksFor(count), static_cast<word_type>(0));

    if (count > oldSize && value) {
        Storage storage(*this);
        for (size_type index = oldSize; index < count; ++index)
            Algorithm::set(storage, index);
    }

    size_ = count;
    Storage storage(*this);
    Algorithm::normalizeTail(storage, size_);
}

template<typename Allocator>
void Bitmap<Allocator>::push_back(bool value) {
    if (size_ == max_size())
        throw std::length_error("Bitmap size exceeds max_size");
    resize(size_ + 1, value);
}

template<typename Allocator>
void Bitmap<Allocator>::pop_back() {
    if (empty())
        throw std::out_of_range("Bitmap::pop_back on empty bitmap");
    resize(size_ - 1);
}

template<typename Allocator>
void Bitmap<Allocator>::clear() noexcept {
    words_.clear();
    size_ = 0;
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator Bitmap<Allocator>::begin() noexcept {
    return iterator(this, 0);
}

template<typename Allocator>
typename Bitmap<Allocator>::iterator Bitmap<Allocator>::end() noexcept {
    return iterator(this, size_);
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator Bitmap<Allocator>::begin() const noexcept {
    return const_iterator(this, 0);
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator Bitmap<Allocator>::end() const noexcept {
    return const_iterator(this, size_);
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator Bitmap<Allocator>::cbegin() const noexcept {
    return const_iterator(this, 0);
}

template<typename Allocator>
typename Bitmap<Allocator>::const_iterator Bitmap<Allocator>::cend() const noexcept {
    return const_iterator(this, size_);
}

template<typename Allocator>
typename Bitmap<Allocator>::reverse_iterator Bitmap<Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

template<typename Allocator>
typename Bitmap<Allocator>::reverse_iterator Bitmap<Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

template<typename Allocator>
typename Bitmap<Allocator>::const_reverse_iterator Bitmap<Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

template<typename Allocator>
typename Bitmap<Allocator>::const_reverse_iterator Bitmap<Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

template<typename Allocator>
typename Bitmap<Allocator>::const_reverse_iterator Bitmap<Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(cend());
}

template<typename Allocator>
typename Bitmap<Allocator>::const_reverse_iterator Bitmap<Allocator>::crend() const noexcept {
    return const_reverse_iterator(cbegin());
}

template<typename Allocator>
typename Bitmap<Allocator>::reference Bitmap<Allocator>::operator[](size_type position) noexcept {
    return referenceAt(position);
}

template<typename Allocator>
bool Bitmap<Allocator>::operator[](size_type position) const noexcept {
    return valueAt(position);
}

template<typename Allocator>
bool Bitmap<Allocator>::test(size_type position) const {
    checkPosition(position);
    return valueAt(position);
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::set(size_type position, bool value) {
    checkPosition(position);
    Storage storage(*this);
    Algorithm::set(storage, position, value);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::reset(size_type position) {
    checkPosition(position);
    Storage storage(*this);
    Algorithm::reset(storage, position);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::flip(size_type position) {
    checkPosition(position);
    Storage storage(*this);
    Algorithm::flip(storage, position);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::set() noexcept {
    Storage storage(*this);
    Algorithm::setAll(storage, size_);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::reset() noexcept {
    Storage storage(*this);
    Algorithm::resetAll(storage);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::flip() noexcept {
    Storage storage(*this);
    Algorithm::flipAll(storage, size_);
    return *this;
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type Bitmap<Allocator>::count() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::count(storage, size_);
}

template<typename Allocator>
bool Bitmap<Allocator>::any() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::any(storage, size_);
}

template<typename Allocator>
bool Bitmap<Allocator>::none() const noexcept {
    return !any();
}

template<typename Allocator>
bool Bitmap<Allocator>::all() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::all(storage, size_);
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type Bitmap<Allocator>::find_first() const noexcept {
    ConstStorage storage(*this);
    const size_type result = ConstAlgorithm::findFirst(storage, size_, true);
    return result == size_ ? npos : result;
}

template<typename Allocator>
typename Bitmap<Allocator>::size_type
Bitmap<Allocator>::find_next(size_type previous) const noexcept {
    ConstStorage storage(*this);
    const size_type result = ConstAlgorithm::findNext(storage, size_, previous, true);
    return result == size_ ? npos : result;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator&=(const Bitmap& other) {
    ensureSameSize(other);
    Storage left(*this);
    ConstStorage right(other);
    Algorithm::bitAnd(left, right, size_);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator|=(const Bitmap& other) {
    ensureSameSize(other);
    Storage left(*this);
    ConstStorage right(other);
    Algorithm::bitOr(left, right, size_);
    return *this;
}

template<typename Allocator>
Bitmap<Allocator>& Bitmap<Allocator>::operator^=(const Bitmap& other) {
    ensureSameSize(other);
    Storage left(*this);
    ConstStorage right(other);
    Algorithm::bitXor(left, right, size_);
    return *this;
}

template<typename Allocator>
std::string Bitmap<Allocator>::to_string(char zero, char one) const {
    std::string result(size_, zero);
    for (size_type index = 0; index < size_; ++index) {
        if (valueAt(index))
            result[index] = one;
    }
    return result;
}

template<typename Allocator>
void Bitmap<Allocator>::swap(Bitmap& other) {
    if (this == &other)
        return;

    typedef typename allocator_traits_type::propagate_on_container_swap Propagate;
    if (Propagate::value || get_allocator() == other.get_allocator()) {
        words_.swap(other.words_);
        using std::swap;
        swap(size_, other.size_);
        return;
    }

    Bitmap thisForOther(*this, other.get_allocator());
    Bitmap otherForThis(other, get_allocator());
    *this = std::move(otherForThis);
    other = std::move(thisForOther);
}

template<typename Allocator>
typename Bitmap<Allocator>::reference
Bitmap<Allocator>::referenceAt(size_type position) noexcept {
    return reference(
        words_[Layout::blockIndex(position)],
        Layout::mask(position)
    );
}

template<typename Allocator>
bool Bitmap<Allocator>::valueAt(size_type position) const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::test(storage, position);
}

template<typename Allocator>
void Bitmap<Allocator>::checkPosition(size_type position) const {
    if (position >= size_)
        throw std::out_of_range("Bitmap position out of range");
}

template<typename Allocator>
void Bitmap<Allocator>::ensureSameSize(const Bitmap& other) const {
    if (size_ != other.size_)
        throw std::invalid_argument("Bitmap bitwise operands must have equal size");
}

template<typename Allocator>
const typename Bitmap<Allocator>::size_type Bitmap<Allocator>::npos;

/// 返回两个等长位图的按位与结果。
template<typename Allocator>
Bitmap<Allocator> operator&(
    Bitmap<Allocator> left,
    const Bitmap<Allocator>& right
) {
    left &= right;
    return left;
}

/// 返回两个等长位图的按位或结果。
template<typename Allocator>
Bitmap<Allocator> operator|(
    Bitmap<Allocator> left,
    const Bitmap<Allocator>& right
) {
    left |= right;
    return left;
}

/// 返回两个等长位图的按位异或结果。
template<typename Allocator>
Bitmap<Allocator> operator^(
    Bitmap<Allocator> left,
    const Bitmap<Allocator>& right
) {
    left ^= right;
    return left;
}

/// 返回位图的逐位取反副本。
template<typename Allocator>
Bitmap<Allocator> operator~(Bitmap<Allocator> value) {
    value.flip();
    return value;
}

/// 比较两个位图的逻辑大小和全部位值。
template<typename Allocator>
bool operator==(
    const Bitmap<Allocator>& left,
    const Bitmap<Allocator>& right
) {
    if (left.size() != right.size())
        return false;
    for (typename Bitmap<Allocator>::size_type index = 0; index < left.size(); ++index) {
        if (left[index] != right[index])
            return false;
    }
    return true;
}

/// 判断两个位图是否不同。
template<typename Allocator>
bool operator!=(
    const Bitmap<Allocator>& left,
    const Bitmap<Allocator>& right
) {
    return !(left == right);
}

/// 交换两个工业位图。
template<typename Allocator>
void swap(Bitmap<Allocator>& left, Bitmap<Allocator>& right) {
    left.swap(right);
}

} // namespace container
} // namespace dsa

#endif
