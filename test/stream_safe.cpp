// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/stream_safe.hpp>
#include <boost/text/transcode_view.hpp>

#include <gtest/gtest.h>

#include <string>


using namespace boost::text;

TEST(stream_safe, no_truncation_needed)
{
    {
        char const * already_stream_safe =
            "This is already in stream-safe format.";

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);

        {
            std::string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }
    }

    {
        char const * already_stream_safe =
            (char const *)u8"This is already in \u0308 stream-safe format.";

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);

        {
            std::string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }
    }

    {
        char const * already_stream_safe =
            (char const *)u8"This is already in "
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            " stream-safe format.";

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);

        {
            std::string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }
    }
}

TEST(stream_safe, truncation_needed_short)
{
    {
        char const * stream_unsafe =
            (char const *)u8"Needs truncation: 2"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            ".";

        EXPECT_FALSE(is_stream_safe(as_utf32(stream_unsafe)));

        std::string const expected =
            (char const *)u8"Needs truncation: 2"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            ".";

        {
            std::string result;
            stream_safe_copy(
                as_utf32(stream_unsafe), from_utf32_back_inserter(result));
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }

        {
            std::string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32.begin(), utf32.end());
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }
 
        {
            std::string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32.begin(), null_sentinel{});
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }

        {
            std::string str = stream_unsafe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_NE(it, utf32.end());
            str.erase(it.base(), str.end());
            EXPECT_EQ(str, expected);
        }
   }
}

TEST(stream_safe, truncation_needed_long)
{
    {
        char const * stream_unsafe =
            (char const *)u8"Needs truncation: 2"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            ".";

        EXPECT_FALSE(is_stream_safe(as_utf32(stream_unsafe)));

        std::string const expected =
            (char const *)u8"Needs truncation: 2"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            // 10 combiners
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            "\u0308"
            ".";

        {
            std::string result;
            stream_safe_copy(
                as_utf32(stream_unsafe), from_utf32_back_inserter(result));
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }
 
        {
            std::string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32.begin(), utf32.end());
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }
 
        {
            std::string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32.begin(), null_sentinel{});
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_NE(stream_unsafe, result);
            EXPECT_EQ(result, expected);
        }

        {
            std::string str = stream_unsafe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_NE(it, utf32.end());
            str.erase(it.base(), str.end());
            EXPECT_EQ(str, expected);
        }
    }
}
