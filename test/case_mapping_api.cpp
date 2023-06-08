// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/case_mapping.hpp>
#include <boost/text/string_utility.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/transcode_view.hpp>

#include <gtest/gtest.h>


using namespace boost;
using namespace boost::text::detail;

using u32_iter =
    text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>;
using null_sent_cp_subrange =
    std::ranges::subrange<u32_iter, text::null_sentinel_t>;
using sentinel_cp_range_t =
    text::utf_view<text::format::utf32, null_sent_cp_subrange>;

void to_sentinel_cp_range(
    std::u8string & s, sentinel_cp_range_t & r, std::vector<char32_t> cps)
{
    auto str = boost::text::to_string(cps.begin(), cps.end());
    s = std::u8string(str.begin(), str.end());
    r = sentinel_cp_range_t{null_sent_cp_subrange(
        u32_iter(s.data(), s.data(), text::null_sentinel),
        text::null_sentinel)};
}


TEST(case_mapping_api, all)
{
    // Taken from case_mapping.cpp case 000.

    // 00DF; 00DF; 0053 0073; 0053 0053; # LATIN SMALL LETTER SHARP S
    std::u8string cp_;
    sentinel_cp_range_t cp;
    to_sentinel_cp_range(cp_, cp, {0x00DF});

    EXPECT_TRUE(text::is_lower(cp));
    EXPECT_FALSE(text::is_title(cp));
    EXPECT_FALSE(text::is_upper(cp));

    { // to_lower
        std::vector<char32_t> lower({0x00DF});
        std::vector<char32_t> result;

        to_lower(cp, std::back_inserter(result));
        EXPECT_EQ(result, lower);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF});
        std::vector<char32_t> title({0x0053, 0x0073});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> title({0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> title({0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0020, 0x0020, 0x0020, 0x00DF});
        std::vector<char32_t> title({0x0020, 0x0020, 0x0020, 0x0053, 0x0073});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0020, 0x0020, 0x0020, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> title(
            {0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0020, 0x0020, 0x0020, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> title(
            {0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0061, 0x0061, 0x0061, 0x00DF});
        std::vector<char32_t> title({0x0041, 0x0061, 0x0061, 0x00DF});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0061, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> title(
            {0x0041, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<char32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        std::u8string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0061, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> title(
            {0x0041, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(title));
        EXPECT_TRUE(text::is_title(title));
        EXPECT_FALSE(text::is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_upper
        std::vector<char32_t> upper({0x0053, 0x0053});
        std::vector<char32_t> result;

        EXPECT_FALSE(text::is_lower(upper));
        EXPECT_FALSE(text::is_title(upper));
        EXPECT_TRUE(text::is_upper(upper));

        to_upper(cp, std::back_inserter(result));
        EXPECT_EQ(result, upper);
    }
}

TEST(case_mapping_api, dutch_special_casing)
{
    char8_t const input_ascii[] = u8"ijssel iJssel Ijssel IJMUIDEN";
    std::vector<char32_t> const input(
        std::begin(input_ascii), std::end(input_ascii) - 1);

    // lower
    {
        char8_t const default_expected_ascii[] = u8"ijssel ijssel ijssel ijmuiden";
        std::vector<char32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char8_t const dutch_expected_ascii[] = u8"ijssel ijssel ijssel ijmuiden";
        std::vector<char32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<char32_t> result;
            text::to_lower(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<char32_t> result;
            text::to_lower(
                input, std::back_inserter(result), text::case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }

    // title
    {
        char8_t const default_expected_ascii[] = u8"Ijssel Ijssel Ijssel Ijmuiden";
        std::vector<char32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char8_t const dutch_expected_ascii[] = u8"IJssel IJssel IJssel IJmuiden";
        std::vector<char32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<char32_t> result;
            text::to_title(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<char32_t> result;
            text::to_title(
                input, std::back_inserter(result), text::case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }

    // upper
    {
        char8_t const default_expected_ascii[] = u8"IJSSEL IJSSEL IJSSEL IJMUIDEN";
        std::vector<char32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char8_t const dutch_expected_ascii[] = u8"IJSSEL IJSSEL IJSSEL IJMUIDEN";
        std::vector<char32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<char32_t> result;
            text::to_upper(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<char32_t> result;
            text::to_upper(
                input, std::back_inserter(result), text::case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }
}

TEST(case_mapping_api, greek_special_casing)
{
    {
        std::u8string const from = u8"άδικος, κείμενο, ίριδα";
        std::u8string const to = u8"ΑΔΙΚΟΣ, ΚΕΙΜΕΝΟ, ΙΡΙΔΑ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Πατάτα";
        std::u8string const to = u8"ΠΑΤΑΤΑ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Αέρας, Μυστήριο, Ωραίο";
        std::u8string const to = u8"ΑΕΡΑΣ, ΜΥΣΤΗΡΙΟ, ΩΡΑΙΟ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Μαΐου, Πόρος, Ρύθμιση";
        std::u8string const to = u8"ΜΑΪΟΥ, ΠΟΡΟΣ, ΡΥΘΜΙΣΗ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"ΰ, Τηρώ, Μάιος";
        std::u8string const to = u8"Ϋ, ΤΗΡΩ, ΜΑΪΟΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"άυλος";
        std::u8string const to = u8"ΑΫΛΟΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"ΑΫΛΟΣ";
        std::u8string const to = u8"ΑΫΛΟΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from =
            u8"Άκλιτα ρήματα ή άκλιτες μετοχές";
        std::u8string const to =
            u8"ΑΚΛΙΤΑ ΡΗΜΑΤΑ Ή ΑΚΛΙΤΕΣ ΜΕΤΟΧΕΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from =
            u8"Επειδή η αναγνώριση της αξιοπρέπειας";
        std::u8string const to =
            u8"ΕΠΕΙΔΗ Η ΑΝΑΓΝΩΡΙΣΗ ΤΗΣ ΑΞΙΟΠΡΕΠΕΙΑΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"νομικού ή διεθνούς";
        std::u8string const to = u8"ΝΟΜΙΚΟΥ Ή ΔΙΕΘΝΟΥΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Ἐπειδὴ ἡ ἀναγνώριση";
        std::u8string const to = u8"ΕΠΕΙΔΗ Η ΑΝΑΓΝΩΡΙΣΗ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"νομικοῦ ἢ διεθνοῦς";
        std::u8string const to = u8"ΝΟΜΙΚΟΥ Ή ΔΙΕΘΝΟΥΣ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Νέο, Δημιουργία";
        std::u8string const to = u8"ΝΕΟ, ΔΗΜΙΟΥΡΓΙΑ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from =
            u8"Ελάτε να φάτε τα καλύτερα παϊδάκια!";
        std::u8string const to =
            u8"ΕΛΑΤΕ ΝΑ ΦΑΤΕ ΤΑ ΚΑΛΥΤΕΡΑ ΠΑΪΔΑΚΙΑ!";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Μαΐου, τρόλεϊ";
        std::u8string const to = u8"ΜΑΪΟΥ, ΤΡΟΛΕΪ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"Το ένα ή το άλλο.";
        std::u8string const to = u8"ΤΟ ΕΝΑ Ή ΤΟ ΑΛΛΟ.";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }

    {
        std::u8string const from = u8"ρωμέικα";
        std::u8string const to = u8"ΡΩΜΕΪΚΑ";
        std::u8string result;
        text::to_upper(
            from | text::as_utf32,
            text::from_utf32_inserter(result, result.end()));
        EXPECT_TRUE(std::ranges::equal(result, to))
            << (result | text::as_utf8) << " != " << (to | text::as_utf8);
    }
}
