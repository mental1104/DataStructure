#ifndef DSA_UF_UNION_FIND_H
#define DSA_UF_UNION_FIND_H

#include <algorithm>
#include <cstdio>
#include <istream>
#include <memory>
#include <stdexcept>
#include <utility>

/// 教学版并查集抽象基类，保留原有公开 API 和受保护字段。
class UnionFind {
protected:
    std::unique_ptr<int[]> _idStorage;
    int* _id;
    int _count;
    int _N;

    /// 构造空基类，供派生类的特殊构造流程使用。
    UnionFind() noexcept;

    /// 分配 N 个元素并初始化为 N 个独立组件。
    explicit UnionFind(int N);

    /// 从输入流读取元素数量，读取失败或数量为负时抛出异常。
    static int readElementCount(std::istream& input);

    /// 读取后续连接对，并通过虚函数接入具体教学实现。
    void loadConnections(std::istream& input);

    /// 检查索引是否落在 [0, N)；失败时抛出 std::out_of_range。
    void validateIndex(int index) const;

    /// 重新初始化父节点或标签数组，并建立单元素组件不变量。
    void resetStorage(int N);

public:
    /// 深拷贝标签或父节点数组，避免教学对象复制后共享所有权。
    UnionFind(const UnionFind& other);

    /// 深拷贝赋值，并在分配失败时保持当前对象不变。
    UnionFind& operator=(const UnionFind& other);

    /// 转移底层数组所有权，并把源对象重置为空状态。
    UnionFind(UnionFind&& other) noexcept;

    /// 转移赋值底层数组所有权，并把源对象重置为空状态。
    UnionFind& operator=(UnionFind&& other) noexcept;

    /// 通过虚析构支持从基类指针安全销毁派生对象。
    virtual ~UnionFind();

    /// 返回当前连通分量数量。
    virtual int count();

    /// 返回当前连通分量数量的只读版本。
    int count() const noexcept;

    /// 返回并查集管理的元素数量。
    int size() const noexcept;

    /// 判断两个元素是否属于同一组件；路径压缩版本可能修改内部父指针。
    virtual bool connected(int p, int q);

    /// 返回元素所属组件的代表元。
    virtual int find(int p) = 0;

    /// 合并两个元素所属组件。
    virtual void unite(int p, int q) = 0;

    /// 保留原教学 API，仅触碰首元素以便演示底层数组存在。
    void traverse();
};

inline UnionFind::UnionFind() noexcept
    : _idStorage(), _id(nullptr), _count(0), _N(0) {}

inline UnionFind::UnionFind(int N)
    : _idStorage(), _id(nullptr), _count(0), _N(0) {
    resetStorage(N);
}

inline UnionFind::UnionFind(const UnionFind& other)
    : _idStorage(), _id(nullptr), _count(other._count), _N(other._N) {
    if (_N > 0) {
        _idStorage.reset(new int[static_cast<std::size_t>(_N)]);
        std::copy(other._id, other._id + _N, _idStorage.get());
        _id = _idStorage.get();
    }
}

inline UnionFind& UnionFind::operator=(const UnionFind& other) {
    if (this == &other)
        return *this;

    std::unique_ptr<int[]> replacement;
    if (other._N > 0) {
        replacement.reset(new int[static_cast<std::size_t>(other._N)]);
        std::copy(other._id, other._id + other._N, replacement.get());
    }

    _idStorage = std::move(replacement);
    _id = _idStorage.get();
    _count = other._count;
    _N = other._N;
    return *this;
}

inline UnionFind::UnionFind(UnionFind&& other) noexcept
    : _idStorage(std::move(other._idStorage)),
      _id(_idStorage.get()),
      _count(other._count),
      _N(other._N) {
    other._id = nullptr;
    other._count = 0;
    other._N = 0;
}

inline UnionFind& UnionFind::operator=(UnionFind&& other) noexcept {
    if (this == &other)
        return *this;

    _idStorage = std::move(other._idStorage);
    _id = _idStorage.get();
    _count = other._count;
    _N = other._N;

    other._id = nullptr;
    other._count = 0;
    other._N = 0;
    return *this;
}

inline UnionFind::~UnionFind() = default;

inline int UnionFind::readElementCount(std::istream& input) {
    int count = 0;
    if (!(input >> count))
        throw std::invalid_argument("UnionFind input is missing the element count");
    if (count < 0)
        throw std::invalid_argument("UnionFind element count cannot be negative");
    return count;
}

inline void UnionFind::loadConnections(std::istream& input) {
    int p = 0;
    int q = 0;
    while (input >> p >> q) {
        if (!connected(p, q))
            unite(p, q);
    }
}

inline void UnionFind::validateIndex(int index) const {
    if (index < 0 || index >= _N)
        throw std::out_of_range("UnionFind index is out of range");
}

inline void UnionFind::resetStorage(int N) {
    if (N < 0)
        throw std::invalid_argument("UnionFind element count cannot be negative");

    std::unique_ptr<int[]> replacement;
    if (N > 0) {
        replacement.reset(new int[static_cast<std::size_t>(N)]);
        for (int index = 0; index < N; ++index)
            replacement[index] = index;
    }

    _idStorage = std::move(replacement);
    _id = _idStorage.get();
    _N = N;
    _count = N;
}

inline int UnionFind::count() {
    return _count;
}

inline int UnionFind::count() const noexcept {
    return _count;
}

inline int UnionFind::size() const noexcept {
    return _N;
}

inline bool UnionFind::connected(int p, int q) {
    return find(p) == find(q);
}

inline void UnionFind::traverse() {
    if (_N <= 0 || _id == nullptr)
        return;
    volatile int sink = _id[0];
    (void)sink;
}

#endif
