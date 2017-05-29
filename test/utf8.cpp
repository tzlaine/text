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
}


// TODO: Add a test like consecutive, that tests all permutations of starting
// in one place in a sequence and iterating to another.  This should produce
// some replacement characters in the output.


TEST(utf_8, test_0xfffd)
{
    // Unicode 9, 3.9/D93b
    // Table 3-8. Use of U+FFFD in UTF-8 Conversion
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

#if 0
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
#endif
}


#if 0
TEST(utf_8, test_survey_of_values)
{
}
#endif
