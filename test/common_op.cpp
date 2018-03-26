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

    string_view sv
    repeated_string_view rsv
    string s
    unencoded_rope ur
    unencoded_rope_view urv

    text_view tv
    repeated_text_view rtv
    text t
    rope r 
    rope_view rv
*/

/*  TODO: Covered operations must be:
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
    EXPECT_NO_THROW(text::string_view(text::string(t)));

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
    EXPECT_NO_THROW(text::unencoded_rope_view(text::string(t)));
    EXPECT_NO_THROW(text::unencoded_rope_view(r));
    EXPECT_NO_THROW(text::unencoded_rope_view(rv));

    EXPECT_NO_THROW(text::string_view() = tv);
    EXPECT_NO_THROW(text::string_view() = t);
    EXPECT_NO_THROW(text::string_view() = text::string(t));

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
    EXPECT_NO_THROW(text::unencoded_rope_view() = text::string(t));
    EXPECT_NO_THROW(text::unencoded_rope_view() = r);
    EXPECT_NO_THROW(text::unencoded_rope_view() = rv);
}
