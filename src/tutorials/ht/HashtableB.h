#ifndef __DSA_HASHTABLEB
#define __DSA_HASHTABLEB

#include "Hashtable.h"

/// 保留原公开 API 的双向平方探测教学哈希表。
template<typename K, typename V>
class QuadraticHT
    : public dsa_teaching_detail::OpenAddressTeachingTable<
          K,
          V,
          dsa_teaching_detail::TeachingQuadraticProbing
      > {
private:
    typedef dsa_teaching_detail::OpenAddressTeachingTable<
        K,
        V,
        dsa_teaching_detail::TeachingQuadraticProbing
    > Base;

public:
    /// 构造平方探测教学哈希表。
    explicit QuadraticHT(int capacity = 5) : Base(capacity) {}
};

#endif
