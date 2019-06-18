#define BOOST_TEXT_USE_SIMD 0
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/utf16.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

// Unicode 3.9/D90-D92
constexpr uint32_t utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
constexpr uint16_t utf16[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
constexpr char utf8[11] = {
    0x4d,
    char(0xd0),
    char(0xb0),
    char(0xe4),
    char(0xba),
    char(0x8c),
    char(0xf0),
    char(0x90),
    char(0x8c),
    char(0x82),
    0,
};

// TODO: Error cases that test error handling when processing utf-8 and utf-16
// inputs.

TEST(transcode_algorthm, from_utf8_non_error)
{
    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_32(
            std::begin(utf8), std::end(utf8) - 1, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_32(
            std::begin(utf8), text::utf8::null_sentinel{}, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        std::list<char> utf8_(std::begin(utf8), std::end(utf8) - 1);
        auto const out_first = &result[0];
        auto const out_last =
            text::transcode_utf_8_to_32(utf8_.begin(), utf8_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        text::transcode_utf_8_to_32(
            std::begin(utf8), std::end(utf8) - 1, std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 pointer and sentinel -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        text::transcode_utf_8_to_32(
            std::begin(utf8),
            text::utf8::null_sentinel{},
            std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        std::list<char> utf8_(std::begin(utf8), std::end(utf8) - 1);
        text::transcode_utf_8_to_32(
            utf8_.begin(), utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }

    // UTF-8 pointer -> UTF-16 pointer
    {
        std::vector<uint16_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_16(
            std::begin(utf8), std::end(utf8) - 1, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer and sentinel -> UTF-16 pointer
    {
        std::vector<uint16_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_16(
            std::begin(utf8), text::utf8::null_sentinel{}, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 non-pointer -> UTF-16 pointer
    {
        std::vector<uint16_t> result(10);
        std::list<char> utf8_(std::begin(utf8), std::end(utf8) - 1);
        auto const out_first = &result[0];
        auto const out_last =
            text::transcode_utf_8_to_16(utf8_.begin(), utf8_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer -> UTF-16 non-pointer
    {
        std::vector<uint16_t> result;
        text::transcode_utf_8_to_16(
            std::begin(utf8), std::end(utf8) - 1, std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 pointer and sentinel -> UTF-16 non-pointer
    {
        std::vector<uint16_t> result;
        text::transcode_utf_8_to_16(
            std::begin(utf8),
            text::utf8::null_sentinel{},
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-8 non-pointer -> UTF-16 non-pointer
    {
        std::vector<uint16_t> result;
        std::list<char> utf8_(std::begin(utf8), std::end(utf8) - 1);
        text::transcode_utf_8_to_16(
            utf8_.begin(), utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
}

TEST(transcode_algorthm, from_utf8_errors_0)
{
    char const bad_utf8[14] = {
        0x61,
        char(0xf1),
        char(0x80),
        char(0x80),
        char(0xe1),
        char(0x80),
        char(0xc2),
        0x62,
        char(0x80),
        0x63,
        char(0x80),
        char(0xbf),
        0x64,
        0,
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_32(
            std::begin(bad_utf8), std::end(bad_utf8) - 1, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
        std::vector<uint32_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_32(
            std::begin(bad_utf8), text::utf8::null_sentinel{}, &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
        std::vector<uint32_t> result(10);
        std::list<char> bad_utf8_(std::begin(bad_utf8), std::end(bad_utf8) - 1);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_8_to_32(
            bad_utf8_.begin(), bad_utf8_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
        std::vector<uint32_t> result;
        text::transcode_utf_8_to_32(
            std::begin(bad_utf8),
            std::end(bad_utf8) - 1,
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
        std::vector<uint32_t> result;
        text::transcode_utf_8_to_32(
            std::begin(bad_utf8),
            text::utf8::null_sentinel{},
            std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
        std::vector<uint32_t> result;
        std::list<char> bad_utf8_(std::begin(bad_utf8), std::end(bad_utf8) - 1);
        text::transcode_utf_8_to_32(
            bad_utf8_.begin(), bad_utf8_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint32_t>({0x0061,
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
    char str_[Size];
    uint32_t utf32_;
};

TEST(transcode_algorthm, from_utf8_errors_1)
{
    utf8_coverage_test_case<1> const cases[] = {
        {{char(0x0)}, 0x0},
        {{char(0x7f)}, 0x7f},
        {{char(0x80)}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_8_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_8_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_2)
{
    utf8_coverage_test_case<2> const cases[] = {
        {{char(0xc1), char(0xbf)}, 0xfffd},
        {{char(0xc1), char(0xc0)}, 0xfffd},

        {{char(0xc2), char(0xbf)}, 0xbf},
        {{char(0xc2), char(0xc0)}, 0xfffd},

        {{char(0xdf), char(0xbf)}, 0x7ff},
        {{char(0xdf), char(0xc0)}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_8_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_8_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_3)
{
    utf8_coverage_test_case<3> const cases[] = {
        {{char(0xe0), char(0x9f), char(0xc0)}, 0xfffd},
        {{char(0xe0), char(0x9f), char(0xbf)}, 0xfffd},
        {{char(0xe0), char(0xa0), char(0xc0)}, 0xfffd},
        {{char(0xe0), char(0xa0), char(0xbf)}, 0x83f},

        {{char(0xe1), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xe1), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xe1), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xe1), char(0xbf), char(0xbf)}, 0x1fff},

        {{char(0xec), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xec), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xec), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xec), char(0xbf), char(0xbf)}, 0xcfff},

        {{char(0xed), char(0xa0), char(0xc0)}, 0xfffd},
        {{char(0xed), char(0xa0), char(0xbf)}, 0xfffd},
        {{char(0xed), char(0x9f), char(0xc0)}, 0xfffd},
        {{char(0xed), char(0x9f), char(0xbf)}, 0xd7ff},

        {{char(0xee), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xee), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xee), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xee), char(0xbf), char(0xbf)}, 0xefff},

        {{char(0xef), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xef), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xef), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xef), char(0xbf), char(0xbf)}, 0xffff},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_8_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_8_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(
                utf8_.begin(), utf8_.end(), std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
}

TEST(transcode_algorthm, from_utf8_errors_4)
{
    utf8_coverage_test_case<4> const cases[] = {
        {{char(0xf0), char(0x8f), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xbf), char(0xbf)}, 0xfffd},

        {{char(0xf0), char(0x90), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xbf), char(0xbf)}, 0x10fff},

        {{char(0xf1), char(0x7f), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf1), char(0x7f), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf1), char(0x7f), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf1), char(0x7f), char(0x80), char(0x80)}, 0xfffd},

        {{char(0xf1), char(0x80), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf1), char(0x80), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf1), char(0x80), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf1), char(0x80), char(0x80), char(0x80)}, 0x40000},

        {{char(0xf3), char(0x7f), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf3), char(0x7f), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf3), char(0x7f), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf3), char(0x7f), char(0x80), char(0x80)}, 0xfffd},

        {{char(0xf3), char(0x80), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf3), char(0x80), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf3), char(0x80), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf3), char(0x80), char(0x80), char(0x80)}, 0xc0000},

        {{char(0xf4), char(0x90), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf4), char(0x90), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf4), char(0x90), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf4), char(0x90), char(0x80), char(0x80)}, 0xfffd},

        {{char(0xf4), char(0x8f), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf4), char(0x8f), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf4), char(0x8f), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf4), char(0x8f), char(0x80), char(0x80)}, 0x10f000},

        {{char(0xf5), char(0x90), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf5), char(0x90), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf5), char(0x90), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf5), char(0x90), char(0x80), char(0x80)}, 0xfffd},

        {{char(0xf5), char(0x8f), char(0x7f), char(0x7f)}, 0xfffd},
        {{char(0xf5), char(0x8f), char(0x7f), char(0x80)}, 0xfffd},
        {{char(0xf5), char(0x8f), char(0x80), char(0x7f)}, 0xfffd},
        {{char(0xf5), char(0x8f), char(0x80), char(0x80)}, 0xfffd},
    };

    // UTF-8 pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_8_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(utf8_.begin(), utf8_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_8_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-8 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<char> utf8_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_8_to_32(
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
        std::vector<uint32_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_16_to_32(
            std::begin(utf16), std::end(utf16), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_16_to_32(
            utf16_.begin(), utf16_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        text::transcode_utf_16_to_32(
            std::begin(utf16), std::end(utf16), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result;
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        text::transcode_utf_16_to_32(
            utf16_.begin(), utf16_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result, std::vector<uint32_t>({0x004d, 0x0430, 0x4e8c, 0x10302}));
    }

    // UTF-16 pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_16_to_8(
            std::begin(utf16), std::end(utf16), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_16_to_8(
            utf16_.begin(), utf16_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        text::transcode_utf_16_to_8(
            std::begin(utf16), std::end(utf16), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        text::transcode_utf_16_to_8(
            utf16_.begin(), utf16_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
    uint16_t str_[Size];
    uint32_t utf32_;
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
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_16_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<uint16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_16_to_32(
                utf16_.begin(), utf16_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_16_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<uint16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_16_to_32(
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
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            text::transcode_utf_16_to_32(
                std::begin(c.str_), std::end(c.str_), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            std::list<uint16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_16_to_32(
                utf16_.begin(), utf16_.end(), &result[0]);
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            text::transcode_utf_16_to_32(
                std::begin(c.str_),
                std::end(c.str_),
                std::back_inserter(result));
            EXPECT_EQ(result[0], c.utf32_) << "i=" << i;
            ++i;
        }
    }
    // UTF-16 non-pointer -> UTF-32 non-pointer
    {
        std::vector<uint32_t> result(10);
        int i = 0;
        for (auto c : cases) {
            result.clear();
            std::list<uint16_t> utf16_(std::begin(c.str_), std::end(c.str_));
            text::transcode_utf_16_to_32(
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
        std::vector<uint16_t> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_32_to_16(
            std::begin(utf16), std::end(utf16), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 non-pointer -> UTF-16 pointer
    {
        std::vector<uint16_t> result(10);
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_32_to_16(
            utf16_.begin(), utf16_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 pointer -> UTF-16 non-pointer
    {
        std::vector<uint16_t> result;
        text::transcode_utf_32_to_16(
            std::begin(utf16), std::end(utf16), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }
    // UTF-32 non-pointer -> UTF-16 non-pointer
    {
        std::vector<uint16_t> result;
        std::list<uint16_t> utf16_(std::begin(utf16), std::end(utf16));
        text::transcode_utf_32_to_16(
            utf16_.begin(), utf16_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<uint16_t>({0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02}));
    }

    // UTF-32 pointer -> UTF-8 pointer
    {
        std::vector<char> result(10);
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_32_to_8(
            std::begin(utf32), std::end(utf32), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        std::list<uint32_t> utf32_(std::begin(utf32), std::end(utf32));
        auto const out_first = &result[0];
        auto const out_last = text::transcode_utf_32_to_8(
            utf32_.begin(), utf32_.end(), &result[0]);
        result.resize(out_last - out_first);
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        text::transcode_utf_32_to_8(
            std::begin(utf32), std::end(utf32), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
        std::list<uint32_t> utf32_(std::begin(utf32), std::end(utf32));
        text::transcode_utf_32_to_8(
            utf32_.begin(), utf32_.end(), std::back_inserter(result));
        EXPECT_EQ(
            result,
            std::vector<char>({0x4d,
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
