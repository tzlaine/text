#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/parser_fwd.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/container/small_vector.hpp>

#include <numeric>
#include <vector>


namespace boost { namespace text {

    namespace detail {
        struct nonsimple_script_reorder
        {
            compressed_collation_element first_;
            compressed_collation_element last_;
            uint32_t lead_byte_mask_;
        };
    }

    /** TODO */
    struct tailored_collation_element_table
    {
        tailored_collation_element_table(
            std::unordered_set<detail::collation_trie_node> &&
                collation_initial_nodes,
            std::vector<detail::collation_trie_node> && collation_trie_nodes,
            container::static_vector<
                detail::nonsimple_script_reorder,
                4> const & nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders,
            optional<collation_strength> strength,
            optional<variable_weighting> weighting,
            optional<l2_weight_order> l2_order) :
            collation_initial_nodes_(std::move(collation_initial_nodes)),
            collation_trie_nodes_(std::move(collation_trie_nodes)),
            nonsimple_reorders_(nonsimple_reorders),
            simple_reorders_(simple_reorders),
            strength_(strength),
            weighting_(weighting),
            l2_order_(l2_order)
        {}

        template<typename Iter>
        detail::longest_collation_t
        longest_collation(Iter first, Iter last) const noexcept
        {
            return detail::longest_collation(
                first,
                last,
                collation_initial_nodes_,
                &collation_trie_nodes_[0]);
        }

        detail::longest_collation_t
        extend_collation(detail::longest_collation_t prev, uint32_t cp) const
            noexcept
        {
            return detail::extend_collation(
                prev, cp, &collation_trie_nodes_[0]);
        }

        uint32_t lead_byte(detail::compressed_collation_element cce) const
            noexcept
        {
            auto const it = std::find_if(
                nonsimple_reorders_.begin(),
                nonsimple_reorders_.end(),
                [cce](detail::nonsimple_script_reorder reorder) {
                    return reorder.first_ <= cce && cce < reorder.last_;
                });
            if (it != nonsimple_reorders_.end())
                return it->lead_byte_mask_;
            auto const masked_primary = cce.l1_ & 0xff000000;
            return simple_reorders_[masked_primary >> 24];
        }

        optional<collation_strength> strength() const noexcept
        {
            return strength_;
        }
        optional<variable_weighting> weighting() const noexcept
        {
            return weighting_;
        }
        optional<l2_weight_order> l2_order() const noexcept
        {
            return l2_order_;
        }

    private:
        std::unordered_set<detail::collation_trie_node>
            collation_initial_nodes_;
        std::vector<detail::collation_trie_node> collation_trie_nodes_;

        container::static_vector<detail::nonsimple_script_reorder, 4>
            nonsimple_reorders_;
        std::array<uint32_t, 256> simple_reorders_;

        optional<collation_strength> strength_;
        optional<variable_weighting> weighting_;
        optional<l2_weight_order> l2_order_;
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

        // TODO: Drop support for this?
        void suppress(temp_table & table, uint32_t cp)
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

        optional<collation_strength> strength_override;
        optional<variable_weighting> weighting_override;
        optional<l2_weight_order> l2_order_override;

        detail::cp_seq_t suppressions;

        container::static_vector<detail::nonsimple_script_reorder, 4>
            nonsimple_reorders;
        std::array<uint32_t, 256> simple_reorders;
        std::iota(simple_reorders.begin(), simple_reorders.end(), 0);
        for (auto & x : simple_reorders) {
            x <<= 24;
        }

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
            [&](collation_strength strength) { strength_override = strength; },
            [&](variable_weighting weighting) {
                weighting_override = weighting;
            },
            [&](l2_weight_order l2_order) { l2_order_override = l2_order; },
            [&](detail::cp_seq_t const & suppressions_) {
                std::copy(
                    suppressions_.begin(),
                    suppressions_.end(),
                    std::back_inserter(suppressions));
            },
            [&](std::vector<detail::reorder_group> const & reorder_groups) {
                uint32_t curr_reorder_lead_byte =
                    detail::g_reorder_groups[0].first_.l1_ & 0xff000000;
                for (auto const & group : reorder_groups) {
                    if (group.simple_) {
                        // TODO: Apply compression; throw from here if
                        // everything does not fit.
                        uint32_t const group_first =
                            group.first_.l1_ & 0xff000000;
                        for (uint32_t byte = group_first,
                                      end = group.last_.l1_ & 0xff000000;
                             byte < end;
                             byte += 0x01000000) {
                            simple_reorders[byte] = curr_reorder_lead_byte;
                            curr_reorder_lead_byte += 0x01000000;
                        }
                    } else {
                        nonsimple_reorders.push_back(
                            detail::nonsimple_script_reorder{
                                group.first_,
                                group.last_,
                                curr_reorder_lead_byte});
                        curr_reorder_lead_byte += 0x01000000;
                    }
                }
            },
            report_errors,
            report_warnings};

        detail::parse(
            tailoring.begin(), tailoring.end(), callbacks, tailoring_filename);

        for (auto cp : suppressions) {
            detail::suppress(table, cp);
        }

        // TODO: Build these from the temp table.
        std::unordered_set<detail::collation_trie_node> collation_initial_nodes;
        std::vector<detail::collation_trie_node> collation_trie_nodes;

        return tailored_collation_element_table(
            std::move(collation_initial_nodes),
            std::move(collation_trie_nodes),
            nonsimple_reorders,
            simple_reorders,
            strength_override,
            weighting_override,
            l2_order_override);
    }

}}

#endif
