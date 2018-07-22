#include <boost/text/unencoded_rope.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(const_rope_iterator, test_default_ctor)
{
    text::detail::const_rope_iterator it1;
    text::detail::const_rope_iterator it2;

    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    EXPECT_FALSE(it1 < it2);
    EXPECT_TRUE(it1 <= it2);
    EXPECT_FALSE(it1 > it2);
    EXPECT_TRUE(it1 >= it2);

    EXPECT_EQ(it1 - it2, 0);
    EXPECT_EQ(it2 - it1, 0);
}

TEST(const_rope_iterator, test_c_str_ctor)
{
    {
        text::unencoded_rope r_empty("");
        text::detail::const_rope_iterator it(r_empty, 0);

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
        text::unencoded_rope r_a("a");
        text::detail::const_rope_iterator first(r_a, 0);
        text::detail::const_rope_iterator last(r_a, r_a.size());

        EXPECT_EQ(*first, 'a');
        EXPECT_EQ(first[0], 'a');

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

        EXPECT_EQ(*(last - 1), 'a');
        EXPECT_EQ(last[-1], 'a');

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

TEST(const_reverse_rope_iterator, test_default_ctor)
{
    {
        text::detail::const_reverse_rope_iterator it1;
        text::detail::const_reverse_rope_iterator it2;

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

TEST(const_reverse_rope_iterator, test_c_str_ctor)
{
    {
        text::unencoded_rope r_empty("");
        text::detail::const_reverse_rope_iterator it = r_empty.rbegin();

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it != it);
        EXPECT_FALSE(it < it);
        EXPECT_TRUE(it <= it);
        EXPECT_FALSE(it > it);
        EXPECT_TRUE(it >= it);

        EXPECT_EQ(it - it, 0);
    }

    {
        text::unencoded_rope r_a("a");
        text::detail::const_reverse_rope_iterator first = r_a.rbegin();
        text::detail::const_reverse_rope_iterator last = r_a.rend();

        EXPECT_EQ(*last.base(), *first);

        EXPECT_EQ(*first, 'a');
        EXPECT_EQ(first[0], 'a');

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

        EXPECT_EQ(*(last - 1), 'a');
        EXPECT_EQ(last[-1], 'a');

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

TEST(both_rope_iterators, test_larger_ropes)
{
    int const copies = 40;

    {
        text::unencoded_rope r;

        text::string_view tv("text");
        int i = 0;
        for (; i < text::detail::min_children - 1; ++i) {
            r.insert(0, tv);
        }

        text::unencoded_rope extra_ref = r;
        (void)extra_ref;

        for (; i < copies; ++i) {
            r.insert(0, tv);
        }

        {
            text::detail::const_rope_iterator first = r.begin();
            text::detail::const_rope_iterator last = r.end();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("text", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }

        {
            text::detail::const_reverse_rope_iterator first = r.rbegin();
            text::detail::const_reverse_rope_iterator last = r.rend();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("txet", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }
    }

    {
        text::unencoded_rope r;
        text::string t("text");
        for (int i = 0; i < copies; ++i) {
            r.insert(0, t);
        }

        {
            text::detail::const_rope_iterator first = r.begin();
            text::detail::const_rope_iterator last = r.end();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("text", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }

        {
            text::detail::const_reverse_rope_iterator first = r.rbegin();
            text::detail::const_reverse_rope_iterator last = r.rend();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("txet", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }
    }

    {
        text::unencoded_rope r;
        text::repeated_string_view rtv("text", 2);
        for (int i = 0; i < copies; ++i) {
            r.insert(r.size(), rtv);
        }

        {
            text::detail::const_rope_iterator first = r.begin();
            text::detail::const_rope_iterator last = r.end();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("texttext", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }

        {
            text::detail::const_reverse_rope_iterator first = r.rbegin();
            text::detail::const_reverse_rope_iterator last = r.rend();
            text::string const from_first_last(first, last);
            text::string const from_first_repeated(
                text::repeated_string_view("txettxet", copies));
            EXPECT_EQ(from_first_last, from_first_repeated);
        }
    }

    {
        text::repeated_string_view repeated("a bit longer text", 64 * 1024);

        text::unencoded_rope r;
        text::string t(repeated);
        r.insert(0, t);
        r.insert(5, "WAT");

        {
            text::detail::const_rope_iterator first = r.begin();
            text::detail::const_rope_iterator last = r.end();
            text::string const from_first_last(first, last);
            text::string built_up_otherwise("a bitWAT longer text");
            built_up_otherwise +=
                text::repeated_string_view("a bit longer text", 64 * 1024 - 1);
            EXPECT_EQ(from_first_last, built_up_otherwise);
        }

        {
            text::detail::const_reverse_rope_iterator first = r.rbegin();
            text::detail::const_reverse_rope_iterator last = r.rend();
            text::string const from_first_last(first, last);
            text::string built_up_otherwise("a bitWAT longer text");
            built_up_otherwise +=
                text::repeated_string_view("a bit longer text", 64 * 1024 - 1);
            std::reverse(built_up_otherwise.begin(), built_up_otherwise.end());
            EXPECT_EQ(from_first_last, built_up_otherwise);
        }
    }
}
