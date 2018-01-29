#ifndef BOOST_TEXT_COLLATION_FWD_HPP
#define BOOST_TEXT_COLLATION_FWD_HPP

#include <boost/container/small_vector.hpp>


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

    struct tailored_collation_element_table;

    namespace detail {

        struct collation_element;

        template<typename OutIter>
        inline OutIter add_derived_elements(
            uint32_t cp,
            variable_weighting weighting,
            OutIter out,
            tailored_collation_element_table const * table);

        template<typename Iter>
        void
        s2(Iter first,
           Iter last,
           variable_weighting weighting,
           container::small_vector<collation_element, 1024> & ces,
           tailored_collation_element_table const * table);

        template<typename CEIter, typename CPIter, typename Container>
        void
        s3(CEIter ces_first,
           CEIter ces_last,
           int ces_size,
           collation_strength strength,
           l2_weight_order l2_order,
           CPIter cps_first,
           CPIter cps_last,
           int cps_size,
           Container & bytes);

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
