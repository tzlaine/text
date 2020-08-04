// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/segmented_vector.hpp>

#include <gtest/gtest.h>


using namespace boost;


TEST(const_vector_iterator, test_default_ctor)
{
    text::detail::const_vector_iterator<int, std::vector<int>> it1;
    text::detail::const_vector_iterator<int, std::vector<int>> it2;

    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    EXPECT_FALSE(it1 < it2);
    EXPECT_TRUE(it1 <= it2);
    EXPECT_FALSE(it1 > it2);
    EXPECT_TRUE(it1 >= it2);

    EXPECT_EQ(it1 - it2, 0);
    EXPECT_EQ(it2 - it1, 0);
}

TEST(const_vector_iterator, test_short_sequences)
{
    {
        text::segmented_vector<int> r_empty;
        text::detail::const_vector_iterator<int, std::vector<int>> it(r_empty, 0);

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it != it);
        EXPECT_FALSE(it < it);
        EXPECT_TRUE(it <= it);
        EXPECT_FALSE(it > it);
        EXPECT_TRUE(it >= it);

        EXPECT_EQ(it - it, 0);
        EXPECT_EQ(it - it, 0);
    }

    {
        text::segmented_vector<int> r_a({13});
        text::detail::const_vector_iterator<int, std::vector<int>> first(r_a, 0);
        text::detail::const_vector_iterator<int, std::vector<int>> last(r_a, r_a.size());

        EXPECT_EQ(*first, 13);
        EXPECT_EQ(first[0], 13);

        {
            auto it = first;
            EXPECT_EQ(++it, last);

            it = first;
            EXPECT_EQ(it++, first);

            it = first;
            it++;
            EXPECT_EQ(it, last);

            it = first;
            it += 1;
            EXPECT_EQ(it, last);

            it = first;
            it = it + 1;
            EXPECT_EQ(it, last);

            it = first;
            it = 1 + it;
            EXPECT_EQ(it, last);
        }
        {
            auto it = last;
            EXPECT_EQ(--it, first);

            it = last;
            EXPECT_EQ(it--, last);

            it = last;
            it--;
            EXPECT_EQ(it, first);

            it = last;
            it -= 1;
            EXPECT_EQ(it, first);

            it = last;
            it = it - 1;
            EXPECT_EQ(it, first);
        }

        EXPECT_EQ(*(last - 1), 13);
        EXPECT_EQ(last[-1], 13);

        EXPECT_EQ(first + 1, last);
        EXPECT_EQ(first, last - 1);

        EXPECT_FALSE(first == last);
        EXPECT_TRUE(first != last);
        EXPECT_TRUE(first < last);
        EXPECT_TRUE(first <= last);
        EXPECT_FALSE(first > last);
        EXPECT_FALSE(first >= last);

        EXPECT_EQ(first - last, -1);
        EXPECT_EQ(last - first, 1);
    }
}

TEST(const_reverse_vector_iterator, test_default_ctor)
{
    {
        stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> it1;
        stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> it2;

        EXPECT_TRUE(it1 == it2);
        EXPECT_FALSE(it1 != it2);
        EXPECT_FALSE(it1 < it2);
        EXPECT_TRUE(it1 <= it2);
        EXPECT_FALSE(it1 > it2);
        EXPECT_TRUE(it1 >= it2);

        EXPECT_EQ(it1 - it2, 0);
        EXPECT_EQ(it2 - it1, 0);
    }
}

TEST(const_reverse_vector_iterator, test_c_str_ctor)
{
    {
        text::segmented_vector<int> r_empty;
        stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> it = r_empty.rbegin();

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it != it);
        EXPECT_FALSE(it < it);
        EXPECT_TRUE(it <= it);
        EXPECT_FALSE(it > it);
        EXPECT_TRUE(it >= it);

        EXPECT_EQ(it - it, 0);
    }

    {
        text::segmented_vector<int> r_a({13});
        stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> first = r_a.rbegin();
        stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> last = r_a.rend();

        EXPECT_EQ(*last.base(), *first);

        EXPECT_EQ(*first, 13);
        EXPECT_EQ(first[0], 13);

        {
            auto it = first;
            EXPECT_EQ(++it, last);

            it = first;
            EXPECT_EQ(it++, first);

            it = first;
            it++;
            EXPECT_EQ(it, last);

            it = first;
            it += 1;
            EXPECT_EQ(it, last);

            it = first;
            it = it + 1;
            EXPECT_EQ(it, last);

            it = first;
            it = it + 1;
            EXPECT_EQ(it, last);
        }
        {
            auto it = last;
            EXPECT_EQ(--it, first);

            it = last;
            EXPECT_EQ(it--, last);

            it = last;
            it--;
            EXPECT_EQ(it, first);

            it = last;
            it -= 1;
            EXPECT_EQ(it, first);

            it = last;
            it = it - 1;
            EXPECT_EQ(it, first);

            it = last;
            it = it - 1;
            EXPECT_EQ(it, first);
        }

        EXPECT_EQ(*(last - 1), 13);
        EXPECT_EQ(last[-1], 13);

        EXPECT_EQ(first + 1, last);
        EXPECT_EQ(first, last - 1);

        EXPECT_FALSE(first == last);
        EXPECT_TRUE(first != last);
        EXPECT_TRUE(first < last);
        EXPECT_TRUE(first <= last);
        EXPECT_FALSE(first > last);
        EXPECT_FALSE(first >= last);

        EXPECT_EQ(first - last, -1);
        EXPECT_EQ(last - first, 1);
    }
}

TEST(both_vector_iterators, test_larger_sequences)
{
    std::size_t const copies = 40;

    {
        text::segmented_vector<int> r;
        std::vector<int> vec;

        std::size_t i = 0;
        for (; i < text::detail::min_children - 1; ++i) {
            r.insert(r.begin(), 17);
            vec.insert(vec.begin(), 17);
        }

        text::segmented_vector<int> extra_ref = r;
        (void)extra_ref;

        for (; i < copies; ++i) {
            r.insert(r.begin(), 17);
            vec.insert(vec.begin(), 17);
        }

        {
            text::detail::const_vector_iterator<int, std::vector<int>> first = r.begin();
            text::detail::const_vector_iterator<int, std::vector<int>> last = r.end();
            std::vector<int> const from_first_last(first, last);
            EXPECT_EQ(from_first_last, vec);
        }

        {
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> first = r.rbegin();
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> last = r.rend();
            std::vector<int> const from_first_last(first, last);
            EXPECT_EQ(from_first_last, vec);
        }
    }

    {
        text::segmented_vector<int> r;
        std::vector<int> vec;

        for (std::size_t i = 0; i < copies; ++i) {
            r.insert(r.begin(), 17);
            vec.insert(vec.begin(), 17);
        }

        {
            text::detail::const_vector_iterator<int, std::vector<int>> first = r.begin();
            text::detail::const_vector_iterator<int, std::vector<int>> last = r.end();
            std::vector<int> const from_first_last(first, last);
            EXPECT_EQ(from_first_last, vec);
        }

        {
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> first = r.rbegin();
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> last = r.rend();
            std::vector<int> const from_first_last(first, last);
            EXPECT_EQ(from_first_last, vec);
        }
    }

    {
        std::vector<int> repeated(64 * 1024, 11);

        text::segmented_vector<int> r(repeated.begin(), repeated.end());
        r.insert(r.begin() + 5, 12345);

        repeated.insert(repeated.begin() + 5, 12345);

        for (std::size_t i = 0; i < r.size(); ++i) {
            EXPECT_EQ(r[i], repeated[i]);
        }

        for (std::size_t i = 0; i < r.size(); ++i) {
            EXPECT_EQ(*(r.begin() + i), *(repeated.begin() + i));
        }

        {
            text::detail::const_vector_iterator<int, std::vector<int>> first = r.begin();
            text::detail::const_vector_iterator<int, std::vector<int>> last = r.end();
            std::vector<int> const from_first_last(first, last);
            EXPECT_EQ(from_first_last, repeated);
        }

        {
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> first = r.rbegin();
            stl_interfaces::reverse_iterator<text::detail::const_vector_iterator<int, std::vector<int>>> last = r.rend();
            std::vector<int> const from_first_last(first, last);
            std::vector<int> const from_repeated(
                repeated.rbegin(), repeated.rend());
            EXPECT_EQ(from_first_last, from_repeated);
        }
    }
}
