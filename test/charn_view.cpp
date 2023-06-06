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
    }

    {
        char8_t char8_utf8_null[] = {'a', 'b', 'c', 'd', 'e', '\0'};

        auto v0 = char8_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected_null));

        auto v1 = (char *)char8_utf8_null | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
    }

    {
        char8_t char8_utf8[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = char8_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (char *)char8_utf8 | boost::text::as_char8_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char8_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
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

        auto v1 = (int16_t *)int_utf16_null | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
    }

    {
        int16_t int_utf16[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = int_utf16 | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (int16_t *)int_utf16 | boost::text::as_char16_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char16_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
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

        auto v1 = (int *)int_utf32_null | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
    }

    {
        int int_utf32[] = {'a', 'b', 'c', 'd', 'e'};

        auto v0 = int_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));

        auto v1 = (int *)int_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v1)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v1, expected));
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
    }

    // weird UDT
    {
        char32_convertible conv_utf32[] = {{'a'}, {'b'}, {'c'}, {'d'}, {'e'}};

        auto v0 = conv_utf32 | boost::text::as_char32_t;
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(v0)>, char32_t>);
        EXPECT_TRUE(std::ranges::equal(v0, expected));
    }
}
