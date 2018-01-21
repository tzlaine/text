#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/parser_fwd.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/detail/collation_data.hpp>

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

        struct temp_table_element
        {
            using ces_t =
                container::small_vector<compressed_collation_element, 4>;

            cp_seq_t cps_;
            ces_t ces_;

            bool tailored_ = false;
        };

        inline bool operator<(
            temp_table_element const & lhs,
            temp_table_element::ces_t const & rhs) noexcept
        {
            auto const pair = algorithm::mismatch(
                lhs.ces_.begin(), lhs.ces_.end(), rhs.begin(), rhs.end());
            if (pair.first == lhs.ces_.end()) {
                if (pair.second == rhs.end())
                    return false;
                return true;
            } else {
                if (pair.second == rhs.end())
                    return false;
                return *pair.first < *pair.second;
            }
        }

        inline bool operator<(
            temp_table_element const & lhs,
            temp_table_element const & rhs) noexcept
        {
            return lhs < rhs.ces_;
        }

        using temp_table_t = std::vector<temp_table_element>;

        struct logical_positions_t
        {
            temp_table_element::ces_t & operator[](uint32_t symbolic)
            {
                return cces_[symbolic - first_tertiary_ignorable];
            }
            std::array<temp_table_element::ces_t, 11> cces_;
        };

        void modify_table(
            tailored_collation_element_table & table,
            temp_table_t & temp_table,
            logical_positions_t & logical_positions,
            cp_seq_t reset,
            bool before,
            collation_strength strength,
            cp_seq_t relation,
            optional_cp_seq_t const & prefix,
            optional_cp_seq_t const & extension);

        void finalize_table(
            temp_table_t & temp_table,
            nonsimple_reorders_t && nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders,
            optional<collation_strength> strength_override,
            optional<variable_weighting> weighting_override,
            optional<l2_weight_order> l2_order_override,
            tailored_collation_element_table & table);
    }

    /** TODO */
    struct tailored_collation_element_table
    {
        detail::collation_trie_t const & trie() const noexcept { return trie_; }

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
        tailored_collation_element_table() {}

        void add_temp_tailoring(
            detail::cp_seq_t const & cps,
            detail::temp_table_element::ces_t const & ces)
        {
            detail::compressed_collation_elements value{
                static_cast<uint16_t>(collation_elements_.size())};
            collation_elements_.insert(
                collation_elements_.end(), ces.begin(), ces.end());
            value.last_ = collation_elements_.size();
            trie_.insert_or_assign(cps, value);
        }

        std::vector<detail::compressed_collation_element> collation_elements_;
        detail::collation_trie_t trie_;

        detail::nonsimple_reorders_t nonsimple_reorders_;
        std::array<uint32_t, 256> simple_reorders_;

        optional<collation_strength> strength_;
        optional<variable_weighting> weighting_;
        optional<l2_weight_order> l2_order_;

        friend void detail::modify_table(
            tailored_collation_element_table & table,
            detail::temp_table_t & temp_table,
            detail::logical_positions_t & logical_positions,
            detail::cp_seq_t reset,
            bool before,
            collation_strength strength,
            detail::cp_seq_t relation,
            detail::optional_cp_seq_t const & prefix,
            detail::optional_cp_seq_t const & extension);

        friend void detail::finalize_table(
            detail::temp_table_t & temp_table,
            detail::nonsimple_reorders_t && nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders,
            optional<collation_strength> strength_override,
            optional<variable_weighting> weighting_override,
            optional<l2_weight_order> l2_order_override,
            tailored_collation_element_table & table);

        friend tailored_collation_element_table
        make_tailored_collation_element_table(
            string_view tailoring,
            string_view tailoring_filename,
            parser_diagnostic_callback report_errors,
            parser_diagnostic_callback report_warnings);
    };

    namespace detail {
        inline temp_table_t make_temp_table()
        {
            temp_table_t retval;
            retval.reserve(g_default_collation_trie.size());
            for (auto element : g_default_collation_trie) {
                retval.resize(retval.size() + 1);
                retval.back().cps_.assign(
                    element.key.begin(), element.key.end());
                retval.back().ces_.assign(
                    element.value.begin(g_collation_elements_first),
                    element.value.end(g_collation_elements_first));
            }
            std::sort(retval.begin(), retval.end());
            return retval;
        }

        inline temp_table_element::ces_t get_ces(
            cp_seq_t initial_cps,
            tailored_collation_element_table const & table) noexcept
        {
            temp_table_element::ces_t retval;

            cp_seq_t fcc_cps;
            pseudonormalize_to_fcc(initial_cps, std::back_inserter(fcc_cps));

            container::small_vector<collation_element, 1024> ces;
            s2(fcc_cps.begin(),
               fcc_cps.end(),
               variable_weighting::non_ignorable,
               ces,
               &table);

            retval.resize(ces.size());
            std::transform(
                ces.begin(),
                ces.end(),
                retval.begin(),
                [](collation_element ce) {
                    assert(!ce.l4_);
                    return compressed_collation_element{ce.l1_, ce.l2_, ce.l3_};
                });

            return retval;
        }

        template<typename Iter>
        Iter last_ce_at_least_strength(
            Iter first, Iter last, collation_strength strength) noexcept
        {
            for (auto it = last; it != first;) {
                if (cce_strength(*--it) <= strength)
                    return it;
            }
            return last;
        }

        // http://www.unicode.org/reports/tr35/tr35-collation.html#Orderings
        inline void modify_table(
            tailored_collation_element_table & table,
            temp_table_t & temp_table,
            logical_positions_t & logical_positions,
            cp_seq_t reset,
            bool before,
            collation_strength strength,
            cp_seq_t relation,
            optional_cp_seq_t const & prefix,
            optional_cp_seq_t const & extension)
        {
            // http://userguide.icu-project.org/collation/customization (see
            // the bit titled "Prefix Example:").
            temp_table_element::ces_t prefix_ces;
            if (prefix) {
                reset.insert(reset.end(), prefix->begin(), prefix->end());
                relation.insert(relation.end(), prefix->begin(), prefix->end());
            }

            temp_table_element::ces_t reset_ces;
            if (reset.size() == 1u && first_tertiary_ignorable <= reset[0] &&
                reset[0] <= first_implicit) {
                reset_ces = logical_positions[reset[0]];
                // TODO: Elsewhere, update these logical positions.
            } else {
                reset_ces = get_ces(reset, table);
            }
            auto table_it = temp_table.end();

            if (before) {
                auto const ces_it = last_ce_at_least_strength(
                    reset_ces.begin(), reset_ces.end(), strength);
                auto const ce = ces_it == reset_ces.end()
                                    ? compressed_collation_element{0, 0, 0}
                                    : *ces_it;
                reset_ces.clear();
                reset_ces.push_back(ce);
                auto it = std::lower_bound(
                    temp_table.begin(), temp_table.end(), reset_ces);
                assert(it != temp_table.begin());
                while (it != temp_table.begin()) {
                    --it;
                    auto const curr_ce = it->ces_[0];
                    if (curr_ce.l1_ != ce.l1_) {
                        table_it = it;
                        break;
                    } else if (
                        collation_strength::secondary <= strength &&
                        curr_ce.l2_ != ce.l2_) {
                        table_it = it;
                        break;
                    } else if (
                        collation_strength::tertiary <= strength &&
                        curr_ce.l3_ != ce.l3_) {
                        table_it = it;
                        break;
                    }
                }
                if (table_it == temp_table.end()) {
                    // TODO: Could not implement this.  Throw here, catch it in
                    // the parser, and produce a reasonable error message baed
                    // on its state.
                }
                reset_ces.erase(std::next(ces_it), reset_ces.end());
                reset_ces.insert(
                    ces_it, table_it->ces_.begin(), table_it->ces_.end());
            } else {
                table_it = std::lower_bound(
                    temp_table.begin(), temp_table.end(), reset_ces);
            }

            // TODO: Throw if table_it points to an element outside the
            // tailorable zone.

            if (extension) {
                auto const extension_ces = get_ces(*extension, table);
                reset_ces.insert(
                    reset_ces.end(),
                    extension_ces.begin(),
                    extension_ces.end());
            }

            if (strength != collation_strength::identical) {
                // "Find the last collation element whose strength is at least
                // as great as the strength of the operator. For example, for <<
                // find the last primary or secondary CE. This CE will be
                // modified; all following CEs should be removed. If there is no
                // such CE, then reset the collation elements to a single
                // completely-ignorable CE."
                auto ces_it = last_ce_at_least_strength(
                    reset_ces.begin(), reset_ces.end(), strength);
                if (ces_it != reset_ces.end())
                    reset_ces.erase(std::next(ces_it), reset_ces.end());
                if (ces_it == reset_ces.end()) {
                    ces_it = reset_ces.insert(
                        ces_it, compressed_collation_element{0, 0, 0});
                }
                auto & ce = *ces_it;

                // "Increment the collation element weight corresponding to the
                // strength of the operator. For example, for << increment the
                // secondary weight."
                switch (strength) {
                case collation_strength::primary: ++ce.l1_;
                    ce.l2_ = common_l2_weight_compressed;
                    ce.l3_ = common_l3_weight_compressed;
                    break;
                case collation_strength::secondary:
                    // TODO: "The new weight must be less than the next weight
                    // for the same combination of higher-level weights of any
                    // collation element according to the current state."
                    // TODO: "Weights must be allocated in accordance with the
                    // UCA well-formedness conditions."
                    ++ce.l2_;
                    ce.l3_ = common_l3_weight_compressed;
                    break;
                case collation_strength::tertiary:
                    // TODO: "The new weight must be less than the next weight
                    // for the same combination of higher-level weights of any
                    // collation element according to the current state."
                    // TODO: "Weights must be allocated in accordance with the
                    // UCA well-formedness conditions."
                    ++ce.l3_;
                    break;
                default:
                    // TODO: Throw (probably).  If so, this limitation should
                    // be documented; make sure none of the existing tailoring
                    // files use quaternary tailorings before doing this!
                    break;
                }
            }

            table.add_temp_tailoring(relation, reset_ces);
            temp_table_element element;
            element.cps_ = std::move(relation);
            element.ces_ = std::move(reset_ces);
            element.tailored_ = true;
            table_it = temp_table.insert(table_it, std::move(element));

            // TODO: I don't think this is necessary; try removing it and
            // seeing if it breaks the tests (once the tests exist).
            auto same_key = [&relation](temp_table_element const & e) {
                return e.cps_ == relation;
            };
            auto remove_it =
                std::remove_if(temp_table.begin(), table_it, same_key);
            if (remove_it == table_it) {
                remove_it = std::remove_if(
                    std::next(table_it), temp_table.end(), same_key);
            }
            if (remove_it != temp_table.end()) {
                assert(std::distance(remove_it, temp_table.end()) == 1);
                temp_table.erase(remove_it);
            }
        }

        // TODO: Drop support for this?  No! Just add table entries for any
        // key that starts with cp that consists of all the CEs that the key
        // would have had generated without the contraction.
        inline void suppress(temp_table_t & table, uint32_t cp)
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

        inline void finalize_table(
            temp_table_t & temp_table,
            nonsimple_reorders_t && nonsimple_reorders,
            std::array<uint32_t, 256> const & simple_reorders,
            optional<collation_strength> strength_override,
            optional<variable_weighting> weighting_override,
            optional<l2_weight_order> l2_order_override,
            tailored_collation_element_table & table)
        {
            table.collation_elements_.clear();
            table.strength_ = strength_override;
            table.weighting_ = weighting_override;
            table.l2_order_ = l2_order_override;

            std::unordered_map<
                temp_table_element::ces_t,
                compressed_collation_elements>
                already_linearized;
            for (auto & e : temp_table) {
                if (!e.tailored_)
                    continue;

                for (auto ce : e.ces_) {
                    auto const lead_byte_ =
                        lead_byte(ce, nonsimple_reorders, simple_reorders);
                    ce.l1_ &= 0x00ffffff;
                    ce.l1_ |= lead_byte_;
                }

                compressed_collation_elements linearized_ces;
                auto const it = already_linearized.find(e.ces_);
                if (it == already_linearized.end()) {
                    linearized_ces.first_ = table.collation_elements_.size();
                    table.collation_elements_.insert(
                        table.collation_elements_.end(),
                        e.ces_.begin(),
                        e.ces_.end());
                    linearized_ces.last_ = table.collation_elements_.size();
                } else {
                    linearized_ces = it->second;
                }

                assert(table.trie_.contains(e.cps_));
                table.trie_[e.cps_] = linearized_ces;
            }
        }

        struct cp_rng
        {
            uint32_t const * begin() const noexcept { return &cp_; }
            uint32_t const * end() const noexcept { return &cp_ + 1; }

            uint32_t cp_;
        };
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

        detail::temp_table_t temp_table = detail::make_temp_table();

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

        tailored_collation_element_table table;

        detail::logical_positions_t logical_positions;
        {
            auto lookup_and_assign = [&logical_positions](uint32_t symbol) {
                auto const elems =
                    detail::g_default_collation_trie[detail::cp_rng{symbol}];
                logical_positions[symbol].assign(
                    elems->begin(detail::g_collation_elements_first),
                    elems->end(detail::g_collation_elements_first));
            };
            lookup_and_assign(detail::first_tertiary_ignorable);
            lookup_and_assign(detail::last_tertiary_ignorable);
            logical_positions[detail::first_secondary_ignorable].push_back(
                detail::compressed_collation_element{0xffffffff, 0, 0});
            logical_positions[detail::last_secondary_ignorable].push_back(
                detail::compressed_collation_element{0xffffffff, 0, 0});
            lookup_and_assign(detail::first_primary_ignorable);
            lookup_and_assign(detail::last_primary_ignorable);
            lookup_and_assign(detail::first_variable);
            lookup_and_assign(detail::last_variable);
            lookup_and_assign(detail::first_regular);
            lookup_and_assign(detail::last_regular);
            lookup_and_assign(detail::first_implicit);
        }

        detail::collation_tailoring_interface callbacks = {
            [&](detail::cp_seq_t const & reset, bool before) {
                curr_reset = reset;
                reset_is_before = before;
            },
            [&](detail::relation_t const & rel) {
                detail::modify_table(
                    table,
                    temp_table,
                    logical_positions,
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
            detail::suppress(temp_table, cp);
        }

        finalize_table(
            temp_table,
            std::move(nonsimple_reorders),
            simple_reorders,
            strength_override,
            weighting_override,
            l2_order_override,
            table);

        return table;
    }

}}

#endif
