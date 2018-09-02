#ifndef BOOST_TEXT_LINE_BREAK_HPP
#define BOOST_TEXT_LINE_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/grapheme_range.hpp>
#include <boost/text/lazy_segment_range.hpp>

#include <boost/assert.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <array>
#include <numeric>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text {

    /** The line properties outlined in Unicode 11. */
    enum class line_property {
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

    namespace detail {
        struct line_prop_interval
        {
            uint32_t lo_;
            uint32_t hi_;
            line_property prop_;
        };

        inline bool
        operator<(line_prop_interval lhs, line_prop_interval rhs) noexcept
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<line_prop_interval, 49> const &
        make_line_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, line_property>
        make_line_prop_map();
    }

    /** Returns the line property associated with code point \a cp. */
    inline line_property line_prop(uint32_t cp) noexcept
    {
        static auto const map = detail::make_line_prop_map();
        static auto const intervals = detail::make_line_prop_intervals();

        auto const it = map.find(cp);
        if (it == map.end()) {
            auto const it2 = std::lower_bound(
                intervals.begin(),
                intervals.end(),
                detail::line_prop_interval{cp, cp + 1});
            if (it2 == intervals.end() || cp < it2->lo_ || it2->hi_ <= cp)
                return line_property::AL; // AL in place of XX, due to Rule LB1
            return it2->prop_;
        }
        return it->second;
    }

    /** The result type for line break algorithms that return an iterator, and
        which may return an iterator to either a hard or possible line break.
        A hard break occurs at the beginning and end of text, and after a code
        point with the line break property BK, CR, LF, or NL (but within a
        CR/LF pair). */
    template<typename CPIter>
    struct line_break_result
    {
        CPIter iter;
        bool hard_break;
    };

    template<typename CPIter, typename Sentinel>
    auto operator==(line_break_result<CPIter> result, Sentinel s) noexcept
        -> decltype(result.iter == s)
    {
        return result.iter == s;
    }

    template<typename CPIter, typename Sentinel>
    auto operator==(Sentinel s, line_break_result<CPIter> result) noexcept
        -> decltype(result.iter == s)
    {
        return result.iter == s;
    }

    template<typename CPIter, typename Sentinel>
    auto operator!=(line_break_result<CPIter> result, Sentinel s) noexcept
        -> decltype(result.iter != s)
    {
        return result.iter != s;
    }

    template<typename CPIter, typename Sentinel>
    auto operator!=(Sentinel s, line_break_result<CPIter> result) noexcept
        -> decltype(result.iter != s)
    {
        return result.iter != s;
    }

    namespace detail {
        // Note that whereas the other kinds of breaks have an 'Other', line
        // break has 'XX'.  However, due to Rule LB1, XX is replaced with AL,
        // so you'll see a lot of initializations from AL in this file.

        inline bool skippable(line_property prop) noexcept
        {
            return prop == line_property::CM || prop == line_property::ZWJ;
        }

        // Can represent the "X" in "X(CM|ZWJ)* -> X" in the LB9 rule.
        inline bool lb9_x(line_property prop) noexcept
        {
            return prop != line_property::BK && prop != line_property::CR &&
                   prop != line_property::LF && prop != line_property::NL &&
                   prop != line_property::SP && prop != line_property::ZW;
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

            line_property prev_prev_prop;
            line_property prev_prop;
            line_property prop;
            line_property next_prop;

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

        inline bool
        table_line_break(line_property lhs, line_property rhs) noexcept
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
                    state.it = temp_it;
                } else {
                    auto const temp_prop = line_prop(*temp_it);
                    state.it = temp_it;
                    state.prop = temp_prop;
                    state.next_prop = line_property::AL;
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
                    return line_prop(cp) == line_property::SP;
                });
                if (it == last)
                    return state;
                auto const temp_prop = line_prop(*it);
                if (after(temp_prop)) {
                    state.it = it;
                    state.prop = temp_prop;
                    state.next_prop = line_property::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                }
            }
            return state;
        }

        template<typename CPIter, typename BeforeFunc, typename AfterFunc>
        line_break_state<CPIter> skip_backward_spaces_between(
            line_break_state<CPIter> state,
            CPIter first,
            BeforeFunc before,
            AfterFunc after)
        {
            if (after(state.prop)) {
                auto const it =
                    find_if_not_backward(first, state.it, [](uint32_t cp) {
                        auto const prop = line_prop(cp);
                        return skippable(prop) || prop == line_property::SP;
                    });
                if (it == state.it)
                    return state;
                auto const temp_prop = line_prop(*it);
                if (before(temp_prop)) {
                    state.it = it;
                    state.it_points_to_prev = true;
                    state.prev_prop = temp_prop;
                    state.prev_prev_prop = line_property::AL;
                    if (state.it != first)
                        state.prev_prev_prop = line_prop(*std::prev(state.it));
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
        line_break_result<CPIter> prev_line_break_impl(
            CPIter first,
            CPIter it,
            Sentinel last,
            bool hard_breaks_only) noexcept
        {
            using result_t = line_break_result<CPIter>;

            if (it == first)
                return result_t{it, true};

            if (it == last && --it == first)
                return result_t{it, true};

            detail::line_break_state<CPIter> state;

            state.it = it;

            state.prop = line_prop(*state.it);

            // Special case: If state.prop is skippable, we need to skip
            // backward until we find a non-skippable, and if we're in one of
            // the space-skipping rules (LB14-LB17), back up to the start of
            // it.
            if (state.prop == line_property::SP ||
                detail::skippable(state.prop)) {
                auto space_or_skip = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_property::SP || detail::skippable(prop);
                };
                auto it_ = find_if_not_backward(first, it, space_or_skip);
                bool in_space_skipper = false;
                bool backed_up = false;
                if (it_ != it) {
                    auto const prop = line_prop(*it_);
                    switch (prop) {
                    case line_property::OP: // LB14
                        in_space_skipper = true;
                        break;
                    case line_property::QU: { // LB15
                        auto it_2 = find_if_not(it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last &&
                            line_prop(*it_2) == line_property::OP;
                        break;
                    }
                    case line_property::CL: // LB16
                    case line_property::CP: {
                        auto it_2 = find_if_not(it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last &&
                            line_prop(*it_2) == line_property::NS;
                        break;
                    }
                    case line_property::B2: { // LB17
                        auto it_2 = find_if_not(it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last &&
                            line_prop(*it_2) == line_property::B2;
                        break;
                    }
                    default: break;
                    }

                    backed_up = in_space_skipper;
                    if (!in_space_skipper && detail::skippable(state.prop)) {
                        it_ = find_if_not_backward(first, it, [](uint32_t cp) {
                            return detail::skippable(line_prop(cp));
                        });
                        backed_up = it_ != it;
                    }

                    if (backed_up) {
                        state.it = it_;
                        state.prop = line_prop(*state.it);
                        state.next_prop = line_prop(*std::next(state.it));
                    }
                }

                // If we end up on a non-skippable that should break before the
                // skippable(s) we just moved over, break on the last skippable.
                if (backed_up && !in_space_skipper &&
                    !detail::skippable(state.prop) &&
                    detail::table_line_break(state.prop, state.next_prop)) {
                    auto const hard = state.prop == line_property::BK ||
                                      state.prop == line_property::CR ||
                                      state.prop == line_property::LF ||
                                      state.prop == line_property::NL;
                    return result_t{++state.it, hard};
                }

                if (state.it == first)
                    return result_t{first, true};
            }

            state.prev_prev_prop = line_property::AL;
            if (std::prev(state.it) != first)
                state.prev_prev_prop = line_prop(*std::prev(state.it, 2));
            state.prev_prop = line_prop(*std::prev(state.it));
            state.next_prop = line_property::AL;
            if (std::next(state.it) != last)
                state.next_prop = line_prop(*std::next(state.it));

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
                    // Don't skip over the skippables id they are immediately
                    // preceded by a breaking character.
                    if (temp_prev_prop != line_property::BK &&
                        temp_prev_prop != line_property::CR &&
                        temp_prev_prop != line_property::LF &&
                        temp_prev_prop != line_property::NL &&
                        temp_prev_prop != line_property::ZW &&
                        temp_prev_prop != line_property::SP) {
                        state.it = temp_it;
                        state.it_points_to_prev = true;
                        state.prev_prop = temp_prev_prop;
                        if (temp_it == first) {
                            state.prev_prev_prop = line_property::AL;
                        } else {
                            state.prev_prev_prop =
                                line_prop(*std::prev(temp_it));
                        }
                    }
                }
                return state;
            };

            for (; state.it != first; state = prev(state)) {
                if (std::prev(state.it) != first)
                    state.prev_prev_prop = line_prop(*std::prev(state.it, 2));
                else
                    state.prev_prev_prop = line_property::AL;

                // LB1 (These should have been handled in data generation.)
                BOOST_ASSERT(state.prev_prop != line_property::AI);
                BOOST_ASSERT(state.prop != line_property::AI);
                BOOST_ASSERT(state.prev_prop != line_property::XX);
                BOOST_ASSERT(state.prop != line_property::XX);
                BOOST_ASSERT(state.prev_prop != line_property::SA);
                BOOST_ASSERT(state.prop != line_property::SA);
                BOOST_ASSERT(state.prev_prop != line_property::CJ);
                BOOST_ASSERT(state.prop != line_property::CJ);

                // When we see an RI, back up to the first RI so we can see what
                // emoji state we're supposed to be in here.
                if (state.emoji_state ==
                        detail::line_break_emoji_state_t::none &&
                    state.prop == line_property::RI) {
                    auto temp_state = state;
                    int ris_before = 0;
                    while (temp_state.it != first) {
                        temp_state = skip(temp_state, first);
                        if (temp_state.it == first) {
                            if (temp_state.prev_prop == line_property::RI) {
                                ++ris_before;
                            }
                            break;
                        }
                        if (temp_state.prev_prop == line_property::RI) {
                            temp_state = prev(temp_state);
                            if (temp_state.it != first &&
                                std::prev(temp_state.it) != first) {
                                temp_state.prev_prev_prop =
                                    line_prop(*std::prev(temp_state.it, 2));
                            } else {
                                temp_state.prev_prev_prop = line_property::AL;
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
                if (state.prev_prop == line_property::BK)
                    return result_t{state.it, true};

                // LB5
                if (state.prev_prop == line_property::CR &&
                    state.prop == line_property::LF) {
                    continue;
                }
                if (state.prev_prop == line_property::CR ||
                    state.prev_prop == line_property::LF ||
                    state.prev_prop == line_property::NL) {
                    return result_t{state.it, true};
                }

                if (hard_breaks_only)
                    continue;

                // LB6
                auto lb6 = [](line_property prop) {
                    return prop == line_property::BK ||
                           prop == line_property::CR ||
                           prop == line_property::LF ||
                           prop == line_property::NL;
                };
                if (lb6(state.prop))
                    continue;

                // LB7
                if (state.prop == line_property::ZW)
                    continue;

                // LB8
                if (state.prev_prop == line_property::ZW &&
                    state.prop != line_property::SP) {
                    return result_t{state.it, false};
                }
                if (state.prev_prop == line_property::SP &&
                    state.prop != line_property::SP) {
                    auto it =
                        find_if_not_backward(first, state.it, [](uint32_t cp) {
                            return line_prop(cp) == line_property::SP;
                        });
                    if (it != state.it && line_prop(*it) == line_property::ZW)
                        return result_t{state.it, false};
                }

                // LB8a
                if (state.prev_prop == line_property::ZWJ)
                    continue;

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
                    return result_t{state.it, true};

                // LB10
                // Inexplicably, implementing this (as required in TR14)
                // breaks a bunch of tests.

                // LB11
                if (state.prop == line_property::WJ ||
                    state.prev_prop == line_property::WJ)
                    continue;

                // LB12
                if (state.prev_prop == line_property::GL)
                    continue;

                // LB12a
                if ((state.prev_prop != line_property::SP &&
                     state.prev_prop != line_property::BA &&
                     state.prev_prop != line_property::HY) &&
                    state.prop == line_property::GL) {
                    continue;
                }

                // Used in LB24.
                auto after_nu = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_property::NU ||
                           prop == line_property::SY ||
                           prop == line_property::IS;
                };

                // LB13
                if (state.prop == line_property::CL ||
                    state.prop == line_property::CP) {
                    continue;
                }
                if (state.prop == line_property::EX ||
                    state.prop == line_property::IS ||
                    state.prop == line_property::SY) {
                    continue;
                }

                // LB14
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        first,
                        [](line_property prop) {
                            return prop == line_property::OP;
                        },
                        [](line_property prop) { return true; });
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB15
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        first,
                        [](line_property prop) {
                            return prop == line_property::QU;
                        },
                        [](line_property prop) {
                            return prop == line_property::OP;
                        });
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB16
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        first,
                        [](line_property prop) {
                            return prop == line_property::CL ||
                                   prop == line_property::CP;
                        },
                        [](line_property prop) {
                            return prop == line_property::NS;
                        });
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB17
                {
                    auto const new_state = skip_backward_spaces_between(
                        state,
                        first,
                        [](line_property prop) {
                            return prop == line_property::B2;
                        },
                        [](line_property prop) {
                            return prop == line_property::B2;
                        });
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB24
                if (after_nu(*state.it)) {
                    auto it = find_if_not_backward(first, state.it, after_nu);
                    if (it != state.it)
                        ++it;
                    if (it != state.it) {
                        if (line_prop(*it) == line_property::NU) {
                            state.it = it;
                            state.prop = line_prop(*state.it);
                            state.prev_prop = line_property::AL;
                            state.prev_prev_prop = line_property::AL;
                            if (state.it != first) {
                                state.prev_prop =
                                    line_prop(*std::prev(state.it));
                                if (std::prev(state.it) != first) {
                                    state.prev_prev_prop =
                                        line_prop(*std::prev(state.it, 2));
                                }
                            }

                            if (table_line_break(state.prev_prop, state.prop))
                                return result_t{state.it, false};

                            continue;
                        }
                    }
                }

                // LB21a
                if (state.prev_prev_prop == line_property::HL &&
                    (state.prev_prop == line_property::HY ||
                     state.prev_prop == line_property::BA)) {
                    continue;
                }

                if (state.emoji_state ==
                    detail::line_break_emoji_state_t::first_emoji) {
                    if (state.prev_prop == line_property::RI) {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::second_emoji;
                        return result_t{after_skip_it, false};
                    } else {
                        state.emoji_state =
                            detail::line_break_emoji_state_t::none;
                    }
                } else if (
                    state.emoji_state ==
                        detail::line_break_emoji_state_t::second_emoji &&
                    state.prev_prop == line_property::RI) {
                    state.emoji_state =
                        detail::line_break_emoji_state_t::first_emoji;
                    continue;
                }

                if (detail::table_line_break(state.prev_prop, state.prop))
                    return result_t{after_skip_it, false};
            }

            return result_t{first, true};
        }

        template<typename CPIter, typename Extent>
        struct no_op_cp_extent
        {
            Extent operator()(CPIter first, CPIter last) { return Extent(); }
        };

        template<typename Iter, typename T, typename Eval>
        Iter prefix_lower_bound(Iter first, Iter last, T x, Eval eval)
        {
            auto n = std::distance(first, last);
            Iter it = first;
            while (0 < n) {
                auto const n_over_2 = n >> 1;
                auto const mid = std::next(it, n_over_2);
                if (eval(first, mid) <= x) {
                    it = mid;
                    n -= n_over_2;
                    if (n == 1)
                        break;
                } else {
                    n = n_over_2;
                }
            }
            return it;
        }

        template<
            typename CPIter,
            typename Sentinel,
            typename Extent,
            typename CPExtentFunc>
        line_break_result<CPIter> next_line_break_impl(
            CPIter first,
            Sentinel last,
            bool hard_breaks_only,
            Extent max_extent,
            CPExtentFunc & cp_extent,
            bool break_overlong_lines) noexcept
        {
            using result_t = line_break_result<CPIter>;

            if (first == last)
                return result_t{first, true};

            line_break_state<CPIter> state;
            state.it = first;

            if (++state.it == last)
                return result_t{state.it, true};

            state.prev_prev_prop = line_property::AL;
            state.prev_prop = line_prop(*first);
            state.prop = line_prop(*state.it);
            state.next_prop = line_property::AL;
            if (std::next(state.it) != last)
                state.next_prop = line_prop(*std::next(state.it));

            state.emoji_state = state.prev_prop == line_property::RI
                                    ? line_break_emoji_state_t::first_emoji
                                    : line_break_emoji_state_t::none;

            optional<result_t> latest_result;
            Extent latest_extent = Extent{};

            auto break_overlong = [&cp_extent,
                                   &latest_result,
                                   &latest_extent,
                                   first,
                                   break_overlong_lines,
                                   max_extent](result_t result) {
                if (break_overlong_lines) {
                    CPIter const latest_extent_it =
                        latest_result ? latest_result->iter : first;
                    auto const extent =
                        cp_extent(latest_extent_it, result.iter);
                    auto const exceeds = max_extent < latest_extent + extent;
                    if (exceeds) {
                        if (latest_result) {
                            result = *latest_result;
                            return result;
                        }

                        Extent last_extent{};
                        auto it = prefix_lower_bound(
                            first,
                            result.iter,
                            max_extent,
                            [&cp_extent, &last_extent](CPIter f, CPIter l) {
                                return last_extent = cp_extent(f, l);
                            });

#if 0 // TODO: Necessary?
      // If it is in the middle of a grapheme, include
      // all same-extent CPs up to the end of the
      // current grapheme.
                            auto const range = grapheme(first, it, last);
                            if (range.begin() != it && range.end() != it) {
                                auto const grapheme_extent =
                                    cp_extent(first, range.end());
                                if (grapheme_extent == extent)
                                    it = range.end();
                            }
#endif

                        result.iter = it;
                    }
                }
                return result;
            };

            auto break_here = [&cp_extent,
                               &break_overlong,
                               &latest_result,
                               &latest_extent,
                               first,
                               max_extent](CPIter it) {
                auto const result = result_t{it, false};
                auto const extent =
                    cp_extent(latest_result ? latest_result->iter : first, it);
                auto const exceeds = max_extent < latest_extent + extent;
                if (exceeds && latest_result)
                    return true;
                auto const retval = (exceeds && !latest_result) ||
                                    latest_extent + extent == max_extent;
                latest_result = retval ? break_overlong(result) : result;
                latest_extent += extent;
                return retval;
            };

            for (; state.it != last; state = next(state)) {
                if (std::next(state.it) != last)
                    state.next_prop = line_prop(*std::next(state.it));
                else
                    state.next_prop = line_property::AL;

                scoped_emoji_state<CPIter> emoji_state_setter(state);

                // LB1 (These should have been handled in data generation.)
                BOOST_ASSERT(state.prev_prop != line_property::AI);
                BOOST_ASSERT(state.prop != line_property::AI);
                BOOST_ASSERT(state.prev_prop != line_property::XX);
                BOOST_ASSERT(state.prop != line_property::XX);
                BOOST_ASSERT(state.prev_prop != line_property::SA);
                BOOST_ASSERT(state.prop != line_property::SA);
                BOOST_ASSERT(state.prev_prop != line_property::CJ);
                BOOST_ASSERT(state.prop != line_property::CJ);

                // LB4
                if (state.prev_prop == line_property::BK)
                    return break_overlong(result_t{state.it, true});

                // LB5
                if (state.prev_prop == line_property::CR &&
                    state.prop == line_property::LF) {
                    continue;
                }
                if (state.prev_prop == line_property::CR ||
                    state.prev_prop == line_property::LF ||
                    state.prev_prop == line_property::NL) {
                    return break_overlong(result_t{state.it, true});
                }

                if (hard_breaks_only)
                    continue;

                // LB6
                auto lb6 = [](line_property prop) {
                    return prop == line_property::BK ||
                           prop == line_property::CR ||
                           prop == line_property::LF ||
                           prop == line_property::NL;
                };
                if (lb6(state.prop))
                    continue;

                // LB7
                // Even though a space means no break, we need to defer our
                // early return until after we've seen if the space will be
                // consumed in LB14-LB17 below.
                bool const lb7_space = state.prop == line_property::SP;
                if (state.prop == line_property::ZW)
                    continue;

                // LB8
                if (state.prev_prop == line_property::ZW && !lb7_space &&
                    break_here(state.it)) {
                    return *latest_result;
                }
                if (state.prev_prop == line_property::ZW &&
                    state.prop == line_property::SP) {
                    auto it = find_if_not(state.it, last, [](uint32_t cp) {
                        return line_prop(cp) == line_property::SP;
                    });
                    if (it == last)
                        return break_overlong(result_t{it, true});
                    auto const prop = line_prop(*it);
                    if (!lb6(prop) && prop != line_property::ZW &&
                        break_here(it)) {
                        return *latest_result;
                    }
                }

                // LB8a
                if (state.prev_prop == line_property::ZWJ &&
                    (state.prop == line_property::ID ||
                     state.prop == line_property::EB ||
                     state.prop == line_property::EM)) {
                    continue;
                }

                // LB9
                // Puting this here means not having to do it explicitly
                // below between prop and next_prop (and transitively,
                // between prev_prop and prop).
                state = skip_forward(state, first, last);
                if (state.it == last)
                    return break_overlong(result_t{state.it, true});

                // LB10
                // Inexplicably, implementing this (as required in TR14)
                // breaks a bunch of tests.

                // LB11
                if (state.prop == line_property::WJ ||
                    state.prev_prop == line_property::WJ)
                    continue;

                // LB12
                if (state.prev_prop == line_property::GL)
                    continue;

                // LB12a
                if ((state.prev_prop != line_property::SP &&
                     state.prev_prop != line_property::BA &&
                     state.prev_prop != line_property::HY) &&
                    state.prop == line_property::GL) {
                    continue;
                }

                // Used in LB24.
                auto after_nu = [](uint32_t cp) {
                    auto const prop = line_prop(cp);
                    return prop == line_property::NU ||
                           prop == line_property::SY ||
                           prop == line_property::IS;
                };

                // LB13
                if (state.prop == line_property::CL ||
                    state.prop == line_property::CP) {
                    // We know from this rule alone that there's no break
                    // here, but we also need to look ahead at whether LB16
                    // applies, since if we didn't, we'd bail out before
                    // ever reaching it due to LB12a above on the next
                    // iteration.
                    if (std::next(state.it) != last) {
                        // LB16
                        auto next_state = next(state);
                        if (std::next(next_state.it) != last) {
                            next_state.next_prop =
                                line_prop(*std::next(next_state.it));
                        } else {
                            next_state.next_prop = line_property::AL;
                        }

                        auto const new_state = skip_forward_spaces_between(
                            next_state,
                            last,
                            [](line_property prop) {
                                return prop == line_property::CL ||
                                       prop == line_property::CP;
                            },
                            [](line_property prop) {
                                return prop == line_property::NS;
                            });

                        if (new_state.it == last)
                            return break_overlong(result_t{new_state.it, true});
                        if (new_state.it != next_state.it)
                            state = new_state;
                    }
                    continue;
                }
                if (state.prop == line_property::EX ||
                    state.prop == line_property::IS ||
                    state.prop == line_property::SY) {
                    // As above, we need to check for the pattern
                    // NU(NU|SY|IS)* from LB24, even though without it we
                    // will still break here.

                    if (state.prev_prop == line_property::NU &&
                        after_nu(*state.it)) {
                        auto it = find_if_not(state.it, last, after_nu);
                        state.it = --it;
                        state.prop = line_prop(*state.it);
                        state.next_prop = line_property::AL;
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
                        [](line_property prop) {
                            return prop == line_property::OP;
                        },
                        [](line_property prop) { return true; });
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
                        [](line_property prop) {
                            return prop == line_property::QU;
                        },
                        [](line_property prop) {
                            return prop == line_property::OP;
                        });
                    if (new_state.it == last)
                        return break_overlong(result_t{new_state.it, true});
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
                        [](line_property prop) {
                            return prop == line_property::CL ||
                                   prop == line_property::CP;
                        },
                        [](line_property prop) {
                            return prop == line_property::NS;
                        });

                    if (new_state.it == last)
                        return break_overlong(result_t{new_state.it, true});
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
                        [](line_property prop) {
                            return prop == line_property::B2;
                        },
                        [](line_property prop) {
                            return prop == line_property::B2;
                        });
                    if (new_state.it == last)
                        return break_overlong(result_t{new_state.it, true});
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                if (lb7_space)
                    continue;

                // LB24
                if (state.prev_prop == line_property::NU &&
                    after_nu(*state.it)) {
                    auto it = find_if_not(state.it, last, after_nu);
                    state.it = --it;
                    state.prop = line_prop(*state.it);
                    state.next_prop = line_property::AL;
                    if (std::next(state.it) != last)
                        state.next_prop = line_prop(*std::next(state.it));
                    continue;
                }

                // LB21a
                if (state.prev_prev_prop == line_property::HL &&
                    (state.prev_prop == line_property::HY ||
                     state.prev_prop == line_property::BA)) {
                    continue;
                }

                emoji_state_setter.release();
                if (state.emoji_state ==
                    line_break_emoji_state_t::first_emoji) {
                    if (state.prop == line_property::RI) {
                        state.emoji_state = line_break_emoji_state_t::none;
                        continue;
                    } else {
                        state.emoji_state = line_break_emoji_state_t::none;
                    }
                } else if (state.prop == line_property::RI) {
                    state.emoji_state = line_break_emoji_state_t::first_emoji;
                }

                if (table_line_break(state.prev_prop, state.prop) &&
                    break_here(state.it)) {
                    return *latest_result;
                }
            }

            return break_overlong(result_t{state.it, false});
        }
    }

    /** Finds the nearest hard line break at or before before
        <code>it</code>. If <code>it == first</code>, that is returned.
        Otherwise, the first code point of the line that <code>it</code> is
        within is returned (even if <code>it</code> is already at the first
        code point of a line).  A hard line break follows any code points with
        the property BK, CR (not followed by LF), LF, or NL. */
    template<typename CPIter, typename Sentinel>
    auto prev_hard_line_break(CPIter first, CPIter it, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::prev_line_break_impl(first, it, last, true).iter;
    }

    /** Finds the nearest line break opportunity at or before before
        <code>it</code>.  If <code>it == first</code>, that is returned.
        Otherwise, the first code point of the line that <code>it</code> is
        within is returned (even if <code>it</code> is already at the first
        code point of a line). */
    template<typename CPIter, typename Sentinel>
    line_break_result<CPIter>
    prev_possible_line_break(CPIter first, CPIter it, Sentinel last) noexcept
    {
        return detail::prev_line_break_impl(first, it, last, false);
    }

    /** Finds the next hard line break after <code>first</code>.  This will be
        the first code point after the current line, or <code>last</code> if
        no next line exists.  A hard line break follows any code points with
        the property BK, CR (not followed by LF), LF, or NL.

        \pre <code>first</code> is at the beginning of a line. */
    template<typename CPIter, typename Sentinel>
    auto next_hard_line_break(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        detail::no_op_cp_extent<CPIter, int> no_op;
        return detail::next_line_break_impl(first, last, true, 0, no_op, false)
            .iter;
    }

    /** Finds the next line break opportunity after <code>first</code>. This
        will be the first code point after the current line, or
        <code>last</code> if no next line exists.

        \pre <code>first</code> is at the beginning of a line. */
    template<typename CPIter, typename Sentinel>
    auto next_possible_line_break(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<line_break_result<CPIter>, CPIter>
    {
        detail::no_op_cp_extent<CPIter, int> no_op;
        return detail::next_line_break_impl(
            first, last, false, 0, no_op, false);
    }

    /** Finds the nearest hard line break at or before before
        <code>it</code>. If <code>it == range.begin()</code>, that is
        returned.  Otherwise, the first code point of the line that
        <code>it</code> is within is returned (even if <code>it</code> is
        already at the first code point of a line).  A hard line break follows
        any code points with the property BK, CR (not followed by LF), LF, or
        NL. */
    template<typename CPRange, typename CPIter>
    auto prev_hard_line_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return prev_hard_line_break(std::begin(range), it, std::end(range));
    }

    /** Returns a grapheme_iterator to the nearest hard line break at or
        before before 'it'.  If it == range.begin(), that is returned.
        Otherwise, the first grapheme of the line that 'it' is within is
        returned (even if 'it' is already at the first grapheme of a line).  A
        hard line break follows any code points with the property BK, CR (not
        followed by LF), LF, or NL. */
    template<typename GraphemeRange, typename GraphemeIter>
    auto
    prev_hard_line_break(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                prev_hard_line_break(
                    range.begin().base(),
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base()),
                range.end().base()};
    }

    /** Finds the next hard line break after <code>it</code>.  This will be
        the first code point after the current line, or
        <code>range.end()</code> if no next line exists.  A hard line break
        follows any code points with the property BK, CR (not followed by LF),
        LF, or NL.

        \pre <code>it</code> is at the beginning of a line. */
    template<typename CPRange, typename CPIter>
    auto next_hard_line_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return next_hard_line_break(it, std::end(range));
    }

    /** Returns a grapheme_iterator to the next hard line break after 'it'.
        This will be the first grapheme after the current line, or range.end()
        if no next line exists.  A hard line break follows any code points
        with the property BK, CR (not followed by LF), LF, or NL.

        \pre 'it' is at the beginning of a line. */
    template<typename GraphemeRange, typename GraphemeIter>
    auto
    next_hard_line_break(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                next_hard_line_break(
                    static_cast<cp_iter_t>(it.base()), range.end().base()),
                range.end().base()};
    }

    /** Finds the nearest line break opportunity at or before before
        <code>it</code>.  If <code>it == range.begin()</code>, that is
        returned.  Otherwise, the first code point of the line that
        <code>it</code> is within is returned (even if <code>it</code> is
        already at the first code point of a line. */
    template<typename CPRange, typename CPIter>
    auto prev_possible_line_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<
            line_break_result<detail::iterator_t<CPRange>>,
            CPRange>
    {
        return prev_possible_line_break(std::begin(range), it, std::end(range));
    }

    /** Returns a grapheme_iterator to the nearest line break opportunity at
        or before before 'it'.  If it == range.begin(), that is returned.
        Otherwise, the first grapheme of the line that 'it' is within is
        returned (even if 'it' is already at the first grapheme of a line). */
    template<typename GraphemeRange, typename GraphemeIter>
    auto prev_possible_line_break(
        GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            line_break_result<detail::iterator_t<GraphemeRange const>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto const prev = prev_possible_line_break(
            range.begin().base(),
            static_cast<cp_iter_t>(it.base()),
            range.end().base());
        return {{range.begin().base(), prev.iter, range.end().base()},
                prev.hard_break};
    }

    /** Finds the next line break opportunity after <code>it</code>.  This
        will be the first code point after the current line, or
        <code>range.end()</code> if no next line exists.

        \pre <code>it</code> is at the beginning of a line. */
    template<typename CPRange, typename CPIter>
    auto next_possible_line_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<
            line_break_result<detail::iterator_t<CPRange>>,
            CPRange>
    {
        return next_possible_line_break(it, std::end(range));
    }

    /** Returns a grapheme_iterator to the next line break opportunity after
        'it'.  This will be the first grapheme after the current line, or
        range.end() if no next line exists.

        \pre 'it' is at the beginning of a line. */
    template<typename GraphemeRange, typename GraphemeIter>
    auto next_possible_line_break(
        GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            line_break_result<detail::iterator_t<GraphemeRange const>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto const next = next_possible_line_break(
            static_cast<cp_iter_t>(it.base()), range.end().base());
        return {{range.begin().base(), next.iter, range.end().base()},
                next.hard_break};
    }

    namespace detail {
        template<typename CPIter, typename Sentinel>
        struct next_hard_line_break_callable
        {
            auto operator()(CPIter it, Sentinel last) noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return next_hard_line_break(it, last);
            }
        };

        template<typename BreakResult, typename Sentinel>
        struct next_possible_line_break_callable
        {
            BreakResult operator()(BreakResult result, Sentinel last) noexcept
            {
                return next_possible_line_break(result.iter, last);
            }
        };

        template<typename Extent, typename CPExtentFunc>
        struct next_possible_line_break_within_extent_callable
        {
            next_possible_line_break_within_extent_callable() :
                extent_(),
                cp_extent_(),
                break_overlong_lines_(true)
            {}

            next_possible_line_break_within_extent_callable(
                Extent extent,
                CPExtentFunc cp_extent,
                bool break_overlong_lines) :
                extent_(extent),
                cp_extent_(std::move(cp_extent)),
                break_overlong_lines_(break_overlong_lines)
            {}

            template<typename BreakResult, typename Sentinel>
            BreakResult operator()(BreakResult result, Sentinel last) const
                noexcept
            {
                return detail::next_line_break_impl(
                    result.iter,
                    last,
                    false,
                    extent_,
                    *cp_extent_,
                    break_overlong_lines_);
            }

        private:
            Extent extent_;
            optional<CPExtentFunc> cp_extent_;
            bool break_overlong_lines_;
        };

        template<typename CPIter>
        struct prev_hard_line_break_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last) noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return prev_hard_line_break(first, it, last);
            }
        };

        template<typename CPIter, typename BreakResult>
        struct prev_possible_line_break_callable
        {
            BreakResult
            operator()(CPIter first, CPIter result, CPIter last) noexcept
            {
                return prev_possible_line_break(first, result, last);
            }
        };
    }

    /** Returns the bounds of the line (using hard line breaks) that
        <code>it</code> lies within. */
    template<typename CPIter, typename Sentinel>
    cp_range<CPIter> line(CPIter first, CPIter it, Sentinel last) noexcept
    {
        first = prev_hard_line_break(first, it, last);
        return cp_range<CPIter>{first, next_hard_line_break(first, last)};
    }

    /** Returns the bounds of the line (using hard line breaks) that
        <code>it</code> lies within. */
    template<typename CPRange, typename CPIter>
    auto line(CPRange & range, CPIter it) noexcept -> detail::
        cp_rng_alg_ret_t<cp_range<detail::iterator_t<CPRange>>, CPRange>
    {
        auto first =
            prev_hard_line_break(std::begin(range), it, std::end(range));
        return cp_range<CPIter>{first,
                                next_hard_line_break(first, std::end(range))};
    }

    /** Returns grapheme range delimiting the bounds of the line (using hard
        line breaks) that 'it' lies within. */
    template<typename GraphemeRange, typename GraphemeIter>
    auto line(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            grapheme_range<decltype(range.begin().base())>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto first = prev_hard_line_break(
            range.begin().base(),
            static_cast<cp_iter_t>(it.base()),
            range.end().base());
        return {first, next_hard_line_break(first, range.end().base())};
    }

    /** Returns a lazy range of the code point ranges delimiting lines (using
        hard line breaks) in <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_hard_line_break_callable<CPIter, Sentinel>>
    lines(CPIter first, Sentinel last) noexcept
    {
        detail::next_hard_line_break_callable<CPIter, Sentinel> next;
        return {std::move(next), {first, last}, {last}};
    }

    /** Returns a lazy range of the code point ranges delimiting lines (using
        hard line breaks) in <code>range</code>. */
    template<typename CPRange>
    auto lines(CPRange & range) noexcept -> detail::cp_rng_alg_ret_t<
        lazy_segment_range<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>,
            detail::next_hard_line_break_callable<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>>>,
        CPRange>
    {
        detail::next_hard_line_break_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>>
            next;
        return {std::move(next),
                {std::begin(range), std::end(range)},
                {std::end(range)}};
    }

    /** Returns a lazy range of the grapheme ranges delimiting lines (using
        hard line breaks) in range. */
    template<typename GraphemeRange>
    auto lines(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::next_hard_line_break_callable<
                    decltype(range.begin().base()),
                    decltype(range.begin().base())>,
                grapheme_range<decltype(range.begin().base())>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::next_hard_line_break_callable<cp_iter_t, cp_iter_t> next;
        return {std::move(next),
                {range.begin().base(), range.end().base()},
                {range.end().base()}};
    }

    /** Returns a lazy range of the code point ranges delimiting lines (using
        hard line breaks) in <code>[first, last)</code>, in reverse. */
    template<typename CPIter>
    lazy_segment_range<
        CPIter,
        CPIter,
        detail::prev_hard_line_break_callable<CPIter>,
        cp_range<CPIter>,
        detail::const_reverse_lazy_segment_iterator,
        true>
    reversed_lines(CPIter first, CPIter last) noexcept
    {
        detail::prev_hard_line_break_callable<CPIter> prev;
        return {std::move(prev), {first, last, last}, {first, first, last}};
    }

    /** Returns a lazy range of the code point ranges delimiting lines (using
        hard line breaks) in <code>range</code>, in reverse. */
    template<typename CPRange>
    auto reversed_lines(CPRange & range) noexcept -> detail::cp_rng_alg_ret_t<
        lazy_segment_range<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>,
            detail::prev_hard_line_break_callable<detail::iterator_t<CPRange>>,
            cp_range<detail::iterator_t<CPRange>>,
            detail::const_reverse_lazy_segment_iterator,
            true>,
        CPRange>
    {
        detail::prev_hard_line_break_callable<detail::iterator_t<CPRange>> prev;
        return {std::move(prev),
                {std::begin(range), std::end(range), std::end(range)},
                {std::begin(range), std::begin(range), std::end(range)}};
    }

    /** Returns a lazy range of the grapheme ranges delimiting lines (using
        hard line breaks) in range, in reverse. */
    template<typename GraphemeRange>
    auto reversed_lines(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::prev_hard_line_break_callable<
                    decltype(range.begin().base())>,
                grapheme_range<decltype(range.begin().base())>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::prev_hard_line_break_callable<cp_iter_t> prev;
        return {
            std::move(prev),
            {range.begin().base(), range.end().base(), range.end().base()},
            {range.begin().base(), range.begin().base(), range.end().base()}};
    }

    // TODO: Document that hard_break is only set when we see a bang-break
    // from the rules (a rule with '!' in it).

    /** A range of code points elements that delimit a pair of line break
        boundaries. */
    template<typename CPIter, typename Sentinel = CPIter>
    struct line_break_cp_range : cp_range<CPIter, Sentinel>
    {
        line_break_cp_range() : cp_range<CPIter, Sentinel>(), hard_break_() {}
        line_break_cp_range(
            line_break_result<CPIter> first, line_break_result<CPIter> last) :
            cp_range<CPIter, Sentinel>(first.iter, last.iter),
            hard_break_(last.hard_break)
        {}

        /** Returns true iff the end of *this is a hard line break
            boundary. */
        bool hard_break() const noexcept { return hard_break_; }

    private:
        bool hard_break_;
    };

    /** A range of grapheme elements that delimit a pair of line break
        boundaries. */
    template<typename CPIter>
    struct line_break_grapheme_range : grapheme_range<CPIter>
    {
        line_break_grapheme_range() : grapheme_range<CPIter>(), hard_break_() {}
        line_break_grapheme_range(
            line_break_result<CPIter> first, line_break_result<CPIter> last) :
            grapheme_range<CPIter>(first.iter, last.iter),
            hard_break_(last.hard_break)
        {}
        template<typename GraphemeIter>
        line_break_grapheme_range(
            line_break_result<GraphemeIter> first,
            line_break_result<GraphemeIter> last) :
            grapheme_range<CPIter>(first.iter.base(), last.iter.base()),
            hard_break_(last.hard_break)
        {}

        /** Returns true iff the end of *this is a hard line break
            boundary. */
        bool hard_break() const noexcept { return hard_break_; }

    private:
        bool hard_break_;
    };

    /** Returns a lazy range of the code point ranges in <code>[first,
        last)</code> delimiting lines.  A line that does not end in a hard
        break will end in a possible break that does not exceed max_extent,
        using the code point extents derived from CPExtentFunc.  When a line
        has no possible breaks before it would exceed max_extent, if will be
        broken only if break_overlong_lines is true.  If break_overlong_lines
        is false, such an unbreakable line will exceed max_extent. */
    template<
        typename CPIter,
        typename Sentinel,
        typename Extent,
        typename CPExtentFunc>
    lazy_segment_range<
        line_break_result<CPIter>,
        Sentinel,
        detail::next_possible_line_break_within_extent_callable<
            Extent,
            CPExtentFunc>,
        line_break_cp_range<CPIter>>
    lines(
        CPIter first,
        Sentinel last,
        Extent max_extent,
        CPExtentFunc cp_extent,
        bool break_overlong_lines = true) noexcept
    {
        detail::next_possible_line_break_within_extent_callable<
            Extent,
            CPExtentFunc>
            next{max_extent, std::move(cp_extent), break_overlong_lines};
        return {std::move(next),
                {line_break_result<CPIter>{first, true}, last},
                {last}};
    }

    /** Returns a lazy range of the code point ranges in range delimiting
        lines. A line that does not end in a hard break will end in a possible
        break that does not exceed max_extent, using the code point extents
        derived from CPExtentFunc.  When a line has no possible breaks before
        it would exceed max_extent, if will be broken only if
        break_overlong_lines is true.  If break_overlong_lines is false, such
        an unbreakable line will exceed max_extent. */
    template<typename CPRange, typename Extent, typename CPExtentFunc>
    detail::cp_rng_alg_ret_t<
        lazy_segment_range<
            line_break_result<detail::iterator_t<CPRange>>,
            detail::sentinel_t<CPRange>,
            detail::next_possible_line_break_within_extent_callable<
                Extent,
                CPExtentFunc>,
            line_break_cp_range<detail::iterator_t<CPRange>>>,
        CPRange>
    lines(
        CPRange & range,
        Extent max_extent,
        CPExtentFunc cp_extent,
        bool break_overlong_lines = true) noexcept
    {
        detail::next_possible_line_break_within_extent_callable<
            Extent,
            CPExtentFunc>
            next{max_extent, std::move(cp_extent), break_overlong_lines};
        return {std::move(next),
                {line_break_result<detail::iterator_t<CPRange>>{
                     std::begin(range), true},
                 std::end(range)},
                {std::end(range)}};
    }

    /** Returns a lazy range of the grapheme ranges in range delimiting
        lines. A line that does not end in a hard break will end in a possible
        break that does not exceed max_extent, using the code point extents
        derived from CPExtentFunc.  When a line has no possible breaks before
        it would exceed max_extent, if will be broken only if
        break_overlong_lines is true.  If break_overlong_lines is false, such
        an unbreakable line will exceed max_extent. */
    template<typename GraphemeRange, typename Extent, typename CPExtentFunc>
    auto lines(
        GraphemeRange const & range,
        Extent max_extent,
        CPExtentFunc cp_extent,
        bool break_overlong_lines = true) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                line_break_result<decltype(range.begin().base())>,
                decltype(range.begin().base()),
                detail::next_possible_line_break_within_extent_callable<
                    Extent,
                    CPExtentFunc>,
                line_break_grapheme_range<decltype(range.begin().base())>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::next_possible_line_break_within_extent_callable<
            Extent,
            CPExtentFunc>
            next{max_extent, std::move(cp_extent), break_overlong_lines};
        return {std::move(next),
                {line_break_result<cp_iter_t>{range.begin().base(), true},
                 range.end().base()},
                {range.end().base()}};
    }

    /** Returns the bounds of the smallest chunk of text that could be broken
        off into a line, searching from <code>it</code> in either
        direction. */
    template<typename CPIter, typename Sentinel>
    line_break_cp_range<CPIter>
    possible_line(CPIter first, CPIter it, Sentinel last) noexcept
    {
        auto const first_ = prev_possible_line_break(first, it, last);
        return line_break_cp_range<CPIter>{
            first_, next_possible_line_break(first_.iter, last)};
    }

    /** Returns the bounds of the smallest chunk of text that could be
       broken off into a line, searching from <code>it</code> in either
        direction. */
    template<typename CPRange, typename CPIter>
    auto possible_line(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<
            line_break_cp_range<detail::iterator_t<CPRange>>,
            CPRange>
    {
        auto const first =
            prev_possible_line_break(std::begin(range), it, std::end(range));
        return line_break_cp_range<CPIter>{
            first, next_possible_line_break(first.iter, std::end(range))};
    }

    /** Returns grapheme range delimiting the bounds of the line (using hard
        line breaks) that 'it' lies within. */
    template<typename GraphemeRange, typename GraphemeIter>
    auto possible_line(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            line_break_grapheme_range<decltype(range.begin().base())>,
            GraphemeRange>
    {
        auto const first = prev_possible_line_break(range, it);
        return {first, next_possible_line_break(range, first.iter)};
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        line_break_result<CPIter>,
        Sentinel,
        detail::next_possible_line_break_callable<
            line_break_result<CPIter>,
            Sentinel>,
        line_break_cp_range<CPIter>>
    possible_lines(CPIter first, Sentinel last) noexcept
    {
        detail::next_possible_line_break_callable<
            line_break_result<CPIter>,
            Sentinel>
            next;
        return {std::move(next),
                {line_break_result<CPIter>{first, true}, last},
                {last}};
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>range</code>. */
    template<typename CPRange>
    auto possible_lines(CPRange & range) noexcept -> detail::cp_rng_alg_ret_t<
        lazy_segment_range<
            line_break_result<detail::iterator_t<CPRange>>,
            detail::sentinel_t<CPRange>,
            detail::next_possible_line_break_callable<
                line_break_result<detail::iterator_t<CPRange>>,
                detail::sentinel_t<CPRange>>,
            line_break_cp_range<detail::iterator_t<CPRange>>>,
        CPRange>
    {
        detail::next_possible_line_break_callable<
            line_break_result<detail::iterator_t<CPRange>>,
            detail::sentinel_t<CPRange>>
            next;
        return {std::move(next),
                {line_break_result<detail::iterator_t<CPRange>>{
                     std::begin(range), true},
                 std::end(range)},
                {std::end(range)}};
    }

    /** Returns a lazy range of the grapheme ranges delimiting possible lines
        in <code>range</code>. */
    template<typename GraphemeRange>
    auto possible_lines(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                line_break_result<decltype(range.begin().base())>,
                decltype(range.begin().base()),
                detail::next_possible_line_break_callable<
                    line_break_result<decltype(range.begin().base())>,
                    decltype(range.begin().base())>,
                line_break_grapheme_range<decltype(range.begin().base())>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::next_possible_line_break_callable<
            line_break_result<cp_iter_t>,
            cp_iter_t>
            next;
        return {std::move(next),
                {line_break_result<cp_iter_t>{range.begin().base(), true},
                 range.end().base()},
                {range.end().base()}};
    }

    namespace detail {
        template<
            typename CPIter,
            typename ResultType,
            typename PrevFunc,
            typename CPRange>
        struct const_reverse_possible_line_iterator
        {
        private:
            PrevFunc * prev_func_;
            CPIter first_;
            ResultType it_;
            ResultType next_;

        public:
            using value_type = CPRange;
            using pointer = detail::segment_arrow_proxy<CPIter, CPRange>;
            using reference = value_type;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            const_reverse_possible_line_iterator() noexcept :
                prev_func_(),
                first_(),
                it_(),
                next_()
            {}

            const_reverse_possible_line_iterator(
                CPIter first, ResultType it, ResultType last) noexcept :
                prev_func_(),
                first_(first),
                it_(it),
                next_(last)
            {}

            reference operator*() const noexcept
            {
                return value_type{it_, next_};
            }

            pointer operator->() const noexcept { return pointer(**this); }

            const_reverse_possible_line_iterator & operator++() noexcept
            {
                if (it_ == first_) {
                    next_.iter = first_;
                    return *this;
                }
                auto const prev_it =
                    (*prev_func_)(first_, std::prev(it_.iter), next_.iter);
                next_ = it_;
                it_ = prev_it;
                return *this;
            }

            void set_next_func(PrevFunc * prev_func) noexcept
            {
                prev_func_ = prev_func;
                ++*this;
            }

            friend bool operator==(
                const_reverse_possible_line_iterator lhs,
                const_reverse_possible_line_iterator rhs) noexcept
            {
                return lhs.next_ == rhs.first_;
            }
            friend bool operator!=(
                const_reverse_possible_line_iterator lhs,
                const_reverse_possible_line_iterator rhs) noexcept
            {
                return !(lhs == rhs);
            }
        };
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>[first, last)</code>, in reverse. */
    template<typename CPIter>
    lazy_segment_range<
        CPIter,
        line_break_result<CPIter>,
        detail::prev_possible_line_break_callable<
            CPIter,
            line_break_result<CPIter>>,
        line_break_cp_range<CPIter>,
        detail::const_reverse_possible_line_iterator,
        true>
    reversed_possible_lines(CPIter first, CPIter last) noexcept
    {
        detail::
            prev_possible_line_break_callable<CPIter, line_break_result<CPIter>>
                prev;
        auto const first_result = line_break_result<CPIter>{first, true};
        auto const last_result = line_break_result<CPIter>{last, true};
        return {std::move(prev),
                {first, last_result, last_result},
                {first, first_result, last_result}};
    }

    /** Returns a lazy range of the code point ranges delimiting possible
        lines in <code>range</code>, in reverse. */
    template<typename CPRange>
    auto reversed_possible_lines(CPRange & range) noexcept
        -> detail::cp_rng_alg_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                line_break_result<detail::iterator_t<CPRange>>,
                detail::prev_possible_line_break_callable<
                    detail::iterator_t<CPRange>,
                    line_break_result<detail::iterator_t<CPRange>>>,
                line_break_cp_range<detail::iterator_t<CPRange>>,
                detail::const_reverse_possible_line_iterator,
                true>,
            CPRange>
    {
        detail::prev_possible_line_break_callable<
            detail::iterator_t<CPRange>,
            line_break_result<detail::iterator_t<CPRange>>>
            prev;
        auto const begin = std::begin(range);
        auto const begin_result =
            line_break_result<detail::iterator_t<CPRange>>{begin, true};
        auto const end_result = line_break_result<detail::iterator_t<CPRange>>{
            std::end(range), true};
        return {std::move(prev),
                {begin, end_result, end_result},
                {begin, begin_result, end_result}};
    }

    /** Returns a lazy range of the grapheme ranges delimiting possible lines
        in <code>range</code>, in reverse. */
    template<typename GraphemeRange>
    auto reversed_possible_lines(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                line_break_result<decltype(range.begin().base())>,
                detail::prev_possible_line_break_callable<
                    decltype(range.begin().base()),
                    line_break_result<decltype(range.begin().base())>>,
                line_break_grapheme_range<decltype(range.begin().base())>,
                detail::const_reverse_possible_line_iterator,
                true>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::prev_possible_line_break_callable<
            cp_iter_t,
            line_break_result<cp_iter_t>>
            prev;
        auto const begin_result =
            line_break_result<cp_iter_t>{range.begin().base(), true};
        auto const end_result =
            line_break_result<cp_iter_t>{range.end().base(), true};
        return {std::move(prev),
                {range.begin().base(), end_result, end_result},
                {range.begin().base(), begin_result, end_result}};
    }

}}

#endif
