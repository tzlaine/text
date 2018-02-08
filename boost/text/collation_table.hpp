#ifndef BOOST_TEXT_COLLATION_TAILORING_HPP
#define BOOST_TEXT_COLLATION_TAILORING_HPP

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


namespace boost { namespace filesystem {
    class path;
}}

namespace boost { namespace text {

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

        bool less(
            temp_table_element::ces_t const & lhs,
            temp_table_element::ces_t const & rhs) noexcept;

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

        struct collation_table_data
        {
            collation_table_data() : collation_elements_(nullptr)
            {
                std::iota(simple_reorders_.begin(), simple_reorders_.end(), 0);
            }

            std::vector<collation_element> collation_element_vec_;
            collation_element const * collation_elements_;
            collation_trie_t trie_;

            nonsimple_reorders_t nonsimple_reorders_;
            std::array<uint32_t, 256> simple_reorders_;

            optional<collation_strength> strength_;
            optional<variable_weighting> weighting_;
            optional<l2_weight_order> l2_order_;
            optional<case_level_t> case_level_;
            optional<case_first_t> case_first_;
        };

        inline void add_temp_tailoring(
            collation_table_data & table,
            detail::cp_seq_t const & cps,
            detail::temp_table_element::ces_t const & ces)
        {
            detail::collation_elements value{
                static_cast<uint16_t>(table.collation_element_vec_.size())};
            table.collation_element_vec_.insert(
                table.collation_element_vec_.end(), ces.begin(), ces.end());
            value.last_ = table.collation_element_vec_.size();
            table.trie_.insert_or_assign(cps, value);

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
    }

    /** TODO */
    // TODO: Serialization.
    struct collation_table
    {
        template<typename Iter>
        container::small_vector<detail::collation_element, 1024>
        collation_elements(
            Iter first,
            Iter last,
            variable_weighting weighting =
                variable_weighting::non_ignorable) const;

        template<typename CodePointRange>
        container::small_vector<detail::collation_element, 1024>
        collation_elements(
            CodePointRange & r,
            variable_weighting weighting =
                variable_weighting::non_ignorable) const
        {
            using std::begin;
            using std::end;
            return collation_elements(begin(r), end(r), weighting);
        }

        optional<l2_weight_order> l2_order() const noexcept
        {
            return data_->l2_order_;
        }

    private:
        collation_table() :
            data_(std::make_shared<detail::collation_table_data>())
        {}

        detail::collation_element const * collation_elements_begin() const
            noexcept
        {
            return data_->collation_elements_
                       ? data_->collation_elements_
                       : &data_->collation_element_vec_[0];
        }

        std::shared_ptr<detail::collation_table_data> data_;

        friend collation_table default_collation_table();

        friend collation_table tailored_collation_table(
            string_view tailoring,
            string_view tailoring_filename,
            parser_diagnostic_callback report_errors,
            parser_diagnostic_callback report_warnings);

        friend void save_table(
            collation_table const & table, filesystem::path const & path);
        friend collation_table load_table(filesystem::path const & path);
    };

    namespace detail {
        inline temp_table_t make_temp_table()
        {
            temp_table_t retval;
            for (int i = 0, end = g_num_trie_elements; i != end; ++i) {
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
                                  << std::setw(2) << std::setfill('0') << ce.l3_
                                  << "] ";
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

        inline temp_table_element::ces_t
        get_ces(cp_seq_t cps, collation_table_data const & table);

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
                if (ce.l2_ & 0xff00)
                    ce.l2_ += 1;
                else
                    ce.l2_ += 0x0100;
                if (initial_bump)
                    ce.l3_ = common_l3_weight_compressed;
                break;
            case collation_strength::tertiary:
                if ((((ce.l3_ & 0x3f00) + 0x0100) & case_level_bits_mask) == 0)
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

        inline bool well_formed_1(collation_element ce)
        {
            bool higher_level_zero = (ce.l3_ & disable_case_level_mask) == 0;
            if (ce.l2_) {
                if (higher_level_zero) {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                    std::cerr << "0x" << std::hex << std::setfill('0')
                              << std::setw(8) << ce.l1_ << " " << std::setw(4)
                              << ce.l2_ << " " << std::setw(2) << ce.l3_ << " "
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
                              << std::setw(8) << ce.l1_ << " " << std::setw(4)
                              << ce.l2_ << " " << std::setw(2) << ce.l3_ << " "
                              << std::setw(8) << ce.l4_
                              << " WF1 violation for L1\n";
#endif
                    return false;
                }
            }
            return true;
        }

        inline bool well_formed_2(
            collation_element ce,
            tailoring_state_t const & tailoring_state)
        {
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
            return true;
        }

        inline bool bump_ces(
            temp_table_element::ces_t & ces,
            collation_strength strength,
            tailoring_state_t const & tailoring_state)
        {
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "bump_ces()\n";
            for (auto ce : ces) {
                std::cerr << "[" << std::setw(8) << std::setfill('0') << ce.l1_
                          << " " << std::setw(4) << ce.l2_ << " "
                          << std::setw(2) << std::setfill('0') << ce.l3_ << "] "
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

            bool retval = false;
            if (!well_formed_2(ce, tailoring_state)) {
                auto const s = ce_strength(ce);
                if (s == collation_strength::secondary) {
                    ce.l2_ = tailoring_state.last_secondary_in_primary_;
                } else if (s == collation_strength::tertiary) {
                    ce.l3_ =
                        tailoring_state.last_tertiary_in_secondary_masked_;
                }
                increment_ce(ce, strength, true);
                retval = true;
            }

            if (!well_formed_1(ce)) {
                if (ce.l1_) {
                    if (!ce.l2_)
                        ce.l2_ = common_l2_weight_compressed;
                    if (!ce.l3_)
                        ce.l3_ = common_l3_weight_compressed;
                }
                if (ce.l2_) {
                    if (!ce.l3_)
                        ce.l3_ = common_l3_weight_compressed;
                }
                retval = true;
            }

#if BOOST_TEXT_TAILORING_INSTRUMENTATION
            std::cerr << "final ces: ";
            for (auto ce : ces) {
                std::cerr << "[" << std::setw(8) << std::setfill('0') << ce.l1_
                          << " " << std::setw(4) << ce.l2_ << " "
                          << std::setw(2) << std::setfill('0') << ce.l3_ << "] "
                    /*<< std::setw(8) << std::setfill('0') << ce.l4_ << " "*/;
            }
            std::cerr << "\n";
#endif

            return retval;
        }

        inline bool well_formed_1(temp_table_element::ces_t const & ces)
        {
            return std::all_of(
                ces.begin(), ces.end(), [](collation_element ce) {
                    return well_formed_1(ce);
                });
        }

        inline bool well_formed_2(
            temp_table_element::ces_t const & ces,
            tailoring_state_t const & tailoring_state)
        {
            return std::all_of(
                ces.begin(), ces.end(), [&](collation_element ce) {
                    return well_formed_2(ce, tailoring_state);
                });
        }

        // Variable naming follows
        // http://www.unicode.org/reports/tr35/tr35-collation.html#Case_Tailored
        inline void adjust_case_bits(
            temp_table_element::ces_t const & initial_relation_ces,
            temp_table_element::ces_t & reset_ces)
        {
            container::small_vector<uint16_t, 64> initial_case_bits;
            for (auto ce : initial_relation_ces) {
                if (ce.l1_)
                    initial_case_bits.push_back(ce.l3_ & case_level_bits_mask);
            }

            auto const N = std::ptrdiff_t(initial_case_bits.size());
            auto const M = std::count_if(
                reset_ces.begin(), reset_ces.end(), [](collation_element ce) {
                    return ce_strength(ce) == collation_strength::primary;
                });

            if (N <= M) {
                auto it = initial_case_bits.begin();
                for (std::ptrdiff_t i = 0; i < M; ++i) {
                    auto & ce = reset_ces[i];
                    if (ce.l1_) {
                        ce.l3_ &= disable_case_level_mask;
                        if (it != initial_case_bits.end())
                            ce.l3_ |= *it++;
                    }
                }
            } else {
                auto it = initial_case_bits.begin();
                for (std::ptrdiff_t i = 0; i < M; ++i) {
                    auto & ce = reset_ces[i];
                    if (ce.l1_) {
                        ce.l3_ &= disable_case_level_mask;
                        if (i < M - 1) {
                            ce.l3_ |= *it++;
                        } else {
                            if (std::all_of(
                                    it,
                                    initial_case_bits.end(),
                                    [](uint16_t bits) {
                                        return bits == upper_case_bits;
                                    })) {
                                ce.l3_ |= upper_case_bits;
                            } else if (std::all_of(
                                           it,
                                           initial_case_bits.end(),
                                           [](uint16_t bits) {
                                               return bits == lower_case_bits;
                                           })) {
                                ce.l3_ |= lower_case_bits;
                            } else {
                                ce.l3_ |= mixed_case_bits;
                            }
                        }
                    }
                }
            }

            for (auto & ce : reset_ces) {
                auto const strength = ce_strength(ce);
                if (strength == collation_strength::secondary) {
                    ce.l3_ &= disable_case_level_mask;
                } else if (strength == collation_strength::tertiary) {
                    ce.l3_ &= disable_case_level_mask;
                    ce.l3_ |= upper_case_bits;
                } else if (strength == collation_strength::quaternary) {
                    ce.l3_ &= disable_case_level_mask;
                    ce.l3_ |= lower_case_bits;
                }
            }
        }

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
            collation_table_data & table,
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
                        (curr_ce.l3_ & disable_case_level_mask) !=
                            (ce.l3_ & disable_case_level_mask)) {
                        prev_it = it;
                        break;
                    }
                }
                if (prev_it == temp_table.end()) {
                    throw tailoring_error(
                        "Could not find the collation table element before the "
                        "one requested here");
                }
                reset_ces.assign(
                    prev_it->ces_.begin(),
                    prev_it->ces_.end());

                if (reset.size() == 1u && reset[0] == first_variable) {
                    // Note: Special case: If the found CEs are < first
                    // variable, we need to set the lead byte to be the same
                    // as the first variable.
                    auto const lead_byte =
                        logical_positions[first_variable][0].l1_ & 0xff000000;
                    reset_ces[0].l1_ =
                        replace_lead_byte(reset_ces[0].l1_, lead_byte);
                }
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                std::cerr << "========== before ces -> ";
                for (auto ce : reset_ces) {
                    std::cerr << "[" << std::hex << std::setw(8)
                              << std::setfill('0') << ce.l1_ << " "
                              << std::setw(4) << ce.l2_ << " " << std::setw(2)
                              << std::setfill('0') << ce.l3_ << "] ";
                }
                std::cerr << "\n";
#endif
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
                if (bump_ces(reset_ces, strength, tailoring_state)) {
                    table_target_it = std::upper_bound(
                        temp_table.begin(), temp_table.end(), reset_ces);
                }

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
                        add_temp_tailoring(table, element.cps_, element.ces_);
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
            if (table.trie_.contains(relation)) {
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

            add_temp_tailoring(table, relation, reset_ces);
            temp_table_element element;
            element.cps_ = std::move(relation);
            element.ces_ = std::move(reset_ces);
            element.tailored_ = true;

            temp_table.insert(table_target_it, std::move(element));
        }

        inline void suppress_impl(
            collation_table_data & table,
            collation_trie_t::match_result subseq,
            bool first)
        {
            if (subseq.match && !first)
                table.trie_.erase(trie_iterator_t(subseq));
            if (!subseq.leaf) {
                container::small_vector<uint32_t, 256> next_cps;
                table.trie_.copy_next_key_elements(
                    subseq, std::back_inserter(next_cps));
                for (auto next_cp : next_cps) {
                    suppress_impl(
                        table,
                        table.trie_.extend_subsequence(subseq, next_cp),
                        false);
                }
            }
        }

        inline void suppress(collation_table_data & table, uint32_t cp)
        {
            auto const first_cp_subseq =
                table.trie_.longest_subsequence(&cp, &cp + 1);
            suppress_impl(table, first_cp_subseq, true);
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

        struct cp_rng
        {
            uint32_t const * begin() const noexcept { return &cp_; }
            uint32_t const * end() const noexcept { return &cp_ + 1; }

            uint32_t cp_;
        };

        struct key_and_index_t
        {
            std::array<uint32_t, 3> cps_ = {{0, 0, 0}};
            int index_;

            friend bool operator<(key_and_index_t lhs, key_and_index_t rhs)
            {
                return lhs.cps_ < rhs.cps_;
            }
        };

        inline collation_trie_t make_default_trie()
        {
            collation_trie_t retval;
            std::vector<key_and_index_t> key_and_indices;
            {
                key_and_indices.resize(g_num_trie_elements);
                for (int i = 0, end = g_num_trie_elements; i < end; ++i) {
                    auto & kai = key_and_indices[i];
                    auto key = g_trie_keys_first[i];
                    std::copy(key.begin(), key.end(), kai.cps_.begin());
                    kai.index_ = i;
                }
                std::sort(key_and_indices.begin(), key_and_indices.end());
            }
            for (auto kai : key_and_indices) {
                retval.insert(
                    g_trie_keys_first[kai.index_],
                    g_trie_values_first[kai.index_]);
            }
            return retval;
        }
    }

    /** TODO */
    inline collation_table default_collation_table()
    {
        collation_table retval;
        retval.data_->collation_elements_ = detail::g_collation_elements_first;
        retval.data_->trie_ = detail::make_default_trie();
        return retval;
    }

    /** TODO */
    collation_table tailored_collation_table(
        string_view tailoring,
        string_view tailoring_filename = "",
        parser_diagnostic_callback report_errors = parser_diagnostic_callback(),
        parser_diagnostic_callback report_warnings =
            parser_diagnostic_callback());

}}

#include <boost/text/collate.hpp>

namespace boost { namespace text {

    template<typename Iter>
    container::small_vector<detail::collation_element, 1024>
    collation_table::collation_elements(
        Iter first, Iter last, variable_weighting weighting) const
    {
        if (data_->weighting_)
            weighting = *data_->weighting_;
        container::small_vector<detail::collation_element, 1024> retval;
        detail::s2(
            first,
            last,
            retval,
            data_->trie_,
            collation_elements_begin(),
            [&](detail::collation_element ce) {
                return detail::lead_byte(
                    ce, data_->nonsimple_reorders_, data_->simple_reorders_);
            },
            weighting,
            detail::retain_case_bits_t::no); // TODO: Other case params.
        return retval;
    }

    inline collation_table tailored_collation_table(
        string_view tailoring,
        string_view tailoring_filename,
        parser_diagnostic_callback report_errors,
        parser_diagnostic_callback report_warnings)
    {
        detail::temp_table_t temp_table = detail::make_temp_table();

        collation_table table;
        table.data_->trie_ = detail::make_default_trie();
        table.data_->collation_element_vec_.assign(
            detail::g_collation_elements_first,
            detail::g_collation_elements_first +
                detail::g_num_collation_elements);

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
            detail::initial_first_implicit,
            detail::initial_first_trailing};

        detail::logical_positions_t logical_positions;
        {
            auto lookup_and_assign = [&](uint32_t symbol) {
                auto const cp =
                    symbol_lookup[symbol - detail::first_tertiary_ignorable];
                auto const elems = table.data_->trie_[detail::cp_rng{cp}];
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
                table.data_->trie_,
                table.collation_elements_begin(),
                [&table](detail::collation_element ce) {
                    return detail::lead_byte(
                        ce,
                        table.data_->nonsimple_reorders_,
                        table.data_->simple_reorders_);
                },
                detail::retain_case_bits_t::yes);

            lookup_and_assign(detail::first_trailing);
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
                    *table.data_,
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
            [&](collation_strength strength) {
                table.data_->strength_ = strength;
            },
            [&](variable_weighting weighting) {
                table.data_->weighting_ = weighting;
            },
            [&](l2_weight_order l2_order) {
                table.data_->l2_order_ = l2_order;
            },
            [&](case_level_t cl) { table.data_->case_level_ = cl; },
            [&](case_first_t cf) { table.data_->case_first_ = cf; },
            [&](detail::cp_seq_t const & suppressions_) {
                for (auto cp : suppressions_) {
                    detail::suppress(*table.data_, cp);
                }
            },
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
                             byte < end &&
                             byte < (detail::implicit_weights_final_lead_byte
                                     << 24);
                             byte += 0x01000000) {
                            table.data_->simple_reorders_[byte >> 24] =
                                curr_reorder_lead_byte >> 24;
                            curr_reorder_lead_byte += 0x01000000;
#if BOOST_TEXT_TAILORING_INSTRUMENTATION
                            std::cerr
                                << " simple reorder " << (byte >> 24) << " -> "
                                << table.data_->simple_reorders_[byte >> 24]
                                << "\n";
#endif
                        }
                        curr_reorder_lead_byte -= 0x01000000;
                    } else {
                        table.data_->nonsimple_reorders_.push_back(
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

        for (auto & ce : table.data_->collation_element_vec_) {
            ce.l1_ = detail::replace_lead_byte(
                ce.l1_,
                detail::lead_byte(
                    ce,
                    table.data_->nonsimple_reorders_,
                    table.data_->simple_reorders_));
        }

        return table;
    }

    namespace detail {

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
               lhs_bytes,
               retain_case_bits_t::no);
            s3(rhs.begin(),
               rhs.end(),
               rhs.size(),
               collation_strength::quaternary,
               l2_weight_order::forward,
               cps,
               cps,
               0,
               rhs_bytes,
               retain_case_bits_t::no);

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

        inline temp_table_element::ces_t
        get_ces(cp_seq_t cps, collation_table_data const & table)
        {
            temp_table_element::ces_t retval;

            container::small_vector<collation_element, 1024> ces;
            detail::s2(
                cps.begin(),
                cps.end(),
                ces,
                table.trie_,
                table.collation_elements_ ? table.collation_elements_
                                          : &table.collation_element_vec_[0],
                [&table](detail::collation_element ce) {
                    return detail::lead_byte(
                        ce, table.nonsimple_reorders_, table.simple_reorders_);
                },
                variable_weighting::non_ignorable,
                detail::retain_case_bits_t::yes);

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
            for (auto ce : retval) {
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
    }

}}

#endif
