// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/rope.hpp>
#include <boost/text/string_utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>

#include <list>
#include <vector>


using namespace boost;

using text_view16 =
    boost::text::basic_text_view<boost::text::nf::fcc, char16_t>;
using rope16 = boost::text::
    basic_rope<boost::text::nf::fcc, char16_t, std::vector<char16_t>>;

rope16 operator"" _t(char16_t const * str, std::size_t len)
{
    return rope16(str, str + len);
}

TEST(rope_utf16, test_empty)
{
    rope16 t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.storage_code_units(), 0u);
    EXPECT_EQ(t.distance(), 0u);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_code_units(), (std::size_t)PTRDIFF_MAX);

    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);

    t.swap(t);
    EXPECT_TRUE(t == t);

#if 201703L <= __cplusplus
    static_assert(std::is_swappable<rope16>::value, "");
#endif

    t.clear();

    std::cout << "r=\"" << t << "\"\n";

    {
        rope16 t2(rope16::string{});
        EXPECT_TRUE(t == t2);
    }
}

TEST(rope_utf16, test_non_empty_const_interface)
{
    rope16 t_a(u"a");
    rope16 t_ab(u"ab");

    EXPECT_EQ(std::distance(t_a.begin(), t_a.end()), 1);
    EXPECT_EQ(std::distance(t_a.rbegin(), t_a.rend()), 1);

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.storage_code_units(), 1u);
    EXPECT_EQ(t_a.distance(), 1u);

    rope16::rope_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.storage_code_units(), 2u);
    EXPECT_EQ(t_ab.distance(), 2u);

    rope16::rope_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_a.max_code_units(), (std::size_t)PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_code_units(), (std::size_t)PTRDIFF_MAX);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);

    EXPECT_FALSE(t_a == u"ab"_t);
    EXPECT_TRUE(t_a != u"ab"_t);

    EXPECT_FALSE(u"a"_t == t_ab);
    EXPECT_TRUE(u"a"_t != t_ab);

    EXPECT_EQ(t_a, u"a"_t);

    rope16 const old_t_a(t_a);
    rope16 const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    {
        EXPECT_EQ(t_a, u"a"_t);
        EXPECT_EQ(t_ab, u"ab"_t);
    }
}

TEST(rope_utf16, test_ctors)
{
    rope16 t;
    EXPECT_EQ(t, u""_t);
    EXPECT_EQ(u""_t, t);

    rope16 t2(u"A nonemtpy string");
    EXPECT_EQ(t2, u"A nonemtpy string"_t);
    EXPECT_EQ(u"A nonemtpy string"_t, t2);

    rope16 t3(t2);
    EXPECT_EQ(t3, u"A nonemtpy string"_t);
    EXPECT_EQ(u"A nonemtpy string"_t, t3);

    rope16 t4(std::move(t2));
    EXPECT_EQ(t4, u"A nonemtpy string"_t);
    EXPECT_EQ(u"A nonemtpy string"_t, t4);
    EXPECT_EQ(t2, u""_t);
    EXPECT_EQ(u""_t, t2);

    char16_t const str_array[] = u"An old-school string";
    rope16::string const s(std::begin(str_array), std::end(str_array) - 1);
    rope16 t5{rope16::string(s)};
    EXPECT_EQ(t5, u"An old-school string"_t);
    EXPECT_EQ(u"An old-school string"_t, t5);

    rope16::string_view const tv(u"a view ");
    rope16 t6(tv);
    EXPECT_EQ(t6, u"a view "_t);
    EXPECT_EQ(u"a view "_t, t6);

    std::list<char16_t> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    rope16 t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, u"a list"_t);
    EXPECT_EQ(u"a list"_t, t8);
}

TEST(rope_utf16, test_assignment)
{
    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t = t2;
        EXPECT_EQ(t, u"A nonemtpy string"_t);
        EXPECT_EQ(t2, u"A nonemtpy string"_t);
    }

    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t2 = t;
        EXPECT_EQ(t, u""_t);
        EXPECT_EQ(t2, u""_t);
    }

    {
        rope16 t(u"small");
        EXPECT_EQ(t, u"small"_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t = t2;
        EXPECT_EQ(t, u"A nonemtpy string"_t);
        EXPECT_EQ(t2, u"A nonemtpy string"_t);
    }

    {
        rope16 t(u"small");
        EXPECT_EQ(t, u"small"_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t2 = t;
        EXPECT_EQ(t, u"small"_t);
        EXPECT_EQ(t2, u"small"_t);
    }

    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2;
        EXPECT_EQ(t2, u""_t);

        t = t2;
        EXPECT_EQ(t, u""_t);
        EXPECT_EQ(t2, u""_t);
    }

    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2;
        EXPECT_EQ(t2, u""_t);

        t2 = t;
        EXPECT_EQ(t, u""_t);
        EXPECT_EQ(t2, u""_t);
    }

    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t = std::move(t2);
        EXPECT_EQ(t, u"A nonemtpy string"_t);
        EXPECT_EQ(t2, u""_t);
    }

    {
        rope16 t;
        EXPECT_EQ(t, u""_t);
        rope16 t2(u"A nonemtpy string");
        EXPECT_EQ(t2, u"A nonemtpy string"_t);

        t = std::move(t2);
        EXPECT_EQ(t, u"A nonemtpy string"_t);
        EXPECT_EQ(t2, u""_t);
    }

    {
        char16_t const str_array[] = u"An old-school string";
        rope16::string const s(std::begin(str_array), std::end(str_array) - 1);
        rope16 t;
        t = rope16::string(s);
        EXPECT_EQ(t, u"An old-school string"_t);
    }

    {
        rope16::string_view const tv(u"a view ");
        rope16 t;
        t = tv;
        EXPECT_EQ(t, u"a view "_t);
    }
}

TEST(rope_utf16, test_iterators_and_index)
{
    rope16 empty;

    {
        int dist = 0;
        for (auto gr : empty) {
            (void)gr;
            ++dist;
        }
        EXPECT_EQ(dist, 0);
    }

    {
        rope16::iterator first = empty.begin();
        rope16::iterator last = empty.end();
        int dist = 0;
        while (first != last) {
            ++dist;
            ++first;
        }
        EXPECT_EQ(dist, 0);
    }

    {
        rope16::reverse_iterator first = empty.rbegin();
        rope16::reverse_iterator last = empty.rend();
        int dist = 0;
        while (first != last) {
            ++dist;
            ++first;
        }
        EXPECT_EQ(dist, 0);
    }

    rope16 non_empty(u"non-empty");

    {
        std::vector<char16_t> vec;

        rope16::reverse_iterator const r_it_begin = non_empty.rbegin();
        rope16::reverse_iterator const r_it_end = non_empty.rend();

        rope16::reverse_iterator r_it = r_it_begin;
        while (r_it != r_it_end) {
            for (auto c : *r_it) {
                vec.push_back(c);
            }
            ++r_it;
        }

        std::reverse(vec.begin(), vec.end());
        EXPECT_TRUE(algorithm::equal(
            r_it_end.base().base(),
            r_it_begin.base().base(),
            vec.begin(),
            vec.end()));
    }
}

TEST(rope_utf16, test_misc)
{
    {
        rope16 t(u"some text");
        t.clear();
        EXPECT_EQ(t.storage_code_units(), 0u);
        EXPECT_EQ(t.distance(), 0u);
    }

    {
        rope16 t1(u"some");
        rope16 t2(u"text");
        t1.swap(t2);
        EXPECT_EQ(t1, u"text"_t);
        EXPECT_EQ(t2, u"some"_t);
    }
}

TEST(rope_utf16, test_substr)
{
    char16_t const str_array[] = u"be careful about its location; ";
    rope16::string const s(std::begin(str_array), std::end(str_array) - 1);
    rope16 const r = rope16(u"When writing a specialization, ") + s +
                     rope16::string_view(
                         u"or to make it compile will be such a trial as to "
                         u"kindle its self-immolation") +
                     rope16::string_view(u"...");

    EXPECT_EQ(rope16(std::prev(r.end(), 4), std::prev(r.end(), 1)), u"n.."_t);

    for (int i = 0, i_end = r.distance(); i < i_end; ++i) {
        for (int j = i, j_end = r.distance(); j < j_end; ++j) {
            auto const first = std::next(r.begin(), i);
            auto const last = std::next(r.begin(), j);
            rope16 const substr(first, last);
            rope16::rope_view const rv(first, last);
            EXPECT_EQ(substr, rv);
        }
    }
}

TEST(rope_utf16, test_insert)
{
    rope16::text const t(u"a view ");
    text_view16 const tv(t);

    {
        rope16 const ct(u"string");

        rope16 t0 = ct;
        t0.insert(std::next(t0.begin(), 0), tv);
        EXPECT_EQ(t0, u"a view string"_t);

        rope16 t1 = ct;
        t1.insert(std::next(t1.begin(), 1), tv);
        EXPECT_EQ(t1, u"sa view tring"_t);

        rope16 t2 = ct;
        t2.insert(std::next(t2.begin(), 2), tv);
        EXPECT_EQ(t2, u"sta view ring"_t);

        rope16 t3 = ct;
        t3.insert(std::next(t3.begin(), 3), tv);
        EXPECT_EQ(t3, u"stra view ing"_t);

        rope16 t4 = ct;
        t4.insert(std::next(t4.begin(), 4), tv);
        EXPECT_EQ(t4, u"stria view ng"_t);

        rope16 t5 = ct;
        t5.insert(std::next(t5.begin(), 5), tv);
        EXPECT_EQ(t5, u"strina view g"_t);

        rope16 t6 = ct;
        t6.insert(std::next(t6.begin(), 6), tv);
        EXPECT_EQ(t6, u"stringa view "_t);
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        rope16 const ct(u"string");
        auto const first = text::utf_32_to_16_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        auto const last = text::utf_32_to_16_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        rope16 t0 = ct;
        t0.insert(std::next(t0.begin(), 0), first, last);
        EXPECT_EQ(t0, u"\u004d\u0430\u4e8c\U00010302string"_t);

        rope16 t1 = ct;
        t1.insert(std::next(t1.begin(), 1), first, last);
        EXPECT_EQ(t1, u"s\u004d\u0430\u4e8c\U00010302tring"_t);

        rope16 t2 = ct;
        t2.insert(std::next(t2.begin(), 2), first, last);
        EXPECT_EQ(t2, u"st\u004d\u0430\u4e8c\U00010302ring"_t);

        rope16 t3 = ct;
        t3.insert(std::next(t3.begin(), 3), first, last);
        EXPECT_EQ(t3, u"str\u004d\u0430\u4e8c\U00010302ing"_t);

        rope16 t4 = ct;
        t4.insert(std::next(t4.begin(), 4), first, last);
        EXPECT_EQ(t4, u"stri\u004d\u0430\u4e8c\U00010302ng"_t);

        rope16 t5 = ct;
        t5.insert(std::next(t5.begin(), 5), first, last);
        EXPECT_EQ(t5, u"strin\u004d\u0430\u4e8c\U00010302g"_t);

        rope16 t6 = ct;
        t6.insert(std::next(t6.begin(), 6), first, last);
        EXPECT_EQ(t6, u"string\u004d\u0430\u4e8c\U00010302"_t);
    }

    {
        {
            rope16 r(u"e");
            auto const it = r.insert(r.begin(), u"f");
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());
        }
        {
            rope16 r(u"e");
            auto const it = r.insert(r.end(), u"f");
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, std::next(r.begin()));
        }

        char16_t const * combining_diaeresis = u"\u0308";

        {
            rope16 r(u"e");
            auto const it = r.insert(r.begin(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());
        }
        {
            rope16 r(u"e");

            auto it = r.insert(r.end(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 1u);
            EXPECT_EQ(it, r.begin());

            it = r.insert(r.end(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 1u);
            EXPECT_EQ(it, r.begin());
        }
        {
            rope16 r(u"et");

            auto it = r.insert(std::next(r.begin()), combining_diaeresis);
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());

            it = r.insert(std::next(r.begin()), combining_diaeresis);
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());
        }
    }

    {
        {
            rope16 r;
            r.insert(r.begin(), 'g'); // text::grapheme
            EXPECT_EQ(r, rope16::text(u"g"));
        }
        {
            rope16 r;
            r.insert(r.begin(), text::grapheme());
            EXPECT_EQ(r, rope16::text(u""));
        }

        {
            rope16::text t(u"g");
            rope16 r;
            r.insert(r.end(), *t.begin()); // text::grapheme_ref
            EXPECT_EQ(r, rope16::text(u"g"));
        }
        {
            rope16::text const t(u"g");
            rope16 r;
            r.insert(r.end(), *t.begin());
            EXPECT_EQ(r, rope16::text(u"g"));
        }
        {
            rope16 r_0(u"g");
            rope16 r;
            r.insert(r.end(), *r_0.begin()); // text::grapheme_ref
            EXPECT_EQ(r, rope16::text(u"g"));
        }
        {
            rope16 const r_0(u"g");
            rope16 r;
            r.insert(r.end(), *r_0.begin());
            EXPECT_EQ(r, rope16::text(u"g"));
        }
        {
            rope16 r;
            r.insert(
                r.begin(),
                text::grapheme_ref<rope16::iterator::iterator>());
            EXPECT_EQ(r, rope16::text(u""));
        }
    }
}

#if 0 // Correct, but takes more than a minute in debug builds.
TEST(rope_utf16, test_insert_rope_view)
{
    rope16 rv_rope;
    rope16::string rv_rope_as_string;
    for (int i = 0; i < 8; ++i) {
        std::size_t const at_idx = i % 2 ? 0 : rv_rope.distance();
        auto const at = std::next(rv_rope.begin(), at_idx);
        switch (i % 3) {
        case 0:
            rv_rope.insert(at, rope16::string(u"text"));
            rv_rope_as_string.insert(at_idx, u"text");
            break;
        case 1:
            rv_rope.insert(at, rope16::string_view(u"text_view"));
            rv_rope_as_string.insert(at_idx, u"text_view");
            break;
        }
    }

    rope16 r;
    rope16::string r_as_string;
    rope16::string local_string;
    for (int i = 0, dist = rv_rope.distance(); i < dist; ++i) {
        for (int j = i + 1; j < dist; ++j) {
            auto const first = std::next(rv_rope.begin(), i);
            auto const last = std::next(rv_rope.begin(), j);
            rope16::rope_view const rv(first, last);

            auto const r_at = r.distance() / 2;
            auto const r_as_string_at = r_as_string.size() / 2;
            r_as_string.insert(
                r_as_string.begin() + r_as_string_at,
                rv.begin().base().base(),
                rv.end().base().base());
            r.insert(std::next(r.begin(), r_at), rv);

            local_string.assign(r.begin().base().base(), r.end().base().base());
            EXPECT_EQ(local_string, r_as_string)
                << "i=" << i << " j=" << j << " insert( " << r_at << ", " << rv
                << ")";
        }
    }
}
#endif

TEST(rope_utf16, test_erase)
{
    rope16 const ct(u"string");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            rope16 t = ct;
            rope16::rope_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            rope16 const substr_copy(substr_first, substr_last);
            rope16::rope_view const after(std::next(ct.begin(), j), ct.end());

            rope16 expected(before);
            expected += after;

            t.erase(substr_first, substr_last);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(rope_utf16, test_replace)
{
    rope16 const ct0(u"REP");
    rope16::rope_view const replacement(ct0);

    {
        rope16 t(u"string");
        t.replace(t.begin(), t.end(), replacement);
        EXPECT_EQ(t, u"REP"_t);
    }

    {
        rope16 t(u"string");
        t.replace(t.begin(), t.end(), replacement);
        EXPECT_EQ(t, u"REP"_t);
    }

    {
        rope16 t(u"string");
        rope16::rope_view const new_substr(
            std::next(t.begin(), 2), std::next(t.begin(), 6));
        t.replace(std::next(t.begin(), 0), std::next(t.begin(), 3), new_substr);
        EXPECT_EQ(t, u"ringing"_t);
    }

    {
        rope16 t(u"string");
        rope16::rope_view const new_substr(
            std::next(t.begin(), 0), std::next(t.begin(), 3));
        t.replace(std::next(t.begin(), 3), std::next(t.begin(), 6), new_substr);
        EXPECT_EQ(t, u"strstr"_t);
    }

    rope16 const ct(u"string");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            rope16 t = ct;
            rope16::rope_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            rope16 const substr_copy(substr_first, substr_last);
            rope16::rope_view const after(std::next(ct.begin(), j), ct.end());

            rope16 expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr_first, substr_last, replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }

    rope16::string_view const really_long_replacement(
        u"REPREPREPREPREPREPREPREPREPREP");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            rope16 t = ct;
            rope16::rope_view const before(t.begin(), std::next(t.begin(), i));
            auto const substr_first = std::next(t.begin(), i);
            auto const substr_last = std::next(t.begin(), j);
            rope16 const substr_copy(substr_first, substr_last);
            rope16::rope_view const after(std::next(ct.begin(), j), ct.end());

            rope16 expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr_first, substr_last, really_long_replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(rope_utf16, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first =
        text::utf_32_to_16_iterator<uint32_t const *>(utf32, utf32, utf32 + 4);
    auto const final_cp = text::utf_32_to_16_iterator<uint32_t const *>(
        utf32, utf32 + 3, utf32 + 4);
    auto const last = text::utf_32_to_16_iterator<uint32_t const *>(
        utf32, utf32 + 4, utf32 + 4);

    rope16 const ct_string(u"string");
    rope16 const ct_text(u"text");

    {
        rope16 t = ct_string;
        t.replace(t.begin(), t.end(), final_cp, last);
        EXPECT_EQ(t, u"\U00010302"_t);
    }

    {
        rope16 t = ct_text;
        t.replace(t.begin(), t.end(), final_cp, last);
        EXPECT_EQ(t, u"\U00010302"_t);
    }

    {
        rope16 t = ct_string;
        t.replace(t.begin(), t.end(), first, last);
        EXPECT_EQ(t, u"\u004d\u0430\u4e8c\U00010302"_t);
    }

    for (int j = 0, end = ct_string.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                rope16 t = ct_string;
                rope16::rope_view const before(
                    t.begin(), std::next(t.begin(), i));
                auto const substr_first = std::next(t.begin(), i);
                auto const substr_last = std::next(t.begin(), j);
                rope16::text const substr_copy(substr_first, substr_last);
                rope16::rope_view const after(std::next(t.begin(), j), t.end());

                rope16::text expected_text(before);
                expected_text.insert(expected_text.end(), final_cp, last);
                expected_text.insert(
                    expected_text.end(), after.begin(), after.end());

                rope16 expected_rope(before);
                expected_rope.insert(expected_rope.end(), final_cp, last);
                expected_rope += after;

                EXPECT_EQ(expected_rope, expected_text)
                    << "i=" << i << " j=" << j << " rope=" << expected_rope
                    << " text=" << expected_text;

                t.replace(substr_first, substr_last, final_cp, last);
                EXPECT_EQ(t, expected_text)
                    << "i=" << i << " j=" << j << " erasing '" << substr_copy
                    << "'";
            }

            {
                rope16 t = ct_string;
                rope16::rope_view const before(
                    t.begin(), std::next(t.begin(), i));
                auto const substr_first = std::next(t.begin(), i);
                auto const substr_last = std::next(t.begin(), j);
                rope16::text const substr_copy(substr_first, substr_last);
                rope16::rope_view const after(std::next(t.begin(), j), t.end());

                rope16::text expected_text(before);
                expected_text.insert(expected_text.end(), first, last);
                expected_text.insert(
                    expected_text.end(), after.begin(), after.end());

                rope16 expected_rope(before);
                expected_rope.insert(expected_rope.end(), first, last);
                expected_rope += after;

                EXPECT_EQ(expected_rope, expected_text)
                    << "i=" << i << " j=" << j << " rope=" << expected_rope
                    << " text=" << expected_text;

                t.replace(substr_first, substr_last, first, last);
                EXPECT_EQ(t, expected_text)
                    << "i=" << i << " j=" << j << " erasing '" << substr_copy
                    << "'";
            }
        }
    }
}

TEST(rope_utf16, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (int i = 0; i < 5000; ++i) {
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
        rope16 t(u"string");
        t.replace(t.begin(), t.end(), first, last);
        rope16 const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        rope16 t;
        t.replace(t.begin(), t.end(), first, last);
        rope16 const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

TEST(rope_utf16, normalization)
{
    uint32_t const circumflex_utf32[] = {0x302};       // ◌̂
    uint32_t const a_with_circumflex_utf32[] = {0xe2}; // â

    auto const s_circumflex_utf16 = text::as_utf16(circumflex_utf32);
    rope16::string const s_circumflex(
        s_circumflex_utf16.begin(), s_circumflex_utf16.end());
    auto const s_a_with_circumflex_utf16 =
        text::as_utf16(a_with_circumflex_utf32);
    rope16::string const s_a_with_circumflex(
        s_a_with_circumflex_utf16.begin(), s_a_with_circumflex_utf16.end());

    rope16 const t_circumflex(s_circumflex);
    rope16 const t_a_with_circumflex(s_a_with_circumflex);
    rope16 const t_a_with_circumflex_2(u"a\u0302" /*a◌̂*/);

    EXPECT_EQ(t_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex_2.distance(), 1u);

    EXPECT_EQ(t_circumflex, u"\u0302"_t /*◌̂*/);
    EXPECT_EQ(t_a_with_circumflex, u"\u00e2"_t /*â*/);
    EXPECT_EQ(t_a_with_circumflex_2, u"\u00e2"_t /*â*/);

    // insert()

    {
        rope16 t = u"aa";
        t.insert(std::next(t.begin(), 0), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t = u"aa";
        t.insert(std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2"
                       u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        rope16 t = u"aa";
        t.insert(std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, rope16::text(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 0), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302\u00e2"
                       u"a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2\u0302"
                       u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(std::next(t.begin(), 2), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, rope16::text(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t = u"aa";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t = u"aa";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2"
                       u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        rope16 t = u"aa";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, rope16::text(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302\u00e2"
                       u"a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2\u0302"
                       u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        rope16 t =
            u"\u00e2"
            u"a";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, rope16::text(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    // replace()

    {
        rope16 t = u"aaa";
        t.replace(t.begin(), std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2"
                       u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        rope16 t = u"aaa";
        t.replace(std::next(t.begin(), 2), t.end(), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, rope16::text(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(t.begin(), std::next(t.begin(), 1), u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            u"\u0302" /*◌̂*/);
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2\u0302"
                       u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(std::next(t.begin(), 2), t.end(), u"\u0302" /*◌̂*/);
        EXPECT_EQ(t, rope16::text(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t = u"aaa";
        t.replace(
            t.begin(),
            std::next(t.begin(), 1),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2"
                       u"a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        rope16 t = u"aaa";
        t.replace(
            std::next(t.begin(), 2),
            t.end(),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(t, rope16::text(u"a\u00e2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            t.begin(),
            std::next(t.begin(), 1),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u0302"
                       u"aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 1),
            std::next(t.begin(), 2),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(
            t,
            rope16::text(u"\u00e2\u0302"
                       u"a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        rope16 t =
            u"\u00e2"
            u"aa";
        t.replace(
            std::next(t.begin(), 2),
            t.end(),
            s_circumflex.begin(),
            s_circumflex.end());
        EXPECT_EQ(t, rope16::text(u"\u00e2\u00e2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }
}

TEST(rope_utf16, test_sentinel_api)
{
    {
        char16_t const * chars = u"chars";
        rope16 s(chars, text::null_sentinel);
        EXPECT_EQ(s, rope16::text(chars));
    }
}

TEST(rope_utf16, test_reverse_iterators)
{
    rope16 rope;
    rope16::string_view const sv(u"qwerty");
    rope.insert(rope.begin(), sv);

    {
        auto it = rope.begin();
        auto sv_it = sv.begin();

        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;

        EXPECT_EQ(it, rope.end());
        EXPECT_TRUE(sv_it == sv.end());

        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));

        EXPECT_EQ(it, rope.begin());
        EXPECT_TRUE(sv_it == sv.begin());
    }

    {
        auto it = rope.rbegin();
        auto sv_it = sv.rbegin();

        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        ++it;
        ++sv_it;

        EXPECT_EQ(it, rope.rend());
        EXPECT_EQ(sv_it, sv.rend());

        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));
        --it;
        --sv_it;
        EXPECT_FALSE(it->empty());
        EXPECT_TRUE(
            algorithm::equal(it->begin(), it->end(), sv_it, std::next(sv_it)));

        EXPECT_EQ(it, rope.rbegin());
        EXPECT_EQ(sv_it, sv.rbegin());
    }
}

TEST(rope_utf16, estimated_width)
{
    EXPECT_EQ(text::detail::width_implied_by_cp(0x0), 1);

    EXPECT_EQ(text::detail::width_implied_by_cp(0x1100 - 1), 1);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x1100), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x1100 + 1), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x115f - 1), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x115f), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x115f + 1), 1);

    EXPECT_EQ(text::detail::width_implied_by_cp(0x3040 - 1), 1);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x3040), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0x3040 + 1), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0xa4cf - 1), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0xa4cf), 2);
    EXPECT_EQ(text::detail::width_implied_by_cp(0xa4cf + 1), 1);
}

TEST(rope_utf16, test_formatted_output)
{
    {
        std::ostringstream oss;
        oss << rope16(u"abc") << rope16(u"def");
        EXPECT_EQ(oss.str(), "abcdef");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << rope16(u"abc");
        EXPECT_EQ(oss.str(), "       abc");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*')
            << rope16(u"abc");
        EXPECT_EQ(oss.str(), "abc*******");
    }

    {
        std::ostringstream oss;
        rope16 abc(u"abc");
        rope16 def(u"def");
        oss << rope16::rope_view(abc) << rope16::rope_view(def);
        EXPECT_EQ(oss.str(), "abcdef");
    }

    {
        std::ostringstream oss;
        rope16 abc(u"abc");
        oss << std::setw(10) << rope16::rope_view(abc);
        EXPECT_EQ(oss.str(), "       abc");
    }

    {
        std::ostringstream oss;
        rope16 abc(u"abc");
        oss << std::setw(10) << std::left << std::setfill('*')
            << rope16::rope_view(abc);
        EXPECT_EQ(oss.str(), "abc*******");
    }
}
