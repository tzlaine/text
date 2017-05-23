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
        EXPECT_TRUE(tv < tv2);
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
        static_assert(tv < tv2, "");
    }
}

TEST(text_view, test_non_empty)
{
    text::text_view tv_a("a");
    text::text_view tv_ab("ab");

    EXPECT_EQ(tv_a.begin() + tv_a.size(), tv_a.end());
    EXPECT_EQ(tv_a.cbegin() + tv_a.size(), tv_a.cend());
    EXPECT_EQ(tv_a.rbegin() + tv_a.size(), tv_a.rend());
    EXPECT_EQ(tv_a.crbegin() + tv_a.size(), tv_a.crend());

    EXPECT_EQ(tv_a.begin(), tv_a.cbegin());
    EXPECT_EQ(tv_a.end(), tv_a.cend());
    EXPECT_EQ(tv_a.rbegin(), tv_a.crbegin());
    EXPECT_EQ(tv_a.rend(), tv_a.crend());

    EXPECT_EQ(tv_ab.begin(), tv_ab.cbegin());
    EXPECT_EQ(tv_ab.end(), tv_ab.cend());
    EXPECT_EQ(tv_ab.rbegin(), tv_ab.crbegin());
    EXPECT_EQ(tv_ab.rend(), tv_ab.crend());

    EXPECT_FALSE(tv_a.empty());
    EXPECT_EQ(tv_a.size(), 2);
    EXPECT_NE(tv_a.data(), nullptr);

    EXPECT_FALSE(tv_ab.empty());
    EXPECT_EQ(tv_ab.size(), 3);
    EXPECT_NE(tv_ab.data(), nullptr);

    EXPECT_EQ(tv_a.front(), 'a');
    EXPECT_EQ(tv_a.back(), '\0');
    EXPECT_EQ(tv_a[1], '\0');

    EXPECT_EQ(tv_ab.front(), 'a');
    EXPECT_EQ(tv_ab.back(), '\0');
    EXPECT_EQ(tv_ab[1], 'b');

    EXPECT_EQ(tv_a.max_size(), INT_MAX);
    EXPECT_EQ(tv_ab.max_size(), INT_MAX);

    EXPECT_EQ(tv_a.compare(tv_ab), -1);
    EXPECT_FALSE(tv_a == tv_ab);
    EXPECT_TRUE(tv_a != tv_ab);
    EXPECT_TRUE(tv_a < tv_ab);
    EXPECT_TRUE(tv_a <= tv_ab);
    EXPECT_FALSE(tv_a > tv_ab);
    EXPECT_FALSE(tv_a >= tv_ab);

    EXPECT_EQ(tv_a.compare("ab"), -1);
    EXPECT_EQ(tv_a, "a");

    EXPECT_EQ(swapped(tv_a, tv_ab).left, tv_ab);
    EXPECT_EQ(swapped(tv_a, tv_ab).right, tv_a);

    EXPECT_EQ(tv_a.begin(), begin(tv_a));
    EXPECT_EQ(tv_a.end(), end(tv_a));
    EXPECT_EQ(tv_a.cbegin(), cbegin(tv_a));
    EXPECT_EQ(tv_a.cend(), cend(tv_a));

    EXPECT_EQ(tv_a.rbegin(), rbegin(tv_a));
    EXPECT_EQ(tv_a.rend(), rend(tv_a));
    EXPECT_EQ(tv_a.crbegin(), crbegin(tv_a));
    EXPECT_EQ(tv_a.crend(), crend(tv_a));

    {
        using namespace text::literals;
        EXPECT_EQ(tv_a, "a"_tv);
        EXPECT_EQ(tv_ab, "ab"_tv);
    }
}

TEST(text_view, test_non_empty_constexpr)
{
    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_ab("ab");

    static_assert(tv_a.begin() + tv_a.size() == tv_a.end(), "");
    static_assert(tv_a.cbegin() + tv_a.size() == tv_a.cend(), "");
    static_assert(tv_a.rbegin() + tv_a.size() == tv_a.rend(), "");
    static_assert(tv_a.crbegin() + tv_a.size() == tv_a.crend(), "");

    static_assert(tv_a.begin() == tv_a.cbegin(), "");
    static_assert(tv_a.end() == tv_a.cend(), "");
    static_assert(tv_a.rbegin() == tv_a.crbegin(), "");
    static_assert(tv_a.rend() == tv_a.crend(), "");

    static_assert(tv_ab.begin() == tv_ab.cbegin(), "");
    static_assert(tv_ab.end() == tv_ab.cend(), "");
    static_assert(tv_ab.rbegin() == tv_ab.crbegin(), "");
    static_assert(tv_ab.rend() == tv_ab.crend(), "");

    static_assert(!tv_a.empty(), "");
    static_assert(tv_a.size() == 2, "");
    static_assert(tv_a.data() != nullptr, "");

    static_assert(!tv_ab.empty(), "");
    static_assert(tv_ab.size() == 3, "");
    static_assert(tv_ab.data() != nullptr, "");

    static_assert(tv_a.front() == 'a', "");
    static_assert(tv_a.back() == '\0', "");
    static_assert(tv_a[1] == '\0', "");

    static_assert(tv_ab.front() == 'a', "");
    static_assert(tv_ab.back() == '\0', "");
    static_assert(tv_ab[1] == 'b', "");

    static_assert(tv_a.max_size() == INT_MAX, "");
    static_assert(tv_ab.max_size() == INT_MAX, "");

    static_assert(tv_a.compare(tv_ab) == -1, "");
    static_assert(!(tv_a == tv_ab), "");
    static_assert(tv_a != tv_ab, "");
    static_assert(tv_a < tv_ab, "");
    static_assert(tv_a <= tv_ab, "");
    static_assert(!(tv_a > tv_ab), "");
    static_assert(!(tv_a >= tv_ab), "");

    static_assert(tv_a.compare("ab") == -1, "");
    static_assert(tv_a == "a", "");

    static_assert(swapped(tv_a, tv_ab).left == tv_ab, "");
    static_assert(swapped(tv_a, tv_ab).right == tv_a, "");

    static_assert(tv_a.begin() == begin(tv_a), "");
    static_assert(tv_a.end() == end(tv_a), "");
    static_assert(tv_a.cbegin() == cbegin(tv_a), "");
    static_assert(tv_a.cend() == cend(tv_a), "");

    static_assert(tv_a.rbegin() == rbegin(tv_a), "");
    static_assert(tv_a.rend() == rend(tv_a), "");
    static_assert(tv_a.crbegin() == crbegin(tv_a), "");
    static_assert(tv_a.crend() == crend(tv_a), "");

    {
        using namespace text::literals;
        static_assert(tv_a == "a"_tv, "");
        static_assert(tv_ab == "ab"_tv, "");
    }
}
