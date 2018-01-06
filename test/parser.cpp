#include <boost/text/detail/parser.hpp>

#include "parser_tests.hpp"

#include <gtest/gtest.h>


using namespace boost;

TEST(parser, exceptions)
{
    text::detail::collation_tailoring_interface callbacks = {
        [](text::detail::cp_seq_t const & reset_, bool before_) {},
        [](text::detail::relation_t const & rel) {},
        [](text::collation_strength strength) {},
        [](text::variable_weighting weighting) {},
        [](text::l2_weight_order order) {},
        [](text::detail::cp_seq_t const & suppressions) {},
        [](std::vector<text::string> && reorderings) {},
        [](text::string const & s) { std::cout << s; },
        [](text::string const & s) { std::cout << s; }};

    // Exceptions produced by parse() itself.
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

    // Exceptions produced by parse_rule()
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

    // Exceptions produced by parse_option().
    {
        text::string_view sv = "[]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[|]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[import foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[import";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[optimize [foo";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[optimize [foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[optimize [";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[optimize";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[optimize []";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[suppressContractions [foo";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[suppressContractions [foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[suppressContractions";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[suppressContractions [";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[suppressContractions []";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[strength]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[strength";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[strength foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[strength I";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[alternate]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[alternate";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[alternate foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[alternate shifted";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[backwards]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[backwards";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[backwards 1]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[backwards 2";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder foo] [reorder bar]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }
}

TEST(parser, options)
{
    {
        text::detail::cp_seq_t result;
        text::detail::cp_seq_t const expected = {'a', 'b', 'c'};
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [&result](text::detail::cp_seq_t const & suppressions) {
                result = suppressions;
            },
            [](std::vector<text::string> && reorderings) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv = "[suppressContractions [abc]]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, expected);
    }

    {
        text::collation_strength result;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [&result](text::collation_strength strength) { result = strength; },
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::string> && reorderings) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[strength 1]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::collation_strength::primary);

        sv = "[strength 2]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::collation_strength::secondary);

        sv = "[strength 3]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::collation_strength::tertiary);

        sv = "[strength 4]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::collation_strength::quaternary);

        sv = "[strength I]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::collation_strength::identical);
    }

    {
        text::variable_weighting result;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [&result](text::variable_weighting weighting) {
                result = weighting;
            },
            [](text::l2_weight_order order) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::string> && reorderings) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[alternate non-ignorable]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::variable_weighting::non_ignorable);

        sv = "[alternate shifted]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::variable_weighting::shifted);
    }

    {
        text::l2_weight_order result;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [&result](text::l2_weight_order order) { result = order; },
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::string> && reorderings) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[backwards 2]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::l2_weight_order::backward);
    }

    {
        std::vector<text::string> result;
        std::vector<text::string> const expected = {"foo", "others", "bar"};
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [&result](std::vector<text::string> && reorderings) {
                result = std::move(reorderings);
            },
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv = "[reorder foo others bar]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, expected);
    }
}
