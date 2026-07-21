#ifndef __DSA_PRIME_NLT
#define __DSA_PRIME_NLT

#include <cstddef>
#include "Bitmap.h"
#include <dsa/algorithm/Prime.h>

namespace dsa_prime_compat {
struct BitmapComposite {
    const Bitmap* bitmap;
    bool operator()(std::size_t index) const {
        return bitmap->test(static_cast<int>(index));
    }
};
}

inline int primeNLT(int c, int n, const char* file) {
    Bitmap bitmap(file, n);
    return static_cast<int>(dsa::algorithm::nextPrime(
        c < 0 ? 0u : static_cast<std::size_t>(c),
        n < 0 ? 0u : static_cast<std::size_t>(n),
        dsa_prime_compat::BitmapComposite{&bitmap}
    ));
}

inline int primeNLT(int c, int n, char* file) {
    return primeNLT(c, n, static_cast<const char*>(file));
}

inline int primeQHT(int c, int n, const char* file) {
    Bitmap bitmap(file, n);
    return static_cast<int>(dsa::algorithm::nextPrimeCongruent(
        c < 0 ? 0u : static_cast<std::size_t>(c),
        n < 0 ? 0u : static_cast<std::size_t>(n),
        4u,
        3u,
        dsa_prime_compat::BitmapComposite{&bitmap}
    ));
}

inline int primeQHT(int c, int n, char* file) {
    return primeQHT(c, n, static_cast<const char*>(file));
}

inline int primeNLT_mem(int c, int n, const Bitmap* bitmap) {
    return static_cast<int>(dsa::algorithm::nextPrime(
        c < 0 ? 0u : static_cast<std::size_t>(c),
        n < 0 ? 0u : static_cast<std::size_t>(n),
        dsa_prime_compat::BitmapComposite{bitmap}
    ));
}

inline int primeNLT_mem(int c, int n, Bitmap* bitmap) {
    return primeNLT_mem(c, n, static_cast<const Bitmap*>(bitmap));
}

inline int primeQHT_mem(int c, int n, const Bitmap* bitmap) {
    return static_cast<int>(dsa::algorithm::nextPrimeCongruent(
        c < 0 ? 0u : static_cast<std::size_t>(c),
        n < 0 ? 0u : static_cast<std::size_t>(n),
        4u,
        3u,
        dsa_prime_compat::BitmapComposite{bitmap}
    ));
}

inline int primeQHT_mem(int c, int n, Bitmap* bitmap) {
    return primeQHT_mem(c, n, static_cast<const Bitmap*>(bitmap));
}

#endif
