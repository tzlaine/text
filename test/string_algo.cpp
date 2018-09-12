#include <boost/text/text.hpp>
#include <boost/text/utility.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <vector>

#include <gtest/gtest.h>


struct cp_range_cmp
{
    template<typename CPRange>
    bool operator()(CPRange lhs, CPRange rhs) const noexcept
    {
        return boost::algorithm::equal(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
};

TEST(string_algo, text_starts_with)
{
    boost::text::text t_1("String to search");
    boost::text::text const t_2("String to");

    EXPECT_TRUE(boost::algorithm::starts_with(t_1, t_2));
    EXPECT_TRUE(boost::algorithm::starts_with(t_1, t_2, cp_range_cmp{}));
    EXPECT_FALSE(boost::algorithm::starts_with(t_2, t_1));
    EXPECT_FALSE(boost::algorithm::starts_with(t_2, t_1, cp_range_cmp{}));
}

struct is_any_of
{
    is_any_of(boost::text::text const & graphemes) : graphemes_(graphemes) {}

    template<typename CPRange>
    bool operator()(CPRange r) const noexcept
    {
        return std::find(graphemes_.begin(), graphemes_.end(), r) !=
               graphemes_.end();
    }

    boost::text::text const & graphemes_;
};

TEST(string_algo, text_split)
{
    boost::text::text const t("String to search");
    boost::text::text const break_graphemes("t ");

    {
        std::vector<boost::text::text_view> split_result;
        boost::algorithm::split(split_result, t, is_any_of(break_graphemes));
        EXPECT_EQ((int)split_result.size(), 5);
        EXPECT_EQ(split_result[0], boost::text::text("S"));
        EXPECT_EQ(split_result[1], boost::text::text("ring"));
        EXPECT_EQ(split_result[2], boost::text::text(""));
        EXPECT_EQ(split_result[3], boost::text::text("o"));
        EXPECT_EQ(split_result[4], boost::text::text("search"));
    }

    {
        std::vector<boost::text::text> split_result;
        boost::algorithm::split(split_result, t, is_any_of(break_graphemes));
        EXPECT_EQ((int)split_result.size(), 5);
        EXPECT_EQ(split_result[0], boost::text::text("S"));
        EXPECT_EQ(split_result[1], boost::text::text("ring"));
        EXPECT_EQ(split_result[2], boost::text::text(""));
        EXPECT_EQ(split_result[3], boost::text::text("o"));
        EXPECT_EQ(split_result[4], boost::text::text("search"));
    }
}

TEST(string_algo, text_join)
{
    std::vector<boost::text::text> chunks = {
        boost::text::text("Three"),
        boost::text::text("separate"),
        boost::text::text("pieces."),
    };

    boost::text::text const result =
        boost::algorithm::join(chunks, boost::text::text(" "));
    EXPECT_EQ(result, boost::text::text("Three separate pieces."));
}

#if 0 // Does not work!  The internals of replace_first() attempt to do
      // *lhs_it = *rhs_it, which does not work for graphemes.
TEST(string_algo, text_replace_first)
{
    boost::text::text t("football");

    boost::algorithm::replace_first(
        t, boost::text::text("foo"), boost::text::text("bar"));

    EXPECT_EQ(t, boost::text::text("bartball"));
}
#endif
