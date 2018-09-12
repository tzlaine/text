#include <boost/text/detail/canonical_closure.hpp>

#include <gtest/gtest.h>


using namespace boost::text;

using string_t = detail::canonical_closure_string_t;

void print(string_t s)
{
    for (auto cp : s) {
        std::cout << std::hex << std::setw(4) << std::setfill('0') << cp << " ";
    }
}

void print(std::vector<string_t> results)
{
    for (auto s : results) {
        print(s);
        std::cout << "\n";
    }
}

TEST(detail, canonical_closure)
{
    // Second segment from example from top of ICU's caniter.cpp
    {
        int const N = 4;

        string_t equivalent_strings[N] = {
            string_t({0x0064, 0x0307, 0x0327}), // LATIN SMALL LETTER D, COMBINING DOT ABOVE, COMBINING CEDILLA
            string_t({0x0064, 0x0327, 0x0307}), // LATIN SMALL LETTER D, COMBINING CEDILLA, COMBINING DOT ABOVE
            string_t({0x1E0B, 0x0327}),         // LATIN SMALL LETTER D WITH DOT ABOVE, COMBINING CEDILLA
            string_t({0x1E11, 0x0307})          // LATIN SMALL LETTER D WITH CEDILLA, COMBINING DOT ABOVE
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }

    // Full example from caniter.cpp
    {
        int const N = 12;

        string_t equivalent_strings[N] = {
            string_t({0x0041, 0x030A, 0x0064, 0x0307, 0x0327}), // LATIN CAPITAL LETTER A, COMBINING RING ABOVE, LATIN SMALL LETTER D, COMBINING DOT ABOVE, COMBINING CEDILLA
            string_t({0x0041, 0x030A, 0x0064, 0x0327, 0x0307}), // LATIN CAPITAL LETTER A, COMBINING RING ABOVE, LATIN SMALL LETTER D, COMBINING CEDILLA, COMBINING DOT ABOVE
            string_t({0x0041, 0x030A, 0x1E0B, 0x0327}), // LATIN CAPITAL LETTER A, COMBINING RING ABOVE, LATIN SMALL LETTER D WITH DOT ABOVE, COMBINING CEDILLA
            string_t({0x0041, 0x030A, 0x1E11, 0x0307}), // LATIN CAPITAL LETTER A, COMBINING RING ABOVE, LATIN SMALL LETTER D WITH CEDILLA, COMBINING DOT ABOVE
            string_t({0x00C5, 0x0064, 0x0307, 0x0327}), // LATIN CAPITAL LETTER A WITH RING ABOVE, LATIN SMALL LETTER D, COMBINING DOT ABOVE, COMBINING CEDILLA
            string_t({0x00C5, 0x0064, 0x0327, 0x0307}), // LATIN CAPITAL LETTER A WITH RING ABOVE, LATIN SMALL LETTER D, COMBINING CEDILLA, COMBINING DOT ABOVE
            string_t({0x00C5, 0x1E0B, 0x0327}), // LATIN CAPITAL LETTER A WITH RING ABOVE, LATIN SMALL LETTER D WITH DOT ABOVE, COMBINING CEDILLA
            string_t({0x00C5, 0x1E11, 0x0307}), // LATIN CAPITAL LETTER A WITH RING ABOVE, LATIN SMALL LETTER D WITH CEDILLA, COMBINING DOT ABOVE
            string_t({0x212B, 0x0064, 0x0307, 0x0327}), // ANGSTROM SIGN, LATIN SMALL LETTER D, COMBINING DOT ABOVE, COMBINING CEDILLA
            string_t({0x212B, 0x0064, 0x0327, 0x0307}), // ANGSTROM SIGN, LATIN SMALL LETTER D, COMBINING CEDILLA, COMBINING DOT ABOVE
            string_t({0x212B, 0x1E0B, 0x0327}), // ANGSTROM SIGN, LATIN SMALL LETTER D WITH DOT ABOVE, COMBINING CEDILLA
            string_t({0x212B, 0x1E11, 0x0307}) // ANGSTROM SIGN, LATIN SMALL LETTER D WITH CEDILLA, COMBINING DOT ABOVE
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }

    // Examples from https://www.unicode.org/reports/tr10/#Canonical_Equivalence
    {
        int const N = 3;

        string_t equivalent_strings[] = {
            string_t({0x212B}),                 // Å           U+212B ANGSTROM SIGN
            string_t({0x00C5}),                 // Å           U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
            string_t({0x0041, 0x030A})          // A ◌̊         U+0041 LATIN CAPITAL LETTER A, U+030A COMBINING RING ABOVE
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }

    {
        int const N = 2;

        string_t equivalent_strings[] = {
            string_t({0x0078, 0x031B, 0x0323}), // x ◌̛ ◌̣       U+0078 LATIN SMALL LETTER X, U+031B COMBINING HORN, U+0323 COMBINING DOT BELOW
            string_t({0x0078, 0x0323, 0x031B})  // x ◌̣ ◌̛       U+0078 LATIN SMALL LETTER X, U+0323 COMBINING DOT BELOW, U+031B COMBINING HORN
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }

    {
        int const N = 5;

        string_t equivalent_strings[] = {
            string_t({0x1EF1}),                 // ự           U+1EF1 LATIN SMALL LETTER U WITH HORN AND DOT BELOW
            string_t({0x1EE5, 0x031B}),         // ụ ◌̛         U+1EE5 LATIN SMALL LETTER U WITH DOT BELOW, U+031B COMBINING HORN
            string_t({0x0075, 0x031B, 0x0323}), // u ◌̛ ◌̣       U+0075 LATIN SMALL LETTER U, U+031B COMBINING HORN, U+0323 COMBINING DOT BELOW
            string_t({0x01B0, 0x0323}),         // ư ◌̣         U+01B0 LATIN SMALL LETTER U WITH HORN, U+0323 COMBINING DOT BELOW
            string_t({0x0075, 0x0323, 0x031B})  // u ◌̣ ◌̛       U+0075 LATIN SMALL LETTER U, U+0323 COMBINING DOT BELOW, U+031B COMBINING HORN
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }

    // Example from https://www.unicode.org/reports/tr10/#Avoiding_Normalization
    {
        int const N = 5;

        string_t equivalent_strings[] = {
            string_t({0x01ED}),                 // ǭ           U+01ED LATIN SMALL LETTER O WITH OGONEK AND MACRON
            string_t({0x01EB, 0x0304}),         // ǫ + ̄       U+01EB LATIN SMALL LETTER O WITH OGONEK, U+0304 COMBINING MACRON
            string_t({0x014D, 0x0328}),         // ō + ̨       U+014D LATIN SMALL LETTER O WITH MACRON, U+0328 COMBINING OGONEK
            string_t({0x006F, 0x0304, 0x0328}), // o + ̄ + ̨   U+006F LATIN SMALL LETTER O, U+0304 COMBINING MACRON, U+0328 COMBINING OGONEK
            string_t({0x006F, 0x0328, 0x0304})  // o + ̨ + ̄   U+006F LATIN SMALL LETTER O, U+0328 COMBINING OGONEK, U+0304 COMBINING MACRON
        };

        std::vector<string_t> results[N];
        for (int i = 0; i < N; ++i) {
            detail::canonical_closure(
                equivalent_strings[i].begin(),
                equivalent_strings[i].end(),
                std::back_inserter(results[i]));
            std::sort(results[i].begin(), results[i].end());
            EXPECT_EQ(results[i].size(), N);
        }

        std::sort(std::begin(equivalent_strings), std::end(equivalent_strings));

        EXPECT_TRUE(std::equal(
            std::begin(equivalent_strings),
            std::end(equivalent_strings),
            results[0].begin()));

        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(results[i], results[0]);
        }
    }
}
