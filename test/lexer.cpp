// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/detail/lexer.hpp>

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

char const * all_cases[] = {
    (char const *)case_3_3,    (char const *)case_3_5_a,
    (char const *)case_3_5_b,  (char const *)case_3_6_a,
    (char const *)case_3_6_b,  (char const *)case_3_6_c,
    (char const *)case_3_6_d,  (char const *)case_3_6_e,
    (char const *)case_3_6_f,  (char const *)case_3_6_g,
    (char const *)case_3_6_h,  (char const *)case_3_6_i,
    (char const *)case_3_6_j,  (char const *)case_3_6_k,
    (char const *)case_3_6_l,  (char const *)case_3_6_m,
    (char const *)case_3_6_n,  (char const *)case_3_6_o,
    (char const *)case_3_6_p,  (char const *)case_3_7,
    (char const *)case_3_9_a,  (char const *)case_3_9_b,
    (char const *)case_3_9_c,  (char const *)case_3_9_d,
    (char const *)case_3_10_a, (char const *)case_3_10_b,
    (char const *)case_3_10_c, (char const *)case_3_11,
    (char const *)case_3_12_a, (char const *)case_3_12_b,
    (char const *)case_3_12_c, (char const *)case_3_12_d,
    (char const *)case_3_12_e, (char const *)case_3_12_f,
    (char const *)case_3_13,   (char const *)quotes};


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
                [](std::string const & s) { std::cout << s << "\n"; },
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
                    [](std::string const & s) {
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
            [](std::string const & s) { std::cout << s << "\n"; },
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
            [](std::string const & s) { std::cout << s << "\n"; },
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
            [](std::string const & s) { std::cout << s << "\n"; },
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
