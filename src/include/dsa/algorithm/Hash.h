#ifndef DSA_ALGORITHM_HASH_H
#define DSA_ALGORITHM_HASH_H

#include <cstddef>
#include <cstring>
#include <functional>
#include <string>

namespace dsa {
namespace algorithm {

inline std::size_t cyclicShiftHash(const char* text) {
    if (!text)
        return 0;
    unsigned int hash = 0;
    for (std::size_t i = 0, count = std::strlen(text); i < count; ++i) {
        hash = (hash << 5) | (hash >> 27);
        hash += static_cast<unsigned char>(text[i]);
    }
    return static_cast<std::size_t>(hash);
}

inline std::size_t hashCode(char value) {
    return static_cast<unsigned char>(value);
}

inline std::size_t hashCode(int value) {
    return std::hash<int>()(value);
}

inline std::size_t hashCode(long long value) {
    return std::hash<long long>()(value);
}

inline std::size_t hashCode(const char* value) {
    return cyclicShiftHash(value);
}

inline std::size_t hashCode(const std::string& value) {
    return cyclicShiftHash(value.c_str());
}

} // namespace algorithm
} // namespace dsa

#endif
