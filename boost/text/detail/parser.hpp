#ifndef BOOST_TEXT_DETAIL_PARSER_HPP
#define BOOST_TEXT_DETAIL_PARSER_HPP

#include <boost/text/collation_weights.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/detail/lexer.hpp>

#include <boost/optional.hpp>


namespace boost { namespace text { namespace detail {

    // code-point = ? A valid Unicode code point not in the range
    // U+FFFD..U+FFFF ? ; nfd-inert-cp = ? A valid Unicode code point not in
    // the range U+FFFD..U+FFFF with ccc=0 ? ;

    using cp_seq_t = std::vector<uint32_t>;
    using optional_cp_seq_t = optional<cp_seq_t>;

    struct prefix_and_extension_t
    {
        optional_cp_seq_t prefix_;
        optional_cp_seq_t extension_;
    };

    struct relation_t
    {
        token_kind op_;
        cp_seq_t cps_;
        prefix_and_extension_t prefix_and_extension_;
    };

    using reset_callback =
        std::function<void(cp_seq_t const & seq, int before_strength)>;
    using relation_callback = std::function<void(relation_t const &)>;

    struct collation_tailoring_interface
    {
        reset_callback reset_;
        relation_callback relation_;

        // TODO: options, etc.

        parser_diagnostic_callback errors_;
        parser_diagnostic_callback warnings_;
    };

    struct one_token_parse_error : parse_error
    {
        one_token_parse_error(string_view msg, token_iter it) :
            parse_error(msg, it->line(), it->column())
        {}
    };

    struct two_token_parse_error : parse_error
    {
        two_token_parse_error(
            string_view msg,
            token_iter it,
            string_view prev_msg,
            token_iter prev_it) :
            parse_error(msg, it->line(), it->column()),
            prev_msg_(prev_msg),
            prev_line_(prev_it->line()),
            prev_column_(prev_it->column())
        {}

        string_view prev_msg() const noexcept { return prev_msg_; }
        int prev_line() const noexcept { return prev_line_; }
        int prev_column() const noexcept { return prev_column_; }

    private:
        string prev_msg_;
        int prev_line_;
        int prev_column_;
    };

    inline bool require(token_iter & it, token_iter end, token_kind kind)
    {
        if (it == end)
            return false;
        bool const retval = it->kind() == kind;
        if (retval)
            ++it;
        return retval;
    }
    inline bool require(token_iter & it, token_iter end, string_view sv)
    {
        if (it == end)
            return false;
        bool const retval =
            it->kind() == token_kind::identifier && it->identifier() == sv;
        if (retval)
            ++it;
        return retval;
    }
    inline bool require_impl(token_iter & it, token_iter end) { return true; }
    template<typename T, typename... Ts>
    inline bool require_impl(token_iter & it, token_iter end, T t, Ts... ts)
    {
        if (require_impl(it, end) && require_impl(it, end, ts...))
            return true;
        return false;
    }
    template<typename T, typename... Ts>
    inline bool require(token_iter & it, token_iter end, T t, Ts... ts)
    {
        auto const initial_it = it;
        if (!require_impl(it, end, t, ts...)) {
            it = initial_it;
            return false;
        }
        return true;
    }

    inline optional<token_kind> next_tok(token_iter & it, token_iter end)
    {
        if (it == end)
            return {};
        if (it->kind() == token_kind::identifier ||
            it->kind() == token_kind::code_point)
            return {};
        return (it++)->kind();
    }
    inline optional<string> next_identifier(token_iter & it, token_iter end)
    {
        if (it == end)
            return {};
        if (it->kind() != token_kind::identifier)
            return {};
        return (it++)->identifier();
    }
    inline optional<uint32_t>
    next_cp(token_iter & it, token_iter end, bool dashes_too = false)
    {
        if (it == end)
            return {};
        if (dashes_too && it->kind() == token_kind::code_point &&
            it->cp() == '-') {
            return '-';
        }
        if (it->kind() != token_kind::code_point)
            return {};
        if (0xfffd <= it->cp() && it->cp() <= 0xffff) {
            throw one_token_parse_error(
                "U+FFFD..U+FFFF are not tailorable, and nothing can tailor to "
                "them.",
                it);
        }
        return (it++)->cp();
    }

    struct cp_range_t
    {
        uint32_t first_;
        uint32_t last_;
    };

    // cp-range = nfd-inert-cp | nfd-inert-cp, "-", nfd-inert-cp ;
    inline optional<cp_range_t> next_cp_range(token_iter & it, token_iter end)
    {
        auto const lo = next_cp(it, end);
        if (!lo)
            return {};

        if (!require(it, end, token_kind::dash))
            return cp_range_t{*lo, *lo + 1};

        auto const hi = next_cp(it, end);
        if (!hi) {
            throw one_token_parse_error(
                "Expected code point after dash; did you forget to escape or "
                "quote the dash?",
                it);
        }

        return cp_range_t{*lo, *hi};
    }

    // cp-sequence = code-point, {code-point} ;
    inline cp_seq_t next_cp_seq(token_iter & it, token_iter end)
    {
        cp_seq_t retval;
        optional<uint32_t> cp;
        do {
            if ((cp = next_cp(it, end, true)))
                retval.push_back(*cp);
        } while (cp);
        return retval;
    }

    // before-strength = "[", "before", ("1" | "2" | "3"), "]" ;
    inline int before_strength(token_iter & it, token_iter end)
    {
        auto const open_bracket_it = it;
        if (!require(it, end, token_kind::open_bracket))
            return 0;

        if (!require(it, end, "before"))
            return 0;

        int retval = 0;
        if (require(it, end, "1"))
            retval = 1;
        else if (require(it, end, "2"))
            retval = 2;
        else if (require(it, end, "3"))
            retval = 3;
        else
            throw one_token_parse_error("Expected '1', '2', or '3' here", it);

        if (!require(it, end, token_kind::close_bracket)) {
            throw two_token_parse_error(
                "Expected close bracket here",
                it,
                "to match previous open bracket",
                open_bracket_it);
        }

        return retval;
    }

    // logical-position = "[", (
    //    ("first", "tertiary", "ignorable") |
    //    ("last", "tertiary", "ignorable") |
    //    ("first", "secondary", "ignorable") |
    //    ("last", "secondary", "ignorable") |
    //    ("first", "primary", "ignorable") |
    //    ("last", "primary", "ignorable") |
    //    ("first", "variable") |
    //    ("last", "variable") |
    //    ("first", "regular") |
    //    ("last", "regular") |
    //    ("first", "implicit") |
    //    ("last", "implicit") |
    //    ("first", "trailing") |
    //    ("last", "trailing") |
    // ), "]" ;
    inline optional<uint32_t> logical_position(token_iter & it, token_iter end)
    {
        auto const open_bracket_it = it;
        if (!require(it, end, token_kind::open_bracket))
            return 0;

        uint32_t position;
        if (require(it, end, "first", "tertiary", "ignorable"))
            position = first_tertiary_ignorable;
        else if (require(it, end, "last", "tertiary", "ignorable"))
            position = last_tertiary_ignorable;
        else if (require(it, end, "first", "secondary", "ignorable"))
            position = first_secondary_ignorable;
        else if (require(it, end, "last", "secondary", "ignorable"))
            position = last_secondary_ignorable;
        else if (require(it, end, "first", "primary", "ignorable"))
            position = first_primary_ignorable;
        else if (require(it, end, "last", "primary", "ignorable"))
            position = last_primary_ignorable;
        else if (require(it, end, "first", "variable"))
            position = first_variable;
        else if (require(it, end, "last", "variable"))
            position = last_variable;
        else if (require(it, end, "first", "regular"))
            position = first_regular;
        else if (require(it, end, "last", "regular"))
            position = last_regular;
        else if (require(it, end, "first", "implicit"))
            position = first_implicit;
        else if (require(it, end, "last", "implicit"))
            throw one_token_parse_error(
                "Logical position [last implicit] is not supported", it);
        else if (require(it, end, "first", "trailing"))
            throw one_token_parse_error(
                "Logical position [first trailing] is not supported", it);
        else if (require(it, end, "last", "trailing"))
            throw one_token_parse_error(
                "Logical position [last trailing] is not supported", it);
        else
            throw one_token_parse_error("Unknown logical position", it);

        if (!require(it, end, token_kind::close_bracket)) {
            throw two_token_parse_error(
                "Expected close bracket here",
                it,
                "to match previous open bracket",
                open_bracket_it);
        }

        return position;
    }

    // prefix = "|", cp-sequence ;
    // extension = "/", cp-sequence ;
    inline optional_cp_seq_t
    op_and_seq(token_iter & it, token_iter end, token_kind op)
    {
        if (!require(it, end, op))
            return {};
        auto seq = next_cp_seq(it, end);
        if (seq.empty()) {
            throw one_token_parse_error(
                "Expected one or more code points to the right of an operator",
                it);
        }
        return optional_cp_seq_t(std::move(seq));
    }

    inline prefix_and_extension_t
    prefix_and_extension(token_iter & it, token_iter end)
    {
        prefix_and_extension_t retval;
        if (auto prefix_seq = op_and_seq(it, end, token_kind::or_)) {
            retval.prefix_ = std::move(prefix_seq);
            if (auto extension_seq = op_and_seq(it, end, token_kind::slash))
                retval.extension_ = std::move(extension_seq);
        } else if (
            auto extension_seq = op_and_seq(it, end, token_kind::slash)) {
            retval.extension_ = std::move(extension_seq);
            if ((prefix_seq = op_and_seq(it, end, token_kind::or_)))
                retval.prefix_ = std::move(prefix_seq);
        }
        return retval;
    }

    // relation-op = "=" | "<" | "<<" | "<<<" | "<*" | "<<*" | "<<<*" ;
    // relation = relation-op, cp-sequence, (([prefix], extension) |
    // ([extension], prefix)) ;
    inline optional<relation_t> relation(token_iter & it, token_iter end)
    {
        auto const op_it = it;
        auto const op = next_tok(it, end);
        if (!op) {
            return {};
        } else if (
            token_kind::equal <= op && op <= token_kind::quaternary_before) {
            auto seq = next_cp_seq(it, end);
            if (seq.empty()) {
                throw one_token_parse_error(
                    "Expected one or more code points to the right of an "
                    "operator",
                    it);
            }
            return relation_t{
                *op, std::move(seq), prefix_and_extension(it, end)};
        } else if (
            token_kind::equal_star <= op &&
            op <= token_kind::quaternary_before_star) {
            cp_seq_t seq;
            auto const start_of_range_it = it;
            auto range = next_cp_range(it, end);
            if (!range) {
                throw one_token_parse_error(
                    "Expected one or more code points to the right of an "
                    "operator",
                    it);
            }

            auto check_ccc_0_and_append = [&](cp_range_t r) {
                for (auto cp = r.first_, cp_end = r.last_; cp < cp_end; ++cp) {
                    if (ccc(cp)) {
                        seq.push_back(cp);
                    } else {
                        throw one_token_parse_error(
                            "All code points following an abbreviated relation "
                            "operator must be ccc=0",
                            start_of_range_it);
                    }
                }
            };

            check_ccc_0_and_append(*range);

            while ((range = next_cp_range(it, end))) {
                check_ccc_0_and_append(*range);
            }

            return relation_t{*op, std::move(seq)};
        } else {
            throw one_token_parse_error(
                "Expected one of '<', '<<', '<<<', '<<<<', '=', '<*', '<<*', "
                "'<<<*', '<<<<*', or '=*' here",
                op_it);
        }
    }

    inline bool strength_matches_op(int str, token_kind op) noexcept
    {
        return str == 0 || op == token_kind::equal ||
               str ==
                   static_cast<int>(op) - static_cast<int>(token_kind::equal);
    }

    // reset = cp-sequence | logical-position ;
    // rule-chain = "&", [before-strength], reset, relation, {relation} ;
    inline bool rule_chain(
        token_iter & it,
        token_iter end,
        int before_strength_,
        cp_seq_t const & reset,
        collation_tailoring_interface & tailoring)
    {
        auto rel_it = it;
        auto rel = relation(it, end);
        if (!rel)
            return false;

        if (!strength_matches_op(before_strength_, rel->op_)) {
            throw one_token_parse_error(
                "Relation strength must match S in [before S], unless the "
                "relation operator is '=' or '=*'",
                rel_it);
        }

        auto record = [&]() {
            tailoring.reset_(reset, before_strength_);
            if (rel->op_ < token_kind::primary_before_star) {
                tailoring.relation_(*rel);
            } else {
                cp_seq_t cps = std::move(rel->cps_);
                rel->op_ =
                    static_cast<token_kind>(static_cast<int>(rel->op_) - 5);
                for (auto cp : cps) {
                    rel->cps_.clear();
                    rel->cps_.push_back(cp);
                    tailoring.relation_(*rel);
                }
            }
        };

        record();

        while (rel) {
            rel_it = it;
            rel = relation(it, end);
            if (!strength_matches_op(before_strength_, rel->op_)) {
                throw one_token_parse_error(
                    "Relation strength must match S in [before S], unless the "
                    "relation operator is '=' or '=*'",
                    rel_it);
            }
            record();
        }

        return true;
    }

    inline void rule(
        token_iter & it,
        token_iter end,
        collation_tailoring_interface & tailoring)
    {
        if (!require(it, end, token_kind::and_)) {
            assert(!"Something has gone horribly wrong.");
        }

        auto const strength = before_strength(it, end);

        auto lhs = next_cp_seq(it, end);
        if (lhs.empty()) {
            auto cp = logical_position(it, end);
            if (!cp) {
                throw one_token_parse_error(
                    "Expected code points or a logical position after '&' here",
                    it);
            }
            lhs.push_back(*cp);
        }

        if (!rule_chain(it, end, strength, lhs, tailoring)) {
            throw one_token_parse_error(
                "Expected one or more relation operators here", it);
        }
    }

    inline void option(
        token_iter & it,
        token_iter end,
        char const * const first,
        char const * const last,
        collation_tailoring_interface & tailoring,
        std::vector<int> const & line_starts,
        string_view filename)
    {
        auto const open_bracket_it = it;
        if (!require(it, end, token_kind::open_bracket)) {
            assert(!"Something has gone horribly wrong.");
        }

        auto identifier = next_identifier(it, end);
        if (!identifier) {
            // TODO: This list is far too short!
            throw one_token_parse_error(
                "Expected one 'import', 'optimize', or 'suppressContractions' "
                "here",
                it);
        } else if (*identifier == "import") {
            throw one_token_parse_error(
                "[import ...] is not supported; manually copy and paste into a "
                "single input",
                it);
        } else if (*identifier == "optimize") {
            if (tailoring.warnings_) {
                tailoring.warnings_(parse_diagnostic(
                    diag_kind::warning,
                    "[optimize ...] is not supported; ignoring...",
                    it->line(),
                    it->column(),
                    string_view(first, last - first),
                    line_starts,
                    filename));
            }

            auto const inner_open_bracket_it = it;
            if (!require(it, end, token_kind::open_bracket))
                throw one_token_parse_error("Expect open bracket here", it);

            it = std::find_if(it, end, [](token const & t) {
                return t.kind() == token_kind::close_bracket;
            });
            if (it == end) {
                throw two_token_parse_error(
                    "Expected close bracket here",
                    it,
                    "to match previous open bracket",
                    inner_open_bracket_it);
            }
            ++it;
        } else if (*identifier == "suppressContractions") {
            // TODO: Document that this only supports code points and "-" code
            // point ranges.
            auto const inner_open_bracket_it = it;
            if (!require(it, end, token_kind::open_bracket))
                throw one_token_parse_error("Expect open bracket here", it);

            cp_seq_t seq;
            auto range = next_cp_range(it, end);
            if (!range) {
                throw one_token_parse_error(
                    "Expect one or more code points or ranges of code points "
                    "here",
                    it);
            }

            auto append_cps = [&seq](cp_range_t r) {
                for (auto cp = r.first_, cp_end = r.last_; cp < cp_end; ++cp) {
                    seq.push_back(cp);
                }
            };

            append_cps(*range);

            while ((range = next_cp_range(it, end))) {
                append_cps(*range);
            }

            if (!require(it, end, token_kind::close_bracket)) {
                throw two_token_parse_error(
                    "Expected close bracket here",
                    it,
                    "to match previous open bracket",
                    inner_open_bracket_it);
            }

            // TODO: Record the suppression.
        } else {
            // TODO: This list is far too short!
            // TODO: This leaves out all the options ([strength 2], etc.)!
            throw one_token_parse_error(
                "Expected one 'import', 'optimize', or 'suppressContractions' "
                "here",
                it);
        }

        if (!require(it, end, token_kind::close_bracket)) {
            throw two_token_parse_error(
                "Expected close bracket here",
                it,
                "to match previous open bracket",
                open_bracket_it);
        }
    }

    inline bool parse_impl(
        char const * first,
        char const * const last,
        collation_tailoring_interface & tailoring,
        string_view filename = "")
    {
        auto const lat = lex(first, last, tailoring.errors_);

        try {
            auto it = lat.tokens_.begin();
            auto const end = lat.tokens_.end();
            while (it != end) {
                if (*it == token_kind::and_) {
                    rule(it, end, tailoring);
                } else if (*it == token_kind::open_bracket) {
                    option(
                        it,
                        end,
                        first,
                        last,
                        tailoring,
                        lat.line_starts_,
                        filename);
                } else {
                    throw one_token_parse_error(
                        "Illegal token; expected a rule ('& ...') or an "
                        "option/special ('[...]')",
                        it);
                }
            }
        } catch (two_token_parse_error const & e) {
            if (tailoring.errors_) {
                tailoring.errors_(parse_diagnostic(
                    diag_kind::error,
                    e.what(),
                    e.line(),
                    e.column(),
                    string_view(first, last - first),
                    lat.line_starts_,
                    filename));
                tailoring.errors_(parse_diagnostic(
                    diag_kind::note,
                    e.prev_msg(),
                    e.prev_line(),
                    e.prev_column(),
                    string_view(first, last - first),
                    lat.line_starts_,
                    filename));
            }
            return false;
        } catch (parse_error const & e) {
            if (tailoring.errors_) {
                tailoring.errors_(parse_diagnostic(
                    diag_kind::error,
                    e.what(),
                    e.line(),
                    e.column(),
                    string_view(first, last - first),
                    lat.line_starts_,
                    filename));
            }
            return false;
        }

        return true;
    }

}}}

#endif
