#include <boost/text/collation_table.hpp>

#include <gtest/gtest.h>


using namespace boost::text;

TEST(detail, canonical_closure)
{
    using string_t = detail::canonical_closure_string_t;

    // Example from top of ICU's caniter.cpp
    {
        string_t const equivalent_strings[] = {
            string_t({0x0064, 0x0307, 0x0327}), // LATIN SMALL LETTER D, COMBINING DOT ABOVE, COMBINING CEDILLA
            string_t({0x0064, 0x0327, 0x0307}), // LATIN SMALL LETTER D, COMBINING CEDILLA, COMBINING DOT ABOVE
            string_t({0x1E0B, 0x0327}),         // LATIN SMALL LETTER D WITH DOT ABOVE, COMBINING CEDILLA
            string_t({0x1E11, 0x0307})          // LATIN SMALL LETTER D WITH CEDILLA, COMBINING DOT ABOVE
        };

        {
            std::vector<string_t> results;
            detail::canonical_closure(
                equivalent_strings[0].begin(),
                equivalent_strings[0].end(),
                std::back_inserter(results));
            EXPECT_EQ(results.size(), 3);
            // TODO: Check that they match the above.
        }
    }

    // Examples from https://www.unicode.org/reports/tr10/#Canonical_Equivalence
    {
        string_t const equivalent_strings[] = {
            string_t({0x212B}),                 // Å           U+212B ANGSTROM SIGN
            string_t({0x00C5}),                 // Å           U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
            string_t({0x0041, 0x030A})          // A ◌̊         U+0041 LATIN CAPITAL LETTER A, U+030A COMBINING RING ABOVE
        };

        {
            std::vector<string_t> results;
            detail::canonical_closure(
                equivalent_strings[0].begin(),
                equivalent_strings[0].end(),
                std::back_inserter(results));
            EXPECT_EQ(results.size(), 3);
            // TODO: Check that they match the above.
        }
    }

    {
        string_t const equivalent_strings[] = {
            string_t({0x0078, 0x031B, 0x0323}), // x ◌̛ ◌̣       U+0078 LATIN SMALL LETTER X, U+031B COMBINING HORN, U+0323 COMBINING DOT BELOW
            string_t({0x0078, 0x0323, 0x031B})  // x ◌̣ ◌̛       U+0078 LATIN SMALL LETTER X, U+0323 COMBINING DOT BELOW, U+031B COMBINING HORN
        };

        {
            std::vector<string_t> results;
            detail::canonical_closure(
                equivalent_strings[0].begin(),
                equivalent_strings[0].end(),
                std::back_inserter(results));
            EXPECT_EQ(results.size(), 1);
            // TODO: Check that they match the above.
        }
    }

    {
        string_t const equivalent_strings[] = {
            string_t({0x1EF1}),                 // ự           U+1EF1 LATIN SMALL LETTER U WITH HORN AND DOT BELOW
            string_t({0x1EE5, 0x031B}),         // ụ ◌̛         U+1EE5 LATIN SMALL LETTER U WITH DOT BELOW, U+031B COMBINING HORN
            string_t({0x0075, 0x031B, 0x0323}), // u ◌̛ ◌̣       U+0075 LATIN SMALL LETTER U, U+031B COMBINING HORN, U+0323 COMBINING DOT BELOW
            string_t({0x01B0, 0x0323}),         // ư ◌̣         U+01B0 LATIN SMALL LETTER U WITH HORN, U+0323 COMBINING DOT BELOW
            string_t({0x0075, 0x0323, 0x031B})  // u ◌̣ ◌̛       U+0075 LATIN SMALL LETTER U, U+0323 COMBINING DOT BELOW, U+031B COMBINING HORN
        };

        {
            std::vector<string_t> results;
            detail::canonical_closure(
                equivalent_strings[0].begin(),
                equivalent_strings[0].end(),
                std::back_inserter(results));
            EXPECT_EQ(results.size(), 4);
            // TODO: Check that they match the above.
        }
    }

    // Example from https://www.unicode.org/reports/tr10/#Avoiding_Normalization
    {
        string_t const equivalent_strings[] = {
            string_t({0x01ED}),                 // ǭ           U+01ED LATIN SMALL LETTER O WITH OGONEK AND MACRON
            string_t({0x01EB, 0x0304}),         // ǫ + ̄       U+01EB LATIN SMALL LETTER O WITH OGONEK, U+0304 COMBINING MACRON
            string_t({0x014D, 0x0328}),         // ō + ̨       U+014D LATIN SMALL LETTER O WITH MACRON, U+0328 COMBINING OGONEK
            string_t({0x006F, 0x0304, 0x0328}), // o + ̄ + ̨   U+006F LATIN SMALL LETTER O, U+0304 COMBINING MACRON, U+0328 COMBINING OGONEK
            string_t({0x006F, 0x0328, 0x0304})  // o + ̨ + ̄   U+006F LATIN SMALL LETTER O, U+0328 COMBINING OGONEK, U+0304 COMBINING MACRON
        };

        {
            std::vector<string_t> results;
            detail::canonical_closure(
                equivalent_strings[0].begin(),
                equivalent_strings[0].end(),
                std::back_inserter(results));
            EXPECT_EQ(results.size(), 4);
            // TODO: Check that they match the above.
        }
    }
}
