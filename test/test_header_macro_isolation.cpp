#include "MSD.h"

#ifdef R
#error "MSD.h must not leak the R macro"
#endif

// 遗留教学头可能定义短宏；共享算法头不得使用容易被宏替换的单字母模板参数。
#define R 128
#include <dsa/core/tree/BTreeAlgorithm.h>
#include <dsa/core/tree/SearchTreeAlgorithm.h>
#undef R

#include "gtest/gtest.h"

TEST(HeaderMacroIsolationTest, MsdDoesNotLeakRadixMacro) {
    SUCCEED();
}
