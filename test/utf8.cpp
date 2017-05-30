#include <boost/text/text_view.hpp>

#include <gtest/gtest.h>


using namespace boost;


TEST(utf_8, test_consecutive)
{
    // Unicode 9, 3.9/D90-D92
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char const utf8[] = {0x4d, char(0xd0), char(0xb0), char(0xe4), char(0xba), char(0x8c), char(0xf0), char(0x90), char(0x8c), char(0x82)};

    // UTF-8 -> UTF-32
    {
        auto it = text::utf8::to_utf32_iterator(utf8);

        EXPECT_EQ(*it++, utf32[0]);
        EXPECT_EQ(*it++, utf32[1]);
        EXPECT_EQ(*it++, utf32[2]);
        EXPECT_EQ(*it++, utf32[3]);

        EXPECT_EQ(*--it, utf32[3]);
        EXPECT_EQ(*--it, utf32[2]);
        EXPECT_EQ(*--it, utf32[1]);
        EXPECT_EQ(*--it, utf32[0]);

        it = text::utf8::to_utf32_iterator(utf8 + 10);

        EXPECT_EQ(*--it, utf32[3]);
        EXPECT_EQ(*--it, utf32[2]);
        EXPECT_EQ(*--it, utf32[1]);
        EXPECT_EQ(*--it, utf32[0]);

        EXPECT_EQ(*it++, utf32[0]);
        EXPECT_EQ(*it++, utf32[1]);
        EXPECT_EQ(*it++, utf32[2]);
        EXPECT_EQ(*it++, utf32[3]);
    }

    // UTF-32 -> UTF-8
    {
        auto it = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);

        EXPECT_EQ(*it++, utf8[0]);
        EXPECT_EQ(*it++, utf8[1]);
        EXPECT_EQ(*it++, utf8[2]);
        EXPECT_EQ(*it++, utf8[3]);
        EXPECT_EQ(*it++, utf8[4]);
        EXPECT_EQ(*it++, utf8[5]);
        EXPECT_EQ(*it++, utf8[6]);
        EXPECT_EQ(*it++, utf8[7]);
        EXPECT_EQ(*it++, utf8[8]);
        EXPECT_EQ(*it++, utf8[9]);

        EXPECT_EQ(*--it, utf8[9]);
        EXPECT_EQ(*--it, utf8[8]);
        EXPECT_EQ(*--it, utf8[7]);
        EXPECT_EQ(*--it, utf8[6]);
        EXPECT_EQ(*--it, utf8[5]);
        EXPECT_EQ(*--it, utf8[4]);
        EXPECT_EQ(*--it, utf8[3]);
        EXPECT_EQ(*--it, utf8[2]);
        EXPECT_EQ(*--it, utf8[1]);
        EXPECT_EQ(*--it, utf8[0]);

        it = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

        EXPECT_EQ(*--it, utf8[9]);
        EXPECT_EQ(*--it, utf8[8]);
        EXPECT_EQ(*--it, utf8[7]);
        EXPECT_EQ(*--it, utf8[6]);
        EXPECT_EQ(*--it, utf8[5]);
        EXPECT_EQ(*--it, utf8[4]);
        EXPECT_EQ(*--it, utf8[3]);
        EXPECT_EQ(*--it, utf8[2]);
        EXPECT_EQ(*--it, utf8[1]);
        EXPECT_EQ(*--it, utf8[0]);

        EXPECT_EQ(*it++, utf8[0]);
        EXPECT_EQ(*it++, utf8[1]);
        EXPECT_EQ(*it++, utf8[2]);
        EXPECT_EQ(*it++, utf8[3]);
        EXPECT_EQ(*it++, utf8[4]);
        EXPECT_EQ(*it++, utf8[5]);
        EXPECT_EQ(*it++, utf8[6]);
        EXPECT_EQ(*it++, utf8[7]);
        EXPECT_EQ(*it++, utf8[8]);
        EXPECT_EQ(*it++, utf8[9]);
    }

    // UTF-8 -> UTF-16
    {
        auto it = text::utf8::to_utf16_iterator(utf8);

        EXPECT_EQ(*it++, utf16[0]);
        EXPECT_EQ(*it++, utf16[1]);
        EXPECT_EQ(*it++, utf16[2]);
        EXPECT_EQ(*it++, utf16[3]);
        EXPECT_EQ(*it++, utf16[4]);

        EXPECT_EQ(*--it, utf16[4]);
        EXPECT_EQ(*--it, utf16[3]);
        EXPECT_EQ(*--it, utf16[2]);
        EXPECT_EQ(*--it, utf16[1]);
        EXPECT_EQ(*--it, utf16[0]);

        it = text::utf8::to_utf16_iterator(utf8 + 10);

        EXPECT_EQ(*--it, utf16[4]);
        EXPECT_EQ(*--it, utf16[3]);
        EXPECT_EQ(*--it, utf16[2]);
        EXPECT_EQ(*--it, utf16[1]);
        EXPECT_EQ(*--it, utf16[0]);

        EXPECT_EQ(*it++, utf16[0]);
        EXPECT_EQ(*it++, utf16[1]);
        EXPECT_EQ(*it++, utf16[2]);
        EXPECT_EQ(*it++, utf16[3]);
        EXPECT_EQ(*it++, utf16[4]);
    }

    // UTF-16 -> UTF-8
    {
        auto it = text::utf8::from_utf16_iterator<uint16_t const *>(utf16);

        EXPECT_EQ(*it++, utf8[0]);
        EXPECT_EQ(*it++, utf8[1]);
        EXPECT_EQ(*it++, utf8[2]);
        EXPECT_EQ(*it++, utf8[3]);
        EXPECT_EQ(*it++, utf8[4]);
        EXPECT_EQ(*it++, utf8[5]);
        EXPECT_EQ(*it++, utf8[6]);
        EXPECT_EQ(*it++, utf8[7]);
        EXPECT_EQ(*it++, utf8[8]);
        EXPECT_EQ(*it++, utf8[9]);

        EXPECT_EQ(*--it, utf8[9]);
        EXPECT_EQ(*--it, utf8[8]);
        EXPECT_EQ(*--it, utf8[7]);
        EXPECT_EQ(*--it, utf8[6]);
        EXPECT_EQ(*--it, utf8[5]);
        EXPECT_EQ(*--it, utf8[4]);
        EXPECT_EQ(*--it, utf8[3]);
        EXPECT_EQ(*--it, utf8[2]);
        EXPECT_EQ(*--it, utf8[1]);
        EXPECT_EQ(*--it, utf8[0]);

        it = text::utf8::from_utf16_iterator<uint16_t const *>(utf16 + 5);

        EXPECT_EQ(*--it, utf8[9]);
        EXPECT_EQ(*--it, utf8[8]);
        EXPECT_EQ(*--it, utf8[7]);
        EXPECT_EQ(*--it, utf8[6]);
        EXPECT_EQ(*--it, utf8[5]);
        EXPECT_EQ(*--it, utf8[4]);
        EXPECT_EQ(*--it, utf8[3]);
        EXPECT_EQ(*--it, utf8[2]);
        EXPECT_EQ(*--it, utf8[1]);
        EXPECT_EQ(*--it, utf8[0]);

        EXPECT_EQ(*it++, utf8[0]);
        EXPECT_EQ(*it++, utf8[1]);
        EXPECT_EQ(*it++, utf8[2]);
        EXPECT_EQ(*it++, utf8[3]);
        EXPECT_EQ(*it++, utf8[4]);
        EXPECT_EQ(*it++, utf8[5]);
        EXPECT_EQ(*it++, utf8[6]);
        EXPECT_EQ(*it++, utf8[7]);
        EXPECT_EQ(*it++, utf8[8]);
        EXPECT_EQ(*it++, utf8[9]);
    }
}

// The tests using this struct cover the boundaries of all the cases in Table
// 3-7 in Unicode 9, 3.9/D92
template <int EncodingLength>
struct coverage_test_case
{
    char str_[EncodingLength + 4];
    uint32_t utf32_;
};

TEST(utf_8, test_utf8_coverage_length_1)
{
    coverage_test_case<1> const cases[] = {
        { { char(0x0) }, 0x0 },
        { { char(0x7f) }, 0x7f },
        { { char(0x80) }, 0xfffd },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_2)
{
    coverage_test_case<2> const cases[] = {
        { { char(0xc1), char(0xbf) }, 0xfffd },
        { { char(0xc1), char(0xc0) }, 0xfffd },

        { { char(0xc2), char(0xbf) }, 0xbf },
        { { char(0xc2), char(0xc0) }, 0xfffd },

        { { char(0xdf), char(0xbf) }, 0x7ff },
        { { char(0xdf), char(0xc0) }, 0xfffd },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_a)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xe0), char(0x9f), char(0xc0) }, 0xfffd },
        { { char(0xe0), char(0x9f), char(0xbf) }, 0xfffd },
        { { char(0xe0), char(0xa0), char(0xc0) }, 0xfffd },
        { { char(0xe0), char(0xa0), char(0xbf) }, 0x83f },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_b)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xe1), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xe1), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xe1), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xe1), char(0xbf), char(0xbf) }, 0x1fff },

        { { char(0xec), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xec), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xec), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xec), char(0xbf), char(0xbf) }, 0xcfff },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_c)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xed), char(0xa0), char(0xc0) }, 0xfffd },
        { { char(0xed), char(0xa0), char(0xbf) }, 0xfffd },
        { { char(0xed), char(0x9f), char(0xc0) }, 0xfffd },
        { { char(0xed), char(0x9f), char(0xbf) }, 0xd7ff },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_3_d)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xee), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xee), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xee), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xee), char(0xbf), char(0xbf) }, 0xefff },

        { { char(0xef), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xef), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xef), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xef), char(0xbf), char(0xbf) }, 0xfffd },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_4_a)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xf0), char(0x8f), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xf0), char(0x8f), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xf0), char(0x8f), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xf0), char(0x8f), char(0xbf), char(0xbf) }, 0xfffd },

        { { char(0xf0), char(0x90), char(0xc0), char(0xc0) }, 0xfffd },
        { { char(0xf0), char(0x90), char(0xc0), char(0xbf) }, 0xfffd },
        { { char(0xf0), char(0x90), char(0xbf), char(0xc0) }, 0xfffd },
        { { char(0xf0), char(0x90), char(0xbf), char(0xbf) }, 0x10fff },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

// Note that the only reason we're testing the lower end of the 0x80-0xbf
// range in these next two tests is that using the upper end produces a
// code point reserved for internal use (and thus a 0xfffd).

TEST(utf_8, test_utf8_coverage_length_4_b)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xf1), char(0x7f), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf1), char(0x7f), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf1), char(0x7f), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf1), char(0x7f), char(0x80), char(0x80) }, 0xfffd },

        { { char(0xf1), char(0x80), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf1), char(0x80), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf1), char(0x80), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf1), char(0x80), char(0x80), char(0x80) }, 0x40000 },

        { { char(0xf3), char(0x7f), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf3), char(0x7f), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf3), char(0x7f), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf3), char(0x7f), char(0x80), char(0x80) }, 0xfffd },

        { { char(0xf3), char(0x80), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf3), char(0x80), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf3), char(0x80), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf3), char(0x80), char(0x80), char(0x80) }, 0xc0000 },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}

TEST(utf_8, test_utf8_coverage_length_4_c)
{
    coverage_test_case<3> const cases[] = {
        { { char(0xf4), char(0x90), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf4), char(0x90), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf4), char(0x90), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf4), char(0x90), char(0x80), char(0x80) }, 0xfffd },

        { { char(0xf4), char(0x8f), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf4), char(0x8f), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf4), char(0x8f), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf4), char(0x8f), char(0x80), char(0x80) }, 0x10f000 },

        { { char(0xf5), char(0x90), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf5), char(0x90), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf5), char(0x90), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf5), char(0x90), char(0x80), char(0x80) }, 0xfffd },

        { { char(0xf5), char(0x8f), char(0x7f), char(0x7f) }, 0xfffd },
        { { char(0xf5), char(0x8f), char(0x7f), char(0x80) }, 0xfffd },
        { { char(0xf5), char(0x8f), char(0x80), char(0x7f) }, 0xfffd },
        { { char(0xf5), char(0x8f), char(0x80), char(0x80) }, 0xfffd },
    };

    for (auto c : cases) {
        text::utf8::to_utf32_iterator it(c.str_);
        EXPECT_EQ(*it, c.utf32_);
    }
}


// Unicode 9, 3.9/D93b
// Table 3-8. Use of U+FFFD in UTF-8 Conversion
TEST(utf_8, test_0xfffd)
{
    char const bad_utf8[] = {0x61, char(0xf1), char(0x80), char(0x80), char(0xe1), char(0x80), char(0xc2), 0x62, char(0x80), 0x63, char(0x80), char(0xbf), 0x64};
    uint32_t const expected[] = {0x0061, 0xfffd, 0xfffd, 0xfffd, 0x0062, 0xfffd, 0x0063, 0xfffd, 0xfffd, 0x0064};

    auto it = text::utf8::to_utf32_iterator(bad_utf8);

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

    it = text::utf8::to_utf32_iterator(bad_utf8 + 13);

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
