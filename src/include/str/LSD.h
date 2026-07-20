#ifndef __DSA_LSD
#define __DSA_LSD

#include "Vector.h"
#include "dsa_string.h"
#include "../dsa/algorithm/StringSort.h"

/// 保留教学版入口，排序核心转发到泛型 LSD 字符串排序。
inline void LSD(Vector<String>& values, int width) {
    if (width < 0)
        return;
    dsa::algorithm::lsdStringSort(values, static_cast<std::size_t>(width));
}

#endif
