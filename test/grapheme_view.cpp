// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/grapheme_view.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


using namespace boost::text;

// Unicode 9, 3.9/D90-D92
char32_t const utf32_[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
char16_t const utf16_[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
char8_t const utf8_[10] = {
    0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

char32_t const utf32_null[5] = {0x004d, 0x0430, 0x4e8c, 0x10302, 0};
char16_t const utf16_null[6] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02, 0};
char8_t const utf8_null[11] = {
    0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82, 0};

TEST(grapheme_view, as_graphemes_)
{
    // array
    {
        auto r_8 = as_graphemes(utf8_);
        auto r_16 = as_graphemes(utf16_);
        auto r_32 = as_graphemes(utf32_);

        EXPECT_TRUE(boost::algorithm::equal(
            r_8.begin(), r_8.end(), r_16.begin(), r_16.end()));

        EXPECT_TRUE(boost::algorithm::equal(
            r_8.begin(), r_8.end(), r_32.begin(), r_32.end()));
    }

    // ptr/sentinel
    {
        auto r_8 =
            as_graphemes(std::ranges::subrange(utf8_null, null_sentinel));
        auto r_16 =
            as_graphemes(std::ranges::subrange(utf16_null, null_sentinel));
        auto r_32 =
            as_graphemes(std::ranges::subrange(utf32_null, null_sentinel));

        int i = 0;
        auto r_16_it = r_16.begin();
        for (auto it = r_8.begin(); it != r_8.end(); ++it, ++i, ++r_16_it) {
            EXPECT_EQ(*it, *r_16_it) << "iteration " << i;
        }

        i = 0;
        auto r_32_it = r_32.begin();
        for (auto it = r_8.begin(); it != r_8.end(); ++it, ++i, ++r_32_it) {
            EXPECT_EQ(*it, *r_32_it) << "iteration " << i;
        }
    }

    // single pointers
    {
        auto r_8 = as_graphemes((char8_t const *)utf8_null);
        auto r_16 = as_graphemes((char16_t const *)utf16_null);
        auto r_32 = as_graphemes((char32_t const *)utf32_null);

        int i = 0;
        auto r_16_it = r_16.begin();
        for (auto it = r_8.begin(); it != r_8.end(); ++it, ++i, ++r_16_it) {
            EXPECT_EQ(*it, *r_16_it) << "iteration " << i;
        }

        i = 0;
        auto r_32_it = r_32.begin();
        for (auto it = r_8.begin(); it != r_8.end(); ++it, ++i, ++r_32_it) {
            EXPECT_EQ(*it, *r_32_it) << "iteration " << i;
        }
    }

    // stream inserters
    {
        auto r = as_graphemes(utf8_);
        std::stringstream ss;
        ss << r;
        std::string str_ = ss.str();
        std::u8string str(str_.begin(), str_.end());
        EXPECT_TRUE(std::equal(
            str.begin(),
            str.end(),
            r.begin().base().base(),
            r.end().base().base()));
    }
    {
        auto r = as_graphemes(std::ranges::subrange(utf8_null, null_sentinel));
        std::stringstream ss;
        ss << r;
        std::string str_ = ss.str();
        std::u8string str(str_.begin(), str_.end());
        
        int i = 0;
        auto str_it = str.begin();
        for (auto it = r.begin().base().base(); it != null_sentinel;
             ++it, ++i, ++str_it) {
            EXPECT_EQ(*it, *str_it) << "iteration " << i;
        }
    }
}
