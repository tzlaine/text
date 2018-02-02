#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

#include <boost/text/collation_fwd.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/text/detail/collation_data.hpp>
#include <boost/text/detail/parser.hpp>

#include <numeric>
#include <vector>

#define BOOST_TEXT_TAILORING_INSTRUMENTATION 0
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
#include <iostream>
#endif


namespace boost { namespace text {

    struct tailored_collation_element_table;

    namespace detail {

        extern collation_trie_key<3> const * g_trie_keys_first;
        extern collation_elements const * g_trie_values_first;
        extern int const * g_trie_element_original_order_first;

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
            auto const it = std::find_if(
                nonsimple_reorders.begin(),
                nonsimple_reorders.end(),
                [cce](detail::nonsimple_script_reorder reorder) {
                    return reorder.first_ <= cce && cce < reorder.last_;
                });
            if (it != nonsimple_reorders.end())
                return it->lead_byte_ << 24;
            auto const masked_primary = cce.l1_ & 0xff000000;
            return simple_reorders[masked_primary >> 24] << 24;
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

        using temp_table_t = segmented_vector<temp_table_element>;

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
            cp_seq_t const & initial_relation,
            optional_cp_seq_t const & prefix,
            optional_cp_seq_t const & extension);

        void finalize_table(
            temp_table_t const & temp_table,
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
        tailored_collation_element_table()
        {
            std::iota(simple_reorders_.begin(), simple_reorders_.end(), 0);
        }

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

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "add_temp_tailoring() ";
            bool first = true;
            for (auto cp : cps) {
                if (!first)
                    std::cerr << " ";
                std::cerr << std::hex << std::setw(4) << std::setfill('0')
                          << cp;
                first = false;
            }
            std::cerr << " ";
            for (auto ce : ces) {
                std::cerr << "[" << std::hex << std::setw(8)
                          << std::setfill('0') << ce.l1_ << " " << std::setw(4)
                          << ce.l2_ << " " << std::setw(2) << std::setfill('0')
                          << ce.l3_ << "] ";
            }
            std::cerr << "\n";
#endif
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
            detail::cp_seq_t const & initial_relation,
            detail::optional_cp_seq_t const & prefix,
            detail::optional_cp_seq_t const & extension);

        friend void detail::finalize_table(
            detail::temp_table_t const & temp_table,
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
            for (std::ptrdiff_t i = 0, end = g_default_collation_trie.size();
                 i != end;
                 ++i) {
                temp_table_element element;
                element.cps_.assign(
                    g_trie_keys_first[i].begin(), g_trie_keys_first[i].end());
                element.ces_.assign(
                    g_trie_values_first[i].begin(g_collation_elements_first),
                    g_trie_values_first[i].end(g_collation_elements_first));
                retval = retval.push_back(element);
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                if (g_trie_keys_first[i].size_ == 1 &&
                    *g_trie_keys_first[i].begin() == 0xe5b) {
                    std::cerr << "========== 0xe5b\n";
                    for (auto ce : element.ces_) {
                        std::cerr << "[" << std::hex << std::setw(8)
                                  << std::setfill('0') << ce.l1_ << " "
                                  << std::setw(4) << ce.l2_ << " "
                                  << std::setw(2) << std::setfill('0')
                                  << ce.l3_ << "] ";
                    }
                    std::cerr << "\n========== \n";
                }
#endif
            }
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            // Dump the temp table to allow comparisom with table data from
            // FractionalUCA.txt.
            for (auto const & element : retval) {
                bool first = true;
                for (auto cp : element.cps_) {
                    if (!first)
                        std::cerr << " ";
                    std::cerr << std::hex << std::setw(4) << std::setfill('0')
                              << cp;
                    first = false;
                }
                std::cerr << " ";
                for (auto ce : element.ces_) {
                    std::cerr << "[" << std::hex << std::setw(8)
                              << std::setfill('0') << ce.l1_ << " "
                              << std::setw(4) << ce.l2_ << " " << std::setw(2)
                              << std::setfill('0') << ce.l3_ << "] ";
                }
                std::cerr << "\n";
            }
#endif
            return retval;
        }

        inline temp_table_element::ces_t get_ces(
            cp_seq_t cps,
            tailored_collation_element_table const & table) noexcept
        {
            temp_table_element::ces_t retval;

            container::small_vector<collation_element, 1024> ces;
            s2(cps.begin(),
               cps.end(),
               variable_weighting::non_ignorable,
               ces,
               &table,
               retain_case_bits_t::yes);

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            {
                bool first = true;
                for (auto cp : cps) {
                    if (!first)
                        std::cerr << " ";
                    std::cerr << std::hex << std::setw(8) << std::setfill('0')
                              << cp;
                    first = false;
                }
                std::cerr << "\n";
            }
            std::cerr << "++++++++++\n";
            for (auto ce : ces) {
                std::cerr << "[" << std::hex << std::setw(8)
                          << std::setfill('0') << ce.l1_ << " " << std::setw(4)
                          << ce.l2_ << " " << std::setw(2) << std::setfill('0')
                          << ce.l3_ << "] ";
            }
            std::cerr << "\n++++++++++\n";
#endif

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
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "0x" << std::hex << std::setfill('0') << std::setw(8)
                      << w;
#endif

            // First, try to find the first zero byte and increment that.
            // This keeps sort keys as short as possible.  Don't increment a
            // primary's lead byte though.
            if (!is_primary && !(w & 0xff000000)) {
                w += 0x01000000;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << " -> 0x" << std::hex << std::setfill('0')
                      << std::setw(8) << w << "\n";
#endif
                return w;
            } else if (!(w & 0xff0000)) {
                w += 0x010000;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << " -> 0x" << std::hex << std::setfill('0')
                      << std::setw(8) << w << "\n";
#endif
                return w;
            } else if (!(w & 0xff00)) {
                w += 0x0100;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << " -> 0x" << std::hex << std::setfill('0')
                      << std::setw(8) << w << "\n";
#endif
                return w;
            } else if (!(w & 0xff)) {
                w += 1;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << " -> 0x" << std::hex << std::setfill('0')
                      << std::setw(8) << w << "\n";
#endif
                return w;
            }

            // Otherwise, just add 1 and check that this does not increment
            // the lead byte.
            uint32_t const initial_lead_byte = w & 0xff000000;
            w += 1;
            uint32_t const lead_byte = w & 0xff000000;
            if (lead_byte != initial_lead_byte && is_primary) {
                throw tailoring_error(
                    "Unable to increment collation element value "
                    "without changing its lead bytes");
            }

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << " -> 0x" << std::hex << std::setfill('0')
                      << std::setw(8) << w << "\n";
#endif

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
                        temp_table.begin(), temp_table.end(), group_last_ces);
                }
            }

            return temp_table.end();
        }

        inline void increment_ce(
            collation_element & ce,
            collation_strength strength,
            bool initial_bump)
        {
            switch (strength) {
            case collation_strength::primary:
                ce.l1_ = increment_32_bit(ce.l1_, true);
                if (initial_bump) {
                    ce.l2_ = common_l2_weight_compressed;
                    ce.l3_ = common_l3_weight_compressed;
                }
                break;
            case collation_strength::secondary:
                if (ce.l2_ & 0xff)
                    ce.l2_ += 1;
                else
                    ce.l2_ += 0x0100;
                if (initial_bump)
                    ce.l3_ = common_l3_weight_compressed;
                break;
            case collation_strength::tertiary:
                if ((((ce.l3_ & 0xff00) + 0x0100) & case_level_bits_mask) == 0)
                    ce.l3_ += 0x0100;
                else
                    ++ce.l3_;
                break;
            case collation_strength::quaternary:
                ce.l4_ = increment_32_bit(ce.l4_, false);
                break;
            default: break;
            }
        }

        inline void
        bump_ces(temp_table_element::ces_t & ces, collation_strength strength)
        {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "bump_ces()\n";
            for (auto ce : ces) {
                std::cerr << "[" << std::setw(8) << std::setfill('0') << ce.l1_
                          << " " << std::setw(4) << ce.l2_ << " "
                          << std::setw(2) << std::setfill('0') << ce.l3_
                          << "] "
                    /*<< std::setw(8) << std::setfill('0') << ce.l4_ << " "*/;
            }
            std::cerr << "\n";
            switch (strength) {
            case collation_strength::primary: std::cerr << "primary\n"; break;
            case collation_strength::secondary:
                std::cerr << "secondary\n";
                break;
            case collation_strength::tertiary: std::cerr << "tertiary\n"; break;
            case collation_strength::quaternary:
                std::cerr << "quaternary\n";
                break;
            case collation_strength::identical:
                std::cerr << "identical\n";
                break;
            }
#endif
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
                ces_it = ces.begin();
            }
            auto & ce = *ces_it;

            // "Increment the collation element weight corresponding to the
            // strength of the operator. For example, for << increment the
            // secondary weight."
            increment_ce(ce, strength, true);
        }

        inline bool well_formed_1(temp_table_element::ces_t const & ces)
        {
            for (auto ce : ces) {
                bool higher_level_zero = !ce.l3_;
                if (ce.l2_) {
                    if (higher_level_zero) {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                        std::cerr << "0x" << std::hex << std::setfill('0')
                                  << std::setw(8) << ce.l1_ << " "
                                  << std::setw(4) << ce.l2_ << " "
                                  << std::setw(2) << ce.l3_ << " "
                                  << std::setw(8) << ce.l4_
                                  << " WF1 violation for L2\n";
#endif
                        return false;
                    }
                } else {
                    higher_level_zero = true;
                }
                if (ce.l1_) {
                    if (higher_level_zero) {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                        std::cerr << "0x" << std::hex << std::setfill('0')
                                  << std::setw(8) << ce.l1_ << " "
                                  << std::setw(4) << ce.l2_ << " "
                                  << std::setw(2) << ce.l3_ << " "
                                  << std::setw(8) << ce.l4_
                                  << " WF1 violation for L1\n";
#endif
                        return false;
                    }
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
                        (tailoring_state.last_tertiary_in_secondary_masked_ &
                         disable_case_level_mask)) {
                        return false;
                    }
                    break;
                default: break;
                }
            }
            return true;
        }

        inline void adjust_case_bits(
            temp_table_element::ces_t const & initial_relation_ces,
            temp_table_element::ces_t & reset_ces)
        {
            // TODO
        }

        // TODO: Audit the FDD0/FDD1 situation in the collation data ,and the
        // tailoring data.

        inline void update_key_ces(
            temp_table_element::ces_t const & ces,
            logical_positions_t & logical_positions,
            tailoring_state_t & tailoring_state)
        {
            // Update logical_positions.
            {
                auto const strength = ce_strength(ces[0]);
                if (strength == collation_strength::primary) {
                    if (ces < logical_positions[first_variable]) {
                        assert(
                            (ces[0].l1_ & 0xff000000) ==
                            (logical_positions[first_variable][0].l1_ &
                             0xff000000));
                        logical_positions[first_variable] = ces;
                    } else if (logical_positions[first_regular] < ces) {
                        if ((ces[0].l1_ & 0xff000000) ==
                            (logical_positions[last_variable][0].l1_ &
                             0xff000000)) {
                            logical_positions[last_variable] = ces;
                        } else {
                            logical_positions[first_regular] = ces;
                        }
                    } else if (logical_positions[last_regular] < ces) {
                        logical_positions[last_regular] = ces;
                    }
                } else if (strength == collation_strength::secondary) {
                    if (ces < logical_positions[first_primary_ignorable])
                        logical_positions[first_primary_ignorable] = ces;
                    else if (logical_positions[last_primary_ignorable] < ces)
                        logical_positions[last_primary_ignorable] = ces;
                } else if (strength == collation_strength::tertiary) {
                    if (ces < logical_positions[first_secondary_ignorable])
                        logical_positions[first_secondary_ignorable] = ces;
                    else if (logical_positions[last_secondary_ignorable] < ces)
                        logical_positions[last_secondary_ignorable] = ces;
                } else if (strength == collation_strength::quaternary) {
                    if (ces < logical_positions[first_tertiary_ignorable])
                        logical_positions[first_tertiary_ignorable] = ces;
                    else if (logical_positions[last_tertiary_ignorable] < ces)
                        logical_positions[last_tertiary_ignorable] = ces;
                }
            }

            // Update tailoring_state.
            for (auto ce : ces) {
                auto const strength = ce_strength(ce);
                if (strength == collation_strength::primary) {
                    if (tailoring_state.last_secondary_in_primary_ < ce.l2_)
                        tailoring_state.last_secondary_in_primary_ = ce.l2_;
                } else if (strength == collation_strength::secondary) {
                    if ((tailoring_state.last_tertiary_in_secondary_masked_ &
                         disable_case_level_mask) <
                        (ce.l3_ & disable_case_level_mask)) {
                        tailoring_state.last_tertiary_in_secondary_masked_ =
                            ce.l3_;
                    }
                }
            }
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
            cp_seq_t const & initial_relation,
            optional_cp_seq_t const & prefix,
            optional_cp_seq_t const & extension)
        {
            temp_table_element::ces_t reset_ces;
            if (reset.size() == 1u &&
                detail::first_tertiary_ignorable <= reset[0] &&
                reset[0] <= detail::first_implicit) {
                reset_ces = logical_positions[reset[0]];
            } else {
                reset_ces = detail::get_ces(reset, table);
            }

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "========== reset= "
                      << text::to_string(reset.begin(), reset.end()) << " ";
            for (auto cp : reset) {
                std::cerr << std::hex << std::setw(8) << std::setfill('0') << cp
                          << " ";
            }
            std::cerr << " ";
            for (auto ce : reset_ces) {
                std::cerr << "[" << std::hex << std::setw(8)
                          << std::setfill('0') << ce.l1_ << " " << std::setw(4)
                          << ce.l2_ << " " << std::setw(2) << std::setfill('0')
                          << ce.l3_ << "] ";
            }
            std::cerr << " ";
            switch (strength) {
            case collation_strength::primary: std::cerr << "<"; break;
            case collation_strength::secondary: std::cerr << "<<"; break;
            case collation_strength::tertiary: std::cerr << "<<<"; break;
            case collation_strength::quaternary: std::cerr << "<<<<"; break;
            case collation_strength::identical: std::cerr << "="; break;
            }
            std::cerr << " relation= "
                      << text::to_string(
                             initial_relation.begin(), initial_relation.end())
                      << " ";
            for (auto cp : initial_relation) {
                std::cerr << std::hex << std::setw(8) << std::setfill('0') << cp
                          << " ";
            }
            std::cerr << "\n";
#endif
            temp_table_element::ces_t const initial_relation_ces =
                get_ces(initial_relation, table);

            cp_seq_t relation = initial_relation;

            if (prefix)
                relation.insert(relation.end(), prefix->begin(), prefix->end());

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
                    throw tailoring_error(
                        "Could not find the collation table element before the "
                        "one requested here");
                }
                reset_ces.clear();
                reset_ces.insert(
                    reset_ces.end(),
                    prev_it->ces_.begin(),
                    prev_it->ces_.end());
            }

            adjust_case_bits(initial_relation_ces, reset_ces);

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
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                std::cerr << "before bump:\n";
                bool first = true;
                for (auto cp : reset) {
                    if (!first)
                        std::cerr << " ";
                    std::cerr << std::hex << std::setw(8) << std::setfill('0')
                              << cp;
                    first = false;
                }
                std::cerr << " ";
                for (auto ce : reset_ces) {
                    std::cerr << "[" << std::hex << std::setw(8)
                              << std::setfill('0') << ce.l1_ << " "
                              << std::setw(4) << ce.l2_ << " " << std::setw(2)
                              << std::setfill('0') << ce.l3_ << "] ";
                }
                std::cerr << "\n";
#endif
                bump_ces(reset_ces, strength);

                // "Weights must be allocated in accordance with the UCA
                // well-formedness conditions."
                if (!well_formed_1(reset_ces)) {
                    throw tailoring_error(
                        "Unable to implement this tailoring rule, because it "
                        "was not possible to meet UCA well-formedness "
                        "condition 1; see "
                        "http://www.unicode.org/reports/tr10/#WF1");
                }
                if (!well_formed_2(reset_ces, tailoring_state)) {
                    throw tailoring_error(
                        "Unable to implement this tailoring rule, because it "
                        "was not possible to meet UCA well-formedness "
                        "condition 2; see "
                        "http://www.unicode.org/reports/tr10/#WF2");
                }

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
                    auto const end = bump_region_end(reset_ces, temp_table) -
                                     temp_table.begin();
                    auto i = table_target_it - temp_table.begin();
                    auto prev_ces = reset_ces;
                    do {
                        auto element = temp_table[i];
                        while (!less(prev_ces, element.ces_)) {
                            increment_ce(element.ces_.front(), strength, false);
                        }
                        element.tailored_ = true;
                        table.add_temp_tailoring(element.cps_, element.ces_);
                        assert(well_formed_1(element.ces_));
                        assert(well_formed_2(element.ces_, tailoring_state));
                        update_key_ces(
                            element.ces_, logical_positions, tailoring_state);
                        temp_table.replace(temp_table.begin() + i, element);
                        prev_ces = element.ces_;
                        ++i;
                    } while (i != end && !less(prev_ces, temp_table[i].ces_));
                }
            }

            // Remove the previous instance of relation from the table, if
            // there was one.
            if (table.trie_.contains(relation) ||
                g_default_collation_trie.contains(relation)) {
                temp_table_element::ces_t const relation_ces =
                    get_ces(relation, table);
                auto remove_it = std::lower_bound(
                    temp_table.begin(), temp_table.end(), relation_ces);
                if (remove_it == temp_table.end() ||
                    remove_it->cps_ != relation) {
                    remove_it = std::find_if(
                        temp_table.begin(),
                        temp_table.end(),
                        [&](temp_table_element const & element) {
                            return element.cps_ == relation;
                        });
                }
                if (remove_it != temp_table.end() &&
                    remove_it->cps_ == relation) {
                    if (remove_it < table_target_it)
                        --table_target_it;
                    temp_table.erase(remove_it);
                }
            }

            table.add_temp_tailoring(relation, reset_ces);
            temp_table_element element;
            element.cps_ = std::move(relation);
            element.ces_ = std::move(reset_ces);
            element.tailored_ = true;

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
            temp_table_t const & temp_table,
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
            for (auto const & e : temp_table) {
                if (!e.tailored_)
                    continue;

#if 0 // TODO: Restore this if/when we make collation tables that are
      // independent of the default table.
                for (auto & ce : e.ces_) {
                    ce.l1_ = replace_lead_byte(
                        ce.l1_,
                        lead_byte(ce, nonsimple_reorders, simple_reorders));
                }
#endif

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

            table.nonsimple_reorders_ = std::move(nonsimple_reorders);
            table.simple_reorders_ = simple_reorders;
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
        string_view tailoring_filename = "",
        parser_diagnostic_callback report_errors = parser_diagnostic_callback(),
        parser_diagnostic_callback report_warnings =
            parser_diagnostic_callback())
    {
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
            // These magic numbers come from "{first,last} secondary ignorable"
            // in FractionalUCA.txt.
            logical_positions[detail::first_secondary_ignorable].push_back(
                detail::collation_element{0, 0, 0x3d02, 0});
            logical_positions[detail::last_secondary_ignorable].push_back(
                detail::collation_element{0, 0, 0x3d02, 0});
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

        detail::cp_seq_t curr_reset;
        bool reset_is_before = false;

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
            // [reorder space digit Latn Grek Copt Hani]
            [&](std::vector<detail::reorder_group> const & reorder_groups) {
                uint32_t curr_reorder_lead_byte =
                    (detail::g_reorder_groups[0].first_.l1_ & 0xff000000) -
                    0x01000000;
                bool prev_group_compressible = false;
                detail::collation_element prev_group_first = {
                    0xffffffff, 0, 0, 0};
                detail::collation_element prev_group_last = {
                    0xffffffff, 0, 0, 0};
                bool first = true;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                std::cerr << std::hex;
#endif
                auto compressible = [](detail::collation_element prev_first,
                                       detail::collation_element prev_last,
                                       detail::collation_element curr_first) {
                    // The end of the previous group must stay in the same
                    // lead byte as the beginning of that group.
                    if ((prev_first.l1_ & 0xff000000) !=
                        (prev_last.l1_ & 0xff000000)) {
                        return false;
                    }
                    prev_last.l1_ &= 0x00ffffff;
                    curr_first.l1_ &= 0x00ffffff;
                    return prev_last <= curr_first;
                };
                for (auto const & group : reorder_groups) {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                    std::cerr << "processing group " << group.name_ << "\n";
#endif
                    bool const compress =
                        group.compressible_ && prev_group_compressible &&
                        compressible(
                            prev_group_first, prev_group_last, group.first_);
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                    std::cerr << "  compress=" << compress << "\n";
#endif
                    if (!compress || first) {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                        std::cerr << "  new lead byte\n";
#endif
                        curr_reorder_lead_byte += 0x01000000;
                    }

                    if ((detail::implicit_weights_final_lead_byte << 24) <
                        curr_reorder_lead_byte) {
                        throw tailoring_error(
                            "It was not possible to tailor the "
                            "collation in the way you requested.  "
                            "Try using fewer groups in '[reorder "
                            "...]'.");
                    }

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                    std::cerr << " simple=" << group.simple_ << "\n";
#endif
                    if (!compress && group.simple_) {
                        uint32_t const group_first =
                            group.first_.l1_ & 0xff000000;
                        for (uint32_t byte = group_first,
                                      end = group.last_.l1_ & 0xff000000;
                             byte < end;
                             byte += 0x01000000) {
                            simple_reorders[byte >> 24] =
                                curr_reorder_lead_byte >> 24;
                            curr_reorder_lead_byte += 0x01000000;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                            std::cerr << " simple reorder " << (byte >> 24)
                                      << " -> " << simple_reorders[byte >> 24]
                                      << "\n";
#endif
                        }
                        curr_reorder_lead_byte -= 0x01000000;
                    } else {
                        nonsimple_reorders.push_back(
                            detail::nonsimple_script_reorder{
                                group.first_,
                                group.last_,
                                curr_reorder_lead_byte >> 24});
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                        std::cerr << " nonsimple reorder ";
                        std::cerr << "[" << group.first_.l1_ << " "
                                  << group.first_.l2_ << " " << group.first_.l3_
                                  << " " << group.first_.l4_ << "] - ";
                        std::cerr << "[" << group.last_.l1_ << " "
                                  << group.last_.l2_ << " " << group.last_.l3_
                                  << " " << group.last_.l4_ << "] -> ";
                        std::cerr << (curr_reorder_lead_byte >> 24) << "\n";
#endif
                    }
                    prev_group_compressible = group.compressible_;
                    prev_group_first = group.first_;
                    prev_group_last = group.last_;
                    first = false;
                }
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                std::cerr << std::dec;
#endif
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
