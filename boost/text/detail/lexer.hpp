#ifndef BOOST_TEXT_DETAIL_LEXER_HPP
#define BOOST_TEXT_DETAIL_LEXER_HPP

#include <boost/text/string.hpp>
#include <boost/text/utf8.hpp>
#include <boost/text/utility.hpp>

#include <sstream>
#include <iomanip>


namespace boost { namespace text {

    using parser_diagnostic_callback = std::function<void(string const &)>;

}}

namespace boost { namespace text { namespace detail {

    enum class token_kind {
        code_point,
        quote,
        and_,
        or_,
        slash,
        equal,
        open_bracket,
        close_bracket,
        primary_before,
        secondary_before,
        tertiary_before,
        quaternary_before,
        primary_before_star,
        secondary_before_star,
        tertiary_before_star,
        quaternary_before_star,
        identifier
    };

#ifndef NDEBUG
    inline std::ostream & operator<<(std::ostream & os, token_kind kind)
    {
#    define CASE(x)                                                            \
    case token_kind::x: os << #x; break
        switch (kind) {
            CASE(code_point);
            CASE(quote);
            CASE(and_);
            CASE(or_);
            CASE(slash);
            CASE(equal);
            CASE(open_bracket);
            CASE(close_bracket);
            CASE(primary_before);
            CASE(secondary_before);
            CASE(tertiary_before);
            CASE(quaternary_before);
            CASE(primary_before_star);
            CASE(secondary_before_star);
            CASE(tertiary_before_star);
            CASE(quaternary_before_star);
            CASE(identifier);
        }
#    undef CASE
        return os;
    }
#endif

    struct token
    {
        token() : kind_(token_kind::identifier), cp_(), line_(-1), column_(-1)
        {}
        token(token_kind kind, int line, int column) :
            kind_(kind),
            cp_(),
            line_(line),
            column_(column)
        {}
        token(uint32_t c, int line, int column) :
            kind_(token_kind::code_point),
            cp_(c),
            line_(line),
            column_(column)
        {}
        token(string identifier, int line, int column) :
            kind_(token_kind::identifier),
            cp_(),
            identifier_(std::move(identifier)),
            line_(line),
            column_(column)
        {}

        token_kind kind() const { return kind_; }
        uint32_t cp() const
        {
            assert(kind_ == token_kind::code_point);
            return cp_;
        }
        string const & identifier() const { return identifier_; }
        int line() const { return line_; }
        int column() const { return column_; }

        friend bool operator==(token const & tok, uint32_t cp)
        {
            return tok.kind() == token_kind::code_point && tok.cp() == cp;
        }

        friend bool operator==(token const & tok, token_kind kind)
        {
            assert(
                kind != token_kind::code_point &&
                kind != token_kind::identifier);
            return tok.kind() == kind;
        }

        friend bool operator==(token const & tok, string const & id)
        {
            return tok.kind() == token_kind::identifier &&
                   tok.identifier() == id;
        }

#ifndef NDEBUG
        friend std::ostream & operator<<(std::ostream & os, token tok)
        {
            os << "[" << tok.kind_ << " ";
            if (tok.kind_ == token_kind::code_point)
                os << std::hex << "cp=0x" << tok.cp_ << std::dec << " ";
            else if (tok.kind_ == token_kind::identifier)
                os << "\"" << tok.identifier_ << "\" ";
            os << "(" << tok.line_ << ":" << tok.column_ << ")]";
            return os;
        }
#endif

    private:
        token_kind kind_;
        uint32_t cp_;
        string identifier_;
        int line_;
        int column_;
    };

    struct lex_error : std::exception
    {
        lex_error(string_view msg, int line, int column) :
            msg_(msg),
            line_(line),
            column_(column)
        {}
        char const * what() const noexcept { return msg_.begin(); }
        int line() const { return line_; }
        int column() const { return column_; }

    private:
        string msg_;
        int line_;
        int column_;
    };

    struct lines_and_tokens
    {
        std::vector<int> line_starts_;
        std::vector<token> tokens_;
    };

#ifndef NDEBUG
    std::ostream &
    dump(std::ostream & os, lines_and_tokens const & lat, string_view str)
    {
        auto tok_it = lat.tokens_.begin();
        for (int i = 0, end = lat.line_starts_.size(); i < end; ++i) {
            bool const last_line = end <= i + 1;
            auto tok_end = lat.tokens_.end();
            if (last_line) {
                os << (str.begin() + lat.line_starts_[i]);
                if (str[-1] != '\n')
                    os << "\n";
            } else {
                os.write(
                    str.begin() + lat.line_starts_[i],
                    lat.line_starts_[i + 1] - lat.line_starts_[i]);
                if (str[lat.line_starts_[i + 1] - 1] != '\n')
                    os << "\n";
                tok_end = std::find_if(tok_it, tok_end, [i](token const & tok) {
                    return i < tok.line();
                });
            }
            auto pos = 0;
            for (; tok_it != tok_end; ++tok_it) {
                for (auto new_pos = tok_it->column(); pos < new_pos; ++pos) {
                    os << ' ';
                }
                os << '^';
                ++pos;
            }
            os << "\n";
        }
        return os;
    }
#endif

    inline lines_and_tokens
    lex(char const * first,
        char const * const last,
        parser_diagnostic_callback errors)
    {
        lines_and_tokens retval;

        auto const initial_first = first;

        retval.line_starts_.push_back(0);

        int char_index = 0;
        int line = 0;
        int column = 0;
        int initial_column = column;
        int brackets_nesting = 0;

        char buf[1024];

        auto report_error = [&retval, &initial_first, &last, &line, &errors](
                                char const * msg, int column) {
            if (errors) {
                string str(msg);
                if (str.empty() || str[-1] != '\n')
                    str += '\n';
                auto const line_end = std::find(
                    initial_first + retval.line_starts_[line], last, '\n');
                str.insert(
                    str.end(),
                    initial_first + retval.line_starts_[line],
                    line_end);
                str += '\n';
                str += repeated_string_view(" ", column);
                str += "^\n";
                errors(str);
            }
            throw lex_error(msg, line, column);
        };

        auto check_end =
            [&first, last, &line, &column, &report_error](char const * msg) {
                if (first == last)
                    report_error(msg, column);
            };

        auto consume =
            [&first, &buf, check_end, &column, &char_index](
                int n, char const * end_msg, char * buf_ptr = nullptr) {
                char * buf_ = buf_ptr ? buf_ptr : buf;
                while (n) {
                    check_end(end_msg);
                    *buf_++ = *first++;
                    ++column;
                    ++char_index;
                    --n;
                }
                return buf_;
            };

        auto consume_one = [consume](char const * end_msg) {
            return *(consume(1, end_msg) - 1);
        };

        auto consume_if_equals = [&first, last, consume_one](char c) {
            bool retval = false;
            if (first != last && *first == c) {
                consume_one("");
                retval = true;
            }
            return retval;
        };

        auto is_id_char = [](char c) {
            char const id_chars[] =
                "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                "WXYZ"
                "_-";
            return std::find(std::begin(id_chars), std::end(id_chars), c) !=
                   std::end(id_chars);
        };

        auto is_hex = [](char c) {
            return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
                   ('A' <= c && c <= 'F');
        };

        auto is_octal = [](char c) { return '0' <= c && c <= '8'; };

        auto from_hex = [](char const * str) {
            std::istringstream is(str);
            uint32_t x;
            is >> std::hex >> x;
            return x;
        };

        auto from_octal = [](char const * str) {
            std::istringstream is(str);
            uint32_t x;
            is >> std::oct >> x;
            return x;
        };

        auto newline = [&line, &column, &retval, &char_index]() {
            ++line;
            column = 0;
            retval.line_starts_.push_back(char_index);
        };

        auto push = [&retval, &line, &initial_column](token_kind kind) {
            retval.tokens_.push_back(token(kind, line, initial_column));
        };

        auto push_cp = [&retval, &line, &initial_column](uint32_t cp) {
            retval.tokens_.push_back(token(cp, line, initial_column));
        };

        auto push_identifier =
            [&retval, &line, &initial_column](string identifier) {
                retval.tokens_.push_back(
                    token(std::move(identifier), line, initial_column));
            };

        while (first != last) {
            initial_column = column;
            char const initial_char = consume_one("");

            if (initial_char == ' ' || initial_char == '\t') {
                continue;
            } else if (initial_char == '#') {
                while (first != last && *first != '\n') {
                    consume_one("");
                }
                if (first != last)
                    consume_one("");
                newline();
            } else if (initial_char == '\r') {
                char const c = consume_one(
                    "\\r at end of input (must be followed by \\n)");
                if (c != '\n') {
                    report_error(
                        "Stray \\r without following \\n", initial_column);
                }
                newline();
            } else if (initial_char == '\n') {
                newline();
            } else if (initial_char == '\\') {
                // The escape-handling logic here is derived from the
                // documentation of ICU's UnicodeString::unescape(), as
                // indicated by
                // http://www.unicode.org/reports/tr35/tr35-collation.html#Rules
                char const c = consume_one("\\ at end of input");
                assert(c != 'r' && c != 'n'); // Handled above.
                switch (c) {
                case 'u': {
                    auto buf_end = consume(
                        4, "Incomplete \\uNNNN hexidecimal escape sequence");
                    if (!std::all_of(buf, buf_end, is_hex)) {
                        report_error(
                            "Non-hexidecimal digit in \\uNNNN hexidecimal "
                            "escape sequence",
                            initial_column);
                    }
                    *buf_end = '\0';
                    push_cp(from_hex(buf));
                    break;
                }
                case 'U': {
                    auto buf_end = consume(
                        8,
                        "Incomplete \\UNNNNNNNN hexidecimal escape "
                        "sequence");
                    if (!std::all_of(buf, buf_end, is_hex)) {
                        report_error(
                            "Non-hexidecimal digit in \\UNNNNNNNN hexidecimal "
                            "escape sequence",
                            initial_column);
                    }
                    *buf_end = '\0';
                    push_cp(from_hex(buf));
                    break;
                }
                case 'x': {
                    char local_buf[4] = {0};
                    auto local_buf_end = local_buf;
                    *local_buf_end++ = consume_one(
                        "Incomplete \\xN[N] hexidecimal escape sequence "
                        "(at "
                        "least one hexidecimal digit must follow '\\x')");
                    if (!is_hex(local_buf[0])) {
                        report_error(
                            "Non-octal hexidecimal in \\xN[N] hexidecimal "
                            "escape sequence",
                            initial_column);
                    }
                    if (first != last && is_hex(*first))
                        *local_buf_end++ = consume_one("");
                    *local_buf_end = '\0';
                    push_cp(from_hex(local_buf));
                    break;
                }
                case 'o': {
                    char local_buf[4] = {0};
                    auto local_buf_end = local_buf;
                    *local_buf_end++ = consume_one(
                        "Incomplete \\oN[N][N] octal escape sequence (at "
                        "least one octal digit must follow '\\o')");
                    if (!is_octal(local_buf[0])) {
                        report_error(
                            "Non-octal digit in \\oN[N][N] octal escape "
                            "sequence",
                            initial_column);
                    }
                    if (first != last && is_octal(*first))
                        *local_buf_end++ = consume_one("");
                    if (first != last && is_octal(*first))
                        *local_buf_end++ = consume_one("");
                    *local_buf_end = '\0';
                    push_cp(from_octal(local_buf));
                    break;
                }
                case 'a': push_cp(0x7); break;
                case 'b': push_cp(0x8); break;
                case 't': push_cp(0x9); break;
                case 'v': push_cp(0xb); break;
                case 'f': push_cp(0xc); break;
                case 'e': push_cp(0x1b); break;
                case '"': push_cp(0x22); break;
                case '\'': push_cp(0x27); break;
                case '?': push_cp(0x3f); break;
                case '\\': push_cp(0x5c); break;
                }
            } else if (initial_char == '\'') {
                push(token_kind::quote);
            } else if (initial_char == '&') {
                push(token_kind::and_);
            } else if (initial_char == '|') {
                push(token_kind::or_);
            } else if (initial_char == '/') {
                push(token_kind::slash);
            } else if (initial_char == '=') {
                push(token_kind::equal);
            } else if (initial_char == '[') {
                push(token_kind::open_bracket);
                ++brackets_nesting;
            } else if (initial_char == ']') {
                push(token_kind::close_bracket);
                --brackets_nesting;
            } else if (initial_char == '<') {
                token_kind kind = token_kind::primary_before;
                if (consume_if_equals('<')) {
                    kind = token_kind::secondary_before;
                    if (consume_if_equals('<')) {
                        kind = token_kind::tertiary_before;
                        if (consume_if_equals('<')) {
                            kind = token_kind::quaternary_before;
                        }
                    }
                }
                if (consume_if_equals('*'))
                    kind = static_cast<token_kind>(static_cast<int>(kind) + 4);
                push(kind);
            } else if (1 <= brackets_nesting && is_id_char(initial_char)) {
                string identifier;
                identifier += initial_char;
                while (first != last && is_id_char(*first)) {
                    identifier += consume_one("");
                }
                push_identifier(std::move(identifier));
            } else {
                // UTF-8 encoded code point.
                auto const code_units = utf8::code_point_bytes(initial_char);
                if (code_units < 0) {
                    report_error(
                        "Invalid initial UTF-8 code unit", initial_column);
                }
                *buf = initial_char;
                if (1 < code_units) {
                    consume(
                        code_units - 1, "Incomplete UTF-8 sequence", buf + 1);
                }
                utf32_range as_utf32(buf, buf + code_units);
                push_cp(*as_utf32.begin());
                // Treat this cp as a single column, even though this looks
                // weird for wide glyphs in East Asian scripts.
                column = initial_column + 1;
            }
        }

        if (retval.tokens_.back() == 0)
            retval.tokens_.pop_back();

        return retval;
    }

}}}

#endif
