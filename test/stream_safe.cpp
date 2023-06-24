// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/stream_safe.hpp>
#include <boost/text/transcode_view.hpp>

#include "ill_formed.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>


using namespace boost::text;

TEST(stream_safe, no_truncation_needed)
{
    {
        char8_t const * already_stream_safe =
            u8"This is already in stream-safe format.";

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::u8string result;
        auto const retval = stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(retval.in, std::end(as_utf32(already_stream_safe)));

        EXPECT_TRUE(result == already_stream_safe);

        {
            std::u8string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }

        {
            std::u8string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const ss = as_stream_safe(utf32);
            auto it = ss.begin();
            while (it != ss.end()) {
                ++it;
            }
            std::vector<char32_t> cps;
            std::copy(
                std::make_reverse_iterator(it),
                std::make_reverse_iterator(ss.begin()),
                std::back_inserter(cps));
            std::reverse(cps.begin(), cps.end());
            std::vector<char32_t> ss_copy(ss.begin(), ss.end());
            EXPECT_EQ(cps, ss_copy);
        }
    }

    {
        char8_t const * already_stream_safe =
            u8"This is already in \u0308 stream-safe format.";

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::u8string result;
        auto const retval = stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(retval.in, std::end(as_utf32(already_stream_safe)));

        EXPECT_TRUE(result == already_stream_safe);

        {
            std::u8string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }
    }

    {
        char8_t const * already_stream_safe = u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"This is already in "
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"."; // ' stream-safe format'

        EXPECT_TRUE(is_stream_safe(as_utf32(already_stream_safe)));

        std::u8string result;
        auto const retval = stream_safe_copy(
            as_utf32(already_stream_safe), from_utf32_back_inserter(result));

        EXPECT_EQ(retval.in, std::end(as_utf32(already_stream_safe)));

        EXPECT_TRUE(result == already_stream_safe);

        {
            std::u8string str = already_stream_safe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_EQ(it, utf32.end());
        }

        {
            std::u8string str = already_stream_safe;
            auto const ss = str | as_stream_safe | as_utf8;
            auto const first = ss.begin();
            auto it = ss.end();
            std::u8string result;
            while (it != first) {
                --it;
                result += *it;
            }
            std::ranges::reverse(str);
            EXPECT_TRUE(result == str);
        }
    }
}

TEST(stream_safe, truncation_needed_short)
{
    {
        char8_t const * stream_unsafe =
            u8"Needs truncation: 2"
            // 10 combiners
            u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 10 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 5 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8".";

        EXPECT_FALSE(is_stream_safe(as_utf32(stream_unsafe)));

        std::u8string const expected =
            u8"Needs truncation: 2"
            u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8".";

        {
            std::u8string result;
            auto const retval = stream_safe_copy(
                as_utf32(stream_unsafe), from_utf32_back_inserter(result));
            EXPECT_EQ(retval.in, std::end(as_utf32(stream_unsafe)));
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }

        {
            std::u8string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32);
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }
 
        {
            std::u8string result;
            auto const v = as_stream_safe(stream_unsafe);
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }

        {
            std::u8string str = stream_unsafe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_NE(it, utf32.end());
            str.erase(it.base(), str.end());
            EXPECT_TRUE(str == expected);
        }

        {
            std::u8string str = stream_unsafe;
            auto const utf32 = as_utf32(str);
            auto const ss = as_stream_safe(utf32);
            auto it = ss.begin();
            while (it != ss.end()) {
                ++it;
            }
            std::vector<char32_t> cps;
            std::copy(
                std::make_reverse_iterator(it),
                std::make_reverse_iterator(ss.begin()),
                std::back_inserter(cps));
            std::reverse(cps.begin(), cps.end());
            std::vector<char32_t> ss_copy(ss.begin(), ss.end());
            EXPECT_EQ(cps, ss_copy);
        }
        {
            std::u8string str = stream_unsafe;
            auto const ss = str | as_stream_safe | as_utf8;
            auto const first = ss.begin();
            auto it = ss.end();
            std::u8string result;
            while (it != first) {
                --it;
                result += *it;
            }
            auto reversed_expected = expected;
            std::ranges::reverse(reversed_expected);
            EXPECT_TRUE(result == reversed_expected);
        }
    }
}

TEST(stream_safe, truncation_needed_long)
{
    {
        char8_t const * stream_unsafe = u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 10 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 5 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"Needs truncation: 2"
            // 10 combiners
            u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 10 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 5 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8".";

        EXPECT_FALSE(is_stream_safe(as_utf32(stream_unsafe)));

        std::u8string const expected = u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 10 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"Needs truncation: 2"
            // 10 combiners
            u8"\u0300"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            // 10 combiners
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8"\u0308"
            u8".";

        {
            std::u8string result;
            auto const retval = stream_safe_copy(
                as_utf32(stream_unsafe), from_utf32_back_inserter(result));
            EXPECT_EQ(retval.in, std::end(as_utf32(stream_unsafe)));
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }
 
        {
            std::u8string result;
            auto utf32 = as_utf32(stream_unsafe);
            auto const v = as_stream_safe(utf32);
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);

            std::string result2(result.begin(), result.end());
            std::string expected2(expected.begin(), expected.end());
            EXPECT_EQ(result2, expected2);
        }
 
        {
            std::u8string result;
            auto const v = as_stream_safe(stream_unsafe);
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }

        {
            std::u8string str = stream_unsafe;
            auto const utf32 = as_utf32(str);
            auto const it = stream_safe(utf32);
            EXPECT_NE(it, utf32.end());
            str.erase(it.base(), str.end());
            EXPECT_TRUE(str == expected);
        }
 
        {
            std::u8string result;
            auto const v = stream_unsafe | as_utf32 | as_stream_safe;
            // Can't use std::copy() (or even range-base for) because of the
            // sentinel
            for (auto it = v.begin(); it != v.end(); ++it) {
                *from_utf32_back_inserter(result)++ = *it;
            }
            EXPECT_FALSE(stream_unsafe == result);
            EXPECT_TRUE(result == expected);
        }

        {
            std::u8string str = stream_unsafe;
            auto const ss = str | as_stream_safe | as_utf8;
            auto const first = ss.begin();
            auto it = ss.end();
            std::u8string result;
            while (it != first) {
                --it;
                result += *it;
            }
            auto reversed_expected = expected;
            std::ranges::reverse(reversed_expected);
            EXPECT_TRUE(result == reversed_expected);
        }
    }
}

template<typename T>
using decrement = decltype(--std::declval<T>());

TEST(stream_safe, view)
{
    { // bidi, common_range, and utf_iterators
        std::u8string str = u8".";

        auto v = stream_safe_view(str | as_utf32);
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        --last;
    }
    { // bidi, common_range, and !utf_iterators
        std::u32string str = U".";

        auto v = stream_safe_view(
            std::ranges::subrange(str.data(), str.data() + str.size()));
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        --last;
    }
    { // bidi, !common_range, and utf_iterators
        std::u8string str = u8".";

        auto v = stream_safe_view(str.c_str() | as_utf32);
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        static_assert(ill_formed<decrement, decltype(first)>::value);
    }
    { // bidi, !common_range, and !utf_iterators
        std::u32string str = U".";

        auto v =
            stream_safe_view(std::ranges::subrange(str.c_str(), null_sentinel));
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        static_assert(ill_formed<decrement, decltype(first)>::value);
    }

    { // !bidi, common_range, and utf_iterators
        std::u8string str = u8".";
        std::list<char8_t> list(str.begin(), str.end());

        auto v = stream_safe_view(list | as_utf32);
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        --last;
    }
    { // !bidi, common_range, and !utf_iterators
        std::u32string str = U".";
        std::list<char32_t> list(str.begin(), str.end());

        auto v =
            stream_safe_view(std::ranges::subrange(list.begin(), list.end()));
        auto first = v.begin();
        auto last = v.end();
        ++first;
        EXPECT_TRUE(first == last);
        --last;
    }
}
