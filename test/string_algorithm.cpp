// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_algorithm.hpp>

#include <gtest/gtest.h>


using namespace boost;

char32_t const teehee_array[] = {'t', 'e', 'e', 'h', 'e', 'e'};
char32_t const ee_array[] = {'e', 'e'};

char32_t const here_array[] = {'h', 'e', 'r', 'e'};
char32_t const he_array[] = {'h', 'e'};
char32_t const re_array[] = {'r', 'e'};

TEST(str_algo, find)
{
    // Code point iterators.
    {
        char32_t const * teehee_ptr = U"teehee";
        char32_t const * ee_ptr = U"ee";
        auto const result = text::find(
            teehee_ptr, text::null_sentinel{}, ee_ptr, text::null_sentinel{});
        EXPECT_TRUE(result.begin() == teehee_ptr + 1);
        EXPECT_TRUE(result.end() == teehee_ptr + 3);
    }
    {
        char32_t const * ee_ptr = U"ee";
        auto const result = text::find(
            std::begin(teehee_array),
            std::end(teehee_array),
            ee_ptr,
            text::null_sentinel{});
        EXPECT_TRUE(result.begin() == teehee_array + 1);
        EXPECT_TRUE(result.end() == teehee_array + 3);
    }
    {
        char32_t const * teehee_ptr = U"teehee";
        auto const result = text::find(
            teehee_ptr,
            text::null_sentinel{},
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result.begin() == teehee_ptr + 1);
        EXPECT_TRUE(result.end() == teehee_ptr + 3);
    }
    {
        auto const result = text::find(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result.begin() == teehee_array + 1);
        EXPECT_TRUE(result.end() == teehee_array + 3);
    }

    // Code point ranges.
    {
        char const * here_ptr = "here";
        auto const result = text::find(here_ptr, "he");
        EXPECT_TRUE(
            result.begin() == std::next(text::as_utf32(here_ptr).begin(), 0));
        EXPECT_TRUE(
            result.end() == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const result = text::find(here, "re");
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 2));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 4));
    }
    {
        char const * here_ptr = "here";
        auto const he = text::as_utf32("he");
        auto const result = text::find(here_ptr, he);
        EXPECT_TRUE(
            result.begin() == std::next(text::as_utf32(here_ptr).begin(), 0));
        EXPECT_TRUE(
            result.end() == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const he = text::as_utf32("he");
        auto const result = text::find(here, he);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
    {
        auto const result = text::find(u8"here", u"hea");
        EXPECT_TRUE(result.empty());
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find(here_ptr, u"er");
        EXPECT_TRUE(result.begin() == here_ptr + 1);
        EXPECT_TRUE(result.end() == here_ptr + 3);
    }
    {
        auto const result = text::find(u8"here", U"hr");
        EXPECT_TRUE(result.empty());
    }
    {
        auto const result = text::find("he", "here");
        EXPECT_TRUE(result.empty());
    }
    {
        auto const result = text::find("here", "hea");
        EXPECT_TRUE(result.empty());
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"er", text::null_sentinel{});
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 1));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 3));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"re", text::null_sentinel{});
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 2));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 4));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 2));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::find(here, he);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const result = text::find(text::as_graphemes(here_array), "he");
        EXPECT_TRUE(result.begin() == here_array + 0);
        EXPECT_TRUE(result.end() == here_array + 2);
    }
    {
        char const * he_ptr = "he";
        auto const result = text::find(text::as_graphemes(here_array), he_ptr);
        EXPECT_TRUE(result.begin() == here_array + 0);
        EXPECT_TRUE(result.end() == here_array + 2);
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result =
            text::find(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result = text::find(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
#endif
}

TEST(str_algo, rfind)
{
    // Code point iterators.
    {
        auto const result = text::rfind(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result.begin() == teehee_array + 4);
        EXPECT_TRUE(result.end() == teehee_array + 6);
    }

    // Code point ranges.
    {
        char const * teehee_ptr = "teehee";
        auto const result = text::rfind(teehee_ptr, "ee");
        EXPECT_TRUE(
            result.begin() == std::next(text::as_utf32(teehee_ptr).begin(), 4));
        EXPECT_TRUE(
            result.end() == std::next(text::as_utf32(teehee_ptr).begin(), 6));
    }
    {
        char const * teehee_ptr = "teehee";
        auto const teehee = text::as_utf32(teehee_ptr);
        auto const result = text::rfind(teehee, "ee");
        EXPECT_TRUE(result.begin() == std::next(teehee.begin(), 4));
        EXPECT_TRUE(result.end() == std::next(teehee.begin(), 6));
    }
    {
        char const * teehee_ptr = "teehee";
        auto const he = text::as_utf32("ee");
        auto const result = text::rfind(teehee_ptr, he);
        EXPECT_TRUE(
            result.begin() == std::next(text::as_utf32(teehee_ptr).begin(), 4));
        EXPECT_TRUE(
            result.end() == std::next(text::as_utf32(teehee_ptr).begin(), 6));
    }
    {
        char const * teehee_ptr = "teehee";
        auto const teehee = text::as_utf32(teehee_ptr);
        auto const he = text::as_utf32("ee");
        auto const result = text::rfind(teehee, he);
        EXPECT_TRUE(result.begin() == std::next(teehee.begin(), 4));
        EXPECT_TRUE(result.end() == std::next(teehee.begin(), 6));
    }
    {
        auto const result = text::rfind(u8"teehee", u"hea");
        EXPECT_TRUE(result.empty());
    }
    {
        char32_t const * teehee_ptr = U"teehee";
        auto const result = text::rfind(teehee_ptr, u"he");
        EXPECT_TRUE(result.begin() == teehee_ptr + 3);
        EXPECT_TRUE(result.end() == teehee_ptr + 5);
    }
    {
        auto const result = text::rfind(u8"teehee", U"hr");
        EXPECT_TRUE(result.empty());
    }
    {
        auto const result = text::rfind("he", "teehee");
        EXPECT_TRUE(result.empty());
    }
    {
        char32_t const * teehee_ptr = U"teehee";
        auto const result = text::rfind(teehee_ptr, "teea");
        EXPECT_TRUE(result.begin() == teehee_ptr + 6);
        EXPECT_TRUE(result.empty());
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::rfind(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result.begin() == std::next(r1.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(r1.begin(), 2));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::rfind(here, he);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const here = text::as_graphemes(here_array);
        auto const result = text::rfind(here, "he");
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
    {
        auto const here = text::as_graphemes(here_array);
        char const * he_ptr = "he";
        auto const result = text::rfind(here, he_ptr);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result = text::rfind(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result = text::rfind(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result.begin() == std::next(here.begin(), 0));
        EXPECT_TRUE(result.end() == std::next(here.begin(), 2));
    }
#endif
}

TEST(str_algo, find_first_of)
{
    // Code point iterators.
    {
        char32_t const * teehee_ptr = U"teehee";
        char32_t const * ee_ptr = U"fe";
        auto const result = text::find_first_of(
            teehee_ptr, text::null_sentinel{}, ee_ptr, text::null_sentinel{});
        EXPECT_TRUE(result == teehee_ptr + 1);
    }
    {
        char32_t const * ee_ptr = U"ef";
        auto const result = text::find_first_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            ee_ptr,
            text::null_sentinel{});
        EXPECT_TRUE(result == teehee_array + 1);
    }
    {
        char32_t const * teehee_ptr = U"teehee";
        auto const result = text::find_first_of(
            teehee_ptr,
            text::null_sentinel{},
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_ptr + 1);
    }
    {
        auto const result = text::find_first_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_array + 1);
    }

    // Code point ranges.
    {
        char const * here_ptr = "here";
        auto const result = text::find_first_of(here_ptr, "eh");
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 0));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const result = text::find_first_of(here, "re");
        EXPECT_TRUE(result == std::next(here.begin(), 1));
    }
    {
        char const * here_ptr = "here";
        auto const he = text::as_utf32("he");
        auto const result = text::find_first_of(here_ptr, he);
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 0));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const he = text::as_utf32("he");
        auto const result = text::find_first_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        text::find_first_of(u8"here", u"hea");
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find_first_of(here_ptr, u"cd");
        EXPECT_TRUE(result == here_ptr + 4);
    }
    {
        text::find_first_of(u8"here", U"hr");
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"er", text::null_sentinel{});
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 1));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"re", text::null_sentinel{});
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 1));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_first_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::find_first_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const here = text::as_graphemes(here_array);
        auto const result = text::find_first_of(here, "he");
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        auto const here = text::as_graphemes(here_array);
        char const * he_ptr = "he";
        auto const result = text::find_first_of(here, he_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result =
            text::find_first_of(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result =
            text::find_first_of(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
#endif
}

TEST(str_algo, find_last_of)
{
    // Code point iterators.
    {
        auto const result = text::find_last_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_array + 5);
    }

    // Code point ranges.
    {
        char const * here_ptr = "here";
        auto const result = text::find_last_of(here_ptr, "eh");
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 3));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const result = text::find_last_of(here, "re");
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
    {
        char const * here_ptr = "here";
        auto const he = text::as_utf32("he");
        auto const result = text::find_last_of(here_ptr, he);
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 3));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const he = text::as_utf32("he");
        auto const result = text::find_last_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
    {
        text::find_last_of(u8"here", u"hea");
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find_last_of(here_ptr, u"cd");
        EXPECT_TRUE(result == here_ptr + 4);
    }
    {
        text::find_last_of(u8"here", U"hr");
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_last_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 3));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::find_last_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const here = text::as_graphemes(here_array);
        auto const result = text::find_last_of(here, "he");
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
    {
        auto const here = text::as_graphemes(here_array);
        char const * he_ptr = "he";
        auto const result = text::find_last_of(here, he_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result =
            text::find_last_of(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result =
            text::find_last_of(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 3));
    }
#endif
}

TEST(str_algo, find_first_not_of)
{
    // Code point iterators.
    {
        char32_t const * teehee_ptr = U"teehee";
        char32_t const * ee_ptr = U"fe";
        auto const result = text::find_first_not_of(
            teehee_ptr, text::null_sentinel{}, ee_ptr, text::null_sentinel{});
        EXPECT_TRUE(result == teehee_ptr + 0);
    }
    {
        char32_t const * ee_ptr = U"ef";
        auto const result = text::find_first_not_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            ee_ptr,
            text::null_sentinel{});
        EXPECT_TRUE(result == teehee_array + 0);
    }
    {
        char32_t const * teehee_ptr = U"teehee";
        auto const result = text::find_first_not_of(
            teehee_ptr,
            text::null_sentinel{},
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_ptr + 0);
    }
    {
        auto const result = text::find_first_not_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_array + 0);
    }

    // Code point ranges.
    {
        char const * here_ptr = "here";
        auto const result = text::find_first_not_of(here_ptr, "eh");
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const result = text::find_first_not_of(here, "re");
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        char const * here_ptr = "here";
        auto const he = text::as_utf32("he");
        auto const result = text::find_first_not_of(here_ptr, he);
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const he = text::as_utf32("he");
        auto const result = text::find_first_not_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        text::find_first_not_of(u8"here", u"hea");
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find_first_not_of(here_ptr, u"cd");
        EXPECT_TRUE(result == here_ptr + 0);
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find_first_not_of(here_ptr, u"hre");
        EXPECT_TRUE(result == here_ptr + 4);
    }
    {
        text::find_first_not_of(u8"here", U"hr");
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"er", text::null_sentinel{});
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"re", text::null_sentinel{});
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 0));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 2));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_first_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 2));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::find_first_not_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const here = text::as_graphemes(here_array);
        auto const result = text::find_first_not_of(here, "he");
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        auto const here = text::as_graphemes(here_array);
        char const * he_ptr = "he";
        auto const result = text::find_first_not_of(here, he_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result =
            text::find_first_not_of(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result =
            text::find_first_not_of(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
#endif
}

TEST(str_algo, find_last_not_of)
{
    // Code point iterators.
    {
        auto const result = text::find_last_not_of(
            std::begin(teehee_array),
            std::end(teehee_array),
            std::begin(ee_array),
            std::end(ee_array));
        EXPECT_TRUE(result == teehee_array + 3);
    }

    // Code point ranges.
    {
        char const * here_ptr = "here";
        auto const result = text::find_last_not_of(here_ptr, "eh");
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const result = text::find_last_not_of(here, "re");
        EXPECT_TRUE(result == std::next(here.begin(), 0));
    }
    {
        char const * here_ptr = "here";
        auto const he = text::as_utf32("he");
        auto const result = text::find_last_not_of(here_ptr, he);
        EXPECT_TRUE(result == std::next(text::as_utf32(here_ptr).begin(), 2));
    }
    {
        char const * here_ptr = "here";
        auto const here = text::as_utf32(here_ptr);
        auto const he = text::as_utf32("he");
        auto const result = text::find_last_not_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        text::find_last_not_of(u8"here", u"hea");
    }
    {
        char32_t const * here_ptr = U"here";
        auto const result = text::find_last_not_of(here_ptr, u"cd");
        EXPECT_TRUE(result == here_ptr + 3);
    }
    {
        text::find_last_not_of(u8"here", U"hr");
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        auto const result =
            text::find_last_not_of(r1.begin(), r1.end(), r2.begin(), r2.end());
        EXPECT_TRUE(result == std::next(r1.begin(), 2));
    }

    // Grapheme ranges.
    {
        auto const here = text::as_graphemes(here_array);
        auto const he = text::as_graphemes(he_array);
        auto const result = text::find_last_not_of(here, he);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        auto const here = text::as_graphemes(here_array);
        auto const result = text::find_last_not_of(here, "he");
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        auto const here = text::as_graphemes(here_array);
        char const * he_ptr = "he";
        auto const result = text::find_last_not_of(here, he_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        char16_t const here_array[] = {'h', 'e', 'r', 'e'};
        auto const result =
            text::find_last_not_of(here_array, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_array);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
    {
        char16_t const * here_ptr = u"here";
        auto const result =
            text::find_last_not_of(here_ptr, text::as_graphemes(he_array));
        auto const here = text::as_graphemes(here_ptr);
        EXPECT_TRUE(result == std::next(here.begin(), 2));
    }
#endif
}

TEST(str_algo, starts_with)
{
    // Code point iterators.
    {
        EXPECT_TRUE(text::starts_with(
            U"here", text::null_sentinel{}, U"he", text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::starts_with(
            std::begin(here_array),
            std::end(here_array),
            U"he",
            text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::starts_with(
            U"here",
            text::null_sentinel{},
            std::begin(he_array),
            std::end(he_array)));
    }
    {
        EXPECT_TRUE(text::starts_with(
            std::begin(here_array),
            std::end(here_array),
            std::begin(he_array),
            std::end(he_array)));
    }

    // Code point ranges.
    {
        EXPECT_TRUE(text::starts_with("here", "he"));
    }
    {
        auto const here = text::as_utf32("here");
        EXPECT_TRUE(text::starts_with(here, "he"));
    }
    {
        auto const he = text::as_utf32("he");
        EXPECT_TRUE(text::starts_with("here", he));
    }
    {
        auto const here = text::as_utf32("here");
        auto const he = text::as_utf32("he");
        EXPECT_TRUE(text::starts_with(here, he));
    }
    {
        EXPECT_TRUE(text::starts_with(u8"here", u"he"));
    }
    {
        EXPECT_TRUE(text::starts_with(U"here", u"he"));
    }
    {
        EXPECT_TRUE(text::starts_with(u8"here", U"he"));
    }
    {
        EXPECT_FALSE(text::starts_with("he", "here"));
    }
    {
        EXPECT_FALSE(text::starts_with("here", "re"));
    }
    {
        EXPECT_FALSE(text::starts_with("here", "hea"));
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        EXPECT_TRUE(
            text::starts_with(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        EXPECT_TRUE(
            text::starts_with(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(he_array);
        EXPECT_TRUE(
            text::starts_with(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        EXPECT_TRUE(
            text::starts_with(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }

    // Grapheme ranges.
    {
        EXPECT_TRUE(text::starts_with(
            text::as_graphemes(here_array), text::as_graphemes(he_array)));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        EXPECT_TRUE(text::starts_with(text::as_graphemes(here_array), "he"));
    }
    {
        EXPECT_TRUE(text::starts_with(
            text::as_graphemes(here_array), (char const *)"he"));
    }
    {
        EXPECT_TRUE(text::starts_with(U"here", text::as_graphemes(he_array)));
    }
    {
        EXPECT_TRUE(text::starts_with(
            (char32_t const *)U"here", text::as_graphemes(he_array)));
    }
#endif
}

TEST(str_algo, ends_with)
{
    // Code point iterators.
    {
        EXPECT_TRUE(text::ends_with(
            std::begin(here_array),
            std::end(here_array),
            std::begin(re_array),
            std::end(re_array)));
    }

    // Code point ranges.
    {
        EXPECT_TRUE(text::ends_with("here", "re"));
    }
    {
        auto const here = text::as_utf32("here");
        EXPECT_TRUE(text::ends_with(here, "re"));
    }
    {
        auto const re = text::as_utf32("re");
        EXPECT_TRUE(text::ends_with("here", re));
    }
    {
        auto const here = text::as_utf32("here");
        auto const re = text::as_utf32("re");
        EXPECT_TRUE(text::ends_with(here, re));
    }
    {
        EXPECT_TRUE(text::ends_with(u8"here", u"re"));
    }
    {
        EXPECT_TRUE(text::ends_with(U"here", u"re"));
    }
    {
        EXPECT_TRUE(text::ends_with(u8"here", U"re"));
    }
    {
        EXPECT_FALSE(text::ends_with("re", "here"));
    }
    {
        EXPECT_FALSE(text::ends_with("here", "he"));
    }
    {
        EXPECT_FALSE(text::ends_with("here", "are"));
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here");
        auto const r2 = text::as_graphemes(U"re");
        EXPECT_TRUE(
            text::ends_with(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }

    // Grapheme ranges.
    {
        EXPECT_TRUE(text::ends_with(
            text::as_graphemes(here_array), text::as_graphemes(re_array)));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        EXPECT_TRUE(text::ends_with(text::as_graphemes(here_array), "re"));
    }
    {
        EXPECT_TRUE(text::ends_with(
            text::as_graphemes(here_array), (char const *)"re"));
    }
    {
        EXPECT_TRUE(text::ends_with(U"here", text::as_graphemes(re_array)));
    }
    {
        EXPECT_TRUE(text::ends_with(
            (char32_t const *)U"here", text::as_graphemes(re_array)));
    }
#endif
}

TEST(str_algo, contains)
{
    // Code point iterators.
    {
        EXPECT_TRUE(text::contains(
            U"here", text::null_sentinel{}, U"he", text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::contains(
            U"here", text::null_sentinel{}, U"er", text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::contains(
            U"here", text::null_sentinel{}, U"re", text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::contains(
            std::begin(here_array),
            std::end(here_array),
            U"he",
            text::null_sentinel{}));
    }
    {
        EXPECT_TRUE(text::contains(
            U"here",
            text::null_sentinel{},
            std::begin(he_array),
            std::end(he_array)));
    }
    {
        EXPECT_TRUE(text::contains(
            std::begin(here_array),
            std::end(here_array),
            std::begin(re_array),
            std::end(re_array)));
    }

    // Code point ranges.
    {
        EXPECT_TRUE(text::contains("here", "he"));
    }
    {
        auto const here = text::as_utf32("here");
        EXPECT_TRUE(text::contains(here, "he"));
    }
    {
        auto const he = text::as_utf32("he");
        EXPECT_TRUE(text::contains("here", he));
    }
    {
        auto const here = text::as_utf32("here");
        auto const he = text::as_utf32("he");
        EXPECT_TRUE(text::contains(here, he));
    }
    {
        EXPECT_TRUE(text::contains(u8"here", u"he"));
    }
    {
        EXPECT_TRUE(text::contains(U"here", u"re"));
    }
    {
        EXPECT_TRUE(text::contains(u8"here", U"er"));
    }
    {
        EXPECT_FALSE(text::contains("he", "here"));
    }
    {
        EXPECT_FALSE(text::contains("here", "hea"));
    }

    // Grapheme iterators.
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"er", text::null_sentinel{});
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(U"re", text::null_sentinel{});
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(U"he", text::null_sentinel{});
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(U"here", text::null_sentinel{});
        auto const r2 = text::as_graphemes(he_array);
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }
    {
        auto const r1 = text::as_graphemes(here_array);
        auto const r2 = text::as_graphemes(he_array);
        EXPECT_TRUE(text::contains(r1.begin(), r1.end(), r2.begin(), r2.end()));
    }

    // Grapheme ranges.
    {
        EXPECT_TRUE(text::contains(
            text::as_graphemes(here_array), text::as_graphemes(he_array)));
    }
#if __cplusplus <= 201703L // TODO: Fix!
    {
        EXPECT_TRUE(text::contains(text::as_graphemes(here_array), "he"));
    }
    {
        EXPECT_TRUE(
            text::contains(text::as_graphemes(here_array), (char const *)"he"));
    }
    {
        EXPECT_TRUE(text::contains(u"here", text::as_graphemes(he_array)));
    }
    {
        EXPECT_TRUE(text::contains(
            (char16_t const *)u"here", text::as_graphemes(he_array)));
    }
#endif
}
