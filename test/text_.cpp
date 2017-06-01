#include <boost/text/text.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

TEST(text, test_empty)
{
    text::text t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.cbegin(), t.cend());
    EXPECT_EQ(t.rbegin(), t.rend());
    EXPECT_EQ(t.crbegin(), t.crend());

    EXPECT_EQ(t.begin(), t.cbegin());
    EXPECT_EQ(t.end(), t.cend());
    EXPECT_EQ(t.rbegin(), t.crbegin());
    EXPECT_EQ(t.rend(), t.crend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_size(), INT_MAX);

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
    EXPECT_EQ(t.cbegin(), cbegin(t));
    EXPECT_EQ(t.cend(), cend(t));

    EXPECT_EQ(t.rbegin(), rbegin(t));
    EXPECT_EQ(t.rend(), rend(t));
    EXPECT_EQ(t.crbegin(), crbegin(t));
    EXPECT_EQ(t.crend(), crend(t));

    t.clear();
    t.resize(0, 'c');
    t.shrink_to_fit();

    std::cout << "t=\"" << t << "\"\n";

    {
        using namespace text::literals;
        text::text t2 = ""_t;
        EXPECT_TRUE(t == t2);

        text::text t3 = u8""_t;
        EXPECT_TRUE(t == t3);

        text::text t4 = u""_t;
        EXPECT_TRUE(t == t4);

        text::text t5 = U""_t;
        EXPECT_TRUE(t == t5);
    }
}

TEST(text, test_non_empty_const_interface)
{
    text::text t_a("a");
    text::text t_ab("ab");

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.cbegin() + t_a.size(), t_a.cend());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());
    EXPECT_EQ(t_a.crbegin() + t_a.size(), t_a.crend());

    EXPECT_EQ(t_a.begin(), t_a.cbegin());
    EXPECT_EQ(t_a.end(), t_a.cend());
    EXPECT_EQ(t_a.rbegin(), t_a.crbegin());
    EXPECT_EQ(t_a.rend(), t_a.crend());

    EXPECT_EQ(t_ab.begin(), t_ab.cbegin());
    EXPECT_EQ(t_ab.end(), t_ab.cend());
    EXPECT_EQ(t_ab.rbegin(), t_ab.crbegin());
    EXPECT_EQ(t_ab.rend(), t_ab.crend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1);
    EXPECT_GT(t_a.capacity(), t_a.size());
    EXPECT_NE(t_a.begin(), nullptr);

    text::text_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_EQ(t_a, t_a(0, 1));
    EXPECT_EQ(t_a, t_a(0));

    EXPECT_EQ("", t_a(1, 1));

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);
    EXPECT_GT(t_ab.capacity(), t_ab.size());
    EXPECT_NE(t_ab.begin(), nullptr);

    text::text_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_ab, t_ab(0, 2));
    EXPECT_EQ(t_ab, t_ab(0));

    EXPECT_EQ(t_ab[1], 'b');

    EXPECT_EQ(t_a.max_size(), INT_MAX);
    EXPECT_EQ(t_ab.max_size(), INT_MAX);

    EXPECT_EQ(t_a.compare(t_ab), -1);
    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);
    EXPECT_TRUE(t_a < t_ab);
    EXPECT_TRUE(t_a <= t_ab);
    EXPECT_FALSE(t_a > t_ab);
    EXPECT_FALSE(t_a >= t_ab);

    EXPECT_FALSE(t_a == t_ab(0));
    EXPECT_TRUE(t_a != t_ab(0));
    EXPECT_TRUE(t_a < t_ab(0));
    EXPECT_TRUE(t_a <= t_ab(0));
    EXPECT_FALSE(t_a > t_ab(0));
    EXPECT_FALSE(t_a >= t_ab(0));

    EXPECT_FALSE(t_a(0) == t_ab);
    EXPECT_TRUE(t_a(0) != t_ab);
    EXPECT_TRUE(t_a(0) < t_ab);
    EXPECT_TRUE(t_a(0) <= t_ab);
    EXPECT_FALSE(t_a(0) > t_ab);
    EXPECT_FALSE(t_a(0) >= t_ab);

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

    EXPECT_EQ(t_a.compare("ab"), -1);
    EXPECT_EQ(t_a, "a");

    text::text const old_t_a(t_a);
    text::text const old_t_ab(t_ab);
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

    {
        using namespace text::literals;
        EXPECT_EQ(t_a, "a"_t);
        EXPECT_EQ(t_ab, "ab"_t);
    }
}

TEST(text, test_ctors)
{
    text::text t;
    EXPECT_EQ(t, "");
    EXPECT_EQ("", t);

    text::text t2("A nonemtpy string");
    EXPECT_EQ(t2, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t2);

    text::text t3(t2);
    EXPECT_EQ(t3, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t3);

    text::text t4(std::move(t2));
    EXPECT_EQ(t4, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t4);
    EXPECT_EQ(t2, "");
    EXPECT_EQ("", t2);

    std::string const s("An old-school string");
    text::text t5(s);
    EXPECT_EQ(t5, "An old-school string");
    EXPECT_EQ("An old-school string", t5);

    text::text_view const tv("a view ");
    text::text t6(tv);
    EXPECT_EQ(t6, "a view ");
    EXPECT_EQ("a view ", t6);

    text::repeated_text_view const rtv(tv, 3);
    text::text t7(rtv);
    EXPECT_EQ(t7, "a view a view a view ");
    EXPECT_EQ("a view a view a view ", t7);

    std::list<char> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text::text t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, "a list");
    EXPECT_EQ("a list", t8);
}

TEST(text, test_assignment)
{
    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::text t("small");
        EXPECT_EQ(t, "small");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::text t("small");
        EXPECT_EQ(t, "small");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "small");
        EXPECT_EQ(t2, "small");
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2;
        EXPECT_EQ(t2, "");

        t = t2;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2;
        EXPECT_EQ(t2, "");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = std::move(t);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        std::string const s("An old-school string");
        text::text t;
        t = s;
        EXPECT_EQ(t, "An old-school string");
    }

    {
        text::text_view const tv("a view ");
        text::text t;
        t = tv;
        EXPECT_EQ(t, "a view ");

        text::repeated_text_view const rtv(tv, 3);
        text::text t2;
        t2 = rtv;
        EXPECT_EQ(t2, "a view a view a view ");
    }
}

TEST(text, test_iterators_and_index)
{
    text::text empty;
    int size = 0;
    for (auto c : empty) {
        (void)c;
        ++size;
    }
    EXPECT_EQ(size, 0);

    {
        text::text::iterator first = empty.begin();
        text::text::iterator last = empty.end();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    {
        text::text::const_iterator first = empty.cbegin();
        text::text::const_iterator last = empty.cend();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    EXPECT_EQ(empty.begin(), empty.cbegin());
    EXPECT_EQ(empty.end(), empty.cend());

    {
        text::text::reverse_iterator first = empty.rbegin();
        text::text::reverse_iterator last = empty.rend();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    {
        text::text::const_reverse_iterator first = empty.crbegin();
        text::text::const_reverse_iterator last = empty.crend();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    EXPECT_EQ(empty.rbegin(), empty.crbegin());
    EXPECT_EQ(empty.rend(), empty.crend());

    text::text non_empty("non-empty");

    {
        text::text::iterator it = non_empty.begin();
        text::text::const_iterator c_it = non_empty.cbegin();

        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'n');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'o');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'n');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, '-');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'e');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'm');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'p');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 't');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'y');
        ++it; ++c_it;

        EXPECT_EQ(it, c_it);
        EXPECT_EQ(it, non_empty.end());
        EXPECT_EQ(c_it, non_empty.cend());
    }

    {
        text::text::reverse_iterator it = non_empty.rbegin();
        text::text::const_reverse_iterator c_it = non_empty.crbegin();

        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'y');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 't');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'p');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'm');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'e');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, '-');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'n');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'o');
        ++it; ++c_it;
        EXPECT_EQ(it, c_it);
        EXPECT_EQ(*it, 'n');
        ++it; ++c_it;

        EXPECT_EQ(it, c_it);
        EXPECT_EQ(it, non_empty.rend());
        EXPECT_EQ(c_it, non_empty.crend());
    }


    {
        std::vector<char> vec;

        text::text::reverse_iterator const r_it_begin = non_empty.rbegin();
        text::text::reverse_iterator const r_it_end = non_empty.rend();

        text::text::reverse_iterator r_it = r_it_begin;
        while (r_it != r_it_end) {
            vec.push_back(*r_it);
            ++r_it;
        }

        std::reverse(vec.begin(), vec.end());
        EXPECT_TRUE(std::equal(r_it_end.base(), r_it_begin.base(), vec.begin(), vec.end()));
     }
}

TEST(text, test_misc)
{
    {
        text::text t("some text");
        int const cap = t.capacity();
        t.clear();
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(t.capacity(), cap);
    }

    {
        text::text t("some text");
        int const cap = t.capacity();
        t.resize(0, 'c');
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(t.capacity(), cap);
    }

    {
        text::text t("some text");
        int const cap = t.capacity();
        t.resize(4, 'c');
        EXPECT_EQ(t.size(), 4);
        EXPECT_EQ(t.capacity(), cap);
    }

    {
        text::text t("some text");
        t.resize(12, 'c');
        EXPECT_EQ(t.size(), 12);
        EXPECT_EQ(t, "some textccc");
    }

    {
        text::text t("some text");
        t.reserve(153);
        EXPECT_EQ(t.capacity(), 153);
        EXPECT_EQ(t, "some text");
    }

    {
        text::text t("some text");
        t.reserve(153);
        EXPECT_EQ(t.capacity(), 153);
        EXPECT_EQ(t, "some text");
        t.shrink_to_fit();
        EXPECT_EQ(t.capacity(), 9);
        EXPECT_EQ(t, "some text");
    }

    {
        text::text t1("some");
        text::text t2("text");
        t1.swap(t2);
        EXPECT_EQ(t1, "text");
        EXPECT_EQ(t2, "some");
    }
}


TEST(text, test_insert)
{
    text::text_view const tv("a view ");
    text::repeated_text_view const rtv(tv, 3);

    {
        text::text const ct("string");

        text::text t0 = ct;
        EXPECT_EQ(t0.capacity(), 7);
        t0.insert(0, tv);
        EXPECT_EQ(t0, "a view string");

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, tv);
        EXPECT_EQ(t1, "sa view tring");

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, tv);
        EXPECT_EQ(t2, "sta view ring");

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, tv);
        EXPECT_EQ(t3, "stra view ing");

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, tv);
        EXPECT_EQ(t4, "stria view ng");

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, tv);
        EXPECT_EQ(t5, "strina view g");

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, tv);
        EXPECT_EQ(t6, "stringa view ");
    }

    {
        text::text const ct("string");

        text::text t0 = ct;
        EXPECT_EQ(t0.capacity(), 7);
        t0.insert(0, rtv);
        EXPECT_EQ(t0, "a view a view a view string");

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, rtv);
        EXPECT_EQ(t1, "sa view a view a view tring");

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, rtv);
        EXPECT_EQ(t2, "sta view a view a view ring");

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, rtv);
        EXPECT_EQ(t3, "stra view a view a view ing");

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, rtv);
        EXPECT_EQ(t4, "stria view a view a view ng");

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, rtv);
        EXPECT_EQ(t5, "strina view a view a view g");

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, rtv);
        EXPECT_EQ(t6, "stringa view a view a view ");
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text::text const ct("string");
        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

        text::text t0 = ct;
        EXPECT_EQ(t0.capacity(), 7);
        t0.insert(0, first, last);
        EXPECT_EQ(t0, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82string");

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring");

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring");

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing");

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng");

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g");

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }

    {
        char const * str = "";
        text::text_view const tv(str, 1); // explicitly null-terminated
        text::repeated_text_view const rtv(tv, 3);

        {
            text::text t("text");
            t.insert(2, tv);
            EXPECT_EQ(t, "text"); // no null
        }

        {
            text::text t("text");
            t.insert(2, rtv);
            EXPECT_EQ(t, "text"); // no null
        }
    }

    {
        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::text const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.insert(0, "something"));
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.insert(1, "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.insert(2, "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.insert(3, "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.insert(4, "something"));
        }
    }
}

TEST(text, test_erase)
{
}

TEST(text, test_replace)
{
}
