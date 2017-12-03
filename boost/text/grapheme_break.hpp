#ifndef BOOST_TEXT_GRAPHEME_BREAK_HPP
#define BOOST_TEXT_GRAPHEME_BREAK_HPP

#include <array>

#include <stdint.h>


namespace boost { namespace text {

    enum class grapheme_prop_t {
        Other,
        CR,
        LF,
        Control,
        Extend,
        Prepend,
        SpacingMark,
        L,
        V,
        T,
        LV,
        LVT,
        Regional_Indicator,
        E_Base,
        E_Modifier,
        ZWJ,
        Glue_After_Zwj,
        E_Base_GAZ
    };

    struct grapheme_break_t
    {
        grapheme_break_t () : break_ (false), prop_ (grapheme_prop_t::LF) {}
        grapheme_break_t (bool b, grapheme_prop_t p) : break_ (b), prop_ (p) {}

        operator bool() const { return break_; }

        bool break_;
        grapheme_prop_t prop_;
    };

    grapheme_prop_t grapheme_prop(uint32_t cp);

    inline grapheme_break_t grapheme_break(grapheme_prop_t prop, uint32_t cp)
    {
// See chart at http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakTest.html.
constexpr std::array<std::array<bool, 18>, 18> grapheme_breaks = {{
//  Other CR LF Ctrl Ext Pre SpcMk L  V  T  LV LVT RI E_Bse E_Mod ZWJ GAZ EBG
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // Other
    {{1,   1, 0, 1,   1,  1,  1,    1, 1, 1, 1, 1,  1, 1,    1,    1,  1,  1}}, // CR
    {{1,   1, 1, 1,   1,  1,  1,    1, 1, 1, 1, 1,  1, 1,    1,    1,  1,  1}}, // LF
                             
    {{1,   1, 1, 1,   1,  1,  1,    1, 1, 1, 1, 1,  1, 1,    1,    1,  1,  1}}, // Control
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // Extend
    {{0,   1, 1, 1,   0,  0,  0,    0, 0, 0, 0, 0,  0, 0,    0,    0,  0,  0}}, // Prepend
                             
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // SpacingMark
    {{1,   1, 1, 1,   0,  1,  0,    0, 0, 1, 0, 0,  1, 1,    1,    0,  1,  1}}, // L
    {{1,   1, 1, 1,   0,  1,  0,    1, 0, 0, 1, 1,  1, 1,    1,    0,  1,  1}}, // V
                             
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 0, 1, 1,  1, 1,    1,    0,  1,  1}}, // T
    {{1,   1, 1, 1,   0,  1,  0,    1, 0, 0, 1, 1,  1, 1,    1,    0,  1,  1}}, // LV
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 0, 1, 1,  1, 1,    1,    0,  1,  1}}, // LVT
                             
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  0, 1,    1,    0,  1,  1}}, // Regional_Indicator
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    0,    0,  1,  1}}, // E_Base
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // E_Modifier

    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  0,  0}}, // ZWJ
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // Glue_After_Zwj
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    0,    0,  1,  1}}, // E_Base_GAZ
}};

        auto const prop_int = static_cast<int>(prop);
        auto const cp_prop = grapheme_prop(cp);
        auto const cp_prop_int = static_cast<int>(cp_prop);
        return grapheme_break_t(grapheme_breaks[prop_int][cp_prop_int], cp_prop);
    }

}}

#endif
