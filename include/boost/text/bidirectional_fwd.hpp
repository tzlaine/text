#ifndef BOOST_TEXT_BIDIRECTIONAL_FWD_HPP
#define BOOST_TEXT_BIDIRECTIONAL_FWD_HPP


namespace boost { namespace text {

    /** The bidirectional algorithm character properties outlined in Unicode
        10. */
    enum class bidi_prop_t {
        L,
        R,
        EN,
        ES,
        ET,
        AN,
        CS,
        B,
        S,
        WS,
        ON,
        BN,
        NSM,
        AL,
        LRO,
        RLO,
        LRE,
        RLE,
        PDF,
        LRI,
        RLI,
        FSI,
        PDI
    };

}}

#endif
