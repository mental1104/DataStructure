#ifndef DSA_BITMAP_BITMAP_H
#define DSA_BITMAP_BITMAP_H

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

#include <dsa/core/bitmap/BitmapAlgorithm.h>

/// 教学版动态位图：保留原始类名、头文件、MSB-first 字节格式与自动扩容 API。
class Bitmap {
private:
    typedef unsigned char block_type;
    typedef std::size_t size_type;

    class Storage;
    class ConstStorage;

    typedef dsa::core::BitmapAlgorithm<Storage> Algorithm;
    typedef dsa::core::BitmapAlgorithm<ConstStorage> ConstAlgorithm;

    block_type* M;
    size_type N;
    size_type bitSize_;

    /// 校验构造参数并转换为无符号逻辑位数。
    static size_type checkedBitCount(int bitCount);

    /// 校验位下标，阻止负下标访问底层字节数组。
    static size_type checkedIndex(int index);

    /// 申请零初始化字节数组；成功后才替换旧存储，提供强异常保证。
    void replaceStorage(size_type bitCount);

protected:
    /// 重新初始化为 n 个逻辑位的空位图；替换过程不会泄漏旧存储。
    void init(int n);

public:
    /// 创建包含 n 个逻辑位的空位图；默认保持原教学实现的 8 位容量。
    explicit Bitmap(int n = 8);

    /// 从原始 MSB-first 字节文件加载 n 个逻辑位；文件不足部分保持为 0。
    Bitmap(const char* file, int n = 8);

    /// 深拷贝字节存储，避免默认浅拷贝导致双重释放。
    Bitmap(const Bitmap& other);

    /// 转移字节存储所有权，并把源对象收敛为合法空状态。
    Bitmap(Bitmap&& other) noexcept;

    /// 使用 copy-and-swap 提供强异常保证的深拷贝赋值。
    Bitmap& operator=(const Bitmap& other);

    /// 释放旧存储后接管源对象，并保持源对象可析构、可重新赋值。
    Bitmap& operator=(Bitmap&& other) noexcept;

    /// 释放教学版拥有的字节数组。
    ~Bitmap();

    /// 返回当前逻辑位数量；非 const 查询扩容会增长该值。
    size_type size() const noexcept;

    /// 返回当前字节数组可容纳的位数，可能略大于逻辑位数量。
    size_type capacity() const noexcept;

    /// 返回底层原始字节数量，便于验证文件兼容性。
    size_type byteSize() const noexcept;

    /// 判断逻辑位图是否为空。
    bool empty() const noexcept;

    /// 交换两个教学位图的存储所有权。
    void swap(Bitmap& other) noexcept;

    /// 将第 k 位置 1；保持旧 API 的按需自动扩容行为。
    void set(int k);

    /// 将第 k 位清零；保持旧 API 的按需自动扩容行为。
    void clear(int k);

    /// 翻转第 k 位；保持教学版按需自动扩容行为。
    void flip(int k);

    /// 查询第 k 位；为兼容旧实现，非 const 对象查询越界位置时仍会扩容。
    bool test(int k);

    /// 查询第 k 位但不扩容；超出逻辑范围时返回 false。
    bool test(int k) const;

    /// 将所有逻辑位清零，不改变位图大小和已分配容量。
    void clear() noexcept;

    /// 将所有逻辑位置 1，并保持尾字节未使用位为 0。
    void set() noexcept;

    /// 翻转所有逻辑位，并保持尾字节未使用位为 0。
    void flip() noexcept;

    /// 返回逻辑范围内置位的数量。
    size_type count() const noexcept;

    /// 判断逻辑范围内是否至少存在一个置位。
    bool any() const noexcept;

    /// 判断逻辑范围内是否没有置位。
    bool none() const noexcept;

    /// 判断逻辑范围内是否全部置位；空位图返回 true。
    bool all() const noexcept;

    /// 将原始字节写入文件，保持旧版 dump 的静默失败语义。
    void dump(const char* file) const;

    /// 生成前 n 位的 0/1 C 字符串；返回值所有权仍由调用方负责 delete[]。
    char* bits2string(int n);

    /// 确保第 k 位存在；扩容先分配再提交，失败时保留原位图。
    void expand(int k);

    /// 打印前 n 个逻辑位；保持旧教学 API。
    void print(int n);
};

/// 将教学版字节数组适配为共享位图算法所需的块读写接口。
class Bitmap::Storage {
public:
    typedef Bitmap::block_type block_type;
    typedef Bitmap::size_type size_type;

    /// 绑定可写教学位图，不取得其存储所有权。
    explicit Storage(Bitmap& owner) noexcept;

    /// 返回当前已分配字节块数量。
    size_type blockCount() const noexcept;

    /// 读取指定字节块。
    block_type readBlock(size_type index) const noexcept;

    /// 写回指定字节块。
    void writeBlock(size_type index, block_type value) noexcept;

private:
    Bitmap& owner_;
};

/// 为 const 查询提供只读块访问，避免查询隐式修改存储。
class Bitmap::ConstStorage {
public:
    typedef Bitmap::block_type block_type;
    typedef Bitmap::size_type size_type;

    /// 绑定只读教学位图，不取得其存储所有权。
    explicit ConstStorage(const Bitmap& owner) noexcept;

    /// 返回当前已分配字节块数量。
    size_type blockCount() const noexcept;

    /// 读取指定字节块。
    block_type readBlock(size_type index) const noexcept;

private:
    const Bitmap& owner_;
};

inline Bitmap::Storage::Storage(Bitmap& owner) noexcept
    : owner_(owner) {
}

inline Bitmap::Storage::size_type Bitmap::Storage::blockCount() const noexcept {
    return owner_.N;
}

inline Bitmap::Storage::block_type Bitmap::Storage::readBlock(size_type index) const noexcept {
    return owner_.M[index];
}

inline void Bitmap::Storage::writeBlock(size_type index, block_type value) noexcept {
    owner_.M[index] = value;
}

inline Bitmap::ConstStorage::ConstStorage(const Bitmap& owner) noexcept
    : owner_(owner) {
}

inline Bitmap::ConstStorage::size_type Bitmap::ConstStorage::blockCount() const noexcept {
    return owner_.N;
}

inline Bitmap::ConstStorage::block_type Bitmap::ConstStorage::readBlock(size_type index) const noexcept {
    return owner_.M[index];
}

inline Bitmap::size_type Bitmap::checkedBitCount(int bitCount) {
    if (bitCount < 0)
        throw std::invalid_argument("Bitmap bit count cannot be negative");
    return static_cast<size_type>(bitCount);
}

inline Bitmap::size_type Bitmap::checkedIndex(int index) {
    if (index < 0)
        throw std::out_of_range("Bitmap index cannot be negative");
    return static_cast<size_type>(index);
}

inline void Bitmap::replaceStorage(size_type bitCount) {
    const size_type byteCount = dsa::core::BitmapLayout<block_type>::blocksFor(bitCount);
    block_type* replacement = byteCount == 0 ? nullptr : new block_type[byteCount]();
    delete[] M;
    M = replacement;
    N = byteCount;
    bitSize_ = bitCount;
}

inline void Bitmap::init(int n) {
    replaceStorage(checkedBitCount(n));
}

inline Bitmap::Bitmap(int n)
    : M(nullptr), N(0), bitSize_(0) {
    init(n);
}

inline Bitmap::Bitmap(const char* file, int n)
    : M(nullptr), N(0), bitSize_(0) {
    init(n);
    if (file == nullptr || N == 0)
        return;

    std::ifstream input(file, std::ios::binary);
    if (input) {
        input.read(
            reinterpret_cast<char*>(M),
            static_cast<std::streamsize>(N)
        );
    }
    Storage storage(*this);
    Algorithm::normalizeTail(storage, bitSize_);
}

inline Bitmap::Bitmap(const Bitmap& other)
    : M(nullptr), N(0), bitSize_(0) {
    if (other.N != 0) {
        M = new block_type[other.N];
        std::memcpy(M, other.M, other.N);
    }
    N = other.N;
    bitSize_ = other.bitSize_;
}

inline Bitmap::Bitmap(Bitmap&& other) noexcept
    : M(other.M), N(other.N), bitSize_(other.bitSize_) {
    other.M = nullptr;
    other.N = 0;
    other.bitSize_ = 0;
}

inline Bitmap& Bitmap::operator=(const Bitmap& other) {
    if (this != &other) {
        Bitmap copy(other);
        swap(copy);
    }
    return *this;
}

inline Bitmap& Bitmap::operator=(Bitmap&& other) noexcept {
    if (this != &other) {
        delete[] M;
        M = other.M;
        N = other.N;
        bitSize_ = other.bitSize_;
        other.M = nullptr;
        other.N = 0;
        other.bitSize_ = 0;
    }
    return *this;
}

inline Bitmap::~Bitmap() {
    delete[] M;
}

inline Bitmap::size_type Bitmap::size() const noexcept {
    return bitSize_;
}

inline Bitmap::size_type Bitmap::capacity() const noexcept {
    return N * dsa::core::BitmapLayout<block_type>::bitsPerBlock();
}

inline Bitmap::size_type Bitmap::byteSize() const noexcept {
    return N;
}

inline bool Bitmap::empty() const noexcept {
    return bitSize_ == 0;
}

inline void Bitmap::swap(Bitmap& other) noexcept {
    using std::swap;
    swap(M, other.M);
    swap(N, other.N);
    swap(bitSize_, other.bitSize_);
}

inline void Bitmap::set(int k) {
    const size_type index = checkedIndex(k);
    expand(k);
    Storage storage(*this);
    Algorithm::set(storage, index);
}

inline void Bitmap::clear(int k) {
    const size_type index = checkedIndex(k);
    expand(k);
    Storage storage(*this);
    Algorithm::reset(storage, index);
}

inline void Bitmap::flip(int k) {
    const size_type index = checkedIndex(k);
    expand(k);
    Storage storage(*this);
    Algorithm::flip(storage, index);
}

inline bool Bitmap::test(int k) {
    const size_type index = checkedIndex(k);
    expand(k);
    Storage storage(*this);
    return Algorithm::test(storage, index);
}

inline bool Bitmap::test(int k) const {
    const size_type index = checkedIndex(k);
    if (index >= bitSize_)
        return false;
    ConstStorage storage(*this);
    return ConstAlgorithm::test(storage, index);
}

inline void Bitmap::clear() noexcept {
    Storage storage(*this);
    Algorithm::resetAll(storage);
}

inline void Bitmap::set() noexcept {
    Storage storage(*this);
    Algorithm::setAll(storage, bitSize_);
}

inline void Bitmap::flip() noexcept {
    Storage storage(*this);
    Algorithm::flipAll(storage, bitSize_);
}

inline Bitmap::size_type Bitmap::count() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::count(storage, bitSize_);
}

inline bool Bitmap::any() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::any(storage, bitSize_);
}

inline bool Bitmap::none() const noexcept {
    return !any();
}

inline bool Bitmap::all() const noexcept {
    ConstStorage storage(*this);
    return ConstAlgorithm::all(storage, bitSize_);
}

inline void Bitmap::dump(const char* file) const {
    if (file == nullptr)
        return;
    std::ofstream output(file, std::ios::binary);
    if (output && N != 0) {
        output.write(
            reinterpret_cast<const char*>(M),
            static_cast<std::streamsize>(N)
        );
    }
}

inline char* Bitmap::bits2string(int n) {
    const size_type length = checkedBitCount(n);
    if (length != 0)
        expand(n - 1);

    char* result = new char[length + 1];
    result[length] = '\0';
    for (size_type index = 0; index < length; ++index)
        result[index] = test(static_cast<int>(index)) ? '1' : '0';
    return result;
}

inline void Bitmap::expand(int k) {
    const size_type index = checkedIndex(k);
    const size_type requiredBits = index + 1;
    if (requiredBits <= capacity()) {
        if (requiredBits > bitSize_)
            bitSize_ = requiredBits;
        return;
    }

    const size_type maximumBits = static_cast<size_type>((std::numeric_limits<int>::max)());
    if (requiredBits > maximumBits)
        throw std::length_error("Bitmap size exceeds legacy int range");

    size_type grownBits = capacity() == 0 ? 8 : capacity();
    if (grownBits > maximumBits - grownBits)
        grownBits = maximumBits;
    else
        grownBits += grownBits;
    if (grownBits < requiredBits)
        grownBits = requiredBits;

    const size_type newByteCount = dsa::core::BitmapLayout<block_type>::blocksFor(grownBits);
    block_type* replacement = new block_type[newByteCount]();
    if (N != 0)
        std::memcpy(replacement, M, N);

    delete[] M;
    M = replacement;
    N = newByteCount;
    bitSize_ = requiredBits;
}

inline void Bitmap::print(int n) {
    const size_type length = checkedBitCount(n);
    if (length != 0)
        expand(n - 1);
    for (size_type index = 0; index < length; ++index)
        std::putchar(test(static_cast<int>(index)) ? '1' : '0');
}

/// 交换两个教学位图的存储所有权。
inline void swap(Bitmap& left, Bitmap& right) noexcept {
    left.swap(right);
}

#endif
