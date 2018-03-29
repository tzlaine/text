#include <boost/text/string.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>

#include <boost/text/text.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/rope_view.hpp>

#include <gtest/gtest.h>


using namespace boost;

/*  Each of the operations below should work for the following types:

    char (&literal)[N]
    char const * c_str
    std::string str

    repeated_string_view rsv

    string_view sv
    string s
    unencoded_rope ur
    unencoded_rope_view urv

    text_view tv
    text t
    rope r 
    rope_view rv
*/

// TODO: finish writing these.
/*  For each text-defined T above, covered operations must be:
    T x = "str"; // Init from string literal.
    std::vector<T> v = {"str"};  // Init from string literal in initializer_list.

    T (U const &)
    T (U &&)
    T::operator=(U const &)
    T::operator=(U &&)
    operator==(T const &, U const &)
    operator+(T const &, U const &)
    operator<<(std::ostream&, T const &)

    T::begin(), T::end(), T::cbegin(), T::cend()
    T::rbegin(), T::rend(), T::crbegin(), T::crend()
    begin(T const &), end(T const &), cbegin(T const &), cend(T const &)
    rbegin(T const &), rend(T const &), crbegin(T const &), crend(T const &)

    Only for string/unencoded_rope/text/rope:
    operator+=(const &)
    operator+=(&&)
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
    text::string const s = "s";
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
    std::vector<text::string> const s = {"s"};
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
    std::string const str("str");                                              \
                                                                               \
    text::repeated_string_view const rsv("rep", 2);                            \
                                                                               \
    text::string_view const sv("sv");                                          \
    text::string const s("s");                                                 \
    text::unencoded_rope const ur("ur");                                       \
    text::unencoded_rope_view const urv(ur);                                   \
                                                                               \
    text::text const t("t");                                                   \
    text::text_view const tv(t.begin(), t.end());                              \
    text::rope const r("r");                                                   \
    text::rope_view const rv(r)

// TODO: Each of the "OK" cases below needs a compile-fail test.

// T (U const &)
TEST(common_operations, construction)
{
    ONE_OF_EACH();

    // text::repeated_string_view
    {
        text::repeated_string_view const rsv_literal("literal", 2);
        (void)rsv_literal;
        text::repeated_string_view const rsv_c_str(c_str, 2);
        (void)rsv_c_str;
        text::repeated_string_view const rsv_str(str, 2);
        (void)rsv_str;
        text::repeated_string_view const rsv_rsv(rsv);
        (void)rsv_rsv;
        text::repeated_string_view const rsv_sv(sv, 2);
        (void)rsv_sv;
        text::repeated_string_view const rsv_s(s, 2);
        (void)rsv_s;
        // OK text::repeated_string_view const rsv_ur(ur, 2);
        // OK text::repeated_string_view const rsv_urv(urv, 2);
        text::repeated_string_view const rsv_t(t, 2);
        (void)rsv_t;
        text::repeated_string_view const rsv_tv(tv, 2);
        (void)rsv_tv;
        // OK text::repeated_string_view const rsv_r(r, 2);
        // OK text::repeated_string_view const rsv_rv(rv, 2);
    }

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
        text::string_view const sv_t(t);
        (void)sv_t;
        text::string_view const sv_tv(tv);
        (void)sv_tv;
        // OK text::string_view const sv_r(r);
        // OK text::string_view const sv_rv(rv);
    }

    // text::string
    {
        text::string const s_literal("literal");
        (void)s_literal;
        text::string const s_c_str(c_str);
        (void)s_c_str;
        text::string const s_str(str);
        (void)s_str;
        text::string const s_rsv(rsv);
        (void)s_rsv;
        text::string const s_sv(sv);
        (void)s_sv;
        text::string const s_s(s);
        (void)s_s;
        text::string const s_ur(ur);
        (void)s_ur;
        text::string const s_urv(urv);
        (void)s_urv;
        text::string const s_t(t);
        (void)s_t;
        text::string const s_tv(tv);
        (void)s_tv;
        text::string const s_r(r);
        (void)s_r;
        text::string const s_rv(rv);
        (void)s_rv;
    }

    // text::unencoded_rope
    {
        text::unencoded_rope const ur_literal("literal");
        (void)ur_literal;
        text::unencoded_rope const ur_c_str(c_str);
        (void)ur_c_str;
        text::unencoded_rope const ur_str(str);
        (void)ur_str;
        text::unencoded_rope const ur_rsv(rsv);
        (void)ur_rsv;
        text::unencoded_rope const ur_sv(sv);
        (void)ur_sv;
        text::unencoded_rope const ur_s(s);
        (void)ur_s;
        text::unencoded_rope const ur_ur(ur);
        (void)ur_ur;
        text::unencoded_rope const ur_urv(urv);
        (void)ur_urv;
        text::unencoded_rope const ur_t(t);
        (void)ur_t;
        text::unencoded_rope const ur_tv(tv);
        (void)ur_tv;
        text::unencoded_rope const ur_r(r);
        (void)ur_r;
        text::unencoded_rope const ur_rv(rv);
        (void)ur_rv;
    }

    // text::unencoded_rope_view
    {
        text::unencoded_rope_view const urv_literal("literal");
        (void)urv_literal;
        text::unencoded_rope_view const urv_c_str(c_str);
        (void)urv_c_str;
        text::unencoded_rope_view const urv_str(str);
        (void)urv_str;
        text::unencoded_rope_view const urv_rsv(rsv);
        (void)urv_rsv;
        text::unencoded_rope_view const urv_sv(sv);
        (void)urv_sv;
        text::unencoded_rope_view const urv_s(s);
        (void)urv_s;
        text::unencoded_rope_view const urv_ur(ur);
        (void)urv_ur;
        text::unencoded_rope_view const urv_urv(urv);
        (void)urv_urv;
        text::unencoded_rope_view const urv_t(t);
        (void)urv_t;
        text::unencoded_rope_view const urv_tv(tv);
        (void)urv_tv;
        // OK text::unencoded_rope_view const urv_r(r);
        // OK text::unencoded_rope_view const urv_rv(rv);
    }

    // text::text_view
    {
        // OK text::text_view const tv_literal("literal");
        // OK text::text_view const tv_c_str(c_str);
        // OK text::text_view const tv_str(str);
        // OK text::text_view const tv_rsv(rsv);
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
        text::text const t_rsv(rsv);
        (void)t_rsv;
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
        text::rope const r_rsv(rsv);
        (void)r_rsv;
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
        // OK text::rope_view const rv_rsv(rsv);
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
    // TODO
}

// T::operator=(U const &)
TEST(common_operations, assignment_operators)
{
    ONE_OF_EACH();

    // text::repeated_string_view

    // text::string_view
    {
        text::string_view sv_literal;
        sv_literal = "literal";
        text::string_view sv_c_str;
        sv_c_str = c_str;
        text::string_view sv_str;
        sv_str = str;
        // OK text::string_view sv_rsv(rsv);
        text::string_view sv_sv;
        sv_sv = sv;
        text::string_view sv_s;
        sv_s = s;
        // OK text::string_view sv_ur(ur);
        // OK text::string_view sv_urv(urv);
        text::string_view sv_t;
        sv_t = t;
        text::string_view sv_tv;
        sv_tv = tv;
        // OK text::string_view sv_r(r);
        // OK text::string_view sv_rv(rv);
    }

    // text::string
    {
        text::string s_literal;
        s_literal = "literal";
        text::string s_c_str;
        s_c_str = c_str;
        text::string s_str;
        s_str = str;
        text::string s_rsv;
        s_rsv = rsv;
        text::string s_sv;
        s_sv = sv;
        text::string s_s;
        s_s = s;
        text::string s_ur;
        s_ur = ur;
        text::string s_urv;
        s_urv = urv;
        text::string s_t;
        s_t = t;
        text::string s_tv;
        s_tv = tv;
        text::string s_r;
        s_r = r;
        text::string s_rv;
        s_rv = rv;
    }

    // text::unencoded_rope
    {
        text::unencoded_rope ur_literal;
        ur_literal = "literal";
        text::unencoded_rope ur_c_str;
        ur_c_str = c_str;
        text::unencoded_rope ur_str;
        ur_str = str;
        text::unencoded_rope ur_rsv;
        ur_rsv = rsv;
        text::unencoded_rope ur_sv;
        ur_sv = sv;
        text::unencoded_rope ur_s;
        ur_s = s;
        text::unencoded_rope ur_ur;
        ur_ur = ur;
        text::unencoded_rope ur_urv;
        ur_urv = urv;
        text::unencoded_rope ur_t;
        ur_t = t;
        text::unencoded_rope ur_tv;
        ur_tv = tv;
        text::unencoded_rope ur_r;
        ur_r = r;
        text::unencoded_rope ur_rv;
        ur_rv = rv;
    }

    // text::unencoded_rope_view
    {
        text::unencoded_rope_view urv_literal;
        urv_literal = "literal";
        text::unencoded_rope_view urv_c_str;
        urv_c_str = c_str;
        text::unencoded_rope_view urv_str;
// TODO        urv_str = str;
        text::unencoded_rope_view urv_rsv;
        urv_rsv = rsv;
        text::unencoded_rope_view urv_sv;
        urv_sv = sv;
        text::unencoded_rope_view urv_s;
        urv_s = s;
        text::unencoded_rope_view urv_ur;
        urv_ur = ur;
        text::unencoded_rope_view urv_urv;
        urv_urv = urv;
        text::unencoded_rope_view urv_t;
        urv_t = t;
        text::unencoded_rope_view urv_tv;
        urv_tv = tv;
        // OK text::unencoded_rope_view urv_r(r);
        // OK text::unencoded_rope_view urv_rv(rv);
    }

    // text::text_view
    {
        // OK text::text_view tv_literal("literal");
        // OK text::text_view tv_c_str(c_str);
        // OK text::text_view tv_str(str);
        // OK text::text_view tv_rsv(rsv);
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
        text::text t_rsv;
        t_rsv = rsv;
        text::text t_sv;
        t_sv = sv;
        text::text t_s;
        t_s = s;
        text::text t_ur;
        t_ur = ur;
        text::text t_urv;
        t_urv = urv;
        text::text t_t;
        t_t = t;
        text::text t_tv;
        t_tv = tv;
        text::text t_r;
        t_r = r;
        text::text t_rv;
        t_rv = rv;
    }

    // text::rope
    {
        text::rope r_literal;
        r_literal = "literal";
        text::rope r_c_str;
        r_c_str = c_str;
        text::rope r_str;
        r_str = str;
        text::rope r_rsv;
        r_rsv = rsv;
        text::rope r_sv;
        r_sv = sv;
        text::rope r_s;
        r_s = s;
        text::rope r_ur;
        r_ur = ur;
        text::rope r_urv;
        r_urv = urv;
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
        // OK text::rope_view rv_rsv(rsv);
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

// T::operator=(U &&)
TEST(common_operations, move_assignment_operators)
{
//    ONE_OF_EACH();
}

// operator==(T const &, U const &)
TEST(common_operations, equality_comparisons)
{
    ONE_OF_EACH();

    // text::repeated_string_view

    // text::string_view
    {
        text::string_view sv_literal("literal");
        EXPECT_TRUE(sv_literal == "literal");
        EXPECT_TRUE("literal" == sv_literal);
        text::string_view sv_c_str(c_str);
        EXPECT_TRUE(sv_c_str == c_str);
        EXPECT_TRUE(c_str == sv_c_str);
        // OK text::string_view sv_str(str);
        // OK text::string_view sv_rsv(rsv);
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

    // text::string
    {
        text::string s_literal("literal");
        EXPECT_TRUE(s_literal == "literal");
        EXPECT_TRUE("literal" == s_literal);
        text::string s_c_str(c_str);
        EXPECT_TRUE(s_c_str == c_str);
        EXPECT_TRUE(c_str == s_c_str);
        text::string s_str(str);
        EXPECT_TRUE(s_str == str);
        EXPECT_TRUE(str == s_str);
        text::string s_rsv(rsv);
        EXPECT_TRUE(s_rsv == rsv);
        EXPECT_TRUE(rsv == s_rsv);
        text::string s_sv(sv);
        EXPECT_TRUE(s_sv == sv);
        EXPECT_TRUE(sv == s_sv);
        text::string s_s(s);
        EXPECT_TRUE(s_s == s);
        EXPECT_TRUE(s == s_s);
        text::string s_ur(ur);
        EXPECT_TRUE(s_ur == ur);
        EXPECT_TRUE(ur == s_ur);
        text::string s_urv(urv);
        EXPECT_TRUE(s_urv == urv);
        EXPECT_TRUE(urv == s_urv);
        // OK text::string s_t(t);
        // OK text::string s_tv(tv);
        // OK text::string s_r(r);
        // OK text::string s_rv(rv);
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
        text::unencoded_rope ur_rsv(rsv);
        EXPECT_TRUE(ur_rsv == rsv);
        EXPECT_TRUE(rsv == ur_rsv);
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
        // OK text::unencoded_rope_view urv_rsv(rsv);
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
        // OK text::text_view tv_rsv(rsv);
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
        // OK text::text t_rsv(rsv);
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
        // OK text::rope r_rsv(rsv);
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
        // OK text::rope_view rv_rsv(rsv);
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

TEST(common_operations, test_operator_plus)
{
    text::string_view const tv("tv");
    text::repeated_string_view const rtv(tv, 3);
    text::string const t("t");
    text::unencoded_rope const r("r");
    text::unencoded_rope_view const rv(r);

    text::unencoded_rope result;

    EXPECT_EQ((result = tv + tv), "tvtv");
    EXPECT_EQ((result = tv + rtv), "tvtvtvtv");
    EXPECT_EQ((result = tv + t), "tvt");
    EXPECT_EQ((result = tv + std::move(t)), "tvt");
    EXPECT_EQ((result = tv + r), "tvr");
    EXPECT_EQ((result = tv + std::move(r)), "tvr");
    EXPECT_EQ((result = tv + rv), "tvr");

    EXPECT_EQ((result = rtv + tv), "tvtvtvtv");
    EXPECT_EQ((result = rtv + rtv), "tvtvtvtvtvtv");
    EXPECT_EQ((result = rtv + t), "tvtvtvt");
    EXPECT_EQ((result = rtv + std::move(t)), "tvtvtvt");
    EXPECT_EQ((result = rtv + r), "tvtvtvr");
    EXPECT_EQ((result = rtv + std::move(r)), "tvtvtvr");
    EXPECT_EQ((result = rtv + rv), "tvtvtvr");

    EXPECT_EQ((result = t + tv), "ttv");
    EXPECT_EQ((result = t + rtv), "ttvtvtv");
    EXPECT_EQ((result = t + t), "tt");
    EXPECT_EQ((result = t + std::move(t)), "tt");
    EXPECT_EQ((result = t + r), "tr");
    EXPECT_EQ((result = t + std::move(r)), "tr");
    EXPECT_EQ((result = t + rv), "tr");

    EXPECT_EQ((result = std::move(t) + tv), "ttv");
    EXPECT_EQ((result = std::move(t) + rtv), "ttvtvtv");
    EXPECT_EQ((result = std::move(t) + t), "tt");
    EXPECT_EQ((result = std::move(t) + std::move(t)), "tt");
    EXPECT_EQ((result = std::move(t) + r), "tr");
    EXPECT_EQ((result = std::move(t) + std::move(r)), "tr");
    EXPECT_EQ((result = std::move(t) + rv), "tr");

    EXPECT_EQ((result = r + tv), "rtv");
    EXPECT_EQ((result = r + rtv), "rtvtvtv");
    EXPECT_EQ((result = r + t), "rt");
    EXPECT_EQ((result = r + std::move(t)), "rt");
    EXPECT_EQ((result = r + r), "rr");
    EXPECT_EQ((result = r + std::move(r)), "rr");
    EXPECT_EQ((result = r + rv), "rr");

    EXPECT_EQ((result = std::move(r) + tv), "rtv");
    EXPECT_EQ((result = std::move(r) + rtv), "rtvtvtv");
    EXPECT_EQ((result = std::move(r) + t), "rt");
    EXPECT_EQ((result = std::move(r) + std::move(t)), "rt");
    EXPECT_EQ((result = std::move(r) + r), "rr");
    EXPECT_EQ((result = std::move(r) + std::move(r)), "rr");
    EXPECT_EQ((result = std::move(r) + rv), "rr");

    EXPECT_EQ((result = rv + tv), "rtv");
    EXPECT_EQ((result = rv + rtv), "rtvtvtv");
    EXPECT_EQ((result = rv + t), "rt");
    EXPECT_EQ((result = rv + std::move(t)), "rt");
    EXPECT_EQ((result = rv + r), "rr");
    EXPECT_EQ((result = rv + std::move(r)), "rr");
    EXPECT_EQ((result = rv + rv), "rr");
}

TEST(common_operations, test_operator_assign)
{
    {
        text::string_view const tv("tv");
        text::repeated_string_view const rtv(tv, 3);
        text::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::string result_t;

        EXPECT_EQ((result_t = tv), "tv");
        EXPECT_EQ((result_t = rtv), "tvtvtv");
        EXPECT_EQ((result_t = t), "t");
        EXPECT_EQ((result_t = std::move(t)), "t");
    }

    {
        text::string_view const tv("tv");
        text::repeated_string_view const rtv(tv, 3);
        text::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::unencoded_rope result_r;

        EXPECT_EQ((result_r = tv), "tv");
        EXPECT_EQ((result_r = rtv), "tvtvtv");
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
        text::repeated_string_view const rtv(tv, 3);
        text::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::string result_t;

        EXPECT_EQ((result_t += tv), "tv");
        EXPECT_EQ((result_t += rtv), "tvtvtvtv");
        EXPECT_EQ((result_t += t), "tvtvtvtvt");
        EXPECT_EQ((result_t += std::move(t)), "tvtvtvtvtt");
        EXPECT_EQ((result_t += r), "tvtvtvtvttr");
        EXPECT_EQ((result_t += std::move(r)), "tvtvtvtvttrr");
        EXPECT_EQ((result_t += rv), "tvtvtvtvttrrr");
    }

    {
        text::string_view const tv("tv");
        text::repeated_string_view const rtv(tv, 3);
        text::string const t("t");
        text::unencoded_rope const r("r");
        text::unencoded_rope_view const rv(r);

        text::unencoded_rope result_r;

        EXPECT_EQ((result_r += tv), "tv");
        EXPECT_EQ((result_r += rtv), "tvtvtvtv");
        EXPECT_EQ((result_r += t), "tvtvtvtvt");
        EXPECT_EQ((result_r += std::move(t)), "tvtvtvtvtt");
        EXPECT_EQ((result_r += r), "tvtvtvtvttr");
        EXPECT_EQ((result_r += std::move(r)), "tvtvtvtvttrr");
        EXPECT_EQ((result_r += rv), "tvtvtvtvttrrr");
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
    text::repeated_string_view const rtv(tv, 3);
    text::string const t(tv);
    text::unencoded_rope const r(tv);

    EXPECT_NO_THROW(text::unencoded_rope_view(tv));
    EXPECT_NO_THROW(text::unencoded_rope_view(rtv));
    EXPECT_NO_THROW(text::unencoded_rope_view(t));

    text::unencoded_rope_view const rv(r);

    EXPECT_NO_THROW(text::string_view(tv));
    EXPECT_NO_THROW(text::string_view(t));
    EXPECT_NO_THROW(text::string_view(t));

    EXPECT_NO_THROW(text::repeated_string_view(tv, 2));
    EXPECT_NO_THROW(text::repeated_string_view(rtv));

    EXPECT_NO_THROW(text::string(tv));
    EXPECT_NO_THROW(text::string(rtv));
    EXPECT_NO_THROW(text::string(t));
    EXPECT_NO_THROW(text::string(text::string(t)));

    EXPECT_NO_THROW(text::unencoded_rope(tv));
    EXPECT_NO_THROW(text::unencoded_rope(rtv));
    EXPECT_NO_THROW(text::unencoded_rope(t));
    EXPECT_NO_THROW(text::unencoded_rope(text::string(t)));
    EXPECT_NO_THROW(text::unencoded_rope(r));
    EXPECT_NO_THROW(text::unencoded_rope(text::unencoded_rope(r)));
    EXPECT_NO_THROW(text::unencoded_rope(rv));

    EXPECT_NO_THROW(text::unencoded_rope_view(tv));
    EXPECT_NO_THROW(text::unencoded_rope_view(rtv));
    EXPECT_NO_THROW(text::unencoded_rope_view(t));
    EXPECT_NO_THROW(text::unencoded_rope_view(t));
    EXPECT_NO_THROW(text::unencoded_rope_view(r));
    EXPECT_NO_THROW(text::unencoded_rope_view(rv));

    EXPECT_NO_THROW(text::string_view() = tv);
    EXPECT_NO_THROW(text::string_view() = t);
    EXPECT_NO_THROW(text::string_view() = t);

    EXPECT_NO_THROW(text::repeated_string_view() = rtv);

    EXPECT_NO_THROW(text::string() = tv);
    EXPECT_NO_THROW(text::string() = rtv);
    EXPECT_NO_THROW(text::string() = t);
    EXPECT_NO_THROW(text::string() = text::string(t));

    EXPECT_NO_THROW(text::unencoded_rope() = tv);
    EXPECT_NO_THROW(text::unencoded_rope() = rtv);
    EXPECT_NO_THROW(text::unencoded_rope() = t);
    EXPECT_NO_THROW(text::unencoded_rope() = text::string(t));
    EXPECT_NO_THROW(text::unencoded_rope() = r);
    EXPECT_NO_THROW(text::unencoded_rope() = text::unencoded_rope(r));
    EXPECT_NO_THROW(text::unencoded_rope() = rv);

    EXPECT_NO_THROW(text::unencoded_rope_view() = tv);
    EXPECT_NO_THROW(text::unencoded_rope_view() = rtv);
    EXPECT_NO_THROW(text::unencoded_rope_view() = t);
    EXPECT_NO_THROW(text::unencoded_rope_view() = t);
    EXPECT_NO_THROW(text::unencoded_rope_view() = r);
    EXPECT_NO_THROW(text::unencoded_rope_view() = rv);
}
