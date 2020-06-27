// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/segmented_vector.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

TEST(segmented_vector, test_empty)
{
    text::segmented_vector<int> t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.begin(), t.end());

    t.swap(t);
    EXPECT_TRUE(t == t);

    t.clear();
}

TEST(segmented_vector, test_non_empty_const_interface)
{
    text::segmented_vector<int> t_a({0});
    text::segmented_vector<int> t_ab({1, 2});

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1);

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);

    EXPECT_EQ(t_ab[1], 2);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);

    text::segmented_vector<int> const old_t_a(t_a);
    text::segmented_vector<int> const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);
}

TEST(segmented_vector, test_ctors)
{
    text::segmented_vector<int> t;
    EXPECT_EQ(t, text::segmented_vector<int>{});
    EXPECT_EQ(text::segmented_vector<int>{}, t);

    text::segmented_vector<int> t2({7, 8, 9});
    EXPECT_EQ(t2[0], 7);
    EXPECT_EQ(t2[1], 8);
    EXPECT_EQ(t2[2], 9);

    text::segmented_vector<int> t3(t2);
    EXPECT_EQ(t3[0], 7);
    EXPECT_EQ(t3[1], 8);
    EXPECT_EQ(t3[2], 9);

    text::segmented_vector<int> t4(std::move(t2));
    EXPECT_EQ(t4[0], 7);
    EXPECT_EQ(t4[1], 8);
    EXPECT_EQ(t4[2], 9);
    EXPECT_EQ(t2, text::segmented_vector<int>{});
    EXPECT_EQ(text::segmented_vector<int>{}, t2);
}

TEST(segmented_vector, test_insert)
{
    std::vector<int> const _789{7, 8, 9};

    {
        text::segmented_vector<int> t({0, 1, 2, 3, 4, 5});
        t.erase(t.begin(), t.end());
        t.insert(t.begin() + 0, _789.begin(), _789.end());
        std::vector<int> const vec{7, 8, 9};
        EXPECT_TRUE(
            algorithm::equal(t.begin(), t.end(), vec.begin(), vec.end()));
    }

    {
        text::segmented_vector<int> const ct({0, 1, 2, 3, 4, 5});

        {
            text::segmented_vector<int> t0 = ct;
            t0.insert(t0.begin(), _789.begin(), _789.end());
            std::vector<int> const vec{7, 8, 9, 0, 1, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t0.begin(), t0.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t1 = ct;
            t1.insert(t1.begin() + 1, _789.begin(), _789.end());
            std::vector<int> const vec{0, 7, 8, 9, 1, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t1.begin(), t1.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t2 = ct;
            t2.insert(t2.begin() + 2, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 7, 8, 9, 2, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t2.begin(), t2.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t3 = ct;
            t3.insert(t3.begin() + 3, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 7, 8, 9, 3, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t3.begin(), t3.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t4 = ct;
            t4.insert(t4.begin() + 4, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 7, 8, 9, 4, 5};
            EXPECT_TRUE(
                algorithm::equal(t4.begin(), t4.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t5 = ct;
            t5.insert(t5.begin() + 5, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 4, 7, 8, 9, 5};
            EXPECT_TRUE(
                algorithm::equal(t5.begin(), t5.end(), vec.begin(), vec.end()));
        }

        {
            text::segmented_vector<int> t6 = ct;
            t6.insert(t6.begin() + 6, _789.begin(), _789.end());
            std::vector<int> const vec{0, 1, 2, 3, 4, 5, 7, 8, 9};
            EXPECT_TRUE(
                algorithm::equal(t6.begin(), t6.end(), vec.begin(), vec.end()));
        }
    }
}

TEST(segmented_vector, test_erase)
{
    {
        text::segmented_vector<int> t(
            {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
             17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
             34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
             51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
             68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
             85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99});
        t.erase(t.begin(), t.end());
        EXPECT_EQ(t, text::segmented_vector<int>{});
    }

    text::segmented_vector<int> const ct(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
         51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
         68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
         85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99});

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;

            text::segmented_vector<int> expected(ct.begin(), ct.begin() + i);
            expected.insert(expected.end(), ct.begin() + j, ct.end());

            t.erase(t.begin() + i, t.begin() + j);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j;
        }
    }
}

TEST(segmented_vector, test_replace)
{
    std::vector<int> const vec({7, 8, 9});

    {
        text::segmented_vector<int> t({0, 1, 2, 3, 4, 5});
        t.replace(t.begin(), t.end(), vec);
        EXPECT_TRUE(
            algorithm::equal(t.begin(), t.end(), vec.begin(), vec.end()));
    }

    text::segmented_vector<int> const ct(
        {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
         17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
         34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
         51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
         68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
         85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99});

    // Small replacement.
    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;

            text::segmented_vector<int> expected(ct.begin(), ct.begin() + i);
            expected.insert(expected.end(), vec);
            expected.insert(expected.end(), ct.begin() + j, ct.end());

            t.replace(t.begin() + i, t.begin() + j, vec.begin(), vec.end());
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j;
        }
    }

    // Large replacement.
    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::segmented_vector<int> t = ct;

            text::segmented_vector<int> expected(ct.begin(), ct.begin() + i);
            expected.insert(expected.end(), ct.begin(), ct.end());
            expected.insert(expected.end(), ct.begin() + j, ct.end());

            t.replace(t.begin() + i, t.begin() + j, ct.begin(), ct.end());
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j;
        }
    }
}

TEST(segmented_vector, test_sentinel_api)
{
    {
        char const * chars = "chars";
        text::segmented_vector<char> v(chars, text::null_sentinel{});
        EXPECT_TRUE(algorithm::equal(
            chars, chars + std::strlen(chars), v.begin(), v.end()));
    }
    {
        char const * chars = "chars";
        text::segmented_vector<char> v;
        v.insert(v.end(), chars, text::null_sentinel{});
        EXPECT_TRUE(algorithm::equal(
            chars, chars + std::strlen(chars), v.begin(), v.end()));
    }
    {
        char const * chars = "chars";
        text::segmented_vector<char> v;
        v.replace(v.begin(), v.end(), chars, text::null_sentinel{});
        EXPECT_TRUE(algorithm::equal(
            chars, chars + std::strlen(chars), v.begin(), v.end()));
    }
}
