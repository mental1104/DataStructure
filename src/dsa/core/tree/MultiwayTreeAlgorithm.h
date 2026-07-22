#ifndef DSA_CORE_TREE_MULTIWAY_TREE_ALGORITHM_H
#define DSA_CORE_TREE_MULTIWAY_TREE_ALGORITHM_H

#include <cstddef>

#include <dsa/container/vector/Vector.h>

namespace dsa {
namespace core {

class MultiwayTreeAlgorithm {
public:
    template<typename Sequence, typename Key, typename Compare>
    static std::size_t lowerBound(const Sequence& values, const Key& key, const Compare& compare) {
        std::size_t first = 0;
        std::size_t count = static_cast<std::size_t>(values.size());
        while (count > 0) {
            const std::size_t step = count / 2;
            const std::size_t middle = first + step;
            if (compare(values[static_cast<decltype(values.size())>(middle)], key)) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    template<typename Sequence, typename Key, typename Compare>
    static std::size_t upperBound(const Sequence& values, const Key& key, const Compare& compare) {
        std::size_t first = 0;
        std::size_t count = static_cast<std::size_t>(values.size());
        while (count > 0) {
            const std::size_t step = count / 2;
            const std::size_t middle = first + step;
            if (!compare(key, values[static_cast<decltype(values.size())>(middle)])) {
                first = middle + 1;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return first;
    }

    // 将 total 均匀分组，每组不超过 maximum；可行时同时满足 minimum。
    static dsa::container::Vector<std::size_t> partitionCounts(
        std::size_t total,
        std::size_t maximum,
        std::size_t minimum
    ) {
        dsa::container::Vector<std::size_t> result;
        if (total == 0)
            return result;
        std::size_t groups = (total + maximum - 1) / maximum;
        if (groups == 0)
            groups = 1;
        while (groups > 1 && total / groups < minimum)
            --groups;
        if ((total + groups - 1) / groups > maximum)
            groups = (total + maximum - 1) / maximum;
        const std::size_t base = total / groups;
        const std::size_t remainder = total % groups;
        result.reserve(groups);
        for (std::size_t index = 0; index < groups; ++index)
            result.push_back(base + (index < remainder ? 1 : 0));
        return result;
    }
};

} // namespace core
} // namespace dsa

#endif
