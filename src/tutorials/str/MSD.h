#ifndef __DSA_MSD
#define __DSA_MSD

#include "Vector.h"
#include "dsa_string.h"
#include <dsa/algorithm/StringSort.h>

/// 保留教学版类门面，排序核心转发到泛型 MSD 字符串排序。
class MSD {
public:
    static void sort(Vector<String>& values) {
        dsa::algorithm::msdStringSort(values);
    }
};

#endif
