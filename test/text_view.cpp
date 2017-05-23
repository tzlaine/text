#include <boost/text/text_view.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(text_view, test_empty)
{
    text::text_view tv;

    EXPECT_EQ(tv.begin(), tv.end());
    EXPECT_EQ(tv.cbegin(), tv.cend());
    EXPECT_EQ(tv.rbegin(), tv.rend());
    EXPECT_EQ(tv.crbegin(), tv.crend());

    EXPECT_EQ(tv.begin(), tv.cbegin());
    EXPECT_EQ(tv.end(), tv.cend());
    EXPECT_EQ(tv.rbegin(), tv.crbegin());
    EXPECT_EQ(tv.rend(), tv.crend());

    EXPECT_TRUE(tv.empty());
    EXPECT_EQ(tv.size(), 0);
    EXPECT_EQ(tv.data(), nullptr);

    EXPECT_EQ(tv.max_size(), INT_MAX);

    EXPECT_EQ(tv.compare(tv), 0);
    EXPECT_TRUE(tv == tv);
    EXPECT_FALSE(tv != tv);
    EXPECT_FALSE(tv < tv);
    EXPECT_TRUE(tv <= tv);
    EXPECT_FALSE(tv > tv);
    EXPECT_TRUE(tv >= tv);

    tv.swap(tv);
    EXPECT_TRUE(tv == tv);

    EXPECT_EQ(tv.begin(), begin(tv));
    EXPECT_EQ(tv.end(), end(tv));
    EXPECT_EQ(tv.cbegin(), cbegin(tv));
    EXPECT_EQ(tv.cend(), cend(tv));

    EXPECT_EQ(tv.rbegin(), rbegin(tv));
    EXPECT_EQ(tv.rend(), rend(tv));
    EXPECT_EQ(tv.crbegin(), crbegin(tv));
    EXPECT_EQ(tv.crend(), crend(tv));

    std::cout << "tv=\"" << tv << "\"\n";

    {
        using namespace text::literals;
        constexpr text::text_view tv2 = ""_tv;
        EXPECT_EQ(tv, tv2);
    }
}

struct text_views
{
    text::text_view left;
    text::text_view right;
};

constexpr text_views swapped (text::text_view lhs, text::text_view rhs)
{
    lhs.swap(rhs);
    return text_views{lhs, rhs};
}

TEST(text_view, test_empty_constexpr)
{
    constexpr text::text_view tv;

    static_assert(tv.begin() == tv.end(), "");
    static_assert(tv.cbegin() == tv.cend(), "");
    static_assert(tv.rbegin() == tv.rend(), "");
    static_assert(tv.crbegin() == tv.crend(), "");

    static_assert(tv.begin() == tv.cbegin(), "");
    static_assert(tv.end() == tv.cend(), "");
    static_assert(tv.rbegin() == tv.crbegin(), "");
    static_assert(tv.rend() == tv.crend(), "");

    static_assert(tv.empty(), "");
    static_assert(tv.size() == 0, "");
    static_assert(tv.data() == nullptr, "");

    // constexpr char front () const noexcept
    // constexpr char back () const noexcept
    // constexpr char operator[] (int i) const noexcept

    static_assert(tv.max_size() == INT_MAX, "");

    static_assert(tv.compare(tv) == 0, "");
    static_assert(tv == tv, "");
    static_assert(!(tv != tv), "");
    static_assert(!(tv < tv), "");
    static_assert(tv <= tv, "");
    static_assert(!(tv > tv), "");
    static_assert(tv >= tv, "");

    static_assert(swapped(tv, tv).left == tv, "");
    static_assert(swapped(tv, tv).right == tv, "");

    static_assert(tv.begin() == begin(tv), "");
    static_assert(tv.end() == end(tv), "");
    static_assert(tv.cbegin() == cbegin(tv), "");
    static_assert(tv.cend() == cend(tv), "");

    static_assert(tv.rbegin() == rbegin(tv), "");
    static_assert(tv.rend() == rend(tv), "");
    static_assert(tv.crbegin() == crbegin(tv), "");
    static_assert(tv.crend() == crend(tv), "");

    {
        using namespace text::literals;
        constexpr text::text_view tv2 = ""_tv;
        static_assert(tv == tv2, "");
    }
}
