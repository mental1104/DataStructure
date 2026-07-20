#ifndef DSA_CORE_VECTOR_VECTOR_ALGORITHM_H
#define DSA_CORE_VECTOR_VECTOR_ALGORITHM_H

#include <stdexcept>
#include <utility>

namespace dsa {
namespace core {

// Shared mutation workflow for vector-like contiguous containers.
//
// Storage owns allocation, object lifetime, element movement and the concrete
// exception guarantee. The algorithm only coordinates the common vector
// mutation sequence.
//
// Required Storage contract:
//   typedef size_type
//   size_type size() const
//   size_type maxSize() const
//   void ensureCapacity(size_type required)
//   void openGap(size_type position, size_type count, size_type oldSize)
//   template<typename Value> void writeGap(size_type position, Value&& value)
//   void rollbackGap(size_type position, size_type count, size_type oldSize)
//   void closeGap(size_type first, size_type last, size_type oldSize)
//   void commitSize(size_type newSize)
//   void afterErase()
template<typename Storage>
class VectorAlgorithm {
public:
    typedef typename Storage::size_type size_type;

    static size_type recommendCapacity(
        size_type current,
        size_type required,
        size_type minimum,
        size_type maximum
    ) {
        if (required > maximum)
            throw std::length_error("Vector capacity exceeds max_size");

        size_type base = current < minimum ? minimum : current;
        if (base >= required && base > maximum - base)
            return maximum;

        size_type grown = base > maximum - base ? maximum : base + base;
        return grown < required ? required : grown;
    }

    template<typename Value>
    static size_type insert(
        Storage& storage,
        size_type position,
        Value&& value
    ) {
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

        storage.commitSize(oldSize + 1);
        return position;
    }

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
