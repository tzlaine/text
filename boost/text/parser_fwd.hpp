#ifndef BOOST_TEXT_PARSER_FWD_HPP
#define BOOST_TEXT_PARSER_FWD_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/string.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>

#include <functional>


namespace boost { namespace text {

    using parser_diagnostic_callback = std::function<void(string const &)>;

    struct parse_error : std::exception
    {
        parse_error(string_view msg, int line, int column) :
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

    namespace detail {
        enum class token_kind {
            primary_before = static_cast<int>(collation_strength::primary),
            secondary_before = static_cast<int>(collation_strength::secondary),
            tertiary_before = static_cast<int>(collation_strength::tertiary),
            quaternary_before =
                static_cast<int>(collation_strength::quaternary),
            equal = static_cast<int>(collation_strength::identical),

            primary_before_star,
            secondary_before_star,
            tertiary_before_star,
            quaternary_before_star,
            equal_star,

            code_point,

            // Code point ranges like "x-y" appear after abbreviated relations,
            // but "-" is fine as a regular code point elsewhere.  The lexer
            // does not have the necessary context to distinguish these two
            // cases.  To resolve this, a "-" inside quotes or escaped ("\-") is
            // treated as a regular code point, but is otherwise a special dash
            // token.
            dash,

            and_,
            or_,
            slash,
            open_bracket,
            close_bracket,
            identifier
        };

#ifndef NDEBUG
        inline std::ostream & operator<<(std::ostream & os, token_kind kind)
        {
#    define CASE(x)                                                            \
    case token_kind::x: os << #x; break
            switch (kind) {
                CASE(code_point);
                CASE(dash);
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
                CASE(equal_star);
                CASE(identifier);
            }
#    undef CASE
            return os;
        }
#endif

        using cp_seq_t = container::small_vector<uint32_t, 8>;
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
        using collation_strength_callback =
            std::function<void(collation_strength)>;
        using variable_weighting_callback =
            std::function<void(variable_weighting)>;
        using l2_weight_order_callback = std::function<void(l2_weight_order)>;
        using case_level_callback = std::function<void(case_level_t)>;
        using case_first_callback = std::function<void(case_first_t)>;
        using suppression_callback = std::function<void(cp_seq_t const &)>;
        using reorder_callback =
            std::function<void(std::vector<reorder_group> const &)>;

        struct collation_tailoring_interface
        {
            // rules
            reset_callback reset_;
            relation_callback relation_;

            // options
            collation_strength_callback collation_strength_;
            variable_weighting_callback variable_weighting_;
            l2_weight_order_callback l2_weight_order_;
            case_level_callback case_level_;
            case_first_callback case_first_;

            // special purpose
            suppression_callback suppress_;
            reorder_callback reorder_;

            parser_diagnostic_callback errors_;
            parser_diagnostic_callback warnings_;
        };

        void parse(
            char const * first,
            char const * const last,
            collation_tailoring_interface & tailoring,
            string_view filename);
    }

}}

#endif
