#ifndef BOOST_TEXT_LINE_BREAK_HPP
#define BOOST_TEXT_LINE_BREAK_HPP

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

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,  1, 1, 1, 0}}, // CL
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

    {{1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,  1, 1, 1, 0}}, // RI
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
        template<typename CPIter>
        line_break_state<CPIter>
        skip_forward(line_break_state<CPIter> state, CPIter first, CPIter last)
        {
            if (state.it != first && !skippable(state.prev_prop) &&
                lb9_x(state.prev_prop) && skippable(state.prop)) {
                // TODO: -> find_if (here and elsewhere!)
                auto temp_it = state.it;
                while (std::next(temp_it) != last) {
                    auto temp_next_prop = line_prop(*++temp_it);
                    if (!skippable(temp_next_prop))
                        break;
                }
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

        template<typename CPIter, typename FromFunc, typename ToFunc>
        line_break_state<CPIter> skip_forward_spaces_between(
            line_break_state<CPIter> state,
            CPIter last,
            FromFunc from,
            ToFunc to)
        {
            if (from(state.prev_prop) && state.prop == line_prop_t::SP) {
                auto const it =
                    std::find_if_not(state.it, last, [](uint32_t cp) {
                        return line_prop(cp) == line_prop_t::SP;
                    });
                if (it == last)
                    return state;
                auto const temp_prop = line_prop(*it);
                if (to(temp_prop)) {
                    state.it = it;
                    state.prop = temp_prop;
                    state.next_prop = line_prop_t::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                }
            }
            return state;
        }

        // TODO: Get rid of 'first' and initial '++it'.  Since the
        // precondition for calling this function is that 'it' is at SOT or a
        // line break, previous state can be ignored.  This also applies to
        // other break algorithms.
        template<typename CPIter>
        inline CPIter next_line_break_impl(
            CPIter first,
            CPIter it,
            CPIter last,
            bool hard_breaks_only) noexcept
        {
            if (it == last)
                return last;

            if (++it == last)
                return last;

            detail::line_break_state<CPIter> state;

            state.it = it;

            state.prev_prev_prop = line_prop_t::AL;
            state.prev_prop = line_prop_t::AL;
            if (state.it != first) {
                state.prev_prop = line_prop(*std::prev(state.it));
                if (std::prev(state.it) != first)
                    state.prev_prev_prop = line_prop(*std::prev(state.it, 2));
            }
            state.prop = line_prop(*state.it);
            state.next_prop = line_prop_t::AL;
            if (std::next(state.it) != last)
                state.next_prop = line_prop(*std::next(state.it));

            state.emoji_state =
                state.prev_prop == line_prop_t::RI
                    ? detail::line_break_emoji_state_t::first_emoji
                    : detail::line_break_emoji_state_t::none;

            for (; state.it != last; state = next(state)) {
                if (std::next(state.it) != last)
                    state.next_prop = line_prop(*std::next(state.it));
                else
                    state.next_prop = line_prop_t::AL;

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
                if (state.prop == line_prop_t::BK ||
                    state.prop == line_prop_t::CR ||
                    state.prop == line_prop_t::LF ||
                    state.prop == line_prop_t::NL) {
                    continue;
                }

                // LB7
                if (state.prop == line_prop_t::SP ||
                    state.prop == line_prop_t::ZW) {
                    continue;
                }

                // LB8, LB8a
                if (state.prev_prop == line_prop_t::ZW) {
                    if (state.prop == line_prop_t::ID ||
                        state.prop == line_prop_t::EB ||
                        state.prop == line_prop_t::EM) {
                        continue;
                    }
                    while (state.it != last && state.prop == line_prop_t::SP) {
                        state = next(state);
                        if (std::next(state.it) != last)
                            state.next_prop = line_prop(*std::next(state.it));
                        else
                            state.next_prop = line_prop_t::AL;
                    }
                    return state.it;
                }

                // LB9
                // Puting this here means not having to do it explicitly below
                // between prop and next_prop (and transitively, between
                // prev_prop and prop).
                state = detail::skip_forward(state, first, last);
                if (state.it == last)
                    return last;

                // LB10
                auto prop = state.prop;
                if (prop == line_prop_t::CM || prop == line_prop_t::ZWJ)
                    prop = line_prop_t::AL;

                // LB11
                if (prop == line_prop_t::WJ ||
                    state.prev_prop == line_prop_t::WJ)
                    continue;

                // LB12
                if (state.prev_prop == line_prop_t::GL)
                    continue;

                // LB12a
                if ((state.prev_prop != line_prop_t::SP &&
                     state.prev_prop != line_prop_t::BA &&
                     state.prev_prop != line_prop_t::HY) &&
                    prop == line_prop_t::GL) {
                    continue;
                }

                // Used in LB24.
                auto after_nu = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_prop_t::NU || prop == line_prop_t::SY ||
                           prop == line_prop_t::IS;
                };

                // LB13
                if (prop == line_prop_t::CL || prop == line_prop_t::CP) {
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

                        auto const new_state =
                            detail::skip_forward_spaces_between(
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
                            return last;
                        if (new_state.it != next_state.it)
                            state = new_state;
                    }
                    continue;
                }
                if (prop == line_prop_t::EX || prop == line_prop_t::IS ||
                    prop == line_prop_t::SY) {
                    // As above, we need to check for the pattern
                    // NU(NU|SY|IS)* from LB24, even though without it there
                    // will still be a break here.

                    if (state.prev_prop == line_prop_t::NU &&
                        after_nu(*state.it)) {
                        auto it = std::find_if_not(state.it, last, after_nu);
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
                    auto const new_state = detail::skip_forward_spaces_between(
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
                    auto const new_state = detail::skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::QU;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::OP;
                        });
                    if (new_state.it == last)
                        return last;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB16 is handled as part of LB13.

                // LB17
                {
                    auto const new_state = detail::skip_forward_spaces_between(
                        state,
                        last,
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        },
                        [](line_prop_t prop) {
                            return prop == line_prop_t::B2;
                        });
                    if (new_state.it == last)
                        return last;
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB24
                if (state.prev_prop == line_prop_t::NU && after_nu(*state.it)) {
                    auto it = std::find_if_not(state.it, last, after_nu);
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
                    if (prop == line_prop_t::RI) {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::none;
                        continue;
                    } else {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::none;
                    }
                } else if (prop == line_prop_t::RI) {
                    state.emoji_state =
                        detail::line_break_emoji_state_t::first_emoji;
                    return state.it;
                }

                if (detail::table_line_break(state.prev_prop, prop))
                    return state.it;
            }

            return last;
        }
    }

    /** TODO */
    template<typename CPIter>
    inline CPIter
    next_hard_line_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        return detail::next_line_break_impl(first, it, last, true);
    }

    /** TODO */
    template<typename CPIter>
    inline CPIter next_line_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        return detail::next_line_break_impl(first, it, last, false);
    }

}}

#endif
// b line_break_47.cpp:168
// b line_break.hpp:306
