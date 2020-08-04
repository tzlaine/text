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


using namespace boost;
using namespace text::literals;

TEST(rope, test_empty)
{
    text::rope t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.storage_bytes(), 0u);
    EXPECT_EQ(t.distance(), 0u);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_bytes(), (std::size_t)PTRDIFF_MAX);

    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);

    t.swap(t);
    EXPECT_TRUE(t == t);

    EXPECT_EQ(t.begin(), begin(t));
    EXPECT_EQ(t.end(), end(t));

    EXPECT_EQ(t.rbegin(), rbegin(t));
    EXPECT_EQ(t.rend(), rend(t));

    t.clear();

    std::cout << "r=\"" << t << "\"\n";

    {
        text::rope t2(std::string{""});
        EXPECT_TRUE(t == t2);
    }
}

TEST(rope, test_non_empty_const_interface)
{
    text::rope t_a("a");
    text::rope t_ab("ab");

    EXPECT_EQ(std::distance(t_a.begin(), t_a.end()), 1);
    EXPECT_EQ(std::distance(t_a.rbegin(), t_a.rend()), 1);

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.storage_bytes(), 1u);
    EXPECT_EQ(t_a.distance(), 1u);

    text::rope_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.storage_bytes(), 2u);
    EXPECT_EQ(t_ab.distance(), 2u);

    text::rope_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_a.max_bytes(), (std::size_t)PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_bytes(), (std::size_t)PTRDIFF_MAX);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);

    EXPECT_FALSE(t_a == "ab"_t);
    EXPECT_TRUE(t_a != "ab"_t);

    EXPECT_FALSE("a"_t == t_ab);
    EXPECT_TRUE("a"_t != t_ab);

    EXPECT_EQ(t_a, "a"_t);

    text::rope const old_t_a(t_a);
    text::rope const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    EXPECT_EQ(t_a.begin(), begin(t_a));
    EXPECT_EQ(t_a.end(), end(t_a));

    EXPECT_EQ(t_a.rbegin(), rbegin(t_a));
    EXPECT_EQ(t_a.rend(), rend(t_a));

    {
        EXPECT_EQ(t_a, "a"_t);
        EXPECT_EQ(t_ab, "ab"_t);
    }
}

TEST(rope, test_ctors)
{
    text::rope t;
    EXPECT_EQ(t, ""_t);
    EXPECT_EQ(""_t, t);

    text::rope t2("A nonemtpy string");
    EXPECT_EQ(t2, "A nonemtpy string"_t);
    EXPECT_EQ("A nonemtpy string"_t, t2);

    text::rope t3(t2);
    EXPECT_EQ(t3, "A nonemtpy string"_t);
    EXPECT_EQ("A nonemtpy string"_t, t3);

    text::rope t4(std::move(t2));
    EXPECT_EQ(t4, "A nonemtpy string"_t);
    EXPECT_EQ("A nonemtpy string"_t, t4);
    EXPECT_EQ(t2, ""_t);
    EXPECT_EQ(""_t, t2);

    std::string const s("An old-school string");
    text::rope t5{std::string(s)};
    EXPECT_EQ(t5, "An old-school string"_t);
    EXPECT_EQ("An old-school string"_t, t5);

    text::string_view const tv("a view ");
    text::rope t6(tv);
    EXPECT_EQ(t6, "a view "_t);
    EXPECT_EQ("a view "_t, t6);

    std::list<char> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text::rope t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, "a list"_t);
    EXPECT_EQ("a list"_t, t8);
}

TEST(rope, test_assignment)
{
    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string"_t);
        EXPECT_EQ(t2, "A nonemtpy string"_t);
    }

    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t2 = t;
        EXPECT_EQ(t, ""_t);
        EXPECT_EQ(t2, ""_t);
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small"_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string"_t);
        EXPECT_EQ(t2, "A nonemtpy string"_t);
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small"_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t2 = t;
        EXPECT_EQ(t, "small"_t);
        EXPECT_EQ(t2, "small"_t);
    }

    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2;
        EXPECT_EQ(t2, ""_t);

        t = t2;
        EXPECT_EQ(t, ""_t);
        EXPECT_EQ(t2, ""_t);
    }

    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2;
        EXPECT_EQ(t2, ""_t);

        t2 = t;
        EXPECT_EQ(t, ""_t);
        EXPECT_EQ(t2, ""_t);
    }

    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string"_t);
        EXPECT_EQ(t2, ""_t);
    }

    {
        text::rope t;
        EXPECT_EQ(t, ""_t);
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string"_t);

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string"_t);
        EXPECT_EQ(t2, ""_t);
    }

    {
        std::string const s("An old-school string");
        text::rope t;
        t = std::string(s);
        EXPECT_EQ(t, "An old-school string"_t);
    }

    {
        text::string_view const tv("a view ");
        text::rope t;
        t = tv;
        EXPECT_EQ(t, "a view "_t);
    }
}

TEST(rope, test_iterators_and_index)
{
    text::rope empty;

    {
        int dist = 0;
        for (auto gr : empty) {
            (void)gr;
            ++dist;
        }
        EXPECT_EQ(dist, 0);
    }

    {
        text::rope::iterator first = empty.begin();
        text::rope::iterator last = empty.end();
        int dist = 0;
        while (first != last) {
            ++dist;
            ++first;
        }
        EXPECT_EQ(dist, 0);
    }

    {
        text::rope::reverse_iterator first = empty.rbegin();
        text::rope::reverse_iterator last = empty.rend();
        int dist = 0;
        while (first != last) {
            ++dist;
            ++first;
        }
        EXPECT_EQ(dist, 0);
    }

    text::rope non_empty("non-empty");

    {
        std::vector<char> vec;

        text::rope::reverse_iterator const r_it_begin = non_empty.rbegin();
        text::rope::reverse_iterator const r_it_end = non_empty.rend();

        text::rope::reverse_iterator r_it = r_it_begin;
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

TEST(rope, test_misc)
{
    {
        text::rope t("some text");
        t.clear();
        EXPECT_EQ(t.storage_bytes(), 0u);
        EXPECT_EQ(t.distance(), 0u);
    }

    {
        text::rope t1("some");
        text::rope t2("text");
        t1.swap(t2);
        EXPECT_EQ(t1, "text"_t);
        EXPECT_EQ(t2, "some"_t);
    }
}

TEST(rope, test_substr)
{
    text::rope const r = text::rope("When writing a specialization, ") +
                         std::string("be careful about its location; ") +
                         text::string_view(
                             "or to make it compile will be such a trial as to "
                             "kindle its self-immolation") +
                         text::string_view("...");

    EXPECT_EQ(
        text::rope(std::prev(r.end(), 4), std::prev(r.end(), 1)), "n.."_t);

    for (int i = 0, i_end = r.distance(); i < i_end; ++i) {
        for (int j = i, j_end = r.distance(); j < j_end; ++j) {
            auto const first = std::next(r.begin(), i);
            auto const last = std::next(r.begin(), j);
            text::rope const substr(first, last);
            text::rope_view const rv(first, last);
            EXPECT_EQ(substr, rv);
        }
    }
}

TEST(rope, test_insert)
{
    text::text const t("a view ");
    text::text_view const tv(t);

    {
        text::rope const ct("string");

        text::rope t0 = ct;
        t0.insert(std::next(t0.begin(), 0), tv);
        EXPECT_EQ(t0, "a view string"_t);

        text::rope t1 = ct;
        t1.insert(std::next(t1.begin(), 1), tv);
        EXPECT_EQ(t1, "sa view tring"_t);

        text::rope t2 = ct;
        t2.insert(std::next(t2.begin(), 2), tv);
        EXPECT_EQ(t2, "sta view ring"_t);

        text::rope t3 = ct;
        t3.insert(std::next(t3.begin(), 3), tv);
        EXPECT_EQ(t3, "stra view ing"_t);

        text::rope t4 = ct;
        t4.insert(std::next(t4.begin(), 4), tv);
        EXPECT_EQ(t4, "stria view ng"_t);

        text::rope t5 = ct;
        t5.insert(std::next(t5.begin(), 5), tv);
        EXPECT_EQ(t5, "strina view g"_t);

        text::rope t6 = ct;
        t6.insert(std::next(t6.begin(), 6), tv);
        EXPECT_EQ(t6, "stringa view "_t);
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text::rope const ct("string");
        auto const first = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        auto const last = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        text::rope t0 = ct;
        t0.insert(std::next(t0.begin(), 0), first, last);
        EXPECT_EQ(t0, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82string"_t);

        text::rope t1 = ct;
        t1.insert(std::next(t1.begin(), 1), first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring"_t);

        text::rope t2 = ct;
        t2.insert(std::next(t2.begin(), 2), first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring"_t);

        text::rope t3 = ct;
        t3.insert(std::next(t3.begin(), 3), first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing"_t);

        text::rope t4 = ct;
        t4.insert(std::next(t4.begin(), 4), first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng"_t);

        text::rope t5 = ct;
        t5.insert(std::next(t5.begin(), 5), first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g"_t);

        text::rope t6 = ct;
        t6.insert(std::next(t6.begin(), 6), first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82"_t);
    }

    {
        char const * str = "";
        text::string_view const sv(str, 1); // explicitly null-terminated

        {
            text::rope t("text");
            t.insert(std::next(t.begin(), 2), sv);
            EXPECT_EQ(t, "text"_t); // no null in the middle
        }
    }

    {
        {
            text::rope r("e");
            auto const it = r.insert(r.begin(), "f");
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());
        }
        {
            text::rope r("e");
            auto const it = r.insert(r.end(), "f");
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, std::next(r.begin()));
        }

        char const * combining_diaeresis = (char const *)u8"\u0308";

        {
            text::rope r("e");
            auto const it = r.insert(r.begin(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 2u);
            EXPECT_EQ(it, r.begin());
        }
        {
            text::rope r("e");

            auto it = r.insert(r.end(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 1u);
            EXPECT_EQ(it, r.begin());

            it = r.insert(r.end(), combining_diaeresis);
            EXPECT_EQ(r.distance(), 1u);
            EXPECT_EQ(it, r.begin());
        }
        {
            text::rope r("et");

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
            text::rope r;
            r.insert(r.begin(), 'g'); // text::grapheme
            EXPECT_EQ(r, text::text("g"));
        }
        {
            text::rope r;
            r.insert(r.begin(), text::grapheme());
            EXPECT_EQ(r, text::text(""));
        }

        {
            text::text t("g");
            text::rope r;
            r.insert(r.end(), *t.begin()); // text::grapheme_ref
            EXPECT_EQ(r, text::text("g"));
        }
        {
            text::text const t("g");
            text::rope r;
            r.insert(r.end(), *t.begin());
            EXPECT_EQ(r, text::text("g"));
        }
        {
            text::rope r_0("g");
            text::rope r;
            r.insert(r.end(), *r_0.begin()); // text::grapheme_ref
            EXPECT_EQ(r, text::text("g"));
        }
        {
            text::rope const r_0("g");
            text::rope r;
            r.insert(r.end(), *r_0.begin());
            EXPECT_EQ(r, text::text("g"));
        }
        {
            text::rope r;
            r.insert(
                r.begin(),
                text::grapheme_ref<text::rope::iterator::iterator_type>());
            EXPECT_EQ(r, text::text(""));
        }
    }
}

#if 0 // Correct, but takes more than a minute in debug builds.
TEST(rope, test_insert_rope_view)
{
    text::rope rv_rope;
    std::string rv_rope_as_string;
    for (int i = 0; i < 8; ++i) {
        std::size_t const at_idx = i % 2 ? 0 : rv_rope.distance();
        auto const at = std::next(rv_rope.begin(), at_idx);
        switch (i % 3) {
        case 0:
            rv_rope.insert(at, std::string("text"));
            rv_rope_as_string.insert(at_idx, "text");
            break;
        case 1:
            rv_rope.insert(at, text::string_view("text_view"));
            rv_rope_as_string.insert(at_idx, "text_view");
            break;
        }
    }

    text::rope r;
    std::string r_as_string;
    std::string local_string;
    for (int i = 0, dist = rv_rope.distance(); i < dist; ++i) {
        for (int j = i + 1; j < dist; ++j) {
            auto const first = std::next(rv_rope.begin(), i);
            auto const last = std::next(rv_rope.begin(), j);
            text::rope_view const rv(first, last);

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

TEST(rope, test_erase)
{
    text::rope const ct("string");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before(t.begin(), std::next(t.begin(), i));
            text::rope_view const substr(
                std::next(t.begin(), i), std::next(t.begin(), j));
            text::rope const substr_copy(substr);
            text::rope_view const after(std::next(ct.begin(), j), ct.end());

            text::rope expected(before);
            expected += after;

            t.erase(substr);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(rope, test_replace)
{
    text::rope const ct0("REP");
    text::rope_view const replacement(ct0);
    // Explicitly null-terminated.
    char const * rep = "REP";
    text::string_view const replacement_with_null(rep, 4);

    {
        text::rope t("string");
        t.replace(t, replacement_with_null);
        EXPECT_EQ(t, "REP"_t);
    }

    {
        text::rope t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP"_t);
    }

    {
        text::rope t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP"_t);
    }

    {
        text::rope t("string");
        text::rope_view const old_substr(
            std::next(t.begin(), 0), std::next(t.begin(), 3));
        text::rope_view const new_substr(
            std::next(t.begin(), 2), std::next(t.begin(), 6));
        t.replace(old_substr, new_substr);
        EXPECT_EQ(t, "ringing"_t);
    }

    {
        text::rope t("string");
        text::rope_view const old_substr(
            std::next(t.begin(), 3), std::next(t.begin(), 6));
        text::rope_view const new_substr(
            std::next(t.begin(), 0), std::next(t.begin(), 3));
        t.replace(old_substr, new_substr);
        EXPECT_EQ(t, "strstr"_t);
    }

    text::rope const ct("string");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before(t.begin(), std::next(t.begin(), i));
            text::rope_view const substr(
                std::next(t.begin(), i), std::next(t.begin(), j));
            text::rope const substr_copy(substr);
            text::rope_view const after(std::next(ct.begin(), j), ct.end());

            text::rope expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr, replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }

    text::string_view const really_long_replacement(
        "REPREPREPREPREPREPREPREPREPREP");

    for (int j = 0, end = ct.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before(t.begin(), std::next(t.begin(), i));
            text::rope_view const substr(
                std::next(t.begin(), i), std::next(t.begin(), j));
            text::rope const substr_copy(substr);
            text::rope_view const after(std::next(ct.begin(), j), ct.end());

            text::rope expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr, really_long_replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '"
                                   << substr_copy << "'";
        }
    }
}

TEST(rope, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first =
        text::utf_32_to_8_iterator<uint32_t const *>(utf32, utf32, utf32 + 4);
    auto const final_cp = text::utf_32_to_8_iterator<uint32_t const *>(
        utf32, utf32 + 3, utf32 + 4);
    auto const last = text::utf_32_to_8_iterator<uint32_t const *>(
        utf32, utf32 + 4, utf32 + 4);

    text::rope const ct_string("string");
    text::rope const ct_text("text");

    {
        text::rope t = ct_string;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82"_t);
    }

    {
        text::rope t = ct_text;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82"_t);
    }

    {
        text::rope t = ct_string;
        t.replace(t, first, last);
        EXPECT_EQ(t, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82"_t);
    }

    for (int j = 0, end = ct_string.distance(); j <= end; ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                text::rope t = ct_string;
                text::rope_view const before(
                    t.begin(), std::next(t.begin(), i));
                text::rope_view const substr(
                    std::next(t.begin(), i), std::next(t.begin(), j));
                text::text const substr_copy(substr);
                text::rope_view const after(std::next(t.begin(), j), t.end());

                text::text expected_text(before);
                expected_text.insert(expected_text.end(), final_cp, last);
                expected_text += after;

                text::rope expected_rope(before);
                expected_rope.insert(expected_rope.end(), final_cp, last);
                expected_rope += after;

                EXPECT_EQ(expected_rope, expected_text)
                    << "i=" << i << " j=" << j << " rope=" << expected_rope
                    << " text=" << expected_text;

                t.replace(substr, final_cp, last);
                EXPECT_EQ(t, expected_text)
                    << "i=" << i << " j=" << j << " erasing '" << substr_copy
                    << "'";
            }

            {
                text::rope t = ct_string;
                text::rope_view const before(
                    t.begin(), std::next(t.begin(), i));
                text::rope_view const substr(
                    std::next(t.begin(), i), std::next(t.begin(), j));
                text::text const substr_copy(substr);
                text::rope_view const after(std::next(t.begin(), j), t.end());

                text::text expected_text(before);
                expected_text.insert(expected_text.end(), first, last);
                expected_text += after;

                text::rope expected_rope(before);
                expected_rope.insert(expected_rope.end(), first, last);
                expected_rope += after;

                EXPECT_EQ(expected_rope, expected_text)
                    << "i=" << i << " j=" << j << " rope=" << expected_rope
                    << " text=" << expected_text;

                t.replace(substr, first, last);
                EXPECT_EQ(t, expected_text)
                    << "i=" << i << " j=" << j << " erasing '" << substr_copy
                    << "'";
            }
        }
    }
}

TEST(rope, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (int i = 0; i < 5000; ++i) {
        utf32_repeated.insert(utf32_repeated.end(), utf32, utf32 + 4);
    }
    auto const first =
        text::utf_32_to_8_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin(),
            utf32_repeated.begin(),
            utf32_repeated.end());
    auto const last =
        text::utf_32_to_8_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin(), utf32_repeated.end(), utf32_repeated.end());

    {
        text::rope t("string");
        t.replace(t, first, last);
        text::rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text::rope t;
        t.replace(t, first, last);
        text::rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

TEST(rope, normalization)
{
    uint32_t const circumflex_utf32[] = {0x302};       // ◌̂
    uint32_t const a_with_circumflex_utf32[] = {0xe2}; // â

    std::string const s_circumflex =
        text::to_string(circumflex_utf32, circumflex_utf32 + 1);
    std::string const s_a_with_circumflex =
        text::to_string(a_with_circumflex_utf32, a_with_circumflex_utf32 + 1);

    text::rope const t_circumflex(s_circumflex);
    text::rope const t_a_with_circumflex(s_a_with_circumflex);
    text::rope const t_a_with_circumflex_2("a\xcc\x82" /*a◌̂*/);

    EXPECT_EQ(t_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex.distance(), 1u);
    EXPECT_EQ(t_a_with_circumflex_2.distance(), 1u);

    EXPECT_EQ(t_circumflex, "\xcc\x82"_t /*◌̂*/);
    EXPECT_EQ(t_a_with_circumflex, "\xc3\xa2"_t /*â*/);
    EXPECT_EQ(t_a_with_circumflex_2, "\xc3\xa2"_t /*â*/);

    // insert()

    {
        text::rope t = "aa";
        t.insert(std::next(t.begin(), 0), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t = "aa";
        t.insert(std::next(t.begin(), 1), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2"
                       "a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text::rope t = "aa";
        t.insert(std::next(t.begin(), 2), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(t, text::text("a\xc3\xa2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(std::next(t.begin(), 0), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xcc\x82\xc3\xa2"
                       "a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(std::next(t.begin(), 1), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2\xcc\x82"
                       "a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(std::next(t.begin(), 2), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(t, text::text("\xc3\xa2\xc3\xa2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t = "aa";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t = "aa";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2"
                       "a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text::rope t = "aa";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text::text("a\xc3\xa2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(
            std::next(t.begin(), 0), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xcc\x82\xc3\xa2"
                       "a") /*◌̂âa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(
            std::next(t.begin(), 1), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2\xcc\x82"
                       "a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text::rope t =
            "\xc3\xa2"
            "a";
        t.insert(
            std::next(t.begin(), 2), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text::text("\xc3\xa2\xc3\xa2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    // replace()

    auto first = [](text::rope & t) {
        return text::rope_view(t.begin(), std::next(t.begin(), 1));
    };
    auto second = [](text::rope & t) {
        return text::rope_view(
            std::next(t.begin(), 1), std::next(t.begin(), 2));
    };
    auto third = [](text::rope & t) {
        return text::rope_view(std::next(t.begin(), 2), t.end());
    };

    {
        text::rope t = "aaa";
        t.replace(first(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t = "aaa";
        t.replace(second(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2"
                       "a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text::rope t = "aaa";
        t.replace(third(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(t, text::text("a\xc3\xa2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(first(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(second(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2\xcc\x82"
                       "a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(third(t), "\xcc\x82" /*◌̂*/);
        EXPECT_EQ(t, text::text("\xc3\xa2\xc3\xa2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t = "aaa";
        t.replace(first(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t = "aaa";
        t.replace(second(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2"
                       "a") /*âa*/);
        EXPECT_EQ(t.distance(), 2u);
    }
    {
        text::rope t = "aaa";
        t.replace(third(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text::text("a\xc3\xa2") /*aâ*/);
        EXPECT_EQ(t.distance(), 2u);
    }

    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(first(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xcc\x82"
                       "aa") /*◌̂aa*/);
        EXPECT_EQ(t.distance(), 3u);
    }
    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(second(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(
            t,
            text::text("\xc3\xa2\xcc\x82"
                       "a") /*â◌̂a*/);
        EXPECT_EQ(t.distance(), 2u); // not 3 because â◌̂ is a single grapheme
    }
    {
        text::rope t =
            "\xc3\xa2"
            "aa";
        t.replace(third(t), s_circumflex.begin(), s_circumflex.end());
        EXPECT_EQ(t, text::text("\xc3\xa2\xc3\xa2") /*ââ*/);
        EXPECT_EQ(t.distance(), 2u);
    }
}

TEST(rope, test_sentinel_api)
{
    {
        char const * chars = "chars";
        text::rope s(chars, text::null_sentinel{});
        EXPECT_EQ(s, text::text(chars));
    }
    {
        char const * chars = "chars";
        text::rope s;
        s.insert(s.end(), chars, text::null_sentinel{});
        EXPECT_EQ(s, text::text(chars));
    }
    {
        char const * chars = "chars";
        text::rope s;
        s.replace(
            text::rope_view(s.begin(), s.begin()),
            chars,
            text::null_sentinel{});
        EXPECT_EQ(s, text::text(chars));
    }
}

TEST(rope, test_reverse_iterators)
{
    text::rope rope;
    text::string_view const sv("qwerty");
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
        EXPECT_EQ(sv_it, sv.end());

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
        EXPECT_EQ(sv_it, sv.begin());
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

TEST(rope, estimated_width)
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
