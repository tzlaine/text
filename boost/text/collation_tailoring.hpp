#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/parser_fwd.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/container/small_vector.hpp>

#include <vector>


namespace boost { namespace text {

    namespace detail {
        struct collation_tailoring
        {
            struct rule
            {
                cp_seq_t reset_;
                cp_seq_t relation_;
                cp_seq_t prefix_;
                cp_seq_t extension_;
                collation_strength strength_;
                bool before_;
            };

            std::vector<rule> rules_;

            collation_strength collation_strength_;
            variable_weighting variable_weighting_;
            l2_weight_order l2_weight_order_;

            cp_seq_t suppressions_;
            std::vector<string> reorderings_;
        };
    }

    /** TODO */
    struct tailored_collation_element_table
    {
        explicit tailored_collation_element_table(/*TODO*/)
        {
            // TODO
        }

        detail::longest_collation_t
        extend_collation(detail::longest_collation_t prev, uint32_t cp) const
            noexcept
        {
            // TODO
            return detail::longest_collation_t{/*TODO*/};
        }

        template<typename Iter>
        detail::longest_collation_t
        longest_collation(Iter first, Iter last) const noexcept
        {
            // TODO
            return detail::longest_collation_t{/*TODO*/};
        }

    private:
        collation_strength collation_strength_;
        variable_weighting variable_weighting_;
        l2_weight_order l2_weight_order_;

        // TODO: Table data!
    };

    namespace detail {
        struct temp_table_element
        {
            cp_seq_t cps_;
            container::small_vector<compressed_collation_element, 4> ces_;
        };

        using temp_table = std::vector<temp_table_element>;

        temp_table make_temp_table()
        {
            std::vector<temp_table_element> retval;
            // TODO
            return retval;
        }

        void modify_table(
            temp_table & table,
            cp_seq_t const & reset,
            bool before,
            collation_strength strength,
            cp_seq_t const & relation,
            optional_cp_seq_t const & prefix,
            optional_cp_seq_t const & extension)
        {
            // TODO
        }

        void suppress(temp_table & table, uint32_t cp)
        {
            // TODO
        }

        void reorder_table(
            temp_table & table,
            std::vector<string> const & reorderings,
            bool simple)
        {
            // TODO
        }
    }

    /** TODO */
    inline tailored_collation_element_table
    make_tailored_collation_element_table(
        string_view tailoring,
        string_view tailoring_filename = "",
        parser_diagnostic_callback report_errors = parser_diagnostic_callback(),
        parser_diagnostic_callback report_warnings =
            parser_diagnostic_callback())
    {
        detail::cp_seq_t curr_reset;
        bool reset_is_before = false;

        detail::temp_table table = detail::make_temp_table();

        collation_strength strength_override;
        variable_weighting weighting_override;
        l2_weight_order order_override;

        detail::cp_seq_t suppressions;
        std::vector<string> reorderings;
        bool reorderings_are_simple = false;

        detail::collation_tailoring_interface callbacks = {
            [&](detail::cp_seq_t const & reset, bool before) {
                curr_reset = reset;
                reset_is_before = before;
            },
            [&](detail::relation_t const & rel) {
                detail::modify_table(
                    table,
                    curr_reset,
                    reset_is_before,
                    static_cast<collation_strength>(rel.op_),
                    rel.cps_,
                    rel.prefix_and_extension_.prefix_,
                    rel.prefix_and_extension_.extension_);
                curr_reset = rel.cps_;
                reset_is_before = false;
            },
            [&](collation_strength strength) {
                strength_override = strength;
            },
            [&](variable_weighting weighting) {
                weighting_override = weighting;
            },
            [&](l2_weight_order order) {
                order_override = order;
            },
            [&](detail::cp_seq_t const & suppressions_) {
                std::copy(
                    suppressions_.begin(),
                    suppressions_.end(),
                    std::back_inserter(suppressions));
            },
            [&](std::vector<string> && reorderings_, bool simple) {
                reorderings = std::move(reorderings_);
                reorderings_are_simple = simple;
            },
            report_errors,
            report_warnings};

        detail::parse(
            tailoring.begin(), tailoring.end(), callbacks, tailoring_filename);

        for (auto cp : suppressions) {
            detail::suppress(table, cp);
        }

        detail::reorder_table(table, reorderings, simple);

        return tailored_collation_element_table();
    }

}}

#endif
