#include <boost/text/text_view.hpp>
#include <boost/text/algorithm.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(algorithm, test_empty_view)
{
    text::text_view const tv;
    EXPECT_EQ(tv.begin(), nullptr);
    EXPECT_EQ(tv.size(), 0);

    int find_index = find(tv, tv);
    EXPECT_EQ(find_index, 0);
    text::text_view find_view_view = find_view(tv, tv);
    EXPECT_EQ(find_view_view, tv);

    int rfind_index = rfind(tv, tv);
    EXPECT_EQ(rfind_index, 0);
    text::text_view rfind_view_view = rfind_view(tv, tv);
    EXPECT_EQ(rfind_view_view, tv);

    int find_first_of_index = find_first_of(tv, tv);
    EXPECT_EQ(find_first_of_index, 0);
    int find_first_not_of_index = find_first_not_of(tv, tv);
    EXPECT_EQ(find_first_not_of_index, 0);

    int find_last_of_index = find_last_of(tv, tv);
    EXPECT_EQ(find_last_of_index, 0);
    int find_last_not_of_index = find_last_not_of(tv, tv);
    EXPECT_EQ(find_last_not_of_index, 0);

    text::text_view substr_view = tv(0, 0);
    EXPECT_EQ(substr_view, tv);

    bool compare_result = compare(tv, tv);
    EXPECT_EQ(compare_result, 0);
}

TEST(algorithm, test_empty_view_constexpr)
{
    constexpr text::text_view tv;
    static_assert(tv.begin() == nullptr, "");
    static_assert(tv.size() == 0, "");

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr int find_index = find(tv, tv);
    static_assert(find_index == 0, "");
    constexpr text::text_view find_view_view = find_view(tv, tv);
    static_assert(find_view_view == tv, "");

    constexpr int rfind_index = rfind(tv, tv);
    static_assert(rfind_index == 0, "");
    constexpr text::text_view rfind_view_view = rfind_view(tv, tv);
    static_assert(rfind_view_view == tv, "");

    constexpr int find_first_of_index = find_first_of(tv, tv);
    static_assert(find_first_of_index == 0, "");
    constexpr int find_first_not_of_index = find_first_not_of(tv, tv);
    static_assert(find_first_not_of_index ==0, "");

    constexpr int find_last_of_index = find_last_of(tv, tv);
    static_assert(find_last_of_index == 0, "");
    constexpr int find_last_not_of_index = find_last_not_of(tv, tv);
    static_assert(find_last_not_of_index == 0, "");

    constexpr text::text_view substr_view = tv(0, 0);
    static_assert(substr_view == tv, "");

    constexpr int compare_result = compare(tv, tv);
    static_assert(compare_result == 0, "");

#endif
}

TEST(algorithm, test_view_view_compare)
{
    text::text_view tv_empty;
    text::text_view tv_a("a");
    text::text_view tv_b("b");
    text::text_view tv_ab("ab");

    int compare_empty_a = compare(tv_empty, tv_a);
    EXPECT_EQ(compare_empty_a, -1);
    int compare_a_empty = compare(tv_a, tv_empty);
    EXPECT_EQ(compare_a_empty, 1);

    int compare_a_ab = compare(tv_a, tv_ab);
    EXPECT_EQ(compare_a_ab, -1);
    int compare_ab_a = compare(tv_ab, tv_a);
    EXPECT_EQ(compare_ab_a, 1);
    int compare_a_a = compare(tv_a, tv_a);
    EXPECT_EQ(compare_a_a, 0);

    int compare_a_b = compare(tv_a, tv_b);
    EXPECT_EQ(compare_a_b, -1);
    int compare_b_a = compare(tv_b, tv_a);
    EXPECT_EQ(compare_b_a, 1);
}

TEST(algorithm, test_view_view_compare_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");

    constexpr int compare_empty_a = compare(tv_empty, tv_a);
    static_assert(compare_empty_a == -1, "");
    constexpr int compare_a_empty = compare(tv_a, tv_empty);
    static_assert(compare_a_empty == 1, "");

    constexpr int compare_a_ab = compare(tv_a, tv_ab);
    static_assert(compare_a_ab == -1, "");
    constexpr int compare_ab_a = compare(tv_ab, tv_a);
    static_assert(compare_ab_a == 1, "");
    constexpr int compare_a_a = compare(tv_a, tv_a);
    static_assert(compare_a_a == 0, "");

    constexpr int compare_a_b = compare(tv_a, tv_b);
    static_assert(compare_a_b == -1, "");
    constexpr int compare_b_a = compare(tv_b, tv_a);
    static_assert(compare_b_a == 1, "");

#endif
}

TEST(algorithm, test_view_view_find_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");

    constexpr int find_a_empty = find(tv_a, tv_empty);
    static_assert(find_a_empty == 0, "");
    constexpr int find_empty_a = find(tv_empty, tv_a);
    static_assert(find_empty_a == -1, "");

    constexpr int find_a_ab = find(tv_a, tv_ab);
    static_assert(find_a_ab == -1, "");
    constexpr int find_ab_a = find(tv_ab, tv_a);
    static_assert(find_ab_a == 0, "");
    constexpr int find_a_a = find(tv_a, tv_a);
    static_assert(find_a_a == 0, "");

    constexpr int find_b_ab = find(tv_b, tv_ab);
    static_assert(find_b_ab == -1, "");
    constexpr int find_ab_b = find(tv_ab, tv_b);
    static_assert(find_ab_b == 1, "");
    constexpr int find_b_b = find(tv_b, tv_b);
    static_assert(find_b_b == 0, "");

    constexpr int find_a_b = find(tv_a, tv_b);
    static_assert(find_a_b == -1, "");
    constexpr int find_b_a = find(tv_b, tv_a);
    static_assert(find_b_a == -1, "");

    constexpr int find_ab_aab = find(tv_ab, tv_aab);
    static_assert(find_ab_aab == -1, "");
    constexpr int find_aab_ab = find(tv_aab, tv_ab);
    static_assert(find_aab_ab == 1, "");

#endif
}

TEST(algorithm, test_view_view_find_view_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");

    constexpr text::text_view find_a_empty = find_view(tv_a, tv_empty);
    static_assert(find_a_empty.begin() == tv_a.begin(), "");
    static_assert(find_a_empty.size() == 0, "");
    constexpr text::text_view find_empty_a = find_view(tv_empty, tv_a);
    static_assert(find_empty_a.begin() == nullptr, "");
    static_assert(find_empty_a.size() == 0, "");

    constexpr text::text_view find_a_ab = find_view(tv_a, tv_ab);
    static_assert(find_a_ab.begin() == nullptr, "");
    static_assert(find_a_ab.size() == 0, "");
    constexpr text::text_view find_ab_a = find_view(tv_ab, tv_a);
    static_assert(find_ab_a.begin() == tv_ab.begin(), "");
    static_assert(find_ab_a.size() == 1, "");
    constexpr text::text_view find_a_a = find_view(tv_a, tv_a);
    static_assert(find_a_a.begin() == tv_a.begin(), "");
    static_assert(find_a_a.size() == 1, "");

    constexpr text::text_view find_b_ab = find_view(tv_b, tv_ab);
    static_assert(find_b_ab.begin() == nullptr, "");
    static_assert(find_b_ab.size() == 0, "");
    constexpr text::text_view find_ab_b = find_view(tv_ab, tv_b);
    static_assert(find_ab_b.begin() == tv_ab.begin() + 1, "");
    static_assert(find_ab_b.size() == 1, "");
    constexpr text::text_view find_b_b = find_view(tv_b, tv_b);
    static_assert(find_b_b.begin() == tv_b.begin(), "");
    static_assert(find_b_b.size() == 1, "");

    constexpr text::text_view find_a_b = find_view(tv_a, tv_b);
    static_assert(find_a_b.begin() == nullptr, "");
    static_assert(find_a_b.size() == 0, "");
    constexpr text::text_view find_b_a = find_view(tv_b, tv_a);
    static_assert(find_b_a.begin() == nullptr, "");
    static_assert(find_b_a.size() == 0, "");

    constexpr text::text_view find_ab_aab = find_view(tv_ab, tv_aab);
    static_assert(find_ab_aab.begin() == nullptr, "");
    static_assert(find_ab_aab.size() == 0, "");
    constexpr text::text_view find_aab_ab = find_view(tv_aab, tv_ab);
    static_assert(find_aab_ab.begin() == tv_aab.begin() + 1, "");
    static_assert(find_aab_ab.size() == 2, "");

#endif
}

TEST(algorithm, test_view_view_find_first_of_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");
    constexpr text::text_view tv_ca("ca");

    constexpr int find_first_of_a_empty = find_first_of(tv_a, tv_empty);
    static_assert(find_first_of_a_empty == 0, "");
    constexpr int find_first_of_empty_a = find_first_of(tv_empty, tv_a);
    static_assert(find_first_of_empty_a == -1, "");

    constexpr int find_first_of_a_ab = find_first_of(tv_a, tv_ab);
    static_assert(find_first_of_a_ab == 0, "");
    constexpr int find_first_of_ab_a = find_first_of(tv_ab, tv_a);
    static_assert(find_first_of_ab_a == 0, "");
    constexpr int find_first_of_a_a = find_first_of(tv_a, tv_a);
    static_assert(find_first_of_a_a == 0, "");

    constexpr int find_first_of_b_ab = find_first_of(tv_b, tv_ab);
    static_assert(find_first_of_b_ab == 0, "");
    constexpr int find_first_of_ab_b = find_first_of(tv_ab, tv_b);
    static_assert(find_first_of_ab_b == 1, "");
    constexpr int find_first_of_b_b = find_first_of(tv_b, tv_b);
    static_assert(find_first_of_b_b == 0, "");

    constexpr int find_first_of_a_b = find_first_of(tv_a, tv_b);
    static_assert(find_first_of_a_b == -1, "");
    constexpr int find_first_of_b_a = find_first_of(tv_b, tv_a);
    static_assert(find_first_of_b_a == -1, "");

    constexpr int find_first_of_ab_aab = find_first_of(tv_ab, tv_aab);
    static_assert(find_first_of_ab_aab == 0, "");
    constexpr int find_first_of_aab_ab = find_first_of(tv_aab, tv_ab);
    static_assert(find_first_of_aab_ab == 0, "");

    constexpr int find_first_of_ca_aab = find_first_of(tv_ca, tv_aab);
    static_assert(find_first_of_ca_aab == 1, "");
    constexpr int find_first_of_aab_ca = find_first_of(tv_aab, tv_ca);
    static_assert(find_first_of_aab_ca == 0, "");

#endif
}

TEST(algorithm, test_view_view_find_first_not_of_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");
    constexpr text::text_view tv_ca("ca");

    constexpr int find_first_not_of_a_empty = find_first_not_of(tv_a, tv_empty);
    static_assert(find_first_not_of_a_empty == 0, "");
    constexpr int find_first_not_of_empty_a = find_first_not_of(tv_empty, tv_a);
    static_assert(find_first_not_of_empty_a == -1, "");

    constexpr int find_first_not_of_a_ab = find_first_not_of(tv_a, tv_ab);
    static_assert(find_first_not_of_a_ab == -1, "");
    constexpr int find_first_not_of_ab_a = find_first_not_of(tv_ab, tv_a);
    static_assert(find_first_not_of_ab_a == 1, "");
    constexpr int find_first_not_of_a_a = find_first_not_of(tv_a, tv_a);
    static_assert(find_first_not_of_a_a == -1, "");

    constexpr int find_first_not_of_b_ab = find_first_not_of(tv_b, tv_ab);
    static_assert(find_first_not_of_b_ab == -1, "");
    constexpr int find_first_not_of_ab_b = find_first_not_of(tv_ab, tv_b);
    static_assert(find_first_not_of_ab_b == 0, "");
    constexpr int find_first_not_of_b_b = find_first_not_of(tv_b, tv_b);
    static_assert(find_first_not_of_b_b == -1, "");

    constexpr int find_first_not_of_a_b = find_first_not_of(tv_a, tv_b);
    static_assert(find_first_not_of_a_b == 0, "");
    constexpr int find_first_not_of_b_a = find_first_not_of(tv_b, tv_a);
    static_assert(find_first_not_of_b_a == 0, "");

    constexpr int find_first_not_of_ab_aab = find_first_not_of(tv_ab, tv_aab);
    static_assert(find_first_not_of_ab_aab == -1, "");
    constexpr int find_first_not_of_aab_ab = find_first_not_of(tv_aab, tv_ab);
    static_assert(find_first_not_of_aab_ab == -1, "");

    constexpr int find_first_not_of_ca_aab = find_first_not_of(tv_ca, tv_aab);
    static_assert(find_first_not_of_ca_aab == 0, "");
    constexpr int find_first_not_of_aab_ca = find_first_not_of(tv_aab, tv_ca);
    static_assert(find_first_not_of_aab_ca == 2, "");

#endif
}

TEST(algorithm, test_view_view_rfind_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");

    constexpr int rfind_a_empty = rfind(tv_a, tv_empty);
    static_assert(rfind_a_empty == 1, "");
    constexpr int rfind_empty_a = rfind(tv_empty, tv_a);
    static_assert(rfind_empty_a == -1, "");

    constexpr int rfind_a_ab = rfind(tv_a, tv_ab);
    static_assert(rfind_a_ab == -1, "");
    constexpr int rfind_ab_a = rfind(tv_ab, tv_a);
    static_assert(rfind_ab_a == 0, "");
    constexpr int rfind_a_a = rfind(tv_a, tv_a);
    static_assert(rfind_a_a == 0, "");

    constexpr int rfind_b_ab = rfind(tv_b, tv_ab);
    static_assert(rfind_b_ab == -1, "");
    constexpr int rfind_ab_b = rfind(tv_ab, tv_b);
    static_assert(rfind_ab_b == 1, "");
    constexpr int rfind_b_b = rfind(tv_b, tv_b);
    static_assert(rfind_b_b == 0, "");

    constexpr int rfind_a_b = rfind(tv_a, tv_b);
    static_assert(rfind_a_b == -1, "");
    constexpr int rfind_b_a = rfind(tv_b, tv_a);
    static_assert(rfind_b_a == -1, "");

    constexpr int rfind_ab_aab = rfind(tv_ab, tv_aab);
    static_assert(rfind_ab_aab == -1, "");
    constexpr int rfind_aab_ab = rfind(tv_aab, tv_ab);
    static_assert(rfind_aab_ab == 4, "");

#endif
}

TEST(algorithm, test_view_view_rfind_view_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");

    constexpr text::text_view find_a_empty = rfind_view(tv_a, tv_empty);
    static_assert(find_a_empty.begin() == tv_a.end(), "");
    static_assert(find_a_empty.size() == 0, "");
    constexpr text::text_view find_empty_a = rfind_view(tv_empty, tv_a);
    static_assert(find_empty_a.begin() == nullptr, "");
    static_assert(find_empty_a.size() == 0, "");

    constexpr text::text_view find_a_ab = rfind_view(tv_a, tv_ab);
    static_assert(find_a_ab.begin() == nullptr, "");
    static_assert(find_a_ab.size() == 0, "");
    constexpr text::text_view find_ab_a = rfind_view(tv_ab, tv_a);
    static_assert(find_ab_a.begin() == tv_ab.begin(), "");
    static_assert(find_ab_a.size() == 1, "");
    constexpr text::text_view find_a_a = rfind_view(tv_a, tv_a);
    static_assert(find_a_a.begin() == tv_a.begin(), "");
    static_assert(find_a_a.size() == 1, "");

    constexpr text::text_view find_b_ab = rfind_view(tv_b, tv_ab);
    static_assert(find_b_ab.begin() == nullptr, "");
    static_assert(find_b_ab.size() == 0, "");
    constexpr text::text_view find_ab_b = rfind_view(tv_ab, tv_b);
    static_assert(find_ab_b.begin() == tv_ab.begin() + 1, "");
    static_assert(find_ab_b.size() == 1, "");
    constexpr text::text_view find_b_b = rfind_view(tv_b, tv_b);
    static_assert(find_b_b.begin() == tv_b.begin(), "");
    static_assert(find_b_b.size() == 1, "");

    constexpr text::text_view find_a_b = rfind_view(tv_a, tv_b);
    static_assert(find_a_b.begin() == nullptr, "");
    static_assert(find_a_b.size() == 0, "");
    constexpr text::text_view find_b_a = rfind_view(tv_b, tv_a);
    static_assert(find_b_a.begin() == nullptr, "");
    static_assert(find_b_a.size() == 0, "");

    constexpr text::text_view find_ab_aab = rfind_view(tv_ab, tv_aab);
    static_assert(find_ab_aab.begin() == nullptr, "");
    static_assert(find_ab_aab.size() == 0, "");
    constexpr text::text_view find_aab_ab = rfind_view(tv_aab, tv_ab);
    static_assert(find_aab_ab.begin() == tv_aab.begin() + 4, "");
    static_assert(find_aab_ab.size() == 2, "");

#endif
}

TEST(algorithm, test_view_view_find_last_of_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");
    constexpr text::text_view tv_ca("ca");

    constexpr int find_last_of_a_empty = find_last_of(tv_a, tv_empty);
    static_assert(find_last_of_a_empty == 1, "");
    constexpr int find_last_of_empty_a = find_last_of(tv_empty, tv_a);
    static_assert(find_last_of_empty_a == -1, "");

    constexpr int find_last_of_a_ab = find_last_of(tv_a, tv_ab);
    static_assert(find_last_of_a_ab == 0, "");
    constexpr int find_last_of_ab_a = find_last_of(tv_ab, tv_a);
    static_assert(find_last_of_ab_a == 0, "");
    constexpr int find_last_of_a_a = find_last_of(tv_a, tv_a);
    static_assert(find_last_of_a_a == 0, "");

    constexpr int find_last_of_b_ab = find_last_of(tv_b, tv_ab);
    static_assert(find_last_of_b_ab == 0, "");
    constexpr int find_last_of_ab_b = find_last_of(tv_ab, tv_b);
    static_assert(find_last_of_ab_b == 1, "");
    constexpr int find_last_of_b_b = find_last_of(tv_b, tv_b);
    static_assert(find_last_of_b_b == 0, "");

    constexpr int find_last_of_a_b = find_last_of(tv_a, tv_b);
    static_assert(find_last_of_a_b == -1, "");
    constexpr int find_last_of_b_a = find_last_of(tv_b, tv_a);
    static_assert(find_last_of_b_a == -1, "");

    constexpr int find_last_of_ab_aab = find_last_of(tv_ab, tv_aab);
    static_assert(find_last_of_ab_aab == 1, "");
    constexpr int find_last_of_aab_ab = find_last_of(tv_aab, tv_ab);
    static_assert(find_last_of_aab_ab == 5, "");

    constexpr int find_last_of_ca_aab = find_last_of(tv_ca, tv_aab);
    static_assert(find_last_of_ca_aab == 1, "");
    constexpr int find_last_of_aab_ca = find_last_of(tv_aab, tv_ca);
    static_assert(find_last_of_aab_ca == 4, "");

#endif
}

TEST(algorithm, test_view_view_find_last_not_of_constexpr)
{
    constexpr text::text_view tv_empty;

#ifndef BOOST_NO_CXX14_CONSTEXPR

    constexpr text::text_view tv_a("a");
    constexpr text::text_view tv_b("b");
    constexpr text::text_view tv_ab("ab");
    constexpr text::text_view tv_aab("aabaab");
    constexpr text::text_view tv_ca("ca");

    constexpr int find_last_not_of_a_empty = find_last_not_of(tv_a, tv_empty);
    static_assert(find_last_not_of_a_empty == 1, "");
    constexpr int find_last_not_of_empty_a = find_last_not_of(tv_empty, tv_a);
    static_assert(find_last_not_of_empty_a == -1, "");

    constexpr int find_last_not_of_a_ab = find_last_not_of(tv_a, tv_ab);
    static_assert(find_last_not_of_a_ab == -1, "");
    constexpr int find_last_not_of_ab_a = find_last_not_of(tv_ab, tv_a);
    static_assert(find_last_not_of_ab_a == 1, "");
    constexpr int find_last_not_of_a_a = find_last_not_of(tv_a, tv_a);
    static_assert(find_last_not_of_a_a == -1, "");

    constexpr int find_last_not_of_b_ab = find_last_not_of(tv_b, tv_ab);
    static_assert(find_last_not_of_b_ab == -1, "");
    constexpr int find_last_not_of_ab_b = find_last_not_of(tv_ab, tv_b);
    static_assert(find_last_not_of_ab_b == 0, "");
    constexpr int find_last_not_of_b_b = find_last_not_of(tv_b, tv_b);
    static_assert(find_last_not_of_b_b == -1, "");

    constexpr int find_last_not_of_a_b = find_last_not_of(tv_a, tv_b);
    static_assert(find_last_not_of_a_b == 0, "");
    constexpr int find_last_not_of_b_a = find_last_not_of(tv_b, tv_a);
    static_assert(find_last_not_of_b_a == 0, "");

    constexpr int find_last_not_of_ab_aab = find_last_not_of(tv_ab, tv_aab);
    static_assert(find_last_not_of_ab_aab == -1, "");
    constexpr int find_last_not_of_aab_ab = find_last_not_of(tv_aab, tv_ab);
    static_assert(find_last_not_of_aab_ab == -1, "");

    constexpr int find_last_not_of_ca_aab = find_last_not_of(tv_ca, tv_aab);
    static_assert(find_last_not_of_ca_aab == 0, "");
    constexpr int find_last_not_of_aab_ca = find_last_not_of(tv_aab, tv_ca);
    static_assert(find_last_not_of_aab_ca == 5, "");

#endif
}

// TODO: Coverage for types other than text_view.
