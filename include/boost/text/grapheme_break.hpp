#ifndef BOOST_TEXT_GRAPHEME_BREAK_HPP
#define BOOST_TEXT_GRAPHEME_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/lazy_segment_range.hpp>
#include <boost/text/utility.hpp>

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
            bool it_points_to_prev = false;

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

        template<typename CPIter>
        grapheme_break_state<CPIter> prev(grapheme_break_state<CPIter> state)
        {
            if (!state.it_points_to_prev)
                --state.it;
            state.it_points_to_prev = false;
            state.prop = state.prev_prop;
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
        if (it == first)
            return it;

        if (it == last && --it == first)
            return it;

        detail::grapheme_break_state<CPIter> state;
        state.it = it;
        state.prop = grapheme_prop(*state.it);
        state.prev_prop = grapheme_prop(*std::prev(state.it));
        state.emoji_state = detail::grapheme_break_emoji_state_t::none;

        // GB10
        auto skip = [](detail::grapheme_break_state<CPIter> state,
                       CPIter first) {
            if (state.prop == grapheme_prop_t::E_Modifier &&
                detail::skippable(state.prev_prop)) {
                auto temp_it =
                    find_if_not_backward(first, state.it, [](uint32_t cp) {
                        return detail::skippable(grapheme_prop(cp));
                    });
                if (temp_it == state.it)
                    return state;
                auto temp_prev_prop = grapheme_prop(*temp_it);
                if (temp_prev_prop == grapheme_prop_t::E_Base ||
                    temp_prev_prop == grapheme_prop_t::E_Base_GAZ) {
                    state.it = temp_it;
                    state.it_points_to_prev = true;
                    state.prev_prop = temp_prev_prop;
                }
            }
            return state;
        };

        for (; state.it != first; state = prev(state)) {
            state.prev_prop = grapheme_prop(*std::prev(state.it));

            // When we see an RI, back up to the first RI so we can see what
            // emoji state we're supposed to be in here.
            if (state.emoji_state == detail::grapheme_break_emoji_state_t::none &&
                state.prop == grapheme_prop_t::Regional_Indicator) {
                int ris_before = 0;
                find_if_not_backward(
                    first, state.it, [&ris_before](uint32_t cp) {
                        bool const ri = grapheme_prop(cp) ==
                                        grapheme_prop_t::Regional_Indicator;
                        if (ri)
                            ++ris_before;
                        return ri;
                    });
                state.emoji_state =
                    (ris_before % 2 == 0)
                        ? detail::grapheme_break_emoji_state_t::first_emoji
                        : detail::grapheme_break_emoji_state_t::second_emoji;
            }

            // If we end up breaking durign this iteration, we want the break
            // to show up after the skip, so that the skippable CPs go with
            // the CP before them.  This is to maintain symmetry with
            // next_grapheme_break().
            auto after_skip_it = state.it;

            // Puting this here means not having to do it explicitly below
            // between prev_prop and prop (and transitively, between prop and
            // next_prop).
            state = skip(state, first);

            if (state.emoji_state ==
                detail::grapheme_break_emoji_state_t::first_emoji) {
                if (state.prev_prop == grapheme_prop_t::Regional_Indicator) {
                    state.emoji_state =
                        detail::grapheme_break_emoji_state_t::second_emoji;
                    return after_skip_it;
                } else {
                    state.emoji_state = detail::grapheme_break_emoji_state_t::none;
                }
            } else if (
                state.emoji_state ==
                    detail::grapheme_break_emoji_state_t::second_emoji &&
                state.prev_prop == grapheme_prop_t::Regional_Indicator) {
                state.emoji_state =
                    detail::grapheme_break_emoji_state_t::first_emoji;
                continue;
            }

            if (detail::table_grapheme_break(state.prev_prop, state.prop))
                return after_skip_it;
        }

        return first;
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

    namespace detail {
        template<typename CPIter, typename Sentinel>
        struct next_grapheme_callable
        {
            CPIter operator()(CPIter it, Sentinel last) noexcept
            {
                return next_grapheme_break(it, last);
            }
        };
    }

    /** Returns the bounds of the grapheme that <code>it</code> lies within. */
    template<typename CPIter, typename Sentinel>
    inline cp_range<CPIter>
    grapheme(CPIter first, CPIter it, Sentinel last) noexcept
    {
        first = prev_grapheme_break(first, it, last);
        return cp_range<CPIter>{first, next_grapheme_break(first, last)};
    }

    /** Returns a lazy range of the code point ranges delimiting graphemes in
        <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_grapheme_callable<CPIter, Sentinel>>
    graphemes(CPIter first, Sentinel last) noexcept
    {
        return {{first, last}, {first, last}};
    }

}}

#endif
