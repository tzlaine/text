// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
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
    EXPECT_EQ(rtv.size(), 0u);

    EXPECT_EQ(rtv.max_size(), (std::size_t)PTRDIFF_MAX);

    EXPECT_EQ(rtv.compare(rtv), 0);
    EXPECT_TRUE(rtv == rtv);
    EXPECT_FALSE(rtv != rtv);
    EXPECT_FALSE(rtv < rtv);
    EXPECT_TRUE(rtv <= rtv);
    EXPECT_FALSE(rtv > rtv);
    EXPECT_TRUE(rtv >= rtv);

    std::cout << "rtv=\"" << rtv << "\"\n";

    text::unencoded_rope_view::iterator it;

    {
        text::unencoded_rope r("txttxt");
        text::unencoded_rope_view rv_from_rtv(r, 1, 5);
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

TEST(unencoded_rope_view, test_non_empty)
{
    text::unencoded_rope r_a("a");
    text::unencoded_rope r_ab("ab");

    text::unencoded_rope_view rtv_a = r_a;
    text::unencoded_rope_view rtv_ab = r_ab;

    EXPECT_EQ(rtv_a.begin() + rtv_a.size(), rtv_a.end());
    EXPECT_EQ(rtv_a.rbegin() + rtv_a.size(), rtv_a.rend());

    EXPECT_FALSE(rtv_a.empty());
    EXPECT_EQ(rtv_a.size(), 1u);

    EXPECT_FALSE(rtv_ab.empty());
    EXPECT_EQ(rtv_ab.size(), 2u);

    EXPECT_EQ(rtv_ab[1], 'b');

    EXPECT_EQ(rtv_a.max_size(), (std::size_t)PTRDIFF_MAX);
    EXPECT_EQ(rtv_ab.max_size(), (std::size_t)PTRDIFF_MAX);

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
    EXPECT_EQ(rtv_a(0, 0), "");
    EXPECT_EQ(rtv_a(0, 1), rtv_a);
    EXPECT_EQ(rtv_a(-1, rtv_a.size()), rtv_a);
    EXPECT_EQ(rtv_a(0, -1), rtv_empty);
    EXPECT_EQ(rtv_a(-1, -1), rtv_empty);

    EXPECT_EQ(rtv_abc(0, 7), rtv_abc);
    EXPECT_EQ(rtv_abc(0, 0), "");
    EXPECT_EQ(rtv_abc(0, 1), "a");
    EXPECT_EQ(rtv_abc(0, 2), "ab");
    EXPECT_EQ(rtv_abc(0, 3), "abc");
    EXPECT_EQ(rtv_abc(-1, rtv_abc.size()), "g");
    EXPECT_EQ(rtv_abc(-2, rtv_abc.size()), "fg");
    EXPECT_EQ(rtv_abc(-3, rtv_abc.size()), "efg");
    EXPECT_EQ(rtv_abc(0, -1), "abcdef");

    EXPECT_EQ(rtv_a(0, 1), "a");

    EXPECT_EQ(rtv_abc(0, 7), "abcdefg");
    EXPECT_EQ(rtv_abc(2, 5), "cde");
}

TEST(unencoded_rope_view, test_foreach_segment)
{
    {
        text::unencoded_rope r("some");
        r += "   ";
        r += std::string("text");

        for (std::size_t i = 0; i < r.size(); ++i) {
            for (std::size_t j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::string_view(oss.str().c_str()), rv)
                    << "i=" << i << " j=" << j;
            }
        }
    }

    {
        text::unencoded_rope r("some");

        for (std::size_t i = 0; i < r.size(); ++i) {
            for (std::size_t j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::string_view(oss.str().c_str()), rv)
                    << "i=" << i << " j=" << j;
            }
        }
    }

    {
        text::unencoded_rope r(text::string_view("txttxt"));

        for (std::size_t i = 0; i < r.size(); ++i) {
            for (std::size_t j = i; j < r.size(); ++j) {
                text::unencoded_rope_view rv = r(i, j);
                std::ostringstream oss;
                oss << rv;

                EXPECT_EQ(text::string_view(oss.str().c_str()), rv)
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
        EXPECT_EQ(oss.str(), "       abc");
    }

#if 0 // TODO: Fix!
    {
        text::unencoded_rope r("abc");
        text::unencoded_rope_view rv = r;

        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*') << rv;
        EXPECT_EQ(oss.str(), "abc*******");
    }
#endif
}
