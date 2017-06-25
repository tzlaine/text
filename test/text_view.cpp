#include <boost/text/text_view.hpp>

#include <gtest/gtest.h>

#include <iomanip>


using namespace boost;

TEST(text_view, test_empty)
{
    text::text_view tv;

    EXPECT_EQ(tv.begin(), tv.end());
    EXPECT_EQ(tv.rbegin(), tv.rend());

    EXPECT_TRUE(tv.empty());
    EXPECT_EQ(tv.size(), 0);
    EXPECT_EQ(tv.begin(), nullptr);

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

    EXPECT_EQ(tv.rbegin(), rbegin(tv));
    EXPECT_EQ(tv.rend(), rend(tv));

    std::cout << "tv=\"" << tv << "\"\n";

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

    {
        using namespace text::literals;

        constexpr text::text_view tv2 = ""_tv;
        EXPECT_TRUE(tv == tv2);

        constexpr text::text_view tv3 = u8""_tv;
        EXPECT_TRUE(tv == tv3);
    }

#endif
}

struct text_views
{
    text::text_view left;
    text::text_view right;
};

BOOST_TEXT_CXX14_CONSTEXPR text_views swapped (text::text_view lhs, text::text_view rhs)
{
    lhs.swap(rhs);
    return text_views{lhs, rhs};
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

TEST(text_view, test_empty_constexpr)
{
    constexpr text::text_view tv;

    static_assert(tv.begin() == tv.end(), "");
    static_assert(tv.rbegin() == tv.rend(), "");

    static_assert(tv.empty(), "");
    static_assert(tv.size() == 0, "");
    static_assert(tv.begin() == nullptr, "");

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

    static_assert(tv.rbegin() == rbegin(tv), "");
    static_assert(tv.rend() == rend(tv), "");

    {
        using namespace text::literals;
        constexpr text::text_view tv2 = ""_tv;
        static_assert(tv == tv2, "");
    }
}

#endif

TEST(text_view, test_non_empty)
{
    text::text_view tv_a("a");
    text::text_view tv_ab("ab");

    EXPECT_EQ(tv_a.begin() + tv_a.size(), tv_a.end());
    EXPECT_EQ(tv_a.rbegin() + tv_a.size(), tv_a.rend());

    EXPECT_FALSE(tv_a.empty());
    EXPECT_EQ(tv_a.size(), 1);
    EXPECT_NE(tv_a.begin(), nullptr);

    EXPECT_FALSE(tv_ab.empty());
    EXPECT_EQ(tv_ab.size(), 2);
    EXPECT_NE(tv_ab.begin(), nullptr);

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

    EXPECT_EQ(tv_a.rbegin(), rbegin(tv_a));
    EXPECT_EQ(tv_a.rend(), rend(tv_a));

    {
        using namespace text::literals;
        EXPECT_EQ(tv_a, "a"_tv);
        EXPECT_EQ(tv_ab, u8"ab"_tv);
    }
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

TEST(text_view, test_non_empty_constexpr)
{
    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_ab("ab");

    static_assert(tv_a.begin() + tv_a.size() == tv_a.end(), "");
    static_assert(tv_a.rbegin() + tv_a.size() == tv_a.rend(), "");

    static_assert(!tv_a.empty(), "");
    static_assert(tv_a.size() == 1, "");
    static_assert(tv_a.begin() != nullptr, "");

    static_assert(!tv_ab.empty(), "");
    static_assert(tv_ab.size() == 2, "");
    static_assert(tv_ab.begin() != nullptr, "");

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

    static_assert(!(tv_a == "ab"), "");
    static_assert(tv_a != "ab", "");
    static_assert(tv_a < "ab", "");
    static_assert(tv_a <= "ab", "");
    static_assert(!(tv_a > "ab"), "");
    static_assert(!(tv_a >= "ab"), "");

    static_assert(!("a" == tv_ab), "");
    static_assert("a" != tv_ab, "");
    static_assert("a" < tv_ab, "");
    static_assert("a" <= tv_ab, "");
    static_assert(!("a" > tv_ab), "");
    static_assert(!("a" >= tv_ab), "");

    static_assert(tv_a.compare("ab") == -1, "");
    static_assert(tv_a == "a", "");

    static_assert(swapped(tv_a, tv_ab).left == tv_ab, "");
    static_assert(swapped(tv_a, tv_ab).right == tv_a, "");

    static_assert(tv_a.begin() == begin(tv_a), "");
    static_assert(tv_a.end() == end(tv_a), "");

    static_assert(tv_a.rbegin() == rbegin(tv_a), "");
    static_assert(tv_a.rend() == rend(tv_a), "");

    {
        using namespace text::literals;
        static_assert(tv_a == u8"a"_tv, "");
        static_assert(tv_ab == "ab"_tv, "");
    }
}

#endif

TEST(text_view, test_substr)
{
    text::text_view tv_empty;
    text::text_view tv_a("a");
    text::text_view tv_abc("abcdefg");

    EXPECT_EQ(tv_empty(0, 0), tv_empty);

    EXPECT_EQ(tv_a(0, 1), tv_a);
    EXPECT_EQ(tv_a(0), "");
    EXPECT_EQ(tv_a(1), tv_a);
    EXPECT_EQ(tv_a(-1), tv_a);
    EXPECT_EQ(tv_a(0, -1), tv_empty);

    EXPECT_EQ(tv_abc(0, 7), tv_abc);
    EXPECT_EQ(tv_abc(0), "");
    EXPECT_EQ(tv_abc(1), "a");
    EXPECT_EQ(tv_abc(2), "ab");
    EXPECT_EQ(tv_abc(3), "abc");
    EXPECT_EQ(tv_abc(-1), "g");
    EXPECT_EQ(tv_abc(-2), "fg");
    EXPECT_EQ(tv_abc(-3), "efg");
    EXPECT_EQ(tv_abc(0, -1), text::text_view("abcdef"));

    EXPECT_EQ(tv_a(0, 1), text::text_view("a"));

    EXPECT_EQ(tv_abc(0, 7), text::text_view("abcdefg"));
    EXPECT_EQ(tv_abc(2, 5), text::text_view("cde"));
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

TEST(text_view, test_substr_constexpr)
{
    constexpr text::text_view tv_empty;
    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_abc("abcdefg");

    static_assert(tv_empty(0, 0) == tv_empty, "");

    static_assert(tv_a(0, 1) == tv_a, "");
    static_assert(tv_a(0) == "", "");
    static_assert(tv_a(1) == tv_a, "");
    static_assert(tv_a(-1) == tv_a, "");
    static_assert(tv_a(0, -1) == tv_empty, "");

    static_assert(tv_abc(0, 7) == tv_abc, "");
    static_assert(tv_abc(0) == "", "");
    static_assert(tv_abc(1) == "a", "");
    static_assert(tv_abc(2) == "ab", "");
    static_assert(tv_abc(3) == "abc", "");
    static_assert(tv_abc(-1) == "g", "");
    static_assert(tv_abc(-2) == "fg", "");
    static_assert(tv_abc(-3) == "efg", "");
    static_assert(tv_abc(0, -1) == text::text_view("abcdef"), "");

    static_assert(tv_a(0, 1) == text::text_view("a"), "");

    static_assert(tv_abc(0, 7) == text::text_view("abcdefg"), "");
    static_assert(tv_abc(2, 5) == text::text_view("cde"), "");
}

#endif

TEST(text_view, test_formatted_output)
{
    {
        std::ostringstream oss;
        oss << std::setw(10) << text::text_view("abc");
        EXPECT_EQ(oss.str(), "       abc");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*') << text::text_view("abc");
        EXPECT_EQ(oss.str(), "abc*******");
    }
}


#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

struct repeated_text_views
{
    text::repeated_text_view left;
    text::repeated_text_view right;
};

constexpr repeated_text_views swapped (text::repeated_text_view lhs, text::repeated_text_view rhs)
{
    lhs.swap(rhs);
    return repeated_text_views{lhs, rhs};
}

TEST(repeated_text_view, test_swap_and_comparisons_constexpr)
{
    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_abab("abab");

    constexpr text::repeated_text_view tv_a_3(tv_a, 3);
    constexpr text::repeated_text_view tv_ab_1(tv_ab, 1);
    constexpr text::repeated_text_view tv_ab_2(tv_ab, 2);
    constexpr text::repeated_text_view tv_ab_3(tv_ab, 3);
    constexpr text::repeated_text_view tv_abab_1(tv_abab, 1);

    {
        static_assert(swapped(tv_a_3, tv_ab_2).left.view() == tv_ab_2.view(), "");
        static_assert(swapped(tv_a_3, tv_ab_2).left.count() == tv_ab_2.count(), "");
        static_assert(swapped(tv_a_3, tv_ab_2).right.view() == tv_a_3.view(), "");
        static_assert(swapped(tv_a_3, tv_ab_2).right.count() == tv_a_3.count(), "");
    }

    static_assert(tv_ab_2 == tv_ab_2, "");

    static_assert(tv_a_3 < tv_ab_2, "");
    static_assert(tv_a_3 < tv_ab_3, "");

    static_assert(tv_ab_2 < tv_ab_3, "");
    static_assert(tv_ab_3 > tv_ab_2, "");

    static_assert(tv_ab_1 < tv_abab_1, "");
    static_assert(tv_abab_1 > tv_ab_1, "");

    static_assert(tv_ab_2 == tv_abab_1, "");
    static_assert(tv_abab_1 == tv_ab_2, "");

    static_assert(tv_ab_3 > tv_abab_1, "");
    static_assert(tv_abab_1 < tv_ab_3, "");
}

#endif
