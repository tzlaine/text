#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/detail/collation_data.hpp>
#include <boost/text/detail/parser.hpp>

#include <numeric>
#include <vector>


namespace boost { namespace text {

    struct tailored_collation_element_table;

    namespace detail {

        struct nonsimple_script_reorder
        {
            collation_element first_;
            collation_element last_;
            uint32_t lead_byte_;
        };

        using nonsimple_reorders_t = container::
            static_vector<nonsimple_script_reorder, g_reorder_groups.size()>;

        uint32_t lead_byte(
            collation_element cce,
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
            using ces_t = container::small_vector<collation_element, 4>;

            cp_seq_t cps_;
            ces_t ces_;

            bool tailored_ = false;
        };

        inline bool less(
            temp_table_element::ces_t const & lhs,
            temp_table_element::ces_t const & rhs) noexcept
        {
            container::static_vector<uint32_t, 256> lhs_bytes;
            container::static_vector<uint32_t, 256> rhs_bytes;

            uint32_t const * cps = nullptr;
            s3(lhs.begin(),
               lhs.end(),
               lhs.size(),
               collation_strength::quaternary,
               l2_weight_order::forward,
               cps,
               cps,
               0,
               lhs_bytes);
            s3(rhs.begin(),
               rhs.end(),
               rhs.size(),
               collation_strength::quaternary,
               l2_weight_order::forward,
               cps,
               cps,
               0,
               rhs_bytes);

            auto const pair = algorithm::mismatch(
                lhs_bytes.begin(),
                lhs_bytes.end(),
                rhs_bytes.begin(),
                rhs_bytes.end());
            if (pair.first == lhs_bytes.end()) {
                if (pair.second == rhs_bytes.end())
                    return false;
                return true;
            } else {
                if (pair.second == rhs_bytes.end())
                    return false;
                return *pair.first < *pair.second;
            }
        }

        inline bool less_equal(
            temp_table_element::ces_t const & lhs,
            temp_table_element::ces_t const & rhs) noexcept
        {
            if (lhs == rhs)
                return true;
            return less(lhs, rhs);
        }

        bool operator<(
            temp_table_element const & lhs, temp_table_element const & rhs)
        {
            return less(lhs.ces_, rhs.ces_);
        }

        bool operator<(
            temp_table_element::ces_t const & lhs,
            temp_table_element const & rhs)
        {
            return less(lhs, rhs.ces_);
        }

        bool operator<(
            temp_table_element const & lhs,
            temp_table_element::ces_t const & rhs)
        {
            return less(lhs.ces_, rhs);
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

        struct tailoring_state_t
        {
            uint8_t first_tertiary_in_secondary_masked_ =
                first_tertiary_in_secondary_masked;
            uint8_t last_tertiary_in_secondary_masked_ =
                last_tertiary_in_secondary_masked;
            uint16_t first_secondary_in_primary_ = first_secondary_in_primary;
            uint16_t last_secondary_in_primary_ = last_secondary_in_primary;
        };

        void modify_table(
            tailored_collation_element_table & table,
            temp_table_t & temp_table,
            logical_positions_t & logical_positions,
            tailoring_state_t & tailoring_state,
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
            optional<case_level_t> case_level_override,
            optional<case_first_t> case_first_override,
            tailored_collation_element_table & table);
    }

    /** TODO */
    struct tailored_collation_element_table
    {
        detail::collation_trie_t const & trie() const noexcept { return trie_; }

        detail::collation_element const * collation_elements_begin() const
            noexcept
        {
            return &collation_elements_[0];
        }

        uint32_t lead_byte(detail::collation_element cce) const noexcept
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
        optional<case_level_t> case_level() const noexcept
        {
            return case_level_;
        }
        optional<case_first_t> case_first() const noexcept
        {
            return case_first_;
        }

    private:
        tailored_collation_element_table() {}

        void add_temp_tailoring(
            detail::cp_seq_t const & cps,
            detail::temp_table_element::ces_t const & ces)
        {
            detail::collation_elements value{
                static_cast<uint16_t>(collation_elements_.size())};
            collation_elements_.insert(
                collation_elements_.end(), ces.begin(), ces.end());
            value.last_ = collation_elements_.size();
            trie_.insert_or_assign(cps, value);
        }

        std::vector<detail::collation_element> collation_elements_;
        detail::collation_trie_t trie_;

        detail::nonsimple_reorders_t nonsimple_reorders_;
        std::array<uint32_t, 256> simple_reorders_;

        optional<collation_strength> strength_;
        optional<variable_weighting> weighting_;
        optional<l2_weight_order> l2_order_;
        optional<case_level_t> case_level_;
        optional<case_first_t> case_first_;

        friend void detail::modify_table(
            tailored_collation_element_table & table,
            detail::temp_table_t & temp_table,
            detail::logical_positions_t & logical_positions,
            detail::tailoring_state_t & tailoring_state,
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
            optional<case_level_t> case_level_override,
            optional<case_first_t> case_first_override,
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
            std::copy(ces.begin(), ces.end(), retval.begin());

            return retval;
        }

        template<typename Iter>
        Iter last_ce_at_least_strength(
            Iter first, Iter last, collation_strength strength) noexcept
        {
            for (auto it = last; it != first;) {
                if (ce_strength(*--it) <= strength)
                    return it;
            }
            return last;
        }

        inline uint32_t increment_32_bit(uint32_t w, bool is_primary)
        {
            uint32_t byte = w & 0xff;
            if (0 < byte && byte < 0xff) {
                w += 1;
            } else {
                byte = (w >> 8) & 0xff;
                if (0 < byte && byte < 0xff) {
                    w += 0x0100;
                } else {
                    // Stop here so we don't change the lead byte.
                    byte = (w >> 16) & 0xff;
                    if (byte == 0xff && is_primary)
                        ; // TODO: throw
                    w += 0x010000;
                }
            }
            return w;
        }

        inline temp_table_t::iterator bump_region_end(
            temp_table_element::ces_t const & ces, temp_table_t & temp_table)
        {
            temp_table_element::ces_t group_first_ces;
            group_first_ces.push_back(g_reorder_groups[0].first_);
            if (less(ces, group_first_ces)) {
                return std::lower_bound(
                    temp_table.begin(), temp_table.end(), group_first_ces);
            }

            temp_table_element::ces_t group_last_ces;
            for (auto const & group : g_reorder_groups) {
                group_first_ces.clear();
                group_first_ces.push_back(group.first_);
                group_last_ces.clear();
                group_last_ces.push_back(group.last_);

                if (less_equal(group_first_ces, ces) &&
                    less_equal(ces, group_last_ces)) {
                    return std::lower_bound(
                        temp_table.begin(), temp_table.end(), group_first_ces);
                }
            }

            return temp_table.end();
        }

        inline void
        bump_ces(temp_table_element::ces_t & ces, collation_strength strength)
        {
            // "Find the last collation element whose strength is at least
            // as great as the strength of the operator. For example, for <<
            // find the last primary or secondary CE. This CE will be
            // modified; all following CEs should be removed. If there is no
            // such CE, then reset the collation elements to a single
            // completely-ignorable CE."
            auto ces_it =
                last_ce_at_least_strength(ces.begin(), ces.end(), strength);
            if (ces_it != ces.end())
                ces.erase(std::next(ces_it), ces.end());
            if (ces_it == ces.end()) {
                ces.clear();
                ces.push_back(collation_element{0, 0, 0, 0});
                ces_it = ces.end();
            }
            auto & ce = *ces_it;

            // "Increment the collation element weight corresponding to the
            // strength of the operator. For example, for << increment the
            // secondary weight."
            switch (strength) {
            case collation_strength::primary:
                ce.l1_ = increment_32_bit(ce.l1_, true);
                ce.l2_ = common_l2_weight_compressed;
                ce.l3_ = common_l3_weight_compressed;
                break;
            case collation_strength::secondary:
                if (ce.l2_ & 0xff)
                    ce.l2_ += 1;
                else
                    ce.l2_ += 0x0100;
                ce.l3_ = common_l3_weight_compressed;
                break;
            case collation_strength::tertiary: ++ce.l3_; break;
            case collation_strength::quaternary:
                ce.l4_ = increment_32_bit(ce.l4_, false);
                break;
            default: break;
            }
        }

        inline bool well_formed_1(temp_table_element::ces_t const & ces)
        {
            for (auto ce : ces) {
                bool higher_level_zero = !ce.l1_;
                if (ce.l2_) {
                    if (higher_level_zero)
                        return false;
                } else {
                    higher_level_zero = true;
                }
                if (ce.l3_) {
                    if (higher_level_zero)
                        return false;
                } else {
                    higher_level_zero = true;
                }
                if (ce.l4_) {
                    if (higher_level_zero)
                        return false;
                }
            }
            return true;
        }

        inline bool well_formed_2(
            temp_table_element::ces_t const & ces,
            tailoring_state_t const & tailoring_state)
        {
            for (auto ce : ces) {
                switch (ce_strength(ce)) {
                case collation_strength::secondary:
                    if (ce.l2_ <= tailoring_state.last_secondary_in_primary_)
                        return false;
                    break;
                case collation_strength::tertiary:
                    if ((ce.l3_ & disable_case_level_mask) <=
                        tailoring_state.last_tertiary_in_secondary_masked_) {
                        return false;
                    }
                    break;
                default: break;
                }
            }
            return true;
        }

        inline void update_key_ces(
            temp_table_element::ces_t const & ces,
            logical_positions_t & logical_positions,
            tailoring_state_t & tailoring_state)
        {
            // TODO: Update logical_positions with ces
            // TODO: Update tailoring_state with ces
        }

        // http://www.unicode.org/reports/tr35/tr35-collation.html#Orderings
        inline void modify_table(
            tailored_collation_element_table & table,
            temp_table_t & temp_table,
            logical_positions_t & logical_positions,
            tailoring_state_t & tailoring_state,
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
            } else {
                reset_ces = get_ces(reset, table);
            }

            if (before) {
                auto ces_it = last_ce_at_least_strength(
                    reset_ces.begin(), reset_ces.end(), strength);
                if (ces_it == reset_ces.end()) {
                    reset_ces.clear();
                    reset_ces.push_back(collation_element{0, 0, 0, 0});
                    ces_it = reset_ces.begin();
                }
                auto const ce = *ces_it;

                reset_ces.clear();
                reset_ces.push_back(ce);
                auto it = std::lower_bound(
                    temp_table.begin(), temp_table.end(), reset_ces);
                assert(it != temp_table.begin());
                auto prev_it = temp_table.end();
                while (it != temp_table.begin()) {
                    --it;
                    auto const curr_ce = it->ces_[0];
                    if (curr_ce.l1_ != ce.l1_) {
                        prev_it = it;
                        break;
                    } else if (
                        collation_strength::secondary <= strength &&
                        curr_ce.l2_ != ce.l2_) {
                        prev_it = it;
                        break;
                    } else if (
                        collation_strength::tertiary <= strength &&
                        curr_ce.l3_ != ce.l3_) {
                        prev_it = it;
                        break;
                    }
                }
                if (prev_it == temp_table.end()) {
                    // TODO: Could not implement this.  Throw here, catch it in
                    // the parser, and produce a reasonable error message based
                    // on its state.
                }
                reset_ces.clear();
                reset_ces.insert(
                    reset_ces.end(),
                    prev_it->ces_.begin(),
                    prev_it->ces_.end());
            }

            // TODO: Adjust reset_ces case bits here.

            if (extension) {
                auto const extension_ces = get_ces(*extension, table);
                reset_ces.insert(
                    reset_ces.end(),
                    extension_ces.begin(),
                    extension_ces.end());
            }

            // The insert should happen at/before this point.  We may need to
            // adjust CEs at/after this to make that work.
            auto table_target_it = std::upper_bound(
                temp_table.begin(), temp_table.end(), reset_ces);

            if (strength != collation_strength::identical) {
                bump_ces(reset_ces, strength);

                // "Weights must be allocated in accordance with the UCA
                // well-formedness conditions."
                if (!well_formed_1(reset_ces))
                    ; // TODO: Throw.
                if (!well_formed_2(reset_ces, tailoring_state))
                    ; // TODO: Throw.

                update_key_ces(reset_ces, logical_positions, tailoring_state);

                assert(table_target_it != temp_table.end());

                // The checks in here only need to be performed if the increment
                // above did not slot cleanly between two existing CEs.
                if (!less(reset_ces, table_target_it->ces_)) {
                    // "The new weight must be less than the next weight for the
                    // same combination of higher-level weights of any collation
                    // element according to the current state." -- this will be
                    // true as long as we can bump one or more subsequent CEs up
                    // so that this condition is maintained.

                    // For reorderings to work, we can't keep bumping
                    // indefinitely; stop before we leave the current script, if
                    // applicable.
                    auto const end = bump_region_end(reset_ces, temp_table);
                    auto prev_it = table_target_it;
                    auto it = prev_it;
                    do {
                        bump_ces(it->ces_, strength);
                        it->tailored_ = true;
                        table.add_temp_tailoring(it->cps_, it->ces_);
                        assert(well_formed_1(it->ces_));
                        assert(well_formed_2(it->ces_, tailoring_state));
                        update_key_ces(
                            it->ces_, logical_positions, tailoring_state);
                        ++it;
                    } while (it != end && !less(prev_it->ces_, it->ces_));
                }
            }

            table.add_temp_tailoring(relation, reset_ces);
            temp_table_element element;
            element.cps_ = std::move(relation);
            element.ces_ = std::move(reset_ces);
            element.tailored_ = true;
            table_target_it =
                temp_table.insert(table_target_it, std::move(element));

            // http://www.unicode.org/reports/tr10/#WF5 "If a table contains a
            // contraction consisting of a sequence of N code points, with N > 2
            // and the last code point being a non-starter, then the table must
            // also contain a contraction consisting of the sequence of the
            // first N-1 code points."
            // TODO: For any contraction of length N, add its N-1 prefixes based
            // on the current state.  This enforces WF5, and ensures that the
            // in-tailored-table and in-default-table longest matches are
            // disjoint.

            // TODO: If we add anything to this table that is a prefix of
            // something in the default table, add those suffixes to the new
            // table.  This also ensures that the in-tailored-table and
            // in-default-table longest matches are disjoint.

            // TODO: I don't think this is necessary; try removing it and
            // seeing if it breaks the tests (once the tests exist).
#if 0
            auto same_key = [&relation](temp_table_element const & e) {
                return e.cps_ == relation;
            };
            auto remove_it =
                std::remove_if(temp_table.begin(), table_target_it, same_key);
            if (remove_it == table_target_it) {
                remove_it = std::remove_if(
                    std::next(table_target_it), temp_table.end(), same_key);
            }
            if (remove_it != temp_table.end()) {
                assert(std::distance(remove_it, temp_table.end()) == 1);
                temp_table.erase(remove_it);
            }
#endif
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
            optional<case_level_t> case_level_override,
            optional<case_first_t> case_first_override,
            tailored_collation_element_table & table)
        {
            table.collation_elements_.clear();
            table.strength_ = strength_override;
            table.weighting_ = weighting_override;
            table.l2_order_ = l2_order_override;
            table.case_level_ = case_level_override;
            table.case_first_ = case_first_override;

            std::unordered_map<temp_table_element::ces_t, collation_elements>
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

                collation_elements linearized_ces;
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
        optional<case_level_t> case_level_override;
        optional<case_first_t> case_first_override;

        detail::cp_seq_t suppressions;

        detail::nonsimple_reorders_t nonsimple_reorders;
        std::array<uint32_t, 256> simple_reorders;
        std::iota(simple_reorders.begin(), simple_reorders.end(), 0);
        for (auto & x : simple_reorders) {
            x <<= 24;
        }

        tailored_collation_element_table table;

        uint32_t const symbol_lookup[] = {
            detail::initial_first_tertiary_ignorable,
            detail::initial_last_tertiary_ignorable,
            detail::initial_first_secondary_ignorable,
            detail::initial_last_secondary_ignorable,
            detail::initial_first_primary_ignorable,
            detail::initial_last_primary_ignorable,
            detail::initial_first_variable,
            detail::initial_last_variable,
            detail::initial_first_regular,
            detail::initial_last_regular,
            detail::initial_first_implicit};

        detail::logical_positions_t logical_positions;
        {
            auto lookup_and_assign = [&](uint32_t symbol) {
                auto const cp =
                    symbol_lookup[symbol - detail::first_tertiary_ignorable];
                auto const elems =
                    detail::g_default_collation_trie[detail::cp_rng{cp}];
                logical_positions[symbol].assign(
                    elems->begin(detail::g_collation_elements_first),
                    elems->end(detail::g_collation_elements_first));
            };
            lookup_and_assign(detail::first_tertiary_ignorable);
            lookup_and_assign(detail::last_tertiary_ignorable);
            logical_positions[detail::first_secondary_ignorable].push_back(
                detail::collation_element{0xffffffff, 0, 0, 0});
            logical_positions[detail::last_secondary_ignorable].push_back(
                detail::collation_element{0xffffffff, 0, 0, 0});
            lookup_and_assign(detail::first_primary_ignorable);
            lookup_and_assign(detail::last_primary_ignorable);
            lookup_and_assign(detail::first_variable);
            lookup_and_assign(detail::last_variable);
            lookup_and_assign(detail::first_regular);
            lookup_and_assign(detail::last_regular);

            detail::add_derived_elements(
                symbol_lookup
                    [detail::first_implicit - detail::first_tertiary_ignorable],
                variable_weighting::non_ignorable,
                std::back_inserter(logical_positions[detail::first_implicit]),
                nullptr);
        }

        detail::tailoring_state_t tailoring_state;

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
                    tailoring_state,
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
            [&](case_level_t cl) { case_level_override = cl; },
            [&](case_first_t cf) { case_first_override = cf; },
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
                detail::collation_element prev_group_last = {
                    0xffffffff, 0, 0, 0};
                for (auto const & group : reorder_groups) {
                    bool const compress = group.compressible_ &&
                                          prev_group_compressible &&
                                          prev_group_last < group.last_;
                    if (!compress)
                        curr_reorder_lead_byte += 0x01000000;

                    if ((detail::implicit_weights_final_lead_byte << 24) <
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
                            simple_reorders[byte >> 24] =
                                curr_reorder_lead_byte;
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
            case_level_override,
            case_first_override,
            table);

        return table;
    }

}}

#endif
