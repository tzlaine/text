#include <boost/text/grapheme_break.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/sentence_break.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>

#include <boost/text/utf8.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


TEST(break_apis, grapheme_break)
{
    // ÷ 1F3FB × 0308 ÷ 1100 ÷	
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0] COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};

    {
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_grapheme_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_grapheme_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_grapheme_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_grapheme_break(cps) - cps.begin(), 2);
    }

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

    {
        auto const all_graphemes =
            boost::text::graphemes(cps.begin(), cps.end());

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
    }
    // Range API
    {
        auto const all_graphemes = boost::text::graphemes(cps);

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
    }
}

TEST(break_apis, grapheme_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // ÷ 1F3FB × 0308 ÷ 1100 ÷	
    // ÷ [0.2] EMOJI MODIFIER FITZPATRICK TYPE-1-2 (E_Modifier) × [9.0] COMBINING DIAERESIS (Extend) ÷ [999.0] HANGUL CHOSEONG KIYEOK (L) ÷ [0.3]
    boost::text::string s;
    {
        // 4,2,3 code units, respectively.
        std::array<uint32_t, 3> cps = {{0x1f3fb, 0x308, 0x1100}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(s.size() == 9);
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, two, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(two, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(begin, three, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(two, end)), 3);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_grapheme_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_grapheme_break(cp_range)), 2);
    }

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

    {
        auto const all_graphemes = boost::text::graphemes(begin, end);

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_graphemes = boost::text::graphemes(cp_range);

        std::array<std::pair<int, int>, 3> const grapheme_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto grapheme : all_graphemes) {
            EXPECT_EQ(
                std::distance(begin, grapheme.begin()), grapheme_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(begin, grapheme.end()), grapheme_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, word_break)
{
    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷	
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) × [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [0.3]
    std::array<uint32_t, 6> cps = {{0x61, 0x5f, 0x61, 0x2e, 0x3a, 0x61}};

    {
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 0, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 0, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 0, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 3, cps.end()) - cps.begin(), 4);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 4, cps.end()) - cps.begin(), 4);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 4, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 5, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 5, cps.end()) - cps.begin(), 6);
        EXPECT_EQ(boost::text::prev_word_break(cps.begin(), cps.begin() + 6, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::next_word_break(cps.begin() + 5, cps.end()) - cps.begin(), 6);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_word_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_word_break(cps) - cps.begin(), 3);
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
    }
}

TEST(break_apis, word_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷	
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) × [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [0.3]
    boost::text::string s;
    {
        std::array<uint32_t, 6> cps = {{0x61, 0x5f, 0x61, 0x2e, 0x3a, 0x61}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const four = std::next(three);
    auto const five = std::next(four);
    auto const six = std::next(five);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(begin, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(begin, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, two, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(two, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, three, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(three, end)), 4);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, four, end)), 4);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(four, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, five, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(five, end)), 6);
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(begin, six, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(five, end)), 6);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_word_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_word_break(cp_range)), 3);
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
    }
}

TEST(break_apis, sentence_break)
{
    // ÷ 5B57 × 3002 ÷ 5B83 ÷	
    // ÷ [0.2] CJK UNIFIED IDEOGRAPH-5B57 (OLetter) × [998.0] IDEOGRAPHIC FULL STOP (STerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x5b57, 0x3002, 0x5b83}};

    {
        EXPECT_EQ(boost::text::prev_sentence_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_sentence_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_sentence_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_sentence_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_sentence_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_sentence_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_sentence_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_sentence_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_sentence_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_sentence_break(cps) - cps.begin(), 2);
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
        auto const all_sentences = boost::text::sentences(cps.begin(), cps.end());

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
    }
}

TEST(break_apis, sentence_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // ÷ 5B57 × 3002 ÷ 5B83 ÷	
    // ÷ [0.2] CJK UNIFIED IDEOGRAPH-5B57 (OLetter) × [998.0] IDEOGRAPHIC FULL STOP (STerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    boost::text::string s;
    {
        std::array<uint32_t, 3> cps = {{0x5b57, 0x3002, 0x5b83}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_sentence_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_sentence_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_sentence_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_sentence_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_sentence_break(begin, two, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_sentence_break(two, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_sentence_break(begin, three, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_sentence_break(two, end)), 3);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_sentence_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_sentence_break(cp_range)), 2);
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

    {
        auto const all_sentences = boost::text::sentences(begin, end);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(std::distance(begin, sentence.begin()), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, sentence.end()), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_sentences = boost::text::sentences(cp_range);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(std::distance(begin, sentence.begin()), sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, sentence.end()), sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, line_break)
{
    // × 200B × 0020 ÷ 0030 ÷	
    // × [0.3] ZERO WIDTH SPACE (ZW) × [7.01] SPACE (SP) ÷ [8.0] DIGIT ZERO (NU) ÷ [0.3]
    std::array<uint32_t, 3> cps = {{0x200b, 0x20, 0x30}};

    {
        EXPECT_EQ(boost::text::prev_possible_line_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_possible_line_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_possible_line_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_possible_line_break(cps.begin() + 0, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::prev_possible_line_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_possible_line_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_possible_line_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 2);
        EXPECT_EQ(boost::text::next_possible_line_break(cps.begin() + 2, cps.end()) - cps.begin(), 3);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_possible_line_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_possible_line_break(cps) - cps.begin(), 2);
    }

    {
        auto const range =
            boost::text::possible_line(cps.begin(), cps.begin() + 0, cps.end());
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }
    // Range API
    {
        auto const range = boost::text::possible_line(cps, cps.begin() + 0);
        EXPECT_EQ(range.begin() - cps.begin(), 0);
        EXPECT_EQ(range.end() - cps.begin(), 2);
    }

    {
        auto const all_lines = boost::text::possible_lines(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 4> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_lines = boost::text::possible_lines(cps);

        std::array<std::pair<int, int>, 4> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(line.begin() - cps.begin(), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(line.end() - cps.begin(), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, line_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // × 200B × 0020 ÷ 0030 ÷	
    // × [0.3] ZERO WIDTH SPACE (ZW) × [7.01] SPACE (SP) ÷ [8.0] DIGIT ZERO (NU) ÷ [0.3]
    boost::text::string s;
    {
        std::array<uint32_t, 3> cps = {{0x200b, 0x20, 0x30}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_possible_line_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_possible_line_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_possible_line_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_possible_line_break(begin, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::prev_possible_line_break(begin, two, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_possible_line_break(two, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_possible_line_break(begin, three, end)), 2);
        EXPECT_EQ(std::distance(begin, boost::text::next_possible_line_break(two, end)), 3);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_possible_line_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_possible_line_break(cp_range)), 2);
    }

    {
        auto const range = boost::text::possible_line(begin, begin, end);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }
    // Range API
    {
        auto const range = boost::text::possible_line(cp_range, begin);
        EXPECT_EQ(std::distance(begin, range.begin()), 0);
        EXPECT_EQ(std::distance(begin, range.end()), 2);
    }

    {
        auto const all_lines = boost::text::possible_lines(begin, end);

        std::array<std::pair<int, int>, 4> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_lines = boost::text::possible_lines(cp_range);

        std::array<std::pair<int, int>, 4> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(begin, line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, paragraph_break)
{
    // ÷ 0061 × 000D × 000A ÷ 002E × 000A ÷ 0061 ÷
    std::array<uint32_t, 6> cps = {{0x61, 0xd, 0xa, 0x2e, 0xa, 0x61}};

    {
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 0, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 0, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 1, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 0, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 2, cps.end()) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 3, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 4, cps.end()) - cps.begin(), 3);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 3, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 5, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 5, cps.end()) - cps.begin(), 6);
        EXPECT_EQ(boost::text::prev_paragraph_break(cps.begin(), cps.begin() + 6, cps.end()) - cps.begin(), 5);
        EXPECT_EQ(boost::text::next_paragraph_break(cps.begin() + 5, cps.end()) - cps.begin(), 6);
    }
    // Range API
    {
        EXPECT_EQ(boost::text::prev_paragraph_break(cps, cps.begin() + 0) - cps.begin(), 0);
        EXPECT_EQ(boost::text::next_paragraph_break(cps) - cps.begin(), 3);
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
        auto const all_paragraphs = boost::text::paragraphs(cps.begin(), cps.end());

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 7}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_paragraphs = boost::text::paragraphs(cps);

        std::array<std::pair<int, int>, 3> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 7}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(paragraph.begin() - cps.begin(), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(paragraph.end() - cps.begin(), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}

TEST(break_apis, paragraph_break_sentinel)
{
    using u32_iter =
        boost::text::utf8::to_utf32_iterator<char const *, char const *>;

    // ÷ 0061 × 000D × 000A ÷ 002E × 000A ÷ 0061 ÷
    boost::text::string s;
    {
        std::array<uint32_t, 6> cps = {{0x61, 0xd, 0xa, 0x2e, 0xa, 0x61}};
        s = boost::text::string(
            boost::text::utf8::make_from_utf32_iterator(cps.begin()),
            boost::text::utf8::make_from_utf32_iterator(cps.end()));
        assert(std::equal(
            cps.begin(),
            cps.end(),
            u32_iter(s.begin(), s.begin(), s.end()),
            u32_iter(s.begin(), s.end(), s.end())));
    }

    char const * c_str = s.begin();

    boost::text::cp_range<u32_iter, boost::text::utf8::null_sentinel> cp_range{
        u32_iter(c_str, c_str, s.end()), boost::text::utf8::null_sentinel{}};

    auto const begin = cp_range.begin();
    auto const one = std::next(begin);
    auto const two = std::next(one);
    auto const three = std::next(two);
    auto const four = std::next(three);
    auto const five = std::next(four);
    auto const six = std::next(five);
    auto const end = cp_range.end();

    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, begin, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(begin, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, one, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(begin, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, two, end)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(begin, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, three, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(three, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, four, end)), 3);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(three, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, five, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(five, end)), 6);
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(begin, six, end)), 5);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(five, end)), 6);
    }
    // Range API
    {
        EXPECT_EQ(std::distance(begin, boost::text::prev_paragraph_break(cp_range, begin)), 0);
        EXPECT_EQ(std::distance(begin, boost::text::next_paragraph_break(cp_range)), 3);
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

    {
        auto const all_paragraphs = boost::text::paragraphs(begin, end);

        std::array<std::pair<int, int>, 4> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 7}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(std::distance(begin, paragraph.begin()), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, paragraph.end()), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
    // Range API
    {
        auto const all_paragraphs = boost::text::paragraphs(cp_range);

        std::array<std::pair<int, int>, 4> const paragraph_bounds = {
            {{0, 3}, {3, 5}, {5, 7}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(std::distance(begin, paragraph.begin()), paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(begin, paragraph.end()), paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
    }
}
