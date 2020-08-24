// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/text.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/sentence_break.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>
#include <boost/text/bidirectional.hpp>
#include <boost/text/collation_search.hpp>
#include <boost/text/data/da.hpp>
#include <boost/text/case_mapping.hpp>
#include <boost/text/in_place_case_mapping.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


TEST(text_algorithm, word_break)
{
    // ÷ 0061 × 005F × 0061 ÷ 002E ÷ 003A ÷ 0061 ÷	
    // ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [13.1] LOW LINE (ExtendNumLet) × [13.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [0.3]
    text cps("\x61\x5f\x61\x2e\x3a\x61");

    {
        EXPECT_EQ(std::distance(cps.begin(), prev_word_break(cps, std::next(cps.begin(), 2))), 0);
        EXPECT_EQ(std::distance(cps.begin(), next_word_break(cps, std::next(cps.begin(), 5))), 6);
    }
    {
        auto const range = word(cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.begin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.begin(), range.end()), 3);
    }

    {
        auto const all_words = words(cps);

        std::array<std::pair<int, int>, 4> const word_bounds = {
            {{0, 3}, {3, 4}, {4, 5}, {5, 6}}};

        int i = 0;
        for (auto word : all_words) {
            EXPECT_EQ(
                std::distance(cps.begin(), word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), word.end()), word_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)word_bounds.size());

        auto const all_words_reversed = reversed_words(cps);
        i = word_bounds.size();
        for (auto word : all_words_reversed) {
            --i;
            EXPECT_EQ(
                std::distance(cps.begin(), word.begin()), word_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), word.end()), word_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(text_algorithm, sentence_break)
{
    // ÷ 0065 × 0074 × 0063 × 002E ÷ 5B83 ÷
    // ÷ [0.2] LATIN SMALL LETTER E (Lower) × [998.0] LATIN SMALL LETTER T (Lower) × [998.0] LATIN SMALL LETTER C (Lower) × [998.0] FULL STOP (ATerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    text cps("\x65\x74\x63\x2e\xe5\xae\x83");

    {
        EXPECT_EQ(std::distance(cps.begin(), prev_sentence_break(cps, std::next(cps.begin(), 2))), 0);
        EXPECT_EQ(std::distance(cps.begin(), next_sentence_break(cps, std::next(cps.begin(), 0))), 4);
    }
    {
        auto const range =
            sentence(cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.begin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.begin(), range.end()), 4);
    }

    {
        auto const all_sentences = sentences(cps);

        std::array<std::pair<int, int>, 2> const sentence_bounds = {
            {{0, 4}, {4, 5}}};

        int i = 0;
        for (auto sentence : all_sentences) {
            EXPECT_EQ(
                std::distance(cps.begin(), sentence.begin()),
                sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), sentence.end()),
                sentence_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)sentence_bounds.size());

        auto const all_sentences_reversed = reversed_sentences(cps);
        i = sentence_bounds.size();
        for (auto sentence : all_sentences_reversed) {
            --i;
            EXPECT_EQ(
                std::distance(cps.begin(), sentence.begin()),
                sentence_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), sentence.end()),
                sentence_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
}

TEST(break_apis, line_break)
{
    // × 200B × 0020 ÷ 0030 ÷
    // × [0.3] ZERO WIDTH SPACE (ZW) × [7.01] SPACE (SP) ÷ [8.0] DIGIT ZERO (NU) ÷ [0.3]
    text cps("\xe2\x80\x8b\x20\x30");

    auto const begin = cps.begin();

    {
        auto const prev =
            prev_hard_line_break(cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.begin(), prev), 0);

        auto const next =
            next_hard_line_break(cps, std::next(cps.begin(), 0));
        EXPECT_EQ(std::distance(cps.begin(), next), 3);
    }
    {
        auto const prev = prev_allowed_line_break(
            cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.begin(), prev.iter), 0);
        EXPECT_FALSE(prev.hard_break);

        auto const next = next_allowed_line_break(
            cps, std::next(cps.begin(), 0));
        EXPECT_EQ(std::distance(cps.begin(), next.iter), 2);
        EXPECT_FALSE(next.hard_break);
    }

    {
        auto const range = line(cps, begin);
        EXPECT_EQ(std::distance(cps.begin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.begin(), range.end()), 3);
    }
    {
        auto const range = allowed_line(cps, begin);
        EXPECT_EQ(std::distance(cps.begin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.begin(), range.end()), 2);
        EXPECT_FALSE(range.hard_break());
    }

    {
        auto const all_lines = lines(cps);

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed = reversed_lines(cps);
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }

    {
        auto const all_lines = allowed_lines(cps);

        std::array<std::pair<int, int>, 2> const line_bounds = {
            {{0, 2}, {2, 3}}};

        int i = 0;
        for (auto line : all_lines) {
            EXPECT_EQ(std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            EXPECT_EQ(line.hard_break(), false) << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());

        auto const all_lines_reversed =
            reversed_allowed_lines(cps);
        i = line_bounds.size();
        for (auto line : all_lines_reversed) {
            --i;
            EXPECT_EQ(std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            EXPECT_EQ(line.hard_break(), false);
        }
        EXPECT_EQ(i, 0);
    }

    // 80 columns -> don't take the allowed break in the middle.
    {
        auto const _80_column_lines = lines(
            cps,
            80,
            [](text::const_iterator::iterator_type it,
               text::const_iterator::iterator_type last) {
                return distance(it, last);
            });

        std::array<std::pair<int, int>, 1> const line_bounds = {{{0, 3}}};

        int i = 0;
        for (auto line : _80_column_lines) {
            EXPECT_EQ(std::distance(cps.begin(), line.begin()), line_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(std::distance(cps.begin(), line.end()), line_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)line_bounds.size());
    }
}

TEST(text_algorithm, paragraph_break)
{
    // ÷ 0065 × 0074 × 0063 × 002E ÷ 5B83 ÷
    // ÷ [0.2] LATIN SMALL LETTER E (Lower) × [998.0] LATIN SMALL LETTER T (Lower) × [998.0] LATIN SMALL LETTER C (Lower) × [998.0] FULL STOP (ATerm) ÷ [11.0] CJK UNIFIED IDEOGRAPH-5B83 (OLetter) ÷ [0.3]
    text cps("\x65\x74\x63\x2e\xe5\xae\x83");

    {
        EXPECT_EQ(std::distance(cps.begin(), prev_paragraph_break(cps, std::next(cps.begin(), 2))), 0);
        EXPECT_EQ(std::distance(cps.begin(), next_paragraph_break(cps, std::next(cps.begin(), 0))), 5);
    }
    {
        auto const range =
            paragraph(cps, std::next(cps.begin(), 1));
        EXPECT_EQ(std::distance(cps.begin(), range.begin()), 0);
        EXPECT_EQ(std::distance(cps.begin(), range.end()), 5);
    }

    {
        auto const all_paragraphs = paragraphs(cps);

        std::array<std::pair<int, int>, 1> const paragraph_bounds = {{{0, 5}}};

        int i = 0;
        for (auto paragraph : all_paragraphs) {
            EXPECT_EQ(
                std::distance(cps.begin(), paragraph.begin()),
                paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), paragraph.end()),
                paragraph_bounds[i].second)
                << "i=" << i;
            ++i;
        }
        EXPECT_EQ(i, (int)paragraph_bounds.size());

        auto const all_paragraphs_reversed = reversed_paragraphs(cps);
        i = paragraph_bounds.size();
        for (auto paragraph : all_paragraphs_reversed) {
            --i;
            EXPECT_EQ(
                std::distance(cps.begin(), paragraph.begin()),
                paragraph_bounds[i].first)
                << "i=" << i;
            EXPECT_EQ(
                std::distance(cps.begin(), paragraph.end()),
                paragraph_bounds[i].second)
                << "i=" << i;
        }
        EXPECT_EQ(i, 0);
    }
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

TEST(text_algorithm, bidi)
{
    // ON RLE ON FSI ON R RLO L PDF ON PDI ON PDF ON; 3 ('LTR') (line 496999)
    std::vector<uint32_t> const cp_vec = { 0x0021, 0x202B, 0x0021, 0x2068, 0x0021, 0x05BE, 0x202E, 0x0041, 0x202C, 0x0021, 0x2069, 0x0021, 0x202C, 0x0021 };
    text cps(to_string(cp_vec.begin(), cp_vec.end()));
    std::vector<uint32_t> const expected_reordered_indices = { 0, 11, 10, 9, 7, 5, 4, 3, 2, 13 };

    {
        std::vector<uint32_t> reordered;
        for (auto subrange : bidirectional_subranges(cps, 0)) {
            for (auto grapheme : subrange) {
                for (auto cp : grapheme) {
                    reordered.push_back(cp);
                }
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cp_vec[idx] < 0x2066 || 0x2069 < cp_vec[idx]) {
                EXPECT_EQ(reordered[i], cp_vec[idx])
                    << std::hex
                    << " 0x" << reordered[i]
                    << " 0x" << cp_vec[idx]
                    << std::dec << " i=" << i;
            }
            ++i;
        }
        EXPECT_EQ(i, (int)reordered.size());
    }

    {
        std::vector<uint32_t> reordered;
        for (auto subrange : bidirectional_subranges(cps)) {
            for (auto grapheme : subrange) {
                for (auto cp : grapheme) {
                    reordered.push_back(cp);
                }
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cp_vec[idx] < 0x2066 || 0x2069 < cp_vec[idx]) {
                EXPECT_EQ(reordered[i], cp_vec[idx])
                    << std::hex
                    << " 0x" << reordered[i]
                    << " 0x" << cp_vec[idx]
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
        for (auto subrange : bidirectional_subranges(
                 cps, 80, bidi_stateful_cp_extent{}, 0)) {
            for (auto grapheme : subrange) {
                for (auto cp : grapheme) {
                    reordered.push_back(cp);
                }
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cp_vec[idx] < 0x2066 || 0x2069 < cp_vec[idx]) {
                EXPECT_EQ(reordered[i], cp_vec[idx])
                    << std::hex
                    << " 0x" << reordered[i]
                    << " 0x" << cp_vec[idx]
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
        for (auto subrange : bidirectional_subranges(
                 cps, 80, bidi_stateful_cp_extent{})) {
            for (auto grapheme : subrange) {
                for (auto cp : grapheme) {
                    reordered.push_back(cp);
                }
            }
        }
        int i = 0;
        for (int idx : expected_reordered_indices) {
            if (cp_vec[idx] < 0x2066 || 0x2069 < cp_vec[idx]) {
                EXPECT_EQ(reordered[i], cp_vec[idx])
                    << std::hex
                    << " 0x" << reordered[i]
                    << " 0x" << cp_vec[idx]
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

collation_table const default_table = default_collation_table();
collation_table const danish_table =
    tailored_collation_table(data::da::standard_collation_tailoring());

void do_simple_search(
    collation_table const & table,
    text str,
    text substr,
    int expected_first,
    int expected_last,
    int line,
    collation_flags flags)
{
    {
        auto const r = collation_search(str, substr, table, flags);
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str, make_simple_collation_searcher(substr, table, flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
}

void do_boyer_moore_search(
    collation_table const & table,
    text str,
    text substr,
    int expected_first,
    int expected_last,
    int line,
    collation_flags flags)
{
    {
        auto r = collation_search(
            str, make_boyer_moore_collation_searcher(substr, table, flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BM, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BM, line " << line;
    }
    {
        auto r = collation_search(
            str,
            make_boyer_moore_collation_searcher(
                substr,
                detail::coll_search_prev_grapheme_callable{},
                table,
                flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BM, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BM, line " << line;
    }
}

void do_boyer_moore_horspool_search(
    collation_table const & table,
    text str,
    text substr,
    int expected_first,
    int expected_last,
    int line,
    collation_flags flags)
{
    {
        auto r = collation_search(
            str,
            make_boyer_moore_horspool_collation_searcher(substr, table, flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BMH, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BMH, line " << line;
    }
    {
        auto r = collation_search(
            str,
            make_boyer_moore_horspool_collation_searcher(
                substr,
                detail::coll_search_prev_grapheme_callable{},
                table,
                flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BMH, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BMH, line " << line;
    }
}

void do_search(
    collation_table const & table,
    text str,
    text substr,
    int expected_first,
    int expected_last,
    int line,
    collation_flags flags = collation_flags::none)
{
    do_simple_search(
        table, str, substr, expected_first, expected_last, line, flags);
    do_boyer_moore_search(
        table, str, substr, expected_first, expected_last, line, flags);
    do_boyer_moore_horspool_search(
        table, str, substr, expected_first, expected_last, line, flags);
}

// Test strings cribbed from Boost.Algorithm's search tests.  Thanks, Marshall!
TEST(text_algorithm, coll_search_default)
{
    text const haystack_1("NOW AN FOWE\220ER ANNMAN THE ANPANMANEND");
    text const needle_1("ANPANMAN");
    text const needle_2("MAN THE");
    text const needle_3("WE\220ER");
    text const needle_4("NOW ");
    text const needle_5("NEND");
    text const needle_6("NOT FOUND");
    text const needle_7("NOT FO\340ND");

    text const haystack_2("ABC ABCDAB ABCDABCDABDE");
    text const needle_11("ABCDABD");

    text const haystack_3("abra abracad abracadabra");
    text const needle_12("abracadabra");

    auto table = default_table;

    do_search(table, haystack_1, needle_1, 26, 26 + needle_1.distance(), __LINE__);
    do_search(table, haystack_1, needle_2, 18, 18 + needle_2.distance(), __LINE__);
    do_search(table, haystack_1, needle_3, 9, 9 + needle_3.distance(), __LINE__);
    do_search(table, haystack_1, needle_4, 0, needle_4.distance(), __LINE__);
    do_search(table, haystack_1, needle_5, 33, 33 + needle_5.distance(), __LINE__);
    do_search(
        table,
        haystack_1,
        needle_6,
        haystack_1.distance(),
        haystack_1.distance(),
        __LINE__);
    do_search(
        table,
        haystack_1,
        needle_7,
        haystack_1.distance(),
        haystack_1.distance(),
        __LINE__);

    do_search(
        table,
        needle_1,
        haystack_1,
        needle_1.distance(),
        needle_1.distance(),
        __LINE__);
    do_search(table, haystack_1, haystack_1, 0, haystack_1.distance(), __LINE__);
    do_search(table, haystack_2, haystack_2, 0, haystack_2.distance(), __LINE__);

    do_search(
        table, haystack_2, needle_11, 15, 15 + needle_11.distance(), __LINE__);
    do_search(
        table, haystack_3, needle_12, 13, 13 + needle_12.distance(), __LINE__);

    do_search(table, haystack_1, "", 0, 0, __LINE__);
    do_search(table, "", needle_1, 0, 0, __LINE__);

    {
        text const base_pairs =
            "GATACACCTACCTTCACCAGTTACTCTATGCACTAGGTGCGCCAGGCCCATGCACAAGGGCTTGAG"
            "TGGATGGGAAGGATGTGCCCTAGTGATGGCAGCATAAGCTACGCAGAGAAGTTCCAGGGCAGAGTC"
            "ACCATGACCAGGGACACATCCACGAGCACAGCCTACATGGAGCTGAGCAGCCTGAGATCTGAAGAC"
            "ACGGCCATGTATTACTGTGGGAGAGATGTCTGGAGTGGTTATTATTGCCCCGGTAATATTACTACT"
            "ACTACTACTACATGGACGTCTGGGGCAAAGGGACCACG";
        text const corpus = "aaaaaaaa" + base_pairs;

        do_search(table, corpus, base_pairs, 8, corpus.distance(), __LINE__);
    }
}

TEST(text_algorithm, coll_search_danish)
{
    text const haystack_1((char const *)u8"Danish aa ");
    text const haystack_2((char const *)u8"Danish aa");
    text const haystack_3((char const *)u8"Danish a");
    text const haystack_4((char const *)u8"Danish Å ");
    text const haystack_5((char const *)u8"Danish Å");
    text const needle_1((char const *)u8"Å");
    text const needle_2((char const *)u8"aa");
    text const needle_3((char const *)u8"AA");

    auto table = danish_table;

    // The Danish collation includes this line:
    // &[before 1]ǀ<æ<<<Æ<<ä<<<Ä<ø<<<Ø<<ö<<<Ö<<ő<<<Ő<å<<<Å<<<aa<<<Aa<<<AA
    // This implies that we should expect no non-identical matches with the
    // default collation strength (tertiary), but should expect non-identical
    // matches (e.g. AA and aa) at secondary strength.

    // Tertiary strength

    do_search(
        table,
        haystack_1,
        needle_1,
        haystack_1.storage_code_units(),
        haystack_1.storage_code_units(),
        __LINE__);
    do_search(table, haystack_1, needle_2, 7, 9, __LINE__);
    do_search(
        table,
        haystack_1,
        needle_3,
        haystack_1.storage_code_units(),
        haystack_1.storage_code_units(),
        __LINE__);

    do_search(
        table,
        haystack_2,
        needle_1,
        haystack_2.storage_code_units(),
        haystack_2.storage_code_units(),
        __LINE__);
    do_search(table, haystack_2, needle_2, 7, 9, __LINE__);
    do_search(
        table,
        haystack_2,
        needle_3,
        haystack_2.storage_code_units(),
        haystack_2.storage_code_units(),
        __LINE__);

    do_search(
        table,
        haystack_3,
        needle_1,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__);
    do_search(
        table,
        haystack_3,
        needle_2,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__);
    do_search(
        table,
        haystack_3,
        needle_3,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__);

    do_search(table, haystack_4, needle_1, 7, 8, __LINE__);
    do_search(
        table,
        haystack_4,
        needle_2,
        haystack_4.storage_code_units() - 1,
        haystack_4.storage_code_units() - 1,
        __LINE__);
    do_search(
        table,
        haystack_4,
        needle_3,
        haystack_4.storage_code_units() - 1,
        haystack_4.storage_code_units() - 1,
        __LINE__);

    do_search(table, haystack_5, needle_1, 7, 8, __LINE__);
    do_search(
        table,
        haystack_5,
        needle_2,
        haystack_5.storage_code_units() - 1,
        haystack_5.storage_code_units() - 1,
        __LINE__);
    do_search(
        table,
        haystack_5,
        needle_3,
        haystack_5.storage_code_units() - 1,
        haystack_5.storage_code_units() - 1,
        __LINE__);

    // Secondary strength

    do_search(
        table,
        haystack_1,
        needle_1,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_1,
        needle_2,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_1,
        needle_3,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);

    do_search(
        table,
        haystack_2,
        needle_1,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_2,
        needle_2,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_2,
        needle_3,
        7,
        9,
        __LINE__,
        collation_flags::ignore_case);

    do_search(
        table,
        haystack_3,
        needle_1,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_3,
        needle_2,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_3,
        needle_3,
        haystack_3.storage_code_units(),
        haystack_3.storage_code_units(),
        __LINE__,
        collation_flags::ignore_case);

    do_search(
        table,
        haystack_4,
        needle_1,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_4,
        needle_2,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_4,
        needle_3,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);

    do_search(
        table,
        haystack_5,
        needle_1,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_5,
        needle_2,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);
    do_search(
        table,
        haystack_5,
        needle_3,
        7,
        8,
        __LINE__,
        collation_flags::ignore_case);
}

void do_full_match_search(
    collation_table const & table,
    text str_1,
    text str_2,
    int line,
    collation_flags flags = collation_flags::none)
{
    auto size = std::distance(str_1.begin(), str_1.end());
    do_search(table, str_1, str_2, 0, size, line, flags);
    size = std::distance(str_2.begin(), str_2.end());
    do_search(table, str_2, str_1, 0, size, line, flags);
}

void do_full_no_match_search(
    collation_table const & table,
    text str_1,
    text str_2,
    int line,
    collation_flags flags = collation_flags::none)
{
    auto size = std::distance(str_1.begin(), str_1.end());
    do_search(table, str_1, str_2, size, size, line, flags);
    size = std::distance(str_2.begin(), str_2.end());
    do_search(table, str_2, str_1, size, size, line, flags);
}

TEST(text_algorithm, coll_search_case_accents_and_punct)
{
    auto const table = default_table;

    // Ignore accents and case.
    {
        text const forms[9] = {
            (char const *)u8"resume",
            (char const *)u8"Resume",
            (char const *)u8"RESUME",
            (char const *)u8"résumé",
            (char const *)u8"re\u0301sume\u0301", // same as above, decomposed
            (char const *)u8"rèsumè",
            (char const *)u8"re\u0300sume\u0300", // same as above, decomposed
            (char const *)u8"Résumé",
            (char const *)u8"RÉSUMÉ",
        };

        // At primary strength (ignore accents and case), all the above should
        // match each other, and those matches should be symmetric.
        for (int i = 0; i < 9; ++i) {
            for (int j = i; j < 9; ++j) {
                do_full_match_search(
                    table,
                    forms[i],
                    forms[j],
                    -(i * 10000 + j * 100),
                    collation_flags::ignore_accents |
                        collation_flags::ignore_case);
            }
        }
    }

    // Ignore accents, but consider case.
    {
        text const matchers_1[5] = {
            (char const *)u8"resume",
            (char const *)u8"résumé",
            (char const *)u8"re\u0301sume\u0301", // same as above, decomposed
            (char const *)u8"rèsumè",
            (char const *)u8"re\u0300sume\u0300", // same as above, decomposed
        };

        for (int i = 0; i < 5; ++i) {
            for (int j = i; j < 5; ++j) {
                do_full_match_search(
                    table,
                    matchers_1[i],
                    matchers_1[j],
                    -(i * 10000 + j * 100),
                    collation_flags::ignore_accents);
            }
        }

        do_full_match_search(
            table,
            (char const *)u8"Resume",
            (char const *)u8"Resume",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"Resume",
            (char const *)u8"Résumé",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"Résumé",
            (char const *)u8"Résumé",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"RESUME",
            (char const *)u8"RESUME",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"RESUME",
            (char const *)u8"RÉSUMÉ",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"RÉSUMÉ",
            (char const *)u8"RÉSUMÉ",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"resume",
            (char const *)u8"résumé",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_match_search(
            table,
            (char const *)u8"resume",
            (char const *)u8"re\u0301sume\u0301",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_no_match_search(
            table,
            (char const *)u8"resume",
            (char const *)u8"Resume",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_no_match_search(
            table,
            (char const *)u8"resume",
            (char const *)u8"RESUME",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_no_match_search(
            table,
            (char const *)u8"résumé",
            (char const *)u8"RÉSUMÉ",
            __LINE__,
            collation_flags::ignore_accents);

        do_full_no_match_search(
            table,
            (char const *)u8"résumé",
            (char const *)u8"RÉSUMÉ",
            __LINE__,
            collation_flags::ignore_accents);
    }

    // Consider accents, but ignore case.
    {
        do_full_match_search(
            table,
            (char const *)u8"resume",
            (char const *)u8"RESUME",
            __LINE__,
            collation_flags::ignore_case);

        do_full_match_search(
            table,
            (char const *)u8"résumé",
            (char const *)u8"RÉSUMÉ",
            __LINE__,
            collation_flags::ignore_case);

        do_full_match_search(
            table,
            (char const *)u8"re\u0301sume\u0301", // same as above, decomposed
            (char const *)u8"Résumé",
            __LINE__,
            collation_flags::ignore_case);

        do_full_no_match_search(
            table,
            (char const *)u8"résumé",
            (char const *)u8"rèsumè",
            __LINE__,
            collation_flags::ignore_case);
    }

    // Completely ignore punctuation.
    {
        do_full_match_search(
            table,
            (char const *)u8"ellipsis",
            (char const *)u8"ellips...is",
            __LINE__,
            collation_flags::ignore_punctuation);

        do_full_match_search(
            table,
            (char const *)u8"el...lipsis",
            (char const *)u8"ellips...is",
            __LINE__,
            collation_flags::ignore_punctuation);
    }
}

TEST(text_algorithm, coll_search_grapheme_boundaries)
{
    auto const table = default_table;

    do_search(table, (char const *)u8"e\u0301\u0300", (char const *)u8"e\u0301\u0300", 0, 1, __LINE__);

    do_search(table, (char const *)u8"e\u0301", (char const *)u8"e\u0301\u0300", 1, 1, __LINE__);
    do_search(table, (char const *)u8"\u0301\u0300", (char const *)u8"e\u0301\u0300", 1, 1, __LINE__);
    do_search(table, (char const *)u8"e", (char const *)u8"e\u0301\u0300", 1, 1, __LINE__);
    do_search(table, (char const *)u8"\u0301", (char const *)u8"e\u0301\u0300", 1, 1, __LINE__);
    do_search(table, (char const *)u8"\u0300", (char const *)u8"e\u0301\u0300", 1, 1, __LINE__);
}

struct prev_word_callable_t
{
    template<typename CPIter, typename Sentinel>
    CPIter operator()(CPIter first, CPIter it, Sentinel last) const noexcept
    {
        return prev_word_break(first, it, last);
    }
};

void do_simple_word_search(
    collation_table const & table,
    text str,
    text substr,
    int expected_first,
    int expected_last,
    int line,
    collation_flags flags = collation_flags::none)
{
    {
        auto const r =
            collation_search(str, substr, prev_word_callable_t{}, table, flags);
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str,
            make_simple_collation_searcher(
                substr, prev_word_callable_t{}, table, flags));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
}

void do_simple_word_search_not_found(
    collation_table const & table,
    text str,
    text substr,
    int line,
    collation_flags flags = collation_flags::none)
{
    {
        auto const r =
            collation_search(str, substr, prev_word_callable_t{}, table, flags);
        EXPECT_TRUE(r.empty()) << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str,
            make_simple_collation_searcher(
                substr, prev_word_callable_t{}, table, flags));
        EXPECT_TRUE(r.empty()) << "simple, line " << line;
    }
}

TEST(text_algorithm, coll_search_word_boundaries)
{
    auto const table = default_table;

    do_simple_word_search(
        table, (char const *)u8"pause resume ...", (char const *)u8"resume", 6, 12, __LINE__);
    do_simple_word_search_not_found(table, (char const *)u8"resumed", (char const *)u8"resume", __LINE__);
    do_simple_word_search_not_found(table, (char const *)u8"unresumed", (char const *)u8"resume", __LINE__);
    do_simple_word_search_not_found(table, (char const *)u8"unresume", (char const *)u8"resume", __LINE__);
}

TEST(text_algorithm, case_mapping_)
{
    // Taken from case_mapping.cpp case 000.

    // 00DF; 00DF; 0053 0073; 0053 0053; # LATIN SMALL LETTER SHARP S
    text cp("\xc3\x9f"); // 00DF

    EXPECT_TRUE(is_lower(cp));
    EXPECT_FALSE(is_title(cp));
    EXPECT_FALSE(is_upper(cp));

    { // to_lower
        std::vector<uint32_t> lower({0x00DF});
        std::vector<uint32_t> result;

        to_lower(cp, std::back_inserter(result));
        EXPECT_EQ(result, lower);
    }
    { // to_title
        text from("\x61\x61\x61\xc3\x9f");
        std::vector<uint32_t> title({0x0041, 0x0061, 0x0061, 0x00DF});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_upper
        std::vector<uint32_t> upper({0x0053, 0x0053});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(upper));
        EXPECT_FALSE(is_title(upper));
        EXPECT_TRUE(is_upper(upper));

        to_upper(cp, std::back_inserter(result));
        EXPECT_EQ(result, upper);
    }
}

TEST(text_algorithm, in_place_case_mapping)
{
    { // to_lower
        text from("\x53\x53");
        text const lower("\x73\x73");

        EXPECT_TRUE(is_lower(lower));
        EXPECT_FALSE(is_title(lower));
        EXPECT_FALSE(is_upper(lower));

        in_place_to_lower(from);
        EXPECT_EQ(from, lower);
    }
    { // to_title
        text from("\x61\x61\x61\xc3\x9f");
        text const title("\x41\x61\x61\xc3\x9f");

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        in_place_to_title(from);
        EXPECT_EQ(from, title);
    }
    { // to_upper
        text from("\xc3\x9f");
        text const upper("\x53\x53");

        EXPECT_FALSE(is_lower(upper));
        EXPECT_FALSE(is_title(upper));
        EXPECT_TRUE(is_upper(upper));

        in_place_to_upper(from);
        EXPECT_EQ(from, upper);
    }
}
