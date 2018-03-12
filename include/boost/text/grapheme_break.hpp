#ifndef BOOST_TEXT_GRAPHEME_BREAK_HPP
#define BOOST_TEXT_GRAPHEME_BREAK_HPP

#include <boost/text/algorithm.hpp>

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

    /** Returns the grapheme property associated with code point \a cp. */
    grapheme_prop_t grapheme_prop(uint32_t cp) noexcept;

    namespace detail {
        inline bool skippable(grapheme_prop_t prop) noexcept
        {
            return prop == grapheme_prop_t::Extend;
        }

        enum class grapheme_break_emoji_state_t {
            none,
            first_emoji, // Indicates that prop points to an odd-count
                         // emoji.
            second_emoji // Indicates that prop points to an even-count
                         // emoji.
        };

        template<typename CPIter>
        struct grapheme_break_state
        {
            CPIter it;

            grapheme_prop_t prev_prop;
            grapheme_prop_t prop;

            grapheme_break_emoji_state_t emoji_state;
        };

        template<typename CPIter>
        grapheme_break_state<CPIter> next(grapheme_break_state<CPIter> state)
        {
            ++state.it;
            state.prev_prop = state.prop;
            return state;
        }

        inline bool
        table_grapheme_break(grapheme_prop_t lhs, grapheme_prop_t rhs) noexcept
        {
            // Note that RI.RI was changed to '1' since that case is handled in
            // the grapheme break FSM.

            // clang-format off
// See chart at http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakTest.html.
constexpr std::array<std::array<bool, 18>, 18> grapheme_breaks = {{
//   Other CR LF Ctrl Ext Pre SpcMk L  V  T  LV LVT RI E_Bse E_Mod ZWJ GAZ EBG
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
            // clang-format on
            auto const lhs_int = static_cast<int>(lhs);
            auto const rhs_int = static_cast<int>(rhs);
            return grapheme_breaks[lhs_int][rhs_int];
        }

        template<typename CPIter, typename Sentinel>
        grapheme_break_state<CPIter> skip_forward(
            grapheme_break_state<CPIter> state, CPIter first, Sentinel last)
        {
            if (state.it != first &&
                (state.prev_prop == grapheme_prop_t::E_Base ||
                 state.prev_prop == grapheme_prop_t::E_Base_GAZ) &&
                skippable(state.prop)) {
                auto temp_it =
                    find_if_not(std::next(state.it), last, [](uint32_t cp) {
                        return skippable(grapheme_prop(cp));
                    });
                if (temp_it != last &&
                    grapheme_prop(*temp_it) == grapheme_prop_t::E_Modifier) {
                    auto const temp_prop = grapheme_prop(*temp_it);
                    state.it = temp_it;
                    state.prop = temp_prop;
                }
            }
            return state;
        }
    }

    /** Searches backward to find the start of the grapheme in which \a
        current is found, without searching before \a first or after \a
        last. */
    template<typename CPIter, typename Sentinel>
    CPIter prev_grapheme_break(CPIter first, CPIter it, Sentinel last) noexcept
    {
        auto current = it;
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
                if (detail::table_grapheme_break(prop, current_prop))
                    break;

                current = it;
                current_prop = prop;
            }
        }

        return current;
    }

    /** TODO */
    template<typename CPIter, typename Sentinel>
    CPIter next_grapheme_break(CPIter first, Sentinel last) noexcept
    {
        if (first == last)
            return first;

        detail::grapheme_break_state<CPIter> state;
        state.it = first;

        if (++state.it == last)
            return state.it;

        state.prev_prop = grapheme_prop(*std::prev(state.it));
        state.prop = grapheme_prop(*state.it);

        state.emoji_state =
            state.prev_prop == grapheme_prop_t::Regional_Indicator
                ? detail::grapheme_break_emoji_state_t::first_emoji
                : detail::grapheme_break_emoji_state_t::none;

        for (; state.it != last; state = next(state)) {
            state.prop = grapheme_prop(*state.it);

            // GB10
            state = detail::skip_forward(state, first, last);
            if (state.it == last)
                return state.it;

            if (state.emoji_state ==
                detail::grapheme_break_emoji_state_t::first_emoji) {
                if (state.prop == grapheme_prop_t::Regional_Indicator) {
                    state.emoji_state =
                        detail::grapheme_break_emoji_state_t::none;
                    continue;
                } else {
                    state.emoji_state =
                        detail::grapheme_break_emoji_state_t::none;
                }
            } else if (state.prop == grapheme_prop_t::Regional_Indicator) {
                state.emoji_state =
                    detail::grapheme_break_emoji_state_t::first_emoji;
            }

            if (detail::table_grapheme_break(state.prev_prop, state.prop))
                return state.it;
        }

        return state.it;
    }

}}

#endif
