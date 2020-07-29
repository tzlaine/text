// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <gtest/gtest.h>

#include <deque>
#include <list>


using namespace boost;

uint32_t const cps[] = {
    0x1053B, 0x0062, 0x1053C, 0x0021, 0x1053C, 0x003F, 0x1053C, 0x0334,
    0x1053C, 0x0061, 0x1053C, 0x0041, 0x1053C, 0x0062, 0x1053D, 0x0021,
    0x1053D, 0x003F, 0x1053D, 0x0334, 0x1053D, 0x0061, 0x1053D, 0x0041,
    0x1053D, 0x0062, 0x1053E, 0x0021, 0x1053E, 0x003F, 0x1053E, 0x0334,
    0x1053E, 0x0061, 0x1053E, 0x0041, 0x1053E, 0x0062, 0x1053F, 0x0021,
    0x1053F, 0x003F, 0x1053F, 0x0334, 0x1053F, 0x0061, 0x1053F, 0x0041,
    0x1053F, 0x0062, 0x10540, 0x0021, 0x10540, 0x003F, 0x10540, 0x0334,
    0x10540, 0x0061, 0x10540, 0x0041, 0x10540, 0x0062, 0x10541, 0x0021,
    0x10541, 0x003F, 0x10541, 0x0334, 0x10541, 0x0061, 0x10541, 0x0041,
    0x10541, 0x0062, 0x10542, 0x0021, 0x10542, 0x003F, 0x10542, 0x0334,
    0x10542, 0x0061, 0x10542, 0x0041, 0x10542, 0x0062, 0x10543, 0x0021,
    0x10543, 0x003F, 0x10543, 0x0334, 0x10543, 0x0061, 0x10543, 0x0041,
    0x10543, 0x0062, 0x10544, 0x0021, 0x10544, 0x003F, 0x10544, 0x0334,
    0x10544, 0x0061, 0x10544, 0x0041, 0x10544, 0x0062, 0x10545, 0x0021,
    0x10545, 0x003F, 0x10545, 0x0334, 0x10545, 0x0061, 0x10545, 0x0041,
    0x10545, 0x0062, 0x10546, 0x0021, 0x10546, 0x003F, 0x10546, 0x0334,
    0x10546, 0x0061, 0x10546, 0x0041, 0x10546, 0x0062, 0x10547, 0x0021,
    0x10547, 0x003F, 0x10547, 0x0334, 0x10547, 0x0061, 0x10547, 0x0041,
    0x10547, 0x0062, 0x10548, 0x0021, 0x10548, 0x003F, 0x10548, 0x0334,
    0x10548, 0x0061, 0x10548, 0x0041, 0x10548, 0x0062, 0x10549, 0x0021,
    0x10549, 0x003F, 0x10549, 0x0334, 0x10549, 0x0061, 0x10549, 0x0041,
    0x10549, 0x0062, 0x1054A, 0x0021, 0x1054A, 0x003F, 0x1054A, 0x0334,
    0x1054A, 0x0061, 0x1054A, 0x0041, 0x1054A, 0x0062, 0x1054B, 0x0021,
    0x1054B, 0x003F, 0x1054B, 0x0334, 0x1054B, 0x0061, 0x1054B, 0x0041,
    0x1054B, 0x0062, 0x1054C, 0x0021, 0x1054C, 0x003F, 0x1054C, 0x0334,
    0x1054C, 0x0061, 0x1054C, 0x0041, 0x1054C, 0x0062, 0x1054D, 0x0021,
    0x1054D, 0x003F, 0x1054D, 0x0334, 0x1054D, 0x0061, 0x1054D, 0x0041,
    0x1054D, 0x0062, 0x1054E, 0x0021, 0x1054E, 0x003F, 0x1054E, 0x0334,
    0x1054E, 0x0061, 0x1054E, 0x0041, 0x1054E, 0x0062, 0x1054F, 0x0021,
    0x1054F, 0x003F, 0x1054F, 0x0334, 0x1054F, 0x0061, 0x1054F, 0x0041,
    0x1054F, 0x0062, 0x10550, 0x0021, 0x10550, 0x003F, 0x10550, 0x0334,
    0x10550, 0x0061, 0x10550, 0x0041, 0x10550, 0x0062, 0x10551, 0x0021,
    0x10551, 0x003F, 0x10551, 0x0334, 0x10551, 0x0061, 0x10551, 0x0041,
    0x10551, 0x0062, 0x10552, 0x0021, 0x10552, 0x003F, 0x10552, 0x0334,
    0x10552, 0x0061, 0x10552, 0x0041, 0x10552, 0x0062, 0x10553, 0x0021,
    0x10553, 0x003F, 0x10553, 0x0334, 0x10553, 0x0061, 0x10553, 0x0041,
    0x10553, 0x0062, 0x10554, 0x0021, 0x10554, 0x003F, 0x10554, 0x0334,
    0x10554, 0x0061, 0x10554, 0x0041, 0x10554, 0x0062, 0x10555, 0x0021,
    0x10555, 0x003F, 0x10555, 0x0334, 0x10555, 0x0061, 0x10555, 0x0041,
    0x10555, 0x0062, 0x10556, 0x0021, 0x10556, 0x003F, 0x10556, 0x0334,
    0x10556, 0x0061, 0x10556, 0x0041, 0x10556, 0x0062, 0x10557, 0x0021,
    0x10557, 0x003F, 0x10557, 0x0334, 0x10557, 0x0061, 0x10557, 0x0041,
    0x10557, 0x0062, 0x10558, 0x0021, 0x10558, 0x003F, 0x10558, 0x0334,
    0x10558, 0x0061, 0x10558, 0x0041, 0x10558, 0x0062, 0x10559, 0x0021,
    0x10559, 0x003F, 0x10559, 0x0334, 0x10559, 0x0061, 0x10559, 0x0041,
    0x10559, 0x0062, 0x1055A, 0x0021, 0x1055A, 0x003F, 0x1055A, 0x0334,
    0x1055A, 0x0061, 0x1055A, 0x0041, 0x1055A, 0x0062, 0x1055B, 0x0021,
    0x1055B, 0x003F, 0x1055B, 0x0334, 0x1055B, 0x0061, 0x1055B, 0x0041,
    0x1055B, 0x0062, 0x1055C, 0x0021, 0x1055C, 0x003F, 0x1055C, 0x0334,
    0x1055C, 0x0061, 0x1055C, 0x0041, 0x1055C, 0x0062, 0x1055D, 0x0021,
    0x1055D, 0x003F, 0x1055D, 0x0334, 0x1055D, 0x0061, 0x1055D, 0x0041,
    0x1055D, 0x0062, 0x1055E, 0x0021, 0x1055E, 0x003F, 0x1055E, 0x0334,
    0x1055E, 0x0061, 0x1055E, 0x0041, 0x1055E, 0x0062, 0x1055F, 0x0021,
    0x1055F, 0x003F, 0x1055F, 0x0334, 0x1055F, 0x0061, 0x1055F, 0x0041,
    0x1055F, 0x0062, 0x10560, 0x0021, 0x10560, 0x003F, 0x10560, 0x0334,
    0x10560, 0x0061, 0x10560, 0x0041, 0x10560, 0x0062, 0x10561, 0x0021,
    0x10561, 0x003F, 0x10561, 0x0334, 0x10561, 0x0061, 0x10561, 0x0041,
    0x10561, 0x0062, 0x10562, 0x0021, 0x10562, 0x003F, 0x10562, 0x0334,
    0x10562, 0x0061, 0x10562, 0x0041, 0x10562, 0x0062, 0x10563, 0x0021,
    0x10563, 0x003F, 0x10563, 0x0334, 0x10563, 0x0061, 0x10563, 0x0041,
    0x10563, 0x0062, 0x110D0, 0x0021, 0x110D0, 0x003F, 0x110D0, 0x0334,
    0x110D0, 0x0061, 0x110D0, 0x0041, 0x110D0, 0x0062, 0x110D1, 0x0021};

TEST(transcode_insert_iterator, utf16_inserters_long)
{
    std::vector<uint16_t> cus;
    auto const utf16_first =
        text::utf16_iterator(std::begin(cps), std::begin(cps), std::end(cps));
    auto const utf16_last =
        text::utf16_iterator(std::begin(cps), std::end(cps), std::end(cps));
    std::copy(utf16_first, utf16_last, std::back_inserter(cus));

    std::vector<uint32_t> const cps_copy(std::begin(cps), std::end(cps));

    // UTF-16 pointer -> UTF-32 inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf16_inserter(result, result.end()));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-16 pointer -> UTF-32 back inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-16 pointer -> UTF-32 front inserter
    {
        std::deque<uint32_t> const cps_copy_deque(
            std::begin(cps), std::end(cps));
        std::deque<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, cps_copy_deque);
    }

    // UTF-16 pointer -> UTF-8 inserter
    {
        std::vector<char> chars;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf16_inserter(chars, chars.end()));
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-16 pointer -> UTF-8 back inserter
    {
        std::vector<char> chars;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_back_inserter(chars));
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-16 pointer -> UTF-8 front inserter
    {
        std::deque<char> chars;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_front_inserter(chars));
        std::reverse(chars.begin(), chars.end());
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
}

TEST(transcode_insert_iterator, utf16_inserters_errors)
{
    std::vector<uint16_t> cus = {
        (uint16_t)text::low_surrogate_min, // -> replacement
        (uint16_t)text::high_surrogate_min, // -> replacement, due to next CU
        (uint16_t)text::high_surrogate_min, // -> ok
        (uint16_t)text::low_surrogate_min,  // -> ok
        (uint16_t)text::high_surrogate_min, // -> replacement, due to next CU
        0,
    };
    std::vector<uint32_t> expected_cps = {
        text::replacement_character(),
        text::replacement_character(),
        0x10000,
        text::replacement_character(),
        0,
    };

    // UTF-16 pointer -> UTF-32 inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf16_inserter(result, result.end()));
        EXPECT_EQ(result, expected_cps);
    }
    // UTF-16 pointer -> UTF-32 back inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_back_inserter(result));
        EXPECT_EQ(result, expected_cps);
    }
    // UTF-16 pointer -> UTF-32 front inserter
    {
        std::deque<uint32_t> const expected_cps_deque(
            std::begin(expected_cps), std::end(expected_cps));
        std::deque<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expected_cps_deque);
    }
}

TEST(transcode_insert_iterator, utf8_inserters_long)
{
    std::vector<char> cus;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(cus));

    std::vector<uint32_t> const cps_copy(std::begin(cps), std::end(cps));

    // UTF-8 pointer -> UTF-32 inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf8_inserter(result, result.end()));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-8 pointer -> UTF-32 back inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf8_back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-8 pointer -> UTF-32 front inserter
    {
        std::deque<uint32_t> const cps_copy_deque(
            std::begin(cps), std::end(cps));
        std::deque<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf8_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, cps_copy_deque);
    }

    // UTF-8 pointer -> UTF-16 inserter
    {
        std::vector<uint16_t> chars_utf16;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf8_inserter(chars_utf16, chars_utf16.end()));
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars_utf16, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-8 pointer -> UTF-16 back inserter
    {
        std::vector<uint16_t> chars_utf16;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf8_back_inserter(chars_utf16));
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars_utf16, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
    // UTF-8 pointer -> UTF-16 front inserter
    {
        std::deque<uint16_t> chars_utf16;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf8_front_inserter(chars_utf16));
        std::reverse(chars_utf16.begin(), chars_utf16.end());
        std::vector<uint32_t> result;
        text::transcode_to_utf32(chars_utf16, std::back_inserter(result));
        EXPECT_EQ(result, cps_copy);
    }
}

#if 0 // TODO: UTF-8
TEST(transcode_insert_iterator, utf8_inserters_errors)
{
    std::vector<uint16_t> cus = {
        text::low_surrogate_min,  // -> replacement
        text::high_surrogate_min, // -> replacement, due to next CU
        text::high_surrogate_min, // -> ok
        text::low_surrogate_min,  // -> ok
        text::high_surrogate_min, // -> replacement, due to next CU
        0,
    };
    std::vector<uint32_t> expected_cps = {
        text::replacement_character(),
        text::replacement_character(),
        0x10000,
        text::replacement_character(),
        0,
    };

    // UTF-8 pointer -> UTF-32 inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(),
            cus.end(),
            text::from_utf16_inserter(result, result.end()));
        EXPECT_EQ(result, expected_cps);
    }
    // UTF-8 pointer -> UTF-32 back inserter
    {
        std::vector<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_back_inserter(result));
        EXPECT_EQ(result, expected_cps);
    }
    // UTF-8 pointer -> UTF-32 front inserter
    {
        std::deque<uint32_t> const expected_cps_deque(
            std::begin(expected_cps), std::end(expected_cps));
        std::deque<uint32_t> result;
        std::copy(
            cus.begin(), cus.end(), text::from_utf16_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(result, expected_cps_deque);
    }
}
#endif
