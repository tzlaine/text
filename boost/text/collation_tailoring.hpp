#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/parser_fwd.hpp>
#include <boost/text/detail/collation_data.hpp>

#include <boost/container/small_vector.hpp>

#include <numeric>
#include <vector>


namespace boost { namespace text {

    struct tailored_collation_element_table;

    namespace detail {

        struct nonsimple_script_reorder
        {
            compressed_collation_element first_;
            compressed_collation_element last_;
            uint32_t lead_byte_;
        };

        using nonsimple_reorders_t = container::
            static_vector<nonsimple_script_reorder, g_reorder_groups.size()>;

        uint32_t lead_byte(
            compressed_collation_element cce,
            nonsimple_reorders_t const & nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders) noexcept
        {
            // TODO: lower_bound?
            auto const it = std::find_if(
                nonsimple_reorders.begin(),
                nonsimple_reorders.end(),
                [cce](detail::nonsimple_script_reorder reorder) {
                    return reorder.first_ <= cce && cce < reorder.last_;
                });
            if (it != nonsimple_reorders.end())
                return it->lead_byte_;
            auto const masked_primary = cce.l1_ & 0xff000000;
            return simple_reorders[masked_primary >> 24];
        }

        tailored_collation_element_table make_tailored_collation_element_table(
            string_view tailoring,
            string_view tailoring_filename = "",
            parser_diagnostic_callback report_errors =
                parser_diagnostic_callback(),
            parser_diagnostic_callback report_warnings =
                parser_diagnostic_callback());
    }

    /** TODO */
    struct tailored_collation_element_table
    {
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

        detail::compressed_collation_element const *
        collation_elements_begin() const noexcept
        {
            return &collation_elements_[0];
        }

        uint32_t lead_byte(detail::compressed_collation_element cce) const
            noexcept
        {
            return detail::lead_byte(
                cce, nonsimple_reorders_, simple_reorders_);
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
        tailored_collation_element_table(
            std::vector<detail::compressed_collation_element> &&
                collation_elements,
            std::unordered_set<detail::collation_trie_node> &&
                collation_initial_nodes,
            std::vector<detail::collation_trie_node> && collation_trie_nodes,
            detail::nonsimple_reorders_t && nonsimple_reorders,
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

        friend tailored_collation_element_table
        make_tailored_collation_element_table(
            string_view tailoring,
            string_view tailoring_filename,
            parser_diagnostic_callback report_error,
            parser_diagnostic_callback report_warnings);

        std::vector<detail::compressed_collation_element> collation_elements_;
        std::unordered_set<detail::collation_trie_node>
            collation_initial_nodes_;
        std::vector<detail::collation_trie_node> collation_trie_nodes_;

        detail::nonsimple_reorders_t nonsimple_reorders_;
        std::array<uint32_t, 256> simple_reorders_;

        optional<collation_strength> strength_;
        optional<variable_weighting> weighting_;
        optional<l2_weight_order> l2_order_;
    };

    namespace detail {
        struct temp_table_element
        {
            using ces_t =
                container::small_vector<compressed_collation_element, 4>;

            cp_seq_t cps_;
            ces_t ces_;

            // Populated during trie-ification.
            compressed_collation_elements linearized_ces_;
        };

        using temp_table = std::vector<temp_table_element>;

        void temp_table_subtree(
            temp_table & table, collation_trie_node node, cp_seq_t & cps)
        {
            cps.push_back(node.cp_);
            if (node.match()) {
                table.push_back(
                    temp_table_element{cps,
                                       temp_table_element::ces_t(
                                           node.collation_elements_.begin(
                                               g_collation_elements_first),
                                           node.collation_elements_.end(
                                               g_collation_elements_first))});
            }
            if (!node.leaf()) {
                for (auto it = node.begin(g_collation_trie_nodes),
                          end = node.end(g_collation_trie_nodes);
                     it != end;
                     ++it) {
                    temp_table_subtree(table, *it, cps);
                }
            }
            cps.pop_back();
        }

        temp_table make_temp_table()
        {
            std::vector<temp_table_element> retval;
            for (auto node : g_collation_initial_nodes) {
                cp_seq_t cps;
                temp_table_subtree(retval, node, cps);
            }
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
}}

namespace std {
    template<>
    struct hash<boost::text::detail::temp_table_element::ces_t>
    {
        using argument_type = boost::text::detail::temp_table_element::ces_t;
        using result_type = std::size_t;
        result_type operator()(argument_type const & ces) const noexcept
        {
            result_type retval = ces.size();
            for (auto ce : ces) {
                retval = combine(
                    retval,
                    (result_type(ce.l1_) << 32) | (ce.l2_ << 16) | ce.l1_);
            }
            return retval;
        }

    private:
        static result_type combine(result_type seed, result_type value) noexcept
        {
            return seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };
}

namespace boost { namespace text {

    namespace detail {

        void temp_table_to_trie(
            temp_table & table,
            nonsimple_reorders_t const & nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders,
            std::vector<compressed_collation_element> collation_elements,
            std::unordered_set<collation_trie_node> & collation_initial_nodes,
            std::vector<collation_trie_node> & collation_trie_nodes)
        {
            std::unordered_map<
                temp_table_element::ces_t,
                compressed_collation_elements>
                already_linearized;
            for (auto & e : table) {
                for (auto ce : e.ces_) {
                    auto const lead_byte_ =
                        lead_byte(ce, nonsimple_reorders, simple_reorders);
                    ce.l1_ &= 0x00ffffff;
                    ce.l1_ |= lead_byte_;
                }
                auto const it = already_linearized.find(e.ces_);
                if (it == already_linearized.end()) {
                    e.linearized_ces_.first_ = collation_elements.size();
                    collation_elements.insert(
                        collation_elements.end(), e.ces_.begin(), e.ces_.end());
                    e.linearized_ces_.last_ = collation_elements.size();
                }
            }

            for (auto const e : table) {
                collation_trie_node node{e.cps_.front(), 0, 0};
                auto it = collation_initial_nodes.find(node);
                if (it == collation_initial_nodes.end()) {
                    if (e.cps_.size() == 1u)
                        node.collation_elements_ = e.linearized_ces_;
                    it = collation_initial_nodes.insert(node).first;
                } else {
                    node = *it;
                }

                // TODO
                
            }
        }
    }

    /** TODO */
    inline tailored_collation_element_table
    make_tailored_collation_element_table(
        string_view tailoring,
        string_view tailoring_filename,
        parser_diagnostic_callback report_errors,
        parser_diagnostic_callback report_warnings)
    {
        detail::cp_seq_t curr_reset;
        bool reset_is_before = false;

        detail::temp_table table = detail::make_temp_table();

        optional<collation_strength> strength_override;
        optional<variable_weighting> weighting_override;
        optional<l2_weight_order> l2_order_override;

        detail::cp_seq_t suppressions;

        detail::nonsimple_reorders_t nonsimple_reorders;
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
                    (detail::g_reorder_groups[0].first_.l1_ & 0xff000000) -
                    0x01000000;
                bool prev_group_compressible = false;
                detail::compressed_collation_element prev_group_last = {
                    0xffffffff};
                for (auto const & group : reorder_groups) {
                    bool const compress = group.compressible_ &&
                                          prev_group_compressible &&
                                          prev_group_last < group.last_;
                    if (!compress)
                        curr_reorder_lead_byte += 0x01000000;

                    if (detail::implicit_weights_final_lead_byte <
                        curr_reorder_lead_byte) {
                        throw parse_error(
                            "It was not possible to tailor the "
                            "collation in the way you requested.  "
                            "Try using fewer groups in '[reorder "
                            "...]'.",
                            0,
                            0);
                    }

                    if (!compress && group.simple_) {
                        uint32_t const group_first =
                            group.first_.l1_ & 0xff000000;
                        for (uint32_t byte = group_first,
                                      end = group.last_.l1_ & 0xff000000;
                             byte < end;
                             byte += 0x01000000) {
                            simple_reorders[byte] = curr_reorder_lead_byte;
                        }
                    } else {
                        nonsimple_reorders.push_back(
                            detail::nonsimple_script_reorder{
                                group.first_,
                                group.last_,
                                curr_reorder_lead_byte});
                    }
                    prev_group_compressible = group.compressible_;
                    prev_group_last = group.last_;
                }
            },
            report_errors,
            report_warnings};

        detail::parse(
            tailoring.begin(), tailoring.end(), callbacks, tailoring_filename);

        for (auto cp : suppressions) {
            detail::suppress(table, cp);
        }

        std::vector<detail::compressed_collation_element> collation_elements;
        std::unordered_set<detail::collation_trie_node> collation_initial_nodes;
        std::vector<detail::collation_trie_node> collation_trie_nodes;
        temp_table_to_trie(
            table,
            nonsimple_reorders,
            simple_reorders,
            collation_elements,
            collation_initial_nodes,
            collation_trie_nodes);

        return tailored_collation_element_table(
            std::move(collation_elements),
            std::move(collation_initial_nodes),
            std::move(collation_trie_nodes),
            std::move(nonsimple_reorders),
            simple_reorders,
            strength_override,
            weighting_override,
            l2_order_override);
    }

}}

#endif
