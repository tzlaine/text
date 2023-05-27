// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/grapheme_view.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;

char32_t cps_0[2] = {1, 0};
char32_t const (&cps_1)[2] = cps_0;
char8_t cus_0[2] = {1, 0};
char8_t const (&cus_1)[2] = cus_0;

TEST(break_apis, grapheme_break)
{
    {
        auto r_0 = cps_0 | as_graphemes;
        auto r_1 = cps_1 | as_graphemes;

        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }
    {
        auto r_0 = (uint32_t *)cps_0 | as_graphemes;
        auto r_1 = (uint32_t const *)cps_1 | as_graphemes;

        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }

    {
        auto r_0 = cus_0 | as_graphemes;
        auto r_1 = cus_1 | as_graphemes;

        auto x = typename std::iterator_traits<
            utf_iterator<format::utf8, format::utf32, char8_t *>>::
            iterator_category();
        r_0.begin();
        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }
    {
        auto r_0 = (uint32_t *)cus_0 | as_graphemes;
        auto r_1 = (uint32_t const *)cus_1 | as_graphemes;

        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }
}
