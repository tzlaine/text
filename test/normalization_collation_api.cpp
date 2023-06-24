// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// TODO #include <boost/text/collate.hpp>
#include <boost/text/normalize_string.hpp>
#include <boost/text/string_utility.hpp>
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <gtest/gtest.h>

#include <deque>


using namespace boost;
using namespace boost::text::detail;

using u32_iter =
    text::utf_8_to_32_iterator<char8_t const *, text::null_sentinel_t>;
using null_sent_cp_rubrange =
    std::ranges::subrange<u32_iter, text::null_sentinel_t>;
using sentinel_cp_range_t =
    text::utf_view<text::format::utf32, null_sent_cp_rubrange>;

void to_sentinel_cp_range(
    std::u8string & s,
    sentinel_cp_range_t & r,
    std::vector<char32_t> cps,
    bool normalize = false)
{
    s = text::to_u8string(cps.begin(), cps.end());
    if (normalize)
        boost::text::normalize<text::nf::d>(s);
    r = sentinel_cp_range_t{null_sent_cp_rubrange(
        u32_iter(s.data(), s.data(), text::null_sentinel),
        text::null_sentinel)};
}


// This test also covers to_string()'s sentinel API.
TEST(sentinel_apis, nfd)
{
    // Taken from normalization_collation_api.cpp, first case.

    // 1E0A;1E0A;0044 0307;1E0A;0044 0307;
    // (Ḋ; Ḋ; D◌̇; Ḋ; D◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE
    {
        std::u8string c1_;
        sentinel_cp_range_t c1;
        to_sentinel_cp_range(c1_, c1, {0x1E0A});

        std::u8string c2_;
        sentinel_cp_range_t c2;
        to_sentinel_cp_range(c2_, c2, {0x1E0A});

        std::u8string c3_;
        sentinel_cp_range_t c3;
        to_sentinel_cp_range(c3_, c3, {0x0044, 0x0307});

        std::u8string c4_;
        sentinel_cp_range_t c4;
        to_sentinel_cp_range(c4_, c4, {0x1E0A});

        std::u8string c5_;
        sentinel_cp_range_t c5;
        to_sentinel_cp_range(c5_, c5, {0x0044, 0x0307});

        EXPECT_TRUE(text::is_normalized<text::nf::c>(c2.begin(), c2.end()));
        EXPECT_TRUE(text::is_normalized<text::nf::kc>(c2.begin(), c2.end()));

        EXPECT_TRUE(text::is_normalized<text::nf::d>(c3.begin(), c3.end()));
        EXPECT_TRUE(text::is_normalized<text::nf::kd>(c3.begin(), c3.end()));

        EXPECT_TRUE(text::is_normalized<text::nf::c>(c4.begin(), c4.end()));
        EXPECT_TRUE(text::is_normalized<text::nf::kc>(c4.begin(), c4.end()));

        EXPECT_TRUE(text::is_normalized<text::nf::d>(c5.begin(), c5.end()));
        EXPECT_TRUE(text::is_normalized<text::nf::kd>(c5.begin(), c5.end()));
    }

    {
        std::u8string c1_;
        sentinel_cp_range_t c1;
        to_sentinel_cp_range(c1_, c1, {0x1E0A}, true);

        std::u8string c2_;
        sentinel_cp_range_t c2;
        to_sentinel_cp_range(c2_, c2, {0x1E0A}, true);

        std::u8string c3_;
        sentinel_cp_range_t c3;
        to_sentinel_cp_range(c3_, c3, {0x0044, 0x0307}, true);

        std::u8string c4_;
        sentinel_cp_range_t c4;
        to_sentinel_cp_range(c4_, c4, {0x1E0A}, true);

        std::u8string c5_;
        sentinel_cp_range_t c5;
        to_sentinel_cp_range(c5_, c5, {0x0044, 0x0307}, true);

        {
            EXPECT_EQ(
                distance(c1.begin(), c1.end()), distance(c3.begin(), c3.end()));
            auto c1_it = c1.begin();
            auto c3_it = c3.begin();
            int i = 0;
            for (; c1_it != c1.end(); ++c1_it, ++c3_it, ++i) {
                EXPECT_EQ(*c1_it, *c3_it) << "iteration " << i;
            }
        }

        {
            EXPECT_EQ(
                distance(c2.begin(), c2.end()), distance(c3.begin(), c3.end()));
            auto c2_it = c2.begin();
            auto c3_it = c3.begin();
            int i = 0;
            for (; c2_it != c2.end(); ++c2_it, ++c3_it, ++i) {
                EXPECT_EQ(*c2_it, *c3_it) << "iteration " << i;
            }
        }

        {
            EXPECT_EQ(
                distance(c4.begin(), c4.end()), distance(c5.begin(), c5.end()));
            auto c4_it = c4.begin();
            auto c5_it = c5.begin();
            int i = 0;
            for (; c4_it != c4.end(); ++c4_it, ++c5_it, ++i) {
                EXPECT_EQ(*c4_it, *c5_it) << "iteration " << i;
            }
        }
    }
}

#if 0 // TODO
TEST(sentinel_apis, collation)
{
    // Taken from relative_collation_test_non_ignorable.cpp, first iteration.

    std::u8string cps_;
    sentinel_cp_range_t cps;
    to_sentinel_cp_range(cps_, cps, {0x0338, 0x0334}, true);

    std::vector<char32_t> other_cps({0x0338, 0x0334});

    collate(
        cps.begin(),
        cps.end(),
        other_cps.begin(),
        other_cps.end(),
        text::default_collation_table(),
        text::collation_strength::identical,
        text::case_first::off,
        text::case_level::off,
        text::variable_weighting::non_ignorable);

    text::collation_sort_key(
        cps, text::default_collation_table(), text::collation_flags{});

    text::collate(
        cps,
        other_cps,
        text::default_collation_table(),
        text::collation_flags{});
}
#endif

char32_t const cps[] = {
    0x1053B, 0x0062, 0x1053C, 0x0021, 0x1053C, 0x003F, 0x1053C, 0x0334,
    0x1053C, 0x0061, 0x1053C, 0x0041, 0x1053C, 0x0062, 0x1053D, 0x0021,
    0x1053D, 0x003F, 0x1053D, 0x0334, 0x1053D, 0x0061, 0x1053D, 0x0041,
    0x1053D, 0x0062, 0x1053E, 0x0021, 0x1053E, 0x003F, 0x1053E, 0x0334,
    0x1053E, 0x0061, 0x1053E, 0x0041, 0x1053E, 0x0062, 0x1053F, 0x0021,
    0x1053F, 0x003F, 0x1053F, 0x0334, 0x1053F, 0x0061, 0x1053F, 0x0041,
    0x1053F, 0x0062, 0x10540, 0x0021, 0x10540, 0x003F, 0x10540, 0x0334,
    0x10540, 0x0061, 0x10540, 0x0041, 0x10540, 0x0062, 0x10541, 0x0021,
    0x10541, 0x003F, 0x10541, 0x0334, 0x10541, 0x0061, 0x10541, 0x0041,
    0x10541, 0x0062, 0x10542, 0x0021, 0x10542, 0x003F, 0x10542, 0x0334,
    0x10542, 0x0061, 0x10542, 0x0041, 0x10542, 0x0062, 0x10543, 0x0021,
    0x10543, 0x003F, 0x10543, 0x0334, 0x10543, 0x0061, 0x10543, 0x0041,
    0x10543, 0x0062, 0x10544, 0x0021, 0x10544, 0x003F, 0x10544, 0x0334,
    0x10544, 0x0061, 0x10544, 0x0041, 0x10544, 0x0062, 0x10545, 0x0021,
    0x10545, 0x003F, 0x10545, 0x0334, 0x10545, 0x0061, 0x10545, 0x0041,
    0x10545, 0x0062, 0x10546, 0x0021, 0x10546, 0x003F, 0x10546, 0x0334,
    0x10546, 0x0061, 0x10546, 0x0041, 0x10546, 0x0062, 0x10547, 0x0021,
    0x10547, 0x003F, 0x10547, 0x0334, 0x10547, 0x0061, 0x10547, 0x0041,
    0x10547, 0x0062, 0x10548, 0x0021, 0x10548, 0x003F, 0x10548, 0x0334,
    0x10548, 0x0061, 0x10548, 0x0041, 0x10548, 0x0062, 0x10549, 0x0021,
    0x10549, 0x003F, 0x10549, 0x0334, 0x10549, 0x0061, 0x10549, 0x0041,
    0x10549, 0x0062, 0x1054A, 0x0021, 0x1054A, 0x003F, 0x1054A, 0x0334,
    0x1054A, 0x0061, 0x1054A, 0x0041, 0x1054A, 0x0062, 0x1054B, 0x0021,
    0x1054B, 0x003F, 0x1054B, 0x0334, 0x1054B, 0x0061, 0x1054B, 0x0041,
    0x1054B, 0x0062, 0x1054C, 0x0021, 0x1054C, 0x003F, 0x1054C, 0x0334,
    0x1054C, 0x0061, 0x1054C, 0x0041, 0x1054C, 0x0062, 0x1054D, 0x0021,
    0x1054D, 0x003F, 0x1054D, 0x0334, 0x1054D, 0x0061, 0x1054D, 0x0041,
    0x1054D, 0x0062, 0x1054E, 0x0021, 0x1054E, 0x003F, 0x1054E, 0x0334,
    0x1054E, 0x0061, 0x1054E, 0x0041, 0x1054E, 0x0062, 0x1054F, 0x0021,
    0x1054F, 0x003F, 0x1054F, 0x0334, 0x1054F, 0x0061, 0x1054F, 0x0041,
    0x1054F, 0x0062, 0x10550, 0x0021, 0x10550, 0x003F, 0x10550, 0x0334,
    0x10550, 0x0061, 0x10550, 0x0041, 0x10550, 0x0062, 0x10551, 0x0021,
    0x10551, 0x003F, 0x10551, 0x0334, 0x10551, 0x0061, 0x10551, 0x0041,
    0x10551, 0x0062, 0x10552, 0x0021, 0x10552, 0x003F, 0x10552, 0x0334,
    0x10552, 0x0061, 0x10552, 0x0041, 0x10552, 0x0062, 0x10553, 0x0021,
    0x10553, 0x003F, 0x10553, 0x0334, 0x10553, 0x0061, 0x10553, 0x0041,
    0x10553, 0x0062, 0x10554, 0x0021, 0x10554, 0x003F, 0x10554, 0x0334,
    0x10554, 0x0061, 0x10554, 0x0041, 0x10554, 0x0062, 0x10555, 0x0021,
    0x10555, 0x003F, 0x10555, 0x0334, 0x10555, 0x0061, 0x10555, 0x0041,
    0x10555, 0x0062, 0x10556, 0x0021, 0x10556, 0x003F, 0x10556, 0x0334,
    0x10556, 0x0061, 0x10556, 0x0041, 0x10556, 0x0062, 0x10557, 0x0021,
    0x10557, 0x003F, 0x10557, 0x0334, 0x10557, 0x0061, 0x10557, 0x0041,
    0x10557, 0x0062, 0x10558, 0x0021, 0x10558, 0x003F, 0x10558, 0x0334,
    0x10558, 0x0061, 0x10558, 0x0041, 0x10558, 0x0062, 0x10559, 0x0021,
    0x10559, 0x003F, 0x10559, 0x0334, 0x10559, 0x0061, 0x10559, 0x0041,
    0x10559, 0x0062, 0x1055A, 0x0021, 0x1055A, 0x003F, 0x1055A, 0x0334,
    0x1055A, 0x0061, 0x1055A, 0x0041, 0x1055A, 0x0062, 0x1055B, 0x0021,
    0x1055B, 0x003F, 0x1055B, 0x0334, 0x1055B, 0x0061, 0x1055B, 0x0041,
    0x1055B, 0x0062, 0x1055C, 0x0021, 0x1055C, 0x003F, 0x1055C, 0x0334,
    0x1055C, 0x0061, 0x1055C, 0x0041, 0x1055C, 0x0062, 0x1055D, 0x0021,
    0x1055D, 0x003F, 0x1055D, 0x0334, 0x1055D, 0x0061, 0x1055D, 0x0041,
    0x1055D, 0x0062, 0x1055E, 0x0021, 0x1055E, 0x003F, 0x1055E, 0x0334,
    0x1055E, 0x0061, 0x1055E, 0x0041, 0x1055E, 0x0062, 0x1055F, 0x0021,
    0x1055F, 0x003F, 0x1055F, 0x0334, 0x1055F, 0x0061, 0x1055F, 0x0041,
    0x1055F, 0x0062, 0x10560, 0x0021, 0x10560, 0x003F, 0x10560, 0x0334,
    0x10560, 0x0061, 0x10560, 0x0041, 0x10560, 0x0062, 0x10561, 0x0021,
    0x10561, 0x003F, 0x10561, 0x0334, 0x10561, 0x0061, 0x10561, 0x0041,
    0x10561, 0x0062, 0x10562, 0x0021, 0x10562, 0x003F, 0x10562, 0x0334,
    0x10562, 0x0061, 0x10562, 0x0041, 0x10562, 0x0062, 0x10563, 0x0021,
    0x10563, 0x003F, 0x10563, 0x0334, 0x10563, 0x0061, 0x10563, 0x0041,
    0x10563, 0x0062, 0x110D0, 0x0021, 0x110D0, 0x003F, 0x110D0, 0x0334,
    0x110D0, 0x0061, 0x110D0, 0x0041, 0x110D0, 0x0062, 0x110D1, 0x0021,
    0x110D1, 0x003F, 0x110D1, 0x0334, 0x110D1, 0x0061, 0x110D1, 0x0041,
    0x110D1, 0x0062, 0x110D2, 0x0021, 0x110D2, 0x003F, 0x110D2, 0x0334,
    0x110D2, 0x0061, 0x110D2, 0x0041, 0x110D2, 0x0062, 0x110D3, 0x0021,
    0x110D3, 0x003F, 0x110D3, 0x0334, 0x110D3, 0x0061, 0x110D3, 0x0041,
    0x110D3, 0x0062, 0x110D4, 0x0021, 0x110D4, 0x003F, 0x110D4, 0x0334,
    0x110D4, 0x0061, 0x110D4, 0x0041, 0x110D4, 0x0062, 0x110D5, 0x0021,
    0x110D5, 0x003F, 0x110D5, 0x0334, 0x110D5, 0x0061, 0x110D5, 0x0041,
    0x110D5, 0x0062, 0x110D6, 0x0021, 0x110D6, 0x003F, 0x110D6, 0x0334,
    0x110D6, 0x0061, 0x110D6, 0x0041, 0x110D6, 0x0062, 0x110D7, 0x0021,
    0x110D7, 0x003F, 0x110D7, 0x0334, 0x110D7, 0x0061, 0x110D7, 0x0041,
    0x110D7, 0x0062, 0x110D8, 0x0021, 0x110D8, 0x003F, 0x110D8, 0x0334,
    0x110D8, 0x0061, 0x110D8, 0x0041, 0x110D8, 0x0062, 0x110D9, 0x0021,
    0x110D9, 0x003F, 0x110D9, 0x0334, 0x110D9, 0x0061, 0x110D9, 0x0041,
    0x110D9, 0x0062, 0x110DA, 0x0021, 0x110DA, 0x003F, 0x110DA, 0x0334,
    0x110DA, 0x0061, 0x110DA, 0x0041, 0x110DA, 0x0062, 0x110DB, 0x0021,
    0x110DB, 0x003F, 0x110DB, 0x0334, 0x110DB, 0x0061, 0x110DB, 0x0041,
    0x110DB, 0x0062, 0x110DC, 0x0021, 0x110DC, 0x003F, 0x110DC, 0x0334,
    0x110DC, 0x0061, 0x110DC, 0x0041, 0x110DC, 0x0062, 0x110DD, 0x0021,
    0x110DD, 0x003F, 0x110DD, 0x0334, 0x110DD, 0x0061, 0x110DD, 0x0041,
    0x110DD, 0x0062, 0x110DE, 0x0021, 0x110DE, 0x003F, 0x110DE, 0x0334,
    0x110DE, 0x0061, 0x110DE, 0x0041, 0x110DE, 0x0062, 0x110DF, 0x0021,
    0x110DF, 0x003F, 0x110DF, 0x0334, 0x110DF, 0x0061, 0x110DF, 0x0041,
    0x110DF, 0x0062, 0x110E0, 0x0021, 0x110E0, 0x003F, 0x110E0, 0x0334,
    0x110E0, 0x0061, 0x110E0, 0x0041, 0x110E0, 0x0062, 0x110E1, 0x0021,
    0x110E1, 0x003F, 0x110E1, 0x0334, 0x110E1, 0x0061, 0x110E1, 0x0041,
    0x110E1, 0x0062, 0x110E2, 0x0021, 0x110E2, 0x003F, 0x110E2, 0x0334,
    0x110E2, 0x0061, 0x110E2, 0x0041, 0x110E2, 0x0062, 0x110E3, 0x0021,
    0x110E3, 0x003F, 0x110E3, 0x0334, 0x110E3, 0x0061, 0x110E3, 0x0041,
    0x110E3, 0x0062, 0x110E4, 0x0021, 0x110E4, 0x003F, 0x110E4, 0x0334,
    0x110E4, 0x0061, 0x110E4, 0x0041, 0x110E4, 0x0062, 0x110E5, 0x0021,
    0x110E5, 0x003F, 0x110E5, 0x0334, 0x110E5, 0x0061, 0x110E5, 0x0041,
    0x110E5, 0x0062, 0x110E6, 0x0021, 0x110E6, 0x003F, 0x110E6, 0x0334,
    0x110E6, 0x0061, 0x110E6, 0x0041, 0x110E6, 0x0062, 0x110E7, 0x0021,
    0x110E7, 0x003F, 0x110E7, 0x0334, 0x110E7, 0x0061, 0x110E7, 0x0041,
    0x110E7, 0x0062, 0x110E8, 0x0021, 0x110E8, 0x003F, 0x110E8, 0x0334,
    0x110E8, 0x0061, 0x110E8, 0x0041, 0x110E8, 0x0062, 0x16A40, 0x0021,
    0x16A40, 0x003F, 0x16A40, 0x0334, 0x16A40, 0x0061, 0x16A40, 0x0041,
    0x16A40, 0x0062, 0x16A41, 0x0021, 0x16A41, 0x003F, 0x16A41, 0x0334,
    0x16A41, 0x0061, 0x16A41, 0x0041, 0x16A41, 0x0062, 0x16A42, 0x0021,
    0x16A42, 0x003F, 0x16A42, 0x0334, 0x16A42, 0x0061, 0x16A42, 0x0041,
    0x16A42, 0x0062, 0x16A43, 0x0021, 0x16A43, 0x003F, 0x16A43, 0x0334,
    0x16A43, 0x0061, 0x16A43, 0x0041, 0x16A43, 0x0062, 0x16A44, 0x0021,
    0x16A44, 0x003F, 0x16A44, 0x0334, 0x16A44, 0x0061, 0x16A44, 0x0041,
    0x16A44, 0x0062, 0x16A45, 0x0021, 0x16A45, 0x003F, 0x16A45, 0x0334,
    0x16A45, 0x0061, 0x16A45, 0x0041, 0x16A45, 0x0062, 0x16A46, 0x0021,
    0x16A46, 0x003F, 0x16A46, 0x0334, 0x16A46, 0x0061, 0x16A46, 0x0041,
    0x16A46, 0x0062, 0x16A47, 0x0021, 0x16A47, 0x003F, 0x16A47, 0x0334,
    0x16A47, 0x0061, 0x16A47, 0x0041, 0x16A47, 0x0062, 0x16A48, 0x0021,
    0x16A48, 0x003F, 0x16A48, 0x0334, 0x16A48, 0x0061, 0x16A48, 0x0041,
    0x16A48, 0x0062, 0x16A49, 0x0021, 0x16A49, 0x003F, 0x16A49, 0x0334,
    0x16A49, 0x0061, 0x16A49, 0x0041, 0x16A49, 0x0062, 0x16A4A, 0x0021,
    0x16A4A, 0x003F, 0x16A4A, 0x0334, 0x16A4A, 0x0061, 0x16A4A, 0x0041,
    0x16A4A, 0x0062, 0x16A4B, 0x0021, 0x16A4B, 0x003F, 0x16A4B, 0x0334,
    0x16A4B, 0x0061, 0x16A4B, 0x0041, 0x16A4B, 0x0062, 0x16A4C, 0x0021,
    0x16A4C, 0x003F, 0x16A4C, 0x0334, 0x16A4C, 0x0061, 0x16A4C, 0x0041,
    0x16A4C, 0x0062, 0x16A4D, 0x0021, 0x16A4D, 0x003F, 0x16A4D, 0x0334,
    0x16A4D, 0x0061, 0x16A4D, 0x0041, 0x16A4D, 0x0062, 0x16A4E, 0x0021,
    0x16A4E, 0x003F, 0x16A4E, 0x0334, 0x16A4E, 0x0061, 0x16A4E, 0x0041,
    0x16A4E, 0x0062, 0x16A4F, 0x0021, 0x16A4F, 0x003F, 0x16A4F, 0x0334,
    0x16A4F, 0x0061, 0x16A4F, 0x0041, 0x16A4F, 0x0062, 0x16A50, 0x0021,
    0x16A50, 0x003F, 0x16A50, 0x0334, 0x16A50, 0x0061, 0x16A50, 0x0041,
    0x16A50, 0x0062, 0x16A51, 0x0021, 0x16A51, 0x003F, 0x16A51, 0x0334,
    0x16A51, 0x0061, 0x16A51, 0x0041, 0x16A51, 0x0062, 0x16A52, 0x0021,
    0x16A52, 0x003F, 0x16A52, 0x0334, 0x16A52, 0x0061, 0x16A52, 0x0041,
    0x16A52, 0x0062, 0x16A53, 0x0021, 0x16A53, 0x003F, 0x16A53, 0x0334,
    0x16A53, 0x0061, 0x16A53, 0x0041, 0x16A53, 0x0062, 0x16A54, 0x0021,
    0x16A54, 0x003F, 0x16A54, 0x0334, 0x16A54, 0x0061, 0x16A54, 0x0041,
    0x16A54, 0x0062, 0x16A55, 0x0021, 0x16A55, 0x003F, 0x16A55, 0x0334,
    0x16A55, 0x0061, 0x16A55, 0x0041, 0x16A55, 0x0062, 0x16A56, 0x0021,
    0x16A56, 0x003F, 0x16A56, 0x0334, 0x16A56, 0x0061, 0x16A56, 0x0041,
    0x16A56, 0x0062, 0x16A57, 0x0021, 0x16A57, 0x003F, 0x16A57, 0x0334,
    0x16A57, 0x0061, 0x16A57, 0x0041, 0x16A57, 0x0062, 0x16A58, 0x0021,
    0x16A58, 0x003F, 0x16A58, 0x0334, 0x16A58, 0x0061, 0x16A58, 0x0041,
    0x16A58, 0x0062, 0x16A59, 0x0021, 0x16A59, 0x003F, 0x16A59, 0x0334,
    0x16A59, 0x0061, 0x16A59, 0x0041, 0x16A59, 0x0062, 0x16A5A, 0x0021,
    0x16A5A, 0x003F, 0x16A5A, 0x0334, 0x16A5A, 0x0061, 0x16A5A, 0x0041,
    0x16A5A, 0x0062, 0x16A5B, 0x0021, 0x16A5B, 0x003F, 0x16A5B, 0x0334,
    0x16A5B, 0x0061, 0x16A5B, 0x0041, 0x16A5B, 0x0062, 0x16A5C, 0x0021,
    0x16A5C, 0x003F, 0x16A5C, 0x0334, 0x16A5C, 0x0061, 0x16A5C, 0x0041,
    0x16A5C, 0x0062, 0x16A5D, 0x0021, 0x16A5D, 0x003F, 0x16A5D, 0x0334,
    0x16A5D, 0x0061, 0x16A5D, 0x0041, 0x16A5D, 0x0062, 0x16A5E, 0x0021,
    0x16A5E, 0x003F, 0x16A5E, 0x0334, 0x16A5E, 0x0061, 0x16A5E, 0x0041,
    0x16A5E, 0x0062, 0x10000, 0x0021, 0x10000, 0x003F, 0x10000, 0x0334,
    0x10000, 0x0061, 0x10000, 0x0041, 0x10000, 0x0062, 0x10001, 0x0021,
    0x10001, 0x003F, 0x10001, 0x0334, 0x10001, 0x0061, 0x10001, 0x0041,
    0x10001, 0x0062, 0x10002, 0x0021, 0x10002, 0x003F, 0x10002, 0x0334,
    0x10002, 0x0061, 0x10002, 0x0041, 0x10002, 0x0062, 0x10003, 0x0021,
    0x10003, 0x003F, 0x10003, 0x0334, 0x10003, 0x0061, 0x10003, 0x0041,
    0x10003, 0x0062, 0x10004, 0x0021, 0x10004, 0x003F, 0x10004, 0x0334,
    0x10004, 0x0061, 0x10004, 0x0041, 0x10004, 0x0062, 0x10005, 0x0021,
    0x10005, 0x003F, 0x10005, 0x0334, 0x10005, 0x0061, 0x10005, 0x0041,
    0x10005, 0x0062, 0x10006, 0x0021, 0x10006, 0x003F, 0x10006, 0x0334,
    0x10006, 0x0061, 0x10006, 0x0041, 0x10006, 0x0062, 0x10007, 0x0021,
    0x10007, 0x003F, 0x10007, 0x0334, 0x10007, 0x0061, 0x10007, 0x0041,
    0x10007, 0x0062, 0x10008, 0x0021, 0x10008, 0x003F, 0x10008, 0x0334,
    0x10008, 0x0061, 0x10008, 0x0041, 0x10008, 0x0062, 0x10009, 0x0021,
    0x10009, 0x003F, 0x10009, 0x0334, 0x10009, 0x0061, 0x10009, 0x0041,
    0x10009, 0x0062, 0x1000A, 0x0021, 0x1000A, 0x003F, 0x1000A, 0x0334,
    0x1000A, 0x0061, 0x1000A, 0x0041, 0x1000A, 0x0062, 0x1000B, 0x0021,
    0x1000B, 0x003F, 0x1000B, 0x0334, 0x1000B, 0x0061, 0x1000B, 0x0041,
    0x1000B, 0x0062, 0x1000D, 0x0021, 0x1000D, 0x003F, 0x1000D, 0x0334,
    0x1000D, 0x0061, 0x1000D, 0x0041, 0x1000D, 0x0062, 0x1000E, 0x0021,
    0x1000E, 0x003F, 0x1000E, 0x0334, 0x1000E, 0x0061, 0x1000E, 0x0041,
    0x1000E, 0x0062, 0x1000F, 0x0021, 0x1000F, 0x003F, 0x1000F, 0x0334,
    0x1000F, 0x0061, 0x1000F, 0x0041, 0x1000F, 0x0062, 0x10010, 0x0021,
    0x10010, 0x003F, 0x10010, 0x0334, 0x10010, 0x0061, 0x10010, 0x0041,
    0x10010, 0x0062, 0x10011, 0x0021, 0x10011, 0x003F, 0x10011, 0x0334,
    0x10011, 0x0061, 0x10011, 0x0041, 0x10011, 0x0062, 0x10012, 0x0021,
    0x10012, 0x003F, 0x10012, 0x0334, 0x10012, 0x0061, 0x10012, 0x0041,
    0x10012, 0x0062, 0x10013, 0x0021, 0x10013, 0x003F, 0x10013, 0x0334,
    0x10013, 0x0061, 0x10013, 0x0041, 0x10013, 0x0062, 0x10014, 0x0021,
    0x10014, 0x003F, 0x10014, 0x0334, 0x10014, 0x0061, 0x10014, 0x0041,
    0x10014, 0x0062, 0x10015, 0x0021, 0x10015, 0x003F, 0x10015, 0x0334,
    0x10015, 0x0061, 0x10015, 0x0041, 0x10015, 0x0062, 0x10016, 0x0021,
    0x10016, 0x003F, 0x10016, 0x0334, 0x10016, 0x0061, 0x10016, 0x0041,
    0x10016, 0x0062, 0x10017, 0x0021, 0x10017, 0x003F, 0x10017, 0x0334,
    0x10017, 0x0061, 0x10017, 0x0041, 0x10017, 0x0062, 0x10018, 0x0021,
    0x10018, 0x003F, 0x10018, 0x0334, 0x10018, 0x0061, 0x10018, 0x0041,
    0x10018, 0x0062, 0x10019, 0x0021, 0x10019, 0x003F, 0x10019, 0x0334,
    0x10019, 0x0061, 0x10019, 0x0041, 0x10019, 0x0062, 0x1001A, 0x0021,
    0x1001A, 0x003F, 0x1001A, 0x0334, 0x1001A, 0x0061, 0x1001A, 0x0041,
    0x1001A, 0x0062, 0x1001B, 0x0021, 0x1001B, 0x003F, 0x1001B, 0x0334,
    0x1001B, 0x0061, 0x1001B, 0x0041, 0x1001B, 0x0062, 0x1001C, 0x0021,
    0x1001C, 0x003F, 0x1001C, 0x0334, 0x1001C, 0x0061, 0x1001C, 0x0041,
    0x1001C, 0x0062, 0x1001D, 0x0021, 0x1001D, 0x003F, 0x1001D, 0x0334,
    0x1001D, 0x0061, 0x1001D, 0x0041, 0x1001D, 0x0062, 0x1001E, 0x0021,
    0x1001E, 0x003F, 0x1001E, 0x0334, 0x1001E, 0x0061, 0x1001E, 0x0041,
    0x1001E, 0x0062, 0x1001F, 0x0021, 0x1001F, 0x003F, 0x1001F, 0x0334,
    0x1001F, 0x0061, 0x1001F, 0x0041, 0x1001F, 0x0062, 0x10020, 0x0021,
    0x10020, 0x003F, 0x10020, 0x0334, 0x10020, 0x0061, 0x10020, 0x0041,
    0x10020, 0x0062, 0x10021, 0x0021, 0x10021, 0x003F, 0x10021, 0x0334,
    0x10021, 0x0061, 0x10021, 0x0041, 0x10021, 0x0062, 0x10022, 0x0021,
    0x10022, 0x003F, 0x10022, 0x0334, 0x10022, 0x0061, 0x10022, 0x0041,
    0x10022, 0x0062, 0x10023, 0x0021, 0x10023, 0x003F, 0x10023, 0x0334,
    0x10023, 0x0061, 0x10023, 0x0041, 0x10023, 0x0062, 0x10024, 0x0021,
    0x10024, 0x003F, 0x10024, 0x0334, 0x10024, 0x0061, 0x10024, 0x0041,
    0x10024, 0x0062, 0x10025, 0x0021, 0x10025, 0x003F, 0x10025, 0x0334,
    0x10025, 0x0061, 0x10025, 0x0041, 0x10025, 0x0062, 0x10026, 0x0021,
    0x10026, 0x003F, 0x10026, 0x0334, 0x10026, 0x0061, 0x10026, 0x0041,
    0x10026, 0x0062, 0x10028, 0x0021, 0x10028, 0x003F, 0x10028, 0x0334,
    0x10028, 0x0061, 0x10028, 0x0041, 0x10028, 0x0062, 0x10029, 0x0021,
    0x10029, 0x003F, 0x10029, 0x0334, 0x10029, 0x0061, 0x10029, 0x0041,
    0x10029, 0x0062, 0x1002A, 0x0021, 0x1002A, 0x003F, 0x1002A, 0x0334,
    0x1002A, 0x0061, 0x1002A, 0x0041, 0x1002A, 0x0062, 0x1002B, 0x0021,
    0x1002B, 0x003F, 0x1002B, 0x0334, 0x1002B, 0x0061, 0x1002B, 0x0041,
    0x1002B, 0x0062, 0x1002C, 0x0021, 0x1002C, 0x003F, 0x1002C, 0x0334,
    0x1002C, 0x0061, 0x1002C, 0x0041, 0x1002C, 0x0062, 0x1002D, 0x0021,
    0x1002D, 0x003F, 0x1002D, 0x0334, 0x1002D, 0x0061, 0x1002D, 0x0041,
    0x1002D, 0x0062, 0x1002E, 0x0021, 0x1002E, 0x003F, 0x1002E, 0x0334,
    0x1002E, 0x0061, 0x1002E, 0x0041, 0x1002E, 0x0062, 0x1002F, 0x0021,
    0x1002F, 0x003F, 0x1002F, 0x0334, 0x1002F, 0x0061, 0x1002F, 0x0041,
    0x1002F, 0x0062, 0x10030, 0x0021, 0x10030, 0x003F, 0x10030, 0x0334,
    0x10030, 0x0061, 0x10030, 0x0041, 0x10030, 0x0062, 0x10031, 0x0021,
    0x10031, 0x003F, 0x10031, 0x0334, 0x10031, 0x0061, 0x10031, 0x0041,
    0x10031, 0x0062, 0x10032, 0x0021, 0x10032, 0x003F, 0x10032, 0x0334,
    0x10032, 0x0061, 0x10032, 0x0041, 0x10032, 0x0062, 0x10033, 0x0021,
    0x10033, 0x003F, 0x10033, 0x0334, 0x10033, 0x0061, 0x10033, 0x0041,
    0x10033, 0x0062, 0x10034, 0x0021, 0x10034, 0x003F, 0x10034, 0x0334,
    0x10034, 0x0061, 0x10034, 0x0041, 0x10034, 0x0062, 0x10035, 0x0021,
    0x10035, 0x003F, 0x10035, 0x0334, 0x10035, 0x0061, 0x10035, 0x0041,
    0x10035, 0x0062, 0x10036, 0x0021, 0x10036, 0x003F, 0x10036, 0x0334,
    0x10036, 0x0061, 0x10036, 0x0041, 0x10036, 0x0062, 0x10037, 0x0021,
    0x10037, 0x003F, 0x10037, 0x0334, 0x10037, 0x0061, 0x10037, 0x0041,
    0x10037, 0x0062, 0x10038, 0x0021, 0x10038, 0x003F, 0x10038, 0x0334,
    0x10038, 0x0061, 0x10038, 0x0041, 0x10038, 0x0062, 0x10039, 0x0021,
    0x10039, 0x003F, 0x10039, 0x0334, 0x10039, 0x0061, 0x10039, 0x0041,
    0x10039, 0x0062, 0x1003A, 0x0021, 0x1003A, 0x003F, 0x1003A, 0x0334,
    0x1003A, 0x0061, 0x1003A, 0x0041, 0x1003A, 0x0062, 0x1003C, 0x0021,
    0x1003C, 0x003F, 0x1003C, 0x0334, 0x1003C, 0x0061, 0x1003C, 0x0041,
    0x1003C, 0x0062, 0x1003D, 0x0021, 0x1003D, 0x003F, 0x1003D, 0x0334,
    0x1003D, 0x0061, 0x1003D, 0x0041, 0x1003D, 0x0062, 0x1003F, 0x0021,
    0x1003F, 0x003F, 0x1003F, 0x0334, 0x1003F, 0x0061, 0x1003F, 0x0041,
    0x1003F, 0x0062, 0x10040, 0x0021, 0x10040, 0x003F, 0x10040, 0x0334,
    0x10040, 0x0061, 0x10040, 0x0041, 0x10040, 0x0062, 0x10041, 0x0021,
    0x10041, 0x003F, 0x10041, 0x0334, 0x10041, 0x0061, 0x10041, 0x0041,
    0x10041, 0x0062, 0x10042, 0x0021, 0x10042, 0x003F, 0x10042, 0x0334,
    0x10042, 0x0061, 0x10042, 0x0041, 0x10042, 0x0062, 0x10043, 0x0021,
    0x10043, 0x003F, 0x10043, 0x0334, 0x10043, 0x0061, 0x10043, 0x0041,
    0x10043, 0x0062, 0x10044, 0x0021, 0x10044, 0x003F, 0x10044, 0x0334,
    0x10044, 0x0061, 0x10044, 0x0041, 0x10044, 0x0062, 0x10045, 0x0021,
    0x10045, 0x003F, 0x10045, 0x0334, 0x10045, 0x0061, 0x10045, 0x0041,
    0x10045, 0x0062, 0x10046, 0x0021, 0x10046, 0x003F, 0x10046, 0x0334,
    0x10046, 0x0061, 0x10046, 0x0041, 0x10046, 0x0062, 0x10047, 0x0021,
    0x10047, 0x003F, 0x10047, 0x0334, 0x10047, 0x0061, 0x10047, 0x0041,
    0x10047, 0x0062, 0x10048, 0x0021, 0x10048, 0x003F, 0x10048, 0x0334,
    0x10048, 0x0061, 0x10048, 0x0041, 0x10048, 0x0062, 0x10049, 0x0021,
    0x10049, 0x003F, 0x10049, 0x0334, 0x10049, 0x0061, 0x10049, 0x0041,
    0x10049, 0x0062, 0x1004A, 0x0021, 0x1004A, 0x003F, 0x1004A, 0x0334,
    0x1004A, 0x0061, 0x1004A, 0x0041, 0x1004A, 0x0062, 0x1004B, 0x0021,
    0x1004B, 0x003F, 0x1004B, 0x0334, 0x1004B, 0x0061, 0x1004B, 0x0041,
    0x1004B, 0x0062, 0x1004C, 0x0021, 0x1004C, 0x003F, 0x1004C, 0x0334,
    0x1004C, 0x0061, 0x1004C, 0x0041, 0x1004C, 0x0062, 0x1004D, 0x0021,
    0x1004D, 0x003F, 0x1004D, 0x0334, 0x1004D, 0x0061, 0x1004D, 0x0041,
    0x1004D, 0x0062, 0x10050, 0x0021, 0x10050, 0x003F, 0x10050, 0x0334,
    0x10050, 0x0061, 0x10050, 0x0041, 0x10050, 0x0062, 0x10051, 0x0021,
    0x10051, 0x003F, 0x10051, 0x0334, 0x10051, 0x0061, 0x10051, 0x0041,
    0x10051, 0x0062, 0x10052, 0x0021, 0x10052, 0x003F, 0x10052, 0x0334,
    0x10052, 0x0061, 0x10052, 0x0041, 0x10052, 0x0062, 0x10053, 0x0021,
    0x10053, 0x003F, 0x10053, 0x0334, 0x10053, 0x0061, 0x10053, 0x0041,
    0x10053, 0x0062, 0x10054, 0x0021, 0x10054, 0x003F, 0x10054, 0x0334,
    0x10054, 0x0061, 0x10054, 0x0041, 0x10054, 0x0062, 0x10055, 0x0021,
    0x10055, 0x003F, 0x10055, 0x0334, 0x10055, 0x0061, 0x10055, 0x0041,
    0x10055, 0x0062, 0x10056, 0x0021, 0x10056, 0x003F, 0x10056, 0x0334,
    0x10056, 0x0061, 0x10056, 0x0041, 0x10056, 0x0062, 0x10057, 0x0021,
    0x10057, 0x003F, 0x10057, 0x0334, 0x10057, 0x0061, 0x10057, 0x0041,
    0x10057, 0x0062, 0x10058, 0x0021, 0x10058, 0x003F, 0x10058, 0x0334,
    0x10058, 0x0061, 0x10058, 0x0041, 0x10058, 0x0062, 0x10059, 0x0021,
    0x10059, 0x003F, 0x10059};

int const num_cps = std::end(cps) - std::begin(cps);

TEST(sentinel_apis, normalize_nfd)
{
    std::u8string utf8;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(utf8));
    std::vector<char16_t> utf16;
    text::transcode_to_utf16(
        std::begin(cps), std::end(cps), std::back_inserter(utf16));
    std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

    // range overload
    std::vector<char32_t> result1;
    text::normalize<text::nf::d>(cps_copy, std::back_inserter(result1));

    // char32_t iterator/char32_t iterator
    std::vector<char32_t> result2;
    text::normalize<text::nf::d>(
        std::begin(cps), std::end(cps), std::back_inserter(result2));

    EXPECT_EQ(result2, result1);

    // utf_8_to_32_iterator/sentinel
    std::vector<char32_t> result3;
    auto utf8_it = text::utf32_iterator(
        &*utf8.begin(), &*utf8.begin(), text::null_sentinel);
    text::normalize<text::nf::d>(
        utf8_it, text::null_sentinel, std::back_inserter(result3));

    EXPECT_EQ(result3, result1);

    // utf_8_to_32_iterator/utf_8_to_32_iterator
    std::vector<char32_t> result4;
    auto utf8_end = text::utf32_iterator(
        &*utf8.begin(), &*utf8.begin() + utf8.size(), text::null_sentinel);
    text::normalize<text::nf::d>(
        utf8_it, utf8_end, std::back_inserter(result4));

    EXPECT_EQ(result4, result1);

    // utf_16_to_32_iterator/utf_16_to_32_iterator
    std::vector<char32_t> result5;
    auto utf16_it = text::utf32_iterator(
        utf16.begin(), utf16.begin(), utf16.end());
    auto utf16_end = text::utf32_iterator(
        utf16.begin(), utf16.end(), utf16.end());
    text::normalize<text::nf::d>(
        utf16_it, utf16_end, std::back_inserter(result5));

    EXPECT_EQ(result5, result1);
}

TEST(sentinel_apis, normalize_nfkd)
{
    std::u8string utf8;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(utf8));
    std::vector<char16_t> utf16;
    text::transcode_to_utf16(
        std::begin(cps), std::end(cps), std::back_inserter(utf16));
    std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

    // range overload
    std::vector<char32_t> result1;
    text::normalize<text::nf::kd>(cps_copy, std::back_inserter(result1));

    // char32_t iterator/char32_t iterator
    std::vector<char32_t> result2;
    text::normalize<text::nf::kd>(
        std::begin(cps), std::end(cps), std::back_inserter(result2));

    EXPECT_EQ(result2, result1);

    // utf_8_to_32_iterator/sentinel
    std::vector<char32_t> result3;
    auto utf8_it = text::utf32_iterator(
        &*utf8.begin(), &*utf8.begin(), text::null_sentinel);
    text::normalize<text::nf::kd>(
        utf8_it, text::null_sentinel, std::back_inserter(result3));

    EXPECT_EQ(result3, result1);

    // utf_8_to_32_iterator/utf_8_to_32_iterator
    std::vector<char32_t> result4;
    auto utf8_end = text::utf32_iterator(
        &*utf8.begin(), &*utf8.begin() + utf8.size(), text::null_sentinel);
    text::normalize<text::nf::kd>(
        utf8_it, utf8_end, std::back_inserter(result4));

    EXPECT_EQ(result4, result1);

    // utf_16_to_32_iterator/utf_16_to_32_iterator
    std::vector<char32_t> result5;
    auto utf16_it = text::utf32_iterator(
        utf16.begin(), utf16.begin(), utf16.end());
    auto utf16_end = text::utf32_iterator(
        utf16.begin(), utf16.end(), utf16.end());
    text::normalize<text::nf::kd>(
        utf16_it, utf16_end, std::back_inserter(result5));

    EXPECT_EQ(result5, result1);
}

TEST(sentinel_apis, normalize_nfc)
{
    std::u8string utf8;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(utf8));
    std::vector<char16_t> utf16;
    text::transcode_to_utf16(
        std::begin(cps), std::end(cps), std::back_inserter(utf16));
    std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

    // range overload
    std::vector<char32_t> result1;
    text::normalize<text::nf::c>(cps_copy, std::back_inserter(result1));

    // char32_t iterator/char32_t iterator
    std::vector<char32_t> result2;
    text::normalize<text::nf::c>(
        std::begin(cps), std::end(cps), std::back_inserter(result2));

    EXPECT_EQ(result2, result1);

    // utf_8_to_32_iterator/sentinel
    std::vector<char32_t> result3;
    auto utf8_rng_0 = &*utf8.begin() | text::as_utf32;
    text::normalize<text::nf::c>(
        utf8_rng_0.begin(), text::null_sentinel, std::back_inserter(result3));

    EXPECT_EQ(result3, result1);

    // utf_8_to_32_iterator/utf_8_to_32_iterator
    std::vector<char32_t> result4;
    auto utf8_rng_1 =
        std::ranges::subrange(&*utf8.begin(), &*utf8.begin() + utf8.size()) |
        text::as_utf32;
    text::normalize<text::nf::c>(
        utf8_rng_1.begin(), utf8_rng_1.end(), std::back_inserter(result4));

    EXPECT_EQ(result4, result1);

    // utf_16_to_32_iterator/utf_16_to_32_iterator
    std::vector<char32_t> result5;
    auto utf16_it = text::utf32_iterator(
        utf16.begin(), utf16.begin(), utf16.end());
    auto utf16_end = text::utf32_iterator(
        utf16.begin(), utf16.end(), utf16.end());
    text::normalize<text::nf::c>(
        utf16_it, utf16_end, std::back_inserter(result5));

    EXPECT_EQ(result5, result1);
}

TEST(sentinel_apis, normalize_nfkc)
{
    std::u8string utf8;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(utf8));
    std::vector<char16_t> utf16;
    text::transcode_to_utf16(
        std::begin(cps), std::end(cps), std::back_inserter(utf16));
    std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

    // range overload
    std::vector<char32_t> result1;
    text::normalize<text::nf::kc>(cps_copy, std::back_inserter(result1));

    // char32_t iterator/char32_t iterator
    std::vector<char32_t> result2;
    text::normalize<text::nf::kc>(
        std::begin(cps), std::end(cps), std::back_inserter(result2));

    EXPECT_EQ(result2, result1);

    // utf_8_to_32_iterator/sentinel
    std::vector<char32_t> result3;
    auto utf8_rng_0 = &*utf8.begin() | text::as_utf32;
    text::normalize<text::nf::kc>(
        utf8_rng_0.begin(), text::null_sentinel, std::back_inserter(result3));

    EXPECT_EQ(result3, result1);

    // utf_8_to_32_iterator/utf_8_to_32_iterator
    std::vector<char32_t> result4;
    auto utf8_rng_1 =
        std::ranges::subrange(&*utf8.begin(), &*utf8.begin() + utf8.size()) |
        text::as_utf32;
    text::normalize<text::nf::kc>(
        utf8_rng_1.begin(), utf8_rng_1.end(), std::back_inserter(result4));

    EXPECT_EQ(result4, result1);

    // utf_16_to_32_iterator/utf_16_to_32_iterator
    std::vector<char32_t> result5;
    auto utf16_it = text::utf32_iterator(
        utf16.begin(), utf16.begin(), utf16.end());
    auto utf16_end = text::utf32_iterator(
        utf16.begin(), utf16.end(), utf16.end());
    text::normalize<text::nf::kc>(
        utf16_it, utf16_end, std::back_inserter(result5));

    EXPECT_EQ(result5, result1);
}

TEST(sentinel_apis, normalize_fcc)
{
    std::u8string utf8;
    text::transcode_to_utf8(
        std::begin(cps), std::end(cps), std::back_inserter(utf8));
    std::vector<char16_t> utf16;
    text::transcode_to_utf16(
        std::begin(cps), std::end(cps), std::back_inserter(utf16));
    std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

    // range overload
    std::vector<char32_t> result1;
    text::normalize<text::nf::fcc>(cps_copy, std::back_inserter(result1));

    // char32_t iterator/char32_t iterator
    std::vector<char32_t> result2;
    text::normalize<text::nf::fcc>(
        std::begin(cps), std::end(cps), std::back_inserter(result2));

    EXPECT_EQ(result2, result1);

    // utf_8_to_32_iterator/sentinel
    std::vector<char32_t> result3;
    auto utf8_rng_0 = utf8.c_str() | text::as_utf32;
    text::normalize<text::nf::fcc>(
        utf8_rng_0.begin(), text::null_sentinel, std::back_inserter(result3));

    EXPECT_EQ(result3, result1);

    EXPECT_TRUE(text::is_normalized<text::nf::fcc>(utf8.c_str()));
    EXPECT_TRUE(text::is_normalized<text::nf::fcc>(
        std::ranges::subrange(utf8.c_str(), text::null_sentinel)));

    // utf_8_to_32_iterator/utf_8_to_32_iterator
    std::vector<char32_t> result4;
    auto utf8_rng_1 =
        std::ranges::subrange(&*utf8.begin(), &*utf8.begin() + utf8.size()) |
        text::as_utf32;
    text::normalize<text::nf::fcc>(
        utf8_rng_1.begin(), utf8_rng_1.end(), std::back_inserter(result4));

    EXPECT_EQ(result4, result1);

    // utf_16_to_32_iterator/utf_16_to_32_iterator
    std::vector<char32_t> result5;
    auto utf16_it = text::utf32_iterator(
        utf16.begin(), utf16.begin(), utf16.end());
    auto utf16_end = text::utf32_iterator(
        utf16.begin(), utf16.end(), utf16.end());
    text::normalize<text::nf::fcc>(
        utf16_it, utf16_end, std::back_inserter(result5));

    EXPECT_EQ(result5, result1);

    EXPECT_TRUE(text::is_normalized<text::nf::fcc>(result5));
    EXPECT_TRUE(
        text::is_normalized<text::nf::fcc>(result5.begin(), result5.end()));
}

TEST(transcoding, output_iterators)
{
    // 8 -> 32
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result(num_cps);
        auto const out = std::copy(
            utf8.begin(), utf8.end(), text::utf_8_to_32_out(result.begin()));
        EXPECT_EQ(out.base() - result.begin(), num_cps);
        EXPECT_EQ(cps_copy, result);
    }
    // 8 -> 16
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result(num_cps * 2);
        auto const out = std::copy(
            utf8.begin(), utf8.end(), text::utf_8_to_16_out(result.begin()));
        result.erase(out.base(), result.end());

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 16 -> 32
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result(num_cps);
        auto const out = std::copy(
            utf16.begin(), utf16.end(), text::utf_16_to_32_out(result.begin()));
        EXPECT_EQ(out.base() - result.begin(), num_cps);
        EXPECT_EQ(cps_copy, result);
    }
    // 16 -> 8
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result(num_cps * 4);
        auto const out = std::copy(
            utf16.begin(), utf16.end(), text::utf_16_to_8_out(result.begin()));
        result.erase(out.base(), result.end());

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }

    // 32 -> 8
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result(num_cps * 4);
        auto const out = std::copy(
            std::begin(cps),
            std::end(cps),
            text::utf_32_to_8_out(result.begin()));
        result.erase(out.base(), result.end());

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 32 -> 16
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result(num_cps * 2);
        auto const out = std::copy(
            std::begin(cps),
            std::end(cps),
            text::utf_32_to_16_out(result.begin()));
        result.erase(out.base(), result.end());

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
}

TEST(transcoding, insert_iterators)
{
    // 8 -> 32
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result;
        std::copy(
            utf8.begin(),
            utf8.end(),
            text::from_utf8_inserter(result, result.begin()));
        EXPECT_EQ(cps_copy, result);
    }
    // 8 -> 16
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result;
        std::copy(
            utf8.begin(),
            utf8.end(),
            text::from_utf8_inserter(result, result.begin()));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 16 -> 32
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_inserter(result, result.begin()));
        EXPECT_EQ(cps_copy, result);
    }
    // 16 -> 8
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_inserter(result, result.begin()));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }

    // 32 -> 8
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_inserter(result, result.begin()));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 32 -> 16
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_inserter(result, result.begin()));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
}

TEST(transcoding, front_insert_iterators)
{
    // 8 -> 32
    {
        std::deque<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char32_t> result;
        std::copy(
            utf8.begin(), utf8.end(), text::from_utf8_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(cps_copy, result);
    }
    // 8 -> 16
    {
        std::deque<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char16_t> result;
        std::copy(
            utf8.begin(), utf8.end(), text::from_utf8_front_inserter(result));
        std::reverse(result.begin(), result.end());

        std::deque<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 16 -> 32
    {
        std::deque<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char32_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_front_inserter(result));
        std::reverse(result.begin(), result.end());
        EXPECT_EQ(cps_copy, result);
    }
    // 16 -> 8
    {
        std::deque<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char8_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_front_inserter(result));
        std::reverse(result.begin(), result.end());

        std::deque<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }

    // 32 -> 8
    {
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char8_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_front_inserter(result));
        std::reverse(result.begin(), result.end());

        std::deque<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 32 -> 16
    {
        std::deque<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::deque<char16_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_front_inserter(result));
        std::reverse(result.begin(), result.end());

        std::deque<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
}

TEST(transcoding, back_insert_iterators)
{
    // 8 -> 32
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result;
        std::copy(
            utf8.begin(), utf8.end(), text::from_utf8_back_inserter(result));
        EXPECT_EQ(cps_copy, result);
    }
    // 8 -> 16
    {
        std::vector<char8_t> utf8;
        text::transcode_to_utf8(
            std::begin(cps), std::end(cps), std::back_inserter(utf8));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result;
        std::copy(
            utf8.begin(), utf8.end(), text::from_utf8_back_inserter(result));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 16 -> 32
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char32_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_back_inserter(result));
        EXPECT_EQ(cps_copy, result);
    }
    // 16 -> 8
    {
        std::vector<char16_t> utf16;
        text::transcode_to_utf16(
            std::begin(cps), std::end(cps), std::back_inserter(utf16));
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result;
        std::copy(
            utf16.begin(),
            utf16.end(),
            text::from_utf16_back_inserter(result));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }

    // 32 -> 8
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char8_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_back_inserter(result));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
    // 32 -> 16
    {
        std::vector<char32_t> const cps_copy(std::begin(cps), std::end(cps));

        std::vector<char16_t> result;
        std::copy(
            std::begin(cps),
            std::end(cps),
            text::from_utf32_back_inserter(result));

        std::vector<char32_t> cps_copy_from_result;
        text::transcode_to_utf32(
            result, std::back_inserter(cps_copy_from_result));
        EXPECT_EQ(cps_copy_from_result, cps_copy);
    }
}

struct my_text_type
{
    my_text_type() = default;
    my_text_type(std::u8string utf8) : utf8_(std::move(utf8)) {}

    auto begin() const
    {
        return text::utf_8_to_32_iterator(
            utf8_.begin(), utf8_.begin(), utf8_.end());
    }
    auto end() const
    {
        return text::utf_8_to_32_iterator(
            utf8_.begin(), utf8_.end(), utf8_.end());
    }

private:
    std::u8string utf8_;
};

TEST(transcoding, normalize_adaptors)
{
    static_assert(std::is_same_v<
                  decltype(my_text_type(u8"text") | boost::text::as_nfc),
                  boost::text::nfc_view<
                      boost::text::utf32_view<boost::text::unpacking_view<
                          std::ranges::owning_view<my_text_type>>>>>);

    static_assert(std::is_same_v<
                  decltype(u8"text" | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<
                      std::ranges::subrange<const char8_t *>>>>);

    static_assert(std::is_same_v<
                  decltype(std::u8string(u8"text") | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<
                      std::ranges::owning_view<std::u8string>>>>);

    std::u8string const str = u8"text";

    static_assert(std::is_same_v<
                  decltype(str | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<
                      std::ranges::ref_view<std::u8string const>>>>);

    static_assert(std::is_same_v<
                  decltype(str.c_str() | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<std::ranges::subrange<
                      const char8_t *,
                      boost::text::null_sentinel_t>>>>);

    static_assert(std::is_same_v<
                  decltype(std::ranges::empty_view<int>{} | boost::text::as_char16_t),
                  std::ranges::empty_view<char16_t>>);

    std::u16string str2 = u"text";

    static_assert(std::is_same_v<
                  decltype(str2 | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<
                      std::ranges::ref_view<std::u16string>>>>);

    static_assert(std::is_same_v<
                  decltype(str2.c_str() | boost::text::as_nfc),
                  boost::text::nfc_view<boost::text::utf32_view<std::ranges::subrange<
                      const char16_t *,
                      boost::text::null_sentinel_t>>>>);
}

TEST(transcoding, normalize_view_bidi)
{
    // 0061 0315 0300 05AE 1DF3 0062;00E0 05AE 1DF3 0315 0062;0061 05AE 0300 1DF3 0315 0062;00E0 05AE 1DF3 0315 0062;0061 05AE 0300 1DF3 0315 0062;
    // (a◌̕◌̀◌֮◌ᷳb; à◌֮◌ᷳ◌̕b; a◌֮◌̀◌ᷳ◌̕b; à◌֮◌ᷳ◌̕b; a◌֮◌̀◌ᷳ◌̕b; ) LATIN SMALL LETTER A, COMBINING COMMA ABOVE RIGHT, COMBINING GRAVE ACCENT, HEBREW ACCENT ZINOR, COMBINING LATIN SMALL LETTER O WITH DIAERESIS, LATIN SMALL LETTER B
    std::array<char32_t, 6> const un_norm = {
        {0x0061, 0x0315, 0x0300, 0x05AE, 0x1DF3, 0x0062}};
    std::array<char32_t, 5> const nfc = {
        {0x00E0, 0x05AE, 0x1DF3, 0x0315, 0x0062}};

    auto v = un_norm | text::as_nfc;
    std::vector<decltype(v.begin())> fwd_iterators;
    auto it = v.begin();
    auto nfc_it = nfc.begin();
    int i = 0;
    for (auto last = v.end(); it != last; ++it, ++nfc_it, ++i) {
        EXPECT_EQ(*it, nfc[i]) << "element " << i;
        fwd_iterators.push_back(it);
    }
    fwd_iterators.push_back(it);

    it = v.end();
    std::vector<decltype(v.begin())> rev_iterators;
    i = nfc.size();
    for (auto first = v.begin(); it != first; --it, --i) {
        if (it != v.end()) {
            EXPECT_EQ(*it, nfc[i]) << "element " << i;
        }
        rev_iterators.push_back(it);
    }
    EXPECT_EQ(*it, nfc[i]) << "element " << i;
    rev_iterators.push_back(it);
    std::ranges::reverse(rev_iterators);

    EXPECT_EQ(fwd_iterators.size(), rev_iterators.size());

    for (int i = 0; i != (int)fwd_iterators.size(); ++i) {
        EXPECT_TRUE(fwd_iterators[i] == rev_iterators[i]) << "iteration " << i;
    }
}
