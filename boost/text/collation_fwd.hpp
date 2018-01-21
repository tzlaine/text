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

    struct tailored_collation_element_table;

    namespace detail {

        struct collation_element;

        template<typename Iter>
        void
        s2(Iter first,
           Iter last,
           variable_weighting weighting,
           container::small_vector<collation_element, 1024> & ces,
           tailored_collation_element_table const * table);
    }

}}

#endif
