#ifndef BOOST_TEXT_DETAIL_PARSER_HPP
#define BOOST_TEXT_DETAIL_PARSER_HPP

#include <boost/text/detail/lexer.hpp>
#include <boost/text/detail/lexer.hpp>

#include <boost/optional.hpp>


namespace boost { namespace text { namespace detail {

    // TODO: [last implicit], [first trailing], and [last trailing] are not supported.

    // TODO: U+FFFD..U+FFFF are not tailorable, and nothing can tailor to them.

    // code-point = ? A valid Unicode code point not in the range U+FFFD..U+FFFF ? ;
    // nfd-inert-cp = ? A valid Unicode code point not in the range U+FFFD..U+FFFF with ccc=0 ? ;

    struct collation_tailoring
    {};

    using cp_seq_t = std::vector<uint32_t>;
    using optional_cp_seq_t = optional<cp_seq_t>;

    struct scoped_token_iter
    {
        scoped_token_iter(token_iter & it) :
            it_(it),
            initial_it_(it),
            released_(false)
        {}
        ~scoped_token_iter()
        {
            if (!released_)
                it_ = initial_it;
        }
        void release() { released_ = true; }

    private:
        token_iter & it_;
        token_iter initial_it_;
        bool released_;
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
    inline bool require(token_iter & it, token_iter end) { return true; }
    template<typename T, typename... Ts>
    inline bool require(token_iter & it, token_iter end, T t, Ts... ts)
    {
        scoped_token_iter rollback(it);
        if (require(it, end) && require(it, end, ts...)) {
            rollback.release();
            return true;
        }
        return false;
    }

    inline optional<kind> next_tok(token_iter & it, token_iter end)
    {
        if (it == end)
            return {};
        if (it->kind() == token_kind::identifier ||
            it->kind() == token_kind::code_point)
            return {};
        return (it++)->kind();
    }
    inline optional<string> next_string(token_iter & it, token_iter end)
    {
        if (it == end)
            return {};
        if (it->kind() != token_kind::identifier)
            return {};
        return (it++)->identifier();
    }
    inline optional<uint32_t> next_cp(token_iter & it, token_iter end)
    {
        if (it == end)
            return {};
        if (it->kind() != token_kind::code_point)
            return {};
        return (it++)->code_point();
    }

    struct cp_range_t
    {
        uint32_t lo_;
        uint32_t hi_;
    };

    // cp-range = nfd-inert-cp | nfd-inert-cp, "-", nfd-inert-cp ;
    inline optional<cp_range_t> next_cp_range(token_iter & it, token_iter end)
    {
        cp_range_t cps;

        auto const lo = next_cp(it, end);
        if (!lo)
            return {};

        cps_lo_ = lo;
        cps.hi_ = cps.lo_ + 1;

        auto const dash = next_cp(it, end);
        if (!dash || *dash != '-')
            return cps;

        auto const hi = next(it, end);
        if (!hi) {
            // TODO: Error!
        }

        cps.hi_ = hi;

        return cps;
    }

    // cp-sequence = code-point, {code-point} ;
    inline cp_seq_t next_cp_seq(token_iter & it, token_iter end)
    {
        cp_seq_t retval;
        optional<uint32_t> cp;
        do {
            if ((cp = next_cp(it, end)))
                retval.push_back(*cp);
        } while (cp);
        return retval;
    }

    // before-strength = "[", "before", ("1" | "2" | "3"), "]" ;
    inline int before_strength(token_iter & it, token_iter end)
    {
        scoped_token_iter rollback(it);

        if (!require(it, end, token_kind::open_bracket))
            return 0;

        if (!require(it, end, "before"))
            return 0;

        int retval = 0;
        if (require(it, end, "1")) {
            retval = 1;
        } else if (require(it, end, "2")) {
            retval = 2;
        } else if (require(it, end, "3")) {
            retval = 3;
        } else {
            // TODO: Error!
            return 0;
        }

        if (!require(it, end, token_kind::close_bracket)) {
            // TODO: Error!
        }

        rollback.release();

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
        scoped_token_iter rollback(it);

        if (!require(it, end, token_kind::open_bracket))
            return 0;

        uint32_t position;
        if (require(it, end, "first", "tertiary", "ignorable"))
            position = TODO;
        else if (require(it, end, "last", "tertiary", "ignorable"))
            position = TODO;
        else if (require(it, end, "first", "secondary", "ignorable"))
            position = TODO;
        else if (require(it, end, "last", "secondary", "ignorable"))
            position = TODO;
        else if (require(it, end, "first", "primary", "ignorable"))
            position = TODO;
        else if (require(it, end, "last", "primary", "ignorable"))
            position = TODO;
        else if (require(it, end, "first", "variable"))
            position = TODO;
        else if (require(it, end, "last", "variable"))
            position = TODO;
        else if (require(it, end, "first", "regular"))
            position = TODO;
        else if (require(it, end, "last", "regular"))
            position = TODO;
        else if (require(it, end, "first", "implicit"))
            position = TODO;
        else if (require(it, end, "last", "implicit"))
            position = TODO;
        else if (require(it, end, "first", "trailing"))
            position = TODO;
        else if (require(it, end, "last", "trailing"))
            position = TODO;
        else
            return {};

        if (!require(it, end, token_kind::close_bracket)) {
            // TODO: Error!
        }

        rollback.release();

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
            // TODO: Error!
        }
        return optional_cp_seq_t(std::move(seq));
    }

    struct prefix_and_extension_t
    {
        optional_cp_seq_t prefix_;
        optional_cp_seq_t extension_;
    };

    inline prefix_and_extension_t
    prefix_and_extension(token_iter & it, token_iter end)
    {
        prefix_and_extension_t retval;
        if (auto prefix_seq = op_and_seq(it, end, toke_kind::or_)) {
            retval.prefix = std::move(prefix_seq);
            if ((auto extension_seq = op_and_seq(it, end, toke_kind::slash)))
                retval.prefix = std::move(extension_seq);
        } else if (auto extension_seq = op_and_seq(it, end, toke_kind::slash)) {
            retval.prefix = std::move(extension_seq);
            if ((prefix_seq = op_and_seq(it, end, toke_kind::or_)))
                retval.prefix = std::move(prefix_seq);
        }
        return retval;
    }

    // relation-op = "=" | "<" | "<<" | "<<<" ;
    // relation = relation-op, cp-sequence, (([prefix], extension) |
    // ([extension], prefix)) ;
    struct relation_t
    {
        token_kind op_;
        cp_seq_t cps_;
        prefix_and_extension_t prefix_and_extension_;
    };

    inline optional<relation_t> relation(token_iter & it, token_iter end)
    {
        auto const op = next_tok(it, end);
        if (!op || op < token_kind::equal || token_kind::quaternary_before < op)
            return {};

        auto seq = next_cp_seq(it, end);
        if (!seq) {
            // TODO: Error!
        }

        return relation_t{op, std::move(seq), prefix_and_extension(it, end)};
    }

    // TODO: There seems to be no way to differentiate a quoted or escaped
    // '-' from a range-dash in an abreviated ordering.  The lexer should
    // address this.

    // reset = cp-sequence | logical-position ;
    // before-rule = "&", before-strength, reset, relation ;
    inline bool before_rule(
        token_iter & it,
        token_iter end,
        int strength,
        cp_seq_t const & reset,
        collation_tailoring & tailoring)
    {
        return false;
    }

    // abberviated-op = "<*", "<<*", "<<<*" ;
    // abberviated-rule = "&", reset, abbreviated-op, cp-range, {cp-range} ;
    inline bool abbreviated_rule(
        token_iter & it,
        token_iter end,
        cp_seq_t const & reset,
        collation_tailoring & tailoring)
    {
        scoped_token_iter rollback(it);

        auto relation = next_tok(it, end);
        if (!relation || *relation < token_kind::primary_before_star ||
            token_kind::quaternary_before_star < *relation) {
            return false;
        }

        auto check_ccc_0 = [](cp_range_t r) {
            for (auto cp = r.first_, cp_end = r.last_; cp < cp_end; ++cp) {
                if (!ccc(cp)) {
                    // TODO: Error!
                }
            }
        };

        auto lhs_range = next_cp_range(it, end);
        if (!lhs_range)
            // TODO: Error!
            return false;

        check_ccc_0(*lhs_range);

        // TODO: Record "lhs relation cps" somewhere.
        rollback.release();

        while (lhs_range) {
            if ((lhs_range = next_cp_range(it, end)))
                check_ccc_0(*lhs_range);
            // TODO: Record "lhs relation cps" somewhere.
        }

        return true;
    }

    // after-rule = "&", reset, relation, {relation} ;
    inline bool after_rule(
        token_iter & it,
        token_iter end,
        cp_seq_t const & reset,
        collation_tailoring & tailoring)
    {
        scoped_token_iter rollback(it);

        auto rel = relation(it, end);
        if (!rel)
            return false;

        // TODO: Record "reset relation" somehwere.
        rollback.release();

        cp_seq_t current_lhs(std::move(rel->cps_));
        while ((rel = relation(it, end))) {
            // TODO: Record "current_lhs relation" somehwere.
            current_lhs.swap(rel->cps_);
        }

        return true;
    }

    // rule = before-rule | abbreviated-rule | after-rule ;
    inline void
    rule(token_iter & it, token_iter end, collation_tailoring & tailoring)
    {
        if (!require(it, end, token_kind::and_))
            assert(!"Something has gone horribly wrong.");

        auto const strength = before_strength(it, end);

        auto lhs = next_cp_seq(it, end);
        if (lhs.empty()) {
            auto cp = logical_position(it, end);
            if (!cp) {
                // TODO: Error!
            }
            lhs.push_back(*cp);
        }

        if (strength) {
            if (!before_rule(it, end, strength, lhs, tailoring)) {
                // Error!
            }
        } else {
            if (!abbreviated_rule(it, end, lhs, tailoring) &&
                !after_rule(it, end, lhs, tailoring)) {
                // TODO: Error!
            }
        }
    }

    inline void
    option(token_iter & it, token_iter end, collation_tailoring & tailoring)
    {
        // TODO
    }

    inline collation_tailoring parse_impl(
        char const * first,
        char const * const last,
        parser_diagnostic_callback errors = parser_diagnostic_callback(),
        parser_diagnostic_callback warnings = parser_diagnostic_callback())
    {
        collation_tailoring retval;

        auto const lat = lex(first, last, errors);

        auto it = lat.tokens_.begin();
        auto const end = lat.tokens_.end();
        while (it != end) {
            if (*first == token_kind::and_) {
                rule(it, end, retval);
            } else if (*first == token_kind::open_bracket) {
                option(it, end, retval);
            } else {
                // TODO: Error!
            }
        }

        return retval;
    }

}}}

#endif
