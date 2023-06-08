// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/normalize_algorithm.hpp>

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


template<typename T>
using erase_from = decltype(boost::text::normalize_erase<boost::text::nf::c>(
    std::declval<T &>(),
    std::declval<T &>().begin(),
    std::declval<T &>().end()));

#if BOOST_TEXT_CODE_UNIT_CONCEPT_OPTION_2
static_assert(well_formed<erase_from, std::string>::value, "");
#if defined(_MSC_VER)
static_assert(well_formed<erase_from, std::wstring>::value, "");
#endif
#endif
static_assert(well_formed<erase_from, std::vector<char8_t>>::value, "");
static_assert(well_formed<erase_from, std::vector<char16_t>>::value, "");
#if defined(__cpp_lib_concepts)
static_assert(ill_formed<erase_from, std::vector<char32_t>>::value, "");
#endif


TEST(normalization_algorithm, detail_cons_view)
{
    std::string const str1 = "foo";
    std::string str2 = "bar";
    auto r1 = str1 | boost::text::as_char8_t | boost::text::as_utf32;
    auto r2 = str2 | boost::text::as_char8_t | boost::text::as_utf32;
    auto v =
        boost::text::detail::cons_view<char32_t>(r1, r2, r1.begin(), r2.end());

    std::vector<int> result;
    std::copy(v.begin(), v.end(), std::back_inserter(result));
    EXPECT_EQ(result, std::vector<int>({'f', 'o', 'o', 'b', 'a', 'r'}));

    result.clear();
    v = boost::text::detail::cons_view<char32_t>(
        r1, r2, std::ranges::prev(r1.end(), 1), std::next(r2.begin(), 1));
    std::copy(v.begin(), v.end(), std::back_inserter(result));
    EXPECT_EQ(result, std::vector<int>({'o', 'b'}));

#if defined(__cpp_lib_concepts)
    static_assert(boost::text::code_point_range<decltype(v)>);
#endif
}

TEST(normalization_algorithm, erase_nfc)
{
    char16_t const nfc_a_cedilla_ring_above[] = {
        0x0041 /*A 65*/, 0x00b8 /*cedilla, 184*/, 0x030A /*ring above, 778*/
    };

    {
        // erase a prefix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 1);
        EXPECT_EQ(
            str,
            std::vector<char16_t>({0x00b8 /*cedilla*/, 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase a combiner in the middle, causing a combination of the CPs
        // before and after
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>({0x00c5 /*A+ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // erase a suffix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 2, str.begin() + 3);
        EXPECT_EQ(
            str, std::vector<char16_t>({0x0041 /*A*/, 0x00b8 /*cedilla*/}));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    char16_t const nfc_a_c_cedilla_ring_above[] = {
        0x0061 /*a*/, 0x00E7 /*c+cedilla*/, 0x030A /*ring above*/
    };

    {
        // erase noncombiner in the middle, causing a combination of the CPs
        // before and after
        std::vector<char16_t> str(
            std::begin(nfc_a_c_cedilla_ring_above),
            std::end(nfc_a_c_cedilla_ring_above));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>({0x00E5 /*a+ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // erase the whole thing
        std::vector<char16_t> str(
            std::begin(nfc_a_c_cedilla_ring_above),
            std::end(nfc_a_c_cedilla_ring_above));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin(), str.end());
        EXPECT_TRUE(str.empty());
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    char16_t const nfc_5_rings[] = {
        0x030A /*ring above*/,
        0x030A /*ring above*/,
        0x030A /*ring above*/,
        0x030A /*ring above*/,
        0x030A /*ring above*/
    };

    {
        // erase a prefix
        std::vector<char16_t> str(
            std::begin(nfc_5_rings), std::end(nfc_5_rings));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>(3, 0x030A /*ring above*/));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase noncombiner in the middle
        std::vector<char16_t> str(
            std::begin(nfc_5_rings), std::end(nfc_5_rings));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>(4, 0x030A /*ring above*/));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // erase a suffix
        std::vector<char16_t> str(
            std::begin(nfc_5_rings), std::end(nfc_5_rings));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 2, str.begin() + 5);
        EXPECT_EQ(str, std::vector<char16_t>(2, 0x030A /*ring above*/));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    char16_t const nfc_5_as[] = {
        0x0061 /*a*/, 0x0061 /*a*/, 0x0061 /*a*/, 0x0061 /*a*/, 0x0061 /*a*/
    };

    {
        // erase a prefix
        std::vector<char16_t> str(std::begin(nfc_5_as), std::end(nfc_5_as));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>(3, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase noncombiner in the middle
        std::vector<char16_t> str(std::begin(nfc_5_as), std::end(nfc_5_as));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2);
        EXPECT_EQ(str, std::vector<char16_t>(4, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // erase a suffix
        std::vector<char16_t> str(std::begin(nfc_5_as), std::end(nfc_5_as));
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 2, str.begin() + 5);
        EXPECT_EQ(str, std::vector<char16_t>(2, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }
}

TEST(normalization_algorithm, replace_nfc)
{
    char16_t const nfc_a_cedilla_ring_above[] = {
        0x0041 /*A 65*/, 0x00b8 /*cedilla, 184*/, 0x030A /*ring above, 778*/
    };

    {
        // replace the entire string
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0044 /*D*/, 0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin(), str.end(), r.begin(), r.end());
        EXPECT_EQ(str, std::vector<char16_t>({0x1e0a /*D+dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace the entire string with noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin(), str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace the entire string with nothing
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin(), str.end(), r.begin(), r.end());
        EXPECT_EQ(str, std::vector<char16_t>());
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace a prefix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0044 /*D*/, 0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x1e0a /*D+dot above*/, 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end() - 1);
    }

    {
        // replace a prefix with noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end() - 1);
    }

    {
        // replace a prefix with nothing
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(str, std::vector<char16_t>({0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // replace a suffix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0044 /*D*/, 0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>({0x0041 /*A 65*/, 0x1e0a /*D+dot above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace a suffix with noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0226 /*A+dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace a suffix with nothing
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3, r.begin(), r.end());
        EXPECT_EQ(str, std::vector<char16_t>({0x0041 /*A*/}));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // add a dot below, causing the previous composed code point to
        // decompose and recompose
        std::vector<char16_t> str({0x0226 /*A+dot above*/, 'B'});
        std::vector<char16_t> const insertion({0x0323 /*dot below*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x1ea0 /*A+dot below*/, 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace noncombiner(s) with combiner
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0041 /*A*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/, 0x0226 /*A+dot above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // remove a code point, and the code points at the edges of the
        // deletion combine
        std::vector<char16_t> str(
            {0x0041 /*A*/, 0x00b8 /*cedilla*/, 0x030A /*ring above*/});
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(str, std::vector<char16_t>({0x00c5 /*A+ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace a code point, and the inserted code point(s) combine with
        // the one(s) after the replacement
        std::vector<char16_t> str(
            {0x0041 /*A*/, 0x00b8 /*cedilla*/, 0x030A /*ring above*/});
        std::vector<char16_t> const insertion({0x0041 /*A*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>({0x0041 /*A*/, 0x00c5 /*A+ring above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // replace a code point, and the inserted code point(s) combine with
        // the one(s) before the replacement
        std::vector<char16_t> str(
            {0x0041 /*A*/, 0x00b8 /*cedilla*/, 0x030A /*ring above*/});
        std::vector<char16_t> const insertion({0x030A /*ring above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_replace<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x00c5 /*A+ring above*/, 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 1);
    }
}

TEST(normalization_algorithm, insert_nfc)
{
    char16_t const nfc_a_cedilla_ring_above[] = {
        0x0041 /*A*/, 0x00b8 /*cedilla*/, 0x030A /*ring above*/
    };

    {
        // insert an empty prefix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfc_a_cedilla_ring_above),
                std::end(nfc_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // insert a prefix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0044 /*D*/, 0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x1e0a /*D+dot above*/,
                 0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert a prefix of noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 4);
    }

    {
        // insert a combiner prefix before noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x1e0a /*D+dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert empty sequence
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfc_a_cedilla_ring_above),
                std::end(nfc_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0226 /*A+dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 4);
    }

    {
        // insert a combiner into noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x1e0a /*D+dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 2);
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert an empty suffix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfc_a_cedilla_ring_above),
                std::end(nfc_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0044 /*D*/, 0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x1e0a /*D+dot above*/}));
        EXPECT_EQ(result.begin(), str.end() - 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix, claiming that the suffix is already normalized
        // (this makes forms a result that is not NFC, but proves that the
        // don't-normalize logic works)
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0105 /*a+ogonek*/,
             0x0061 /*a*/,
             0x0328 /*ogonek*/,
             0x0105 /*a+ogonek*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str,
            str.end(),
            r.begin(),
            r.end(),
            boost::text::insertion_normalized);
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x0105 /*a+ogonek*/,
                 0x0061 /*a*/,
                 0x0328 /*ogonek*/,
                 0x0105 /*a+ogonek*/}));
        EXPECT_EQ(result.begin(), str.begin() + 3);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix of noncombiners
        std::vector<char16_t> str(
            std::begin(nfc_a_cedilla_ring_above),
            std::end(nfc_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.end() - 4);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a combiner suffix after noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0044 /*D*/}));
        EXPECT_EQ(result.begin(), str.end() - 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix that causes the previous end code point to
        // recompose
        std::vector<char16_t> str({0x0103 /*a+breve*/});
        std::vector<char16_t> const insertion({0x0328 /*ogonek*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>({0x0105 /*a+ogonek*/, 0x0306 /*breve*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }
}

TEST(normalization_algorithm, insert_nfd)
{
    char16_t const nfd_a_cedilla_ring_above[] = {
        0x0041 /*A*/, 0x00b8 /*cedilla*/, 0x030A /*ring above*/
    };

    {
        // insert an empty prefix
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfd_a_cedilla_ring_above),
                std::end(nfd_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // insert a prefix
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion({0x1e0a /*D+dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0044 /*D*/,
                 0x0307 /*dot above*/,
                 0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 2);
    }

    {
        // insert a prefix of noncombiners
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 4);
    }

    {
        // insert a combiner prefix before noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0044 /*D*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert empty sequence
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfd_a_cedilla_ring_above),
                std::end(nfd_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert noncombiners
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 5);
    }

    {
        // insert a combiner into noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.begin() + 2, r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0044 /*D*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.begin() + 2);
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert an empty suffix
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                std::begin(nfd_a_cedilla_ring_above),
                std::end(nfd_a_cedilla_ring_above)));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion({0x1e0a /*D+dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x0044 /*D*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.end() - 2);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix, claiming that the suffix is already normalized
        // (this makes forms a result that is not NFD, but proves that the
        // don't-normalize logic works)
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0061 /*a*/, 0x0105 /*a+ogonek*/, 0x0061 /*a*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str,
            str.end(),
            r.begin(),
            r.end(),
            boost::text::insertion_normalized);
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x0061 /*a*/,
                 0x0105 /*a+ogonek*/,
                 0x0061 /*a*/}));
        EXPECT_EQ(result.begin(), str.begin() + 3);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix of noncombiners
        std::vector<char16_t> str(
            std::begin(nfd_a_cedilla_ring_above),
            std::end(nfd_a_cedilla_ring_above));
        std::vector<char16_t> const insertion(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0041 /*A*/,
                 0x00b8 /*cedilla*/,
                 0x030A /*ring above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/}));
        EXPECT_EQ(result.begin(), str.end() - 4);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a combiner suffix after noncombiners
        std::vector<char16_t> str(
            {0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/,
             0x0307 /*dot above*/});
        std::vector<char16_t> const insertion({0x0044 /*D*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0307 /*dot above*/,
                 0x0044 /*D*/}));
        EXPECT_EQ(result.begin(), str.end() - 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix that causes the previous end code point to
        // recompose.
        std::vector<char16_t> str({0x0103 /*a+breve*/});
        std::vector<char16_t> const insertion({0x0328 /*ogonek*/});
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::d>(
            str, str.end(), r.begin(), r.end());
        EXPECT_EQ(
            str,
            std::vector<char16_t>(
                {0x0061 /*a*/, 0x0328 /*ogonek*/, 0x0306 /*breve*/}));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }
}

TEST(normalization_algorithm, erase_nfc_utf8)
{
    std::u8string const nfc_a_cedilla_ring_above = u8"A\xcc\xa7\xcc\x8a";

    {
        // erase a prefix
        std::u8string str = nfc_a_cedilla_ring_above;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 1);
        EXPECT_TRUE(str == u8"\xcc\xa7\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase a combiner in the middle, causing a combination of the CPs
        // before and after
        std::u8string str = nfc_a_cedilla_ring_above;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3);
        EXPECT_TRUE(str == u8"Å");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // erase a suffix
        std::u8string str = nfc_a_cedilla_ring_above;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 3, str.begin() + 5);
        EXPECT_TRUE(str == u8"A\xcc\xa7");
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    std::u8string const nfc_a_c_cedilla_ring_above = u8"a\xc3\xa7\xcc\x8a";

    {
        // erase noncombiner in the middle, causing a combination of the CPs
        // before and after
        std::u8string str = nfc_a_c_cedilla_ring_above;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 3);
        EXPECT_TRUE(str == u8"å");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // erase the whole thing
        std::u8string str = nfc_a_c_cedilla_ring_above;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin(), str.end());
        EXPECT_TRUE(str.empty());
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    std::u8string const nfc_5_rings = u8"\xcc\x8a\xcc\x8a\xcc\x8a\xcc\x8a\xcc\x8a";

    {
        // erase a prefix
        std::u8string str = nfc_5_rings;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 4);
        EXPECT_TRUE(str == u8"\xcc\x8a\xcc\x8a\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase noncombiner in the middle
        std::u8string str = nfc_5_rings;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 2, str.begin() + 4);
        EXPECT_TRUE(str == u8"\xcc\x8a\xcc\x8a\xcc\x8a\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin() + 2);
        EXPECT_EQ(result.end(), str.begin() + 2);
    }

    {
        // erase a suffix
        std::u8string str = nfc_5_rings;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 4, str.begin() + 10);
        EXPECT_TRUE(str == u8"\xcc\x8a\xcc\x8a");
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    std::u8string const nfc_5_as(5, 'a');

    {
        // erase a prefix
        std::u8string str = nfc_5_as;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 0, str.begin() + 2);
        EXPECT_TRUE(str == std::u8string(3, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // erase noncombiner in the middle
        std::u8string str = nfc_5_as;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 1, str.begin() + 2);
        EXPECT_TRUE(str == std::u8string(4, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // erase a suffix
        std::u8string str = nfc_5_as;
        auto const result = boost::text::normalize_erase<boost::text::nf::c>(
            str, str.begin() + 2, str.begin() + 5);
        EXPECT_TRUE(str == std::u8string(2, 0x0061 /*a*/));
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }
}

TEST(normalization_algorithm, insert_nfc_utf8)
{
    std::u8string const nfc_a_cedilla_ring_above = u8"A\xc2\xb8\xcc\x8a";

    {
        // insert an empty prefix
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // insert a prefix
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion = u8"D\xcc\x87"; // D dot above
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xe1\xb8\x8a" /*D+dot above*/
            u8"A\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert a prefix of noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;

        std::u8string const insertion =
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87" /*dots above*/
            u8"A\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 8);
    }

    {
        // insert a combiner prefix before noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        std::u8string const insertion = u8"D";
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xe1\xb8\x8a"               /*D+dot above*/
            u8"\xcc\x87\xcc\x87\xcc\x87"); // dots above
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert empty sequence
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion =
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xc8\xa6"                 /*A+dot above*/
            u8"\xcc\x87\xcc\x87\xcc\x87" // dots above
            u8"\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 8);
    }

    {
        // insert a combiner into noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        std::u8string const insertion = u8"D";
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 4, r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87" /*dots above*/
            u8"\xe1\xb8\x8a"     /*D+dot above*/
            u8"\xcc\x87");       // dot above
        EXPECT_EQ(result.begin(), str.begin() + 4);
        EXPECT_EQ(result.end(), str.begin() + 7);
    }

    {
        // insert an empty suffix
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion = u8"D\xcc\x87";
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xe1\xb8\x8a"); /*D+dot above*/
        EXPECT_EQ(result.begin(), str.end() - 3);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix, claiming that the suffix is already normalized
        // (this makes forms a result that is not NFC, but proves that the
        // don't-normalize logic works)
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion =
            u8"\xc4\x85" /*a+ogonek*/
            u8"a"
            u8"\xcc\xa8" /*ogonek*/
            u8"\xc4\x85" /*a+ogonek*/;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str,
            str.end(),
            r.begin(),
            r.end(),
            boost::text::insertion_normalized);
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xc4\x85" /*a+ogonek*/
            u8"a"
            u8"\xcc\xa8" /*ogonek*/
            u8"\xc4\x85" /*a+ogonek*/);
        EXPECT_EQ(result.begin(), str.begin() + 5);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix of noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;
        std::u8string const insertion =
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"); // dots above
        EXPECT_EQ(result.begin(), str.begin() + 5);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a combiner suffix after noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        std::u8string const insertion = u8"D";
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87" // dots above
            u8"D");
        EXPECT_EQ(result.begin(), str.end() - 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix that causes the previous end code point to
        // recompose
        std::u8string str = u8"\xc4\x83"; // a+breve
        std::u8string const insertion = u8"\xcc\xa8" /*ogonek*/;
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(str == u8"\xc4\x85" /*a+ogonek*/ u8"\xcc\x86" /*breve*/);
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }
}

TEST(normalization_algorithm, insert_nfc_inserting_utf32)
{
    std::u8string const nfc_a_cedilla_ring_above = u8"A\xc2\xb8\xcc\x8a";

    {
        // insert an empty prefix
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {0};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin());
    }

    {
        // insert a prefix
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {0x0044 /*D*/, 0x0307 /*dot above*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xe1\xb8\x8a" /*D+dot above*/
            u8"A\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert a prefix of noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;

        char32_t const insertion[] = {
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87" /*dots above*/
            u8"A\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 8);
    }

    {
        // insert a combiner prefix before noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        char32_t const insertion[] = {0x0044 /*D*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xe1\xb8\x8a"               /*D+dot above*/
            u8"\xcc\x87\xcc\x87\xcc\x87"); // dots above
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 3);
    }

    {
        // insert empty sequence
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {0};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.begin() + 1);
        EXPECT_EQ(result.end(), str.begin() + 1);
    }

    {
        // insert noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 1, r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xc8\xa6"                 /*A+dot above*/
            u8"\xcc\x87\xcc\x87\xcc\x87" // dots above
            u8"\xc2\xb8\xcc\x8a");
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.begin() + 8);
    }

    {
        // insert a combiner into noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        char32_t const insertion[] = {0x0044 /*D*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.begin() + 4, r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87" /*dots above*/
            u8"\xe1\xb8\x8a"     /*D+dot above*/
            u8"\xcc\x87");       // dot above
        EXPECT_EQ(result.begin(), str.begin() + 4);
        EXPECT_EQ(result.end(), str.begin() + 7);
    }

    {
        // insert an empty suffix
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {0};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(str == nfc_a_cedilla_ring_above);
        EXPECT_EQ(result.begin(), str.end());
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {0x0044 /*D*/, 0x0307 /*dot above*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xe1\xb8\x8a"); /*D+dot above*/
        EXPECT_EQ(result.begin(), str.end() - 3);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix, claiming that the suffix is already normalized
        // (this makes forms a result that is not NFC, but proves that the
        // don't-normalize logic works)
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {
            0x0105 /*a+ogonek*/,
            0x0061 /*a*/,
            0x0328 /*ogonek*/,
            0x0105 /*a+ogonek*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str,
            str.end(),
            r.begin(),
            r.end(),
            boost::text::insertion_normalized);
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xc4\x85" /*a+ogonek*/
            u8"a"
            u8"\xcc\xa8" /*ogonek*/
            u8"\xc4\x85" /*a+ogonek*/);
        EXPECT_EQ(result.begin(), str.begin() + 5);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix of noncombiners
        std::u8string str = nfc_a_cedilla_ring_above;
        char32_t const insertion[] = {
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/,
            0x0307 /*dot above*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"A\xc2\xb8\xcc\x8a"
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"); // dots above
        EXPECT_EQ(result.begin(), str.begin() + 5);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a combiner suffix after noncombiners
        std::u8string str = u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87"; // dots above
        char32_t const insertion[] = {0x0044 /*D*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(
            str ==
            u8"\xcc\x87\xcc\x87\xcc\x87\xcc\x87" // dots above
            u8"D");
        EXPECT_EQ(result.begin(), str.end() - 1);
        EXPECT_EQ(result.end(), str.end());
    }

    {
        // insert a suffix that causes the previous end code point to
        // recompose
        std::u8string str = u8"\xc4\x83"; // a+breve
        char32_t const insertion[] = {0x0328 /*ogonek*/};
        auto const r = insertion | boost::text::as_utf32;
        auto const result = boost::text::normalize_insert<boost::text::nf::c>(
            str, str.end(), r.begin(), r.end());
        EXPECT_TRUE(str == u8"\xc4\x85" /*a+ogonek*/ u8"\xcc\x86" /*breve*/);
        EXPECT_EQ(result.begin(), str.begin());
        EXPECT_EQ(result.end(), str.end());
    }
}
