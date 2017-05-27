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
    EXPECT_EQ(find_index, -1);
    text::text_view find_view_view = find_view(tv, tv);
    EXPECT_EQ(find_view_view, tv);

    int rfind_index = rfind(tv, tv);
    EXPECT_EQ(rfind_index, -1);
    text::text_view rfind_view_view = rfind_view(tv, tv);
    EXPECT_EQ(rfind_view_view, tv);

    int find_first_of_index = find_first_of(tv, tv);
    EXPECT_EQ(find_first_of_index, -1);
    int find_first_not_of_index = find_first_not_of(tv, tv);
    EXPECT_EQ(find_first_not_of_index, -1);

    int find_last_of_index = find_last_of(tv, tv);
    EXPECT_EQ(find_last_of_index, -1);
    int find_last_not_of_index = find_last_not_of(tv, tv);
    EXPECT_EQ(find_last_not_of_index, -1);

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

    constexpr int find_index = find(tv, tv);
    static_assert(find_index == -1, "");
    constexpr text::text_view find_view_view = find_view(tv, tv);
    static_assert(find_view_view == tv, "");

    constexpr int rfind_index = rfind(tv, tv);
    static_assert(rfind_index == -1, "");
    constexpr text::text_view rfind_view_view = rfind_view(tv, tv);
    static_assert(rfind_view_view == tv, "");

    constexpr int find_first_of_index = find_first_of(tv, tv);
    static_assert(find_first_of_index == -1, "");
    constexpr int find_first_not_of_index = find_first_not_of(tv, tv);
    static_assert(find_first_not_of_index == -1, "");

    constexpr int find_last_of_index = find_last_of(tv, tv);
    static_assert(find_last_of_index == -1, "");
    constexpr int find_last_not_of_index = find_last_not_of(tv, tv);
    static_assert(find_last_not_of_index == -1, "");

    constexpr text::text_view substr_view = tv(0, 0);
    static_assert(substr_view == tv, "");

    constexpr int compare_result = compare(tv, tv);
    static_assert(compare_result == 0, "");
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
}

// TODO: find()
// TODO: find_view()
// TODO: find_first_of()
// TODO: find_last_of()
// TODO: find_first_not_of()
// TODO: find_last_not_of()
// TODO: rfind()
// TODO: rfind_view()
