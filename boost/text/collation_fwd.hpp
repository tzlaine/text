#ifndef BOOST_TEXT_COLLATION_FWD_HPP
#define BOOST_TEXT_COLLATION_FWD_HPP


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

}}

#endif
