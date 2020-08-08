// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_view.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>

#include <boost/text/text.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/rope_view.hpp>

#include <sstream>

#include <gtest/gtest.h>


using namespace boost;

/*  Each of the operations below should work for the following types:

    char (&literal)[N]
    char const * c_str
    std::string str

    string_view sv
    std::string s             // Right-hand side only.
    unencoded_rope ur
    unencoded_rope_view urv

    text_view tv
    text t
    rope r 
    rope_view rv
*/

/*  For each text-defined T above, covered operations must be:
    T x = "str"; // Init from string literal.
    std::vector<T> v = {"str"};  // Init from string literal in initializer_list.

    T (U const &)
    T (U &&)
    T::operator=(U const &)
    operator==(T const &, U const &)
    operator+(T const &, U const &)
    operator<<(std::ostream&, T const &)

    T::begin(), T::end(), T::cbegin(), T::cend()
    T::rbegin(), T::rend(), T::crbegin(), T::crend()
    begin(T const &), end(T const &), cbegin(T const &), cend(T const &)
    rbegin(T const &), rend(T const &), crbegin(T const &), crend(T const &)

    Only for unencoded_rope/text/rope:
    operator+=(U const &)
    operator+=(U &&)
    insert()
    erase()
    replace()
*/

// T x = "str"; // Init from string literal.
TEST(common_operations, string_literal_init)
{
    char const * const c_str = "c_str";
    (void)c_str;
    std::string const str = "str";
    (void)str;

    text::string_view const sv = "sv";
    (void)sv;
    std::string const s = "s";
    (void)s;
    text::unencoded_rope const ur = "ur";
    (void)ur;
    text::unencoded_rope_view const urv = "urv";
    (void)urv;

    text::text const t = "t";
    (void)t;
    text::rope const r = "r";
    (void)r;
}

// std::vector<T> v = {"str"};  // Init from string literal in initializer_list.
TEST(common_operations, string_literal_init_initializer_list)
{
    std::vector<char const *> const c_str = {"c_str"};
    (void)c_str;
    std::vector<std::string> const str = {"str"};
    (void)str;

    std::vector<text::string_view> const sv = {"sv"};
    (void)sv;
    std::vector<std::string> const s = {"s"};
    (void)s;
    std::vector<text::unencoded_rope> const ur = {"ur"};
    (void)ur;
    std::vector<text::unencoded_rope_view> const urv = {"urv"};
    (void)urv;

    std::vector<text::text> const t = {"t"};
    (void)t;
    std::vector<text::rope> const r = {"r"};
    (void)r;
}

#define ONE_OF_EACH()                                                          \
    /* Should be written out by hand so that it does not decay: char           \
       (&literal)[N]*/                                                         \
    char const * const c_str = "c_str";                                        \
    (void)c_str;                                                               \
    std::string const str("str");                                              \
    (void)str;                                                                 \
                                                                               \
    text::string_view const sv("sv");                                          \
    (void)sv;                                                                  \
    std::string const s("s");                                                  \
    (void)s;                                                                   \
    text::unencoded_rope const ur("ur");                                       \
    (void)ur;                                                                  \
    text::unencoded_rope_view const urv(ur);                                   \
    (void)urv;                                                                 \
                                                                               \
    text::text const t("t");                                                   \
    (void)t;                                                                   \
    text::text_view const tv(t.begin(), t.end());                              \
    (void)tv;                                                                  \
    text::rope const r("r");                                                   \
    (void)r;                                                                   \
    text::rope_view const rv(r);                                               \
    (void)rv

#define ONE_OF_EACH_MUTABLE()                                                  \
    /* Should be written out by hand so that it does not decay: char           \
       (&literal)[N]*/                                                         \
    char const * c_str = "c_str";                                              \
    (void)c_str;                                                               \
    std::string str("str");                                                    \
    (void)str;                                                                 \
                                                                               \
    text::string_view sv("sv");                                                \
    (void)sv;                                                                  \
    std::string s("s");                                                        \
    (void)s;                                                                   \
    text::unencoded_rope ur("ur");                                             \
    (void)ur;                                                                  \
    text::unencoded_rope_view urv(ur);                                         \
    (void)urv;                                                                 \
                                                                               \
    text::text t("t");                                                         \
    (void)t;                                                                   \
    text::text_view tv(t.cbegin(), t.cend());                                  \
    (void)tv;                                                                  \
    text::rope r("r");                                                         \
    (void)r;                                                                   \
    text::rope_view rv(r);                                                     \
    (void)rv

// T (U const &)
TEST(common_operations, construction)
{
    ONE_OF_EACH();

    // text::string_view
    {
        text::string_view const sv_literal("literal");
        (void)sv_literal;
        text::string_view const sv_c_str(c_str);
        (void)sv_c_str;
        text::string_view const sv_str(str);
        (void)sv_str;
        // OK text::string_view const sv_rsv(rsv);
        text::string_view const sv_sv(sv);
        (void)sv_sv;
        text::string_view const sv_s(s);
        (void)sv_s;
        // OK text::string_view const sv_ur(ur);
        // OK text::string_view const sv_urv(urv);
        // OK text::string_view const sv_t(t);
        // OK text::string_view const sv_tv(tv);
        // OK text::string_view const sv_r(r);
        // OK text::string_view const sv_rv(rv);
    }

    // text::unencoded_rope
    {
        text::unencoded_rope const ur_literal("literal");
        (void)ur_literal;
        text::unencoded_rope const ur_c_str(c_str);
        (void)ur_c_str;
        text::unencoded_rope const ur_str(str);
        (void)ur_str;
        text::unencoded_rope const ur_sv(sv);
        (void)ur_sv;
        text::unencoded_rope const ur_s(s);
        (void)ur_s;
        text::unencoded_rope const ur_ur(ur);
        (void)ur_ur;
        text::unencoded_rope const ur_urv(urv);
        (void)ur_urv;
        // OK text::unencoded_rope const ur_t(t);
        // OK text::unencoded_rope const ur_tv(tv);
        // OK text::unencoded_rope const ur_r(r);
        // OK text::unencoded_rope const ur_rv(rv);
    }

    // text::unencoded_rope_view
    {
        text::unencoded_rope_view const urv_literal("literal");
        (void)urv_literal;
        text::unencoded_rope_view const urv_c_str(c_str);
        (void)urv_c_str;
        text::unencoded_rope_view const urv_str(str);
        (void)urv_str;
        text::unencoded_rope_view const urv_sv(sv);
        (void)urv_sv;
        text::unencoded_rope_view const urv_s(s);
        (void)urv_s;
        text::unencoded_rope_view const urv_ur(ur);
        (void)urv_ur;
        text::unencoded_rope_view const urv_urv(urv);
        (void)urv_urv;
        // OK text::unencoded_rope_view const urv_t(t);
        // OK text::unencoded_rope_view const urv_tv(tv);
        // OK text::unencoded_rope_view const urv_r(r);
        // OK text::unencoded_rope_view const urv_rv(rv);
    }

    // text::text_view
    {
        // OK text::text_view const tv_literal("literal");
        // OK text::text_view const tv_c_str(c_str);
        // OK text::text_view const tv_str(str);
        // OK text::text_view const tv_sv(sv);
        // OK text::text_view const tv_s(s);
        // OK text::text_view const tv_ur(ur);
        // OK text::text_view const tv_urv(urv);
        text::text_view const tv_t(t);
        (void)tv_t;
        text::text_view const tv_tv(tv);
        (void)tv_tv;
        // OK text::text_view const tv_r(r);
        // OK text::text_view const tv_rv(rv);
    }

    // text::text
    {
        text::text const t_literal("literal");
        (void)t_literal;
        text::text const t_c_str(c_str);
        (void)t_c_str;
        text::text const t_str(str);
        (void)t_str;
        text::text const t_sv(sv);
        (void)t_sv;
        text::text const t_s(s);
        (void)t_s;
        text::text const t_ur(ur);
        (void)t_ur;
        text::text const t_urv(urv);
        (void)t_urv;
        text::text const t_t(t);
        (void)t_t;
        text::text const t_tv(tv);
        (void)t_tv;
        text::text const t_r(r);
        (void)t_r;
        text::text const t_rv(rv);
        (void)t_rv;
    }

    // text::rope
    {
        text::rope const r_literal("literal");
        (void)r_literal;
        text::rope const r_c_str(c_str);
        (void)r_c_str;
        text::rope const r_str(str);
        (void)r_str;
        text::rope const r_sv(sv);
        (void)r_sv;
        text::rope const r_s(s);
        (void)r_s;
        text::rope const r_ur(ur);
        (void)r_ur;
        text::rope const r_urv(urv);
        (void)r_urv;
        text::rope const r_t(t);
        (void)r_t;
        text::rope const r_tv(tv);
        (void)r_tv;
        text::rope const r_r(r);
        (void)r_r;
        text::rope const r_rv(rv);
        (void)r_rv;
    }

    // text::rope_view
    {
        // OK text::rope_view const rv_literal("literal");
        // OK text::rope_view const rv_c_str(c_str);
        // OK text::rope_view const rv_str(str);
        // OK text::rope_view const rv_sv(sv);
        // OK text::rope_view const rv_s(s);
        // OK text::rope_view const rv_ur(ur);
        // OK text::rope_view const rv_urv(urv);
        text::rope_view const rv_t(t);
        (void)rv_t;
        text::rope_view const rv_tv(tv);
        (void)rv_tv;
        text::rope_view const rv_r(r);
        (void)rv_r;
        text::rope_view const rv_rv(rv);
        (void)rv_rv;
    }
}

// T (U &&)
TEST(common_operations, move_construction)
{
    // text::string_view
    {
        ONE_OF_EACH_MUTABLE();

        // MSVC doesn't like moving an array reference.
        // text::string_view const sv_literal(std::move("literal"));
        text::string_view const sv_c_str(std::move(c_str));
        (void)sv_c_str;
        text::string_view const sv_str(std::move(str));
        (void)sv_str;
        text::string_view const sv_sv(std::move(sv));
        (void)sv_sv;
        // OK text::string_view const sv_s(std::move(s));
        // OK text::string_view const sv_urv(urv);
        // OK text::string_view const sv_ur(ur);
        // OK text::string_view const sv_tv(std::move(tv));
        // OK text::string_view const sv_t(std::move(t));
        // OK text::string_view const sv_rv(rv);
        // OK text::string_view const sv_r(r);
    }

    // text::unencoded_rope
    {
        ONE_OF_EACH_MUTABLE();

        // MSVC doesn't like moving an array reference.
        // text::unencoded_rope const ur_literal(std::move("literal"));
        text::unencoded_rope const ur_c_str(std::move(c_str));
        (void)ur_c_str;
        text::unencoded_rope const ur_str(std::move(str));
        (void)ur_str;
        text::unencoded_rope const ur_sv(std::move(sv));
        (void)ur_sv;
        text::unencoded_rope const ur_s(std::move(s));
        (void)ur_s;
        text::unencoded_rope const ur_urv(std::move(urv));
        (void)ur_urv;
        text::unencoded_rope const ur_ur(std::move(ur));
        (void)ur_ur;
        // OK text::unencoded_rope const ur_tv(std::move(tv));
        // OK text::unencoded_rope const ur_t(std::move(t));
        // OK text::unencoded_rope const ur_rv(std::move(rv));
        // OK text::unencoded_rope const ur_r(std::move(r));
    }

    // text::unencoded_rope_view
    {
        ONE_OF_EACH_MUTABLE();

        // MSVC doesn't like moving an array reference.
        // text::unencoded_rope_view const urv_literal(std::move("literal"));
        text::unencoded_rope_view const urv_c_str(std::move(c_str));
        (void)urv_c_str;
        // OK text::unencoded_rope_view const urv_str(std::move(str));
        text::unencoded_rope_view const urv_sv(std::move(sv));
        (void)urv_sv;
        // OK text::unencoded_rope_view const urv_s(std::move(s));
        text::unencoded_rope_view const urv_urv(std::move(urv));
        (void)urv_urv;
        // OK text::unencoded_rope_view const urv_ur(std::move(ur));
        // OK text::unencoded_rope_view const urv_tv(std::move(tv));
        // OK text::unencoded_rope_view const urv_t(std::move(t));
        // OK text::unencoded_rope_view const urv_rv(rv);
        // OK text::unencoded_rope_view const urv_r(r);
    }

    // text::text_view
    {
        ONE_OF_EACH_MUTABLE();

        // OK text::text_view const tv_literal("literal");
        // OK text::text_view const tv_c_str(c_str);
        // OK text::text_view const tv_str(str);
        // OK text::text_view const tv_sv(sv);
        // OK text::text_view const tv_s(s);
        // OK text::text_view const tv_urv(urv);
        // OK text::text_view const tv_ur(ur);
        text::text_view const tv_tv(std::move(tv));
        (void)tv_tv;
        // OK text::text_view const tv_t(std::move(t));
        // OK text::text_view const tv_rv(rv);
        // OK text::text_view const tv_r(r);
    }

    // text::text
    {
        ONE_OF_EACH_MUTABLE();

        // MSVC doesn't like moving an array reference.
        // text::text const t_literal(std::move("literal"));
        text::text const t_c_str(std::move(c_str));
        (void)t_c_str;
        text::text const t_str(std::move(str));
        (void)t_str;
        text::text const t_sv(std::move(sv));
        (void)t_sv;
        text::text const t_s(std::move(s));
        (void)t_s;
        text::text const t_urv(std::move(urv));
        (void)t_urv;
        text::text const t_ur(std::move(ur));
        (void)t_ur;
        text::text const t_tv(std::move(tv));
        (void)t_tv;
        text::text const t_t(std::move(t));
        (void)t_t;
        text::text const t_rv(std::move(rv));
        (void)t_rv;
        text::text const t_r(std::move(r));
        (void)t_r;
    }

    // text::rope
    {
        ONE_OF_EACH_MUTABLE();

        // MSVC doesn't like moving an array reference.
        // text::rope const r_literal(std::move("literal"));
        text::rope const r_c_str(std::move(c_str));
        (void)r_c_str;
        text::rope const r_str(std::move(str));
        (void)r_str;
        text::rope const r_sv(std::move(sv));
        (void)r_sv;
        text::rope const r_s(std::move(s));
        (void)r_s;
        text::rope const r_urv(std::move(urv));
        (void)r_urv;
        text::rope const r_ur(std::move(ur));
        (void)r_ur;
        text::rope const r_tv(std::move(tv));
        (void)r_tv;
        text::rope const r_t(std::move(t));
        (void)r_t;
        text::rope const r_rv(std::move(rv));
        (void)r_rv;
        text::rope const r_r(std::move(r));
        (void)r_r;
    }

    // text::rope_view
    {
        ONE_OF_EACH_MUTABLE();

        // OK text::rope_view const rv_literal("literal");
        // OK text::rope_view const rv_c_str(c_str);
        // OK text::rope_view const rv_str(str);
        // OK text::rope_view const rv_sv(sv);
        // OK text::rope_view const rv_s(s);
        // OK text::rope_view const rv_urv(urv);
        // OK text::rope_view const rv_ur(ur);
        text::rope_view const rv_tv(std::move(tv));
        (void)rv_tv;
        // OK text::rope_view const rv_t(std::move(t));
        text::rope_view const rv_rv(std::move(rv));
        (void)rv_rv;
        // OK text::rope_view const rv_r(std::move(r));
    }
}

// T::operator=(U const &)
TEST(common_operations, assignment_operators)
{
    ONE_OF_EACH();

    // text::string_view
    {
        text::string_view sv_literal;
        sv_literal = "literal";
        text::string_view sv_c_str;
        sv_c_str = c_str;
        text::string_view sv_str;
        sv_str = str;
        text::string_view sv_sv;
        sv_sv = sv;
        text::string_view sv_s;
        sv_s = s;
        // OK text::string_view sv_ur(ur);
        // OK text::string_view sv_urv(urv);
        // OK text::string_view sv_t;
        // OK text::string_view sv_tv;
        // OK text::string_view sv_r(r);
        // OK text::string_view sv_rv(rv);
    }

    // text::unencoded_rope
    {
        text::unencoded_rope ur_literal;
        ur_literal = "literal";
        text::unencoded_rope ur_c_str;
        ur_c_str = c_str;
        text::unencoded_rope ur_str;
        ur_str = str;
        text::unencoded_rope ur_sv;
        ur_sv = sv;
        text::unencoded_rope ur_s;
        ur_s = s;
        text::unencoded_rope ur_ur;
        ur_ur = ur;
        text::unencoded_rope ur_urv;
        ur_urv = urv;
        // OK text::unencoded_rope ur_t;
        // OK text::unencoded_rope ur_tv;
        // OK text::unencoded_rope ur_r;
        // OK text::unencoded_rope ur_rv;
    }

    // text::unencoded_rope_view
    {
        text::unencoded_rope_view urv_literal;
        urv_literal = "literal";
        text::unencoded_rope_view urv_c_str;
        urv_c_str = c_str;
        text::unencoded_rope_view urv_str;
        urv_str = str;
        text::unencoded_rope_view urv_sv;
        urv_sv = sv;
        text::unencoded_rope_view urv_s;
        urv_s = s;
        text::unencoded_rope_view urv_ur;
        urv_ur = ur;
        text::unencoded_rope_view urv_urv;
        urv_urv = urv;
        // OK text::unencoded_rope_view urv_t;
        // OK text::unencoded_rope_view urv_tv;
        // OK text::unencoded_rope_view urv_r(r);
        // OK text::unencoded_rope_view urv_rv(rv);
    }

    // text::text_view
    {
        // OK text::text_view tv_literal("literal");
        // OK text::text_view tv_c_str(c_str);
        // OK text::text_view tv_str(str);
        // OK text::text_view tv_sv(sv);
        // OK text::text_view tv_s(s);
        // OK text::text_view tv_ur(ur);
        // OK text::text_view tv_urv(urv);
        text::text_view tv_t;
        tv_t = t;
        text::text_view tv_tv;
        tv_tv = tv;
        // OK text::text_view tv_r(r);
        // OK text::text_view tv_rv(rv);
    }

    // text::text
    {
        text::text t_literal;
        t_literal = "literal";
        text::text t_c_str;
        t_c_str = c_str;
        text::text t_str;
        t_str = str;
        text::text t_sv;
        t_sv = sv;
        text::text t_s;
        t_s = s;
        // OK text::text t_ur;
        // OK text::text t_urv;
        text::text t_t;
        t_t = t;
        text::text t_tv;
        t_tv = tv;
#if 0 // TODO: Waiting on operator=(rope_view) support in text.
        text::text t_r;
        t_r = r;
        text::text t_rv;
        t_rv = rv;
#endif
    }

    // text::rope
    {
        text::rope r_literal;
        r_literal = "literal";
        text::rope r_c_str;
        r_c_str = c_str;
        text::rope r_str;
        r_str = str;
        text::rope r_sv;
        r_sv = sv;
        text::rope r_s;
        r_s = s;
        // OK text::rope r_ur;
        // OK text::rope r_urv;
        text::rope r_t;
        r_t = t;
        text::rope r_tv;
        r_tv = tv;
        text::rope r_r;
        r_r = r;
        text::rope r_rv;
        r_rv = rv;
    }

    // text::rope_view
    {
        // OK text::rope_view rv_literal("literal");
        // OK text::rope_view rv_c_str(c_str);
        // OK text::rope_view rv_str(str);
        // OK text::rope_view rv_sv(sv);
        // OK text::rope_view rv_s(s);
        // OK text::rope_view rv_ur(ur);
        // OK text::rope_view rv_urv(urv);
        text::rope_view rv_t;
        rv_t = t;
        text::rope_view rv_tv;
        rv_tv = tv;
        text::rope_view rv_r;
        rv_r = r;
        text::rope_view rv_rv;
        rv_rv = rv;
    }
}

// operator==(T const &, U const &)
TEST(common_operations, equality_comparisons)
{
    ONE_OF_EACH();

    // text::string_view
    {
        text::string_view sv_literal("literal");
        EXPECT_TRUE(sv_literal == "literal");
        EXPECT_TRUE("literal" == sv_literal);
        text::string_view sv_c_str(c_str);
        EXPECT_TRUE(sv_c_str == c_str);
        EXPECT_TRUE(c_str == sv_c_str);
        // OK text::string_view sv_str(str);
        text::string_view sv_sv(sv);
        EXPECT_TRUE(sv_sv == sv);
        EXPECT_TRUE(sv == sv_sv);
        text::string_view sv_s(s);
        EXPECT_TRUE(sv_s == s);
        EXPECT_TRUE(s == sv_s);
        // OK text::string_view sv_ur(ur);
        // OK text::string_view sv_urv(urv);
        // OK text::string_view sv_t(t);
        // OK text::string_view sv_tv(tv);
        // OK text::string_view sv_r(r);
        // OK text::string_view sv_rv(rv);
    }

    // text::unencoded_rope
    {
        text::unencoded_rope ur_literal("literal");
        EXPECT_TRUE(ur_literal == "literal");
        EXPECT_TRUE("literal" == ur_literal);
        text::unencoded_rope ur_c_str(c_str);
        EXPECT_TRUE(ur_c_str == c_str);
        EXPECT_TRUE(c_str == ur_c_str);
        text::unencoded_rope ur_str(str);
        EXPECT_TRUE(ur_str == str);
        EXPECT_TRUE(str == ur_str);
        text::unencoded_rope ur_sv(sv);
        EXPECT_TRUE(ur_sv == sv);
        EXPECT_TRUE(sv == ur_sv);
        text::unencoded_rope ur_s(s);
        EXPECT_TRUE(ur_s == s);
        EXPECT_TRUE(s == ur_s);
        text::unencoded_rope ur_ur(ur);
        EXPECT_TRUE(ur_ur == ur);
        EXPECT_TRUE(ur == ur_ur);
        text::unencoded_rope ur_urv(urv);
        EXPECT_TRUE(ur_urv == urv);
        EXPECT_TRUE(urv == ur_urv);
        // OK text::unencoded_rope ur_t(t);
        // OK text::unencoded_rope ur_tv(tv);
        // OK text::unencoded_rope ur_r(r);
        // OK text::unencoded_rope ur_rv(rv);
    }

    // text::unencoded_rope_view
    {
        text::unencoded_rope_view urv_literal("literal");
        EXPECT_TRUE(urv_literal == "literal");
        EXPECT_TRUE("literal" == urv_literal);
        text::unencoded_rope_view urv_c_str(c_str);
        EXPECT_TRUE(urv_c_str == c_str);
        EXPECT_TRUE(c_str == urv_c_str);
        // OK text::unencoded_rope_view urv_str(str);
        text::unencoded_rope_view urv_sv(sv);
        EXPECT_TRUE(urv_sv == sv);
        EXPECT_TRUE(sv == urv_sv);
        text::unencoded_rope_view urv_s(s);
        EXPECT_TRUE(urv_s == s);
        EXPECT_TRUE(s == urv_s);
        text::unencoded_rope_view urv_ur(ur);
        EXPECT_TRUE(urv_ur == ur);
        EXPECT_TRUE(ur == urv_ur);
        text::unencoded_rope_view urv_urv(urv);
        EXPECT_TRUE(urv_urv == urv);
        EXPECT_TRUE(urv == urv_urv);
        // OK text::unencoded_rope_view urv_t(t);
        // OK text::unencoded_rope_view urv_tv(tv);
        // OK text::unencoded_rope_view urv_r(r);
        // OK text::unencoded_rope_view urv_rv(rv);
    }

    // text::text_view
    {
        // OK text::text_view tv_literal("literal");
        // OK text::text_view tv_c_str(c_str);
        // OK text::text_view tv_str(str);
        // OK text::text_view tv_sv(sv);
        // OK text::text_view tv_s(s);
        // OK text::text_view tv_ur(ur);
        // OK text::text_view tv_urv(urv);
        text::text_view tv_t(t);
        EXPECT_TRUE(tv_t == t);
        EXPECT_TRUE(t == tv_t);
        text::text_view tv_tv(tv);
        EXPECT_TRUE(tv_tv == tv);
        EXPECT_TRUE(tv == tv_tv);
        // OK text::text_view tv_r(r);
        // OK text::text_view tv_rv(rv);
    }

    // text::text
    {
        // OK text::text t_literal("literal");
        // OK text::text t_c_str(c_str);
        // OK text::text t_str(str);
        // OK text::text t_sv(sv);
        // OK text::text t_s(s);
        // OK text::text t_ur(ur);
        // OK text::text t_urv(urv);
        text::text t_t(t);
        EXPECT_TRUE(t_t == t);
        EXPECT_TRUE(t == t_t);
        text::text t_tv(tv);
        EXPECT_TRUE(t_tv == tv);
        EXPECT_TRUE(tv == t_tv);
        text::text t_r(r);
        EXPECT_TRUE(t_r == r);
        EXPECT_TRUE(r == t_r);
        text::text t_rv(rv);
        EXPECT_TRUE(t_rv == rv);
        EXPECT_TRUE(rv == t_rv);
    }

    // text::rope
    {
        // OK text::rope r_literal("literal");
        // OK text::rope r_c_str(c_str);
        // OK text::rope r_str(str);
        // OK text::rope r_sv(sv);
        // OK text::rope r_s(s);
        // OK text::rope r_ur(ur);
        // OK text::rope r_urv(urv);
        text::rope r_t(t);
        EXPECT_TRUE(r_t == t);
        EXPECT_TRUE(t == r_t);
        text::rope r_tv(tv);
        EXPECT_TRUE(r_tv == tv);
        EXPECT_TRUE(tv == r_tv);
        text::rope r_r(r);
        EXPECT_TRUE(r_r == r);
        EXPECT_TRUE(r == r_r);
        text::rope r_rv(rv);
        EXPECT_TRUE(r_rv == rv);
        EXPECT_TRUE(rv == r_rv);
    }

    // text::rope_view
    {
        // OK text::rope_view rv_literal("literal");
        // OK text::rope_view rv_c_str(c_str);
        // OK text::rope_view rv_str(str);
        // OK text::rope_view rv_sv(sv);
        // OK text::rope_view rv_s(s);
        // OK text::rope_view rv_ur(ur);
        // OK text::rope_view rv_urv(urv);
        text::rope_view rv_t(t);
        EXPECT_TRUE(rv_t == t);
        EXPECT_TRUE(t == rv_t);
        text::rope_view rv_tv(tv);
        EXPECT_TRUE(rv_tv == tv);
        EXPECT_TRUE(tv == rv_tv);
        text::rope_view rv_r(r);
        EXPECT_TRUE(rv_r == r);
        EXPECT_TRUE(r == rv_r);
        text::rope_view rv_rv(rv);
        EXPECT_TRUE(rv_rv == rv);
        EXPECT_TRUE(rv == rv_rv);
    }
}

// operator+(T const &, U const &)
TEST(common_operations, operator_plus)
{
    ONE_OF_EACH();

    // text::string_view
    {
        // OK sv + "literal";
        // OK sv + c_str;
        // OK sv + str;
        // OK sv + sv;
        // OK EXPECT_EQ(sv + s, "svs");
        // OK EXPECT_EQ(s + sv, "ssv");
        EXPECT_EQ(sv + ur, "svur");
        EXPECT_EQ(ur + sv, "ursv");
        // OK sv + urv;
        EXPECT_EQ(sv + t, text::text("svt"));
        EXPECT_EQ(t + sv, text::text("tsv"));
        // OK sv + tv;
        EXPECT_EQ(sv + r, text::rope("svr"));
        EXPECT_EQ(r + sv, text::rope("rsv"));
        // OK sv + rv;
    }

    // text::unencoded_rope
    {
        EXPECT_EQ(ur + "literal", "urliteral");
        EXPECT_EQ("literal" + ur, "literalur");
        EXPECT_EQ(ur + c_str, "urc_str");
        EXPECT_EQ(c_str + ur, "c_strur");
        EXPECT_EQ(ur + str, "urstr");
        EXPECT_EQ(str + ur, "strur");
        EXPECT_EQ(ur + sv, "ursv");
        EXPECT_EQ(sv + ur, "svur");
        EXPECT_EQ(ur + s, "urs");
        EXPECT_EQ(s + ur, "sur");
        EXPECT_EQ(ur + ur, "urur");
        EXPECT_EQ(ur + ur, "urur");
        EXPECT_EQ(ur + urv, "urur");
        EXPECT_EQ(urv + ur, "urur");
        // OK EXPECT_EQ(ur + t, text::text("urt"));
        // OK EXPECT_EQ(t + ur, text::text("tur"));
        // OK ur + tv;
        // OK EXPECT_EQ(ur + r, text::rope("urr"));
        // OK EXPECT_EQ(r + ur, text::rope("rur"));
        // OK ur + rv;
    }

    // text::unencoded_rope_view
    {
        // OK urv + "literal";
        // OK urv + c_str;
        // OK urv + str;
        // OK urv + sv;
        // OK EXPECT_EQ(urv + s, "urs");
        // OK EXPECT_EQ(s + urv, "sur");
        EXPECT_EQ(urv + ur, "urur");
        EXPECT_EQ(ur + urv, "urur");
        // OK urv + urv;
        // OK EXPECT_EQ(urv + t, text::text("urt"));
        // OK EXPECT_EQ(t + urv, text::text("tur"));
        // OK urv + tv;
        // OK EXPECT_EQ(urv + r, text::rope("urr"));
        // OK EXPECT_EQ(r + urv, text::rope("rur"));
        // OK urv + rv;
    }

    // text::text_view
    {
        // OK tv + "literal";
        // OK tv + c_str;
        // OK tv + str;
        // OK tv + sv;
        // OK tv + s;
        // OK tv + ur;
        // OK tv + urv;
        EXPECT_EQ(tv + t, text::text("tt"));
        EXPECT_EQ(t + tv, text::text("tt"));
        // OK tv + tv;
        EXPECT_EQ(tv + r, text::rope("tr"));
        EXPECT_EQ(r + tv, text::rope("rt"));
        // OK tv + rv;
    }

    // text::text
    {
        EXPECT_EQ(t + "literal", text::text("tliteral"));
        EXPECT_EQ("literal" + t, text::text("literalt"));
        EXPECT_EQ(t + c_str, text::text("tc_str"));
        EXPECT_EQ(c_str + t, text::text("c_strt"));
        EXPECT_EQ(t + str, text::text("tstr"));
        EXPECT_EQ(str + t, text::text("strt"));
        EXPECT_EQ(t + sv, text::text("tsv"));
        EXPECT_EQ(sv + t, text::text("svt"));
        EXPECT_EQ(t + s, text::text("ts"));
        EXPECT_EQ(s + t, text::text("st"));
        // OK EXPECT_EQ(t + ur, text::text("tur"));
        // OK EXPECT_EQ(ur + t, text::text("urt"));
        // OK EXPECT_EQ(t + urv, text::text("tur"));
        // OK EXPECT_EQ(urv + t, text::text("urt"));
        EXPECT_EQ(t + t, text::text("tt"));
        EXPECT_EQ(t + t, text::text("tt"));
        EXPECT_EQ(t + tv, text::text("tt"));
        EXPECT_EQ(tv + t, text::text("tt"));
        // OK EXPECT_EQ(t + r, text::rope("tr"));
        // OK EXPECT_EQ(r + t, text::rope("rt"));
        EXPECT_EQ(t + rv, text::rope("tr"));
        EXPECT_EQ(rv + t, text::rope("rt"));
    }

    // text::rope
    {
        EXPECT_EQ(r + "literal", text::rope("rliteral"));
        EXPECT_EQ("literal" + r, text::rope("literalr"));
        EXPECT_EQ(r + c_str, text::rope("rc_str"));
        EXPECT_EQ(c_str + r, text::rope("c_strr"));
        EXPECT_EQ(r + str, text::rope("rstr"));
        EXPECT_EQ(str + r, text::rope("strr"));
        EXPECT_EQ(r + sv, text::rope("rsv"));
        EXPECT_EQ(sv + r, text::rope("svr"));
        EXPECT_EQ(r + s, text::rope("rs"));
        EXPECT_EQ(s + r, text::rope("sr"));
        // OK EXPECT_EQ(r + ur, text::rope("rur"));
        // OK EXPECT_EQ(ur + r, text::rope("urr"));
        // OK EXPECT_EQ(r + urv, text::rope("rur"));
        // OK EXPECT_EQ(urv + r, text::rope("urr"));
        EXPECT_EQ(r + r, text::rope("rr"));
        EXPECT_EQ(r + r, text::rope("rr"));
        EXPECT_EQ(r + tv, text::rope("rt"));
        EXPECT_EQ(tv + r, text::rope("tr"));
        EXPECT_EQ(r + r, text::rope("rr"));
        EXPECT_EQ(r + r, text::rope("rr"));
        EXPECT_EQ(r + rv, text::rope("rr"));
        EXPECT_EQ(rv + r, text::rope("rr"));
    }

    // text::rope_view
    {
        // OK rv + "literal";
        // OK rv + c_str;
        // OK rv + str;
        // OK rv + sv;
        // OK rv + s;
        // OK rv + ur;
        // OK rv + urv;
        EXPECT_EQ(rv + t, text::rope("rt"));
        EXPECT_EQ(t + rv, text::rope("tr"));
        // OK rv + tv;
        EXPECT_EQ(rv + r, text::rope("rr"));
        EXPECT_EQ(r + rv, text::rope("rr"));
        // OK rv + rv;
    }
}

TEST(common_operations, stream_inserters)
{
    ONE_OF_EACH();

    {
        std::stringstream ss;
        ss << sv;
        EXPECT_EQ(ss.str(), "sv");
    }

    {
        std::stringstream ss;
        ss << s;
        EXPECT_EQ(ss.str(), "s");
    }

    {
        std::stringstream ss;
        ss << ur;
        EXPECT_EQ(ss.str(), "ur");
    }

    {
        std::stringstream ss;
        ss << urv;
        EXPECT_EQ(ss.str(), "ur");
    }

    {
        std::stringstream ss;
        ss << t;
        EXPECT_EQ(ss.str(), "t");
    }
    {
        std::stringstream ss;
        ss << std::setw(4) << t;
        EXPECT_EQ(ss.str(), "   t");
    }
    {
        std::stringstream ss;
        ss << std::left << std::setfill('*') << std::setw(4) << t;
        EXPECT_EQ(ss.str(), "t***");
    }

    {
        std::stringstream ss;
        ss << tv;
        EXPECT_EQ(ss.str(), "t");
    }
    {
        std::stringstream ss;
        ss << std::setw(4) << tv;
        EXPECT_EQ(ss.str(), "   t");
    }
    {
        std::stringstream ss;
        ss << std::left << std::setfill('*') << std::setw(4) << tv;
        EXPECT_EQ(ss.str(), "t***");
    }

    {
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), "r");
    }
    {
        std::stringstream ss;
        ss << std::setw(4) << r;
        EXPECT_EQ(ss.str(), "   r");
    }
    {
        std::stringstream ss;
        ss << std::left << std::setfill('*') << std::setw(4) << r;
        EXPECT_EQ(ss.str(), "r***");
    }

    {
        std::stringstream ss;
        ss << rv;
        EXPECT_EQ(ss.str(), "r");
    }
    {
        std::stringstream ss;
        ss << std::setw(4) << rv;
        EXPECT_EQ(ss.str(), "   r");
    }
    {
        std::stringstream ss;
        ss << std::left << std::setfill('*') << std::setw(4) << rv;
        EXPECT_EQ(ss.str(), "r***");
    }
}

TEST(common_operations, begin_end_variants)
{
    ONE_OF_EACH();

    EXPECT_EQ(sv.begin(), sv.cbegin());
    EXPECT_EQ(sv.end(), sv.cend());
    EXPECT_EQ(sv.rbegin(), sv.crbegin());
    EXPECT_EQ(sv.rend(), sv.crend());
    EXPECT_EQ(begin(sv), cbegin(sv));
    EXPECT_EQ(end(sv), cend(sv));
    EXPECT_EQ(rbegin(sv), crbegin(sv));
    EXPECT_EQ(rend(sv), crend(sv));

    EXPECT_EQ(s.begin(), s.cbegin());
    EXPECT_EQ(s.end(), s.cend());
    EXPECT_EQ(s.rbegin(), s.crbegin());
    EXPECT_EQ(s.rend(), s.crend());
    EXPECT_EQ(begin(s), cbegin(s));
    EXPECT_EQ(end(s), cend(s));
    EXPECT_EQ(rbegin(s), crbegin(s));
    EXPECT_EQ(rend(s), crend(s));

    EXPECT_EQ(ur.begin(), ur.cbegin());
    EXPECT_EQ(ur.end(), ur.cend());
    EXPECT_EQ(ur.rbegin(), ur.crbegin());
    EXPECT_EQ(ur.rend(), ur.crend());

    EXPECT_EQ(urv.begin(), urv.cbegin());
    EXPECT_EQ(urv.end(), urv.cend());
    EXPECT_EQ(urv.rbegin(), urv.crbegin());
    EXPECT_EQ(urv.rend(), urv.crend());

    EXPECT_EQ(t.begin(), t.cbegin());
    EXPECT_EQ(t.end(), t.cend());
    EXPECT_EQ(t.rbegin(), t.crbegin());
    EXPECT_EQ(t.rend(), t.crend());

    EXPECT_EQ(tv.begin(), tv.cbegin());
    EXPECT_EQ(tv.end(), tv.cend());
    EXPECT_EQ(tv.rbegin(), tv.crbegin());
    EXPECT_EQ(tv.rend(), tv.crend());

    EXPECT_EQ(r.begin(), r.cbegin());
    EXPECT_EQ(r.end(), r.cend());
    EXPECT_EQ(r.rbegin(), r.crbegin());
    EXPECT_EQ(r.rend(), r.crend());

    EXPECT_EQ(rv.begin(), rv.cbegin());
    EXPECT_EQ(rv.end(), rv.cend());
    EXPECT_EQ(rv.rbegin(), rv.crbegin());
    EXPECT_EQ(rv.rend(), rv.crend());
}

// operator+=(U const &)
TEST(common_operations, operator_plus_equal)
{
    ONE_OF_EACH();

    // text::string_view

    // text::unencoded_rope
    {
        text::unencoded_rope ur_;

        ur_ = ur;
        EXPECT_EQ(ur_ += "literal", "urliteral");
        ur_ = ur;
        EXPECT_EQ(ur_ += c_str, "urc_str");
        ur_ = ur;
        EXPECT_EQ(ur_ += str, "urstr");
        ur_ = ur;
        EXPECT_EQ(ur_ += sv, "ursv");
        ur_ = ur;
        EXPECT_EQ(ur_ += s, "urs");
        ur_ = ur;
        EXPECT_EQ(ur_ += ur, "urur");
        ur_ = ur;
        EXPECT_EQ(ur_ += urv, "urur");
        // OK ur_ += t;
        // OK ur_ += tv;
        // OK ur_ += r;
        // OK ur_ += rv;
    }

    // text::unencoded_rope_view
    // text::text_view

    // text::text
    {
        text::text t_;

        t_ = t;
        EXPECT_EQ(t_ += "literal", text::text("tliteral"));
        t_ = t;
        EXPECT_EQ(t_ += c_str, text::text("tc_str"));
        t_ = t;
        EXPECT_EQ(t_ += str, text::text("tstr"));
        t_ = t;
        EXPECT_EQ(t_ += sv, text::text("tsv"));
        t_ = t;
        EXPECT_EQ(t_ += s, text::text("ts"));
        t_ = t;
        // OK EXPECT_EQ(t_ += ur, text::text("tur"));
        // OK EXPECT_EQ(t_ += urv, text::text("tur"));
        EXPECT_EQ(t_ += t, text::text("tt"));
        t_ = t;
        EXPECT_EQ(t_ += tv, text::text("tt"));
        t_ = t;
        EXPECT_EQ(t_ += r, text::rope("tr"));
        t_ = t;
        EXPECT_EQ(t_ += rv, text::rope("tr"));
    }

    // text::rope
    {
        text::rope r_;

        r_ = r;
        EXPECT_EQ(r_ += "literal", text::rope("rliteral"));
        r_ = r;
        EXPECT_EQ(r_ += c_str, text::rope("rc_str"));
        r_ = r;
        EXPECT_EQ(r_ += str, text::rope("rstr"));
        r_ = r;
        EXPECT_EQ(r_ += sv, text::rope("rsv"));
        r_ = r;
        EXPECT_EQ(r_ += s, text::rope("rs"));
        r_ = r;
        // OK EXPECT_EQ(r_ += ur, text::rope("rur"));
        // OK EXPECT_EQ(r_ += urv, text::rope("rur"));
        EXPECT_EQ(r_ += t, text::rope("rt"));
        r_ = r;
        EXPECT_EQ(r_ += tv, text::rope("rt"));
        r_ = r;
        EXPECT_EQ(r_ += r, text::rope("rr"));
        r_ = r;
        EXPECT_EQ(r_ += rv, text::rope("rr"));
    }

    // text::rope_view
}

// operator+=(U &&)
TEST(common_operations, operator_plus_equal_move)
{
    ONE_OF_EACH();

    // text::string_view

    // text::unencoded_rope
    {
        text::unencoded_rope ur_;

        // MSVC doesn't like moving an array reference.
        // EXPECT_EQ(ur_ += std::move("literal"), "urliteral");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(c_str), "urc_str");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(str), "urstr");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(sv), "ursv");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(s), "urs");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(ur), "urur");
        ur_ = ur;
        EXPECT_EQ(ur_ += std::move(urv), "urur");
        // OK ur_ += std::move(t);
        // OK ur_ += std::move(tv);
        // OK ur_ += std::move(r);
        // OK ur_ += std::move(rv);
    }

    // text::unencoded_rope_view
    // text::text_view

    // text::text
    {
        text::text t_;

        // MSVC doesn't like moving an array reference.
        // EXPECT_EQ(t_ += std::move("literal"), text::text("tliteral"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(c_str), text::text("tc_str"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(str), text::text("tstr"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(sv), text::text("tsv"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(s), text::text("ts"));
        t_ = t;
        // OK EXPECT_EQ(t_ += std::move(ur), text::text("tur"));
        // OK EXPECT_EQ(t_ += std::move(urv), text::text("tur"));
        EXPECT_EQ(t_ += std::move(t), text::text("tt"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(tv), text::text("tt"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(r), text::rope("tr"));
        t_ = t;
        EXPECT_EQ(t_ += std::move(rv), text::rope("tr"));
    }

    // text::rope
    {
        text::rope r_;

        // MSVC doesn't like moving an array reference.
        // EXPECT_EQ(r_ += std::move("literal"), text::rope("rliteral"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(c_str), text::rope("rc_str"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(str), text::rope("rstr"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(sv), text::rope("rsv"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(s), text::rope("rs"));
        r_ = r;
        // OK EXPECT_EQ(r_ += std::move(ur), text::rope("rur"));
        // OK EXPECT_EQ(r_ += std::move(urv), text::rope("rur"));
        EXPECT_EQ(r_ += std::move(t), text::rope("rt"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(tv), text::rope("rt"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(r), text::rope("rr"));
        r_ = r;
        EXPECT_EQ(r_ += std::move(rv), text::rope("rr"));
    }

    // text::rope_view
}

// insert()
TEST(common_operations, insert)
{
    ONE_OF_EACH();

    // text::string_view

    // text::unencoded_rope
    {
        text::unencoded_rope ur_;

        ur_ = s;
        ur_.insert(ur_.end(), "literal");
        EXPECT_EQ(ur_, "sliteral");
        ur_ = s;
        ur_.insert(ur_.end(), c_str);
        EXPECT_EQ(ur_, "sc_str");
        ur_ = s;
        ur_.insert(ur_.end(), str);
        EXPECT_EQ(ur_, "sstr");
        ur_ = s;
        ur_.insert(ur_.end(), sv);
        EXPECT_EQ(ur_, "ssv");
        ur_ = s;
        ur_.insert(ur_.end(), s);
        EXPECT_EQ(ur_, "ss");
        ur_ = s;
        ur_.insert(ur_.end(), urv);
        EXPECT_EQ(ur_, "sur");
        // OK ur_.insert(ur_.end(), t);
        // OK ur_.insert(ur_.end(), tv);
        // OK ur_.insert(ur_.end(), r);
        // OK ur_.insert(ur_.end(), rv);
    }

    // text::unencoded_rope_view
    // text::text_view

    // text::text
    {
        text::text t_;

        t_ = t;
        t_.insert(t_.end(), "literal");
        EXPECT_EQ(t_, text::text("tliteral"));
        t_ = t;
        t_.insert(t_.end(), c_str);
        EXPECT_EQ(t_, text::text("tc_str"));
        t_ = t;
        t_.insert(t_.end(), str);
        EXPECT_EQ(t_, text::text("tstr"));
        t_ = t;
        t_.insert(t_.end(), sv);
        EXPECT_EQ(t_, text::text("tsv"));
        t_ = t;
        t_.insert(t_.end(), s);
        EXPECT_EQ(t_, text::text("ts"));
        t_ = t;
        t_.insert(t_.end(), ur);
        EXPECT_EQ(t_, text::text("tur"));
        t_ = t;
        t_.insert(t_.end(), urv);
        EXPECT_EQ(t_, text::text("tur"));
        t_ = t;
        t_.insert(t_.end(), t);
        EXPECT_EQ(t_, text::text("tt"));
        t_ = t;
        t_.insert(t_.end(), tv);
        EXPECT_EQ(t_, text::text("tt"));
        t_ = t;
        t_.insert(t_.end(), r);
        EXPECT_EQ(t_, text::rope("tr"));
        t_ = t;
        t_.insert(t_.end(), rv);
        EXPECT_EQ(t_, text::rope("tr"));
    }

    // text::rope
    {
        text::rope r_;

        r_ = r;
        r_.insert(r_.end(), "literal");
        EXPECT_EQ(r_, text::rope("rliteral"));
        r_ = r;
        r_.insert(r_.end(), c_str);
        EXPECT_EQ(r_, text::rope("rc_str"));
        r_ = r;
        r_.insert(r_.end(), str);
        EXPECT_EQ(r_, text::rope("rstr"));
        r_ = r;
        r_.insert(r_.end(), sv);
        EXPECT_EQ(r_, text::rope("rsv"));
        r_ = r;
        r_.insert(r_.end(), s);
        EXPECT_EQ(r_, text::rope("rs"));
        r_ = r;
        r_.insert(r_.end(), ur);
        EXPECT_EQ(r_, text::rope("rur"));
        r_ = r;
        r_.insert(r_.end(), urv);
        EXPECT_EQ(r_, text::rope("rur"));
        r_ = r;
        r_.insert(r_.end(), t);
        EXPECT_EQ(r_, text::rope("rt"));
        r_ = r;
        r_.insert(r_.end(), tv);
        EXPECT_EQ(r_, text::rope("rt"));
        r_ = r;
        r_.insert(r_.end(), r);
        EXPECT_EQ(r_, text::rope("rr"));
        r_ = r;
        r_.insert(r_.end(), rv);
        EXPECT_EQ(r_, text::rope("rr"));
    }

    // text::rope_view
}

// erase()
TEST(common_operations, erase)
{
    ONE_OF_EACH();

    // text::string_view

    // text::unencoded_rope
    {
        text::unencoded_rope ur_;

        ur_ = ur;
        ur_.erase(ur_(0, 1));
        EXPECT_EQ(ur_, "r");
        ur_ = ur;
        ur_.erase(ur_.begin(), ur_.end());
        EXPECT_EQ(ur_, "");
    }

    // text::unencoded_rope_view
    // text::text_view

    // text::text
    {
        text::text t_;

        t_ = t;
        t_.erase(t_.begin(), t_.end());
        EXPECT_EQ(t_, text::text(""));
        t_ = t;
        t_.erase(t_.begin(), t_.end());
        EXPECT_EQ(t_, text::text(""));
    }

    // text::rope
    {
        text::rope r_;

        r_ = r;
        r_.erase(r_.begin(), r_.end());
        EXPECT_EQ(r_, text::rope(""));
        r_ = r;
        r_.erase(r_.begin(), r_.end());
        EXPECT_EQ(r_, text::rope(""));
    }

    // text::rope_view
}

// replace()
TEST(common_operations, replace)
{
    ONE_OF_EACH();

    // text::string_view

    // text::unencoded_rope
    {
        text::unencoded_rope ur_;

        ur_ = s;
        ur_.replace(ur_(1, 1), "literal");
        EXPECT_EQ(ur_, "sliteral");
        ur_ = s;
        ur_.replace(ur_(0, 1), c_str);
        EXPECT_EQ(ur_, "c_str");
        ur_ = s;
        ur_.replace(ur_(1, 1), str);
        EXPECT_EQ(ur_, "sstr");
        ur_ = s;
        ur_.replace(ur_(1, 1), sv);
        EXPECT_EQ(ur_, "ssv");
        ur_ = s;
        ur_.replace(ur_(0, 1), s);
        EXPECT_EQ(ur_, "s");
        ur_ = s;
        ur_.replace(ur_(1, 1), urv);
        EXPECT_EQ(ur_, "sur");
        // OK ur_.replace(ur_(0, 1), t);
        // OK ur_.replace(ur_(1, 1), tv);
        // OK ur_.replace(ur_(0, 1), r);
        // OK ur_.replace(ur_(1, 1), rv);
    }

    // text::unencoded_rope_view
    // text::text_view

    // text::text
    {
        text::text t_;

        t_ = t;
        t_.replace(t_.end(), t_.end(), "literal");
        EXPECT_EQ(t_, text::text("tliteral"));
        t_ = t;
        t_.replace(t_.begin(), t_.end(), c_str);
        EXPECT_EQ(t_, text::text("c_str"));
        t_ = t;
        t_.replace(t_.end(), t_.end(), str);
        EXPECT_EQ(t_, text::text("tstr"));
        t_ = t;
        t_.replace(t_.end(), t_.end(), sv);
        EXPECT_EQ(t_, text::text("tsv"));
        t_ = t;
        t_.replace(t_.begin(), t_.end(), s);
        EXPECT_EQ(t_, text::text("s"));
        t_ = t;
        t_.replace(t_.end(), t_.end(), ur);
        EXPECT_EQ(t_, text::text("tur"));
        t_ = t;
        t_.replace(t_.begin(), t_.end(), urv);
        EXPECT_EQ(t_, text::text("ur"));
        t_ = t;
        t_.replace(t_.end(), t_.end(), t);
        EXPECT_EQ(t_, text::text("tt"));
        t_ = t;
        t_.replace(t_.begin(), t_.end(), tv);
        EXPECT_EQ(t_, text::text("t"));
        t_ = t;
        t_.replace(t_.end(), t_.end(), r);
        EXPECT_EQ(t_, text::rope("tr"));
        t_ = t;
        t_.replace(t_.begin(), t_.end(), rv);
        EXPECT_EQ(t_, text::rope("r"));
    }

    // text::rope
    {
        text::rope r_;

        r_ = r;
        r_.replace(r_.end(), r_.end(), "literal");
        EXPECT_EQ(r_, text::rope("rliteral"));
        r_ = r;
        r_.replace(r_.begin(), r_.end(), c_str);
        EXPECT_EQ(r_, text::rope("c_str"));
        r_ = r;
        r_.replace(r_.end(), r_.end(), str);
        EXPECT_EQ(r_, text::rope("rstr"));
        r_ = r;
        r_.replace(r_.end(), r_.end(), sv);
        EXPECT_EQ(r_, text::rope("rsv"));
        r_ = r;
        r_.replace(r_.begin(), r_.end(), s);
        EXPECT_EQ(r_, text::rope("s"));
        r_ = r;
        r_.replace(r_.end(), r_.end(), ur);
        EXPECT_EQ(r_, text::rope("rur"));
        r_ = r;
        r_.replace(r_.begin(), r_.end(), urv);
        EXPECT_EQ(r_, text::rope("ur"));
        r_ = r;
        r_.replace(r_.end(), r_.end(), t);
        EXPECT_EQ(r_, text::rope("rt"));
        r_ = r;
        r_.replace(r_.begin(), r_.end(), tv);
        EXPECT_EQ(r_, text::rope("t"));
        r_ = r;
        r_.replace(r_.end(), r_.end(), r);
        EXPECT_EQ(r_, text::rope("rr"));
        r_ = r;
        r_.replace(r_.begin(), r_.end(), rv);
        EXPECT_EQ(r_, text::rope("r"));
    }

    // text::rope_view
}

TEST(common_operations, test_operator_plus)
{
    text::string_view const tv("tv");
    std::string const t("t");
    text::unencoded_rope const r("r");
    text::unencoded_rope_view const rv(r);

    text::unencoded_rope result;

    // OK EXPECT_EQ((result = tv + t), "tvt");
    // OK EXPECT_EQ((result = tv + std::move(t)), "tvt");
    EXPECT_EQ((result = tv + r), "tvr");
    EXPECT_EQ((result = tv + std::move(r)), "tvr");

    // OK EXPECT_EQ((result = t + tv), "ttv");
    EXPECT_EQ((result = t + t), "tt");
    EXPECT_EQ((result = t + std::move(t)), "tt");
    EXPECT_EQ((result = t + r), "tr");
    EXPECT_EQ((result = t + std::move(r)), "tr");
    // OK EXPECT_EQ((result = t + rv), "tr");

    // OK EXPECT_EQ((result = std::move(t) + tv), "ttv");
    EXPECT_EQ((result = std::move(t) + t), "tt");
    EXPECT_EQ((result = std::move(t) + std::move(t)), "tt");
    EXPECT_EQ((result = std::move(t) + r), "tr");
    EXPECT_EQ((result = std::move(t) + std::move(r)), "tr");
    // OK EXPECT_EQ((result = std::move(t) + rv), "tr");

    EXPECT_EQ((result = r + tv), "rtv");
    EXPECT_EQ((result = r + t), "rt");
    EXPECT_EQ((result = r + std::move(t)), "rt");
    EXPECT_EQ((result = r + r), "rr");
    EXPECT_EQ((result = r + std::move(r)), "rr");
    EXPECT_EQ((result = r + rv), "rr");

    EXPECT_EQ((result = std::move(r) + tv), "rtv");
    EXPECT_EQ((result = std::move(r) + t), "rt");
    EXPECT_EQ((result = std::move(r) + std::move(t)), "rt");
    EXPECT_EQ((result = std::move(r) + r), "rr");
    EXPECT_EQ((result = std::move(r) + std::move(r)), "rr");
    EXPECT_EQ((result = std::move(r) + rv), "rr");

    // OK EXPECT_EQ((result = rv + t), "rt");
    // OK EXPECT_EQ((result = rv + std::move(t)), "rt");
    EXPECT_EQ((result = rv + r), "rr");
    EXPECT_EQ((result = rv + std::move(r)), "rr");
}

TEST(common_operations, test_operator_assign)
{
    {
        text::string_view const tv("tv");
        std::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::unencoded_rope result_r;

        EXPECT_EQ((result_r = tv), "tv");
        EXPECT_EQ((result_r = t), "t");
        EXPECT_EQ((result_r = std::move(t)), "t");
        EXPECT_EQ((result_r = r), "r");
        EXPECT_EQ((result_r = std::move(r)), "r");
        EXPECT_EQ((result_r = rv), "r");
    }
}

TEST(common_operations, test_operator_plus_assign)
{
    {
        text::string_view const tv("tv");
        std::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::unencoded_rope result_r;

        EXPECT_EQ((result_r += tv), "tv");
        EXPECT_EQ((result_r += t), "tvt");
        EXPECT_EQ((result_r += std::move(t)), "tvtt");
        EXPECT_EQ((result_r += r), "tvttr");
        EXPECT_EQ((result_r += std::move(r)), "tvttrr");
        EXPECT_EQ((result_r += rv), "tvttrrr");
    }
}

TEST(common_operations, copy_assign)
{
    // Unicode 9, 3.9/D90-D92
    // uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char const utf8[] = {0x4d,
                         char(0xd0),
                         char(0xb0),
                         char(0xe4),
                         char(0xba),
                         char(0x8c),
                         char(0xf0),
                         char(0x90),
                         char(0x8c),
                         char(0x82)};

    text::string_view const tv(utf8, sizeof(utf8) - 1);
    std::string const t(tv);
    text::unencoded_rope const r(tv);

    EXPECT_NO_THROW(text::unencoded_rope_view{tv});
    EXPECT_NO_THROW(text::unencoded_rope_view{t});

    text::unencoded_rope_view const rv(r);

    EXPECT_NO_THROW(text::string_view{tv});
    EXPECT_NO_THROW(text::string_view{t});
    EXPECT_NO_THROW(text::string_view{t});

    EXPECT_NO_THROW(std::string{tv});
    EXPECT_NO_THROW(std::string{t});
    EXPECT_NO_THROW(std::string(std::string(t)));

    EXPECT_NO_THROW(text::unencoded_rope{tv});
    EXPECT_NO_THROW(text::unencoded_rope{t});
    EXPECT_NO_THROW(text::unencoded_rope(std::string(t)));
    EXPECT_NO_THROW(text::unencoded_rope{r});
    EXPECT_NO_THROW(text::unencoded_rope(text::unencoded_rope(r)));
    EXPECT_NO_THROW(text::unencoded_rope{rv});

    EXPECT_NO_THROW(text::unencoded_rope_view{tv});
    EXPECT_NO_THROW(text::unencoded_rope_view{t});
    EXPECT_NO_THROW(text::unencoded_rope_view{t});
    EXPECT_NO_THROW(text::unencoded_rope_view{r});
    EXPECT_NO_THROW(text::unencoded_rope_view{rv});

    EXPECT_NO_THROW(text::string_view() = tv);
    EXPECT_NO_THROW(text::string_view() = t);
    EXPECT_NO_THROW(text::string_view() = t);

    EXPECT_NO_THROW(text::unencoded_rope() = tv);
    EXPECT_NO_THROW(text::unencoded_rope() = t);
    EXPECT_NO_THROW(text::unencoded_rope() = std::string(t));
    EXPECT_NO_THROW(text::unencoded_rope() = r);
    EXPECT_NO_THROW(text::unencoded_rope() = text::unencoded_rope(r));
    EXPECT_NO_THROW(text::unencoded_rope() = rv);

    EXPECT_NO_THROW(text::unencoded_rope_view() = tv);
    EXPECT_NO_THROW(text::unencoded_rope_view() = t);
    EXPECT_NO_THROW(text::unencoded_rope_view() = t);
    EXPECT_NO_THROW(text::unencoded_rope_view() = r);
    EXPECT_NO_THROW(text::unencoded_rope_view() = rv);
}
