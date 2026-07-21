#ifndef DSA_ALGORITHM_PRIME_H
#define DSA_ALGORITHM_PRIME_H

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace dsa {
namespace algorithm {

// 返回 [0, limit) 的合数标记；0 和 1 同样标记为非素数。
inline std::vector<bool> eratosthenesComposite(std::size_t limit) {
    std::vector<bool> composite(limit, false);
    if (limit > 0)
        composite[0] = true;
    if (limit > 1)
        composite[1] = true;
    if (limit < 3)
        return composite;
    for (std::size_t prime = 2; prime <= (limit - 1) / prime; ++prime) {
        if (composite[prime])
            continue;
        for (std::size_t multiple = prime * prime; multiple < limit; multiple += prime)
            composite[multiple] = true;
    }
    return composite;
}

inline std::vector<std::size_t> primesBelow(std::size_t limit) {
    const std::vector<bool> composite = eratosthenesComposite(limit);
    std::vector<std::size_t> result;
    for (std::size_t value = 2; value < limit; ++value) {
        if (!composite[value])
            result.push_back(value);
    }
    return result;
}

// 在 [first, limit) 返回第一个未被 composite 标记的素数；不存在时返回 limit。
template<typename CompositeAccess>
std::size_t nextPrime(
    std::size_t first,
    std::size_t limit,
    const CompositeAccess& composite
) {
    if (first < 2)
        first = 2;
    while (first < limit && composite(first))
        ++first;
    return first;
}

// 返回满足 value % modulus == remainder 的下一个素数；不存在时返回 limit。
template<typename CompositeAccess>
std::size_t nextPrimeCongruent(
    std::size_t first,
    std::size_t limit,
    std::size_t modulus,
    std::size_t remainder,
    const CompositeAccess& composite
) {
    if (modulus == 0)
        throw std::invalid_argument("prime congruence modulus must be positive");
    std::size_t candidate = nextPrime(first, limit, composite);
    while (candidate < limit) {
        if (candidate % modulus == remainder % modulus)
            return candidate;
        candidate = nextPrime(candidate + 1, limit, composite);
    }
    return limit;
}

} // namespace algorithm
} // namespace dsa

#endif
