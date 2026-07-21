#include "MSD.h"

#ifdef R
#error "MSD.h must not leak the R macro"
#endif

// 遗留教学头可能定义短宏；共享算法头不得使用容易被宏替换的单字母模板参数。
#define R 128
#include <dsa/core/tree/BTreeAlgorithm.h>
#include <dsa/core/tree/SearchTreeAlgorithm.h>
#undef R

#include <dsa/algorithm/Fibonacci.h>
#include <dsa/algorithm/Hash.h>
#include <dsa/algorithm/Prime.h>
#include <dsa/algorithm/Sort.h>
#include <dsa/algorithm/SuffixArray.h>
#include <dsa/container/hash/ChainedHashMap.h>
#include <dsa/container/hash/Dictionary.h>
#include <dsa/container/hash/HashMap.h>
#include <dsa/container/segment/SegmentTree.h>
#include <dsa/container/skiplist/QuadList.h>
#include <dsa/container/skiplist/SkipList.h>
#include <dsa/container/string/StringSymbolTable.h>
#include <dsa/container/tree/BPlusTree.h>
#include <dsa/container/tree/BStarTree.h>
#include <dsa/container/tree/Splay.h>
#include <dsa/core/segment/SegmentTreeAlgorithm.h>
#include <dsa/core/skiplist/SkipListAlgorithm.h>
#include <dsa/core/tree/MultiwayTreeAlgorithm.h>
#include <dsa/core/tree/SplayTreeAlgorithm.h>

#include "gtest/gtest.h"

TEST(HeaderMacroIsolationTest, MsdDoesNotLeakRadixMacro) {
    SUCCEED();
}
