// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#define BOOST_TEXT_USE_SIMD 0
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

// Unicode 3.9/D90-D92
constexpr char32_t utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
constexpr char16_t utf16[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
constexpr char8_t utf8[11] = {
    0x4d,
    0xd0,
    0xb0,
    0xe4,
    0xba,
    0x8c,
    0xf0,
    0x90,
    0x8c,
    0x82,
    0,
};

TEST(transcode_algorthm, from_utf8_non_error)
{
    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            std::begin(utf8), std::end(utf8) - 1, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            std::begin(utf8), text::null_sentinel, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        std::list<char8_t> utf8_(std::begin(utf8), std::end(utf8) - 1);
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf32(utf8_.begin(), utf8_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        text::transcode_to_utf32(
            std::begin(utf8), std::end(utf8) - 1, std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        text::transcode_to_utf32(
            std::begin(utf8),
            text::null_sentinel,
            std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        std::list<char8_t> utf8_(std::begin(utf8), std::end(utf8) - 1);
        text::transcode_to_utf32(
            utf8_.begin(), utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }

    // UTF-8 pointer -> UTF-16 pointer
    {
        std::vector<char16_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf16(
            std::begin(utf8), std::end(utf8) - 1, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer and sentinel -> UTF-16 pointer
    {
        std::vector<char16_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf16(
            std::begin(utf8), text::null_sentinel, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 non-pointer -> UTF-16 pointer
    {
        std::vector<char16_t> result(10);
        std::list<char8_t> utf8_(std::begin(utf8), std::end(utf8) - 1);
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf16(utf8_.begin(), utf8_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer -> UTF-16 non-pointer
    {
        std::vector<char16_t> result;
        text::transcode_to_utf16(
            std::begin(utf8), std::end(utf8) - 1, std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer and sentinel -> UTF-16 non-pointer
    {
        std::vector<char16_t> result;
        text::transcode_to_utf16(
            std::begin(utf8),
            text::null_sentinel,
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 non-pointer -> UTF-16 non-pointer
    {
        std::vector<char16_t> result;
        std::list<char8_t> utf8_(std::begin(utf8), std::end(utf8) - 1);
        text::transcode_to_utf16(
            utf8_.begin(), utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
}

TEST(transcode_algorthm, from_utf8_errors_0)
{
    char8_t const bad_utf8[14] = {
        0x61,
        0xf1,
        0x80,
        0x80,
        0xe1,
        0x80,
        0xc2,
        0x62,
        0x80,
        0x63,
        0x80,
        0xbf,
        0x64,
        0,
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            std::begin(bad_utf8), std::end(bad_utf8) - 1, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            std::begin(bad_utf8), text::null_sentinel, &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        std::list<char8_t> bad_utf8_(std::begin(bad_utf8), std::end(bad_utf8) - 1);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            bad_utf8_.begin(), bad_utf8_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        text::transcode_to_utf32(
            std::begin(bad_utf8),
            std::end(bad_utf8) - 1,
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        text::transcode_to_utf32(
            std::begin(bad_utf8),
            text::null_sentinel,
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        std::list<char8_t> bad_utf8_(std::begin(bad_utf8), std::end(bad_utf8) - 1);
        text::transcode_to_utf32(
            bad_utf8_.begin(), bad_utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char32_t>(
                {0x0061,
                 0xfffd,
                 0xfffd,
                 0xfffd,
                 0x0062,
                 0xfffd,
                 0x0063,
                 0xfffd,
                 0xfffd,
                 0x0064}));
    }
}

// The tests using this struct cover the boundaries of all the cases in Table
// 3-7 in Unicode 9, 3.9/D92
template<int Size>
struct utf8_coverage_test_case
{
    char8_t str_[Size];
    char32_t utf32_;
};

TEST(transcode_algorthm, from_utf8_errors_1)
{
    utf8_coverage_test_case<1> const cases[] = {
        {{0x0}, 0x0},
        {{0x7f}, 0x7f},
        {{0x80}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_2)
{
    utf8_coverage_test_case<2> const cases[] = {
        {{0xc1, 0xbf}, 0xfffd},
        {{0xc1, 0xc0}, 0xfffd},

        {{0xc2, 0xbf}, 0xbf},
        {{0xc2, 0xc0}, 0xfffd},

        {{0xdf, 0xbf}, 0x7ff},
        {{0xdf, 0xc0}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_3)
{
    utf8_coverage_test_case<3> const cases[] = {
        {{0xe0, 0x9f, 0xc0}, 0xfffd},
        {{0xe0, 0x9f, 0xbf}, 0xfffd},
        {{0xe0, 0xa0, 0xc0}, 0xfffd},
        {{0xe0, 0xa0, 0xbf}, 0x83f},

        {{0xe1, 0xc0, 0xc0}, 0xfffd},
        {{0xe1, 0xc0, 0xbf}, 0xfffd},
        {{0xe1, 0xbf, 0xc0}, 0xfffd},
        {{0xe1, 0xbf, 0xbf}, 0x1fff},

        {{0xec, 0xc0, 0xc0}, 0xfffd},
        {{0xec, 0xc0, 0xbf}, 0xfffd},
        {{0xec, 0xbf, 0xc0}, 0xfffd},
        {{0xec, 0xbf, 0xbf}, 0xcfff},

        {{0xed, 0xa0, 0xc0}, 0xfffd},
        {{0xed, 0xa0, 0xbf}, 0xfffd},
        {{0xed, 0x9f, 0xc0}, 0xfffd},
        {{0xed, 0x9f, 0xbf}, 0xd7ff},

        {{0xee, 0xc0, 0xc0}, 0xfffd},
        {{0xee, 0xc0, 0xbf}, 0xfffd},
        {{0xee, 0xbf, 0xc0}, 0xfffd},
        {{0xee, 0xbf, 0xbf}, 0xefff},

        {{0xef, 0xc0, 0xc0}, 0xfffd},
        {{0xef, 0xc0, 0xbf}, 0xfffd},
        {{0xef, 0xbf, 0xc0}, 0xfffd},
        {{0xef, 0xbf, 0xbf}, 0xffff},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_4)
{
    utf8_coverage_test_case<4> const cases[] = {
        {{0xf0, 0x8f, 0xc0, 0xc0}, 0xfffd},
        {{0xf0, 0x8f, 0xc0, 0xbf}, 0xfffd},
        {{0xf0, 0x8f, 0xbf, 0xc0}, 0xfffd},
        {{0xf0, 0x8f, 0xbf, 0xbf}, 0xfffd},

        {{0xf0, 0x90, 0xc0, 0xc0}, 0xfffd},
        {{0xf0, 0x90, 0xc0, 0xbf}, 0xfffd},
        {{0xf0, 0x90, 0xbf, 0xc0}, 0xfffd},
        {{0xf0, 0x90, 0xbf, 0xbf}, 0x10fff},

        {{0xf1, 0x7f, 0x7f, 0x7f}, 0xfffd},
        {{0xf1, 0x7f, 0x7f, 0x80}, 0xfffd},
        {{0xf1, 0x7f, 0x80, 0x7f}, 0xfffd},
        {{0xf1, 0x7f, 0x80, 0x80}, 0xfffd},

        {{0xf1, 0x80, 0x7f, 0x7f}, 0xfffd},
        {{0xf1, 0x80, 0x7f, 0x80}, 0xfffd},
        {{0xf1, 0x80, 0x80, 0x7f}, 0xfffd},
        {{0xf1, 0x80, 0x80, 0x80}, 0x40000},

        {{0xf3, 0x7f, 0x7f, 0x7f}, 0xfffd},
        {{0xf3, 0x7f, 0x7f, 0x80}, 0xfffd},
        {{0xf3, 0x7f, 0x80, 0x7f}, 0xfffd},
        {{0xf3, 0x7f, 0x80, 0x80}, 0xfffd},

        {{0xf3, 0x80, 0x7f, 0x7f}, 0xfffd},
        {{0xf3, 0x80, 0x7f, 0x80}, 0xfffd},
        {{0xf3, 0x80, 0x80, 0x7f}, 0xfffd},
        {{0xf3, 0x80, 0x80, 0x80}, 0xc0000},

        {{0xf4, 0x90, 0x7f, 0x7f}, 0xfffd},
        {{0xf4, 0x90, 0x7f, 0x80}, 0xfffd},
        {{0xf4, 0x90, 0x80, 0x7f}, 0xfffd},
        {{0xf4, 0x90, 0x80, 0x80}, 0xfffd},

        {{0xf4, 0x8f, 0x7f, 0x7f}, 0xfffd},
        {{0xf4, 0x8f, 0x7f, 0x80}, 0xfffd},
        {{0xf4, 0x8f, 0x80, 0x7f}, 0xfffd},
        {{0xf4, 0x8f, 0x80, 0x80}, 0x10f000},

        {{0xf5, 0x90, 0x7f, 0x7f}, 0xfffd},
        {{0xf5, 0x90, 0x7f, 0x80}, 0xfffd},
        {{0xf5, 0x90, 0x80, 0x7f}, 0xfffd},
        {{0xf5, 0x90, 0x80, 0x80}, 0xfffd},

        {{0xf5, 0x8f, 0x7f, 0x7f}, 0xfffd},
        {{0xf5, 0x8f, 0x7f, 0x80}, 0xfffd},
        {{0xf5, 0x8f, 0x80, 0x7f}, 0xfffd},
        {{0xf5, 0x8f, 0x80, 0x80}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char8_t> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf16_non_error)
{
    // UTF-16 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf32(
            std::begin(utf16), std::end(utf16), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        std::list<char16_t> utf16_(std::begin(utf16), std::end(utf16));
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf32(utf16_.begin(), utf16_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        text::transcode_to_utf32(
            std::begin(utf16), std::end(utf16), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result;
        std::list<char16_t> utf16_(std::begin(utf16), std::end(utf16));
        text::transcode_to_utf32(
            utf16_.begin(), utf16_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<char32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }

    // UTF-16 pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf8(
            std::begin(utf16), std::end(utf16), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-16 non-pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        std::list<char16_t> utf16_(std::begin(utf16), std::end(utf16));
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf8(utf16_.begin(), utf16_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-16 pointer -> UTF-8 non-pointer
    {
        std::vector<char> result;
        text::transcode_to_utf8(
            std::begin(utf16), std::end(utf16), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-16 non-pointer -> UTF-8 non-pointer
    {
        std::vector<char> result;
        std::list<char16_t> utf16_(std::begin(utf16), std::end(utf16));
        text::transcode_to_utf8(
            utf16_.begin(), utf16_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
}

// The tests using this struct cover the boundaries of all the cases in Table
// 3-7 in Unicode 9, 3.9/D91
template<int Size>
struct utf16_coverage_test_case
{
    char16_t str_[Size];
    char32_t utf32_;
};

TEST(transcode_algorthm, from_utf16_errors_1)
{
    utf16_coverage_test_case<1> const cases[] = {
        {{0xd7ffu}, 0xd7ffu},
        {{0xd800u}, 0xfffdu},
        {{0xdbffu}, 0xfffdu},
        {{0xdc00u}, 0xfffdu},
        {{0xdfffu}, 0xfffdu},
        {{0xe000u}, 0xe000u},
        {{0xffffu}, 0xffffu},
    };

    // UTF-16 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf16_.begin(), utf16_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf16_.begin(), utf16_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf16_errors_2)
{
    utf16_coverage_test_case<2> const cases[] = {
        {{0xd800u, 0xdbffu}, 0xfffdu},
        {{0xd800u, 0xdc00u}, 0x10000u},
        {{0xd800u, 0xdfffu}, 0x103ff},
        {{0xd800u, 0xe000u}, 0xfffdu},
        {{0xdbffu, 0xdbffu}, 0xfffdu},
        {{0xdbffu, 0xdc00u}, 0x10fc00},
        {{0xdbffu, 0xdfffu}, 0x10ffffu},
        {{0xdbffu, 0xe000u}, 0xfffdu},
    };

    // UTF-16 pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_to_utf32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(utf16_.begin(), utf16_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_to_utf32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<char32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_to_utf32(
                utf16_.begin(), utf16_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf32)
{
    // UTF-32 pointer -> UTF-16 pointer
    {
        std::vector<char16_t> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf16(
            std::begin(utf32), std::end(utf32), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 non-pointer -> UTF-16 pointer
    {
        std::vector<char16_t> result(10);
        std::list<char32_t> utf32_(std::begin(utf32), std::end(utf32));
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf16(utf32_.begin(), utf32_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 pointer -> UTF-16 non-pointer
    {
        std::vector<char16_t> result;
        text::transcode_to_utf16(
            std::begin(utf32), std::end(utf32), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 non-pointer -> UTF-16 non-pointer
    {
        std::vector<char16_t> result;
        std::list<char32_t> utf32_(std::begin(utf32), std::end(utf32));
        text::transcode_to_utf16(
            utf32_.begin(), utf32_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }

    // UTF-32 pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        auto const out_first = &result[0];
        auto const in_out = text::transcode_to_utf8(
            std::begin(utf32), std::end(utf32), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-32 non-pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        std::list<char32_t> utf32_(std::begin(utf32), std::end(utf32));
        auto const out_first = &result[0];
        auto const in_out =
            text::transcode_to_utf8(utf32_.begin(), utf32_.end(), &result[0]);
        result.resize(in_out.out - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-32 pointer -> UTF-8 non-pointer
    {
        std::vector<char> result;
        text::transcode_to_utf8(
            std::begin(utf32), std::end(utf32), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
    // UTF-32 non-pointer -> UTF-8 non-pointer
    {
        std::vector<char> result;
        std::list<char32_t> utf32_(std::begin(utf32), std::end(utf32));
        text::transcode_to_utf8(
            utf32_.begin(), utf32_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>(
                {0x4d,
                 char(0xd0),
                 char(0xb0),
                 char(0xe4),
                 char(0xba),
                 char(0x8c),
                 char(0xf0),
                 char(0x90),
                 char(0x8c),
                 char(0x82)}));
    }
}
