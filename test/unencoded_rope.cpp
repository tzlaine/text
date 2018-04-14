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
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(t.compare(t), 0);
    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);
    EXPECT_FALSE(t < t);
    EXPECT_TRUE(t <= t);
    EXPECT_FALSE(t > t);
    EXPECT_TRUE(t >= t);

    t.swap(t);
    EXPECT_TRUE(t == t);

    EXPECT_EQ(t.begin(), begin(t));
    EXPECT_EQ(t.end(), end(t));

    EXPECT_EQ(t.rbegin(), rbegin(t));
    EXPECT_EQ(t.rend(), rend(t));

    t.clear();

    std::cout << "t=\"" << t << "\"\n";

    {
        using namespace text::literals;
        text::unencoded_rope t2(""_s);
        EXPECT_TRUE(t == t2);

        text::unencoded_rope t3(u8""_s);
        EXPECT_TRUE(t == t3);
    }
}

TEST(unencoded_rope, test_non_empty_const_interface)
{
    text::unencoded_rope t_a("a");
    text::unencoded_rope t_ab("ab");

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1);

    text::unencoded_rope_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_EQ(t_a, t_a(0, 1));
    EXPECT_EQ(t_a, t_a(1));
    EXPECT_EQ(t_a, t_a(-1));

    EXPECT_EQ("", t_a(1, 1));
    EXPECT_EQ("", t_a(-1, -1));

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);

    text::unencoded_rope_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_ab, t_ab(0, 2));
    EXPECT_EQ(t_ab, t_ab(2));
    EXPECT_EQ(t_ab, t_ab(-2));

    EXPECT_EQ(t_ab[1], 'b');

    EXPECT_EQ(t_a.max_size(), PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(t_a.compare(t_ab), -1);
    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);
    EXPECT_TRUE(t_a < t_ab);
    EXPECT_TRUE(t_a <= t_ab);
    EXPECT_FALSE(t_a > t_ab);
    EXPECT_FALSE(t_a >= t_ab);

    EXPECT_FALSE(t_a == t_ab(2));
    EXPECT_TRUE(t_a != t_ab(2));
    EXPECT_TRUE(t_a < t_ab(2));
    EXPECT_TRUE(t_a <= t_ab(2));
    EXPECT_FALSE(t_a > t_ab(2));
    EXPECT_FALSE(t_a >= t_ab(2));

    EXPECT_FALSE(t_a(1) == t_ab);
    EXPECT_TRUE(t_a(1) != t_ab);
    EXPECT_TRUE(t_a(1) < t_ab);
    EXPECT_TRUE(t_a(1) <= t_ab);
    EXPECT_FALSE(t_a(1) > t_ab);
    EXPECT_FALSE(t_a(1) >= t_ab);

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

    EXPECT_EQ(t_a.compare(text::unencoded_rope("ab")), -1);
    EXPECT_EQ(t_a, "a");

    text::unencoded_rope const old_t_a(t_a);
    text::unencoded_rope const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    EXPECT_EQ(t_a.begin(), begin(t_a));
    EXPECT_EQ(t_a.end(), end(t_a));

    EXPECT_EQ(t_a.rbegin(), rbegin(t_a));
    EXPECT_EQ(t_a.rend(), rend(t_a));

    {
        using namespace text::literals;
        EXPECT_EQ(t_a, "a"_s);
        EXPECT_EQ(t_ab, "ab"_s);
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
    text::unencoded_rope t5{text::string(s)};
    EXPECT_EQ(t5, "An old-school string");
    EXPECT_EQ("An old-school string", t5);

    text::string_view const tv("a view ");
    text::unencoded_rope t6(tv);
    EXPECT_EQ(t6, "a view ");
    EXPECT_EQ("a view ", t6);

    text::repeated_string_view const rtv(tv, 3);
    text::unencoded_rope t7(rtv);
    EXPECT_EQ(t7, "a view a view a view ");
    EXPECT_EQ("a view a view a view ", t7);

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
        t = text::string(s);
        EXPECT_EQ(t, "An old-school string");
    }

    {
        text::string_view const tv("a view ");
        text::unencoded_rope t;
        t = tv;
        EXPECT_EQ(t, "a view ");

        text::repeated_string_view const rtv(tv, 3);
        text::unencoded_rope t2;
        t2 = rtv;
        EXPECT_EQ(t2, "a view a view a view ");
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
        EXPECT_EQ(t.size(), 0);
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
        text::string("be careful about its location; ") +
        text::string_view(
            "or to make it compile will be such a trial as to "
            "kindle its self-immolation") +
        text::repeated_string_view(".", 3);

    EXPECT_EQ(r.substr(-4, -1), "n..");

    for (int i = 0; i < r.size(); ++i) {
        for (int j = i; j < r.size(); ++j) {
            text::unencoded_rope const substr = r.substr(i, j);
            text::unencoded_rope_view const rv = r(i, j);
            EXPECT_EQ(substr, rv);
        }
    }

    for (int i = 0; i < r.size(); ++i) {
        {
            text::unencoded_rope const substr = r.substr(i);
            text::unencoded_rope_view const rv = r(i);
            EXPECT_EQ(substr, rv);
        }
        {
            text::unencoded_rope const substr = r.substr(-i - 1);
            text::unencoded_rope_view const rv = r(-i - 1);
            EXPECT_EQ(substr, rv);
        }
    }
}

TEST(unencoded_rope, test_insert)
{
    text::string_view const tv("a view ");
    text::repeated_string_view const rtv(tv, 3);

    {
        text::unencoded_rope const ct("string");

        text::unencoded_rope t0 = ct;
        t0.insert(0, tv);
        EXPECT_EQ(t0, "a view string");

        text::unencoded_rope t1 = ct;
        t1.insert(1, tv);
        EXPECT_EQ(t1, "sa view tring");

        text::unencoded_rope t2 = ct;
        t2.insert(2, tv);
        EXPECT_EQ(t2, "sta view ring");

        text::unencoded_rope t3 = ct;
        t3.insert(3, tv);
        EXPECT_EQ(t3, "stra view ing");

        text::unencoded_rope t4 = ct;
        t4.insert(4, tv);
        EXPECT_EQ(t4, "stria view ng");

        text::unencoded_rope t5 = ct;
        t5.insert(5, tv);
        EXPECT_EQ(t5, "strina view g");

        text::unencoded_rope t6 = ct;
        t6.insert(6, tv);
        EXPECT_EQ(t6, "stringa view ");

        text::unencoded_rope t7 = ct;
        t7.insert(6, t7(0, 3));
        EXPECT_EQ(t7, "stringstr");

        text::unencoded_rope t8 = ct;
        t8.insert(2, t8(0, 3));
        EXPECT_EQ(t8, "ststrring");

        text::unencoded_rope t9 = ct;
        t9.insert(6, t9(3, 6));
        EXPECT_EQ(t9, "stringing");
    }

    {
        text::unencoded_rope const ct("string");

        text::unencoded_rope t0 = ct;
        t0.insert(0, rtv);
        EXPECT_EQ(t0, "a view a view a view string");

        text::unencoded_rope t1 = ct;
        t1.insert(1, rtv);
        EXPECT_EQ(t1, "sa view a view a view tring");

        text::unencoded_rope t2 = ct;
        t2.insert(2, rtv);
        EXPECT_EQ(t2, "sta view a view a view ring");

        text::unencoded_rope t3 = ct;
        t3.insert(3, rtv);
        EXPECT_EQ(t3, "stra view a view a view ing");

        text::unencoded_rope t4 = ct;
        t4.insert(4, rtv);
        EXPECT_EQ(t4, "stria view a view a view ng");

        text::unencoded_rope t5 = ct;
        t5.insert(5, rtv);
        EXPECT_EQ(t5, "strina view a view a view g");

        text::unencoded_rope t6 = ct;
        t6.insert(6, rtv);
        EXPECT_EQ(t6, "stringa view a view a view ");

        text::unencoded_rope t7 = ct;
        t7.insert(6, t7(0, 3));
        EXPECT_EQ(t7, "stringstr");

        text::unencoded_rope t8 = ct;
        t8.insert(2, t8(0, 3));
        EXPECT_EQ(t8, "ststrring");

        text::unencoded_rope t9 = ct;
        t9.insert(6, t9(3, 6));
        EXPECT_EQ(t9, "stringing");
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text::unencoded_rope const ct("string");
        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(
            utf32, utf32, utf32 + 4);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(
            utf32, utf32 + 4, utf32 + 4);

        text::unencoded_rope t0 = ct;
        t0.insert(0, first, last);
        EXPECT_EQ(t0, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82string");

        text::unencoded_rope t1 = ct;
        t1.insert(1, first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring");

        text::unencoded_rope t2 = ct;
        t2.insert(2, first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring");

        text::unencoded_rope t3 = ct;
        t3.insert(3, first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing");

        text::unencoded_rope t4 = ct;
        t4.insert(4, first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng");

        text::unencoded_rope t5 = ct;
        t5.insert(5, first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g");

        text::unencoded_rope t6 = ct;
        t6.insert(6, first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }

    {
        char const * str = "";
        text::string_view const tv(str, 1); // explicitly null-terminated
        text::repeated_string_view const rtv(tv, 3);

        {
            text::unencoded_rope t("text");
            t.insert(2, tv);
            EXPECT_EQ(t, "text"); // no null in the middle
        }

        {
            text::unencoded_rope t("text");
            t.insert(2, rtv);
            EXPECT_EQ(t, "text"); // no nulls in the middle
        }
    }
}

TEST(unencoded_rope, test_insert_unencoded_rope_view)
{
    text::unencoded_rope rv_rope;
    std::string rv_rope_as_string;
    for (int i = 0; i < 8; ++i) {
        std::ptrdiff_t const at = i % 2 ? 0 : rv_rope.size();
        switch (i % 3) {
        case 0:
            rv_rope.insert(at, text::string("text"));
            rv_rope_as_string.insert(at, "text");
            break;
        case 1:
            rv_rope.insert(at, text::string_view("text_view"));
            rv_rope_as_string.insert(at, "text_view");
            break;
        case 2:
            rv_rope.insert(at, text::repeated_string_view("rtv", 2));
            rv_rope_as_string.insert(at, "rtvrtv");
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
            r.insert(r_at, rv);

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

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
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
    // Explicitly null-terminated.
    text::string_view const replacement_with_null(
        replacement.begin(), replacement.size() + 1);

    {
        text::unencoded_rope t("string");
        text::unencoded_rope_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement_with_null);
        EXPECT_EQ(t, "REP");
    }

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

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
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

    text::repeated_string_view const really_long_replacement(replacement, 10);

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
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
    auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(
        utf32, utf32, utf32 + 4);
    auto const final_cp = text::utf8::from_utf32_iterator<uint32_t const *>(
        utf32, utf32 + 3, utf32 + 4);
    auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(
        utf32, utf32 + 4, utf32 + 4);

    text::unencoded_rope const ct_string("string");
    text::unencoded_rope const ct_text("text");

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

    for (int j = 0; j <= ct_string.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                text::unencoded_rope t = ct_string;
                text::unencoded_rope_view const before = t(0, i);
                text::unencoded_rope_view const substr = t(i, j);
                text::unencoded_rope_view const after = t(j, t.size());

                text::unencoded_rope expected(before);
                expected.insert(expected.size(), final_cp, last);
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
                expected.insert(expected.size(), first, last);
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
        text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(
            utf32_repeated.begin(),
            utf32_repeated.begin(),
            utf32_repeated.end());
    auto const last =
        text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(
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
        oss << std::setw(10) << text::unencoded_rope("abc");
        EXPECT_EQ(oss.str(), "abc");
    }

    {
        std::ostringstream oss;
        oss << std::setw(10) << std::left << std::setfill('*')
            << text::unencoded_rope("abc");
        EXPECT_EQ(oss.str(), "abc");
    }
}
