#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "AhoCorasick.h"
#include "AhoCorasickImpl.h"

TEST(AhoCorasickAlgorithmTest, CoreWorksWithoutBusinessFacade) {
    ac_algorithm::AhoCorasickImpl algorithm;
    EXPECT_EQ(algorithm.addPattern("he", 2), 0u);
    EXPECT_EQ(algorithm.addPattern("she", 3), 1u);
    EXPECT_EQ(algorithm.addPattern("hers", 4), 2u);

    algorithm.build();

    const std::vector<ac_algorithm::AhoCorasickIndexMatch> matches =
        algorithm.findAll("ushers", 6);
    ASSERT_EQ(matches.size(), 3u);
    EXPECT_EQ(matches[0].pattern_index, 1u);
    EXPECT_EQ(matches[0].start, 1u);
    EXPECT_EQ(matches[0].end, 4u);
    EXPECT_EQ(matches[1].pattern_index, 0u);
    EXPECT_EQ(matches[1].start, 2u);
    EXPECT_EQ(matches[1].end, 4u);
    EXPECT_EQ(matches[2].pattern_index, 2u);
    EXPECT_EQ(matches[2].start, 2u);
    EXPECT_EQ(matches[2].end, 6u);
}

TEST(AhoCorasickTest, FindsAllStandardPatterns) {
    AhoCorasick matcher;
    EXPECT_TRUE(matcher.add("he", 10));
    EXPECT_TRUE(matcher.add("she", 20));
    EXPECT_TRUE(matcher.add("his", 30));
    EXPECT_TRUE(matcher.add("hers", 40));
    matcher.build();

    const Vector<AhoCorasick::Match> matches = matcher.findAll("ushers");
    ASSERT_EQ(matches.size(), 3);

    EXPECT_EQ(matches[0].pattern_id, 20);
    EXPECT_EQ(matches[0].start, 1u);
    EXPECT_EQ(matches[0].end, 4u);

    EXPECT_EQ(matches[1].pattern_id, 10);
    EXPECT_EQ(matches[1].start, 2u);
    EXPECT_EQ(matches[1].end, 4u);

    EXPECT_EQ(matches[2].pattern_id, 40);
    EXPECT_EQ(matches[2].start, 2u);
    EXPECT_EQ(matches[2].end, 6u);
}

TEST(AhoCorasickTest, RequiresBuildAndFindsFirst) {
    AhoCorasick matcher;
    matcher.add("error", 7);

    AhoCorasick::Match match;
    EXPECT_FALSE(matcher.contains("fatal error"));
    EXPECT_FALSE(matcher.findFirst("fatal error", match));

    matcher.build();
    EXPECT_TRUE(matcher.isBuilt());
    EXPECT_TRUE(matcher.contains("fatal error"));
    ASSERT_TRUE(matcher.findFirst("fatal error", match));
    EXPECT_EQ(match.pattern_id, 7);
    EXPECT_EQ(match.start, 6u);
    EXPECT_EQ(match.end, 11u);
}

TEST(AhoCorasickTest, AddingAfterBuildInvalidatesMatcher) {
    AhoCorasick matcher;
    matcher.add("alpha", 1);
    matcher.build();
    EXPECT_TRUE(matcher.contains("alpha"));

    EXPECT_TRUE(matcher.add("beta", 2));
    EXPECT_FALSE(matcher.isBuilt());
    EXPECT_FALSE(matcher.contains("alpha beta"));

    matcher.build();
    EXPECT_TRUE(matcher.contains("alpha beta"));
    EXPECT_EQ(matcher.findAll("alpha beta").size(), 2);
}

TEST(AhoCorasickTest, ValidatesPatternsAndBusinessIds) {
    AhoCorasick matcher;
    EXPECT_EQ(matcher.add(""), -1);
    EXPECT_EQ(matcher.add(static_cast<const char*>(nullptr)), -1);
    EXPECT_TRUE(matcher.add("same", 100));
    EXPECT_FALSE(matcher.add("other", 100));
    EXPECT_FALSE(matcher.add("", 101));

    const int generated_id = matcher.add("generated");
    EXPECT_EQ(generated_id, 101);
    EXPECT_EQ(matcher.patternCount(), 2);

    const AhoCorasick::Pattern* by_id = matcher.patternById(100);
    ASSERT_NE(by_id, nullptr);
    EXPECT_STREQ(by_id->text.c_str(), "same");
    EXPECT_EQ(matcher.pattern(-1), nullptr);
    EXPECT_EQ(matcher.pattern(2), nullptr);
}

TEST(AhoCorasickTest, AllowsSameTextWithDifferentBusinessIds) {
    AhoCorasick matcher;
    EXPECT_TRUE(matcher.add("duplicate", 1));
    EXPECT_TRUE(matcher.add("duplicate", 2));
    matcher.build();

    const Vector<AhoCorasick::Match> matches = matcher.findAll("duplicate");
    ASSERT_EQ(matches.size(), 2);
    EXPECT_EQ(matches[0].pattern_id, 1);
    EXPECT_EQ(matches[1].pattern_id, 2);
}

TEST(AhoCorasickTest, HandlesOverlappingPatterns) {
    AhoCorasick matcher;
    matcher.add("a", 1);
    matcher.add("aa", 2);
    matcher.add("aaa", 3);
    matcher.build();

    const Vector<AhoCorasick::Match> matches = matcher.findAll("aaaa");
    EXPECT_EQ(matches.size(), 9);
}

TEST(AhoCorasickTest, SupportsUtf8AsByteSequence) {
    AhoCorasick matcher;
    matcher.add("身份证", 1);
    matcher.add("银行卡", 2);
    matcher.build();

    const String text("请提供身份证和银行卡信息");
    const Vector<AhoCorasick::Match> matches = matcher.findAll(text);
    ASSERT_EQ(matches.size(), 2);

    for (Rank i = 0; i < matches.size(); ++i) {
        const AhoCorasick::Match& match = matches[i];
        const String matched(text.c_str() + match.start, match.end - match.start);
        const AhoCorasick::Pattern* pattern = matcher.pattern(match.pattern_index);
        ASSERT_NE(pattern, nullptr);
        EXPECT_EQ(std::strcmp(matched.c_str(), pattern->text.c_str()), 0);
    }
}

TEST(AhoCorasickTest, ClearResetsReusableMatcher) {
    AhoCorasick matcher;
    matcher.add("old", 9);
    matcher.build();
    EXPECT_TRUE(matcher.contains("old"));

    matcher.clear();
    EXPECT_FALSE(matcher.isBuilt());
    EXPECT_EQ(matcher.patternCount(), 0);
    EXPECT_EQ(matcher.patternById(9), nullptr);

    EXPECT_EQ(matcher.add("new"), 0);
    matcher.build();
    EXPECT_TRUE(matcher.contains("new"));
    EXPECT_FALSE(matcher.contains(static_cast<const char*>(nullptr)));
    EXPECT_EQ(matcher.findAll(static_cast<const char*>(nullptr)).size(), 0);
}
