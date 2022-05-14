// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#define BOOST_TEXT_TESTING
#include <boost/text/text.hpp>
#include <boost/text/string_utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

using string16 = std::basic_string<char16_t>;
using string_view16 = text::basic_string_view<char16_t>;
using text16 = boost::text::
    basic_text<boost::text::nf::fcc, char16_t, std::vector<char16_t>>;
using rope16 = boost::text::
    basic_rope<boost::text::nf::fcc, char16_t, std::vector<char16_t>>;

text16 operator"" _t(char16_t const * str, std::size_t len)
{
    return text16(str, str + len);
}

TEST(text_utf16, test_empty)
{
    text16 t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.cbegin(), t.cend());
    EXPECT_EQ(t.rbegin(), t.rend());
    EXPECT_EQ(t.crbegin(), t.crend());

    EXPECT_EQ(t.begin(), t.cbegin());
    EXPECT_EQ(t.end(), t.cend());
    EXPECT_EQ(t.rbegin(), t.crbegin());
    EXPECT_EQ(t.rend(), t.crend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.storage_code_units(), 0u);
    EXPECT_EQ(t.distance(), 0u);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_code_units(), (std::size_t)PTRDIFF_MAX);

    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);

#if 201703L <= __cplusplus
    static_assert(std::is_swappable<text16>::value, "");
#endif

    t.swap(t);
    EXPECT_TRUE(t == t);

    EXPECT_EQ(t.begin(), begin(t));
    EXPECT_EQ(t.end(), end(t));
    EXPECT_EQ(t.cbegin(), cbegin(t));
    EXPECT_EQ(t.cend(), cend(t));

    EXPECT_EQ(t.rbegin(), rbegin(t));
    EXPECT_EQ(t.rend(), rend(t));
    EXPECT_EQ(t.crbegin(), crbegin(t));
    EXPECT_EQ(t.crend(), crend(t));

    t.clear();
    t.shrink_to_fit();

    std::cout << "t=\"" << t << "\"\n";
}

TEST(text_utf16, test_non_empty_const_interface)
{
    text16 t_a(u"a");
    text16 t_ab(u"ab");

    EXPECT_EQ((std::next(t_a.begin(), t_a.distance())), t_a.end());
    EXPECT_EQ((std::next(t_a.cbegin(), t_a.distance())), t_a.cend());
    EXPECT_EQ((std::next(t_a.rbegin(), t_a.distance())), t_a.rend());
    EXPECT_EQ((std::next(t_a.crbegin(), t_a.distance())), t_a.crend());

    EXPECT_EQ(t_a.begin(), t_a.cbegin());
    EXPECT_EQ(t_a.end(), t_a.cend());
    EXPECT_EQ(t_a.rbegin(), t_a.crbegin());
    EXPECT_EQ(t_a.rend(), t_a.crend());

    EXPECT_EQ(t_ab.begin(), t_ab.cbegin());
    EXPECT_EQ(t_ab.end(), t_ab.cend());
    EXPECT_EQ(t_ab.rbegin(), t_ab.crbegin());
    EXPECT_EQ(t_ab.rend(), t_ab.crend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.distance(), 1u);
    EXPECT_GE(t_a.capacity_bytes(), t_a.distance());

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.distance(), 2u);
    EXPECT_GE(t_ab.capacity_bytes(), t_ab.distance());

    EXPECT_EQ(t_a.max_code_units(), (std::size_t)PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_code_units(), (std::size_t)PTRDIFF_MAX);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);

    text16 const old_t_a(t_a);
    text16 const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    EXPECT_EQ(t_a.begin(), begin(t_a));
    EXPECT_EQ(t_a.end(), end(t_a));
    EXPECT_EQ(t_a.cbegin(), cbegin(t_a));
    EXPECT_EQ(t_a.cend(), cend(t_a));

    EXPECT_EQ(t_a.rbegin(), rbegin(t_a));
    EXPECT_EQ(t_a.rend(), rend(t_a));
    EXPECT_EQ(t_a.crbegin(), crbegin(t_a));
    EXPECT_EQ(t_a.crend(), crend(t_a));
}

TEST(text_utf16, test_ctors)
{
    text16 t;
    EXPECT_EQ(t, u""_t);
    EXPECT_EQ(u""_t, t);

    text16 t2(u"A nonempty string");
    EXPECT_EQ(t2, u"A nonempty string"_t);
    EXPECT_EQ(u"A nonempty string"_t, t2);

    text16 t3(t2);
    EXPECT_EQ(t3, u"A nonempty string"_t);
    EXPECT_EQ(u"A nonempty string"_t, t3);

    text16 t4(std::move(t2));
    EXPECT_EQ(t4, u"A nonempty string"_t);
    EXPECT_EQ(u"A nonempty string"_t, t4);
    EXPECT_EQ(t2, u""_t);
    EXPECT_EQ(u""_t, t2);

    string16 const s(u"An old-school string");
    text16 t5(s);
    EXPECT_EQ(t5, u"An old-school string"_t);
    EXPECT_EQ(u"An old-school string"_t, t5);

    text16 const t5_5(u"a view ");
    text16::text_view const tv(t5_5);
    text16 t6(tv);
    EXPECT_EQ(t6, u"a view "_t);
    EXPECT_EQ(u"a view "_t, t6);

    std::list<char16_t> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text16 t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, u"a list"_t);
    EXPECT_EQ(u"a list"_t, t8);
}

TEST(text_utf16, test_insert)
{
    text16 const ct0(u"a view ");
    text16::text_view const tv(ct0);

    {
        text16 const ct(u"string");

        text16 t0 = ct;
        t0.insert(std::next(t0.begin(), 0), tv);
        EXPECT_EQ(t0, u"a view string"_t);

        text16 t1 = ct;
        t1.insert(std::next(t1.begin(), 1), tv);
        EXPECT_EQ(t1, u"sa view tring"_t);

        text16 t2 = ct;
        t2.insert(std::next(t2.begin(), 2), tv);
        EXPECT_EQ(t2, u"sta view ring"_t);

        text16 t3 = ct;
        t3.insert(std::next(t3.begin(), 3), tv);
        EXPECT_EQ(t3, u"stra view ing"_t);

        text16 t4 = ct;
        t4.insert(std::next(t4.begin(), 4), tv);
        EXPECT_EQ(t4, u"stria view ng"_t);

        text16 t5 = ct;
        t5.insert(std::next(t5.begin(), 5), tv);
        EXPECT_EQ(t5, u"strina view g"_t);

        text16 t6 = ct;
        t6.insert(std::next(t6.begin(), 6), tv);
        EXPECT_EQ(t6, u"stringa view "_t);
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text16 const ct(u"string");
        auto const first = text::utf_32_to_16_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        auto const last = text::utf_32_to_16_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        text16 t0 = ct;
        t0.insert(std::next(t0.begin(), 0), first, last);
        EXPECT_EQ(t0, u"\u004d\u0430\u4e8c\U00010302string"_t);

        text16 t1 = ct;
        t1.insert(std::next(t1.begin(), 1), first, last);
        EXPECT_EQ(t1, u"s\u004d\u0430\u4e8c\U00010302tring"_t);

        text16 t2 = ct;
        t2.insert(std::next(t2.begin(), 2), first, last);
        EXPECT_EQ(t2, u"st\u004d\u0430\u4e8c\U00010302ring"_t);

        text16 t3 = ct;
        t3.insert(std::next(t3.begin(), 3), first, last);
        EXPECT_EQ(t3, u"str\u004d\u0430\u4e8c\U00010302ing"_t);

        text16 t4 = ct;
        t4.insert(std::next(t4.begin(), 4), first, last);
        EXPECT_EQ(t4, u"stri\u004d\u0430\u4e8c\U00010302ng"_t);

        text16 t5 = ct;
        t5.insert(std::next(t5.begin(), 5), first, last);
        EXPECT_EQ(t5, u"strin\u004d\u0430\u4e8c\U00010302g"_t);

        text16 t6 = ct;
        t6.insert(std::next(t6.begin(), 6), first, last);
        EXPECT_EQ(t6, u"string\u004d\u0430\u4e8c\U00010302"_t);
    }

    {
        {
            text16 t(u"e");
            auto const result = t.insert(t.begin(), u"f");
            EXPECT_EQ(t.distance(), 2u);
            EXPECT_EQ(result.begin(), t.begin());
        }
        {
            text16 t(u"e");
            auto const result = t.insert(t.end(), u"f");
            EXPECT_EQ(t.distance(), 2u);
            EXPECT_EQ(result.begin(), std::next(t.begin()));
        }

        char16_t const * combining_diaeresis = u"\u0308";

        {
            text16 t(u"e");
            auto const result = t.insert(t.begin(), combining_diaeresis);
            EXPECT_EQ(t.distance(), 2u);
            EXPECT_EQ(result.begin(), t.begin());
        }
        {
            text16 t(u"e");

            auto result = t.insert(t.end(), combining_diaeresis);
            EXPECT_EQ(t.distance(), 1u);
            EXPECT_EQ(result.begin(), t.begin());

            result = t.insert(t.end(), combining_diaeresis);
            EXPECT_EQ(t.distance(), 1u);
            EXPECT_EQ(result.begin(), t.begin());
        }
        {
            text16 t(u"et");

            auto result = t.insert(std::next(t.begin()), combining_diaeresis);
            EXPECT_EQ(t.distance(), 2u);
            EXPECT_EQ(result.begin(), t.begin());

            result = t.insert(std::next(t.begin()), combining_diaeresis);
            EXPECT_EQ(t.distance(), 2u);
            EXPECT_EQ(result.begin(), t.begin());
        }
    }

    {
        {
            text16 t;
            t.insert(t.begin(), 'g'); // text::grapheme
            EXPECT_EQ(t, text16(u"g"));
        }
        {
            text16 t;
            t.insert(t.begin(), text::grapheme());
            EXPECT_EQ(t, text16(u""));
        }

        {
            text16 t_0(u"g");
            text16 t;
            t.insert(t.end(), *t_0.begin()); // text::grapheme_ref
            EXPECT_EQ(t, text16(u"g"));
        }
        {
            text16 const t_0(u"g");
            text16 t;
            t.insert(t.end(), *t_0.begin());
            EXPECT_EQ(t, text16(u"g"));
        }
        {
            rope16 r(u"g");
            text16 t;
            t.insert(t.end(), *r.begin()); // text::grapheme_ref
            EXPECT_EQ(t, text16(u"g"));
        }
        {
            rope16 const r(u"g");
            text16 t;
            t.insert(t.end(), *r.begin());
            EXPECT_EQ(t, text16(u"g"));
        }
        {
            text16 t;
            t.insert(
                t.begin(),
                text::grapheme_ref<text::rope::iterator::iterator>());
            EXPECT_EQ(t, text16(u""));
        }
    }
}

TEST(text_utf16, test_erase)
{
    text16 const ct(u"string");

    for (std::size_t j = 0, end = ct.distance(); j <= end; ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text16 t = ct;
            text16::text_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            text16 const substr_copy(substr_first, substr_last);
            text16::text_view const after(std::next(ct.begin(), j), ct.end());

            text16 expected(before);
            expected += after;

            t.erase(substr_first, substr_last);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(text_utf16, test_replace)
{
    text16 const ct0(u"REP");
    text16::text_view const replacement(ct0);

    {
        text16 t(u"string");
        t.replace(t.begin(), t.end(), replacement);
        EXPECT_EQ(t, u"REP"_t);
    }

    {
        text16 t(u"string");
        t.replace(t.begin(), t.end(), replacement);
        EXPECT_EQ(t, u"REP"_t);
    }

    {
        text16 t(u"string");
        text16::text_view const new_substr(
            std::next(t.begin(), 2), std::next(t.begin(), 6));
        t.replace(std::next(t.begin(), 0), std::next(t.begin(), 3), new_substr);
        EXPECT_EQ(t, u"ringing"_t);
    }

    {
        text16 t(u"string");
        text16::text_view const new_substr(
            std::next(t.begin(), 0), std::next(t.begin(), 3));
        t.replace(std::next(t.begin(), 3), std::next(t.begin(), 6), new_substr);
        EXPECT_EQ(t, u"strstr"_t);
    }

    text16 const ct(u"string");

    for (std::size_t j = 0, end = ct.distance(); j <= end; ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text16 t = ct;
            text16::text_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            text16 const substr_copy(substr_first, substr_last);
            text16::text_view const after(std::next(ct.begin(), j), ct.end());

            text16 expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr_first, substr_last, replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }

    string_view16 const really_long_replacement(
        u"REPREPREPREPREPREPREPREPREPREP");

    for (std::size_t j = 0, end = ct.distance(); j <= end; ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text16 t = ct;
            text16::text_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            text16 const substr_copy(substr_first, substr_last);
            text16::text_view const after(std::next(ct.begin(), j), ct.end());

            text16 expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr_first, substr_last, really_long_replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(text_utf16, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first =
        text::utf_32_to_16_iterator<uint32_t const *>(utf32, utf32, utf32 + 4);
    auto const final_cp = text::utf_32_to_16_iterator<uint32_t const *>(
        utf32, utf32 + 3, utf32 + 4);
    auto const last = text::utf_32_to_16_iterator<uint32_t const *>(
        utf32, utf32 + 4, utf32 + 4);

    text16 const ct_string(u"string");
    text16 const ct_text(u"text");

    {
        text16 t = ct_string;
        t.replace(t.begin(), t.end(), first, final_cp);
        EXPECT_EQ(t, u"\u004d\u0430\u4e8c"_t);
    }

    {
        text16 t = ct_text;
        t.replace(t.begin(), t.end(), final_cp, last);
        EXPECT_EQ(t, u"\U00010302"_t);
    }

    {
        text16 t = ct_string;
        t.replace(t.begin(), t.end(), first, last);
        EXPECT_EQ(t, u"\u004d\u0430\u4e8c\U00010302"_t);
    }

    for (std::size_t j = 0, end = ct_string.distance(); j <= end; ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            {
                text16 t = ct_string;
                text16::text_view const before(t.begin(), std::next(t.begin(), i));
                auto const substr_first = std::next(t.begin(), i);
                auto const substr_last = std::next(t.begin(), j);
                text16 const substr_copy(substr_first, substr_last);
                text16::text_view const after(std::next(t.begin(), j), t.end());

                text16 expected(before);
                expected.insert(expected.end(), final_cp, last);
                expected.insert(
                    expected.end(),
                    text::as_utf16(after.begin().base(), after.end().base()));

                t.replace(substr_first, substr_last, final_cp, last);
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j
                                       << " erasing '" << substr_copy << "'";
            }

            {
                text16 t = ct_string;
                text16::text_view const before(t.begin(), std::next(t.begin(), i));
                auto const substr_first = std::next(t.begin(), i);
                auto const substr_last = std::next(t.begin(), j);
                text16 const substr_copy(substr_first, substr_last);
                text16::text_view const after(std::next(t.begin(), j), t.end());

                text16 expected(before);
                expected.insert(expected.end(), first, last);
                expected += after;

                t.replace(substr_first, substr_last, first, last);
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j
                                       << " erasing '" << substr_copy << "'";
            }
        }
    }
}

TEST(text_utf16, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (std::size_t i = 0; i < 5000; ++i) {
        utf32_repeated.insert(utf32_repeated.end(), utf32, utf32 + 4);
    }
    auto const first =
        text::utf_32_to_16_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin(),
            utf32_repeated.begin(),
            utf32_repeated.end());
    auto const last =
        text::utf_32_to_16_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin(), utf32_repeated.end(), utf32_repeated.end());

    {
        text16 t(u"string");
        t.replace(t.begin(), t.end(), first, last);
        text16 const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text16 t;
        t.replace(t.begin(), t.end(), first, last);
        text16 const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

TEST(text_utf16, normalization)
{
    uint32_t const circumflex_utf32[] = {0x302};       // ◌̂
    uint32_t const a_with_circumflex_utf32[] = {0xe2}; // â

    auto const r1 = text::as_utf16(circumflex_utf32, circumflex_utf32 + 1);
    string16 const s_circumflex(r1.begin(), r1.end());
    auto const r2 =
        text::as_utf16(a_with_circumflex_utf32, a_with_circumflex_utf32 + 1);
    string16 const s_a_with_circumflex(r2.begin(), r2.end());

    text16 const t_circumflex(s_circumflex);
    text16 const t_a_with_circumflex(s_a_with_circumflex);
    text16 const t_a_with_circumflex_2(u"a\u0302" /*a◌̂*/);

    EXPECT_EQ(t_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex_2.distance(), 1u);

    EXPECT_EQ(t_circumflex, u"\u0302"_t /*◌̂*/);
    EXPECT_EQ(t_a_with_circumflex, u"\u00e2"_t /*â*/);
    EXPECT_EQ(t_a_with_circumflex_2, u"\u00e2"_t /*â*/);

    // insert()

    {
        text16 t = u"aa";
        t.insert(std::next(t.begin(), 0), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t = u"aa";
        t.insert(std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u00e2"
                   u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text16 t = u"aa";
        t.insert(std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, text16(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 0), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u0302\u00e2"
                   u"a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u00e2\u0302"
                   u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, text16(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t = u"aa";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t = u"aa";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u00e2"
                   u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text16 t = u"aa";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text16(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u0302\u00e2"
                   u"a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u00e2\u0302"
                   u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text16(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    // replace()

    {
        text16 t = u"aaa";
        t.replace(t.begin(), std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 1), std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u00e2"
                   u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text16 t = u"aaa";
        t.replace(std::next(t.begin(), 2), t.end(), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, text16(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(t.begin(), std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 1), std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            text16(u"\u00e2\u0302"
                   u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(std::next(t.begin(), 2), t.end(), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, text16(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t = u"aaa";
        t.replace(
            t.begin(),
            std::next(t.begin(), 1),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u00e2"
                   u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 2),
            t.end(),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(t, text16(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            t.begin(),
            std::next(t.begin(), 1),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u0302"
                   u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            text16(u"\u00e2\u0302"
                   u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 2),
            t.end(),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(t, text16(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }
}

TEST(text_utf16, test_sentinel_api)
{
    {
        char16_t const * chars = u"chars";
        text16 s(chars, text::null_sentinel{});
        EXPECT_EQ(s, text16(chars));
    }
}

TEST(text_utf16, test_formatted_output)
{
    {
        std::ostringstream oss;
        oss << text16(u"abc") << text16(u"def");
        EXPECT_EQ(oss.str(), "abcdef");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << text16(u"abc");
        EXPECT_EQ(oss.str(), "       abc");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*')
            << text16(u"abc");
        EXPECT_EQ(oss.str(), "abc*******");
    }

    {
        std::ostringstream oss;
        text16 abc(u"abc");
        text16 def(u"def");
        oss << text16::text_view(abc) << text16::text_view(def);
        EXPECT_EQ(oss.str(), "abcdef");
    }

    {
        std::ostringstream oss;
        text16 abc(u"abc");
        oss << std::setw(10) << text16::text_view(abc);
        EXPECT_EQ(oss.str(), "       abc");
    }

    {
        std::ostringstream oss;
        text16 abc(u"abc");
        oss << std::setw(10) << std::left << std::setfill('*')
            << text16::text_view(abc);
        EXPECT_EQ(oss.str(), "abc*******");
    }
}
