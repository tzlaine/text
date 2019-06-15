#include <boost/text/string_view.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/utf16.hpp>

#include <gtest/gtest.h>


using namespace boost;


TEST(utf_16, test_consecutive)
{
    // Unicode 3.9/D90-D92
    uint32_t const utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};

    // UTF-16 -> UTF-32
    {
        auto it = text::utf16::to_utf32_iterator<uint16_t const *>(
            std::begin(utf16), std::begin(utf16), std::end(utf16));

        auto end = text::utf16::to_utf32_iterator<uint16_t const *>(
            std::begin(utf16), std::end(utf16), std::end(utf16));

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

    // UTF-32 -> UTF-16
    {
        auto it = text::utf16::from_utf32_iterator<uint32_t const *>(
            std::begin(utf32), std::begin(utf32), std::end(utf32));

        auto end = text::utf16::from_utf32_iterator<uint32_t const *>(
            std::begin(utf32), std::end(utf32), std::end(utf32));

        auto const zero = it;
        EXPECT_EQ(*it, utf16[0]);
        it++;
        auto const one = it;
        EXPECT_EQ(*it, utf16[1]);
        it++;
        auto const two = it;
        EXPECT_EQ(*it, utf16[2]);
        it++;
        auto const three = it;
        EXPECT_EQ(*it, utf16[3]);
        it++;
        auto const four = it;
        EXPECT_EQ(*it, utf16[4]);
        it++;
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

        EXPECT_EQ(it, end);

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
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        EXPECT_EQ(*it, utf16[4]);
        ++it;
    }
}

TEST(utf_16, test_back_and_forth)
{
    // Unicode 3.9/D90-D92
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};

    // UTF-16 -> UTF-32
    for (int iterations = 1; iterations <= 4; ++iterations) {
        auto it = text::utf16::to_utf32_iterator<uint16_t const *>(
            std::begin(utf16), std::begin(utf16), std::end(utf16));
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
        auto it = text::utf16::to_utf32_iterator<uint16_t const *>(
            std::begin(utf16), std::end(utf16), std::end(utf16));
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

    // UTF-32 -> UTF-16
    for (int iterations = 1; iterations <= 5; ++iterations) {
        auto it = text::utf16::from_utf32_iterator<uint32_t const *>(
            std::begin(utf32), std::begin(utf32), std::end(utf32));
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
        auto it = text::utf16::from_utf32_iterator<uint32_t const *>(
            std::begin(utf32), std::end(utf32), std::end(utf32));
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
}
