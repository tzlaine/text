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
        text::detail::reverse_char_iterator last(&*tv_a.rbegin() - 1);

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
        text::text_view tv_a("a");
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

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

constexpr text::text_view tv_a("a");
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

#endif

TEST(const_repeated_chars_iterator, test_default_ctor)
{
    {
        text::detail::const_repeated_chars_iterator it1;
        text::detail::const_repeated_chars_iterator it2;

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
        constexpr text::detail::const_repeated_chars_iterator it1;
        constexpr text::detail::const_repeated_chars_iterator it2;

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

TEST(const_repeated_chars_iterator, test_c_str_ctor)
{
    {
        text::text_view tv_empty("");
        text::detail::const_repeated_chars_iterator it(tv_empty.begin(), tv_empty.size(), 0);

        EXPECT_EQ(it, it);
        EXPECT_FALSE(it != it);
        EXPECT_FALSE(it < it);
        EXPECT_TRUE(it <= it);
        EXPECT_FALSE(it > it);
        EXPECT_TRUE(it >= it);

        EXPECT_EQ(it - it, 0);
    }

    {
        text::text_view tv_abc("abc");
        text::detail::const_repeated_chars_iterator first(tv_abc.begin(), tv_abc.size(), 0);
        text::detail::const_repeated_chars_iterator last(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size());

        EXPECT_EQ(*first, 'a');
        EXPECT_EQ(first[0], 'a');
        EXPECT_EQ(first[1], 'b');
        EXPECT_EQ(first[2], 'c');
        EXPECT_EQ(first[3], 'a');
        EXPECT_EQ(first[4], 'b');
        EXPECT_EQ(first[5], 'c');
        EXPECT_EQ(first[6], 'a');
        EXPECT_EQ(first[7], 'b');
        EXPECT_EQ(first[8], 'c');

        EXPECT_EQ((first + 1)[0], 'b');
        EXPECT_EQ((first + 1)[1], 'c');
        EXPECT_EQ((first + 1)[2], 'a');
        EXPECT_EQ((first + 1)[3], 'b');
        EXPECT_EQ((first + 1)[4], 'c');
        EXPECT_EQ((first + 1)[5], 'a');
        EXPECT_EQ((first + 1)[6], 'b');
        EXPECT_EQ((first + 1)[7], 'c');

        EXPECT_EQ((first + 2)[0], 'c');
        EXPECT_EQ((first + 2)[1], 'a');
        EXPECT_EQ((first + 2)[2], 'b');
        EXPECT_EQ((first + 2)[3], 'c');
        EXPECT_EQ((first + 2)[4], 'a');
        EXPECT_EQ((first + 2)[5], 'b');
        EXPECT_EQ((first + 2)[6], 'c');

        EXPECT_EQ(last[-1], 'c');
        EXPECT_EQ(last[-2], 'b');
        EXPECT_EQ(last[-3], 'a');
        EXPECT_EQ(last[-4], 'c');
        EXPECT_EQ(last[-5], 'b');
        EXPECT_EQ(last[-6], 'a');
        EXPECT_EQ(last[-7], 'c');
        EXPECT_EQ(last[-8], 'b');
        EXPECT_EQ(last[-9], 'a');

        EXPECT_EQ((last - 1)[-1], 'b');
        EXPECT_EQ((last - 1)[-2], 'a');
        EXPECT_EQ((last - 1)[-3], 'c');
        EXPECT_EQ((last - 1)[-4], 'b');
        EXPECT_EQ((last - 1)[-5], 'a');
        EXPECT_EQ((last - 1)[-6], 'c');
        EXPECT_EQ((last - 1)[-7], 'b');
        EXPECT_EQ((last - 1)[-8], 'a');

        EXPECT_EQ((last - 2)[-1], 'a');
        EXPECT_EQ((last - 2)[-2], 'c');
        EXPECT_EQ((last - 2)[-3], 'b');
        EXPECT_EQ((last - 2)[-4], 'a');
        EXPECT_EQ((last - 2)[-5], 'c');
        EXPECT_EQ((last - 2)[-6], 'b');
        EXPECT_EQ((last - 2)[-7], 'a');

        {
            auto it = first;
            EXPECT_EQ(++it, first + 1);

            it = first;
            EXPECT_EQ(it++, first);

            it = first;
            it++;
            EXPECT_EQ(it, first + 1);

            it = first;
            it += 1;
            EXPECT_EQ(it, first + 1);

            it = first;
            it = 1 + it;
            EXPECT_EQ(it, first + 1);
        }
        {
            auto it = last;
            EXPECT_EQ(--it, last - 1);

            it = last;
            EXPECT_EQ(it--, last);

            it = last;
            it--;
            EXPECT_EQ(it, last - 1);

            it = last;
            it -= 1;
            EXPECT_EQ(it, last - 1);

            it = last;
            it = 1 - it;
            EXPECT_EQ(it, last - 1);
        }

        auto const it_0 = first;
        auto const it_1 = first + 1;
        auto const it_2 = first + 2;
        auto const it_3 = first + 3;
        auto const it_4 = last;

        EXPECT_EQ(it_0, it_0);
        EXPECT_FALSE(it_0 != it_0);
        EXPECT_FALSE(it_0 < it_0);
        EXPECT_TRUE(it_0 <= it_0);
        EXPECT_FALSE(it_0 > it_0);
        EXPECT_TRUE(it_0 >= it_0);

        EXPECT_EQ(it_4, it_4);
        EXPECT_FALSE(it_4 != it_4);
        EXPECT_FALSE(it_4 < it_4);
        EXPECT_TRUE(it_4 <= it_4);
        EXPECT_FALSE(it_4 > it_4);
        EXPECT_TRUE(it_4 >= it_4);

        EXPECT_TRUE(it_0 < it_1);
        EXPECT_TRUE(it_0 < it_2);
        EXPECT_TRUE(it_0 < it_3);
        EXPECT_TRUE(it_0 < it_4);

        EXPECT_TRUE(it_1 < it_2);
        EXPECT_TRUE(it_1 < it_3);
        EXPECT_TRUE(it_1 < it_4);

        EXPECT_TRUE(it_2 < it_3);
        EXPECT_TRUE(it_2 < it_4);

        EXPECT_TRUE(it_3 < it_4);

        EXPECT_TRUE(it_4 > it_0);
        EXPECT_TRUE(it_4 > it_1);
        EXPECT_TRUE(it_4 > it_2);
        EXPECT_TRUE(it_4 > it_3);

        EXPECT_TRUE(it_3 > it_0);
        EXPECT_TRUE(it_3 > it_1);
        EXPECT_TRUE(it_3 > it_2);

        EXPECT_TRUE(it_2 > it_0);
        EXPECT_TRUE(it_2 > it_1);

        EXPECT_TRUE(it_1 > it_0);

        EXPECT_NE(it_0, it_1);
        EXPECT_NE(it_0, it_2);
        EXPECT_NE(it_0, it_3);
        EXPECT_NE(it_0, it_4);

        EXPECT_NE(it_1, it_2);
        EXPECT_NE(it_1, it_3);
        EXPECT_NE(it_1, it_4);

        EXPECT_NE(it_2, it_3);
        EXPECT_NE(it_2, it_4);

        EXPECT_NE(it_3, it_4);

        EXPECT_EQ(it_0 - it_1, -1);
        EXPECT_EQ(it_0 - it_2, -2);
        EXPECT_EQ(it_0 - it_3, -3);
        EXPECT_EQ(it_0 - it_4, -9);

        EXPECT_EQ(it_1 - it_2, -1);
        EXPECT_EQ(it_1 - it_3, -2);
        EXPECT_EQ(it_1 - it_4, -8);

        EXPECT_EQ(it_2 - it_3, -1);
        EXPECT_EQ(it_2 - it_4, -7);

        EXPECT_EQ(it_3 - it_4, -6);

        EXPECT_EQ(it_1 - it_0, 1);
        EXPECT_EQ(it_2 - it_0, 2);
        EXPECT_EQ(it_3 - it_0, 3);
        EXPECT_EQ(it_4 - it_0, 9);

        EXPECT_EQ(it_2 - it_1, 1);
        EXPECT_EQ(it_3 - it_1, 2);
        EXPECT_EQ(it_4 - it_1, 8);

        EXPECT_EQ(it_3 - it_2, 1);
        EXPECT_EQ(it_4 - it_2, 7);

        EXPECT_EQ(it_4 - it_3, 6);

        EXPECT_EQ(it_0 - it_0, 0);
        EXPECT_EQ(it_1 - it_1, 0);
        EXPECT_EQ(it_2 - it_2, 0);
        EXPECT_EQ(it_3 - it_3, 0);
        EXPECT_EQ(it_4 - it_4, 0);
    }
}

TEST(const_reverse_repeated_chars_iterator, test_c_str_ctor)
{
    {
        text::text_view tv_abc("abc");
        text::detail::const_reverse_repeated_chars_iterator first(
            text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
        );
        text::detail::const_reverse_repeated_chars_iterator last(
            text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
        );

        EXPECT_EQ(*first, 'c');
        EXPECT_EQ(first[0], 'c');
        EXPECT_EQ(first[1], 'b');
        EXPECT_EQ(first[2], 'a');
        EXPECT_EQ(first[3], 'c');
        EXPECT_EQ(first[4], 'b');
        EXPECT_EQ(first[5], 'a');
        EXPECT_EQ(first[6], 'c');

        auto it_0 = first;
        auto it_1 = first + 1;
        auto it_2 = first + 2;
        auto it_3 = first + 3;
        auto it_4 = last;
        (void)it_0;
        (void)it_1;
        (void)it_2;
        (void)it_3;
        (void)it_4;

        EXPECT_TRUE(it_0 == it_0);
        EXPECT_FALSE(it_0 != it_0);
        EXPECT_FALSE(it_0 < it_0);
        EXPECT_TRUE(it_0 <= it_0);
        EXPECT_FALSE(it_0 > it_0);
        EXPECT_TRUE(it_0 >= it_0);

        EXPECT_TRUE(it_4 == it_4);
        EXPECT_FALSE(it_4 != it_4);
        EXPECT_FALSE(it_4 < it_4);
        EXPECT_TRUE(it_4 <= it_4);
        EXPECT_FALSE(it_4 > it_4);
        EXPECT_TRUE(it_4 >= it_4);

        EXPECT_TRUE(it_0 < it_1);
        EXPECT_TRUE(it_0 < it_2);
        EXPECT_TRUE(it_0 < it_3);
        EXPECT_TRUE(it_0 < it_4);

        EXPECT_TRUE(it_1 < it_2);
        EXPECT_TRUE(it_1 < it_3);
        EXPECT_TRUE(it_1 < it_4);

        EXPECT_TRUE(it_2 < it_3);
        EXPECT_TRUE(it_2 < it_4);

        EXPECT_TRUE(it_3 < it_4);

        EXPECT_TRUE(it_4 > it_0);
        EXPECT_TRUE(it_4 > it_1);
        EXPECT_TRUE(it_4 > it_2);
        EXPECT_TRUE(it_4 > it_3);

        EXPECT_TRUE(it_3 > it_0);
        EXPECT_TRUE(it_3 > it_1);
        EXPECT_TRUE(it_3 > it_2);

        EXPECT_TRUE(it_2 > it_0);
        EXPECT_TRUE(it_2 > it_1);

        EXPECT_TRUE(it_1 > it_0);

        EXPECT_TRUE(it_0 != it_1);
        EXPECT_TRUE(it_0 != it_2);
        EXPECT_TRUE(it_0 != it_3);
        EXPECT_TRUE(it_0 != it_4);

        EXPECT_TRUE(it_1 != it_2);
        EXPECT_TRUE(it_1 != it_3);
        EXPECT_TRUE(it_1 != it_4);

        EXPECT_TRUE(it_2 != it_3);
        EXPECT_TRUE(it_2 != it_4);

        EXPECT_TRUE(it_3 != it_4);

        EXPECT_EQ(it_0 - it_1, -1);
        EXPECT_EQ(it_0 - it_2, -2);
        EXPECT_EQ(it_0 - it_3, -3);
        EXPECT_EQ(it_0 - it_4, -9);
    }
}

#ifndef BOOST_TEXT_NO_CXX14_CONSTEXPR

constexpr text::text_view tv_abc("abc");

inline constexpr text::detail::const_repeated_chars_iterator tv_abc_preincremented_begin_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 0); return ++it; }
inline constexpr text::detail::const_repeated_chars_iterator tv_abc_postincremented_begin_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 0); return it++; }
inline constexpr text::detail::const_repeated_chars_iterator tv_abc_plusequals9_begin_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 0); return it += 9; }

inline constexpr text::detail::const_repeated_chars_iterator tv_abc_predecremented_end_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size()); return --it; }
inline constexpr text::detail::const_repeated_chars_iterator tv_abc_postdecremented_end_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size()); return it--; }
inline constexpr text::detail::const_repeated_chars_iterator tv_abc_minusequals9_end_repeat ()
{ text::detail::const_repeated_chars_iterator it(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size()); return it -= 9; }

inline constexpr int count_tv_abc_elements_repeat ()
{
    int retval = 0;
    text::detail::const_repeated_chars_iterator first(tv_abc.begin(), tv_abc.size(), 0);
    text::detail::const_repeated_chars_iterator last(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size());
    for (auto it = first; it != last; ++it) {
        ++retval;
    }
    return retval;
}

TEST(const_repeated_chars_iterator, test_c_str_ctor_constexpr)
{
    {
        constexpr text::text_view tv_empty("");
        constexpr text::detail::const_repeated_chars_iterator it(tv_empty.begin(), tv_empty.size(), 0);

        static_assert(it == it, "");
        static_assert(!(it != it), "");
        static_assert(!(it < it), "");
        static_assert(it <= it, "");
        static_assert(!(it > it), "");
        static_assert(it >= it, "");

        static_assert(it - it == 0, "");
    }

    {
        constexpr text::detail::const_repeated_chars_iterator first(tv_abc.begin(), tv_abc.size(), 0);
        constexpr text::detail::const_repeated_chars_iterator last(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size());

        static_assert(*first == 'a', "");
        static_assert(first[0] == 'a', "");
        static_assert(first[1] == 'b', "");
        static_assert(first[2] == 'c', "");
        static_assert(first[3] == 'a', "");
        static_assert(first[4] == 'b', "");
        static_assert(first[5] == 'c', "");
        static_assert(first[6] == 'a', "");

        static_assert(tv_abc_plusequals9_begin_repeat() == last, "");

        constexpr auto it_0 = first;
        constexpr auto it_1 = first + 1;
        constexpr auto it_2 = first + 2;
        constexpr auto it_3 = first + 3;
        constexpr auto it_4 = last;
        (void)it_0;
        (void)it_1;
        (void)it_2;
        (void)it_3;
        (void)it_4;

        static_assert(it_0 == it_0, "");
        static_assert(!(it_0 != it_0), "");
        static_assert(!(it_0 < it_0), "");
        static_assert(it_0 <= it_0, "");
        static_assert(!(it_0 > it_0), "");
        static_assert(it_0 >= it_0, "");

        static_assert(it_4 == it_4, "");
        static_assert(!(it_4 != it_4), "");
        static_assert(!(it_4 < it_4), "");
        static_assert(it_4 <= it_4, "");
        static_assert(!(it_4 > it_4), "");
        static_assert(it_4 >= it_4, "");

        static_assert(it_0 < it_1, "");
        static_assert(it_0 < it_2, "");
        static_assert(it_0 < it_3, "");
        static_assert(it_0 < it_4, "");

        static_assert(it_1 < it_2, "");
        static_assert(it_1 < it_3, "");
        static_assert(it_1 < it_4, "");

        static_assert(it_2 < it_3, "");
        static_assert(it_2 < it_4, "");

        static_assert(it_3 < it_4, "");

        static_assert(it_4 > it_0, "");
        static_assert(it_4 > it_1, "");
        static_assert(it_4 > it_2, "");
        static_assert(it_4 > it_3, "");

        static_assert(it_3 > it_0, "");
        static_assert(it_3 > it_1, "");
        static_assert(it_3 > it_2, "");

        static_assert(it_2 > it_0, "");
        static_assert(it_2 > it_1, "");

        static_assert(it_1 > it_0, "");

        static_assert(it_0 != it_1, "");
        static_assert(it_0 != it_2, "");
        static_assert(it_0 != it_3, "");
        static_assert(it_0 != it_4, "");

        static_assert(it_1 != it_2, "");
        static_assert(it_1 != it_3, "");
        static_assert(it_1 != it_4, "");

        static_assert(it_2 != it_3, "");
        static_assert(it_2 != it_4, "");

        static_assert(it_3 != it_4, "");

        static_assert(it_0 - it_1 == -1, "");
        static_assert(it_0 - it_2 == -2, "");
        static_assert(it_0 - it_3 == -3, "");
        static_assert(it_0 - it_4 == -9, "");

        static_assert(it_1 - it_2 == -1, "");
        static_assert(it_1 - it_3 == -2, "");
        static_assert(it_1 - it_4 == -8, "");

        static_assert(it_2 - it_3 == -1, "");
        static_assert(it_2 - it_4 == -7, "");

        static_assert(it_3 - it_4 == -6, "");

        static_assert(it_1 - it_0 == 1, "");
        static_assert(it_2 - it_0 == 2, "");
        static_assert(it_3 - it_0 == 3, "");
        static_assert(it_4 - it_0 == 9, "");

        static_assert(it_2 - it_1 == 1, "");
        static_assert(it_3 - it_1 == 2, "");
        static_assert(it_4 - it_1 == 8, "");

        static_assert(it_3 - it_2 == 1, "");
        static_assert(it_4 - it_2 == 7, "");

        static_assert(it_4 - it_3 == 6, "");

        static_assert(it_0 - it_0 == 0, "");
        static_assert(it_1 - it_1 == 0, "");
        static_assert(it_2 - it_2 == 0, "");
        static_assert(it_3 - it_3 == 0, "");
        static_assert(it_4 - it_4 == 0, "");

        static_assert(count_tv_abc_elements_repeat() == 9, "ensure that iterator works in a constexpr function loop");
    }
}

constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_preincremented_begin_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
    );
    return ++it;
}
constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_postincremented_begin_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
    );
    return it++;
}
constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_plusequals9_begin_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
    );
    return it += 9;
}

constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_predecremented_end_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
    );
    return --it;
}
constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_postdecremented_end_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
    );
    return it--;
}
constexpr text::detail::const_reverse_repeated_chars_iterator reverse_tv_abc_minusequals9_end_repeat ()
{
    text::detail::const_reverse_repeated_chars_iterator it(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
    );
    return it -= 9;
}

constexpr int reverse_count_tv_abc_elements_repeat ()
{
    int retval = 0;
    text::detail::const_reverse_repeated_chars_iterator first(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
    );
    text::detail::const_reverse_repeated_chars_iterator last(
        text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
    );
    for (auto it = first; it != last; ++it) {
        ++retval;
    }
    return retval;
}

TEST(const_reverse_repeated_chars_iterator, test_c_str_ctor_constexpr)
{
    {
        constexpr text::text_view tv_empty("");
        constexpr text::detail::const_reverse_repeated_chars_iterator it(
            text::detail::const_repeated_chars_iterator(tv_empty.begin(), tv_empty.size(), tv_empty.size())
        );

        static_assert(it == it, "");
        static_assert(!(it != it), "");
        static_assert(!(it < it), "");
        static_assert(it <= it, "");
        static_assert(!(it > it), "");
        static_assert(it >= it, "");

        static_assert(it - it == 0, "");
    }

    {
        constexpr text::detail::const_reverse_repeated_chars_iterator first(
            text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 3 * tv_abc.size())
        );
        constexpr text::detail::const_reverse_repeated_chars_iterator last(
            text::detail::const_repeated_chars_iterator(tv_abc.begin(), tv_abc.size(), 0)
        );

        static_assert(*first == 'c', "");
        static_assert(first[0] == 'c', "");
        static_assert(first[1] == 'b', "");
        static_assert(first[2] == 'a', "");
        static_assert(first[3] == 'c', "");
        static_assert(first[4] == 'b', "");
        static_assert(first[5] == 'a', "");
        static_assert(first[6] == 'c', "");

        static_assert(reverse_tv_abc_plusequals9_begin_repeat() == last, "");

        constexpr auto it_0 = first;
        constexpr auto it_1 = first + 1;
        constexpr auto it_2 = first + 2;
        constexpr auto it_3 = first + 3;
        constexpr auto it_4 = last;
        (void)it_0;
        (void)it_1;
        (void)it_2;
        (void)it_3;
        (void)it_4;

        static_assert(it_0 == it_0, "");
        static_assert(!(it_0 != it_0), "");
        static_assert(!(it_0 < it_0), "");
        static_assert(it_0 <= it_0, "");
        static_assert(!(it_0 > it_0), "");
        static_assert(it_0 >= it_0, "");

        static_assert(it_4 == it_4, "");
        static_assert(!(it_4 != it_4), "");
        static_assert(!(it_4 < it_4), "");
        static_assert(it_4 <= it_4, "");
        static_assert(!(it_4 > it_4), "");
        static_assert(it_4 >= it_4, "");

        static_assert(it_0 < it_1, "");
        static_assert(it_0 < it_2, "");
        static_assert(it_0 < it_3, "");
        static_assert(it_0 < it_4, "");

        static_assert(it_1 < it_2, "");
        static_assert(it_1 < it_3, "");
        static_assert(it_1 < it_4, "");

        static_assert(it_2 < it_3, "");
        static_assert(it_2 < it_4, "");

        static_assert(it_3 < it_4, "");

        static_assert(it_4 > it_0, "");
        static_assert(it_4 > it_1, "");
        static_assert(it_4 > it_2, "");
        static_assert(it_4 > it_3, "");

        static_assert(it_3 > it_0, "");
        static_assert(it_3 > it_1, "");
        static_assert(it_3 > it_2, "");

        static_assert(it_2 > it_0, "");
        static_assert(it_2 > it_1, "");

        static_assert(it_1 > it_0, "");

        static_assert(it_0 != it_1, "");
        static_assert(it_0 != it_2, "");
        static_assert(it_0 != it_3, "");
        static_assert(it_0 != it_4, "");

        static_assert(it_1 != it_2, "");
        static_assert(it_1 != it_3, "");
        static_assert(it_1 != it_4, "");

        static_assert(it_2 != it_3, "");
        static_assert(it_2 != it_4, "");

        static_assert(it_3 != it_4, "");

        static_assert(it_0 - it_1 == -1, "");
        static_assert(it_0 - it_2 == -2, "");
        static_assert(it_0 - it_3 == -3, "");
        static_assert(it_0 - it_4 == -9, "");

        static_assert(it_1 - it_2 == -1, "");
        static_assert(it_1 - it_3 == -2, "");
        static_assert(it_1 - it_4 == -8, "");

        static_assert(it_2 - it_3 == -1, "");
        static_assert(it_2 - it_4 == -7, "");

        static_assert(it_3 - it_4 == -6, "");

        static_assert(it_1 - it_0 == 1, "");
        static_assert(it_2 - it_0 == 2, "");
        static_assert(it_3 - it_0 == 3, "");
        static_assert(it_4 - it_0 == 9, "");

        static_assert(it_2 - it_1 == 1, "");
        static_assert(it_3 - it_1 == 2, "");
        static_assert(it_4 - it_1 == 8, "");

        static_assert(it_3 - it_2 == 1, "");
        static_assert(it_4 - it_2 == 7, "");

        static_assert(it_4 - it_3 == 6, "");

        static_assert(it_0 - it_0 == 0, "");
        static_assert(it_1 - it_1 == 0, "");
        static_assert(it_2 - it_2 == 0, "");
        static_assert(it_3 - it_3 == 0, "");
        static_assert(it_4 - it_4 == 0, "");

        static_assert(reverse_count_tv_abc_elements_repeat() == 9, "ensure that iterator works in a constexpr function loop");
    }
}

#endif
