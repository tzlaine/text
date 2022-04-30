// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/grapheme_break.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/sentence_break.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>
#include <boost/text/bidirectional.hpp>
#include <boost/text/string_utility.hpp>
#include <boost/text/text.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/reverse.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


TEST(break_apis, grapheme_break)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges =
            boost::text::as_graphemes(empty_cps.begin(), empty_cps.end());
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges =
            boost::text::as_graphemes(boost::text::as_utf32(empty_cus));
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // ÷ 1F3FB × 0308 ÷ 1100 ÷
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0]
    // COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};

    {
        EXPECT_EQ(
            boost::text::prev_grapheme_break(
                cps.begin(), cps.begin() + 0, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_grapheme_break(
                cps.begin(), cps.begin() + 1, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_grapheme_break(
                cps.begin(), cps.begin() + 2, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_grapheme_break(
                cps.begin(), cps.begin() + 3, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) -
                cps.begin(),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_grapheme_break(cps, cps.begin() + 0) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_grapheme_break(cps, cps.begin() + 0) -
                cps.begin(),
            2);
    }

#if 0
    {
        auto const range =
            boost::text::grapheme(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
    // Range API
    {
        auto const range = boost::text::grapheme(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
#endif

    {
        auto const all_graphemes =
            boost::text::as_graphemes(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 2> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme :
             all_graphemes | boost::text::reverse | boost::text::reverse) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());

        auto const all_graphemes_reversed =
            boost::text::as_graphemes(cps.begin(), cps.end()) |
            boost::text::reverse;
        i = grapheme_bounds.size();
        for (auto grapheme : all_graphemes_reversed) {
            --i;
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_graphemes = boost::text::as_graphemes(cps);

        std::array<std::pair<int, int>, 2> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());

        auto const all_graphemes_reversed =
            boost::text::as_graphemes(cps) | boost::text::reverse;
        i = grapheme_bounds.size();
        for (auto grapheme : all_graphemes_reversed) {
            --i;
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }


    // Emoji handling
    {
        // woman ZWJ woman ZWJ man ZWJ man
        std::vector<uint32_t> const cps = {
            0x01f469,
            0x200d,
            0x01f469,
            0x200d,
            0x01f466,
            0x200d,
            0x01f466,
        };

        auto const all_graphemes = boost::text::as_graphemes(cps);

        std::array<std::pair<int, int>, 1> const grapheme_bounds = {{{0, 7}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());
    }
    {
        // ghost woman ZWJ woman ZWJ man ZWJ man
        std::vector<uint32_t> const cps = {
            0x01f478,
            0x01f469,
            0x200d,
            0x01f469,
            0x200d,
            0x01f466,
            0x200d,
            0x01f466,
        };

        auto const all_graphemes = boost::text::as_graphemes(cps);

        std::array<std::pair<int, int>, 2> const grapheme_bounds = {
            {{0, 1}, {1, 8}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(grapheme.begin() - cps.begin(), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(grapheme.end() - cps.begin(), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());
    }
}

TEST(break_apis, grapheme_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    // ÷ 1F3FB × 0308 ÷ 1100 ÷
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0]
    // COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    std::string s;
    {
        // 4,2,3 code units, respectively.
        std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
        assert(s.size() == 9);
    }

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_grapheme_break(begin, begin, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_grapheme_break(begin, end)),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_grapheme_break(begin, one, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_grapheme_break(begin, end)),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_grapheme_break(begin, two, end)),
            2);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_grapheme_break(two, end)),
            3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_grapheme_break(begin, three, end)),
            2);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_grapheme_break(two, end)),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_grapheme_break(cp_range, begin)),
            0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_grapheme_break(cp_range, begin)),
            2);
    }

#if 0
    {
        auto const range = boost::text::grapheme(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
    // Range API
    {
        auto const range = boost::text::grapheme(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
#endif

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    {
        auto const all_graphemes = boost::text::as_graphemes(begin, end);

        std::array<std::pair<int, int>, 2> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()),
                grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());
    }
    // Range API
    {
        auto const all_graphemes = boost::text::as_graphemes(cp_range);

        std::array<std::pair<int, int>, 2> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()),
                grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)grapheme_bounds.size());
    }
#endif
}

TEST(break_apis, word_break)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges = boost::text::words(empty_cps.begin(), empty_cps.end());
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges = boost::text::words(boost::text::as_utf32(empty_cus));
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) ×
    // [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷
    // [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
    // [0.3]
    std::array<uint32_t, 6> cps = {{0x61, 0x5f, 0x61, 0x2e, 0x3a, 0x61}};

    {
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 0, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 1, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 2, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 3, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 3, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 4, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 4, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 5, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 5, cps.end()) -
                cps.begin(),
            6);
        EXPECT_EQ(
            boost::text::prev_word_break(
                cps.begin(), cps.begin() + 6, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::next_word_break(cps.begin() + 5, cps.end()) -
                cps.begin(),
            6);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_word_break(cps, cps.begin() + 0) - cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_word_break(cps, cps.begin() + 0) - cps.begin(),
            3);
    }

    {
        auto const range =
            boost::text::word(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }
    // Range API
    {
        auto const range = boost::text::word(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }

    {
        auto const all_words = boost::text::words(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(word.begin() - cps.begin(), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(word.end() - cps.begin(), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());

        auto const all_words_reversed =
            boost::text::words(cps.begin(), cps.end()) | boost::text::reverse;
        i = word_bounds.size();
        for (auto word : all_words_reversed) {
            --i;
            EXPECT_EQ(word.begin() - cps.begin(), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(word.end() - cps.begin(), word_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_words = boost::text::words(cps);

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(word.begin() - cps.begin(), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(word.end() - cps.begin(), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());

        auto const all_words_reversed =
            boost::text::words(cps) | boost::text::reverse;
        i = word_bounds.size();
        for (auto word : all_words_reversed) {
            --i;
            EXPECT_EQ(word.begin() - cps.begin(), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(word.end() - cps.begin(), word_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, word_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) ×
    // [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷
    // [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
    // [0.3]
    std::string s;
    {
        std::array<uint32_t, 6> cps = {{0x61, 0x5f, 0x61, 0x2e, 0x3a, 0x61}};
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
    }

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const four = std::next(three);
    auto const five = std::next(four);
    auto const six = std::next(five);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_word_break(begin, begin, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(begin, end)), 3);
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(begin, one, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(begin, end)), 3);
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(begin, two, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(two, end)), 3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_word_break(begin, three, end)),
            3);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(three, end)), 4);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_word_break(begin, four, end)),
            4);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(four, end)), 5);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_word_break(begin, five, end)),
            5);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(five, end)), 6);
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(begin, six, end)),
            5);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(five, end)), 6);
    }
    // Range API
    {
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(cp_range, begin)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(cp_range, begin)),
            3);
    }

    {
        auto const range = boost::text::word(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }
    // Range API
    {
        auto const range = boost::text::word(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    {
        auto const all_words = boost::text::words(begin, end);

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(std::distance(begin, word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, word.end()), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());
    }
    // Range API
    {
        auto const all_words = boost::text::words(cp_range);

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(std::distance(begin, word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, word.end()), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());
    }
#endif
}

TEST(break_apis, word_tailoring_MidLetter)
{
    std::string const s = "multi-part words with dashes";

    std::vector<uint32_t> const cps(s.begin(), s.end());
    auto const begin = cps.begin();
    auto const end = cps.end();

    auto const midletter_dash = [](uint32_t cp) {
        if (cp == 0x002d)
            return boost::text::word_property::MidLetter;
        return boost::text::word_prop(cp);
    };

    // Default breaks.
    {
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(cps, begin)), 0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_word_break(cps, std::next(begin, 4))),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(cps, begin)), 5);
    }
    // MidLetter tailoring.
    {
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_word_break(cps, begin, midletter_dash)),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_word_break(
                    cps, std::next(begin, 9), midletter_dash)),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::next_word_break(cps, begin, midletter_dash)),
            10);
    }

    // Default breaks.
    {
        auto const word = boost::text::word(begin, begin, end);
        std::string const expected_word = "multi";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // MidLetter tailoring.
    {
        auto const word = boost::text::word(begin, begin, end, midletter_dash);
        std::string const expected_word = "multi-part";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // Default breaks, range API.
    {
        auto const word = boost::text::word(cps, begin);
        std::string const expected_word = "multi";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // MidLetter tailoring, range API.
    {
        auto const word = boost::text::word(cps, begin, midletter_dash);
        std::string const expected_word = "multi-part";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }

    // Default breaks.
    {
        std::string const expected_words[9] = {
            "multi", "-", "part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(begin, end)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 9);
    }
    // MidLetter tailoring.
    {
        std::string const expected_words[7] = {
            "multi-part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(begin, end, midletter_dash)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 7);

        i = 7;
        for (auto word : boost::text::words(begin, end, midletter_dash) |
                             boost::text::reverse) {
            --i;
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
        }
        EXPECT_EQ(i, 0);
    }
    // Default breaks, range API.
    {
        std::string const expected_words[9] = {
            "multi", "-", "part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(cps)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 9);

        i = 9;
        for (auto word : boost::text::words(cps) | boost::text::reverse) {
            --i;
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
        }
        EXPECT_EQ(i, 0);
    }
    // MidLetter tailoring, range API.
    {
        std::string const expected_words[7] = {
            "multi-part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(cps, midletter_dash)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 7);

        i = 7;
        for (auto word :
             boost::text::words(cps, midletter_dash) | boost::text::reverse) {
            --i;
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, word_tailoring_MidLetter_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    std::string const s = "multi-part words with dashes";

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> const
        cp_range{
            u32_iter(c_str, c_str, boost::text::null_sentinel{}),
            boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const end = cp_range.end();

    auto const midletter_dash = [](uint32_t cp) {
        if (cp == 0x002d)
            return boost::text::word_property::MidLetter;
        return boost::text::word_prop(cp);
    };

    // Default breaks.
    {
        EXPECT_EQ(
            std::distance(begin, boost::text::prev_word_break(cp_range, begin)),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_word_break(cp_range, std::next(begin, 4))),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_word_break(cp_range, begin)),
            5);
    }
    // MidLetter tailoring.
    {
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_word_break(cp_range, begin, midletter_dash)),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_word_break(
                    cp_range, std::next(begin, 9), midletter_dash)),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::next_word_break(cp_range, begin, midletter_dash)),
            10);
    }

    // Default breaks.
    {
        auto const word = boost::text::word(begin, begin, end);
        std::string const expected_word = "multi";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // MidLetter tailoring.
    {
        auto const word = boost::text::word(begin, begin, end, midletter_dash);
        std::string const expected_word = "multi-part";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // Default breaks, range API.
    {
        auto const word = boost::text::word(cp_range, begin);
        std::string const expected_word = "multi";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }
    // MidLetter tailoring, range API.
    {
        auto const word = boost::text::word(cp_range, begin, midletter_dash);
        std::string const expected_word = "multi-part";
        auto const expected = boost::text::as_utf32(expected_word);
        EXPECT_TRUE(boost::algorithm::equal(
            word.begin(), word.end(), expected.begin(), expected.end()));
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    // Default breaks.
    {
        std::string const expected_words[9] = {
            "multi", "-", "part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(begin, end)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 9);
    }
    // MidLetter tailoring.
    {
        std::string const expected_words[7] = {
            "multi-part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(begin, end, midletter_dash)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 7);
    }
    // Default breaks, range API.
    {
        std::string const expected_words[9] = {
            "multi", "-", "part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(cp_range)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 9);
    }
    // MidLetter tailoring, range API.
    {
        std::string const expected_words[7] = {
            "multi-part", " ", "words", " ", "with", " ", "dashes"};
        int i = 0;
        for (auto word : boost::text::words(cp_range, midletter_dash)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 7);
    }
#endif
}

TEST(break_apis, word_tailoring_cp_break)
{
    std::string const s = "snake_case camelCase";

    std::vector<uint32_t> cps(s.begin(), s.end());
    auto const begin = cps.begin();
    auto const end = cps.end();

    auto const midletter_dash = [](uint32_t cp) {
        if (cp == 0x002d)
            return boost::text::word_property::MidLetter;
        return boost::text::word_prop(cp);
    };

    auto const identifier_break = [](uint32_t prev_prev,
                                     uint32_t prev,
                                     uint32_t curr,
                                     uint32_t next,
                                     uint32_t next_next) {
        if ((prev == '_') != (curr == '_'))
            return true;
        if (0x61 <= prev && prev <= 0x7a && 0x41 <= curr && curr <= 0x5a)
            return true;
        return false;
    };

    // Default breaks.
    {
        std::string const expected_words[3] = {"snake_case", " ", "camelCase"};
        int i = 0;
        for (auto word : boost::text::words(begin, end)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 3);
    }
    // MidLetter tailoring with identifier break.
    {
        std::string const expected_words[6] = {
            "snake", "_", "case", " ", "camel", "Case"};
        int i = 0;
        for (auto word :
             boost::text::words(begin, end, midletter_dash, identifier_break)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 6);
    }
    // Default breaks, range API.
    {
        std::string const expected_words[3] = {"snake_case", " ", "camelCase"};
        int i = 0;
        for (auto word : boost::text::words(cps)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 3);
    }
    // MidLetter tailoring with identifier break, range API.
    {
        std::string const expected_words[6] = {
            "snake", "_", "case", " ", "camel", "Case"};
        int i = 0;
        for (auto word :
             boost::text::words(cps, midletter_dash, identifier_break)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 6);
    }
}

// This only works in C++20 and later, because range-for does not support
// non-common_ranges before that.
#if 202002L <= __cplusplus
TEST(break_apis, word_tailoring_cp_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    std::string const s = "snake_case camelCase";

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> const
        cp_range{
            u32_iter(c_str, c_str, boost::text::null_sentinel{}),
            boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const end = cp_range.end();

    auto const midletter_dash = [](uint32_t cp) {
        if (cp == 0x002d)
            return boost::text::word_property::MidLetter;
        return boost::text::word_prop(cp);
    };

    auto const identifier_break = [](uint32_t prev_prev,
                                     uint32_t prev,
                                     uint32_t curr,
                                     uint32_t next,
                                     uint32_t next_next) {
        if ((prev == '_') != (curr == '_'))
            return true;
        if (0x61 <= prev && prev <= 0x7a && 0x41 <= curr && curr <= 0x5a)
            return true;
        return false;
    };

    // Default breaks.
    {
        std::string const expected_words[3] = {"snake_case", " ", "camelCase"};
        int i = 0;
        for (auto word : boost::text::words(begin, end)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 3);
    }
    // MidLetter tailoring with identifier break.
    {
        std::string const expected_words[6] = {
            "snake", "_", "case", " ", "camel", "Case"};
        int i = 0;
        for (auto word :
             boost::text::words(begin, end, midletter_dash, identifier_break)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 6);
    }
    // Default breaks, range API.
    {
        std::string const expected_words[3] = {"snake_case", " ", "camelCase"};
        int i = 0;
        for (auto word : boost::text::words(cp_range)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 3);
    }
    // MidLetter tailoring with identifier break, range API.
    {
        std::string const expected_words[6] = {
            "snake", "_", "case", " ", "camel", "Case"};
        int i = 0;
        for (auto word :
             boost::text::words(cp_range, midletter_dash, identifier_break)) {
            auto const expected = boost::text::as_utf32(expected_words[i]);
            EXPECT_TRUE(boost::algorithm::equal(
                word.begin(), word.end(), expected.begin(), expected.end()));
            ++i;
        }
        EXPECT_EQ(i, 6);
    }
}
#endif

TEST(break_apis, sentence_break)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges =
            boost::text::sentences(empty_cps.begin(), empty_cps.end());
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges =
            boost::text::sentences(boost::text::as_utf32(empty_cus));
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // ÷ 5B57 × 3002 ÷ 5B83 ÷
    // ÷ [0.2] CJK UNIFIED IDEOGRAPH-5B57 (OLetter) × [998.0] IDEOGRAPHIC FULL
    // STOP (STerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x5b57, 0x3002, 0x5b83}};

    {
        EXPECT_EQ(
            boost::text::prev_sentence_break(
                cps.begin(), cps.begin() + 0, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_sentence_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_sentence_break(
                cps.begin(), cps.begin() + 1, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_sentence_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_sentence_break(
                cps.begin(), cps.begin() + 2, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_sentence_break(cps.begin() + 2, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_sentence_break(
                cps.begin(), cps.begin() + 3, cps.end()) -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_sentence_break(cps.begin() + 2, cps.end()) -
                cps.begin(),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_sentence_break(cps, cps.begin() + 0) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_sentence_break(cps, cps.begin() + 0) -
                cps.begin(),
            2);
    }

    {
        auto const range =
            boost::text::sentence(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
    // Range API
    {
        auto const range = boost::text::sentence(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }

    {
        auto const all_sentences =
            boost::text::sentences(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(sentence.begin() - cps.begin(), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(sentence.end() - cps.begin(), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)sentence_bounds.size());

        auto const all_sentences_reversed =
            boost::text::sentences(cps.begin(), cps.end()) |
            boost::text::reverse;
        i = sentence_bounds.size();
        for (auto sentence : all_sentences_reversed) {
            --i;
            EXPECT_EQ(sentence.begin() - cps.begin(), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(sentence.end() - cps.begin(), sentence_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_sentences = boost::text::sentences(cps);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(sentence.begin() - cps.begin(), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(sentence.end() - cps.begin(), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)sentence_bounds.size());

        auto const all_sentences_reversed =
            boost::text::sentences(cps) | boost::text::reverse;
        i = sentence_bounds.size();
        for (auto sentence : all_sentences_reversed) {
            --i;
            EXPECT_EQ(sentence.begin() - cps.begin(), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(sentence.end() - cps.begin(), sentence_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, sentence_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    // ÷ 5B57 × 3002 ÷ 5B83 ÷
    // ÷ [0.2] CJK UNIFIED IDEOGRAPH-5B57 (OLetter) × [998.0] IDEOGRAPHIC FULL
    // STOP (STerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    std::string s;
    {
        std::array<uint32_t, 3> cps = {{0x5b57, 0x3002, 0x5b83}};
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
    }

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_sentence_break(begin, begin, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_sentence_break(begin, end)),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_sentence_break(begin, one, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_sentence_break(begin, end)),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_sentence_break(begin, two, end)),
            2);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_sentence_break(two, end)),
            3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_sentence_break(begin, three, end)),
            2);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_sentence_break(two, end)),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_sentence_break(cp_range, begin)),
            0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_sentence_break(cp_range, begin)),
            2);
    }

    {
        auto const range = boost::text::sentence(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
    // Range API
    {
        auto const range = boost::text::sentence(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    {
        auto const all_sentences = boost::text::sentences(begin, end);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(
                std::distance(begin, sentence.begin()),
                sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, sentence.end()), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)sentence_bounds.size());
    }
    // Range API
    {
        auto const all_sentences = boost::text::sentences(cp_range);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(
                std::distance(begin, sentence.begin()),
                sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, sentence.end()), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)sentence_bounds.size());
    }
#endif
}

TEST(break_apis, line_break)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges = boost::text::lines(empty_cps.begin(), empty_cps.end());
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges = boost::text::lines(boost::text::as_utf32(empty_cus));
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges = empty_cus | boost::text::as_utf32 | boost::text::lines;
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // × 200B × 0020 ÷ 0030 ÷
    // × [0.3] ZERO WIDTH SPACE (ZW) × [7.01] SPACE (SP) ÷ [8.0] DIGIT ZERO (NU)
    // ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x200b, 0x20, 0x30}};

    {
        EXPECT_EQ(
            boost::text::prev_allowed_line_break(
                cps.begin(), cps.begin() + 0, cps.end())
                    .iter -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_allowed_line_break(cps.begin() + 0, cps.end())
                    .iter -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_allowed_line_break(
                cps.begin(), cps.begin() + 1, cps.end())
                    .iter -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_allowed_line_break(cps.begin() + 0, cps.end())
                    .iter -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::prev_allowed_line_break(
                cps.begin(), cps.begin() + 2, cps.end())
                    .iter -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_allowed_line_break(cps.begin() + 2, cps.end())
                    .iter -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_allowed_line_break(
                cps.begin(), cps.begin() + 3, cps.end())
                    .iter -
                cps.begin(),
            2);
        EXPECT_EQ(
            boost::text::next_allowed_line_break(cps.begin() + 2, cps.end())
                    .iter -
                cps.begin(),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_allowed_line_break(cps, cps.begin() + 0).iter -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_allowed_line_break(cps, cps.begin() + 0).iter -
                cps.begin(),
            2);
    }

    {
        auto const range =
            boost::text::allowed_line(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
    // Range API
    {
        auto const range = boost::text::allowed_line(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }

    {
        auto const all_lines = boost::text::lines(
            cps.begin(), cps.end(), boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            boost::text::lines(
                cps.begin(), cps.end(), boost::text::allowed_breaks) |
            boost::text::reverse;
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            EXPECT_EQ(line.hard_break(), false);
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_lines =
            boost::text::lines(cps, boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            boost::text::lines(cps, boost::text::allowed_breaks) |
            boost::text::reverse;
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            EXPECT_EQ(line.hard_break(), false);
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_lines =
            cps| boost::text::lines(boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            cps | boost::text::lines(boost::text::allowed_breaks) |
            boost::text::reverse;
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            EXPECT_EQ(line.hard_break(), false);
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, line_break_terminal_newline)
{
    {
        boost::text::text t("A line. Another line.\nYet one more.");
        int i = 0;
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks)) {
            EXPECT_EQ(line.hard_break(), i == 3);
#if 0
            std::cout << (line.hard_break() ? "*" : " ") << " i=" << i
                      << " line="
                      << boost::text::text_view(line.begin(), line.end())
                      << "\n";
#endif
            ++i;
        }
        EXPECT_EQ(i, 7);
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks) |
                             boost::text::reverse) {
            --i;
            EXPECT_EQ(line.hard_break(), i == 3);
        }
        EXPECT_EQ(i, 0);
    }
    {
        boost::text::text t("A line. Another line.\nYet one more.\n");
        int i = 0;
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks)) {
            EXPECT_EQ(line.hard_break(), i == 3 || i == 6);
            ++i;
        }
        EXPECT_EQ(i, 7);
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks) |
                             boost::text::reverse) {
            --i;
            EXPECT_EQ(line.hard_break(), i == 3 || i == 6);
        }
        EXPECT_EQ(i, 0);
    }
    {
        boost::text::text t("\n");
        int i = 0;
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks)) {
            EXPECT_TRUE(line.hard_break());
            ++i;
        }
        EXPECT_EQ(i, 1);
        for (auto line : boost::text::lines(t, boost::text::allowed_breaks) |
                             boost::text::reverse) {
            --i;
            EXPECT_TRUE(line.hard_break());
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, line_break_hard)
{
    std::array<uint32_t, 5> cps = {{'a', ' ', 'b', '\n', 'c'}};

    {
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 0, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 1, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 1, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 2, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 2, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 3, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 3, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 4, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 4, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::prev_hard_line_break(
                cps.begin(), cps.begin() + 5, cps.end()) -
                cps.begin(),
            4);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps.begin() + 5, cps.end()) -
                cps.begin(),
            5);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_hard_line_break(cps, cps.begin() + 0) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_hard_line_break(cps, cps.begin() + 0) -
                cps.begin(),
            4);
    }

    {
        auto const range =
            boost::text::line(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 4);
    }
    // Range API
    {
        auto const range = boost::text::line(cps, cps.begin() + 4);
        EXPECT_EQ(range.begin() - cps.begin(), 4);
        EXPECT_EQ(range.end() - cps.begin(), 5);
    }

    {
        auto const all_lines = boost::text::lines(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 4}, {4, 5}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            boost::text::lines(cps.begin(), cps.end()) | boost::text::reverse;
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_lines = boost::text::lines(cps);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 4}, {4, 5}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            boost::text::lines(cps) | boost::text::reverse;
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

struct line_stateful_cp_extent
{
    line_stateful_cp_extent() : i(0) {}

    template<typename Iter, typename Sentinel>
    int operator()(Iter first, Sentinel last) const noexcept
    {
        if ((int)index_counts.size() <= i)
            index_counts.resize(i + 1);
        ++index_counts[i];
        ++i;
        return std::distance(first, last);
    }

    mutable int i;

#if !defined(__cpp_coroutines) // TODO: This does not work for GCC's
                               // impementation as of this writing.
    // Unused; here just to check that moves are done properly.
    std::unique_ptr<int> ptr;
#endif

    static std::vector<int> index_counts;
};
std::vector<int> line_stateful_cp_extent::index_counts;

TEST(break_apis, line_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    // × 200B × 0020 ÷ 0030 ÷
    // × [0.3] ZERO WIDTH SPACE (ZW) × [7.01] SPACE (SP) ÷ [8.0] DIGIT ZERO (NU)
    // ÷ [0.3]
    std::string s;
    {
        std::array<uint32_t, 3> cps = {{0x200b, 0x20, 0x30}};
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
    }

    char const * c_str = s.c_str();

#if 0
    std::cout << "s=" << std::quoted(s) << " (size=" << s.size() << ")"
              << std::endl;
#endif

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_allowed_line_break(begin, begin, end).iter),
            0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_allowed_line_break(begin, end).iter),
            2);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_allowed_line_break(begin, one, end).iter),
            0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_allowed_line_break(begin, end).iter),
            2);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_allowed_line_break(begin, two, end).iter),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_allowed_line_break(two, end).iter),
            3);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_allowed_line_break(begin, three, end).iter),
            2);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_allowed_line_break(two, end).iter),
            3);
    }
    // Range API
    {
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::prev_allowed_line_break(cp_range, begin).iter),
            0);
        EXPECT_EQ(
            std::distance(
                begin,
                boost::text::next_allowed_line_break(cp_range, begin).iter),
            2);
    }

    {
        auto const range = boost::text::allowed_line(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
    // Range API
    {
        auto const range = boost::text::allowed_line(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    {
        auto const all_lines =
            boost::text::lines(begin, end, boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    // Range API
    {
        auto const all_lines =
            boost::text::lines(cp_range, boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    // Range API
    {
        auto const all_lines =
            cp_range | boost::text::lines(boost::text::allowed_breaks);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

#if 0 // TODO: Broken!
    // GraphemeRange API
    {
        auto const all_lines = c_str | boost::text::as_graphemes |
                               boost::text::lines(boost::text::allowed_breaks);

        auto const all_lines_begin = all_lines.begin()->begin();

        auto const ultimate_begin = all_lines_begin.base().base();
        EXPECT_EQ(ultimate_begin, c_str);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(
                std::distance(all_lines_begin, line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        auto const all_lines = c_str | boost::text::as_graphemes |
                               boost::text::lines(boost::text::allowed_breaks) |
                               boost::text::reverse;

        auto const all_lines_begin = all_lines.begin()->begin();

        auto const ultimate_begin = all_lines_begin.base().base();
        EXPECT_EQ(ultimate_begin, c_str);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 2;
        for (auto line : all_lines) {
            --i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.end()),
                line_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
#endif
    {
        auto const all_lines =
            c_str | boost::text::as_graphemes | boost::text::lines;

        auto const all_lines_begin = all_lines.begin()->begin();

        auto const ultimate_begin = all_lines_begin.base().base();
        EXPECT_EQ(ultimate_begin, c_str);

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(
                std::distance(all_lines_begin, line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        auto const all_lines = c_str | boost::text::as_graphemes |
                               boost::text::lines | boost::text::reverse;

        auto const all_lines_begin = all_lines.begin()->begin();

        auto const ultimate_begin = all_lines_begin.base().base();
        EXPECT_EQ(ultimate_begin, c_str);

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 1;
        for (auto line : all_lines) {
            --i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(all_lines_begin, line.end()),
                line_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
#endif


    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    using begin_t = decltype(begin);

    // 80 columns -> don't take the allowed break in the middle.
    {
        auto const _80_column_lines =
            boost::text::lines(begin, end, 80, [](begin_t it, begin_t last) {
                return distance(it, last);
            });

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 0;
        for (auto line : _80_column_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // 2 columns -> take the allowed break in the middle.
    {
        auto const _2_column_lines =
            boost::text::lines(begin, end, 2, [](begin_t it, begin_t last) {
                return distance(it, last);
            });

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : _2_column_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // 1 column -> break after every character, since overlong sequences are
    // broken by default.
    {
        auto const _1_column_lines =
            boost::text::lines(begin, end, 1, [](begin_t it, begin_t last) {
                return distance(it, last);
            });

        std::array<std::pair<int, int>, 3> const line_bounds = {
            {{0, 1}, {1, 2}, {2, 3}}};

        int i = 0;
        for (auto line : _1_column_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // 1 column -> ignore the overlong lines, and so only take the allowed
    // break in the middle.
    {
        auto const _1_column_lines = boost::text::lines(
            begin,
            end,
            1,
            [](begin_t it, begin_t last) { return distance(it, last); },
            false);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : _1_column_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
#endif

    std::array<uint32_t, 17> const lb15_cps = {
        {0x0061,
         0x006D,
         0x0062,
         0x0069,
         0x0067,
         0x0075,
         0x0028,
         0x00AB,
         0x0020,
         0x0308,
         0x0020,
         0x00BB,
         0x0029,
         0x0028,
         0x0065,
         0x0308,
         0x0029}};

    // Exercise detail::skip_forward() code.
    {
        // × 0024 × 0308 × 0020 ÷ 002D ÷ 0061 ÷
        // × [0.3] DOLLAR SIGN (PR) × [9.0] COMBINING DIAERESIS (CM1_CM) ×
        // [7.01] SPACE (SP) ÷ [18.0] HYPHEN-MINUS (HY) ÷ [0.3] LATIN SMALL
        // LETTER A (AL) ÷ [0.3]
        std::array<uint32_t, 5> const cps = {
            {0x0024, 0x0308, 0x0020, 0x002D, 0x0061}};

        using cp_iter_t = decltype(cps.begin());
        auto const _5_column_lines =
            boost::text::lines(cps, 5, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 5-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 5}}};

        int i = 0;
        for (auto line : _5_column_lines) {
            EXPECT_EQ(
                std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        // × 0024 × 0308 × 0020 ÷ 002D ÷ 0061 ÷
        // × [0.3] DOLLAR SIGN (PR) × [9.0] COMBINING DIAERESIS (CM1_CM) ×
        // [7.01] SPACE (SP) ÷ [18.0] HYPHEN-MINUS (HY) ÷ [0.3] LATIN SMALL
        // LETTER A (AL) ÷ [0.3]
        std::array<uint32_t, 5> const cps = {
            {0x0024, 0x0308, 0x0020, 0x002D, 0x0061}};

        using cp_iter_t = decltype(cps.begin());
        auto const _5_column_lines =
            cps | boost::text::lines(5, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 5-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 5}}};

        int i = 0;
        for (auto line : _5_column_lines) {
            EXPECT_EQ(
                std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // Exercise detail::skip_forward_spaces_between() code.
    {
        // LB13/LB16
        // × 007D × 0020 × 17D6 ÷ 0061 ÷
        // × [0.3] RIGHT CURLY BRACKET (CL) × [7.01] SPACE (SP) × [16.0] KHMER
        // SIGN CAMNUC PII KUUH (NS) ÷ [0.3] LATIN SMALL LETTER A (AL) ÷ [0.3]
        std::array<uint32_t, 4> const cps = {{0x007D, 0x0020, 0x17D6, 0x0061}};

        using cp_iter_t = decltype(cps.begin());
        auto const _4_column_lines =
            boost::text::lines(cps, 4, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 4-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 4}}};

        int i = 0;
        for (auto line : _4_column_lines) {
            EXPECT_EQ(
                std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        // LB14
        // × 0028 × 0020 × FFFC ÷ 0061 ÷
        // × [0.3] LEFT PARENTHESIS (OP) × [7.01] SPACE (SP) × [14.0] OBJECT
        // REPLACEMENT CHARACTER (CB) ÷ [0.3] LATIN SMALL LETTER A (AL) ÷ [0.3]
        std::array<uint32_t, 4> const cps = {{0x0028, 0x0020, 0xFFFC, 0x0061}};

        using cp_iter_t = decltype(cps.begin());
        auto const _4_column_lines =
            boost::text::lines(cps, 4, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 4-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 4}}};

        int i = 0;
        for (auto line : _4_column_lines) {
            EXPECT_EQ(
                std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        // LB15
        // × 0061 × 006D × 0062 × 0069 × 0067 × 0075 × 0028 × 00AB × 0020 ÷ 0308
        // × 0020 ÷ 00BB × 0029 ÷ 0028 × 0065 × 0308 × 0029 ÷ × [0.3] LATIN
        // SMALL LETTER A (AL) × [28.0] LATIN SMALL LETTER M (AL) × [28.0] LATIN
        // SMALL LETTER B (AL) × [28.0] LATIN SMALL LETTER I (AL) × [28.0] LATIN
        // SMALL LETTER G (AL) × [28.0] LATIN SMALL LETTER U (AL) × [30.01] LEFT
        // PARENTHESIS (OP) × [14.0] LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        // (QU) × [7.01] SPACE (SP) ÷ [18.0] COMBINING DIAERESIS (CM1_CM) ×
        // [7.01] SPACE (SP) ÷ [18.0] RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        // (QU) × [13.02] RIGHT PARENTHESIS (CP) ÷ [999.0] LEFT PARENTHESIS (OP)
        // × [14.0] LATIN SMALL LETTER E (AL) × [9.0] COMBINING DIAERESIS
        // (CM1_CM) × [13.03] RIGHT PARENTHESIS (CP) ÷ [0.3]

        using cp_iter_t = decltype(lb15_cps.begin());
        auto const _17_column_lines =
            boost::text::lines(lb15_cps, 17, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 17-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 17}}};

        int i = 0;
        for (auto line : _17_column_lines) {
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
    {
        // LB16
        // × 2014 × 0020 × 2014 ÷ 0061 ÷
        // × [0.3] EM DASH (B2) × [7.01] SPACE (SP) × [17.0] EM DASH (B2) ÷
        // [0.3] LATIN SMALL LETTER A (AL) ÷ [0.3]
        std::array<uint32_t, 4> const cps = {{0x2014, 0x0020, 0x2014, 0x0061}};

        using cp_iter_t = decltype(cps.begin());
        auto const _4_column_lines =
            boost::text::lines(cps, 4, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        // The first break opportunity is skipped due to the 4-column line
        // width.
        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 4}}};

        int i = 0;
        for (auto line : _4_column_lines) {
            EXPECT_EQ(
                std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    {
        // × 0061 × 006D × 0062    × 0069 × 0067 × 0075 ×    0028 × 00AB × 0020
        // ÷    0308 × 0020 ÷    00BB × 0029 ÷    0028 × 0065 × 0308 ×    0029 ÷
        using cp_iter_t = decltype(lb15_cps.begin());
        auto const _3_column_lines =
            boost::text::lines(lb15_cps, 3, [](cp_iter_t it, cp_iter_t last) {
                return std::distance(it, last);
            });

        std::array<std::pair<int, int>, 7> const line_bounds = {{
            {0, 3},
            {3, 6},
            {6, 9},
            {9, 11},
            {11, 13},
            {13, 16},
            {16, 17},
        }};

        int i = 0;
        for (auto line : _3_column_lines) {
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // Same test as above, but with a stateful cp_extent.
    {
        line_stateful_cp_extent::index_counts.clear();

        // × 0061 × 006D × 0062    × 0069 × 0067 × 0075 ×    0028 × 00AB × 0020
        // ÷    0308 × 0020 ÷    00BB × 0029 ÷    0028 × 0065 × 0308 ×    0029 ÷
        auto const _3_column_lines =
            boost::text::lines(lb15_cps, 3, line_stateful_cp_extent{});

        std::array<std::pair<int, int>, 7> const line_bounds = {{
            {0, 3},
            {3, 6},
            {6, 9},
            {9, 11},
            {11, 13},
            {13, 16},
            {16, 17},
        }};

        int i = 0;
        for (auto line : _3_column_lines) {
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        EXPECT_EQ(
            std::count(
                line_stateful_cp_extent::index_counts.begin(),
                line_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)line_stateful_cp_extent::index_counts.size());
    }

    {
        // × 0061 × 006D ×    0062 × 0069 ×    0067 × 0075 ×    0028 × 00AB ×
        // 0020 ÷    0308 × 0020 ÷    00BB × 0029 ÷    0028 × 0065 ×    0308 ×
        // 0029 ÷
        using cp_iter_t = decltype(lb15_cps.begin());
        auto const _3_column_lines =
            boost::text::lines(lb15_cps, 5, [](cp_iter_t it, cp_iter_t last) {
                return 2 * std::distance(it, last);
            });

        std::array<std::pair<int, int>, 9> const line_bounds = {{
            {0, 2},
            {2, 4},
            {4, 6},
            {6, 8},
            {8, 9},
            {9, 11},
            {11, 13},
            {13, 15},
            {15, 17},
        }};

        int i = 0;
        for (auto line : _3_column_lines) {
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.begin()),
                line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(lb15_cps.begin(), line.end()),
                line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    // Range API
    // 80 columns -> don't take the allowed break in the middle.
    {
        auto const _80_column_lines =
            boost::text::lines(cp_range, 80, [](u32_iter it, u32_iter last) {
                return distance(it, last);
            });

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 0;
        for (auto line : _80_column_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
#endif
}

TEST(break_apis, paragraph_break)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges =
            boost::text::paragraphs(empty_cps.begin(), empty_cps.end());
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges =
            boost::text::paragraphs(boost::text::as_utf32(empty_cus));
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges =
            empty_cus | boost::text::as_utf32 | boost::text::paragraphs;
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // ÷ 0061 × 000D × 000A ÷ 002E × 000A ÷ 0061 ÷
    std::vector<uint32_t> const cps = {0x61, 0xd, 0xa, 0x2e, 0xa, 0x61};

    {
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 0, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 1, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 0, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 2, cps.end()) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 3, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 4, cps.end()) -
                cps.begin(),
            3);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 5, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 5, cps.end()) -
                cps.begin(),
            6);
        EXPECT_EQ(
            boost::text::prev_paragraph_break(
                cps.begin(), cps.begin() + 6, cps.end()) -
                cps.begin(),
            5);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps.begin() + 5, cps.end()) -
                cps.begin(),
            6);
    }
    // Range API
    {
        EXPECT_EQ(
            boost::text::prev_paragraph_break(cps, cps.begin() + 0) -
                cps.begin(),
            0);
        EXPECT_EQ(
            boost::text::next_paragraph_break(cps, cps.begin() + 0) -
                cps.begin(),
            3);
    }

    {
        auto const range =
            boost::text::paragraph(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }
    // Range API
    {
        auto const range = boost::text::paragraph(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 3);
    }

    {
        auto const all_paragraphs =
            boost::text::paragraphs(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());

        auto const all_paragraphs_reversed =
            boost::text::paragraphs(cps.begin(), cps.end()) |
            boost::text::reverse;
        i = paragraph_bounds.size();
        for (auto paragraph : all_paragraphs_reversed) {
            --i;
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    // Range API
    {
        auto const all_paragraphs = boost::text::paragraphs(cps);

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());

        auto const all_paragraphs_reversed =
            boost::text::paragraphs(cps) | boost::text::reverse;
        i = paragraph_bounds.size();
        for (auto paragraph : all_paragraphs_reversed) {
            --i;
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
    {
        auto const all_paragraphs = cps | boost::text::paragraphs;

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());

        auto const all_paragraphs_reversed =
            cps | boost::text::paragraphs | boost::text::reverse;
        i = paragraph_bounds.size();
        for (auto paragraph : all_paragraphs_reversed) {
            --i;
            EXPECT_EQ(
                paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, paragraph_break_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    // ÷ 0061 × 000D × 000A ÷ 002E × 000A ÷ 0061 ÷
    std::string s;
    {
        std::array<uint32_t, 6> const cps = {{0x61, 0xd, 0xa, 0x2e, 0xa, 0x61}};
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
    }

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const four = std::next(three);
    auto const five = std::next(four);
    auto const six = std::next(five);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, begin, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(begin, end)),
            3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, one, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(begin, end)),
            3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, two, end)),
            0);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(begin, end)),
            3);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, three, end)),
            3);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(three, end)),
            5);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, four, end)),
            3);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(three, end)),
            5);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, five, end)),
            5);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(five, end)),
            6);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(begin, six, end)),
            5);
        EXPECT_EQ(
            std::distance(begin, boost::text::next_paragraph_break(five, end)),
            6);
    }
    // Range API
    {
        EXPECT_EQ(
            std::distance(
                begin, boost::text::prev_paragraph_break(cp_range, begin)),
            0);
        EXPECT_EQ(
            std::distance(
                begin, boost::text::next_paragraph_break(cp_range, begin)),
            3);
    }

    {
        auto const range = boost::text::paragraph(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }
    // Range API
    {
        auto const range = boost::text::paragraph(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 3);
    }

    // This only works in C++20 and later, because range-for does not support
    // non-common_ranges before that.
#if 202002L <= __cplusplus
    {
        auto const all_paragraphs = boost::text::paragraphs(begin, end);

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                std::distance(begin, paragraph.begin()),
                paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, paragraph.end()),
                paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());
    }
    // Range API
    {
        auto const all_paragraphs = boost::text::paragraphs(cp_range);

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                std::distance(begin, paragraph.begin()),
                paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, paragraph.end()),
                paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());
    }
    {
        auto const all_paragraphs = cp_range | boost::text::paragraphs;

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 6}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                std::distance(begin, paragraph.begin()),
                paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, paragraph.end()),
                paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());
    }
#endif
}

struct bidi_stateful_cp_extent
{
    bidi_stateful_cp_extent() : i(0) {}

    template<typename Iter, typename Sentinel>
    int operator()(Iter first, Sentinel last) const noexcept
    {
        if ((int)index_counts.size() <= i)
            index_counts.resize(i + 1);
        ++index_counts[i];
        ++i;
        return std::distance(first, last);
    }

    mutable int i;

#if !defined(__cpp_coroutines) // TODO: This does not work for GCC's
                               // impementation as of this writing.
    // Unused; here just to check that moves are done properly.
    std::unique_ptr<int> ptr;
#endif

    static std::vector<int> index_counts;
};
std::vector<int> bidi_stateful_cp_extent::index_counts;

TEST(break_apis, bidi)
{
    // empty range
    {
        std::vector<uint32_t> const empty_cps;
        auto subranges = boost::text::bidirectional_subranges(
            empty_cps.begin(), empty_cps.end(), 0);
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }
    {
        std::string const empty_cus;
        auto subranges = boost::text::bidirectional_subranges(
            boost::text::as_utf32(empty_cus), 0);
        for (auto subrange : subranges) {
            (void)subrange;
        }
    }

    // ON RLE ON FSI ON R RLO L PDF ON PDI ON PDF ON; 3 ('LTR') (line 496999)
    std::vector<uint32_t> const cps = {
        0x0021,
        0x202B,
        0x0021,
        0x2068,
        0x0021,
        0x05BE,
        0x202E,
        0x0041,
        0x202C,
        0x0021,
        0x2069,
        0x0021,
        0x202C,
        0x0021};
    std::vector<uint32_t> const expected_reordered_indices = {
        0, 11, 10, 9, 7, 5, 4, 3, 2, 13};

    {
        std::vector<uint32_t> reordered;
        for (auto subrange :
             boost::text::bidirectional_subranges(cps.begin(), cps.end(), 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    {
        std::vector<uint32_t> reordered;
        for (auto subrange :
             boost::text::bidirectional_subranges(cps.begin(), cps.end())) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    // Range API
    {
        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(cps, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    {
        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(cps)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    // Extent-limited.
    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cps.begin(), cps.end(), 80, bidi_stateful_cp_extent{}, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cps.begin(), cps.end(), 80, bidi_stateful_cp_extent{})) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    // Extent-limited, range API.
    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cps, 80, bidi_stateful_cp_extent{}, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cps, 80, bidi_stateful_cp_extent{})) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }
}

TEST(break_apis, bidi_sentinel)
{
    using u32_iter = boost::text::
        utf_8_to_32_iterator<char const *, boost::text::null_sentinel>;

    std::string s;

    // ON RLE ON FSI ON R RLO L PDF ON PDI ON PDF ON; 3 ('LTR') (line 496999)
    std::vector<uint32_t> const cps = {
        0x0021,
        0x202B,
        0x0021,
        0x2068,
        0x0021,
        0x05BE,
        0x202E,
        0x0041,
        0x202C,
        0x0021,
        0x2069,
        0x0021,
        0x202C,
        0x0021};
    std::vector<uint32_t> const expected_reordered_indices = {
        0, 11, 10, 9, 7, 5, 4, 3, 2, 13};
    {
        s = std::string(
            boost::text::utf8_iterator(cps.begin(), cps.begin(), cps.end()),
            boost::text::utf8_iterator(cps.begin(), cps.end(), cps.end()));
    }

    char const * c_str = s.c_str();

    boost::text::utf32_view<u32_iter, boost::text::null_sentinel> cp_range{
        u32_iter(c_str, c_str, boost::text::null_sentinel{}),
        boost::text::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const end = cp_range.end();

    {
        std::vector<uint32_t> reordered;
        for (auto subrange :
             boost::text::bidirectional_subranges(begin, end, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    {
        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(begin, end)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    // Range API
    {
        std::vector<uint32_t> reordered;
        for (auto subrange :
             boost::text::bidirectional_subranges(cp_range, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    {
        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(cp_range)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    // Extent-limited.
    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 begin, end, 80, bidi_stateful_cp_extent{}, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 begin, end, 80, bidi_stateful_cp_extent{})) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    // Extent-limited, range API.
    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cp_range, 80, bidi_stateful_cp_extent{}, 0)) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }

    {
        bidi_stateful_cp_extent::index_counts.clear();

        std::vector<uint32_t> reordered;
        for (auto subrange : boost::text::bidirectional_subranges(
                 cp_range, 80, bidi_stateful_cp_extent{})) {
            for (auto cp : subrange) {
                reordered.push_back(cp);
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cps[idx] < 0x2066 || 0x2069 < cps[idx]) {
                EXPECT_EQ(reordered[i], cps[idx])
                    << std::hex << " 0x" << reordered[i] << " 0x" << cps[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());

        EXPECT_EQ(
            std::count(
                bidi_stateful_cp_extent::index_counts.begin(),
                bidi_stateful_cp_extent::index_counts.end(),
                1),
            (std::ptrdiff_t)bidi_stateful_cp_extent::index_counts.size());
    }
}
