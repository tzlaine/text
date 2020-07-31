// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/normalize.hpp>
#include <boost/text/transcode_view.hpp>

#include <gtest/gtest.h>

#include <string>


using namespace boost::text;

TEST(stream_safe, no_truncation_needed)
{
    {
        char const * already_stream_safe =
            "This is already in stream-safe format.";
        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);
    }

    {
        char const * already_stream_safe =
            (char const *)u8"This is already in \u0308 stream-safe format.";
        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);
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
        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);
    }
}

TEST(stream_safe, truncation_needed_once)
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
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
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

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }

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
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
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

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }
}

TEST(stream_safe, truncation_needed_twice)
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
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
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

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }

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
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
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

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }
}
