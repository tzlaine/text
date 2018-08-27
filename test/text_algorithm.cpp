#include <boost/text/text.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/sentence_break.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


TEST(text_algorithm, word_break)
{
    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷	
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) × [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [0.3]
    text cps("\x61\x5f\x61\x2e\x3a\x61");

    {
        EXPECT_EQ(std::distance(cps.cbegin(), boost::text::prev_word_break(cps, std::next(cps.begin(), 2))), 0);
        EXPECT_EQ(std::distance(cps.cbegin(), boost::text::next_word_break(cps, std::next(cps.begin(), 5))), 6);
    }
    {
        auto const range = boost::text::word(cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.cbegin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.cbegin(), range.end()), 3);
    }

    {
        auto const all_words = boost::text::words(cps);

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(
                std::distance(cps.cbegin(), word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.cbegin(), word.end()), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());

#if 1
        auto const all_words_reversed = boost::text::reversed_words(cps);
        i = word_bounds.size();
        for (auto word : all_words_reversed) {
            --i;
            EXPECT_EQ(
                std::distance(cps.cbegin(), word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.cbegin(), word.end()), word_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
#endif
    }
}

// TODO: sentences, lines, paragraphs, bidi, search
