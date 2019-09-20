#include <boost/text/case_mapping.hpp>
#include <boost/text/string_utility.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;

using u32_iter = utf_8_to_32_iterator<char const *, null_sentinel>;
using sentinel_cp_range_t = boost::text::cp_range<u32_iter, null_sentinel>;

void to_sentinel_cp_range(
    string & s, sentinel_cp_range_t & r, std::vector<uint32_t> cps)
{
    s = to_string(cps.begin(), cps.end());
    r = sentinel_cp_range_t{u32_iter(s.begin(), s.begin(), null_sentinel{}),
                            null_sentinel{}};
}


TEST(case_mapping_api, all)
{
    // Taken from case_mapping.cpp case 000.

    // 00DF; 00DF; 0053 0073; 0053 0053; # LATIN SMALL LETTER SHARP S
    string cp_;
    sentinel_cp_range_t cp;
    to_sentinel_cp_range(cp_, cp, {0x00DF});

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
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF});
        std::vector<uint32_t> title({0x0053, 0x0073});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title({0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title({0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0020, 0x0020, 0x0020, 0x00DF});
        std::vector<uint32_t> title({0x0020, 0x0020, 0x0020, 0x0053, 0x0073});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0020, 0x0020, 0x0020, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title(
            {0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0020, 0x0020, 0x0020, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title(
            {0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0061, 0x0061, 0x0061, 0x00DF});
        std::vector<uint32_t> title({0x0041, 0x0061, 0x0061, 0x00DF});
        std::vector<uint32_t> result;

        EXPECT_FALSE(is_lower(title));
        EXPECT_TRUE(is_title(title));
        EXPECT_FALSE(is_upper(title));

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0061, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title(
            {0x0041, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(
            from_,
            from,
            {0x0061, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title(
            {0x0041, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
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

TEST(case_mapping_api, dutch_special_casing)
{
    char const input_ascii[] = "ijssel iJssel Ijssel IJMUIDEN";
    std::vector<uint32_t> const input(
        std::begin(input_ascii), std::end(input_ascii) - 1);

    // lower
    {
        char const default_expected_ascii[] = "ijssel ijssel ijssel ijmuiden";
        std::vector<uint32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char const dutch_expected_ascii[] = "ijssel ijssel ijssel ijmuiden";
        std::vector<uint32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<uint32_t> result;
            to_lower(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<uint32_t> result;
            to_lower(input, std::back_inserter(result), case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }

    // title
    {
        char const default_expected_ascii[] = "Ijssel Ijssel Ijssel Ijmuiden";
        std::vector<uint32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char const dutch_expected_ascii[] = "IJssel IJssel IJssel IJmuiden";
        std::vector<uint32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<uint32_t> result;
            to_title(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<uint32_t> result;
            to_title(input, std::back_inserter(result), case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }

    // upper
    {
        char const default_expected_ascii[] = "IJSSEL IJSSEL IJSSEL IJMUIDEN";
        std::vector<uint32_t> const default_expected(
            std::begin(default_expected_ascii),
            std::end(default_expected_ascii) - 1);

        char const dutch_expected_ascii[] = "IJSSEL IJSSEL IJSSEL IJMUIDEN";
        std::vector<uint32_t> const dutch_expected(
            std::begin(dutch_expected_ascii),
            std::end(dutch_expected_ascii) - 1);

        {
            std::vector<uint32_t> result;
            to_upper(input, std::back_inserter(result));
            EXPECT_EQ(result, default_expected);
        }
        {
            std::vector<uint32_t> result;
            to_upper(input, std::back_inserter(result), case_language::dutch);
            EXPECT_EQ(result, dutch_expected);
        }
    }
}

TEST(case_mapping_api, greek_special_casing)
{
    {
        string const from = u8"άδικος, κείμενο, ίριδα";
        string const to = u8"ΑΔΙΚΟΣ, ΚΕΙΜΕΝΟ, ΙΡΙΔΑ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Πατάτα";
        string const to = u8"ΠΑΤΑΤΑ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Αέρας, Μυστήριο, Ωραίο";
        string const to = u8"ΑΕΡΑΣ, ΜΥΣΤΗΡΙΟ, ΩΡΑΙΟ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Μαΐου, Πόρος, Ρύθμιση";
        string const to = u8"ΜΑΪΟΥ, ΠΟΡΟΣ, ΡΥΘΜΙΣΗ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"ΰ, Τηρώ, Μάιος";
        string const to = u8"Ϋ, ΤΗΡΩ, ΜΑΪΟΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"άυλος";
        string const to = u8"ΑΫΛΟΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"ΑΫΛΟΣ";
        string const to = u8"ΑΫΛΟΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Άκλιτα ρήματα ή άκλιτες μετοχές";
        string const to = u8"ΑΚΛΙΤΑ ΡΗΜΑΤΑ Ή ΑΚΛΙΤΕΣ ΜΕΤΟΧΕΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Επειδή η αναγνώριση της αξιοπρέπειας";
        string const to = u8"ΕΠΕΙΔΗ Η ΑΝΑΓΝΩΡΙΣΗ ΤΗΣ ΑΞΙΟΠΡΕΠΕΙΑΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"νομικού ή διεθνούς";
        string const to = u8"ΝΟΜΙΚΟΥ Ή ΔΙΕΘΝΟΥΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Ἐπειδὴ ἡ ἀναγνώριση";
        string const to = u8"ΕΠΕΙΔΗ Η ΑΝΑΓΝΩΡΙΣΗ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"νομικοῦ ἢ διεθνοῦς";
        string const to = u8"ΝΟΜΙΚΟΥ Ή ΔΙΕΘΝΟΥΣ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Νέο, Δημιουργία";
        string const to = u8"ΝΕΟ, ΔΗΜΙΟΥΡΓΙΑ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Ελάτε να φάτε τα καλύτερα παϊδάκια!";
        string const to = u8"ΕΛΑΤΕ ΝΑ ΦΑΤΕ ΤΑ ΚΑΛΥΤΕΡΑ ΠΑΪΔΑΚΙΑ!";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Μαΐου, τρόλεϊ";
        string const to = u8"ΜΑΪΟΥ, ΤΡΟΛΕΪ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"Το ένα ή το άλλο.";
        string const to = u8"ΤΟ ΕΝΑ Ή ΤΟ ΑΛΛΟ.";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }

    {
        string const from = u8"ρωμέικα";
        string const to = u8"ΡΩΜΕΪΚΑ";
        string result;
        to_upper(utf32_range(from), utf_32_to_8_inserter(result, result.end()));
        EXPECT_EQ(result, to) << result << " != " << to;
    }
}
