#include <boost/text/unencoded_rope_view.hpp>

#include <gtest/gtest.h>

#include <iomanip>


using namespace boost;

TEST(unencoded_rope_view, test_empty)
{
    text::unencoded_rope_view rtv;

    EXPECT_EQ(rtv.begin(), rtv.end());
    EXPECT_EQ(rtv.rbegin(), rtv.rend());

    EXPECT_TRUE(rtv.empty());
    EXPECT_EQ(rtv.size(), 0);

    EXPECT_EQ(rtv.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(rtv.compare(rtv), 0);
    EXPECT_TRUE(rtv == rtv);
    EXPECT_FALSE(rtv != rtv);
    EXPECT_FALSE(rtv < rtv);
    EXPECT_TRUE(rtv <= rtv);
    EXPECT_FALSE(rtv > rtv);
    EXPECT_TRUE(rtv >= rtv);

    rtv.swap(rtv);
    EXPECT_TRUE(rtv == rtv);

    EXPECT_EQ(rtv.begin(), begin(rtv));
    EXPECT_EQ(rtv.end(), end(rtv));

    EXPECT_EQ(rtv.rbegin(), rbegin(rtv));
    EXPECT_EQ(rtv.rend(), rend(rtv));

    std::cout << "rtv=\"" << rtv << "\"\n";

    text::unencoded_rope_view::iterator it;

    {
        text::unencoded_rope_view rv_from_rtv(
            text::repeated_text_view("txt", 2), 1, 5);
        std::ostringstream oss;
        oss << rv_from_rtv;
        EXPECT_EQ(oss.str(), "xttx");
    }

    {
        text::unencoded_rope_view rv_from_rtv("fools gold");
        std::ostringstream oss;
        oss << rv_from_rtv;
        EXPECT_EQ(oss.str(), "fools gold");
    }
}

struct rope_views
{
    text::unencoded_rope_view left;
    text::unencoded_rope_view right;
};

rope_views swapped(text::unencoded_rope_view lhs, text::unencoded_rope_view rhs)
{
    lhs.swap(rhs);
    return rope_views{lhs, rhs};
}

TEST(unencoded_rope_view, test_non_empty)
{
    text::unencoded_rope r_a("a");
    text::unencoded_rope r_ab("ab");

    text::unencoded_rope_view rtv_a = r_a;
    text::unencoded_rope_view rtv_ab = r_ab;

    EXPECT_EQ(rtv_a.begin() + rtv_a.size(), rtv_a.end());
    EXPECT_EQ(rtv_a.rbegin() + rtv_a.size(), rtv_a.rend());

    EXPECT_FALSE(rtv_a.empty());
    EXPECT_EQ(rtv_a.size(), 1);

    EXPECT_FALSE(rtv_ab.empty());
    EXPECT_EQ(rtv_ab.size(), 2);

    EXPECT_EQ(rtv_ab[1], 'b');

    EXPECT_EQ(rtv_a.max_size(), PTRDIFF_MAX);
    EXPECT_EQ(rtv_ab.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(rtv_a.compare(rtv_ab), -1);
    EXPECT_FALSE(rtv_a == rtv_ab);
    EXPECT_TRUE(rtv_a != rtv_ab);
    EXPECT_TRUE(rtv_a < rtv_ab);
    EXPECT_TRUE(rtv_a <= rtv_ab);
    EXPECT_FALSE(rtv_a > rtv_ab);
    EXPECT_FALSE(rtv_a >= rtv_ab);

    EXPECT_EQ(rtv_a.compare(rtv_ab), -1);
    EXPECT_EQ(rtv_a, rtv_a);
    EXPECT_EQ(rtv_a, "a");

    EXPECT_EQ(swapped(rtv_a, rtv_ab).left, rtv_ab);
    EXPECT_EQ(swapped(rtv_a, rtv_ab).right, rtv_a);

    EXPECT_EQ(rtv_a.begin(), begin(rtv_a));
    EXPECT_EQ(rtv_a.end(), end(rtv_a));

    EXPECT_EQ(rtv_a.rbegin(), rbegin(rtv_a));
    EXPECT_EQ(rtv_a.rend(), rend(rtv_a));
}

TEST(unencoded_rope_view, test_substr)
{
    text::unencoded_rope r_a("a");
    text::unencoded_rope r_abc("abcdefg");

    text::unencoded_rope_view rtv_empty;
    text::unencoded_rope_view rtv_a = r_a;
    text::unencoded_rope_view rtv_abc = r_abc;

    EXPECT_EQ(rtv_empty(0, 0), rtv_empty);

    EXPECT_EQ(rtv_a(0, 1), rtv_a);
    EXPECT_EQ(rtv_a(0), "");
    EXPECT_EQ(rtv_a(1), rtv_a);
    EXPECT_EQ(rtv_a(-1), rtv_a);
    EXPECT_EQ(rtv_a(0, -1), rtv_empty);
    EXPECT_EQ(rtv_a(-1, -1), rtv_empty);

    EXPECT_EQ(rtv_abc(0, 7), rtv_abc);
    EXPECT_EQ(rtv_abc(0), "");
    EXPECT_EQ(rtv_abc(1), "a");
    EXPECT_EQ(rtv_abc(2), "ab");
    EXPECT_EQ(rtv_abc(3), "abc");
    EXPECT_EQ(rtv_abc(-1), "g");
    EXPECT_EQ(rtv_abc(-2), "fg");
    EXPECT_EQ(rtv_abc(-3), "efg");
    EXPECT_EQ(rtv_abc(0, -1), "abcdef");

    EXPECT_EQ(rtv_a(0, 1), "a");

    EXPECT_EQ(rtv_abc(0, 7), "abcdefg");
    EXPECT_EQ(rtv_abc(2, 5), "cde");
}

TEST(unencoded_rope_view, test_foreach_segment)
{
    {
        text::unencoded_rope r("some");
        r += text::repeat(" ", 3);
        r += text::text("text");

        for (int i = 0; i < r.size(); ++i) {
            for (int j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::text_view(oss.str().c_str()), rv)
                    << "i=" << i << " j=" << j;
            }
        }
    }

    {
        text::unencoded_rope r("some");

        for (int i = 0; i < r.size(); ++i) {
            for (int j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::text_view(oss.str().c_str()), rv)
                    << "i=" << i << " j=" << j;
            }
        }
    }

    {
        text::unencoded_rope r(text::repeated_text_view("txt", 2));

        for (int i = 0; i < r.size(); ++i) {
            for (int j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::text_view(oss.str().c_str()), rv)
                    << "i=" << i << " j=" << j;
            }
        }
    }
}

TEST(unencoded_rope_view, test_unformatted_output)
{
    {
        text::unencoded_rope r("abc");
        text::unencoded_rope_view rv = r;

        std::ostringstream oss;
        oss << std::setw(10) << rv;
        EXPECT_EQ(oss.str(), "abc");
    }

    {
        text::unencoded_rope r("abc");
        text::unencoded_rope_view rv = r;

        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*') << rv;
        EXPECT_EQ(oss.str(), "abc");
    }
}
