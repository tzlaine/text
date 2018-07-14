#include <boost/text/case_mapping.hpp>

#include <boost/text/utf8.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;

using u32_iter = utf8::to_utf32_iterator<char const *, char const *>;
using sentinel_cp_range_t = cp_range<u32_iter, utf8::null_sentinel>;

void to_sentinel_cp_range(
    string & s, sentinel_cp_range_t & r, std::vector<uint32_t> cps)
{
    s = to_string(cps.begin(), cps.end());
    r = sentinel_cp_range_t{u32_iter(s.begin(), s.begin(), s.end()),
                   utf8::null_sentinel{}};
}


TEST(case_mapping_api, all)
{
    // Taken from case_mapping.cpp case 000.

    // 00DF; 00DF; 0053 0073; 0053 0053; # LATIN SMALL LETTER SHARP S
    string cp_;
    sentinel_cp_range_t cp;
    to_sentinel_cp_range(cp_, cp, {0x00DF});


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

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title({0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title({0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0020, 0x0020, 0x0020, 0x00DF});
        std::vector<uint32_t> title({0x0020, 0x0020, 0x0020, 0x0053, 0x0073});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0020, 0x0020, 0x0020, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title({0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0020, 0x0020, 0x0020, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title({0x0020, 0x0020, 0x0020, 0x0053, 0x0073, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0061, 0x0061, 0x0061, 0x00DF});
        std::vector<uint32_t> title({0x0041, 0x0061, 0x0061, 0x00DF});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0061, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> title({0x0041, 0x0061, 0x0061, 0x00DF, 0x0020, 0x0020, 0x0020});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_title
        string from_;
        sentinel_cp_range_t from;
        to_sentinel_cp_range(from_, from, {0x0061, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> title({0x0041, 0x0061, 0x0061, 0x00DF, 0x0061, 0x0061, 0x0061});
        std::vector<uint32_t> result;

        to_title(from, std::back_inserter(result));
        EXPECT_EQ(result, title);
    }
    { // to_upper
        std::vector<uint32_t> upper({0x0053, 0x0053});
        std::vector<uint32_t> result;

        to_upper(cp, std::back_inserter(result));
        EXPECT_EQ(result, upper);
    }
}
