#ifndef __DSA_QUICK3STRING
#define __DSA_QUICK3STRING

#include "Vector.h"
#include "dsa_string.h"
#include <dsa/algorithm/StringSort.h>

/// 保留教学版类门面，排序核心转发到泛型三向字符串快排。
class Quick3String {
public:
    static void sort(Vector<String>& values) {
        dsa::algorithm::quick3StringSort(values);
    }
};

#endif
