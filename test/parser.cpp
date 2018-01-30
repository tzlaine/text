#include <boost/text/detail/parser.hpp>

#include "parser_tests.hpp"

#include <gtest/gtest.h>


using namespace boost;

std::ostream &
dump(std::ostream & os, text::detail::collation_element cce)
{
    os << std::hex << "{" << cce.l1_ << " " << cce.l2_ << " "
       << (uint32_t)cce.l3_ << " " << cce.l4_ << "}";
    return os;
}

namespace std {
    ostream &
    operator<<(ostream & os, vector<text::detail::reorder_group> const & groups)
    {
        os << "\n----------------------------------------\n";
        for (auto group : groups) {
            os << group.name_ << " ";
            ::dump(os, group.first_);
            os << " ";
            ::dump(os, group.first_);
            os << " ";
        }
        os << "\n----------------------------------------\n";
        return os;
    }
}

TEST(parser, exceptions)
{
    text::detail::collation_tailoring_interface callbacks = {
        [](text::detail::cp_seq_t const & reset_, bool before_) {},
        [](text::detail::relation_t const & rel) {},
        [](text::collation_strength strength) {},
        [](text::variable_weighting weighting) {},
        [](text::l2_weight_order order) {},
        [](text::case_level_t case_level) {},
        [](text::case_first_t case_first) {},
        [](text::detail::cp_seq_t const & suppressions) {},
        [](std::vector<text::detail::reorder_group> const & reorder_groups) {},
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
        text::string_view sv = "& a < \\ufffd";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <* \\ufffd";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <* \\ufffc-\\ufffd";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "& a <* \\ufffc-\\U00010000";
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
        text::string_view sv = "[caseLevel]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseLevel";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseLevel foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseLevel on";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseFirst]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseFirst";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseFirst foo]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[caseFirst uppser";
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
        text::string_view sv = "[reorder space] [reorder punct]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder Common]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder Inherited]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder Zzzz others]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder Grek Grek]";
        EXPECT_THROW(
            text::detail::parse(
                sv.begin(), sv.end(), callbacks, "<test-string>"),
            text::parse_error);
    }

    {
        text::string_view sv = "[reorder rando_calrissian]";
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
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [&result](text::detail::cp_seq_t const & suppressions) {
                result = suppressions;
            },
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv = "[suppressContractions [abc]]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, expected);
    }

    {
        text::collation_strength result = text::collation_strength::identical;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [&result](text::collation_strength strength) { result = strength; },
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
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
        text::variable_weighting result = text::variable_weighting::shifted;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [&result](text::variable_weighting weighting) {
                result = weighting;
            },
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
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
        text::l2_weight_order result = text::l2_weight_order::forward;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [&result](text::l2_weight_order order) { result = order; },
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[backwards 2]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::l2_weight_order::backward);
    }

    {
        text::case_level_t result = text::case_level_t::off;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [&result](text::case_level_t case_level) { result = case_level; },
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[caseLevel on]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::case_level_t::on);

        sv = "[caseLevel off]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::case_level_t::off);
    }

    {
        text::case_first_t result = text::case_first_t::off;
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [&result](text::case_first_t case_first) { result = case_first; },
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const &
                   reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        sv = "[caseFirst upper]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::case_first_t::upper);

        sv = "[caseFirst lower]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::case_first_t::lower);

        sv = "[caseFirst off]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, text::case_first_t::off);
    }

    {
        std::vector<text::detail::reorder_group> result;
        std::vector<text::detail::reorder_group> const expected = {
            {"space",
             {0x03020200, 0x0500, 0x0500},
             {0x05060200, 0x0500, 0x0500},
             true,
             false},
            {"currency",
             {0x0D700200, 0x0500, 0x0500},
             {0x0E020200, 0x0500, 0x0500},
             true,
             false},
            {"digit",
             {0x0E020200, 0x0500, 0x0500},
             {0x26020200, 0x0500, 0x0500},
             true,
             false},
            {"symbol",
             {0x0C020200, 0x0500, 0x0500},
             {0x0D700200, 0x0500, 0x0500},
             true,
             false},
            {"Latn",
             {0x28020200, 0x0500, 0x0500},
             {0x5D020200, 0x0500, 0x0500},
             true,
             false},
            {"Grek",
             {0x5F040200, 0x0500, 0x0500},
             {0x5F600200, 0x0500, 0x0500},
             false,
             true},
            {"Copt",
             {0x5F600200, 0x0500, 0x0500},
             {0x60040200, 0x0500, 0x0500},
             false,
             true},
            {"Cyrl",
             {0x60040200, 0x0500, 0x0500},
             {0x61040200, 0x0500, 0x0500},
             true,
             true},
            {"Glag",
             {0x61040200, 0x0500, 0x0500},
             {0x61640200, 0x0500, 0x0500},
             false,
             true},
            {"Perm",
             {0x61640200, 0x0500, 0x0500},
             {0x62040200, 0x0500, 0x0500},
             false,
             true},
            {"Geor",
             {0x62040200, 0x0500, 0x0500},
             {0x63040200, 0x0500, 0x0500},
             true,
             true},
            {"Armn",
             {0x63040200, 0x0500, 0x0500},
             {0x64040200, 0x0500, 0x0500},
             true,
             true},
            {"Hebr",
             {0x64040200, 0x0500, 0x0500},
             {0x64320200, 0x0500, 0x0500},
             false,
             true},
            {"Phnx",
             {0x64320200, 0x0500, 0x0500},
             {0x64330200, 0x0500, 0x0500},
             false,
             true},
            {"Samr",
             {0x64330200, 0x0500, 0x0500},
             {0x65040200, 0x0500, 0x0500},
             false,
             true},
            {"Arab",
             {0x65040200, 0x0500, 0x0500},
             {0x66040200, 0x0500, 0x0500},
             true,
             true},
            {"Syrc",
             {0x66040200, 0x0500, 0x0500},
             {0x661B0200, 0x0500, 0x0500},
             false,
             true},
            {"Mand",
             {0x661B0200, 0x0500, 0x0500},
             {0x661C0200, 0x0500, 0x0500},
             false,
             true},
            {"Thaa",
             {0x661C0200, 0x0500, 0x0500},
             {0x66820200, 0x0500, 0x0500},
             false,
             true},
            {"Nkoo",
             {0x66820200, 0x0500, 0x0500},
             {0x668D0200, 0x0500, 0x0500},
             false,
             true},
            {"Tfng",
             {0x668D0200, 0x0500, 0x0500},
             {0x668F0200, 0x0500, 0x0500},
             false,
             true},
            {"Ethi",
             {0x668F0200, 0x0500, 0x0500},
             {0x67040200, 0x0500, 0x0500},
             false,
             true},
            {"Deva",
             {0x67040200, 0x0500, 0x0500},
             {0x68040200, 0x0500, 0x0500},
             true,
             true},
            {"Beng",
             {0x68040200, 0x0500, 0x0500},
             {0x69040200, 0x0500, 0x0500},
             true,
             true},
            {"Guru",
             {0x69040200, 0x0500, 0x0500},
             {0x6A040200, 0x0500, 0x0500},
             true,
             true},
            {"Gujr",
             {0x6A040200, 0x0500, 0x0500},
             {0x6B040200, 0x0500, 0x0500},
             true,
             true},
            {"Orya",
             {0x6B040200, 0x0500, 0x0500},
             {0x6C040200, 0x0500, 0x0500},
             true,
             true},
            {"Taml",
             {0x6C040200, 0x0500, 0x0500},
             {0x6D040200, 0x0500, 0x0500},
             true,
             true},
            {"Telu",
             {0x6D040200, 0x0500, 0x0500},
             {0x6E040200, 0x0500, 0x0500},
             true,
             true},
            {"Knda",
             {0x6E040200, 0x0500, 0x0500},
             {0x6F040200, 0x0500, 0x0500},
             true,
             true},
            {"Mlym",
             {0x6F040200, 0x0500, 0x0500},
             {0x70040200, 0x0500, 0x0500},
             true,
             true},
            {"Sinh",
             {0x70040200, 0x0500, 0x0500},
             {0x70A00200, 0x0500, 0x0500},
             false,
             true},
            {"Mtei",
             {0x70A00200, 0x0500, 0x0500},
             {0x70A20200, 0x0500, 0x0500},
             false,
             true},
            {"Sylo",
             {0x70A20200, 0x0500, 0x0500},
             {0x70A40200, 0x0500, 0x0500},
             false,
             true},
            {"Saur",
             {0x70A40200, 0x0500, 0x0500},
             {0x70A60200, 0x0500, 0x0500},
             false,
             true},
            {"Kthi",
             {0x70A60200, 0x0500, 0x0500},
             {0x70A80200, 0x0500, 0x0500},
             false,
             true},
            {"Mahj",
             {0x70A80200, 0x0500, 0x0500},
             {0x70AA0200, 0x0500, 0x0500},
             false,
             true},
            {"Shrd",
             {0x70AA0200, 0x0500, 0x0500},
             {0x70AC0200, 0x0500, 0x0500},
             false,
             true},
            {"Khoj",
             {0x70AC0200, 0x0500, 0x0500},
             {0x70AE0200, 0x0500, 0x0500},
             false,
             true},
            {"Sind",
             {0x70AE0200, 0x0500, 0x0500},
             {0x70B00200, 0x0500, 0x0500},
             false,
             true},
            {"Mult",
             {0x70B00200, 0x0500, 0x0500},
             {0x70B20200, 0x0500, 0x0500},
             false,
             true},
            {"Gran",
             {0x70B20200, 0x0500, 0x0500},
             {0x70B40200, 0x0500, 0x0500},
             false,
             true},
            {"Newa",
             {0x70B40200, 0x0500, 0x0500},
             {0x70B60200, 0x0500, 0x0500},
             false,
             true},
            {"Tirh",
             {0x70B60200, 0x0500, 0x0500},
             {0x70B80200, 0x0500, 0x0500},
             false,
             true},
            {"Sidd",
             {0x70B80200, 0x0500, 0x0500},
             {0x70C80200, 0x0500, 0x0500},
             false,
             true},
            {"Modi",
             {0x70C80200, 0x0500, 0x0500},
             {0x70CA0200, 0x0500, 0x0500},
             false,
             true},
            {"Takr",
             {0x70CA0200, 0x0500, 0x0500},
             {0x70CC0200, 0x0500, 0x0500},
             false,
             true},
            {"Ahom",
             {0x70CC0200, 0x0500, 0x0500},
             {0x70D50200, 0x0500, 0x0500},
             false,
             true},
            {"Gonm",
             {0x70D50200, 0x0500, 0x0500},
             {0x70DA0200, 0x0500, 0x0500},
             false,
             true},
            {"Sund",
             {0x70DA0200, 0x0500, 0x0500},
             {0x70E70200, 0x0500, 0x0500},
             false,
             true},
            {"Brah",
             {0x70E70200, 0x0500, 0x0500},
             {0x70E90200, 0x0500, 0x0500},
             false,
             true},
            {"Khar",
             {0x70E90200, 0x0500, 0x0500},
             {0x70EB0200, 0x0500, 0x0500},
             false,
             true},
            {"Bhks",
             {0x70EB0200, 0x0500, 0x0500},
             {0x71040200, 0x0500, 0x0500},
             false,
             true},
            {"Thai",
             {0x71040200, 0x0500, 0x0500},
             {0x72040200, 0x0500, 0x0500},
             true,
             true},
            {"Laoo",
             {0x72040200, 0x0500, 0x0500},
             {0x72660200, 0x0500, 0x0500},
             false,
             true},
            {"Tavt",
             {0x72660200, 0x0500, 0x0500},
             {0x73040200, 0x0500, 0x0500},
             false,
             true},
            {"Tibt",
             {0x73040200, 0x0500, 0x0500},
             {0x73C60200, 0x0500, 0x0500},
             false,
             true},
            {"Zanb",
             {0x73C60200, 0x0500, 0x0500},
             {0x73D10200, 0x0500, 0x0500},
             false,
             true},
            {"Soyo",
             {0x73D10200, 0x0500, 0x0500},
             {0x74040200, 0x0500, 0x0500},
             false,
             true},
            {"Marc",
             {0x74040200, 0x0500, 0x0500},
             {0x74060200, 0x0500, 0x0500},
             false,
             true},
            {"Lepc",
             {0x74060200, 0x0500, 0x0500},
             {0x74080200, 0x0500, 0x0500},
             false,
             true},
            {"Phag",
             {0x74080200, 0x0500, 0x0500},
             {0x740A0200, 0x0500, 0x0500},
             false,
             true},
            {"Limb",
             {0x740A0200, 0x0500, 0x0500},
             {0x74190200, 0x0500, 0x0500},
             false,
             true},
            {"Tglg",
             {0x74190200, 0x0500, 0x0500},
             {0x741A0200, 0x0500, 0x0500},
             false,
             true},
            {"Hano",
             {0x741A0200, 0x0500, 0x0500},
             {0x741B0200, 0x0500, 0x0500},
             false,
             true},
            {"Buhd",
             {0x741B0200, 0x0500, 0x0500},
             {0x741C0200, 0x0500, 0x0500},
             false,
             true},
            {"Tagb",
             {0x741C0200, 0x0500, 0x0500},
             {0x741D0200, 0x0500, 0x0500},
             false,
             true},
            {"Bugi",
             {0x741D0200, 0x0500, 0x0500},
             {0x741E0200, 0x0500, 0x0500},
             false,
             true},
            {"Batk",
             {0x741E0200, 0x0500, 0x0500},
             {0x744B0200, 0x0500, 0x0500},
             false,
             true},
            {"Rjng",
             {0x744B0200, 0x0500, 0x0500},
             {0x744D0200, 0x0500, 0x0500},
             false,
             true},
            {"Kali",
             {0x744D0200, 0x0500, 0x0500},
             {0x744E0200, 0x0500, 0x0500},
             false,
             true},
            {"Mymr",
             {0x744E0200, 0x0500, 0x0500},
             {0x745D0200, 0x0500, 0x0500},
             false,
             true},
            {"Cakm",
             {0x745D0200, 0x0500, 0x0500},
             {0x745F0200, 0x0500, 0x0500},
             false,
             true},
            {"Khmr",
             {0x745F0200, 0x0500, 0x0500},
             {0x75040200, 0x0500, 0x0500},
             false,
             true},
            {"Tale",
             {0x75040200, 0x0500, 0x0500},
             {0x75050200, 0x0500, 0x0500},
             false,
             true},
            {"Talu",
             {0x75050200, 0x0500, 0x0500},
             {0x75120200, 0x0500, 0x0500},
             false,
             true},
            {"Lana",
             {0x75120200, 0x0500, 0x0500},
             {0x75260200, 0x0500, 0x0500},
             false,
             true},
            {"Cham",
             {0x75260200, 0x0500, 0x0500},
             {0x75280200, 0x0500, 0x0500},
             false,
             true},
            {"Bali",
             {0x75280200, 0x0500, 0x0500},
             {0x752A0200, 0x0500, 0x0500},
             false,
             true},
            {"Java",
             {0x752A0200, 0x0500, 0x0500},
             {0x75300200, 0x0500, 0x0500},
             false,
             true},
            {"Mong",
             {0x75300200, 0x0500, 0x0500},
             {0x75340200, 0x0500, 0x0500},
             false,
             true},
            {"Olck",
             {0x75340200, 0x0500, 0x0500},
             {0x75360200, 0x0500, 0x0500},
             false,
             true},
            {"Cher",
             {0x75360200, 0x0500, 0x0500},
             {0x75E40200, 0x0500, 0x0500},
             false,
             true},
            {"Osge",
             {0x75E40200, 0x0500, 0x0500},
             {0x75E60200, 0x0500, 0x0500},
             false,
             true},
            {"Cans",
             {0x75E60200, 0x0500, 0x0500},
             {0x75FA0200, 0x0500, 0x0500},
             false,
             true},
            {"Ogam",
             {0x75FA0200, 0x0500, 0x0500},
             {0x76040200, 0x0500, 0x0500},
             false,
             true},
            {"Runr",
             {0x76040200, 0x0500, 0x0500},
             {0x763C0200, 0x0500, 0x0500},
             false,
             true},
            {"Hung",
             {0x763C0200, 0x0500, 0x0500},
             {0x76900200, 0x0500, 0x0500},
             false,
             true},
            {"Orkh",
             {0x76900200, 0x0500, 0x0500},
             {0x77040200, 0x0500, 0x0500},
             false,
             true},
            {"Vaii",
             {0x77040200, 0x0500, 0x0500},
             {0x774C0200, 0x0500, 0x0500},
             false,
             true},
            {"Bamu",
             {0x774C0200, 0x0500, 0x0500},
             {0x775E0200, 0x0500, 0x0500},
             false,
             true},
            {"Bass",
             {0x775E0200, 0x0500, 0x0500},
             {0x775F0200, 0x0500, 0x0500},
             false,
             true},
            {"Mend",
             {0x775F0200, 0x0500, 0x0500},
             {0x77650200, 0x0500, 0x0500},
             false,
             true},
            {"Adlm",
             {0x77650200, 0x0500, 0x0500},
             {0x78040200, 0x0500, 0x0500},
             false,
             true},
            {"Hang",
             {0x78040200, 0x0500, 0x0500},
             {0x79040200, 0x0500, 0x0500},
             true,
             true},
            {"Hira",
             {0x79040200, 0x0500, 0x0500},
             {0x7A040200, 0x0500, 0x0500},
             true,
             true},
            {"Bopo",
             {0x7A040200, 0x0500, 0x0500},
             {0x7A6C0200, 0x0500, 0x0500},
             false,
             true},
            {"Yiii",
             {0x7A6C0200, 0x0500, 0x0500},
             {0x7A8D0200, 0x0500, 0x0500},
             false,
             true},
            {"Lisu",
             {0x7A8D0200, 0x0500, 0x0500},
             {0x7A8F0200, 0x0500, 0x0500},
             false,
             true},
            {"Plrd",
             {0x7A8F0200, 0x0500, 0x0500},
             {0x7AA10200, 0x0500, 0x0500},
             false,
             true},
            {"Wara",
             {0x7AA10200, 0x0500, 0x0500},
             {0x7AE30200, 0x0500, 0x0500},
             false,
             true},
            {"Pauc",
             {0x7AE30200, 0x0500, 0x0500},
             {0x7AE50200, 0x0500, 0x0500},
             false,
             true},
            {"Hmng",
             {0x7AE50200, 0x0500, 0x0500},
             {0x7AE80200, 0x0500, 0x0500},
             false,
             true},
            {"Lyci",
             {0x7AE80200, 0x0500, 0x0500},
             {0x7AE90200, 0x0500, 0x0500},
             false,
             true},
            {"Cari",
             {0x7AE90200, 0x0500, 0x0500},
             {0x7AEB0200, 0x0500, 0x0500},
             false,
             true},
            {"Lydi",
             {0x7AEB0200, 0x0500, 0x0500},
             {0x7AEC0200, 0x0500, 0x0500},
             false,
             true},
            {"Ital",
             {0x7AEC0200, 0x0500, 0x0500},
             {0x7AED0200, 0x0500, 0x0500},
             false,
             true},
            {"Goth",
             {0x7AED0200, 0x0500, 0x0500},
             {0x7B040200, 0x0500, 0x0500},
             false,
             true},
            {"Dsrt",
             {0x7B040200, 0x0500, 0x0500},
             {0x7B560200, 0x0500, 0x0500},
             false,
             true},
            {"Shaw",
             {0x7B560200, 0x0500, 0x0500},
             {0x7B580200, 0x0500, 0x0500},
             false,
             true},
            {"Dupl",
             {0x7B580200, 0x0500, 0x0500},
             {0x7B5C0200, 0x0500, 0x0500},
             false,
             true},
            {"Osma",
             {0x7B5C0200, 0x0500, 0x0500},
             {0x7B5D0200, 0x0500, 0x0500},
             false,
             true},
            {"Elba",
             {0x7B5D0200, 0x0500, 0x0500},
             {0x7B5F0200, 0x0500, 0x0500},
             false,
             true},
            {"Aghb",
             {0x7B5F0200, 0x0500, 0x0500},
             {0x7B610200, 0x0500, 0x0500},
             false,
             true},
            {"Sora",
             {0x7B610200, 0x0500, 0x0500},
             {0x7B620200, 0x0500, 0x0500},
             false,
             true},
            {"Mroo",
             {0x7B620200, 0x0500, 0x0500},
             {0x7B630200, 0x0500, 0x0500},
             false,
             true},
            {"Linb",
             {0x7B630200, 0x0500, 0x0500},
             {0x7B690200, 0x0500, 0x0500},
             false,
             true},
            {"Lina",
             {0x7B690200, 0x0500, 0x0500},
             {0x7B730200, 0x0500, 0x0500},
             false,
             true},
            {"Cprt",
             {0x7B730200, 0x0500, 0x0500},
             {0x7B750200, 0x0500, 0x0500},
             false,
             true},
            {"Sarb",
             {0x7B750200, 0x0500, 0x0500},
             {0x7B760200, 0x0500, 0x0500},
             false,
             true},
            {"Narb",
             {0x7B760200, 0x0500, 0x0500},
             {0x7B770200, 0x0500, 0x0500},
             false,
             true},
            {"Avst",
             {0x7B770200, 0x0500, 0x0500},
             {0x7B7D0200, 0x0500, 0x0500},
             false,
             true},
            {"Palm",
             {0x7B7D0200, 0x0500, 0x0500},
             {0x7B820200, 0x0500, 0x0500},
             false,
             true},
            {"Nbat",
             {0x7B820200, 0x0500, 0x0500},
             {0x7B9D0200, 0x0500, 0x0500},
             false,
             true},
            {"Hatr",
             {0x7B9D0200, 0x0500, 0x0500},
             {0x7B9E0200, 0x0500, 0x0500},
             false,
             true},
            {"Armi",
             {0x7B9E0200, 0x0500, 0x0500},
             {0x7B9F0200, 0x0500, 0x0500},
             false,
             true},
            {"Prti",
             {0x7B9F0200, 0x0500, 0x0500},
             {0x7BA00200, 0x0500, 0x0500},
             false,
             true},
            {"Phli",
             {0x7BA00200, 0x0500, 0x0500},
             {0x7BA10200, 0x0500, 0x0500},
             false,
             true},
            {"Phlp",
             {0x7BA10200, 0x0500, 0x0500},
             {0x7BA20200, 0x0500, 0x0500},
             false,
             true},
            {"Mani",
             {0x7BA20200, 0x0500, 0x0500},
             {0x7BA70200, 0x0500, 0x0500},
             false,
             true},
            {"Ugar",
             {0x7BA70200, 0x0500, 0x0500},
             {0x7BA80200, 0x0500, 0x0500},
             false,
             true},
            {"Xpeo",
             {0x7BA80200, 0x0500, 0x0500},
             {0x7BAA0200, 0x0500, 0x0500},
             false,
             true},
            {"Xsux",
             {0x7BAA0200, 0x0500, 0x0500},
             {0x7BC90200, 0x0500, 0x0500},
             false,
             true},
            {"Egyp",
             {0x7BC90200, 0x0500, 0x0500},
             {0x7C040200, 0x0500, 0x0500},
             false,
             true},
            {"Merc",
             {0x7C040200, 0x0500, 0x0500},
             {0x7C350200, 0x0500, 0x0500},
             false,
             true},
            {"Hluw",
             {0x7C350200, 0x0500, 0x0500},
             {0x7C460200, 0x0500, 0x0500},
             false,
             true},
            {"Tang",
             {0x7C460200, 0x0500, 0x0500},
             {0x7C7D0200, 0x0500, 0x0500},
             false,
             true},
            {"Nshu",
             {0x7C7D0200, 0x0500, 0x0500},
             {0x7D020200, 0x0500, 0x0500},
             false,
             true},
            {"Hani",
             {0x7D020200, 0x0500, 0x0500},
             {0xe5000000, 0x0500, 0x0500},
             true,
             false},
            {"punct",
             {0x05060200, 0x0500, 0x0500},
             {0x0C020200, 0x0500, 0x0500},
             true,
             false},
        };
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [&result](std::vector<text::detail::reorder_group> const &
                          reorder_groups) { result = reorder_groups; },
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv = "[reorder symbol others punct]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, expected);
    }

    {
        std::vector<text::detail::reorder_group> result;
        std::vector<text::detail::reorder_group> const expected = {
            {"space",
             {0x03020200, 0x0500, 0x0500},
             {0x05060200, 0x0500, 0x0500},
             true,
             false},
            {"currency",
             {0x0D700200, 0x0500, 0x0500},
             {0x0E020200, 0x0500, 0x0500},
             true,
             false},
            {"digit",
             {0x0E020200, 0x0500, 0x0500},
             {0x26020200, 0x0500, 0x0500},
             true,
             false},
            {"symbol",
             {0x0C020200, 0x0500, 0x0500},
             {0x0D700200, 0x0500, 0x0500},
             true,
             false},
            {"punct",
             {0x05060200, 0x0500, 0x0500},
             {0x0C020200, 0x0500, 0x0500},
             true,
             false},
            {"Grek",
             {0x5F040200, 0x0500, 0x0500},
             {0x5F600200, 0x0500, 0x0500},
             false,
             true},
            {"Latn",
             {0x28020200, 0x0500, 0x0500},
             {0x5D020200, 0x0500, 0x0500},
             true,
             false},
            {"Copt",
             {0x5F600200, 0x0500, 0x0500},
             {0x60040200, 0x0500, 0x0500},
             false,
             true},
            {"Cyrl",
             {0x60040200, 0x0500, 0x0500},
             {0x61040200, 0x0500, 0x0500},
             true,
             true},
            {"Glag",
             {0x61040200, 0x0500, 0x0500},
             {0x61640200, 0x0500, 0x0500},
             false,
             true},
            {"Perm",
             {0x61640200, 0x0500, 0x0500},
             {0x62040200, 0x0500, 0x0500},
             false,
             true},
            {"Geor",
             {0x62040200, 0x0500, 0x0500},
             {0x63040200, 0x0500, 0x0500},
             true,
             true},
            {"Armn",
             {0x63040200, 0x0500, 0x0500},
             {0x64040200, 0x0500, 0x0500},
             true,
             true},
            {"Hebr",
             {0x64040200, 0x0500, 0x0500},
             {0x64320200, 0x0500, 0x0500},
             false,
             true},
            {"Phnx",
             {0x64320200, 0x0500, 0x0500},
             {0x64330200, 0x0500, 0x0500},
             false,
             true},
            {"Samr",
             {0x64330200, 0x0500, 0x0500},
             {0x65040200, 0x0500, 0x0500},
             false,
             true},
            {"Arab",
             {0x65040200, 0x0500, 0x0500},
             {0x66040200, 0x0500, 0x0500},
             true,
             true},
            {"Syrc",
             {0x66040200, 0x0500, 0x0500},
             {0x661B0200, 0x0500, 0x0500},
             false,
             true},
            {"Mand",
             {0x661B0200, 0x0500, 0x0500},
             {0x661C0200, 0x0500, 0x0500},
             false,
             true},
            {"Thaa",
             {0x661C0200, 0x0500, 0x0500},
             {0x66820200, 0x0500, 0x0500},
             false,
             true},
            {"Nkoo",
             {0x66820200, 0x0500, 0x0500},
             {0x668D0200, 0x0500, 0x0500},
             false,
             true},
            {"Tfng",
             {0x668D0200, 0x0500, 0x0500},
             {0x668F0200, 0x0500, 0x0500},
             false,
             true},
            {"Ethi",
             {0x668F0200, 0x0500, 0x0500},
             {0x67040200, 0x0500, 0x0500},
             false,
             true},
            {"Deva",
             {0x67040200, 0x0500, 0x0500},
             {0x68040200, 0x0500, 0x0500},
             true,
             true},
            {"Beng",
             {0x68040200, 0x0500, 0x0500},
             {0x69040200, 0x0500, 0x0500},
             true,
             true},
            {"Guru",
             {0x69040200, 0x0500, 0x0500},
             {0x6A040200, 0x0500, 0x0500},
             true,
             true},
            {"Gujr",
             {0x6A040200, 0x0500, 0x0500},
             {0x6B040200, 0x0500, 0x0500},
             true,
             true},
            {"Orya",
             {0x6B040200, 0x0500, 0x0500},
             {0x6C040200, 0x0500, 0x0500},
             true,
             true},
            {"Taml",
             {0x6C040200, 0x0500, 0x0500},
             {0x6D040200, 0x0500, 0x0500},
             true,
             true},
            {"Telu",
             {0x6D040200, 0x0500, 0x0500},
             {0x6E040200, 0x0500, 0x0500},
             true,
             true},
            {"Knda",
             {0x6E040200, 0x0500, 0x0500},
             {0x6F040200, 0x0500, 0x0500},
             true,
             true},
            {"Mlym",
             {0x6F040200, 0x0500, 0x0500},
             {0x70040200, 0x0500, 0x0500},
             true,
             true},
            {"Sinh",
             {0x70040200, 0x0500, 0x0500},
             {0x70A00200, 0x0500, 0x0500},
             false,
             true},
            {"Mtei",
             {0x70A00200, 0x0500, 0x0500},
             {0x70A20200, 0x0500, 0x0500},
             false,
             true},
            {"Sylo",
             {0x70A20200, 0x0500, 0x0500},
             {0x70A40200, 0x0500, 0x0500},
             false,
             true},
            {"Saur",
             {0x70A40200, 0x0500, 0x0500},
             {0x70A60200, 0x0500, 0x0500},
             false,
             true},
            {"Kthi",
             {0x70A60200, 0x0500, 0x0500},
             {0x70A80200, 0x0500, 0x0500},
             false,
             true},
            {"Mahj",
             {0x70A80200, 0x0500, 0x0500},
             {0x70AA0200, 0x0500, 0x0500},
             false,
             true},
            {"Shrd",
             {0x70AA0200, 0x0500, 0x0500},
             {0x70AC0200, 0x0500, 0x0500},
             false,
             true},
            {"Khoj",
             {0x70AC0200, 0x0500, 0x0500},
             {0x70AE0200, 0x0500, 0x0500},
             false,
             true},
            {"Sind",
             {0x70AE0200, 0x0500, 0x0500},
             {0x70B00200, 0x0500, 0x0500},
             false,
             true},
            {"Mult",
             {0x70B00200, 0x0500, 0x0500},
             {0x70B20200, 0x0500, 0x0500},
             false,
             true},
            {"Gran",
             {0x70B20200, 0x0500, 0x0500},
             {0x70B40200, 0x0500, 0x0500},
             false,
             true},
            {"Newa",
             {0x70B40200, 0x0500, 0x0500},
             {0x70B60200, 0x0500, 0x0500},
             false,
             true},
            {"Tirh",
             {0x70B60200, 0x0500, 0x0500},
             {0x70B80200, 0x0500, 0x0500},
             false,
             true},
            {"Sidd",
             {0x70B80200, 0x0500, 0x0500},
             {0x70C80200, 0x0500, 0x0500},
             false,
             true},
            {"Modi",
             {0x70C80200, 0x0500, 0x0500},
             {0x70CA0200, 0x0500, 0x0500},
             false,
             true},
            {"Takr",
             {0x70CA0200, 0x0500, 0x0500},
             {0x70CC0200, 0x0500, 0x0500},
             false,
             true},
            {"Ahom",
             {0x70CC0200, 0x0500, 0x0500},
             {0x70D50200, 0x0500, 0x0500},
             false,
             true},
            {"Gonm",
             {0x70D50200, 0x0500, 0x0500},
             {0x70DA0200, 0x0500, 0x0500},
             false,
             true},
            {"Sund",
             {0x70DA0200, 0x0500, 0x0500},
             {0x70E70200, 0x0500, 0x0500},
             false,
             true},
            {"Brah",
             {0x70E70200, 0x0500, 0x0500},
             {0x70E90200, 0x0500, 0x0500},
             false,
             true},
            {"Khar",
             {0x70E90200, 0x0500, 0x0500},
             {0x70EB0200, 0x0500, 0x0500},
             false,
             true},
            {"Bhks",
             {0x70EB0200, 0x0500, 0x0500},
             {0x71040200, 0x0500, 0x0500},
             false,
             true},
            {"Thai",
             {0x71040200, 0x0500, 0x0500},
             {0x72040200, 0x0500, 0x0500},
             true,
             true},
            {"Laoo",
             {0x72040200, 0x0500, 0x0500},
             {0x72660200, 0x0500, 0x0500},
             false,
             true},
            {"Tavt",
             {0x72660200, 0x0500, 0x0500},
             {0x73040200, 0x0500, 0x0500},
             false,
             true},
            {"Tibt",
             {0x73040200, 0x0500, 0x0500},
             {0x73C60200, 0x0500, 0x0500},
             false,
             true},
            {"Zanb",
             {0x73C60200, 0x0500, 0x0500},
             {0x73D10200, 0x0500, 0x0500},
             false,
             true},
            {"Soyo",
             {0x73D10200, 0x0500, 0x0500},
             {0x74040200, 0x0500, 0x0500},
             false,
             true},
            {"Marc",
             {0x74040200, 0x0500, 0x0500},
             {0x74060200, 0x0500, 0x0500},
             false,
             true},
            {"Lepc",
             {0x74060200, 0x0500, 0x0500},
             {0x74080200, 0x0500, 0x0500},
             false,
             true},
            {"Phag",
             {0x74080200, 0x0500, 0x0500},
             {0x740A0200, 0x0500, 0x0500},
             false,
             true},
            {"Limb",
             {0x740A0200, 0x0500, 0x0500},
             {0x74190200, 0x0500, 0x0500},
             false,
             true},
            {"Tglg",
             {0x74190200, 0x0500, 0x0500},
             {0x741A0200, 0x0500, 0x0500},
             false,
             true},
            {"Hano",
             {0x741A0200, 0x0500, 0x0500},
             {0x741B0200, 0x0500, 0x0500},
             false,
             true},
            {"Buhd",
             {0x741B0200, 0x0500, 0x0500},
             {0x741C0200, 0x0500, 0x0500},
             false,
             true},
            {"Tagb",
             {0x741C0200, 0x0500, 0x0500},
             {0x741D0200, 0x0500, 0x0500},
             false,
             true},
            {"Bugi",
             {0x741D0200, 0x0500, 0x0500},
             {0x741E0200, 0x0500, 0x0500},
             false,
             true},
            {"Batk",
             {0x741E0200, 0x0500, 0x0500},
             {0x744B0200, 0x0500, 0x0500},
             false,
             true},
            {"Rjng",
             {0x744B0200, 0x0500, 0x0500},
             {0x744D0200, 0x0500, 0x0500},
             false,
             true},
            {"Kali",
             {0x744D0200, 0x0500, 0x0500},
             {0x744E0200, 0x0500, 0x0500},
             false,
             true},
            {"Mymr",
             {0x744E0200, 0x0500, 0x0500},
             {0x745D0200, 0x0500, 0x0500},
             false,
             true},
            {"Cakm",
             {0x745D0200, 0x0500, 0x0500},
             {0x745F0200, 0x0500, 0x0500},
             false,
             true},
            {"Khmr",
             {0x745F0200, 0x0500, 0x0500},
             {0x75040200, 0x0500, 0x0500},
             false,
             true},
            {"Tale",
             {0x75040200, 0x0500, 0x0500},
             {0x75050200, 0x0500, 0x0500},
             false,
             true},
            {"Talu",
             {0x75050200, 0x0500, 0x0500},
             {0x75120200, 0x0500, 0x0500},
             false,
             true},
            {"Lana",
             {0x75120200, 0x0500, 0x0500},
             {0x75260200, 0x0500, 0x0500},
             false,
             true},
            {"Cham",
             {0x75260200, 0x0500, 0x0500},
             {0x75280200, 0x0500, 0x0500},
             false,
             true},
            {"Bali",
             {0x75280200, 0x0500, 0x0500},
             {0x752A0200, 0x0500, 0x0500},
             false,
             true},
            {"Java",
             {0x752A0200, 0x0500, 0x0500},
             {0x75300200, 0x0500, 0x0500},
             false,
             true},
            {"Mong",
             {0x75300200, 0x0500, 0x0500},
             {0x75340200, 0x0500, 0x0500},
             false,
             true},
            {"Olck",
             {0x75340200, 0x0500, 0x0500},
             {0x75360200, 0x0500, 0x0500},
             false,
             true},
            {"Cher",
             {0x75360200, 0x0500, 0x0500},
             {0x75E40200, 0x0500, 0x0500},
             false,
             true},
            {"Osge",
             {0x75E40200, 0x0500, 0x0500},
             {0x75E60200, 0x0500, 0x0500},
             false,
             true},
            {"Cans",
             {0x75E60200, 0x0500, 0x0500},
             {0x75FA0200, 0x0500, 0x0500},
             false,
             true},
            {"Ogam",
             {0x75FA0200, 0x0500, 0x0500},
             {0x76040200, 0x0500, 0x0500},
             false,
             true},
            {"Runr",
             {0x76040200, 0x0500, 0x0500},
             {0x763C0200, 0x0500, 0x0500},
             false,
             true},
            {"Hung",
             {0x763C0200, 0x0500, 0x0500},
             {0x76900200, 0x0500, 0x0500},
             false,
             true},
            {"Orkh",
             {0x76900200, 0x0500, 0x0500},
             {0x77040200, 0x0500, 0x0500},
             false,
             true},
            {"Vaii",
             {0x77040200, 0x0500, 0x0500},
             {0x774C0200, 0x0500, 0x0500},
             false,
             true},
            {"Bamu",
             {0x774C0200, 0x0500, 0x0500},
             {0x775E0200, 0x0500, 0x0500},
             false,
             true},
            {"Bass",
             {0x775E0200, 0x0500, 0x0500},
             {0x775F0200, 0x0500, 0x0500},
             false,
             true},
            {"Mend",
             {0x775F0200, 0x0500, 0x0500},
             {0x77650200, 0x0500, 0x0500},
             false,
             true},
            {"Adlm",
             {0x77650200, 0x0500, 0x0500},
             {0x78040200, 0x0500, 0x0500},
             false,
             true},
            {"Hang",
             {0x78040200, 0x0500, 0x0500},
             {0x79040200, 0x0500, 0x0500},
             true,
             true},
            {"Hira",
             {0x79040200, 0x0500, 0x0500},
             {0x7A040200, 0x0500, 0x0500},
             true,
             true},
            {"Bopo",
             {0x7A040200, 0x0500, 0x0500},
             {0x7A6C0200, 0x0500, 0x0500},
             false,
             true},
            {"Yiii",
             {0x7A6C0200, 0x0500, 0x0500},
             {0x7A8D0200, 0x0500, 0x0500},
             false,
             true},
            {"Lisu",
             {0x7A8D0200, 0x0500, 0x0500},
             {0x7A8F0200, 0x0500, 0x0500},
             false,
             true},
            {"Plrd",
             {0x7A8F0200, 0x0500, 0x0500},
             {0x7AA10200, 0x0500, 0x0500},
             false,
             true},
            {"Wara",
             {0x7AA10200, 0x0500, 0x0500},
             {0x7AE30200, 0x0500, 0x0500},
             false,
             true},
            {"Pauc",
             {0x7AE30200, 0x0500, 0x0500},
             {0x7AE50200, 0x0500, 0x0500},
             false,
             true},
            {"Hmng",
             {0x7AE50200, 0x0500, 0x0500},
             {0x7AE80200, 0x0500, 0x0500},
             false,
             true},
            {"Lyci",
             {0x7AE80200, 0x0500, 0x0500},
             {0x7AE90200, 0x0500, 0x0500},
             false,
             true},
            {"Cari",
             {0x7AE90200, 0x0500, 0x0500},
             {0x7AEB0200, 0x0500, 0x0500},
             false,
             true},
            {"Lydi",
             {0x7AEB0200, 0x0500, 0x0500},
             {0x7AEC0200, 0x0500, 0x0500},
             false,
             true},
            {"Ital",
             {0x7AEC0200, 0x0500, 0x0500},
             {0x7AED0200, 0x0500, 0x0500},
             false,
             true},
            {"Goth",
             {0x7AED0200, 0x0500, 0x0500},
             {0x7B040200, 0x0500, 0x0500},
             false,
             true},
            {"Dsrt",
             {0x7B040200, 0x0500, 0x0500},
             {0x7B560200, 0x0500, 0x0500},
             false,
             true},
            {"Shaw",
             {0x7B560200, 0x0500, 0x0500},
             {0x7B580200, 0x0500, 0x0500},
             false,
             true},
            {"Dupl",
             {0x7B580200, 0x0500, 0x0500},
             {0x7B5C0200, 0x0500, 0x0500},
             false,
             true},
            {"Osma",
             {0x7B5C0200, 0x0500, 0x0500},
             {0x7B5D0200, 0x0500, 0x0500},
             false,
             true},
            {"Elba",
             {0x7B5D0200, 0x0500, 0x0500},
             {0x7B5F0200, 0x0500, 0x0500},
             false,
             true},
            {"Aghb",
             {0x7B5F0200, 0x0500, 0x0500},
             {0x7B610200, 0x0500, 0x0500},
             false,
             true},
            {"Sora",
             {0x7B610200, 0x0500, 0x0500},
             {0x7B620200, 0x0500, 0x0500},
             false,
             true},
            {"Mroo",
             {0x7B620200, 0x0500, 0x0500},
             {0x7B630200, 0x0500, 0x0500},
             false,
             true},
            {"Linb",
             {0x7B630200, 0x0500, 0x0500},
             {0x7B690200, 0x0500, 0x0500},
             false,
             true},
            {"Lina",
             {0x7B690200, 0x0500, 0x0500},
             {0x7B730200, 0x0500, 0x0500},
             false,
             true},
            {"Cprt",
             {0x7B730200, 0x0500, 0x0500},
             {0x7B750200, 0x0500, 0x0500},
             false,
             true},
            {"Sarb",
             {0x7B750200, 0x0500, 0x0500},
             {0x7B760200, 0x0500, 0x0500},
             false,
             true},
            {"Narb",
             {0x7B760200, 0x0500, 0x0500},
             {0x7B770200, 0x0500, 0x0500},
             false,
             true},
            {"Avst",
             {0x7B770200, 0x0500, 0x0500},
             {0x7B7D0200, 0x0500, 0x0500},
             false,
             true},
            {"Palm",
             {0x7B7D0200, 0x0500, 0x0500},
             {0x7B820200, 0x0500, 0x0500},
             false,
             true},
            {"Nbat",
             {0x7B820200, 0x0500, 0x0500},
             {0x7B9D0200, 0x0500, 0x0500},
             false,
             true},
            {"Hatr",
             {0x7B9D0200, 0x0500, 0x0500},
             {0x7B9E0200, 0x0500, 0x0500},
             false,
             true},
            {"Armi",
             {0x7B9E0200, 0x0500, 0x0500},
             {0x7B9F0200, 0x0500, 0x0500},
             false,
             true},
            {"Prti",
             {0x7B9F0200, 0x0500, 0x0500},
             {0x7BA00200, 0x0500, 0x0500},
             false,
             true},
            {"Phli",
             {0x7BA00200, 0x0500, 0x0500},
             {0x7BA10200, 0x0500, 0x0500},
             false,
             true},
            {"Phlp",
             {0x7BA10200, 0x0500, 0x0500},
             {0x7BA20200, 0x0500, 0x0500},
             false,
             true},
            {"Mani",
             {0x7BA20200, 0x0500, 0x0500},
             {0x7BA70200, 0x0500, 0x0500},
             false,
             true},
            {"Ugar",
             {0x7BA70200, 0x0500, 0x0500},
             {0x7BA80200, 0x0500, 0x0500},
             false,
             true},
            {"Xpeo",
             {0x7BA80200, 0x0500, 0x0500},
             {0x7BAA0200, 0x0500, 0x0500},
             false,
             true},
            {"Xsux",
             {0x7BAA0200, 0x0500, 0x0500},
             {0x7BC90200, 0x0500, 0x0500},
             false,
             true},
            {"Egyp",
             {0x7BC90200, 0x0500, 0x0500},
             {0x7C040200, 0x0500, 0x0500},
             false,
             true},
            {"Merc",
             {0x7C040200, 0x0500, 0x0500},
             {0x7C350200, 0x0500, 0x0500},
             false,
             true},
            {"Hluw",
             {0x7C350200, 0x0500, 0x0500},
             {0x7C460200, 0x0500, 0x0500},
             false,
             true},
            {"Tang",
             {0x7C460200, 0x0500, 0x0500},
             {0x7C7D0200, 0x0500, 0x0500},
             false,
             true},
            {"Nshu",
             {0x7C7D0200, 0x0500, 0x0500},
             {0x7D020200, 0x0500, 0x0500},
             false,
             true},
            {"Hani",
             {0x7D020200, 0x0500, 0x0500},
             {0xe5000000, 0x0500, 0x0500},
             true,
             false},
        };
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [&result](std::vector<text::detail::reorder_group> const &
                          reorder_groups) { result = reorder_groups; },
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv = "[reorder symbol punct Grek]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(result, expected) << result << expected;
    }

    {
        text::detail::collation_tailoring_interface callbacks = {
            [](text::detail::cp_seq_t const & reset_, bool before_) {},
            [](text::detail::relation_t const & rel) {},
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const & reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};
        text::string_view sv = "[reorder Sinh Mtei Sylo Saur]";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
    }
}

TEST(parser, rules)
{
    {
        text::detail::cp_seq_t reset_result;
        bool before_result = true;
        text::detail::relation_t relation_result;
        text::detail::cp_seq_t const abc = {'a', 'b', 'c'};
        text::detail::cp_seq_t const xyz = {'x', 'y', 'z'};
        text::detail::cp_seq_t const _123 = {'1', '2', '3'};
        text::detail::cp_seq_t const foo = {'f', 'o', 'o'};
        text::detail::cp_seq_t const last_regular = {
            text::detail::last_regular};

        text::detail::collation_tailoring_interface callbacks = {
            [&](text::detail::cp_seq_t const & reset_, bool before_) {
                reset_result = reset_;
                before_result = before_;
            },
            [&](text::detail::relation_t const & rel) {
                relation_result = rel;
            },
            [](text::collation_strength strength) {},
            [](text::variable_weighting weighting) {},
            [](text::l2_weight_order order) {},
            [](text::case_level_t case_level) {},
            [](text::case_first_t case_first) {},
            [](text::detail::cp_seq_t const & suppressions) {},
            [](std::vector<text::detail::reorder_group> const & reorder_groups) {},
            [](text::string const & s) { std::cout << s; },
            [](text::string const & s) { std::cout << s; }};

        text::string_view sv;

        // Testing operators and before/after.

        sv = "& a = b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(relation_result.op_, text::detail::token_kind::equal);

        sv = "& [before 1] a = b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(relation_result.op_, text::detail::token_kind::equal);

        sv = "& a < b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);

        sv = "& [before 1] a < b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);

        sv = "& a << b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::secondary_before);

        sv = "& [before 2] a << b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::secondary_before);

        sv = "& a <<< b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::tertiary_before);

        sv = "& [before 3] a <<< b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::tertiary_before);

        sv = "& a <<<< b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::quaternary_before);


        sv = "& a =* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(relation_result.op_, text::detail::token_kind::equal);

        sv = "& [before 1] a =* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(relation_result.op_, text::detail::token_kind::equal);

        sv = "& a <* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);

        sv = "& [before 1] a <* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);

        sv = "& a <<* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::secondary_before);

        sv = "& [before 2] a <<* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::secondary_before);

        sv = "& a <<<* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::tertiary_before);

        sv = "& [before 3] a <<<* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, true);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::tertiary_before);

        sv = "& a <<<<* b";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::quaternary_before);


        // Testing prefixes and extensions.

        sv = "& abc < xyz | 123 / foo";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(relation_result.prefix_and_extension_.prefix_, _123);
        EXPECT_EQ(relation_result.prefix_and_extension_.extension_, foo);

        sv = "& abc < xyz / foo | 123";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(relation_result.prefix_and_extension_.prefix_, _123);
        EXPECT_EQ(relation_result.prefix_and_extension_.extension_, foo);

        sv = "& abc < xyz | 123";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(relation_result.prefix_and_extension_.prefix_, _123);
        EXPECT_EQ(
            relation_result.prefix_and_extension_.extension_,
            text::detail::optional_cp_seq_t());

        sv = "& abc < xyz / foo";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(
            relation_result.prefix_and_extension_.prefix_,
            text::detail::optional_cp_seq_t());
        EXPECT_EQ(relation_result.prefix_and_extension_.extension_, foo);

        sv = "& abc < xyz";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(
            relation_result.prefix_and_extension_.prefix_,
            text::detail::optional_cp_seq_t());
        EXPECT_EQ(
            relation_result.prefix_and_extension_.extension_,
            text::detail::optional_cp_seq_t());


        // Testing full rules.

        sv = "& abc < xyz | 123 / foo << foo";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, abc);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::secondary_before);
        EXPECT_EQ(relation_result.cps_, foo);
        EXPECT_EQ(
            relation_result.prefix_and_extension_.prefix_,
            text::detail::optional_cp_seq_t());
        EXPECT_EQ(
            relation_result.prefix_and_extension_.extension_,
            text::detail::optional_cp_seq_t());

        sv = "& [last regular] < bar < xyz | foo / 123";
        EXPECT_NO_THROW(text::detail::parse(
            sv.begin(), sv.end(), callbacks, "<test-string>"));
        EXPECT_EQ(reset_result, last_regular);
        EXPECT_EQ(before_result, false);
        EXPECT_EQ(
            relation_result.op_, text::detail::token_kind::primary_before);
        EXPECT_EQ(relation_result.cps_, xyz);
        EXPECT_EQ(relation_result.prefix_and_extension_.prefix_, foo);
        EXPECT_EQ(relation_result.prefix_and_extension_.extension_, _123);
    }
}
