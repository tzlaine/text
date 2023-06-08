// Copyright (C) 2023 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/transcode_view.hpp>

#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


struct char32_convertible
{
    operator char32_t() const { return char32_t(value); }
    int value;
};

TEST(charn_view, char8_t_)
{
    char8_t expected_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};
    char8_t expected[] = {'a', 'b', 'c', 'd', 'e'};

    {
        char char_utf8_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = char_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v1 = (char *)char_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));

        auto v3 = v1 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        char char_utf8[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = char_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (char *)char_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = v1 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        char8_t char8_utf8_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = char8_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v1 = (char8_t *)char8_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char8_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));

        auto v3 = v1 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char8_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        char8_t char8_utf8[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = char8_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (char8_t *)char8_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char8_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = v1 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          char8_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        std::string str = "abcde";

        auto v0 = str | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::string("abcde") | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::string::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        std::u8string const str = u8"abcde";

        auto v0 = str | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::u8string(u8"abcde") | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::u8string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::u8string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        std::vector<int> vec = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = vec | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 =
            std::vector<int>{'a', 'b', 'c', 'd', 'e'} | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::vector<int>::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf32;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          std::vector<int>::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }
}

TEST(charn_view, char16_t_)
{
    char16_t expected_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};
    char16_t expected[] = {'a', 'b', 'c', 'd', 'e'};

    {
        int16_t int_utf16_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = int_utf16_null | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          int16_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));
    }

    {
        int16_t int_utf16[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = int_utf16 | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          int16_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));
    }

    {
        char16_t char16_utf16_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = char16_utf16_null | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v1 = (char16_t *)char16_utf16_null | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          char16_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));

        auto v3 = v1 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          char16_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        char16_t char16_utf16[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = char16_utf16 | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (char16_t *)char16_utf16 | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          char16_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = v1 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          char16_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

#if defined(_MSC_VER)
    {
        std::wstring str = L"abcde";

        auto v0 = str | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::wstring(L"abcde") | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::wstring::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::wstring::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }
#endif

    {
        std::u16string const str = u"abcde";

        auto v0 = str | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::u16string(u"abcde") | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::u16string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::u16string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        std::vector<int> vec = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = vec | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 =
            std::vector<int>{'a', 'b', 'c', 'd', 'e'} | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::vector<int>::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf8;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf16,
                          boost::text::format::utf8,
                          std::vector<int>::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }
}

TEST(charn_view, char32_t_)
{
    char32_t expected_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};
    char32_t expected[] = {'a', 'b', 'c', 'd', 'e'};

    {
        int int_utf32_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = int_utf32_null | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          int *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));
    }

    {
        int int_utf32[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = int_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          int *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));
    }

    {
        char32_t char32_utf32_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = char32_utf32_null | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v1 = (char32_t *)char32_utf32_null | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          char32_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected_null));

        auto v3 = v1 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          char32_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        char32_t char32_utf32[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = char32_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (char32_t *)char32_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          char32_t *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = v1 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          char32_t *,
                          boost::text::null_sentinel_t>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

#if !defined(_MSC_VER)
    {
        std::wstring str = L"abcde";

        auto v0 = str | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::wstring(L"abcde") | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::wstring::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::wstring::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }
#endif

    {
        std::u32string const str = U"abcde";

        auto v0 = str | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = std::u32string(U"abcde") | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::u32string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::u32string::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    {
        std::vector<int> vec = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = vec | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 =
            std::vector<int>{'a', 'b', 'c', 'd', 'e'} | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::vector<int>::iterator>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));

        auto v3 = std::move(v1) | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v3.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          std::vector<int>::const_iterator>>);
        EXPECT_TRUE(std::ranges::equal(v3, expected));
    }

    // weird UDT
    {
        char32_convertible conv_utf32[] = {{'a'}, {'b'}, {'c'}, {'d'}, {'e'}};

        auto v0 = conv_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v2 = v0 | boost::text::as_utf16;
        static_assert(std::same_as<
                      decltype(v2.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf32,
                          boost::text::format::utf16,
                          char32_convertible *>>);
        EXPECT_TRUE(std::ranges::equal(v2, expected));
    }
}
