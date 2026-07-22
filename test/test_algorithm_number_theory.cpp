#include <gtest/gtest.h>

#include <limits>
#include <vector>

#include <dsa/algorithm/Fibonacci.h>
#include <dsa/algorithm/Prime.h>
#include "Fib.h"
#include "Eratosthenes.h"
#include "primeNLT.h"

TEST(FibonacciAlgorithmTest, ComputesAndMovesCursor) {
    EXPECT_EQ(dsa::algorithm::fibonacci<unsigned long long>(0), 0u);
    EXPECT_EQ(dsa::algorithm::fibonacci<unsigned long long>(10), 55u);

    dsa::algorithm::FibonacciCursor<int> cursor(20);
    EXPECT_EQ(cursor.get(), 21);
    EXPECT_EQ(cursor.prev(), 13);
    EXPECT_EQ(cursor.next(), 21);

    Fib teaching(20);
    EXPECT_EQ(teaching.get(), 21);
    EXPECT_EQ(teaching.prev(), 13);
}

TEST(FibonacciAlgorithmTest, ReportsOverflow) {
    EXPECT_THROW(dsa::algorithm::fibonacci<int>(48), std::overflow_error);
}

TEST(PrimeAlgorithmTest, SieveAndCongruence) {
    const dsa::container::Vector<bool> composite = dsa::algorithm::eratosthenesComposite(32);
    EXPECT_TRUE(composite[0]);
    EXPECT_TRUE(composite[1]);
    EXPECT_FALSE(composite[2]);
    EXPECT_TRUE(composite[4]);
    EXPECT_FALSE(composite[31]);

    const auto access = [&composite](std::size_t i) { return composite[i]; };
    EXPECT_EQ(dsa::algorithm::nextPrime(14, 32, access), 17u);
    EXPECT_EQ(dsa::algorithm::nextPrimeCongruent(5, 32, 4, 3, access), 7u);
}

TEST(PrimeAlgorithmTest, TeachingBitmapAdaptersReuseSieve) {
    Bitmap* bitmap = eratosthenes(32);
    ASSERT_NE(bitmap, nullptr);
    EXPECT_EQ(primeNLT_mem(14, 32, bitmap), 17);
    EXPECT_EQ(primeQHT_mem(5, 32, bitmap), 7);
    delete bitmap;
}
