#include <boost/text/detail/lexer.hpp>
// TODO: Remove.
#include <boost/text/collate.hpp>
#include <boost/text/collation_table.hpp>

#include "parser_tests.hpp"


// This is a visual test, because the lexer is very simple.  Looking at the
// output, it will be obvious whether or not the lexer is tokenizing things
// correctly.


char const quotes[] = "' \\UAbcD1234''X#Y& [] /|=<\\t\t'";

char const * failure_cases[] = {
    // Incomplete escape sequences:
    "\t\\u",
    "\\UG",
    "\\",
    "\\x",
    "\\o",

    // Newine char in string:
    "'\n'",
    "'\r'",

    "\r",
    "\r ",

    // Bad UTF-8:
    "\x80",
    "\xc2",

    // Unescaped syntax chars:
    "\"",
    "$",
    "%",
    "(",
    ")",
    "*",
    "+",
    ",",
    ".",
    ";",
    ">",
    "?",
    "^",
    "_",
    "{",
    "}",

    // Unbalanced elements:
    "'",
    "]",
    "[]]",
};

char const * all_cases[] = {case_3_3,    case_3_5_a,  case_3_5_b,  case_3_6_a,
                            case_3_6_b,  case_3_6_c,  case_3_6_d,  case_3_6_e,
                            case_3_6_f,  case_3_6_g,  case_3_6_h,  case_3_6_i,
                            case_3_6_j,  case_3_6_k,  case_3_6_l,  case_3_6_m,
                            case_3_6_n,  case_3_6_o,  case_3_6_p,  case_3_7,
                            case_3_9_a,  case_3_9_b,  case_3_9_c,  case_3_9_d,
                            case_3_10_a, case_3_10_b, case_3_10_c, case_3_11,
                            case_3_12_a, case_3_12_b, case_3_12_c, case_3_12_d,
                            case_3_12_e, case_3_12_f, case_3_13,   quotes};


int main()
{
    {
        auto it = std::begin(all_cases);
        auto const end = std::end(all_cases);
        for (; it != end; ++it) {
            char const * const str_begin = *it;
            char const * const str_end = str_begin + strlen(str_begin);
            auto const lines_and_tokens = boost::text::detail::lex(
                str_begin,
                str_end,
                [](boost::text::string const & s) { std::cout << s << "\n"; },
                "<test-string>");
#ifndef NDEBUG
            dump(std::cout, lines_and_tokens, *it);
#endif
            std::cout
                << "========================================================="
                   "=======================\n";
        }
    }

    {
        auto it = std::begin(failure_cases);
        auto const end = std::end(failure_cases);
        for (; it != end; ++it) {
            char const * const str_begin = *it;
            char const * const str_end = str_begin + strlen(str_begin);
            try {
                auto const lines_and_tokens = boost::text::detail::lex(
                    str_begin,
                    str_end,
                    [](boost::text::string const & s) {
                        std::cout << s << "\n";
                    },
                    "<test-string>");
#ifndef NDEBUG
                dump(std::cout, lines_and_tokens, *it);
                // std::cout << dump(lines_and_tokens.tokens_);
#endif
            } catch (boost::text::parse_error const & e) {
            }
            std::cout
                << "========================================================="
                   "=======================\n";
        }
    }

    {
        boost::text::string_view sv = "a-b";
        auto const lines_and_tokens = boost::text::detail::lex(
            sv.begin(),
            sv.end(),
            [](boost::text::string const & s) { std::cout << s << "\n"; },
            "<test-string>");
#ifndef NDEBUG
        dump(std::cout, lines_and_tokens, sv);
        std::cout << dump(lines_and_tokens.tokens_) << "\n";
#endif
        std::cout << "========================================================="
                     "=======================\n";
    }

    {
        boost::text::string_view sv = "'a-b'";
        auto const lines_and_tokens = boost::text::detail::lex(
            sv.begin(),
            sv.end(),
            [](boost::text::string const & s) { std::cout << s << "\n"; },
            "<test-string>");
#ifndef NDEBUG
        dump(std::cout, lines_and_tokens, sv);
        std::cout << dump(lines_and_tokens.tokens_) << "\n";
#endif
        std::cout << "========================================================="
                     "=======================\n";
    }

    {
        boost::text::string_view sv = "a\\-b";
        auto const lines_and_tokens = boost::text::detail::lex(
            sv.begin(),
            sv.end(),
            [](boost::text::string const & s) { std::cout << s << "\n"; },
            "<test-string>");
#ifndef NDEBUG
        dump(std::cout, lines_and_tokens, sv);
        std::cout << dump(lines_and_tokens.tokens_) << "\n";
#endif
        std::cout << "========================================================="
                     "=======================\n";
    }

    return 0;
}
