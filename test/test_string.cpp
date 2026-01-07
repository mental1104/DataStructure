#include <gtest/gtest.h>
#include <cctype>
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

TEST(StringTest, CharAtAndOutOfRange) {
    String s("abc");
    EXPECT_EQ(s.charAt(1), 'b');
    EXPECT_EQ(s.charAt(3), '\0');
}

TEST(StringTest, BoundsCheck) {
    String s("abc");
    EXPECT_TRUE(s.check(2));
    EXPECT_FALSE(s.check(3));
}

TEST(StringTest, SubstringOutOfRange) {
    String s("abc");
    String sub = s.substr(10, 2);
    EXPECT_STREQ(sub.c_str(), "");
}

TEST(StringTest, SubstringDefaultLength) {
    String s("substring");
    String sub = s.substr(3);
    EXPECT_STREQ(sub.c_str(), "string");
}

TEST(StringTest, PrefixSuffixOutOfRange) {
    String s("abc");
    EXPECT_STREQ(s.prefix(5).c_str(), "abc");
    EXPECT_STREQ(s.suffix(5).c_str(), "abc");
}

TEST(StringTest, ConcatInPlace) {
    String s1("hi");
    String s2("!");
    String& ret = s1.concat(s2);
    EXPECT_EQ(&ret, &s1);
    EXPECT_STREQ(s1.c_str(), "hi!");
}

static void bumpChar(char& c) {
    c = static_cast<char>(c + 1);
}

static void assignString(String& dest, const String& src) {
    dest = src;
}

TEST(StringTest, TraverseOverloads) {
    String s("ab");
    s.traverse([](char& c) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    });
    EXPECT_STREQ(s.c_str(), "AB");

    s.traverse(bumpChar);
    EXPECT_STREQ(s.c_str(), "BC");
}

TEST(StringTest, ConstIndexAndSelfAssign) {
    const String s("xyz");
    EXPECT_EQ(s[2], 'z');
    String s2("self");
    assignString(s2, s2);
    EXPECT_STREQ(s2.c_str(), "self");
}

TEST(StringTest, EqualMethod) {
    String s1("same");
    String s2("same");
    EXPECT_TRUE(s1.equal(s2));
}

TEST(StringTest, CoverageExercise) {
    String empty;
    String single('z');
    EXPECT_STREQ(empty.c_str(), "");
    EXPECT_STREQ(single.c_str(), "z");
    EXPECT_EQ(single.front(), 'z');
    EXPECT_EQ(single.back(), 'z');

    String word("word");
    char& ref = word[1];
    ref = 'o';
    const String const_word("hi");
    EXPECT_EQ(const_word[0], 'h');

    EXPECT_TRUE(word.check(0));
    EXPECT_FALSE(word.check(999));
    EXPECT_TRUE(String("aa") == String("aa"));
    EXPECT_TRUE(String("aa") < String("ab"));

    void (*fn)(char&) = bumpChar;
    word.traverse(fn);
    EXPECT_STREQ(word.c_str(), "xpse");
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

TEST(MSDTest, HandlesEmptyString) {
    Vector<String> strings;
    strings.insert("");
    strings.insert("a");
    strings.insert("aa");
    MSD::sort(strings);
    EXPECT_STREQ(strings[0].c_str(), "");
}

TEST(MSDTest, HandlesEmptyStringInLargerInput) {
    Vector<String> strings;
    strings.insert("");
    strings.insert("b");
    strings.insert("a");
    strings.insert("aa");
    strings.insert("ab");

    MSD::sort(strings);

    EXPECT_STREQ(strings[0].c_str(), "");
    EXPECT_STREQ(strings[1].c_str(), "a");
    EXPECT_STREQ(strings[2].c_str(), "aa");
    EXPECT_STREQ(strings[3].c_str(), "ab");
    EXPECT_STREQ(strings[4].c_str(), "b");
}

TEST(MSDTest, SmallInsertionSort) {
    Vector<String> strings;
    strings.insert("b");
    strings.insert("a");
    strings.insert("c");
    MSD::sort(strings);
    EXPECT_STREQ(strings[0].c_str(), "a");
    EXPECT_STREQ(strings[1].c_str(), "b");
    EXPECT_STREQ(strings[2].c_str(), "c");
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

TEST(Quick3StringTest, HandlesEmptyString) {
    Vector<String> strings;
    strings.insert("");
    strings.insert("a");
    strings.insert("aa");
    Quick3String::sort(strings);
    EXPECT_STREQ(strings[0].c_str(), "");
}
