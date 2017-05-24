#include <boost/text/text_view.hpp>

#include <gtest/gtest.h>


using namespace boost;

TEST(reverse_char_iterator, test_default_ctor)
{
    text::detail::reverse_char_iterator it1;
    text::detail::reverse_char_iterator it2;

    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    EXPECT_FALSE(it1 < it2);
    EXPECT_TRUE(it1 <= it2);
    EXPECT_FALSE(it1 > it2);
    EXPECT_TRUE(it1 >= it2);

    EXPECT_EQ(it1 - it2, 0);
    EXPECT_EQ(it2 - it1, 0);
}

TEST(reverse_char_iterator, test_c_str_ctor)
{
    {
        std::string tv_empty("");
        text::detail::reverse_char_iterator it(&tv_empty[0]);

        EXPECT_EQ(*it, '\0');
        EXPECT_EQ(it[0], '\0');

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
        std::string tv_a("a");
        text::detail::reverse_char_iterator first(&*tv_a.rbegin());
        text::detail::reverse_char_iterator last(&*tv_a.rend());

        EXPECT_EQ(last.base(), &*first);

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

            it = last;
            it = 1 - it;
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

TEST(const_reverse_char_iterator, test_default_ctor)
{
    {
        text::detail::const_reverse_char_iterator it1;
        text::detail::const_reverse_char_iterator it2;

        EXPECT_TRUE(it1 == it2);
        EXPECT_FALSE(it1 != it2);
        EXPECT_FALSE(it1 < it2);
        EXPECT_TRUE(it1 <= it2);
        EXPECT_FALSE(it1 > it2);
        EXPECT_TRUE(it1 >= it2);

        EXPECT_EQ(it1 - it2, 0);
        EXPECT_EQ(it2 - it1, 0);
    }

    {
        constexpr text::detail::const_reverse_char_iterator it1;
        constexpr text::detail::const_reverse_char_iterator it2;

        static_assert(it1 == it2, "");
        static_assert(!(it1 != it2), "");
        static_assert(!(it1 < it2), "");
        static_assert(it1 <= it2, "");
        static_assert(!(it1 > it2), "");
        static_assert(it1 >= it2, "");

        static_assert(it1 - it2 == 0, "");
        static_assert(it2 - it1 == 0, "");
    }
}

TEST(const_reverse_char_iterator, test_c_str_ctor)
{
    {
        text::text_view tv_empty("");
        text::detail::const_reverse_char_iterator it = tv_empty.rbegin();

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it != it);
        EXPECT_FALSE(it < it);
        EXPECT_TRUE(it <= it);
        EXPECT_FALSE(it > it);
        EXPECT_TRUE(it >= it);

        EXPECT_EQ(it - it, 0);
    }

    {
        text::text_view tv_a("a", 1);
        text::detail::const_reverse_char_iterator first = tv_a.rbegin();
        text::detail::const_reverse_char_iterator last = tv_a.rend();

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

            it = last;
            it = 1 - it;
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

constexpr text::text_view tv_a("a", 1);
inline constexpr text::detail::const_reverse_char_iterator tv_a_preincremented_begin ()
{ return ++tv_a.rbegin(); }
inline constexpr text::detail::const_reverse_char_iterator tv_a_postincremented_begin ()
{ return tv_a.rbegin()++; }
inline constexpr text::detail::const_reverse_char_iterator tv_a_plusequals1_begin ()
{ return tv_a.rbegin() += 1; }

inline constexpr text::detail::const_reverse_char_iterator tv_a_predecremented_end ()
{ return --tv_a.rend(); }
inline constexpr text::detail::const_reverse_char_iterator tv_a_postdecremented_end ()
{ return tv_a.rend()--; }
inline constexpr text::detail::const_reverse_char_iterator tv_a_minusequals1_end ()
{ return tv_a.rend() -= 1; }

inline constexpr int count_tv_a_elements ()
{
    int retval = 0;
    for (auto it = tv_a.rbegin(); it != tv_a.rend(); ++it) {
        ++retval;
    }
    return retval;
}

TEST(const_reverse_char_iterator, test_c_str_ctor_constexpr)
{
    {
        constexpr text::text_view tv_empty("");
        constexpr text::detail::const_reverse_char_iterator it = tv_empty.rbegin();

        static_assert(it == it, "");
        static_assert(!(it != it), "");
        static_assert(!(it < it), "");
        static_assert(it <= it, "");
        static_assert(!(it > it), "");
        static_assert(it >= it, "");

        static_assert(it - it == 0, "");
    }

    {
        constexpr text::detail::const_reverse_char_iterator first = tv_a.rbegin();
        constexpr text::detail::const_reverse_char_iterator last = tv_a.rend();

        static_assert(*last.base() == *first, "");

        static_assert(*first == 'a', "");
        static_assert(first[0] == 'a', "");

        static_assert(tv_a_preincremented_begin() == last, "");
        static_assert(tv_a_postincremented_begin() == first, "");
        static_assert(tv_a_plusequals1_begin() == last, "");

        {
            constexpr auto it = first + 1;
            static_assert(it == last, "");
        }

        {
            constexpr auto it = 1 + first;
            static_assert(it == last, "");
        }

        static_assert(tv_a_predecremented_end() == first, "");
        static_assert(tv_a_postdecremented_end() == last, "");
        static_assert(tv_a_minusequals1_end() == first, "");

        {
            constexpr auto it = last - 1;
            static_assert(it == first, "");
        }

        {
            constexpr auto it = 1 - last;
            static_assert(it == first, "");
        }

        static_assert(*(last - 1) == 'a', "");
        static_assert(last[-1] == 'a', "");

        static_assert(first + 1 == last, "");
        static_assert(first == last - 1, "");

        static_assert(!(first == last), "");
        static_assert(first != last, "");
        static_assert(first < last, "");
        static_assert(first <= last, "");
        static_assert(!(first > last), "");
        static_assert(!(first >= last), "");

        static_assert(first - last == -1, "");
        static_assert(last - first == 1, "");

        static_assert(count_tv_a_elements() == 1, "ensure that iterator works in a constexpr function loop");
    }
}
