#ifndef BOOST_TEXT_LINE_BREAK_HPP
#define BOOST_TEXT_LINE_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/lazy_segment_range.hpp>

#include <algorithm>
#include <array>

#include <cassert>
#include <stdint.h>


namespace boost { namespace text {

    /** The line properties outlined in Unicode 10. */
    enum class line_prop_t {
        AL,
        B2,
        BA,
        BB,
        BK,
        CB,
        CL,
        CP,
        CR,
        EX,
        GL,
        H2,
        H3,
        HL,
        HY,
        ID,
        IN,
        IS,
        JL,
        JT,
        JV,
        LF,
        NL,
        NS,
        NU,
        OP,
        PO,
        PR,
        QU,
        SP,
        SY,
        WJ,
        ZW,
        RI,
        EB,
        EM,
        CM,
        ZWJ,
        AI,
        XX,
        SA,
        CJ
    };

    /** Returns the line property associated with code point \a cp. */
    line_prop_t line_prop(uint32_t cp) noexcept;

    namespace detail {
        // Note that whereas the other kinds of breaks have an 'Other', line
        // break has 'XX'.  However, due to Rule LB1, XX is replaced with AL,
        // so you'll see a lot of initializations from AL in this file.

        inline bool skippable(line_prop_t prop) noexcept
        {
            return prop == line_prop_t::CM || prop == line_prop_t::ZWJ;
        }

        // Can represent the "X" in "X(CM|ZWJ)* -> X" in the LB9 rule.
        inline bool lb9_x(line_prop_t prop) noexcept
        {
            return prop != line_prop_t::BK && prop != line_prop_t::CR &&
                   prop != line_prop_t::LF && prop != line_prop_t::NL &&
                   prop != line_prop_t::SP && prop != line_prop_t::ZW;
        }

        inline line_prop_t lb10(line_prop_t prop) noexcept
        {
            if (prop == line_prop_t::CM || prop == line_prop_t::ZWJ)
                return line_prop_t::AL;
            return prop;
        }

        enum class line_break_emoji_state_t {
            none,
            first_emoji, // Indicates that prop points to an odd-count emoji.
            second_emoji // Indicates that prop points to an even-count emoji.
        };

        template<typename CPIter>
        struct line_break_state
        {
            CPIter it;
            bool it_points_to_prev = false;

            line_prop_t prev_prev_prop;
            line_prop_t prev_prop;
            line_prop_t prop;
            line_prop_t next_prop;

            line_break_emoji_state_t emoji_state;
        };

        template<typename CPIter>
        line_break_state<CPIter> next(line_break_state<CPIter> state)
        {
            ++state.it;
            state.prev_prev_prop = state.prev_prop;
            state.prev_prop = state.prop;
            state.prop = state.next_prop;
            return state;
        }

        template<typename CPIter>
        line_break_state<CPIter> prev(line_break_state<CPIter> state)
        {
            if (!state.it_points_to_prev)
                --state.it;
            state.it_points_to_prev = false;
            state.next_prop = state.prop;
            state.prop = state.prev_prop;
            state.prev_prop = state.prev_prev_prop;
            return state;
        }

        inline bool table_line_break(line_prop_t lhs, line_prop_t rhs) noexcept
        {
            // clang-format off
// See chart at http://www.unicode.org/Public/10.0.0/ucd/auxiliary/LineBreakTest.html
constexpr std::array<std::array<bool, 42>, 42> line_breaks = {{
//   AL B2 BA BB BK CB CL CP CR EX GL H2 H3 HL HY ID IN IS JL JT JV LF NL NS NU OP PO PR QU SP SY WJ ZW RI EB EM CM ZWJ AI XX SA CJ
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // AL
    {{1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // B2
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // BA

    {{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0}}, // BB
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // BK
    {{1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 1}}, // CB

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // CL
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // CP
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // CR

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // EX
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0}}, // GL
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // H2

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // H3
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // HL
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // HY

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // ID
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // IN
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // IS

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // JL
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // JT
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // JV

    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // LF
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // NL
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // NS

    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // NU
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0}}, // OP
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // PO

    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0}}, // PR
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0}}, // QU
    {{1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // SP

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // SY
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0}}, // WJ
    {{1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1}}, // ZW

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // RI
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,  1, 1, 1, 0}}, // EB
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // EM

    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // CM
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0}}, // ZWJ
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // AI

    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // XX
    {{0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0}}, // SA
    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // CJ
}};
            // clang-format on
            auto const lhs_int = static_cast<int>(lhs);
            auto const rhs_int = static_cast<int>(rhs);
            return line_breaks[lhs_int][rhs_int];
        }

        // LB9
        template<typename CPIter, typename Sentinel>
        line_break_state<CPIter> skip_forward(
            line_break_state<CPIter> state, CPIter first, Sentinel last)
        {
            if (state.it != first && !skippable(state.prev_prop) &&
                lb9_x(state.prev_prop) && skippable(state.prop)) {
                auto temp_it =
                    find_if_not(std::next(state.it), last, [](uint32_t cp) {
                        return skippable(line_prop(cp));
                    });
                if (temp_it == last) {
                    state.it = last;
                } else {
                    auto const temp_prop = line_prop(*temp_it);
                    state.it = temp_it;
                    state.prop = temp_prop;
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                }
            }
            return state;
        }

        template<
            typename CPIter,
            typename Sentinel,
            typename BeforeFunc,
            typename AfterFunc>
        line_break_state<CPIter> skip_forward_spaces_between(
            line_break_state<CPIter> state,
            Sentinel last,
            BeforeFunc before,
            AfterFunc after)
        {
            if (before(state.prev_prop)) {
                auto const it = find_if_not(state.it, last, [](uint32_t cp) {
                    return line_prop(cp) == line_prop_t::SP;
                });
                if (it == last)
                    return state;
                auto const temp_prop = line_prop(*it);
                if (after(temp_prop)) {
                    state.it = it;
                    state.prop = temp_prop;
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                }
            }
            return state;
        }

        template<
            typename CPIter,
            typename Sentinel,
            typename BeforeFunc,
            typename AfterFunc>
        line_break_state<CPIter> skip_backward_spaces_between(
            line_break_state<CPIter> state,
            Sentinel last,
            BeforeFunc before,
            AfterFunc after)
        {
            if (before(state.prev_prop)) {
                auto const it =
                    find_if_not_backward(state.it, last, [](uint32_t cp) {
                        return line_prop(cp) == line_prop_t::SP;
                    });
                if (it == last)
                    return state;
                auto const temp_prop = line_prop(*it);
                if (after(temp_prop)) {
                    state.it = it;
                    state.prop = temp_prop;
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                }
            }
            return state;
        }

        template<typename CPIter>
        struct scoped_emoji_state
        {
            scoped_emoji_state(line_break_state<CPIter> & state) :
                state_(state),
                released_(false)
            {}
            ~scoped_emoji_state()
            {
                if (!released_)
                    state_.emoji_state = line_break_emoji_state_t::none;
            }
            void release() { released_ = false; }

        private:
            line_break_state<CPIter> & state_;
            bool released_;
        };

        template<typename CPIter, typename Sentinel>
        inline CPIter prev_line_break_impl(
            CPIter first,
            CPIter it,
            Sentinel last,
            bool hard_breaks_only) noexcept
        {
            if (it == first)
                return it;

            if (it == last && --it == first)
                return it;

            detail::line_break_state<CPIter> state;

            state.it = it;

            state.prop = line_prop(*state.it);

            // Special case: If state.prop is skippable, we need to skip
            // backward until we find a non-skippable.
            if (detail::skippable(state.prop)) {
                state.it = find_if_not_backward(first, it, [](uint32_t cp) {
                    return detail::skippable(line_prop(cp));
                });
                state.next_prop = line_prop(*std::next(state.it));
                state.prop = line_prop(*state.it);

                // If we end up on a non-skippable that should break before the
                // skippable(s) we just moved over, break on the last skippable.
                if (!detail::skippable(state.prop) &&
                    detail::table_line_break(state.prop, state.next_prop)) {
                    return ++state.it;
                }
                if (state.it == first)
                    return first;
            }

            state.prev_prev_prop = line_prop_t::AL;
            if (std::prev(state.it) != first)
                state.prev_prev_prop = line_prop(*std::prev(state.it, 2));
            state.prev_prop = line_prop(*std::prev(state.it));
            state.next_prop = line_prop_t::AL;
            if (std::next(state.it) != last)
                state.next_prop = line_prop(*std::next(state.it));

            // Since 'it' may be anywhere within the line in which it sits, we
            // need to look forward to make sure that next_prop doesn't point
            // to skippables.
            {
                if (std::next(state.it) != last) {
                    auto temp_state = state;
                    temp_state = next(temp_state);
                    temp_state = detail::skip_forward(temp_state, first, last);
                    if (temp_state.it == last)
                        state.next_prop = line_prop_t::AL;
                    else
                        state.next_prop = temp_state.prop;
                }
            }

            state.emoji_state = detail::line_break_emoji_state_t::none;

            auto skip = [](detail::line_break_state<CPIter> state,
                           CPIter first) {
                if (detail::skippable(state.prev_prop)) {
                    auto temp_it =
                        find_if_not_backward(first, state.it, [](uint32_t cp) {
                            return detail::skippable(line_prop(cp));
                        });
                    if (temp_it == state.it)
                        return state;
                    auto temp_prev_prop = line_prop(*temp_it);
                    state.it = temp_it;
                    state.it_points_to_prev = true;
                    state.prev_prop = temp_prev_prop;
                    if (temp_it == first)
                        state.prev_prev_prop = line_prop_t::AL;
                    else
                        state.prev_prev_prop = line_prop(*std::prev(temp_it));
                }
                return state;
            };

            for (; state.it != last; state = next(state)) {
                if (std::prev(state.it) != first)
                    state.prev_prev_prop = line_prop(*std::prev(state.it, 2));
                else
                    state.prev_prev_prop = line_prop_t::AL;

                // LB1 (These should have been handled in data generation.)
                assert(state.prev_prop != line_prop_t::AI);
                assert(state.prop != line_prop_t::AI);
                assert(state.prev_prop != line_prop_t::XX);
                assert(state.prop != line_prop_t::XX);
                assert(state.prev_prop != line_prop_t::SA);
                assert(state.prop != line_prop_t::SA);
                assert(state.prev_prop != line_prop_t::CJ);
                assert(state.prop != line_prop_t::CJ);

                // When we see an RI, back up to the first RI so we can see what
                // emoji state we're supposed to be in here.
                if (state.emoji_state ==
                        detail::line_break_emoji_state_t::none &&
                    state.prop == line_prop_t::RI) {
                    auto temp_state = state;
                    int ris_before = 0;
                    while (temp_state.it != first) {
                        temp_state = skip(temp_state, first);
                        if (temp_state.it == first) {
                            if (temp_state.prev_prop == line_prop_t::RI) {
                                ++ris_before;
                            }
                            break;
                        }
                        if (temp_state.prev_prop == line_prop_t::RI) {
                            temp_state = prev(temp_state);
                            if (temp_state.it != first &&
                                std::prev(temp_state.it) != first) {
                                temp_state.prev_prev_prop =
                                    line_prop(*std::prev(temp_state.it, 2));
                            } else {
                                temp_state.prev_prev_prop = line_prop_t::AL;
                            }
                            ++ris_before;
                        } else {
                            break;
                        }
                    }
                    state.emoji_state =
                        (ris_before % 2 == 0)
                            ? detail::line_break_emoji_state_t::first_emoji
                            : detail::line_break_emoji_state_t::second_emoji;
                }

                // LB4
                if (state.prev_prop == line_prop_t::BK)
                    return state.it;

                // LB5
                if (state.prev_prop == line_prop_t::CR &&
                    state.prop == line_prop_t::LF) {
                    continue;
                }
                if (state.prev_prop == line_prop_t::CR ||
                    state.prev_prop == line_prop_t::LF ||
                    state.prev_prop == line_prop_t::NL) {
                    return state.it;
                }

                if (hard_breaks_only)
                    continue;

                // LB6
                auto lb6 = [](line_prop_t prop) {
                    return prop == line_prop_t::BK || prop == line_prop_t::CR ||
                           prop == line_prop_t::LF || prop == line_prop_t::NL;
                };
                if (lb6(state.prop))
                    continue;

                // LB7
                // Even though a space means no break, we need to defer our
                // early return until after we've seen if the space will be
                // consumed in LB14-LB17 below.
                bool const lb7_space = state.prop == line_prop_t::SP;
                if (state.prop == line_prop_t::ZW)
                    continue;

                // LB8
                if (state.prev_prop == line_prop_t::ZW && !lb7_space)
                    return state.it;
                if (state.prev_prop == line_prop_t::ZW &&
                    state.prop == line_prop_t::SP) {
                    auto it = find_if_not(state.it, last, [](uint32_t cp) {
                        return line_prop(cp) == line_prop_t::SP;
                    });
                    if (it == last)
                        return it;
                    auto const prop = line_prop(*it);
                    if (!lb6(prop) && prop != line_prop_t::ZW)
                        return it;
                }

                // LB8a
                if (state.prev_prop == line_prop_t::ZWJ &&
                    (state.prop == line_prop_t::ID ||
                     state.prop == line_prop_t::EB ||
                     state.prop == line_prop_t::EM)) {
                    continue;
                }

                // If we end up breaking durign this iteration, we want the
                // break to show up after the skip, so that the skippable CPs go
                // with the CP before them.  This is to maintain symmetry with
                // next_line_break().
                auto after_skip_it = state.it;

                // LB9
                // Puting this here means not having to do it explicitly below
                // between prop and next_prop (and transitively, between
                // prev_prop and prop).
                state = skip(state, first);
                if (state.it == last)
                    return state.it;

                // LB10
                // Inexplicably, implementing this (as required in TR14)
                // breaks a bunch of tests.

                // LB11
                if (state.prop == line_prop_t::WJ ||
                    state.prev_prop == line_prop_t::WJ)
                    continue;

                // LB12
                if (state.prev_prop == line_prop_t::GL)
                    continue;

                // LB12a
                if ((state.prev_prop != line_prop_t::SP &&
                     state.prev_prop != line_prop_t::BA &&
                     state.prev_prop != line_prop_t::HY) &&
                    state.prop == line_prop_t::GL) {
                    continue;
                }

                // Used in LB24.
                auto after_nu = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_prop_t::NU || prop == line_prop_t::SY ||
                           prop == line_prop_t::IS;
                };

                // LB13
                if (state.prop == line_prop_t::CL ||
                    state.prop == line_prop_t::CP) {
                    // We know from this rule alone that there's no break
                    // here, but we also need to look ahead at whether LB16
                    // applies, since if we didn't, we'd bail out before ever
                    // reaching it due to LB12a above on the next iteration.
                    if (std::next(state.it) != last) {
                        // LB16
                        auto next_state = next(state);
                        if (std::next(next_state.it) != last) {
                            next_state.next_prop =
                                line_prop(*std::next(next_state.it));
                        } else {
                            next_state.next_prop = line_prop_t::AL;
                        }

                        auto const new_state = skip_forward_spaces_between(
                            next_state,
                            last,
                            [](line_prop_t prop) {
                                return prop == line_prop_t::CL ||
                                       prop == line_prop_t::CP;
                            },
                            [](line_prop_t prop) {
                                return prop == line_prop_t::NS;
                            });

                        if (new_state.it == last)
                            return new_state.it;
                        if (new_state.it != next_state.it)
                            state = new_state;
                    }
                    continue;
                }
                if (state.prop == line_prop_t::EX ||
                    state.prop == line_prop_t::IS ||
                    state.prop == line_prop_t::SY) {
                    // As above, we need to check for the pattern
                    // NU(NU|SY|IS)* from LB24, even though without it we will
                    // still break here.

                    if (state.prev_prop == line_prop_t::NU &&
                        after_nu(*state.it)) {
                        auto it = find_if_not(state.it, last, after_nu);
                        state.it = --it;
                        state.prop = line_prop(*state.it);
                        state.next_prop = line_prop_t::AL;
                        if (std::next(state.it) != last)
                            state.next_prop = line_prop(*std::next(state.it));
                    }

                    continue;
                }

                // LB14
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::OP;
                        },
                        [](line_prop_t prop) { return true; });
                    if (new_state.it != state.it) {
                        state = prev(new_state);
                        continue;
                    }
                }

                // LB15
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::QU;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::OP;
                        });
                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB16 is handled as part of LB13.
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::CL ||
                                   prop == line_prop_t::CP;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::NS;
                        });

                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB17
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        });
                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                if (lb7_space)
                    continue;

                // LB24
                if (state.prev_prop == line_prop_t::NU && after_nu(*state.it)) {
                    auto it = find_if_not(state.it, last, after_nu);
                    state.it = --it;
                    state.prop = line_prop(*state.it);
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                    continue;
                }

                // LB21a
                if (state.prev_prev_prop == line_prop_t::HL &&
                    (state.prev_prop == line_prop_t::HY ||
                     state.prev_prop == line_prop_t::BA)) {
                    continue;
                }

                if (state.emoji_state ==
                    detail::line_break_emoji_state_t::first_emoji) {
                    if (state.prev_prop == line_prop_t::RI) {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::second_emoji;
                        return after_skip_it;
                    } else {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::none;
                    }
                } else if (
                    state.emoji_state ==
                        detail::line_break_emoji_state_t::second_emoji &&
                    state.prev_prop == line_prop_t::RI) {
                    state.emoji_state =
                        detail::line_break_emoji_state_t::first_emoji;
                    continue;
                }

                if (detail::table_line_break(state.prev_prop, state.prop))
                    return after_skip_it;
            }

            return first;
        }

        template<typename CPIter, typename Sentinel>
        inline CPIter next_line_break_impl(
            CPIter first, Sentinel last, bool hard_breaks_only) noexcept
        {
            if (first == last)
                return first;

            line_break_state<CPIter> state;
            state.it = first;

            if (++state.it == last)
                return state.it;

            state.prev_prev_prop = line_prop_t::AL;
            state.prev_prop = line_prop(*first);
            state.prop = line_prop(*state.it);
            state.next_prop = line_prop_t::AL;
            if (std::next(state.it) != last)
                state.next_prop = line_prop(*std::next(state.it));

            state.emoji_state = state.prev_prop == line_prop_t::RI
                                    ? line_break_emoji_state_t::first_emoji
                                    : line_break_emoji_state_t::none;

            for (; state.it != last; state = next(state)) {
                if (std::next(state.it) != last)
                    state.next_prop = line_prop(*std::next(state.it));
                else
                    state.next_prop = line_prop_t::AL;

                scoped_emoji_state<CPIter> emoji_state_setter(state);

                // LB1 (These should have been handled in data generation.)
                assert(state.prev_prop != line_prop_t::AI);
                assert(state.prop != line_prop_t::AI);
                assert(state.prev_prop != line_prop_t::XX);
                assert(state.prop != line_prop_t::XX);
                assert(state.prev_prop != line_prop_t::SA);
                assert(state.prop != line_prop_t::SA);
                assert(state.prev_prop != line_prop_t::CJ);
                assert(state.prop != line_prop_t::CJ);

                // LB4
                if (state.prev_prop == line_prop_t::BK)
                    return state.it;

                // LB5
                if (state.prev_prop == line_prop_t::CR &&
                    state.prop == line_prop_t::LF) {
                    continue;
                }
                if (state.prev_prop == line_prop_t::CR ||
                    state.prev_prop == line_prop_t::LF ||
                    state.prev_prop == line_prop_t::NL) {
                    return state.it;
                }

                if (hard_breaks_only)
                    continue;

                // LB6
                auto lb6 = [](line_prop_t prop) {
                    return prop == line_prop_t::BK || prop == line_prop_t::CR ||
                           prop == line_prop_t::LF || prop == line_prop_t::NL;
                };
                if (lb6(state.prop))
                    continue;

                // LB7
                // Even though a space means no break, we need to defer our
                // early return until after we've seen if the space will be
                // consumed in LB14-LB17 below.
                bool const lb7_space = state.prop == line_prop_t::SP;
                if (state.prop == line_prop_t::ZW)
                    continue;

                // LB8
                if (state.prev_prop == line_prop_t::ZW && !lb7_space)
                    return state.it;
                if (state.prev_prop == line_prop_t::ZW &&
                    state.prop == line_prop_t::SP) {
                    auto it = find_if_not(state.it, last, [](uint32_t cp) {
                        return line_prop(cp) == line_prop_t::SP;
                    });
                    if (it == last)
                        return it;
                    auto const prop = line_prop(*it);
                    if (!lb6(prop) && prop != line_prop_t::ZW)
                        return it;
                }

                // LB8a
                if (state.prev_prop == line_prop_t::ZWJ &&
                    (state.prop == line_prop_t::ID ||
                     state.prop == line_prop_t::EB ||
                     state.prop == line_prop_t::EM)) {
                    continue;
                }

                // LB9
                // Puting this here means not having to do it explicitly below
                // between prop and next_prop (and transitively, between
                // prev_prop and prop).
                state = skip_forward(state, first, last);
                if (state.it == last)
                    return state.it;

                // LB10
                // Inexplicably, implementing this (as required in TR14)
                // breaks a bunch of tests.

                // LB11
                if (state.prop == line_prop_t::WJ ||
                    state.prev_prop == line_prop_t::WJ)
                    continue;

                // LB12
                if (state.prev_prop == line_prop_t::GL)
                    continue;

                // LB12a
                if ((state.prev_prop != line_prop_t::SP &&
                     state.prev_prop != line_prop_t::BA &&
                     state.prev_prop != line_prop_t::HY) &&
                    state.prop == line_prop_t::GL) {
                    continue;
                }

                // Used in LB24.
                auto after_nu = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_prop_t::NU || prop == line_prop_t::SY ||
                           prop == line_prop_t::IS;
                };

                // LB13
                if (state.prop == line_prop_t::CL ||
                    state.prop == line_prop_t::CP) {
                    // We know from this rule alone that there's no break
                    // here, but we also need to look ahead at whether LB16
                    // applies, since if we didn't, we'd bail out before ever
                    // reaching it due to LB12a above on the next iteration.
                    if (std::next(state.it) != last) {
                        // LB16
                        auto next_state = next(state);
                        if (std::next(next_state.it) != last) {
                            next_state.next_prop =
                                line_prop(*std::next(next_state.it));
                        } else {
                            next_state.next_prop = line_prop_t::AL;
                        }

                        auto const new_state = skip_forward_spaces_between(
                            next_state,
                            last,
                            [](line_prop_t prop) {
                                return prop == line_prop_t::CL ||
                                       prop == line_prop_t::CP;
                            },
                            [](line_prop_t prop) {
                                return prop == line_prop_t::NS;
                            });

                        if (new_state.it == last)
                            return new_state.it;
                        if (new_state.it != next_state.it)
                            state = new_state;
                    }
                    continue;
                }
                if (state.prop == line_prop_t::EX ||
                    state.prop == line_prop_t::IS ||
                    state.prop == line_prop_t::SY) {
                    // As above, we need to check for the pattern
                    // NU(NU|SY|IS)* from LB24, even though without it we will
                    // still break here.

                    if (state.prev_prop == line_prop_t::NU &&
                        after_nu(*state.it)) {
                        auto it = find_if_not(state.it, last, after_nu);
                        state.it = --it;
                        state.prop = line_prop(*state.it);
                        state.next_prop = line_prop_t::AL;
                        if (std::next(state.it) != last)
                            state.next_prop = line_prop(*std::next(state.it));
                    }

                    continue;
                }

                // LB14
                {
                    auto const new_state = skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::OP;
                        },
                        [](line_prop_t prop) { return true; });
                    if (new_state.it != state.it) {
                        state = prev(new_state);
                        continue;
                    }
                }

                // LB15
                {
                    auto const new_state = skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::QU;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::OP;
                        });
                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB16 is handled as part of LB13.
                {
                    auto const new_state = skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::CL ||
                                   prop == line_prop_t::CP;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::NS;
                        });

                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB17
                {
                    auto const new_state = skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        });
                    if (new_state.it == last)
                        return new_state.it;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                if (lb7_space)
                    continue;

                // LB24
                if (state.prev_prop == line_prop_t::NU && after_nu(*state.it)) {
                    auto it = find_if_not(state.it, last, after_nu);
                    state.it = --it;
                    state.prop = line_prop(*state.it);
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                    continue;
                }

                // LB21a
                if (state.prev_prev_prop == line_prop_t::HL &&
                    (state.prev_prop == line_prop_t::HY ||
                     state.prev_prop == line_prop_t::BA)) {
                    continue;
                }

                emoji_state_setter.release();
                if (state.emoji_state ==
                    line_break_emoji_state_t::first_emoji) {
                    if (state.prop == line_prop_t::RI) {
                        state.emoji_state = line_break_emoji_state_t::none;
                        continue;
                    } else {
                        state.emoji_state = line_break_emoji_state_t::none;
                    }
                } else if (state.prop == line_prop_t::RI) {
                    state.emoji_state = line_break_emoji_state_t::first_emoji;
                }

                if (table_line_break(state.prev_prop, state.prop))
                    return state.it;
            }

            return state.it;
        }
    }

    /** TODO */
    template<typename CPIter, typename Sentinel>
    CPIter next_hard_line_break(CPIter first, Sentinel last) noexcept
    {
        return detail::next_line_break_impl(first, last, true);
    }

    /** TODO */
    template<typename CPIter, typename Sentinel>
    CPIter next_possible_line_break(CPIter first, Sentinel last) noexcept
    {
        return detail::next_line_break_impl(first, last, false);
    }

    /** TODO */
    template<typename CPIter, typename Sentinel>
    CPIter prev_hard_line_break(CPIter first, CPIter it, Sentinel last) noexcept
    {
        return detail::prev_line_break_impl(first, it, last, true);
    }

    /** TODO */
    template<typename CPIter, typename Sentinel>
    CPIter
    prev_possible_line_break(CPIter first, CPIter it, Sentinel last) noexcept
    {
        return detail::prev_line_break_impl(first, it, last, false);
    }

    /** Finds the nearest hard line break at or before before <code>it</code>.
        If <code>it == range.begin()</code>, that is returned.  Otherwise, the
        first code point of the line that <code>it</code> is within is
        returned (even if <code>it</code> is already at the first code point
        of a line. */
    template<typename CPRange, typename CPIter>
    inline auto prev_hard_line_break(CPRange & range, CPIter it) noexcept
        -> detail::iterator_t<CPRange>
    {
        return prev_hard_line_break(range.begin(), it, range.end());
    }

    /** Finds the next hard line break after <code>it</code>.  This will be
        the first code point after the current line, or
        <code>range.end()</code> if no next line exists.

        \pre <code>it</code> is at the beginning of a line. */
    template<typename CPRange>
    inline auto next_hard_line_break(CPRange & range) noexcept
        -> detail::iterator_t<CPRange>
    {
        return next_hard_line_break(range.begin(), range.end());
    }

    /** Finds the nearest possible line break at or before before
        <code>it</code>.  If <code>it == range.begin()</code>, that is
        returned.  Otherwise, the first code point of the line that
        <code>it</code> is within is returned (even if <code>it</code> is
        already at the first code point of a line. */
    template<typename CPRange, typename CPIter>
    inline auto prev_possible_line_break(CPRange & range, CPIter it) noexcept
        -> detail::iterator_t<CPRange>
    {
        return prev_possible_line_break(range.begin(), it, range.end());
    }

    /** Finds the next possible line break after <code>it</code>.  This will
        be the first code point after the current line, or
        <code>range.end()</code> if no next line exists.

        \pre <code>it</code> is at the beginning of a line. */
    template<typename CPRange>
    inline auto next_possible_line_break(CPRange & range) noexcept
        -> detail::iterator_t<CPRange>
    {
        return next_possible_line_break(range.begin(), range.end());
    }

    namespace detail {
        template<typename CPIter, typename Sentinel>
        struct next_hard_line_break_callable
        {
            CPIter operator()(CPIter it, Sentinel last) noexcept
            {
                return next_hard_line_break(it, last);
            }
        };
        template<typename CPIter, typename Sentinel>
        struct next_possible_line_break_callable
        {
            CPIter operator()(CPIter it, Sentinel last) noexcept
            {
                return next_possible_line_break(it, last);
            }
        };
    }

    /** Returns the bounds of the line (using hard line breaks) that
        <code>it</code> lies within. */
    template<typename CPIter, typename Sentinel>
    cp_range<CPIter> line(CPIter first, CPIter it, CPIter last) noexcept
    {
        first = prev_hard_line_break(first, it, last);
        return cp_range<CPIter>{first, next_hard_line_break(first, last)};
    }

    /** Returns the bounds of the line that <code>it</code> lies within. */
    template<typename CPRange, typename CPIter>
    inline auto line(CPRange & range, CPIter it) noexcept
        -> cp_range<detail::iterator_t<CPRange>>
    {
        auto first = prev_line_break(range.begin(), it, range.end());
        return cp_range<CPIter>{first, next_line_break(first, range.end())};
    }

    /** Returns a lazy range of the code point ranges delimiting lines (using
        hard line breaks) in <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_hard_line_break_callable<CPIter, Sentinel>>
    lines(CPIter first, CPIter last) noexcept
    {
        return {{first, last}, {last}};
    }

    /** Returns a lazy range of the code point ranges delimiting lines in
        <code>range</code>. */
    template<typename CPRange>
    auto lines(CPRange & range) noexcept -> lazy_segment_range<
        detail::iterator_t<CPRange>,
        detail::sentinel_t<CPRange>,
        detail::next_hard_line_break_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>>>
    {
        return {{range.begin(), range.end()}, {range.end()}};
    }

    /** Returns the bounds of the smallest chunk of text that could be broken
        off into a line, searching from <code>it</code> in either
        direction. */
    template<typename CPIter, typename Sentinel>
    cp_range<CPIter>
    possible_line(CPIter first, CPIter it, Sentinel last) noexcept
    {
        first = prev_possible_line_break(first, it, last);
        return cp_range<CPIter>{first, next_possible_line_break(first, last)};
    }

    /** Returns the bounds of the possible line that <code>it</code> lies
        within. */
    template<typename CPRange, typename CPIter>
    inline auto possible_line(CPRange & range, CPIter it) noexcept
        -> cp_range<detail::iterator_t<CPRange>>
    {
        auto first = prev_line_break(range.begin(), it, range.end());
        return cp_range<CPIter>{first, next_line_break(first, range.end())};
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_possible_line_break_callable<CPIter, Sentinel>>
    possible_lines(CPIter first, Sentinel last) noexcept
    {
        return {{first, last}, {last}};
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>range</code>. */
    template<typename CPRange>
    auto possible_lines(CPRange & range) noexcept -> lazy_segment_range<
        detail::iterator_t<CPRange>,
        detail::sentinel_t<CPRange>,
        detail::next_possible_line_break_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>>>
    {
        return {{range.begin(), range.end()}, {range.end()}};
    }

}}

#endif
