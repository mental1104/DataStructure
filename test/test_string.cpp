#include <gtest/gtest.h>
#include "Vector.h"
#include "dsa_string.h"
#include "LSD.h"
#include "MSD.h"
#include "Quick3String.h"

TEST(StringTest, DefaultConstructor) {
    String s;
    EXPECT_STREQ(s.c_str(), "");
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.empty());
}

TEST(StringTest, CharConstructor) {
    String s('a');
    EXPECT_STREQ(s.c_str(), "a");
    EXPECT_EQ(s.size(), 1);
    EXPECT_FALSE(s.empty());
}

TEST(StringTest, CStringConstructor) {
    String s("hello");
    EXPECT_STREQ(s.c_str(), "hello");
    EXPECT_EQ(s.size(), 5);
}

TEST(StringTest, CopyConstructor) {
    String s1("copy");
    String s2(s1);
    EXPECT_STREQ(s2.c_str(), "copy");
    EXPECT_EQ(s2.size(), s1.size());
}

TEST(StringTest, AssignmentOperator) {
    String s1("assign");
    String s2;
    s2 = s1;
    EXPECT_STREQ(s2.c_str(), "assign");
    EXPECT_EQ(s2.size(), s1.size());
}

TEST(StringTest, FrontBack) {
    String s("test");
    EXPECT_EQ(s.front(), 't');
    EXPECT_EQ(s.back(), 't');
}

TEST(StringTest, IndexOperator) {
    String s("abc");
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s[1], 'b');
    EXPECT_EQ(s[2], 'c');
}

TEST(StringTest, Concatenation) {
    String s1("hello");
    String s2(" world");
    String s3 = s1 + s2;
    EXPECT_STREQ(s3.c_str(), "hello world");
    EXPECT_EQ(s3.size(), 11);
}

TEST(StringTest, CharConcatenation) {
    String s("a");
    String s2 = s + 'b';
    EXPECT_STREQ(s2.c_str(), "ab");
}

TEST(StringTest, ComparisonOperators) {
    String s1("abc");
    String s2("abc");
    String s3("abcd");
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
    EXPECT_TRUE(s1 != s3);
    EXPECT_FALSE(s1 < s2);
    EXPECT_TRUE(s1 < s3);
}

TEST(StringTest, Substring) {
    String s("substring");
    String sub = s.substr(3, 3);
    EXPECT_STREQ(sub.c_str(), "str");
}

TEST(StringTest, PrefixSuffix) {
    String s("example");
    EXPECT_STREQ(s.prefix(3).c_str(), "exa");
    EXPECT_STREQ(s.suffix(2).c_str(), "le");
}


// Helper function for comparing Vector<String>
bool compareVectors(const Vector<String>& v1, const Vector<String>& v2) {
    if (v1.size() != v2.size()) return false;
    for (Rank i = 0; i < v1.size(); ++i) {
        if (!(v1[i] == v2[i])) {  // 假设 String 类有 equals 方法
            return false;
        }
    }
    return true;
}

TEST(LSDTest, BasicSorting) {
    Vector<String> strings;
    strings.insert("dab");
    strings.insert("cab");
    strings.insert("abc");
    strings.insert("bac");
    strings.insert("cba");

    LSD(strings, 3);

    Vector<String> expected;
    expected.insert("abc");
    expected.insert("bac");
    expected.insert("cab");
    expected.insert("cba");
    expected.insert("dab");

    EXPECT_TRUE(compareVectors(strings, expected));
}

TEST(MSDTest, BasicSorting) {
    Vector<String> strings;
    strings.insert("dab");
    strings.insert("cab");
    strings.insert("abc");
    strings.insert("bac");
    strings.insert("cba");

    MSD::sort(strings);

    Vector<String> expected;
    expected.insert("abc");
    expected.insert("bac");
    expected.insert("cab");
    expected.insert("cba");
    expected.insert("dab");

    EXPECT_TRUE(compareVectors(strings, expected));
}

TEST(Quick3StringTest, BasicSorting) {
    Vector<String> strings;
    strings.insert("dab");
    strings.insert("cab");
    strings.insert("abc");
    strings.insert("bac");
    strings.insert("cba");

    Quick3String::sort(strings);

    Vector<String> expected;
    expected.insert("abc");
    expected.insert("bac");
    expected.insert("cab");
    expected.insert("cba");
    expected.insert("dab");

    EXPECT_TRUE(compareVectors(strings, expected));
}
