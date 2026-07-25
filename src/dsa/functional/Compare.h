#ifndef DSA_FUNCTIONAL_COMPARE_H
#define DSA_FUNCTIONAL_COMPARE_H

namespace dsa {
namespace functional {

/// 默认严格弱序比较器，通过 operator< 比较两个值。
struct DefaultLess {
    template<typename Left, typename Right>
    bool operator()(const Left& left, const Right& right) const {
        return left < right;
    }
};

/// 默认相等比较器，通过 operator== 判断两个值是否相等。
struct DefaultEqual {
    template<typename Left, typename Right>
    bool operator()(const Left& left, const Right& right) const {
        return left == right;
    }
};

} // namespace functional

namespace algorithm {

// 保留算法命名空间中的旧名称，避免现有调用方因公共比较器迁移而失效。
using dsa::functional::DefaultEqual;
using dsa::functional::DefaultLess;

} // namespace algorithm
} // namespace dsa

#endif
