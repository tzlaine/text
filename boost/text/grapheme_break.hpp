#ifndef BOOST_TEXT_GRAPHEME_BREAK_HPP
#define BOOST_TEXT_GRAPHEME_BREAK_HPP

#include <array>

#include <cassert>
#include <stdint.h>


namespace boost { namespace text {

    /** The grapheme properties outlined in Unicode 10. */
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

    /** A state machine used in the detction of extended grapheme clusters.
        Only suitable for detection in the forward direction. */
    struct grapheme_break_fsm
    {
        enum class state {
            use_table,
            emoji_mod, // GB10
            emoji_flag // GB12, GB13
        };

        grapheme_break_fsm() noexcept : state_(state::use_table) {}

        bool no_break(grapheme_prop_t prop) noexcept
        {
            if (state_ == state::emoji_mod) {
                if (prop == grapheme_prop_t::E_Modifier) {
                    state_ = state::use_table;
                    return true;
                }
                if (prop != grapheme_prop_t::Extend)
                    state_ = state::use_table;
            } else if (
                prop == grapheme_prop_t::E_Base ||
                prop == grapheme_prop_t::E_Base_GAZ) {
                state_ = state::emoji_mod;
            }

            if (state_ == state::emoji_flag) {
                if (prop == grapheme_prop_t::Regional_Indicator) {
                    state_ = state::use_table;
                    return true;
                } else {
                    state_ = state::use_table;
                }
            } else if (prop == grapheme_prop_t::Regional_Indicator) {
                state_ = state::emoji_flag;
            }

            return false;
        }

    private:
        state state_;
    };

    /** A bookkeeping struct used to apply a \c grapheme_break_fsm to repeated
        calls to \c grapheme_break(). */
    struct grapheme_break_t
    {
        grapheme_break_t() noexcept : break_(false), prop_(grapheme_prop_t::LF)
        {}
        grapheme_break_t(
            bool b, grapheme_prop_t p, grapheme_break_fsm fsm) noexcept :
            break_(b),
            prop_(p),
            fsm_(fsm)
        {}

        operator bool() const noexcept { return break_; }

        bool break_;
        grapheme_prop_t prop_;
        grapheme_break_fsm fsm_;
    };

    /** Returns the grapheme property associated with code point \a cp. */
    grapheme_prop_t grapheme_prop(uint32_t cp) noexcept;

    /** Returns true if and only if the table from the Unicode Character
        Database indicates a break between \a lhs and \a rhs. */
    inline bool grapheme_table_break(grapheme_prop_t lhs, grapheme_prop_t rhs) noexcept
    {
        // Note that RI.RI was changed to '1' since that case is handled in
        // the grapheme break FSM.

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
                             
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // Regional_Indicator
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    0,    0,  1,  1}}, // E_Base
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // E_Modifier

    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  0,  0}}, // ZWJ
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    1,    0,  1,  1}}, // Glue_After_Zwj
    {{1,   1, 1, 1,   0,  1,  0,    1, 1, 1, 1, 1,  1, 1,    0,    0,  1,  1}}, // E_Base_GAZ
}};
        auto const lhs_int = static_cast<int>(lhs);
        auto const rhs_int = static_cast<int>(rhs);
        return grapheme_breaks[lhs_int][rhs_int];
    }

    /** Returns a \c grapheme_break_t that indicates whether a grapheme break
        was detected and that contains the current break-detection state. */
    inline grapheme_break_t
    grapheme_break(grapheme_break_fsm fsm, grapheme_prop_t prop, uint32_t cp) noexcept
    {
        auto const cp_prop = grapheme_prop(cp);
        if (fsm.no_break(cp_prop)) {
            return grapheme_break_t(false, cp_prop, fsm);
        } else {
            return grapheme_break_t(
                grapheme_table_break(prop, cp_prop), cp_prop, fsm);
        }
    }

    /** Searches backward to find the start of the grapheme in which \a
        current is found, without searching before \a first or after \a
        last. */
    template<typename Iter, typename Sentinel>
    Iter find_grapheme_start(Iter first, Iter current, Sentinel last) noexcept
    {
        assert(current != last);

        // See http://www.unicode.org/reports/tr15/#Stream_Safe_Text_Format
        int const max_steps = 31;

        auto current_prop = grapheme_prop(*current);
        while (current != first) {
            // GB10
            if (current_prop == grapheme_prop_t::E_Modifier) {
                auto it = current;
                for (int i = 0; i < max_steps; ++i) {
                    if (it == first) {
                        it = current;
                        break;
                    }
                    auto const prop = grapheme_prop(*--it);
                    if (prop == grapheme_prop_t::E_Base ||
                        prop == grapheme_prop_t::E_Base_GAZ) {
                        break;
                    } else if (prop != grapheme_prop_t::Extend) {
                        it = current;
                        break;
                    }
                }
                if (it != current) {
                    current_prop = grapheme_prop(*it);
                    current = it;
                }
            } else if (current_prop == grapheme_prop_t::Regional_Indicator) {
                auto it = current;
                auto num_ris = 1;
                for (int i = 0; i < max_steps; ++i) {
                    if (it == first)
                        break;
                    auto const prop = grapheme_prop(*--it);
                    if (prop == grapheme_prop_t::Regional_Indicator) {
                        ++num_ris;
                    } else {
                        break;
                    }
                }
                it = current;
                if ((num_ris & 1) == 0)
                    --it;
                current_prop = grapheme_prop(*it);
                current = it;
            }

            if (current != first) {
                auto it = current;
                auto const prop = grapheme_prop(*--it);
                if (grapheme_table_break(prop, current_prop))
                    break;

                current = it;
                current_prop = prop;
            }
        }

        return current;
    }

}}

#endif
