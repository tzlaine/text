// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_view.hpp>
#include <boost/text/detail/utility.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(utility, test_strlen)
{
    EXPECT_EQ(text::detail::strlen(""), 0);
    EXPECT_EQ(text::detail::strlen("a"), 1);
    EXPECT_EQ(text::detail::strlen("--------------------"), 20);
}

TEST(utility, test_strrchr)
{
    constexpr char const * empty = "";
    EXPECT_EQ(text::detail::strrchr(nullptr, nullptr, 't'), nullptr);
    EXPECT_EQ(text::detail::strrchr(empty, empty, 't'), empty);
    EXPECT_EQ(text::detail::strrchr(empty, empty + 1, 't'), empty + 1);

    {
        char const str_1[] = "not empty";
        char const * str_1_end = str_1 + sizeof(str_1);

        EXPECT_EQ(text::detail::strrchr(str_1, str_1, 't'), str_1);
        EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 'x'), str_1_end);
        EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 't'), str_1 + 7);
        EXPECT_EQ(text::detail::strrchr(str_1, str_1_end, 'n'), str_1);
    }

    {
        char const str_2[] = {'n'};
        char const * str_2_end = str_2 + sizeof(str_2);

        EXPECT_EQ(text::detail::strrchr(str_2, str_2, 'n'), str_2);
        EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, 'x'), str_2_end);
        EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, 'n'), str_2);
        EXPECT_EQ(text::detail::strrchr(str_2, str_2_end, '\0'), str_2_end);
    }
}

TEST(utility, test_min_)
{
    EXPECT_EQ(text::detail::min_(1, 1), 1);
    EXPECT_EQ(text::detail::min_(1, 2), 1);
    EXPECT_EQ(text::detail::min_(2, 1), 1);
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
}
