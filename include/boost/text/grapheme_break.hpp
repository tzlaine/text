// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_GRAPHEME_BREAK_HPP
#define BOOST_TEXT_GRAPHEME_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/detail/pipeable_view.hpp>

#include <boost/assert.hpp>

#include <algorithm>
#include <array>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text {

    /** The grapheme properties defined by Unicode. */
    enum class grapheme_property {
        Other,
        CR,
        LF,
        Control,
        Extend,
        Regional_Indicator,
        Prepend,
        SpacingMark,
        L,
        V,
        T,
        LV,
        LVT,
        ExtPict,
        ZWJ
    };

    namespace detail {
        struct grapheme_prop_interval
        {
            uint32_t lo_;
            uint32_t hi_;
            grapheme_property prop_;
        };

        inline bool operator<(
            grapheme_prop_interval lhs, grapheme_prop_interval rhs)
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<grapheme_prop_interval, 6> const &
        make_grapheme_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, grapheme_property>
        make_grapheme_prop_map();
    }

    /** Returns the grapheme property associated with code point `cp`. */
    inline grapheme_property grapheme_prop(uint32_t cp)
    {
        static auto const map = detail::make_grapheme_prop_map();
        static auto const intervals = detail::make_grapheme_prop_intervals();

        auto const it = map.find(cp);
        if (it == map.end()) {
            auto const it2 = std::lower_bound(
                intervals.begin(),
                intervals.end(),
                detail::grapheme_prop_interval{cp, cp + 1});
            if (it2 == intervals.end() || cp < it2->lo_ || it2->hi_ <= cp)
                return grapheme_property::Other;
            return it2->prop_;
        }
        return it->second;
    }

    namespace detail {
        inline bool skippable(grapheme_property prop)
        {
            return prop == grapheme_property::Extend;
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

            grapheme_property prev_prop;
            grapheme_property prop;

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
            --state.it;
            state.prop = state.prev_prop;
            return state;
        }

        template<typename CPIter>
        bool gb11_prefix(CPIter first, CPIter prev_it)
        {
            auto final_prop = grapheme_property::Other;
            boost::text::find_if_backward(
                first, prev_it, [&final_prop](uint32_t cp) {
                    final_prop = grapheme_prop(cp);
                    return final_prop != grapheme_property::Extend;
                });
            return final_prop == grapheme_property::ExtPict;
        }

        inline bool table_grapheme_break(
            grapheme_property lhs, grapheme_property rhs)
        {
            // Note that RI.RI was changed to '1' since that case is handled
            // in the grapheme break FSM.

            // clang-format off
// See chart at https://unicode.org/Public/11.0.0/ucd/auxiliary/GraphemeBreakTest.html .
constexpr std::array<std::array<bool, 15>, 15> grapheme_breaks = {{
//   Other CR LF Ctrl Ext RI Pre SpcMk L  V  T  LV LVT ExtPict ZWJ
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // Other
    {{1,   1, 0, 1,   1,  1, 1,  1,    1, 1, 1, 1, 1,  1,      1}}, // CR
    {{1,   1, 1, 1,   1,  1, 1,  1,    1, 1, 1, 1, 1,  1,      1}}, // LF
    {{1,   1, 1, 1,   1,  1, 1,  1,    1, 1, 1, 1, 1,  1,      1}}, // Control
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // Extend
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // RI
    {{0,   1, 1, 1,   0,  0, 0,  0,    0, 0, 0, 0, 0,  0,      0}}, // Prepend
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // SpacingMark
    {{1,   1, 1, 1,   0,  1, 1,  0,    0, 0, 1, 0, 0,  1,      0}}, // L
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 0, 0, 1, 1,  1,      0}}, // V
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 0, 1, 1,  1,      0}}, // T
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 0, 0, 1, 1,  1,      0}}, // LV
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 0, 1, 1,  1,      0}}, // LVT
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // ExtPict
    {{1,   1, 1, 1,   0,  1, 1,  0,    1, 1, 1, 1, 1,  1,      0}}, // ZWJ

}};
            // clang-format on
            auto const lhs_int = static_cast<int>(lhs);
            auto const rhs_int = static_cast<int>(rhs);
            return grapheme_breaks[lhs_int][rhs_int];
        }

        template<typename CPIter, typename Sentinel>
        CPIter prev_grapheme_break_impl(
            CPIter first, CPIter it, Sentinel last)
        {
            if (it == first)
                return it;

            if (it == last && --it == first)
                return it;

            grapheme_break_state<CPIter> state;
            state.it = it;
            state.prop = boost::text::grapheme_prop(*state.it);
            state.prev_prop =
                boost::text::grapheme_prop(*detail::prev(state.it));
            state.emoji_state = grapheme_break_emoji_state_t::none;

            for (; state.it != first; state = prev(state)) {
                state.prev_prop =
                    boost::text::grapheme_prop(*detail::prev(state.it));

                // When we see an RI, back up to the first RI so we can see what
                // emoji state we're supposed to be in here.
                if (state.emoji_state == grapheme_break_emoji_state_t::none &&
                    state.prop == grapheme_property::Regional_Indicator) {
                    int ris_before = 0;
                    boost::text::find_if_not_backward(
                        first, state.it, [&ris_before](uint32_t cp) {
                            bool const ri =
                                grapheme_prop(cp) ==
                                grapheme_property::Regional_Indicator;
                            if (ri)
                                ++ris_before;
                            return ri;
                        });
                    state.emoji_state =
                        (ris_before % 2 == 0)
                            ? grapheme_break_emoji_state_t::first_emoji
                            : grapheme_break_emoji_state_t::second_emoji;
                }

                // GB11
                if (state.prev_prop == grapheme_property::ZWJ &&
                    state.prop == grapheme_property::ExtPict &&
                    detail::gb11_prefix(first, detail::prev(state.it))) {
                    continue;
                }

                if (state.emoji_state ==
                    grapheme_break_emoji_state_t::first_emoji) {
                    if (state.prev_prop ==
                        grapheme_property::Regional_Indicator) {
                        state.emoji_state =
                            grapheme_break_emoji_state_t::second_emoji;
                        return state.it;
                    } else {
                        state.emoji_state = grapheme_break_emoji_state_t::none;
                    }
                } else if (
                    state.emoji_state ==
                        grapheme_break_emoji_state_t::second_emoji &&
                    state.prev_prop == grapheme_property::Regional_Indicator) {
                    state.emoji_state =
                        grapheme_break_emoji_state_t::first_emoji;
                    continue;
                }

                if (detail::table_grapheme_break(state.prev_prop, state.prop))
                    return state.it;
            }

            return first;
        }

        template<typename CPIter, typename Sentinel>
        CPIter next_grapheme_break_impl(CPIter first, Sentinel last)
        {
            if (first == last)
                return first;

            grapheme_break_state<CPIter> state;
            state.it = first;

            if (++state.it == last)
                return state.it;

            state.prev_prop =
                boost::text::grapheme_prop(*detail::prev(state.it));
            state.prop = boost::text::grapheme_prop(*state.it);

            state.emoji_state =
                state.prev_prop == grapheme_property::Regional_Indicator
                    ? grapheme_break_emoji_state_t::first_emoji
                    : grapheme_break_emoji_state_t::none;

            for (; state.it != last; state = next(state)) {
                state.prop = boost::text::grapheme_prop(*state.it);

                // GB11
                if (state.prev_prop == grapheme_property::ZWJ &&
                    state.prop == grapheme_property::ExtPict &&
                    detail::gb11_prefix(first, detail::prev(state.it))) {
                    continue;
                }

                if (state.emoji_state ==
                    grapheme_break_emoji_state_t::first_emoji) {
                    if (state.prop == grapheme_property::Regional_Indicator) {
                        state.emoji_state = grapheme_break_emoji_state_t::none;
                        continue;
                    } else {
                        state.emoji_state = grapheme_break_emoji_state_t::none;
                    }
                } else if (
                    state.prop == grapheme_property::Regional_Indicator) {
                    state.emoji_state =
                        grapheme_break_emoji_state_t::first_emoji;
                }

                if (detail::table_grapheme_break(state.prev_prop, state.prop))
                    return state.it;
            }

            return state.it;
        }

        template<typename CPIter, typename Sentinel>
        struct next_grapheme_callable
        {
            CPIter operator()(CPIter it, Sentinel last) const
            {
                return detail::next_grapheme_break_impl(it, last);
            }
        };

        template<typename CPIter>
        struct prev_grapheme_callable
        {
            CPIter operator()(CPIter first, CPIter it, CPIter last) const
            {
                return detail::prev_grapheme_break_impl(first, it, last);
            }
        };
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest grapheme break at or before before `it`.  If `it ==
        first`, that is returned.  Otherwise, the first code point of the
        grapheme that `it` is within is returned (even if `it` is already at
        the first code point of a grapheme). */
    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_grapheme_break(I first, I it, S last);

    /** Finds the next word break after `first`.  This will be the first code
        point after the current word, or `last` if no next word exists.

        \pre `first` is at the beginning of a word. */
    template<code_point_iter I, std::sentinel_for<I> S>
    I next_grapheme_break(I first, S last);

    /** Finds the nearest grapheme break at or before before `it`.  If `it ==
        range.begin()`, that is returned.  Otherwise, the first code point of
        the grapheme that `it` is within is returned (even if `it` is already
        at the first code point of a grapheme). */
    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> prev_grapheme_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Finds the next grapheme break after `it`.  This will be the first code
        point after the current grapheme, or `range.end()` if no next grapheme
        exists.

        \pre `it` is at the beginning of a grapheme. */
    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> next_grapheme_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns true iff `it` is at the beginning of a grapheme, or `it ==
        last`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_grapheme_break(I first, I it, S last);

    /** Returns true iff `it` is at the beginning of a grapheme, or `it ==
        std::ranges::end(range)`. */
    template<code_point_range R>
    bool at_grapheme_break(R && r, std::ranges::iterator_t<R> it);

#else

    template<typename CPIter, typename Sentinel>
    auto prev_grapheme_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::prev_grapheme_break_impl(first, it, last);
    }

    template<typename CPIter, typename Sentinel>
    auto next_grapheme_break(CPIter first, Sentinel last)
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::next_grapheme_break_impl(first, last);
    }

    template<typename CPRange, typename CPIter>
    auto prev_grapheme_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return v1::prev_grapheme_break(
            detail::begin(range), it, detail::end(range));
    }

    template<typename CPRange, typename CPIter>
    auto next_grapheme_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return v1::next_grapheme_break(it, detail::end(range));
    }

    template<typename CPIter, typename Sentinel>
    auto at_grapheme_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        if (it == last)
            return true;
        return v1::prev_grapheme_break(first, it, last) == it;
    }

    template<typename CPRange, typename CPIter>
    auto at_grapheme_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        if (it == detail::end(range))
            return true;
        return v1::prev_grapheme_break(
                   detail::begin(range), it, detail::end(range)) == it;
    }

#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    // TODO: These need to be converted from code point inputs to UTF-N
    // inputs, following P2745.

    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_grapheme_break(I first, I it, S last)
    {
        return detail::prev_grapheme_break_impl(first, it, last);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    I next_grapheme_break(I first, S last)
    {
        return detail::next_grapheme_break_impl(first, last);
    }

    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> prev_grapheme_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return boost::text::prev_grapheme_break(
            detail::begin(r), it, detail::end(r));
    }

    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> next_grapheme_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return boost::text::next_grapheme_break(it, detail::end(r));
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_grapheme_break(I first, I it, S last)
    {
        if (it == last)
            return true;
        return boost::text::prev_grapheme_break(first, it, last) == it;
    }

    template<code_point_range R>
    bool at_grapheme_break(R && r, std::ranges::iterator_t<R> it)
    {
        if (it == detail::end(r))
            return true;
        return boost::text::prev_grapheme_break(
                   detail::begin(r), it, detail::end(r)) == it;
    }

}}}

#endif

#endif
