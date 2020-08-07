// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/unencoded_rope.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

TEST(unencoded_rope, test_empty)
{
    text::unencoded_rope t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0u);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_size(), (std::size_t)PTRDIFF_MAX);

    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);
    EXPECT_FALSE(t < t);
    EXPECT_TRUE(t <= t);
    EXPECT_FALSE(t > t);
    EXPECT_TRUE(t >= t);

    t.swap(t);
    EXPECT_TRUE(t == t);

    t.clear();

    std::cout << "t=\"" << t << "\"\n";

    {
        text::unencoded_rope t2{std::string()};
        EXPECT_TRUE(t2 == text::unencoded_rope{});
    }
}

TEST(unencoded_rope, test_non_empty_const_interface)
{
    text::unencoded_rope t_a("a");
    text::unencoded_rope t_ab("ab");

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1u);

    text::unencoded_rope_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_EQ(t_a, t_a(0, 1));
    EXPECT_EQ(t_a, t_a(-1, t_a.size()));

    EXPECT_EQ("", t_a(1, 1));
    EXPECT_EQ("", t_a(-1, -1));

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2u);

    text::unencoded_rope_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_ab, t_ab(0, 2));
    EXPECT_EQ(t_ab, t_ab(-2, t_ab.size()));

    EXPECT_EQ(t_ab[1], 'b');

    EXPECT_EQ(t_a.max_size(), (std::size_t)PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_size(), (std::size_t)PTRDIFF_MAX);

    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);
    EXPECT_TRUE(t_a < t_ab);
    EXPECT_TRUE(t_a <= t_ab);
    EXPECT_FALSE(t_a > t_ab);
    EXPECT_FALSE(t_a >= t_ab);

    EXPECT_FALSE(t_a == t_ab(0, 2));
    EXPECT_TRUE(t_a != t_ab(0, 2));
    EXPECT_TRUE(t_a < t_ab(0, 2));
    EXPECT_TRUE(t_a <= t_ab(0, 2));
    EXPECT_FALSE(t_a > t_ab(0, 2));
    EXPECT_FALSE(t_a >= t_ab(0, 2));

    EXPECT_FALSE(t_a(0, 1) == t_ab);
    EXPECT_TRUE(t_a(0, 1) != t_ab);
    EXPECT_TRUE(t_a(0, 1) < t_ab);
    EXPECT_TRUE(t_a(0, 1) <= t_ab);
    EXPECT_FALSE(t_a(0, 1) > t_ab);
    EXPECT_FALSE(t_a(0, 1) >= t_ab);

    EXPECT_FALSE(t_a == "ab");
    EXPECT_TRUE(t_a != "ab");
    EXPECT_TRUE(t_a < "ab");
    EXPECT_TRUE(t_a <= "ab");
    EXPECT_FALSE(t_a > "ab");
    EXPECT_FALSE(t_a >= "ab");

    EXPECT_FALSE("a" == t_ab);
    EXPECT_TRUE("a" != t_ab);
    EXPECT_TRUE("a" < t_ab);
    EXPECT_TRUE("a" <= t_ab);
    EXPECT_FALSE("a" > t_ab);
    EXPECT_FALSE("a" >= t_ab);

    EXPECT_EQ(t_a, "a");

    text::unencoded_rope const old_t_a(t_a);
    text::unencoded_rope const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    {
        EXPECT_EQ(t_a, std::string("a"));
        EXPECT_EQ(t_ab, std::string("ab"));
    }
}

TEST(unencoded_rope, test_ctors)
{
    text::unencoded_rope t;
    EXPECT_EQ(t, "");
    EXPECT_EQ("", t);

    text::unencoded_rope t2("A nonemtpy string");
    EXPECT_EQ(t2, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t2);

    text::unencoded_rope t3(t2);
    EXPECT_EQ(t3, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t3);

    text::unencoded_rope t4(std::move(t2));
    EXPECT_EQ(t4, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t4);
    EXPECT_EQ(t2, "");
    EXPECT_EQ("", t2);

    std::string const s("An old-school string");
    text::unencoded_rope t5{std::string(s)};
    EXPECT_EQ(t5, "An old-school string");
    EXPECT_EQ("An old-school string", t5);

    text::string_view const tv("a view ");
    text::unencoded_rope t6(tv);
    EXPECT_EQ(t6, "a view ");
    EXPECT_EQ("a view ", t6);

    std::list<char> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text::unencoded_rope t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, "a list");
    EXPECT_EQ("a list", t8);
}

TEST(unencoded_rope, test_assignment)
{
    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "small");
        EXPECT_EQ(t2, "small");
    }

    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2;
        EXPECT_EQ(t2, "");

        t = t2;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2;
        EXPECT_EQ(t2, "");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        text::unencoded_rope t;
        EXPECT_EQ(t, "");
        text::unencoded_rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        std::string const s("An old-school string");
        text::unencoded_rope t;
        t = std::string(s);
        EXPECT_EQ(t, "An old-school string");
    }

    {
        text::string_view const tv("a view ");
        text::unencoded_rope t;
        t = tv;
        EXPECT_EQ(t, "a view ");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");

        t = t(0, t.size());
        EXPECT_EQ(t, "small");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");

        t = t(2, t.size());
        EXPECT_EQ(t, "all");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");

        t = t(0, t.size() - 2);
        EXPECT_EQ(t, "sma");
    }

    {
        text::unencoded_rope t("small");
        EXPECT_EQ(t, "small");

        t = t(1, t.size() - 1);
        EXPECT_EQ(t, "mal");
    }
}

TEST(unencoded_rope, test_iterators_and_index)
{
    text::unencoded_rope empty;
    int size = 0;
    for (auto c : empty) {
        (void)c;
        ++size;
    }
    EXPECT_EQ(size, 0);

    {
        text::unencoded_rope::iterator first = empty.begin();
        text::unencoded_rope::iterator last = empty.end();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    {
        text::unencoded_rope::reverse_iterator first = empty.rbegin();
        text::unencoded_rope::reverse_iterator last = empty.rend();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    text::unencoded_rope non_empty("non-empty");

    {
        std::vector<char> vec;

        text::unencoded_rope::reverse_iterator const r_it_begin =
            non_empty.rbegin();
        text::unencoded_rope::reverse_iterator const r_it_end =
            non_empty.rend();

        text::unencoded_rope::reverse_iterator r_it = r_it_begin;
        while (r_it != r_it_end) {
            vec.push_back(*r_it);
            ++r_it;
        }

        std::reverse(vec.begin(), vec.end());
        EXPECT_TRUE(algorithm::equal(
            r_it_end.base(), r_it_begin.base(), vec.begin(), vec.end()));
    }
}

TEST(unencoded_rope, test_misc)
{
    {
        text::unencoded_rope t("some text");
        t.clear();
        EXPECT_EQ(t.size(), 0u);
    }

    {
        text::unencoded_rope t1("some");
        text::unencoded_rope t2("text");
        t1.swap(t2);
        EXPECT_EQ(t1, "text");
        EXPECT_EQ(t2, "some");
    }
}

TEST(unencoded_rope, test_substr)
{
    text::unencoded_rope const r =
        text::unencoded_rope("When writing a specialization, ") +
        std::string("be careful about its location; ") +
        text::string_view(
            "or to make it compile will be such a trial as to "
            "kindle its self-immolation") +
        text::string_view("...");

    EXPECT_EQ(r(-4, -1), "n..");

    for (std::size_t i = 0; i < r.size(); ++i) {
        for (std::size_t j = i; j < r.size(); ++j) {
            text::unencoded_rope const substr(r(i, j));
            text::unencoded_rope_view const rv = r(i, j);
            EXPECT_EQ(substr, rv);
        }
    }

    for (std::size_t i = 0; i < r.size(); ++i) {
        {
            text::unencoded_rope const substr(r(0, i));
            text::unencoded_rope_view const rv = r(0, i);
            EXPECT_EQ(substr, rv);
        }
        {
            text::unencoded_rope const substr(r(-i - 1, r.size()));
            text::unencoded_rope_view const rv = r(-i - 1, r.size());
            EXPECT_EQ(substr, rv);
        }
    }
}

TEST(unencoded_rope, test_insert)
{
    text::string_view const tv("a view ");

    {
        text::unencoded_rope const ct("string");

        text::unencoded_rope t0 = ct;
        t0.insert(t0.begin() + 0, tv);
        EXPECT_EQ(t0, "a view string");

        text::unencoded_rope t1 = ct;
        t1.insert(t1.begin() + 1, tv);
        EXPECT_EQ(t1, "sa view tring");

        text::unencoded_rope t2 = ct;
        t2.insert(t2.begin() + 2, tv);
        EXPECT_EQ(t2, "sta view ring");

        text::unencoded_rope t3 = ct;
        t3.insert(t3.begin() + 3, tv);
        EXPECT_EQ(t3, "stra view ing");

        text::unencoded_rope t4 = ct;
        t4.insert(t4.begin() + 4, tv);
        EXPECT_EQ(t4, "stria view ng");

        text::unencoded_rope t5 = ct;
        t5.insert(t5.begin() + 5, tv);
        EXPECT_EQ(t5, "strina view g");

        text::unencoded_rope t6 = ct;
        t6.insert(t6.begin() + 6, tv);
        EXPECT_EQ(t6, "stringa view ");

        text::unencoded_rope t7 = ct;
        t7.insert(t7.begin() + 6, t7(0, 3));
        EXPECT_EQ(t7, "stringstr");

        text::unencoded_rope t8 = ct;
        t8.insert(t8.begin() + 2, t8(0, 3));
        EXPECT_EQ(t8, "ststrring");

        text::unencoded_rope t9 = ct;
        t9.insert(t9.begin() + 6, t9(3, 6));
        EXPECT_EQ(t9, "stringing");
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text::unencoded_rope const ct("string");
        auto const first = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        auto const last = text::utf_32_to_8_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        text::unencoded_rope t0 = ct;
        t0.insert(t0.begin() + 0, first, last);
        EXPECT_EQ(t0, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82string");

        text::unencoded_rope t1 = ct;
        t1.insert(t1.begin() + 1, first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring");

        text::unencoded_rope t2 = ct;
        t2.insert(t2.begin() + 2, first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring");

        text::unencoded_rope t3 = ct;
        t3.insert(t3.begin() + 3, first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing");

        text::unencoded_rope t4 = ct;
        t4.insert(t4.begin() + 4, first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng");

        text::unencoded_rope t5 = ct;
        t5.insert(t5.begin() + 5, first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g");

        text::unencoded_rope t6 = ct;
        t6.insert(t6.begin() + 6, first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }
}

TEST(unencoded_rope, test_insert_unencoded_rope_view)
{
    text::unencoded_rope rv_rope;
    std::string rv_rope_as_string;
    for (int i = 0; i < 8; ++i) {
        std::size_t const at = i % 2 ? 0 : rv_rope.size();
        switch (i % 3) {
        case 0:
            rv_rope.insert(rv_rope.begin() + at, std::string("text"));
            rv_rope_as_string.insert(at, "text");
            break;
        case 1:
            rv_rope.insert(
                rv_rope.begin() + at, text::string_view("text_view"));
            rv_rope_as_string.insert(at, "text_view");
            break;
        }
    }

    text::unencoded_rope r;
    std::string r_as_string;
    std::string local_string;
    for (int i = 0, size = rv_rope.size(); i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            text::unencoded_rope_view const rv = rv_rope(i, j);

            auto const r_at = r.size() / 2;
            auto const r_as_string_at = r_as_string.size() / 2;
            r_as_string.insert(
                r_as_string.begin() + r_as_string_at, rv.begin(), rv.end());
            r.insert(r.begin() + r_at, rv);

            local_string.assign(r.begin(), r.end());
            EXPECT_EQ(local_string, r_as_string)
                << "i=" << i << " j=" << j << " insert( " << r_at << ", " << rv
                << ")";
        }
    }
}

TEST(unencoded_rope, test_erase)
{
    {
        text::unencoded_rope t("string");
        text::unencoded_rope_view const ctv(t, 0, t.size());
        t.erase(ctv);
        EXPECT_EQ(t, "");
    }

    text::unencoded_rope const ct("string");

    for (std::size_t j = 0; j <= ct.size(); ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text::unencoded_rope t = ct;
            text::unencoded_rope_view const before = t(0, i);
            text::unencoded_rope_view const substr = t(i, j);
            text::unencoded_rope_view const after = t(j, t.size());

            text::unencoded_rope expected(before);
            expected += after;

            t.erase(substr);
            EXPECT_EQ(t, expected)
                << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }
}

TEST(unencoded_rope, test_replace)
{
    text::string_view const replacement("REP");

    {
        text::unencoded_rope t("string");
        text::unencoded_rope_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::unencoded_rope t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::unencoded_rope t("string");
        t.replace(t(0, 3), t(2, 6));
        EXPECT_EQ(t, "ringing");
    }

    {
        text::unencoded_rope t("string");
        t.replace(t(3, 6), t(0, 3));
        EXPECT_EQ(t, "strstr");
    }

    text::unencoded_rope const ct("string");

    for (std::size_t j = 0; j <= ct.size(); ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text::unencoded_rope t = ct;
            text::unencoded_rope_view const before = t(0, i);
            text::unencoded_rope_view const substr = t(i, j);
            text::unencoded_rope_view const after = t(j, t.size());

            text::unencoded_rope expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr, replacement);
            EXPECT_EQ(t, expected)
                << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    text::string_view const really_long_replacement(
        "REPREPREPREPREPREPREPREPREPREP");

    for (std::size_t j = 0; j <= ct.size(); ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            text::unencoded_rope t = ct;
            text::unencoded_rope_view const before = t(0, i);
            text::unencoded_rope_view const substr = t(i, j);
            text::unencoded_rope_view const after = t(j, t.size());

            text::unencoded_rope expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr, really_long_replacement);
            EXPECT_EQ(t, expected)
                << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }
}

TEST(unencoded_rope, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first =
        text::utf_32_to_8_iterator<uint32_t const *>(utf32, utf32, utf32 + 4);
    auto const final_cp = text::utf_32_to_8_iterator<uint32_t const *>(
        utf32, utf32 + 3, utf32 + 4);
    auto const last = text::utf_32_to_8_iterator<uint32_t const *>(
        utf32, utf32 + 4, utf32 + 4);

    text::unencoded_rope const ct_string("string");
    text::unencoded_rope const ct_text("text");

    // These test replacement via pointer or array, not iterator.
    {
        {
            text::unencoded_rope t = ct_string;
            t.replace(t, "\xf0\x90\x8c\x82");
            EXPECT_EQ(t, "\xf0\x90\x8c\x82");
        }

        {
            text::unencoded_rope t = ct_string;
            char const * str = "\xf0\x90\x8c\x82";
            t.replace(t, str);
            EXPECT_EQ(t, "\xf0\x90\x8c\x82");
        }
    }

    {
        text::unencoded_rope t = ct_string;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::unencoded_rope t = ct_text;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::unencoded_rope t = ct_string;
        t.replace(t, first, last);
        EXPECT_EQ(t, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }

    for (std::size_t j = 0; j <= ct_string.size(); ++j) {
        for (std::size_t i = 0; i <= j; ++i) {
            {
                text::unencoded_rope t = ct_string;
                text::unencoded_rope_view const before = t(0, i);
                text::unencoded_rope_view const substr = t(i, j);
                text::unencoded_rope_view const after = t(j, t.size());

                text::unencoded_rope expected(before);
                expected.insert(expected.end(), final_cp, last);
                expected += after;

                t.replace(substr, final_cp, last);
                EXPECT_EQ(t, expected)
                    << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }

            {
                text::unencoded_rope t = ct_string;
                text::unencoded_rope_view const before = t(0, i);
                text::unencoded_rope_view const substr = t(i, j);
                text::unencoded_rope_view const after = t(j, t.size());

                text::unencoded_rope expected(before);
                expected.insert(expected.end(), first, last);
                expected += after;

                t.replace(substr, first, last);
                EXPECT_EQ(t, expected)
                    << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }
        }
    }
}

TEST(unencoded_rope, test_replace_iter_large_insertions)
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
        text::unencoded_rope t("string");
        t.replace(t, first, last);
        text::unencoded_rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text::unencoded_rope t;
        t.replace(t, first, last);
        text::unencoded_rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

TEST(unencoded_rope, test_unformatted_output)
{
    {
        std::ostringstream oss;
        oss << text::unencoded_rope("abc") << text::unencoded_rope("def");
        EXPECT_EQ(oss.str(), "abcdef");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << text::unencoded_rope("abc");
        EXPECT_EQ(oss.str(), "       abc");
    }

#if 0 // TODO: Fix!
    {
        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*')
            << text::unencoded_rope("abc");
        EXPECT_EQ(oss.str(), "abc*******");
    }
#endif
}

TEST(unencoded_rope, test_sentinel_api)
{
    {
        char const * chars = "chars";
        text::unencoded_rope s(chars, text::null_sentinel{});
        EXPECT_EQ(s, chars);
    }
    {
        char const * chars = "chars";
        text::unencoded_rope s;
        s.insert(s.begin(), chars, text::null_sentinel{});
        EXPECT_EQ(s, chars);
    }
    {
        char const * chars = "chars";
        text::unencoded_rope s;
        s.insert(s.end(), chars, text::null_sentinel{});
        EXPECT_EQ(s, chars);
    }
    {
        char const * chars = "chars";
        text::unencoded_rope s;
        s.replace(s(0, 0), chars, text::null_sentinel{});
        EXPECT_EQ(s, chars);
    }
    {
        char const * chars = "chars";
        text::unencoded_rope s;
        s.replace(s.begin(), s.begin(), chars, text::null_sentinel{});
        EXPECT_EQ(s, chars);
    }
}
