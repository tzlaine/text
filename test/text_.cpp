#define BOOST_TEXT_TESTING
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
    EXPECT_EQ(t_a, t_a(1));
    EXPECT_EQ(t_a, t_a(-1));

    EXPECT_EQ("", t_a(1, 1));

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);
    EXPECT_GT(t_ab.capacity(), t_ab.size());
    EXPECT_NE(t_ab.begin(), nullptr);

    text::text_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_ab, t_ab(0, 2));
    EXPECT_EQ(t_ab, t_ab(2));
    EXPECT_EQ(t_ab, t_ab(-2));

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
    EXPECT_EQ(t2[t2.size()], '\0');

    text::text t3(t2);
    EXPECT_EQ(t3, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t3);
    EXPECT_EQ(t3[t3.size()], '\0');

    text::text t4(std::move(t2));
    EXPECT_EQ(t4, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t4);
    EXPECT_EQ(t2, "");
    EXPECT_EQ("", t2);
    EXPECT_EQ(t4[t4.size()], '\0');

    std::string const s("An old-school string");
    text::text t5(s);
    EXPECT_EQ(t5, "An old-school string");
    EXPECT_EQ("An old-school string", t5);
    EXPECT_EQ(t5[t5.size()], '\0');

    text::text_view const tv("a view ");
    text::text t6(tv);
    EXPECT_EQ(t6, "a view ");
    EXPECT_EQ("a view ", t6);
    EXPECT_EQ(t6[t6.size()], '\0');

    text::repeated_text_view const rtv(tv, 3);
    text::text t7(rtv);
    EXPECT_EQ(t7, "a view a view a view ");
    EXPECT_EQ("a view a view a view ", t7);
    EXPECT_EQ(t7[t7.size()], '\0');

    std::list<char> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text::text t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, "a list");
    EXPECT_EQ("a list", t8);
    EXPECT_EQ(t8[t8.size()], '\0');
}

TEST(text, test_assignment)
{
    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t[t.size()], '\0');
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t[t2.size()], '\0');
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::text t("small");
        EXPECT_EQ(t, "small");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t[t.size()], '\0');
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');
    }

    {
        text::text t("small");
        EXPECT_EQ(t, "small");
        EXPECT_EQ(t[t.size()], '\0');
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');

        t2 = t;
        EXPECT_EQ(t, "small");
        EXPECT_EQ(t[t.size()], '\0');
        EXPECT_EQ(t2, "small");
        EXPECT_EQ(t2[t2.size()], '\0');
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
        EXPECT_EQ(t2[t2.size()], '\0');

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t;
        EXPECT_EQ(t, "");
        text::text t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
        EXPECT_EQ(t2[t2.size()], '\0');

        t2 = std::move(t);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t[t.size()], '\0');
        EXPECT_EQ(t2, "");
    }

    {
        std::string const s("An old-school string");
        text::text t;
        t = s;
        EXPECT_EQ(t, "An old-school string");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text_view const tv("a view ");
        text::text t;
        t = tv;
        EXPECT_EQ(t, "a view ");
        EXPECT_EQ(t[t.size()], '\0');

        text::repeated_text_view const rtv(tv, 3);
        text::text t2;
        t2 = rtv;
        EXPECT_EQ(t2, "a view a view a view ");
        EXPECT_EQ(t2[t2.size()], '\0');
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
        EXPECT_EQ(t[t.size()], '\0');
        int const cap = t.capacity();
        t.clear();
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(t.capacity(), cap);
    }

    {
        text::text t("some text");
        EXPECT_EQ(t[t.size()], '\0');
        int const cap = t.capacity();
        t.resize(0, 'c');
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(t.capacity(), cap);
    }

    {
        text::text t("some text");
        EXPECT_EQ(t[t.size()], '\0');
        int const cap = t.capacity();
        t.resize(4, 'c');
        EXPECT_EQ(t.size(), 4);
        EXPECT_EQ(t.capacity(), cap);
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t("some text");
        EXPECT_EQ(t[t.size()], '\0');
        t.resize(12, 'c');
        EXPECT_EQ(t.size(), 12);
        EXPECT_EQ(t, "some textccc");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t("some text");
        EXPECT_EQ(t[t.size()], '\0');
        t.reserve(153);
        EXPECT_EQ(t.capacity(), 153);
        EXPECT_EQ(t, "some text");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t("some text");
        EXPECT_EQ(t[t.size()], '\0');
        t.reserve(153);
        EXPECT_EQ(t.capacity(), 153);
        EXPECT_EQ(t, "some text");
        EXPECT_EQ(t[t.size()], '\0');
        t.shrink_to_fit();
        EXPECT_EQ(t.capacity(), 9);
        EXPECT_EQ(t, "some text");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t1("some");
        EXPECT_EQ(t1[t1.size()], '\0');
        text::text t2("text");
        EXPECT_EQ(t2[t2.size()], '\0');
        t1.swap(t2);
        EXPECT_EQ(t1, "text");
        EXPECT_EQ(t1[t1.size()], '\0');
        EXPECT_EQ(t2, "some");
        EXPECT_EQ(t2[t2.size()], '\0');
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
        EXPECT_EQ(t0[t0.size()], '\0');

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, tv);
        EXPECT_EQ(t1, "sa view tring");
        EXPECT_EQ(t1[t1.size()], '\0');

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, tv);
        EXPECT_EQ(t2, "sta view ring");
        EXPECT_EQ(t2[t2.size()], '\0');

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, tv);
        EXPECT_EQ(t3, "stra view ing");
        EXPECT_EQ(t3[t3.size()], '\0');

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, tv);
        EXPECT_EQ(t4, "stria view ng");
        EXPECT_EQ(t4[t4.size()], '\0');

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, tv);
        EXPECT_EQ(t5, "strina view g");
        EXPECT_EQ(t5[t5.size()], '\0');

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, tv);
        EXPECT_EQ(t6, "stringa view ");
        EXPECT_EQ(t6[t6.size()], '\0');
    }

    {
        text::text const ct("string");

        text::text t0 = ct;
        EXPECT_EQ(t0.capacity(), 7);
        t0.insert(0, rtv);
        EXPECT_EQ(t0, "a view a view a view string");
        EXPECT_EQ(t0[t0.size()], '\0');

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, rtv);
        EXPECT_EQ(t1, "sa view a view a view tring");
        EXPECT_EQ(t1[t1.size()], '\0');

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, rtv);
        EXPECT_EQ(t2, "sta view a view a view ring");
        EXPECT_EQ(t2[t2.size()], '\0');

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, rtv);
        EXPECT_EQ(t3, "stra view a view a view ing");
        EXPECT_EQ(t3[t3.size()], '\0');

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, rtv);
        EXPECT_EQ(t4, "stria view a view a view ng");
        EXPECT_EQ(t4[t4.size()], '\0');

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, rtv);
        EXPECT_EQ(t5, "strina view a view a view g");
        EXPECT_EQ(t5[t5.size()], '\0');

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, rtv);
        EXPECT_EQ(t6, "stringa view a view a view ");
        EXPECT_EQ(t6[t6.size()], '\0');
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
        EXPECT_EQ(t0[t0.size()], '\0');

        text::text t1 = ct;
        EXPECT_EQ(t1.capacity(), 7);
        t1.insert(1, first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring");
        EXPECT_EQ(t1[t1.size()], '\0');

        text::text t2 = ct;
        EXPECT_EQ(t2.capacity(), 7);
        t2.insert(2, first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring");
        EXPECT_EQ(t2[t2.size()], '\0');

        text::text t3 = ct;
        EXPECT_EQ(t3.capacity(), 7);
        t3.insert(3, first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing");
        EXPECT_EQ(t3[t3.size()], '\0');

        text::text t4 = ct;
        EXPECT_EQ(t4.capacity(), 7);
        t4.insert(4, first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng");
        EXPECT_EQ(t4[t4.size()], '\0');

        text::text t5 = ct;
        EXPECT_EQ(t5.capacity(), 7);
        t5.insert(5, first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g");
        EXPECT_EQ(t5[t5.size()], '\0');

        text::text t6 = ct;
        EXPECT_EQ(t6.capacity(), 7);
        t6.insert(6, first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
        EXPECT_EQ(t6[t6.size()], '\0');

        // Breaking the encoding is fine with the iterator interface.
        text::text t7(first, last);
        char const * c_str = "a";
        EXPECT_NO_THROW(t7.insert(t7.end() - 2, c_str, c_str + 1));
    }

    {
        char const * str = "";
        text::text_view const tv(str, 1); // explicitly null-terminated
        text::repeated_text_view const rtv(tv, 3);

        {
            text::text t("text");
            t.insert(2, tv);
            EXPECT_EQ(t, "text"); // no null in the middle
            EXPECT_EQ(t[t.size()], '\0');
        }

        {
            text::text t("text");
            t.insert(2, rtv);
            EXPECT_EQ(t, "text"); // no nulls in the middle
            EXPECT_EQ(t[t.size()], '\0');
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
            EXPECT_EQ(t[t.size()], '\0');
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
            EXPECT_EQ(t[t.size()], '\0');
        }

        {
            // Broken encoding in the removed *and* inserted ranges is fine,
            // since the iterator interface is considered unsafe.
            text::text t = ct;
            auto final_cp_plus_one = first;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.insert(4, final_cp_plus_one, last));
        }

        {
            // Broken encoding due to the insertion point *and* inserted
            // ranges is fine, since the iterator interface is considered
            // unsafe.
            text::text t = ct;
            auto final_cp_plus_one = first;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.insert(t.begin() + 1, final_cp_plus_one, last));
        }
    }
}

TEST(text, test_erase)
{
    {
        text::text t("string");
        text::text_view const ctv(t.begin(), t.size() + 1); // Explicitly null-terminated.
        t.erase(ctv);
        EXPECT_EQ(t, "");
        EXPECT_EQ(t[t.size()], '\0');
    }

    text::text const ct("string");

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::text t = ct;
            text::text_view const before = t(0, i);
            text::text_view const substr = t(i, j);
            text::text_view const after = t(j, t.size());

            text::text expected(before);
            expected += after;

            t.erase(substr);
            EXPECT_EQ(t[t.size()], '\0') << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    {
        // Unicode 9, 3.9/D90
        uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::text const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t));
            EXPECT_EQ(t[t.size()], '\0');
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t(0, 0)));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t(1, 1)));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t(2, 2)));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t(3, 3)));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t(4, 4)));
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(-1)), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(-2)), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(-3)), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(1)), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(2)), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.erase(t(3)), std::invalid_argument);
        }

        {
            // Breaking the encoding is fine with the iterator interface.
            text::text t = ct;
            EXPECT_NO_THROW(t.erase(t.end() - 2, t.end() - 1));
        }
    }
}

TEST(text, test_replace)
{
    text::text_view const replacement("REP");
    // Explicitly null-terminated.
    text::text_view const replacement_with_null(replacement.begin(), replacement.size() + 1);

    {
        text::text t("string");
        text::text_view const ctv(t.begin(), t.size() + 1); // Explicitly null-terminated.
        t.replace(ctv, replacement_with_null);
        EXPECT_EQ(t, "REP");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t("string");
        text::text_view const ctv(t.begin(), t.size() + 1); // Explicitly null-terminated.
        t.replace(ctv, replacement);
        EXPECT_EQ(t, "REP");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP");
        EXPECT_EQ(t[t.size()], '\0');
    }

    text::text const ct("string");

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::text t = ct;
            text::text_view const before = t(0, i);
            text::text_view const substr = t(i, j);
            text::text_view const after = t(j, t.size());

            text::text expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr, replacement);
            EXPECT_EQ(t[t.size()], '\0') << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    text::repeated_text_view const really_long_replacement(replacement, 10);

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::text t = ct;
            text::text_view const before = t(0, i);
            text::text_view const substr = t(i, j);
            text::text_view const after = t(j, t.size());

            text::text expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr, really_long_replacement);
            EXPECT_EQ(t[t.size()], '\0') << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    {
        // Unicode 9, 3.9/D90
        uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::text const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t, "something"));
            EXPECT_EQ(t[t.size()], '\0');
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(0, 0), "something"));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(1, 1), "something"));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(2, 2), "something"));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(3, 3), "something"));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(4, 4), "something"));
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -1), "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -2), "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -3), "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(1), "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(2), "something"), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(3), "something"), std::invalid_argument);
        }
    }
}

TEST(text, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);
    auto const final_cp = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
    auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

    text::text const ct_string("string");
    text::text const ct_text("text");

    {
        text::text t = ct_string;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t = ct_text;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
        EXPECT_EQ(t[t.size()], '\0');
    }

    {
        text::text t = ct_string;
        t.replace(t, first, last);
        EXPECT_EQ(t, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
        EXPECT_EQ(t[t.size()], '\0');
    }

    for (int j = 0; j <= ct_string.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                text::text t = ct_string;
                text::text_view const before = t(0, i);
                text::text_view const substr = t(i, j);
                text::text_view const after = t(j, t.size());

                text::text expected(before);
                expected.insert(expected.size(), final_cp, last);
                expected += after;

                t.replace(substr, final_cp, last);
                EXPECT_EQ(t[t.size()], '\0') << "i=" << i << " j=" << j << " erasing '" << substr << "'";
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }

            {
                text::text t = ct_string;
                text::text_view const before = t(0, i);
                text::text_view const substr = t(i, j);
                text::text_view const after = t(j, t.size());

                text::text expected(before);
                expected.insert(expected.size(), first, last);
                expected += after;

                t.replace(substr, first, last);
                EXPECT_EQ(t[t.size()], '\0') << "i=" << i << " j=" << j << " erasing '" << substr << "'";
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }
        }
    }

    {
        text::text const ct(final_cp, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t, final_cp, last));
            EXPECT_EQ(t[t.size()], '\0');
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(0, 0), final_cp, last));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(1, 1), final_cp, last));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(2, 2), final_cp, last));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(3, 3), final_cp, last));
        }

        {
            text::text t = ct;
            EXPECT_NO_THROW(t.replace(t(4, 4), final_cp, last));
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -1), final_cp, last), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -2), final_cp, last), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(0, -3), final_cp, last), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(1), final_cp, last), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(2), final_cp, last), std::invalid_argument);
        }

        {
            text::text t = ct;
            EXPECT_THROW(t.replace(t(3), final_cp, last), std::invalid_argument);
        }

        {
            // Broken encoding in the inserted range is fine, since the
            // iterator interface is considered unsafe.
            text::text t = ct;
            auto final_cp_plus_one = final_cp;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.replace(t, final_cp_plus_one, last));
        }

        {
            // Broken encoding in the removed *and* inserted ranges is fine,
            // since the iterator interface is considered unsafe.
            text::text t = ct;
            auto final_cp_plus_one = final_cp;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.replace(t.begin() + 1, t.end(), final_cp_plus_one, last));
            EXPECT_EQ(t, ct);
        }
    }
}

TEST(text, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (int i = 0; i < 5000; ++i) {
        utf32_repeated.insert(utf32_repeated.end(), utf32, utf32 + 4);
    }
    auto const first = text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(utf32_repeated.begin());
    auto const last = text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(utf32_repeated.end());

    {
        text::text t("string");
        t.replace(t, first, last);
        text::text const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text::text t;
        t.replace(t, first, last);
        text::text const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

// TODO: Add out-of-memory tests (in another file).  These should especially
// test the Iter interfaces.
