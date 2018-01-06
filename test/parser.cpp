#include <boost/text/detail/parser.hpp>

#include "parser_tests.hpp"

#include <gtest/gtest.h>


using namespace boost;

TEST(parser, failures)
{
    text::detail::collation_tailoring_interface callbacks = {
        [](text::detail::cp_seq_t const & reset_, bool before_) {},
        [](text::detail::relation_t const & rel) {},
        [](text::collation_strength strength) {},
        [](text::variable_weighting weighting) {},
        [](text::l2_weight_order order) {},
        [](text::detail::cp_seq_t const & suppressions) {},
        [](std::vector<text::string> &&) {},
        [](text::string const & s) { std::cout << s; },
        [](text::string const & s) { std::cout << s; }};

    {
        text::string_view sv = "";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "a";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& \\ufffd < a";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <* -";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }
    {
        text::string_view sv = "& a <* a-";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }
    {
        text::string_view sv = "& a <* a--";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "&[before a] a < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "&[before 1";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [last implicit] < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [first trailing] < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [last trailing] < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [first foo] < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [first tertiary ignorable";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a < b |";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <*";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a &";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <* \\u0300";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [before 1] a <<* b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& [before 3] a < b";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "&";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    // TODO: Exceptions originating from parse_option().
}
