#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/grapheme_view.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;

uint32_t cps_0[2] = {1, 0};
uint32_t const (&cps_1)[2] = cps_0;

TEST(break_apis, grapheme_break)
{
    {
        auto r_0 = as_graphemes(cps_0);
        auto r_1 = as_graphemes(cps_1);

        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }
    {
        auto r_0 = as_graphemes((uint32_t *)cps_0);
        auto r_1 = as_graphemes((uint32_t const *)cps_1);

        EXPECT_EQ(r_0.begin(), r_0.begin());
        EXPECT_NE(r_0.begin(), r_0.end());

        EXPECT_EQ(r_0.begin(), r_1.begin());
        EXPECT_NE(r_0.begin(), r_1.end());
    }
}
