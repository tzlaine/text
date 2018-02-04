#ifndef BOOST_TEXT_COLLATION_FWD_HPP
#define BOOST_TEXT_COLLATION_FWD_HPP

#include <cstdint>


namespace boost { namespace text {

    /** TODO */
    enum class collation_strength {
        primary,
        secondary,
        tertiary,
        quaternary,
        identical
    };

    /** TODO */
    enum class variable_weighting { non_ignorable, shifted };

    /** TODO */
    enum class l2_weight_order { forward, backward };

    /** TODO */
    enum class case_level_t { on, off };

    /** TODO */
    enum class case_first_t { upper, lower, off };

    struct collation_table;

    namespace detail {

        enum class retain_case_bits_t { no, yes };

#if 0
        template<typename Iter>
        text_sort_key collation_sort_key(
            Iter first,
            Iter last,
            collation_strength strength,
            variable_weighting weighting,
            l2_weight_order l2_order,
            collation_table const & table);
#endif

        inline uint32_t
        replace_lead_byte(uint32_t l1_weight, uint32_t new_lead_byte)
        {
            l1_weight &= 0x00ffffff;
            l1_weight |= new_lead_byte;
            return l1_weight;
        }
    }

}}

#endif
