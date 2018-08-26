#include <boost/text/collate.hpp>
#include <boost/text/normalize_string.hpp>

#include <boost/text/utf8.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;

using u32_iter = utf8::to_utf32_iterator<char const *, char const *>;
using sentinel_cp_range_t =
    boost::text::cp_range<u32_iter, utf8::null_sentinel>;

void to_sentinel_cp_range(
    string & s,
    sentinel_cp_range_t & r,
    std::vector<uint32_t> cps,
    bool normalize = false)
{
    s = to_string(cps.begin(), cps.end());
    if (normalize)
        normalize_to_nfd(s);
    r = sentinel_cp_range_t{u32_iter(s.begin(), s.begin(), s.end()),
                   utf8::null_sentinel{}};
}


// This test also covers to_string()'s sentinel API.
TEST(sentinel_apis, nfd)
{
    // Taken from normalization_collation_api.cpp, first case.

    // 1E0A;1E0A;0044 0307;1E0A;0044 0307; 
    // (Ḋ; Ḋ; D◌̇; Ḋ; D◌̇; ) LATIN CAPITAL LETTER D WITH DOT ABOVE
    {
        string c1_;
        sentinel_cp_range_t c1;
        to_sentinel_cp_range(c1_, c1, {0x1E0A});

        string c2_;
        sentinel_cp_range_t c2;
        to_sentinel_cp_range(c2_, c2, {0x1E0A});

        string c3_;
        sentinel_cp_range_t c3;
        to_sentinel_cp_range(c3_, c3, {0x0044, 0x0307});

        string c4_;
        sentinel_cp_range_t c4;
        to_sentinel_cp_range(c4_, c4, {0x1E0A});

        string c5_;
        sentinel_cp_range_t c5;
        to_sentinel_cp_range(c5_, c5, {0x0044, 0x0307});

        EXPECT_TRUE(normalized_nfc(c2.begin(), c2.end()));
        EXPECT_TRUE(normalized_nfkc(c2.begin(), c2.end()));

        EXPECT_TRUE(normalized_nfd(c3.begin(), c3.end()));
        EXPECT_TRUE(normalized_nfkd(c3.begin(), c3.end()));

        EXPECT_TRUE(normalized_nfc(c4.begin(), c4.end()));
        EXPECT_TRUE(normalized_nfkc(c4.begin(), c4.end()));

        EXPECT_TRUE(normalized_nfd(c5.begin(), c5.end()));
        EXPECT_TRUE(normalized_nfkd(c5.begin(), c5.end()));
    }

    {
        string c1_;
        sentinel_cp_range_t c1;
        to_sentinel_cp_range(c1_, c1, {0x1E0A}, true);

        string c2_;
        sentinel_cp_range_t c2;
        to_sentinel_cp_range(c2_, c2, {0x1E0A}, true);

        string c3_;
        sentinel_cp_range_t c3;
        to_sentinel_cp_range(c3_, c3, {0x0044, 0x0307}, true);

        string c4_;
        sentinel_cp_range_t c4;
        to_sentinel_cp_range(c4_, c4, {0x1E0A}, true);

        string c5_;
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

TEST(sentinel_apis, collation)
{
    // Taken from relative_collation_test_non_ignorable.cpp, first iteration.

    string cps_;
    sentinel_cp_range_t cps;
    to_sentinel_cp_range(cps_, cps, {0x0338, 0x0334}, true);

    std::vector<uint32_t> other_cps({0x0338, 0x0334});

    collate(
        cps.begin(),
        cps.end(),
        other_cps.begin(),
        other_cps.end(),
        default_collation_table(),
        collation_strength::identical,
        case_first::off,
        case_level::off,
        variable_weighting::non_ignorable);

    collation_sort_key(cps, default_collation_table(), collation_flags{});

    collate(cps, other_cps, default_collation_table(), collation_flags{});
}
