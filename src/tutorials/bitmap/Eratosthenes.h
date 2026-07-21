#ifndef __DSA_ERATOSTHENES
#define __DSA_ERATOSTHENES

#include <stdexcept>
#include <vector>

#include "Bitmap.h"
#include <dsa/algorithm/Prime.h>

// 教学 Bitmap 门面：置位表示非素数，所有权由调用方接管。
inline Bitmap* eratosthenes(int n) {
    if (n < 0)
        throw std::invalid_argument("eratosthenes limit must be non-negative");
    Bitmap* bitmap = new Bitmap(n);
    const dsa::container::Vector<bool> composite =
        dsa::algorithm::eratosthenesComposite(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        if (composite[static_cast<std::size_t>(i)])
            bitmap->set(i);
    }
    return bitmap;
}

inline void eratosthenes_to_file(int n, const char* file) {
    Bitmap* bitmap = eratosthenes(n);
    try {
        bitmap->dump(file);
    } catch (...) {
        delete bitmap;
        throw;
    }
    delete bitmap;
}

#endif
