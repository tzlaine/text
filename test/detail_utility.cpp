// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_view.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(utility, test_strlen)
{
    EXPECT_EQ(text::detail::strlen(""), 0);
    EXPECT_EQ(text::detail::strlen("a"), 1);
    EXPECT_EQ(text::detail::strlen("--------------------"), 20);

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

    static_assert(text::detail::strlen("") == 0, "");
    static_assert(text::detail::strlen("a") == 1, "");
    static_assert(text::detail::strlen("--------------------") == 20, "");

#endif
}

TEST(utility, test_strrchr){{constexpr char const * empty = "";
EXPECT_EQ(text::detail::strrchr(nullptr, nullptr, 't'), nullptr);
EXPECT_EQ(text::detail::strrchr(empty, empty, 't'), empty);
EXPECT_EQ(text::detail::strrchr(empty, empty + 1, 't'), empty + 1);

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

static_assert(text::detail::strrchr(nullptr, nullptr, 't') == nullptr, "");
static_assert(text::detail::strrchr(empty, empty, 't') == empty, "");
static_assert(text::detail::strrchr(empty, empty + 1, 't') == empty + 1, "");

#endif
}

{
    char const str_1[] = "not empty";
    char const * str_1_end = str_1 + sizeof(str_1);

    EXPECT_EQ(text::detail::strrchr(str_1, str_1, 't'), str_1);
    EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 'x'), str_1_end);
    EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 't'), str_1 + 7);
    EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 'n'), str_1);
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

{
    constexpr boost::text::string_view const str_1("not empty");

    static_assert(
        text::detail::strrchr(str_1.begin(), str_1.begin(), 't') ==
            str_1.begin(),
        "");
    static_assert(
        text::detail::strrchr(str_1.begin(), str_1.end(), 'x') == str_1.end(),
        "");
    static_assert(
        text::detail::strrchr(str_1.begin(), str_1.end(), 't') ==
            str_1.begin() + 7,
        "");
    static_assert(
        text::detail::strrchr(str_1.begin(), str_1.end(), 'n') == str_1.begin(),
        "");
}

#endif

{
    char const str_2[] = {'n'};
    char const * str_2_end = str_2 + sizeof(str_2);

    EXPECT_EQ(text::detail::strrchr(str_2, str_2, 'n'), str_2);
    EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, 'x'), str_2_end);
    EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, 'n'), str_2);
    EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, '\0'), str_2_end);
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

{
    constexpr boost::text::string_view const str_2("n", 1);

    static_assert(
        text::detail::strrchr(str_2.begin(), str_2.begin(), 'n') ==
            str_2.begin(),
        "");
    static_assert(
        text::detail::strrchr(str_2.begin(), str_2.end(), 'x') == str_2.end(),
        "");
    static_assert(
        text::detail::strrchr(str_2.begin(), str_2.end(), 'n') == str_2.begin(),
        "");
    static_assert(
        text::detail::strrchr(str_2.begin(), str_2.end(), '\0') == str_2.end(),
        "");
}

#endif
}

TEST(utility, test_min_)
{
    EXPECT_EQ(text::detail::min_(1, 1), 1);
    EXPECT_EQ(text::detail::min_(1, 2), 1);
    EXPECT_EQ(text::detail::min_(2, 1), 1);

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

    static_assert(text::detail::min_(1, 1) == 1, "");
    static_assert(text::detail::min_(1, 2) == 1, "");
    static_assert(text::detail::min_(2, 1) == 1, "");

#endif
}

TEST(utility, test_compare_impl)
{
    {
        char const empty[] = "";
        char const non_empty_a[] = "a";
        char const non_empty_b[] = "b";
        char const non_empty_ab[] = "ab";

        EXPECT_EQ(text::detail::compare_impl(empty, empty, empty, empty), 0);

        EXPECT_EQ(
            text::detail::compare_impl(
                non_empty_a, non_empty_a + 1, non_empty_a, non_empty_a + 1),
            0);
        EXPECT_LT(
            text::detail::compare_impl(
                empty, empty, non_empty_a, non_empty_a + 1),
            0);
        EXPECT_GT(
            text::detail::compare_impl(
                non_empty_a, non_empty_a + 1, empty, empty),
            0);

        EXPECT_LT(
            text::detail::compare_impl(
                non_empty_a, non_empty_a + 1, non_empty_b, non_empty_b + 1),
            0);
        EXPECT_GT(
            text::detail::compare_impl(
                non_empty_b, non_empty_b + 1, non_empty_a, non_empty_a + 1),
            0);

        EXPECT_LT(
            text::detail::compare_impl(
                non_empty_a, non_empty_a + 1, non_empty_ab, non_empty_ab + 2),
            0);
        EXPECT_GT(
            text::detail::compare_impl(
                non_empty_ab, non_empty_ab + 2, non_empty_a, non_empty_a + 1),
            0);
    }

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

    {
        constexpr text::string_view const empty("");
        constexpr text::string_view const non_empty_a("a");
        constexpr text::string_view const non_empty_b("b");
        constexpr text::string_view const non_empty_ab("ab");

        static_assert(
            text::detail::compare_impl(
                empty.begin(), empty.begin(), empty.begin(), empty.begin()) ==
                0,
            "");

        static_assert(
            text::detail::compare_impl(
                non_empty_a.begin(),
                non_empty_a.end(),
                non_empty_a.begin(),
                non_empty_a.end()) == 0,
            "");
        static_assert(
            text::detail::compare_impl(
                empty.begin(),
                empty.begin(),
                non_empty_a.begin(),
                non_empty_a.end()) < 0,
            "");
        static_assert(
            text::detail::compare_impl(
                non_empty_a.begin(),
                non_empty_a.end(),
                empty.begin(),
                empty.begin()) > 0,
            "");

        static_assert(
            text::detail::compare_impl(
                non_empty_a.begin(),
                non_empty_a.end(),
                non_empty_b.begin(),
                non_empty_b.end()) < 0,
            "");
        static_assert(
            text::detail::compare_impl(
                non_empty_b.begin(),
                non_empty_b.end(),
                non_empty_a.begin(),
                non_empty_a.end()) > 0,
            "");

        static_assert(
            text::detail::compare_impl(
                non_empty_a.begin(),
                non_empty_a.end(),
                non_empty_ab.begin(),
                non_empty_ab.end()) < 0,
            "");
        static_assert(
            text::detail::compare_impl(
                non_empty_ab.begin(),
                non_empty_ab.end(),
                non_empty_a.begin(),
                non_empty_a.end()) > 0,
            "");
    }

#endif
}
