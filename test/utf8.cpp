// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_view.hpp>
#include <boost/text/transcode_view.hpp>

#include <gtest/gtest.h>


using namespace boost;


TEST(utf_8, test_consecutive)
{
    // Unicode 9, 3.9/D90-D92
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82, 0};

    // UTF-8 -> UTF-32
    {
        auto it =
            text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>(
                utf8, utf8, text::null_sentinel);

        auto const end =
            text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>(
                utf8, utf8 + 10, text::null_sentinel);

        auto const zero = it;
        EXPECT_EQ(*it, utf32[0]);
        ++it;
        auto const one = it;
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        auto const two = it;
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        auto const three = it;
        EXPECT_EQ(*it, utf32[3]);
        ++it;
        auto const four = it;

        EXPECT_EQ(it, end);

        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf32[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf32[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf32[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf32[0]);

        it = end;

        EXPECT_EQ(it, four);

        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf32[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf32[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf32[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf32[0]);

        EXPECT_EQ(*it, utf32[0]);
        ++it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf32[3]);
        ++it;
        EXPECT_EQ(it, four);
    }

    // UTF-32 -> UTF-8
    {
        auto it = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);

        auto const end = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        auto const zero = it;
        EXPECT_EQ(*it, utf8[0]);
        it++;
        auto const one = it;
        EXPECT_EQ(*it, utf8[1]);
        it++;
        auto const two = it;
        EXPECT_EQ(*it, utf8[2]);
        it++;
        auto const three = it;
        EXPECT_EQ(*it, utf8[3]);
        it++;
        auto const four = it;
        EXPECT_EQ(*it, utf8[4]);
        it++;
        auto const five = it;
        EXPECT_EQ(*it, utf8[5]);
        it++;
        auto const six = it;
        EXPECT_EQ(*it, utf8[6]);
        it++;
        auto const seven = it;
        EXPECT_EQ(*it, utf8[7]);
        it++;
        auto const eight = it;
        EXPECT_EQ(*it, utf8[8]);
        it++;
        auto const nine = it;
        EXPECT_EQ(*it, utf8[9]);
        it++;
        auto const ten = it;

        EXPECT_EQ(ten, end);

        --it;
        EXPECT_EQ(it, nine);
        EXPECT_EQ(*it, utf8[9]);
        --it;
        EXPECT_EQ(it, eight);
        EXPECT_EQ(*it, utf8[8]);
        --it;
        EXPECT_EQ(it, seven);
        EXPECT_EQ(*it, utf8[7]);
        --it;
        EXPECT_EQ(it, six);
        EXPECT_EQ(*it, utf8[6]);
        --it;
        EXPECT_EQ(it, five);
        EXPECT_EQ(*it, utf8[5]);
        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf8[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf8[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf8[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf8[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf8[0]);

        it = end;

        EXPECT_EQ(it, end);

        --it;
        EXPECT_EQ(it, nine);
        EXPECT_EQ(*it, utf8[9]);
        --it;
        EXPECT_EQ(it, eight);
        EXPECT_EQ(*it, utf8[8]);
        --it;
        EXPECT_EQ(it, seven);
        EXPECT_EQ(*it, utf8[7]);
        --it;
        EXPECT_EQ(it, six);
        EXPECT_EQ(*it, utf8[6]);
        --it;
        EXPECT_EQ(it, five);
        EXPECT_EQ(*it, utf8[5]);
        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf8[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf8[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf8[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf8[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf8[0]);

        EXPECT_EQ(*it, utf8[0]);
        ++it;
        EXPECT_EQ(*it, utf8[1]);
        ++it;
        EXPECT_EQ(*it, utf8[2]);
        ++it;
        EXPECT_EQ(*it, utf8[3]);
        ++it;
        EXPECT_EQ(*it, utf8[4]);
        ++it;
        EXPECT_EQ(*it, utf8[5]);
        ++it;
        EXPECT_EQ(*it, utf8[6]);
        ++it;
        EXPECT_EQ(*it, utf8[7]);
        ++it;
        EXPECT_EQ(*it, utf8[8]);
        ++it;
        EXPECT_EQ(*it, utf8[9]);
        ++it;
    }

    // UTF-8 -> UTF-16
    {
        auto it = text::utf_8_to_16_iterator<char8_t const *>(
            utf8, utf8, utf8 + sizeof(utf8));

        auto const end = text::utf_8_to_16_iterator<char8_t const *>(
            utf8, utf8 + 10, utf8 + sizeof(utf8));

        auto const zero = it;
        EXPECT_EQ(*it, utf16[0]);
        ++it;
        auto const one = it;
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        auto const two = it;
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        auto const three = it;
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        auto const four = it;
        EXPECT_EQ(*it, utf16[4]);
        ++it;
        auto const five = it;

        EXPECT_EQ(five, end);

        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf16[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf16[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf16[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf16[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf16[0]);

        it = end;

        EXPECT_EQ(it, five);

        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf16[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf16[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf16[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf16[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf16[0]);

        EXPECT_EQ(*it, utf16[0]);
        ++it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf16[4]);
        ++it;
        EXPECT_EQ(it, five);
    }

    // UTF-16 -> UTF-8
    {
        auto it = text::utf_16_to_8_iterator<uint16_t const *>(
            utf16, utf16, utf16 + 5);

        auto const end = text::utf_16_to_8_iterator<uint16_t const *>(
            utf16, utf16 + 5, utf16 + 5);

        auto const zero = it;
        EXPECT_EQ(*it, utf8[0]);
        ++it;
        auto const one = it;
        EXPECT_EQ(*it, utf8[1]);
        ++it;
        auto const two = it;
        EXPECT_EQ(*it, utf8[2]);
        ++it;
        auto const three = it;
        EXPECT_EQ(*it, utf8[3]);
        ++it;
        auto const four = it;
        EXPECT_EQ(*it, utf8[4]);
        ++it;
        auto const five = it;
        EXPECT_EQ(*it, utf8[5]);
        ++it;
        auto const six = it;
        EXPECT_EQ(*it, utf8[6]);
        ++it;
        auto const seven = it;
        EXPECT_EQ(*it, utf8[7]);
        ++it;
        auto const eight = it;
        EXPECT_EQ(*it, utf8[8]);
        ++it;
        auto const nine = it;
        EXPECT_EQ(*it, utf8[9]);
        ++it;
        auto const ten = it;

        EXPECT_EQ(ten, end);

        --it;
        EXPECT_EQ(it, nine);
        EXPECT_EQ(*it, utf8[9]);
        --it;
        EXPECT_EQ(it, eight);
        EXPECT_EQ(*it, utf8[8]);
        --it;
        EXPECT_EQ(it, seven);
        EXPECT_EQ(*it, utf8[7]);
        --it;
        EXPECT_EQ(it, six);
        EXPECT_EQ(*it, utf8[6]);
        --it;
        EXPECT_EQ(it, five);
        EXPECT_EQ(*it, utf8[5]);
        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf8[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf8[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf8[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf8[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf8[0]);

        it = end;

        EXPECT_EQ(it, end);

        --it;
        EXPECT_EQ(it, nine);
        EXPECT_EQ(*it, utf8[9]);
        --it;
        EXPECT_EQ(it, eight);
        EXPECT_EQ(*it, utf8[8]);
        --it;
        EXPECT_EQ(it, seven);
        EXPECT_EQ(*it, utf8[7]);
        --it;
        EXPECT_EQ(it, six);
        EXPECT_EQ(*it, utf8[6]);
        --it;
        EXPECT_EQ(it, five);
        EXPECT_EQ(*it, utf8[5]);
        --it;
        EXPECT_EQ(it, four);
        EXPECT_EQ(*it, utf8[4]);
        --it;
        EXPECT_EQ(it, three);
        EXPECT_EQ(*it, utf8[3]);
        --it;
        EXPECT_EQ(it, two);
        EXPECT_EQ(*it, utf8[2]);
        --it;
        EXPECT_EQ(it, one);
        EXPECT_EQ(*it, utf8[1]);
        --it;
        EXPECT_EQ(it, zero);
        EXPECT_EQ(*it, utf8[0]);

        EXPECT_EQ(*it, utf8[0]);
        ++it;
        EXPECT_EQ(*it, utf8[1]);
        ++it;
        EXPECT_EQ(*it, utf8[2]);
        ++it;
        EXPECT_EQ(*it, utf8[3]);
        ++it;
        EXPECT_EQ(*it, utf8[4]);
        ++it;
        EXPECT_EQ(*it, utf8[5]);
        ++it;
        EXPECT_EQ(*it, utf8[6]);
        ++it;
        EXPECT_EQ(*it, utf8[7]);
        ++it;
        EXPECT_EQ(*it, utf8[8]);
        ++it;
        EXPECT_EQ(*it, utf8[9]);
        ++it;
    }
}

TEST(utf_8, test_back_and_forth)
{
    // Unicode 9, 3.9/D90-D92
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82, 0};

    // UTF-8 -> UTF-32
    for (int iterations = 1; iterations <= 4; ++iterations) {
        auto it = text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>(
            utf8, utf8, text::null_sentinel);
        for (int i = 0; i < iterations; ++i) {
            EXPECT_EQ(*it++, utf32[i])
                << "iterations=" << iterations << " i=" << i;
        }
        for (int i = iterations; i-- > 0;) {
            EXPECT_EQ(*--it, utf32[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    for (int iterations = 0; iterations < 4; ++iterations) {
        auto it = text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>(
            utf8, utf8 + 10, text::null_sentinel);
        int i = 4;
        for (; i-- > iterations;) {
            EXPECT_EQ(*--it, utf32[i])
                << "iterations=" << iterations << " i=" << i;
        }
        ++i;
        for (; i < 4; ++i) {
            EXPECT_EQ(*it++, utf32[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    // UTF-32 -> UTF-8
    for (int iterations = 1; iterations <= 10; ++iterations) {
        auto it = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        for (int i = 0; i < iterations; ++i) {
            EXPECT_EQ(*it++, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
        for (int i = iterations; i-- > 0;) {
            EXPECT_EQ(*--it, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    for (int iterations = 0; iterations < 10; ++iterations) {
        auto it = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);
        int i = 10;
        for (; i-- > iterations;) {
            EXPECT_EQ(*--it, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
        ++i;
        for (; i < 10; ++i) {
            EXPECT_EQ(*it++, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    // UTF-8 -> UTF-16
    for (int iterations = 1; iterations <= 5; ++iterations) {
        auto it = text::utf_8_to_16_iterator<char8_t const *>(
            utf8, utf8, utf8 + sizeof(utf8));
        for (int i = 0; i < iterations; ++i) {
            EXPECT_EQ(*it++, utf16[i])
                << "iterations=" << iterations << " i=" << i;
        }
        for (int i = iterations; i-- > 0;) {
            EXPECT_EQ(*--it, utf16[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    for (int iterations = 0; iterations < 5; ++iterations) {
        auto it = text::utf_8_to_16_iterator<char8_t const *>(
            utf8, utf8 + 10, utf8 + sizeof(utf8));
        int i = 5;
        for (; i-- > iterations;) {
            EXPECT_EQ(*--it, utf16[i])
                << "iterations=" << iterations << " i=" << i;
        }
        ++i;
        for (; i < 5; ++i) {
            EXPECT_EQ(*it++, utf16[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    // UTF-16 -> UTF-8
    for (int iterations = 1; iterations <= 10; ++iterations) {
        auto it = text::utf_16_to_8_iterator<uint16_t const *>(
            utf16, utf16, utf16 + 5);
        for (int i = 0; i < iterations; ++i) {
            EXPECT_EQ(*it++, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
        for (int i = iterations; i-- > 0;) {
            EXPECT_EQ(*--it, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }

    for (int iterations = 0; iterations < 10; ++iterations) {
        auto it = text::utf_16_to_8_iterator<uint16_t const *>(
            utf16, utf16 + 5, utf16 + 5);
        int i = 10;
        for (; i-- > iterations;) {
            EXPECT_EQ(*--it, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
        ++i;
        for (; i < 10; ++i) {
            EXPECT_EQ(*it++, utf8[i])
                << "iterations=" << iterations << " i=" << i;
        }
    }
}

// The tests using this struct cover the boundaries of all the cases in Table
// 3-7 in Unicode 9, 3.9/D92
template<int EncodingLength>
struct coverage_test_case
{
    char str_[EncodingLength + 4];
    uint32_t utf32_;
};

TEST(utf_8, test_utf8_coverage_length_1)
{
    coverage_test_case<1> const cases[] = {
        {{char(0x0)}, 0x0},
        {{char(0x7f)}, 0x7f},
        {{char(0x80)}, 0xfffd},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_2)
{
    coverage_test_case<2> const cases[] = {
        {{char(0xc1), char(0xbf)}, 0xfffd},
        {{char(0xc1), char(0xc0)}, 0xfffd},

        {{char(0xc2), char(0xbf)}, 0xbf},
        {{char(0xc2), char(0xc0)}, 0xfffd},

        {{char(0xdf), char(0xbf)}, 0x7ff},
        {{char(0xdf), char(0xc0)}, 0xfffd},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_a)
{
    coverage_test_case<3> const cases[] = {
        {{char(0xe0), char(0x9f), char(0xc0)}, 0xfffd},
        {{char(0xe0), char(0x9f), char(0xbf)}, 0xfffd},
        {{char(0xe0), char(0xa0), char(0xc0)}, 0xfffd},
        {{char(0xe0), char(0xa0), char(0xbf)}, 0x83f},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_b)
{
    coverage_test_case<3> const cases[] = {
        {{char(0xe1), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xe1), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xe1), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xe1), char(0xbf), char(0xbf)}, 0x1fff},

        {{char(0xec), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xec), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xec), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xec), char(0xbf), char(0xbf)}, 0xcfff},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_c)
{
    coverage_test_case<3> const cases[] = {
        {{char(0xed), char(0xa0), char(0xc0)}, 0xfffd},
        {{char(0xed), char(0xa0), char(0xbf)}, 0xfffd},
        {{char(0xed), char(0x9f), char(0xc0)}, 0xfffd},
        {{char(0xed), char(0x9f), char(0xbf)}, 0xd7ff},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_d)
{
    coverage_test_case<3> const cases[] = {
        {{char(0xee), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xee), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xee), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xee), char(0xbf), char(0xbf)}, 0xefff},

        {{char(0xef), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xef), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xef), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xef), char(0xbf), char(0xbf)}, 0xffff},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_4_a)
{
    coverage_test_case<3> const cases[] = {
        {{char(0xf0), char(0x8f), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x8f), char(0xbf), char(0xbf)}, 0xfffd},

        {{char(0xf0), char(0x90), char(0xc0), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xc0), char(0xbf)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xbf), char(0xc0)}, 0xfffd},
        {{char(0xf0), char(0x90), char(0xbf), char(0xbf)}, 0x10fff},
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

// Note that the only reason we're testing the lower end of the 0x80-0xbf
// range in these next two tests is that using the upper end produces a
// code point reserved for internal use (and thus a 0xfffd).

TEST(utf_8, test_utf8_coverage_length_4_b)
{
    coverage_test_case<3> const cases[] = {
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
    };

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_4_c)
{
    coverage_test_case<3> const cases[] = {
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

    for (auto c : cases) {
        text::utf_8_to_32_iterator<char const *, char const *> it(
            c.str_, c.str_, c.str_ + sizeof(c.str_));
        EXPECT_EQ(*it, c.utf32_);
    }
}


// Unicode 9, 3.9/D93b
// Table 3-8. Use of U+FFFD in UTF-8 Conversion
TEST(utf_8, test_0xfffd)
{
    char const bad_utf8[] = {0x61,
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
                             0};
    uint32_t const expected[] = {0x0061,
                                 0xfffd,
                                 0xfffd,
                                 0xfffd,
                                 0x0062,
                                 0xfffd,
                                 0x0063,
                                 0xfffd,
                                 0xfffd,
                                 0x0064};

    auto it = text::utf_8_to_32_iterator<char const *, text::null_sentinel_t>(
        bad_utf8, bad_utf8, text::null_sentinel);

    EXPECT_EQ(*it++, expected[0]);
    EXPECT_EQ(*it++, expected[1]);
    EXPECT_EQ(*it++, expected[2]);
    EXPECT_EQ(*it++, expected[3]);
    EXPECT_EQ(*it++, expected[4]);
    EXPECT_EQ(*it++, expected[5]);
    EXPECT_EQ(*it++, expected[6]);
    EXPECT_EQ(*it++, expected[7]);
    EXPECT_EQ(*it++, expected[8]);
    EXPECT_EQ(*it++, expected[9]);

    it = text::utf_8_to_32_iterator<char const *, text::null_sentinel_t>(
        bad_utf8, bad_utf8 + 13, text::null_sentinel);

    EXPECT_EQ(*--it, expected[9]);
    EXPECT_EQ(*--it, expected[8]);
    EXPECT_EQ(*--it, expected[7]);
    EXPECT_EQ(*--it, expected[6]);
    EXPECT_EQ(*--it, expected[5]);
    EXPECT_EQ(*--it, expected[4]);
    EXPECT_EQ(*--it, expected[3]);
    EXPECT_EQ(*--it, expected[2]);
    EXPECT_EQ(*--it, expected[1]);
    EXPECT_EQ(*--it, expected[0]);
}

TEST(utf_8, test_end_of_invalid_utf8)
{
    auto const none = text::detail::optional_iter<char const *>{};

    {
        char const bad_utf8[] = {0x61,
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
                                 0x64};
        // uint32_t const expected[] = {0x0061, 0xfffd, 0xfffd, 0xfffd, 0x0062,
        // 0xfffd, 0x0063, 0xfffd, 0xfffd, 0x0064};

        EXPECT_EQ(text::detail::end_of_invalid_utf8(bad_utf8 + 0), none);
        EXPECT_EQ(
            *text::detail::end_of_invalid_utf8(bad_utf8 + 1), bad_utf8 + 4);
        EXPECT_EQ(
            *text::detail::end_of_invalid_utf8(bad_utf8 + 4), bad_utf8 + 6);
        EXPECT_EQ(
            *text::detail::end_of_invalid_utf8(bad_utf8 + 6), bad_utf8 + 7);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(bad_utf8 + 7), none);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(bad_utf8 + 9), none);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(bad_utf8 + 12), none);
    }
    {
        // Unicode 9, 3.9/D90-D92
        // uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
        char const utf8[] = {0x4d,
                             char(0xd0),
                             char(0xb0),
                             char(0xe4),
                             char(0xba),
                             char(0x8c),
                             char(0xf0),
                             char(0x90),
                             char(0x8c),
                             char(0x82)};

        EXPECT_EQ(text::detail::end_of_invalid_utf8(utf8 + 0), none);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(utf8 + 1), none);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(utf8 + 3), none);
        EXPECT_EQ(text::detail::end_of_invalid_utf8(utf8 + 6), none);
    }
}

TEST(utf_8, iterator_conversions)
{
    uint32_t utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char utf8[] = {0x4d,
                   char(0xd0),
                   char(0xb0),
                   char(0xe4),
                   char(0xba),
                   char(0x8c),
                   char(0xf0),
                   char(0x90),
                   char(0x8c),
                   char(0x82),
                   0};

    {
        text::utf_8_to_32_iterator<char *, text::null_sentinel_t> const it =
            text::utf_8_to_32_iterator<char *, text::null_sentinel_t>(
                utf8, utf8, text::null_sentinel);

        text::utf_8_to_32_iterator<char const *, text::null_sentinel_t> const
            it_const = it;

        EXPECT_EQ(it_const, it);
    }

    {
        text::utf_32_to_8_iterator<uint32_t *> const it =
            text::utf_32_to_8_iterator<uint32_t *>(utf32, utf32, utf32 + 4);

        text::utf_32_to_8_iterator<uint32_t const *> const it_const = it;

        EXPECT_EQ(it_const, it);
    }

    {
        text::utf_8_to_16_iterator<char *> it =
            text::utf_8_to_16_iterator<char *>(utf8, utf8, utf8 + sizeof(utf8));

        text::utf_8_to_16_iterator<char const *> it_const = it;

        EXPECT_EQ(it_const, it);
    }
    {
        text::utf_16_to_8_iterator<uint16_t *> const it =
            text::utf_16_to_8_iterator<uint16_t *>(utf16, utf16, utf16 + 5);

        text::utf_16_to_8_iterator<uint16_t const *> const it_const = it;

        EXPECT_EQ(it_const, it);
    }
}

TEST(utf_8, make_utfN_iterator)
{
    // Unicode 9, 3.9/D90-D92
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char const utf8[] = {
        0x4d,
        char(0xd0),
        char(0xb0),
        char(0xe4),
        char(0xba),
        char(0x8c),
        char(0xf0),
        char(0x90),
        char(0x8c),
        char(0x82)};

    // -> UTF-8
    {
        std::string result;
        std::string const expected{std::begin(utf8), std::end(utf8)};
        std::copy(
            text::utf8_iterator(
                std::begin(utf32), std::begin(utf32), std::end(utf32)),
            text::utf8_iterator(
                std::begin(utf32), std::end(utf32), std::end(utf32)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }

    {
        std::string result;
        std::string const expected{std::begin(utf8), std::end(utf8)};
        std::copy(
            text::utf8_iterator(
                std::begin(utf16), std::begin(utf16), std::end(utf16)),
            text::utf8_iterator(
                std::begin(utf16), std::end(utf16), std::end(utf16)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }

    // -> UTF-16
    {
        std::vector<uint16_t> result;
        std::vector<uint16_t> const expected{
            std::begin(utf16), std::end(utf16)};
        std::copy(
            text::utf16_iterator(
                std::begin(utf8), std::begin(utf8), std::end(utf8)),
            text::utf16_iterator(
                std::begin(utf8), std::end(utf8), std::end(utf8)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }

    {
        std::vector<uint16_t> result;
        std::vector<uint16_t> const expected{
            std::begin(utf16), std::end(utf16)};
        std::copy(
            text::utf16_iterator(
                std::begin(utf32), std::begin(utf32), std::end(utf32)),
            text::utf16_iterator(
                std::begin(utf32), std::end(utf32), std::end(utf32)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }

    // -> UTF-32
    {
        std::vector<uint32_t> result;
        std::vector<uint32_t> const expected{
            std::begin(utf32), std::end(utf32)};
        std::copy(
            text::utf32_iterator(
                std::begin(utf8), std::begin(utf8), std::end(utf8)),
            text::utf32_iterator(
                std::begin(utf8), std::end(utf8), std::end(utf8)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }

    {
        std::vector<uint32_t> result;
        std::vector<uint32_t> const expected{
            std::begin(utf32), std::end(utf32)};
        std::copy(
            text::utf32_iterator(
                std::begin(utf16), std::begin(utf16), std::end(utf16)),
            text::utf32_iterator(
                std::begin(utf16), std::end(utf16), std::end(utf16)),
            std::back_inserter(result));
        EXPECT_EQ(result, expected);
    }
}

TEST(utf8, utf_16_to_8_incomplete_surrogate_pair)
{
    uint16_t const utf16[] = {0xd800};
    char8_t const utf8[] = {0xef, 0xbf, 0xbd, 0};

    auto it = text::utf_16_to_8_iterator<uint16_t const *>(
        std::begin(utf16), std::begin(utf16), std::end(utf16));

    auto const end = text::utf_16_to_8_iterator<uint16_t const *>(
        std::begin(utf16), std::end(utf16), std::end(utf16));

    EXPECT_EQ(*it++, utf8[0]);
    EXPECT_EQ(*it++, utf8[1]);
    EXPECT_EQ(*it++, utf8[2]);
    EXPECT_EQ(it, end);
}
