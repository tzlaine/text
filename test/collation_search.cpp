#include <boost/text/collation_search.hpp>
#include <boost/text/data/da.hpp>

#include <gtest/gtest.h>


using namespace boost::text;


collation_table const default_table = default_collation_table();
collation_table const danish_table =
    tailored_collation_table(data::da::standard_collation_tailoring());

void do_simple_search(
    collation_table const & table,
    utf32_range str,
    utf32_range substr,
    int expected_first,
    int expected_last,
    int line,
    collation_strength strength,
    case_level case_lvl,
    variable_weighting weighting)
{
    {
        auto const r =
            collation_search(str, substr, table, strength, case_lvl, weighting);
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    return; // TODO
    {
        auto r = collation_search(
            str,
            make_simple_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }

    // Exercise cases where !std::is_same<CPIter, Sentinel>::value:

    using sentinel_cp_range_iter =
        utf8::to_utf32_iterator<char const *, utf8::null_sentinel>;
    using sentinel_cp_range =
        cp_range<sentinel_cp_range_iter, utf8::null_sentinel>;

    sentinel_cp_range str_cp_range{
        sentinel_cp_range_iter(
            str.begin().base(), str.begin().base(), utf8::null_sentinel{}),
        utf8::null_sentinel{}};
    sentinel_cp_range substr_cp_range{sentinel_cp_range_iter(
                                          substr.begin().base(),
                                          substr.begin().base(),
                                          utf8::null_sentinel{}),
                                      utf8::null_sentinel{}};

    {
        auto const r = collation_search(
            str_cp_range,
            substr_cp_range,
            table,
            strength,
            case_lvl,
            weighting);
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str_cp_range,
            make_simple_collation_searcher(
                substr_cp_range, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str_cp_range,
            make_simple_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
}

void do_boyer_moore_search(
    collation_table const & table,
    utf32_range str,
    utf32_range substr,
    int expected_first,
    int expected_last,
    int line,
    collation_strength strength,
    case_level case_lvl,
    variable_weighting weighting)
{
    {
        auto r = collation_search(
            str,
            make_boyer_moore_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BM, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BM, line " << line;
    }

    // Exercise cases where !std::is_same<CPIter, Sentinel>::value:

    using sentinel_cp_range_iter =
        utf8::to_utf32_iterator<char const *, utf8::null_sentinel>;
    using sentinel_cp_range =
        cp_range<sentinel_cp_range_iter, utf8::null_sentinel>;

    sentinel_cp_range str_cp_range{
        sentinel_cp_range_iter(
            str.begin().base(), str.begin().base(), utf8::null_sentinel{}),
        utf8::null_sentinel{}};
    sentinel_cp_range substr_cp_range{sentinel_cp_range_iter(
                                          substr.begin().base(),
                                          substr.begin().base(),
                                          utf8::null_sentinel{}),
                                      utf8::null_sentinel{}};

    {
        auto r = collation_search(
            str_cp_range,
            make_boyer_moore_collation_searcher(
                substr_cp_range, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str_cp_range,
            make_boyer_moore_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
}

void do_boyer_moore_horspool_search(
    collation_table const & table,
    utf32_range str,
    utf32_range substr,
    int expected_first,
    int expected_last,
    int line,
    collation_strength strength,
    case_level case_lvl,
    variable_weighting weighting)
{
    {
        auto r = collation_search(
            str,
            make_boyer_moore_horspool_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(std::distance(str.begin(), r.begin()), expected_first)
            << "BMH, line " << line;
        EXPECT_EQ(std::distance(str.begin(), r.end()), expected_last)
            << "BMH, line " << line;
    }

    // Exercise cases where !std::is_same<CPIter, Sentinel>::value:

    using sentinel_cp_range_iter =
        utf8::to_utf32_iterator<char const *, utf8::null_sentinel>;
    using sentinel_cp_range =
        cp_range<sentinel_cp_range_iter, utf8::null_sentinel>;

    sentinel_cp_range str_cp_range{
        sentinel_cp_range_iter(
            str.begin().base(), str.begin().base(), utf8::null_sentinel{}),
        utf8::null_sentinel{}};
    sentinel_cp_range substr_cp_range{sentinel_cp_range_iter(
                                          substr.begin().base(),
                                          substr.begin().base(),
                                          utf8::null_sentinel{}),
                                      utf8::null_sentinel{}};

    {
        auto r = collation_search(
            str_cp_range,
            make_boyer_moore_horspool_collation_searcher(
                substr_cp_range, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
    {
        auto r = collation_search(
            str_cp_range,
            make_boyer_moore_horspool_collation_searcher(
                substr, table, strength, case_lvl, weighting));
        EXPECT_EQ(
            std::distance(str_cp_range.begin(), r.begin()), expected_first)
            << "simple, line " << line;
        EXPECT_EQ(std::distance(str_cp_range.begin(), r.end()), expected_last)
            << "simple, line " << line;
    }
}

void do_search(
    collation_table const & table,
    string const & str_,
    string const & substr_,
    int expected_first,
    int expected_last,
    int line,
    collation_strength strength = collation_strength::tertiary,
    case_level case_lvl = case_level::off,
    variable_weighting weighting = variable_weighting::non_ignorable)
{
    auto const str = utf32_range(str_);
    auto const substr = utf32_range(substr_);

    do_simple_search(
        table,
        str,
        substr,
        expected_first,
        expected_last,
        line,
        strength,
        case_lvl,
        weighting);
    return; // TODO
    do_boyer_moore_search(
        table,
        str,
        substr,
        expected_first,
        expected_last,
        line,
        strength,
        case_lvl,
        weighting);
    do_boyer_moore_horspool_search(
        table,
        str,
        substr,
        expected_first,
        expected_last,
        line,
        strength,
        case_lvl,
        weighting);
}

// Test strings cribbed from Boost.Algorithm's search tests.  Thanks, Marshall!
TEST(collation_search, default_)
{
    string const haystack_1("NOW AN FOWE\220ER ANNMAN THE ANPANMANEND");
    string const needle_1("ANPANMAN");
    string const needle_2("MAN THE");
    string const needle_3("WE\220ER");
    string const needle_4("NOW ");
    string const needle_5("NEND");
    string const needle_6("NOT FOUND");
    string const needle_7("NOT FO\340ND");

    string const haystack_2("ABC ABCDAB ABCDABCDABDE");
    string const needle_11("ABCDABD");

    string const haystack_3("abra abracad abracadabra");
    string const needle_12("abracadabra");

    auto table = default_table;

    do_search(table, haystack_1, needle_1, 26, 26 + needle_1.size(), __LINE__);
    do_search(table, haystack_1, needle_2, 18, 18 + needle_2.size(), __LINE__);
    do_search(table, haystack_1, needle_3, 9, 9 + needle_3.size(), __LINE__);
    do_search(table, haystack_1, needle_4, 0, needle_4.size(), __LINE__);
    do_search(table, haystack_1, needle_5, 33, 33 + needle_5.size(), __LINE__);
    do_search(
        table,
        haystack_1,
        needle_6,
        haystack_1.size(),
        haystack_1.size(),
        __LINE__);
    do_search(
        table,
        haystack_1,
        needle_7,
        haystack_1.size(),
        haystack_1.size(),
        __LINE__);

    do_search(
        table,
        needle_1,
        haystack_1,
        needle_1.size(),
        needle_1.size(),
        __LINE__);
    do_search(table, haystack_1, haystack_1, 0, haystack_1.size(), __LINE__);
    do_search(table, haystack_2, haystack_2, 0, haystack_2.size(), __LINE__);

    do_search(
        table, haystack_2, needle_11, 15, 15 + needle_11.size(), __LINE__);
    do_search(
        table, haystack_3, needle_12, 13, 13 + needle_12.size(), __LINE__);

    do_search(table, haystack_1, "", 0, 0, __LINE__);
    do_search(table, "", needle_1, 0, 0, __LINE__);

    {
        string const base_pairs =
            "GATACACCTACCTTCACCAGTTACTCTATGCACTAGGTGCGCCAGGCCCATGCACAAGGGCTTGAG"
            "TGGATGGGAAGGATGTGCCCTAGTGATGGCAGCATAAGCTACGCAGAGAAGTTCCAGGGCAGAGTC"
            "ACCATGACCAGGGACACATCCACGAGCACAGCCTACATGGAGCTGAGCAGCCTGAGATCTGAAGAC"
            "ACGGCCATGTATTACTGTGGGAGAGATGTCTGGAGTGGTTATTATTGCCCCGGTAATATTACTACT"
            "ACTACTACTACATGGACGTCTGGGGCAAAGGGACCACG";
        string const corpus = repeat("a", 8) + base_pairs;

        do_search(table, corpus, base_pairs, 8, corpus.size(), __LINE__);
    }
}

TEST(collation_search, danish)
{
    string const haystack_1(u8"Danish aa ");
    string const haystack_2(u8"Danish aa");
    string const haystack_3(u8"Danish a");
    string const haystack_4(u8"Danish Å ");
    string const haystack_5(u8"Danish Å");
    string const needle_1(u8"Å");
    string const needle_2(u8"aa");
    string const needle_3(u8"AA");

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
        haystack_1.size(),
        haystack_1.size(),
        __LINE__);
    do_search(table, haystack_1, needle_2, 7, 9, __LINE__);
    do_search(
        table,
        haystack_1,
        needle_3,
        haystack_1.size(),
        haystack_1.size(),
        __LINE__);

    do_search(
        table,
        haystack_2,
        needle_1,
        haystack_2.size(),
        haystack_2.size(),
        __LINE__);
    do_search(table, haystack_2, needle_2, 7, 9, __LINE__);
    do_search(
        table,
        haystack_2,
        needle_3,
        haystack_2.size(),
        haystack_2.size(),
        __LINE__);

    do_search(
        table,
        haystack_3,
        needle_1,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__);
    do_search(
        table,
        haystack_3,
        needle_2,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__);
    do_search(
        table,
        haystack_3,
        needle_3,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__);

    do_search(table, haystack_4, needle_1, 7, 8, __LINE__);
    do_search(
        table,
        haystack_4,
        needle_2,
        haystack_4.size() - 1,
        haystack_4.size() - 1,
        __LINE__);
    do_search(
        table,
        haystack_4,
        needle_3,
        haystack_4.size() - 1,
        haystack_4.size() - 1,
        __LINE__);

    do_search(table, haystack_5, needle_1, 7, 8, __LINE__);
    do_search(
        table,
        haystack_5,
        needle_2,
        haystack_5.size() - 1,
        haystack_5.size() - 1,
        __LINE__);
    do_search(
        table,
        haystack_5,
        needle_3,
        haystack_5.size() - 1,
        haystack_5.size() - 1,
        __LINE__);

    // Secondary strength

    do_search(
        table,
        haystack_1,
        needle_1,
        7,
        9,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_1,
        needle_2,
        7,
        9,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_1,
        needle_3,
        7,
        9,
        __LINE__,
        collation_strength::secondary);

    do_search(
        table,
        haystack_2,
        needle_1,
        7,
        9,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_2,
        needle_2,
        7,
        9,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_2,
        needle_3,
        7,
        9,
        __LINE__,
        collation_strength::secondary);

    do_search(
        table,
        haystack_3,
        needle_1,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_3,
        needle_2,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_3,
        needle_3,
        haystack_3.size(),
        haystack_3.size(),
        __LINE__,
        collation_strength::secondary);

    do_search(
        table,
        haystack_4,
        needle_1,
        7,
        8,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_4,
        needle_2,
        7,
        8,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_4,
        needle_3,
        7,
        8,
        __LINE__,
        collation_strength::secondary);

    do_search(
        table,
        haystack_5,
        needle_1,
        7,
        8,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_5,
        needle_2,
        7,
        8,
        __LINE__,
        collation_strength::secondary);
    do_search(
        table,
        haystack_5,
        needle_3,
        7,
        8,
        __LINE__,
        collation_strength::secondary);
}

void do_full_match_search(
    collation_table const & table,
    string const & str_1,
    string const & str_2,
    int line,
    collation_strength strength = collation_strength::tertiary,
    case_level case_lvl = case_level::off,
    variable_weighting weighting = variable_weighting::non_ignorable)
{
    utf32_range as_utf32(str_1);
    auto size = std::distance(as_utf32.begin(), as_utf32.end());
    do_search(
        table, str_1, str_2, 0, size, line, strength, case_lvl, weighting);
    return; // TODO
    as_utf32 = utf32_range(str_2);
    size = std::distance(as_utf32.begin(), as_utf32.end());
    do_search(
        table, str_2, str_1, 0, size, line, strength, case_lvl, weighting);
}

void do_full_no_match_search(
    collation_table const & table,
    string const & str_1,
    string const & str_2,
    int line,
    collation_strength strength = collation_strength::tertiary,
    case_level case_lvl = case_level::off,
    variable_weighting weighting = variable_weighting::non_ignorable)
{
    utf32_range as_utf32(str_1);
    auto size = std::distance(as_utf32.begin(), as_utf32.end());
    do_search(
        table, str_1, str_2, size, size, line, strength, case_lvl, weighting);
    return; // TODO
    as_utf32 = utf32_range(str_2);
    size = std::distance(as_utf32.begin(), as_utf32.end());
    do_search(
        table, str_2, str_1, size, size, line, strength, case_lvl, weighting);
}

TEST(collation_search, case_accents_and_punct)
{
    auto const table = default_table;

    // Ignore accents and case.
    {
        string const forms[9] = {
            u8"resume",
            u8"Resume",
            u8"RESUME",
            u8"résumé",
            u8"re\u0301sume\u0301", // same as above, decomposed
            u8"rèsumè",
            u8"re\u0300sume\u0300", // same as above, decomposed
            u8"Résumé",
            u8"RÉSUMÉ",
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
                    collation_strength::primary);
            }
        }
    }

    // Ignore accents, but consider case.
    {
        string const matchers_1[5] = {
            u8"resume",
            u8"résumé",
            u8"re\u0301sume\u0301", // same as above, decomposed
            u8"rèsumè",
            u8"re\u0300sume\u0300", // same as above, decomposed
        };

        for (int i = 0; i < 5; ++i) {
            for (int j = i; j < 5; ++j) {
                do_full_match_search(
                    table,
                    matchers_1[i],
                    matchers_1[j],
                    -(i * 10000 + j * 100),
                    collation_strength::primary,
                    case_level::on);
            }
        }

        do_full_match_search(
            table,
            u8"Resume",
            u8"Resume",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"Resume",
            u8"Résumé",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"Résumé",
            u8"Résumé",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"RESUME",
            u8"RESUME",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"RESUME",
            u8"RÉSUMÉ",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"RÉSUMÉ",
            u8"RÉSUMÉ",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"resume",
            u8"résumé",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_match_search(
            table,
            u8"resume",
            u8"re\u0301sume\u0301",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_no_match_search(
            table,
            u8"resume",
            u8"Resume",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_no_match_search(
            table,
            u8"resume",
            u8"RESUME",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_no_match_search(
            table,
            u8"résumé",
            u8"RÉSUMÉ",
            __LINE__,
            collation_strength::primary,
            case_level::on);

        do_full_no_match_search(
            table,
            u8"résumé",
            u8"RÉSUMÉ",
            __LINE__,
            collation_strength::primary,
            case_level::on);
    }

    // Consider accents, but ignore case.
    {
        do_full_match_search(
            table,
            u8"resume",
            u8"RESUME",
            __LINE__,
            collation_strength::secondary);

        do_full_match_search(
            table,
            u8"résumé",
            u8"RÉSUMÉ",
            __LINE__,
            collation_strength::secondary);

        do_full_match_search(
            table,
            u8"re\u0301sume\u0301", // same as above, decomposed
            u8"Résumé",
            __LINE__,
            collation_strength::secondary);

        do_full_no_match_search(
            table,
            u8"résumé",
            u8"rèsumè",
            __LINE__,
            collation_strength::secondary);
    }

    // Completely ignore punctuation.
    {
        do_full_match_search(
            table,
            u8"ellipsis",
            u8"ellips...is",
            __LINE__,
            collation_strength::tertiary,
            case_level::off,
            variable_weighting::shifted);

        do_full_match_search(
            table,
            u8"el...lipsis",
            u8"ellips...is",
            __LINE__,
            collation_strength::tertiary,
            case_level::off,
            variable_weighting::shifted);
    }
}

#if 0 // TODO
TEST(collation_search, grapheme_boundaries)
{
    auto const table = default_table;

    do_search(table, u8"e\u0301\u0300", u8"e\u0301", 3, 3, __LINE__);
}
#endif

#if 0 // TODO
TEST(collation_search, word_boundaries)
{
    auto const table = default_table;

}
#endif
