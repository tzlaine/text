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

/* Some code points and their NFKD decompositions (with Canonical Combining Class).
    \u0f81     0xf81   nfkd--> 0xf71(ccc=81) 0xf80(ccc=82)
    \uff9f     0xff9f  nfkd--> 0x309a(ccc=8) 
    \uff9e     0xff9e  nfkd--> 0x3099(ccc=8) 
    \u0f73     0xf73   nfkd--> 0xf71(ccc=81) 0xf72(ccc=82) 
    \u0340     0x340   nfkd--> 0x300(ccc=e6) 
    \U0001d1bd 0x1d1bd nfkd--> 0x1d1b9(ccc=0) 0x1d165(ccc=d8) 0x1d16e(ccc=d8) 
    \U0001d1bc 0x1d1bc nfkd--> 0x1d1ba(ccc=0) 0x1d165(ccc=d8) 
    \U0001ee75 0x1ee75 nfkd--> 0x62a(ccc=0) 
    \U0001ee72 0x1ee72 nfkd--> 0x642(ccc=0) 
*/

TEST(stream_safe, no_CGJ_needed)
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
            (char const *)u8"This is already in \u0f81 stream-safe format.";
        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);
    }

    {
        char const * already_stream_safe =
            (char const *)u8"This is already in "
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            " stream-safe format.";
        std::string result;
        stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(result, already_stream_safe);
    }
}

TEST(stream_safe, CGJ_needed_once)
{
    {
        char const * stream_unsafe =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\uff9f"
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
        std::string const expected =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\u034f\uff9f"
            ".";

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }

    {
        char const * stream_unsafe =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
        std::string const expected =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u034f\u0f81"
            ".";

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }
}

TEST(stream_safe, CGJ_needed_twice)
{
    {
        char const * stream_unsafe =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\uff9f"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\uff9f"
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
        std::string const expected =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\u034f\uff9f"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\uff9f"
            "\u034f\uff9f"
            ".";

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }

    {
        char const * stream_unsafe =  // TODO
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            ".";
        std::string result;
        stream_safe_copy(
            as_utf32(stream_unsafe), from_utf32_back_inserter(result));
        std::string const expected =
            (char const *)u8"Needs CGJ: \U0001d1bc" // one combiner
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u034f\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            // 10 combiners
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u0f81"
            "\u034f\u0f81"
            ".";

        EXPECT_NE(stream_unsafe, result);
        EXPECT_EQ(result, expected);
    }
}
