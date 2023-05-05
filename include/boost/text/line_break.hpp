// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_LINE_BREAK_HPP
#define BOOST_TEXT_LINE_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/view_adaptor.hpp>
#include <boost/text/detail/breaks_impl.hpp>

#include <boost/assert.hpp>
#include <boost/optional.hpp>

#if defined(__GNUC__) && __GNUC__ < 5
#include <boost/type_traits/has_trivial_copy.hpp>
#endif

#include <algorithm>
#include <array>
#if !defined(__GNUC__) || 5 <= __GNUC__
#include <memory>
#endif
#include <numeric>
#include <type_traits>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text {

    /** The line properties defined by Unicode. */
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

        inline bool operator<(line_prop_interval lhs, line_prop_interval rhs)
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<line_prop_interval, 49> const &
        make_line_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, line_property>
        make_line_prop_map();
    }

    /** Returns the line property associated with code point `cp`. */
    inline line_property line_prop(uint32_t cp)
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
        which may return an iterator to either a hard (i.e. mandatory) or
        allowed line break.  A hard break occurs only after a code point with
        the line break property BK, CR, LF, or NL (but only once for a CR/LF
        pair). */
    template<typename CPIter>
    struct line_break_result
    {
        CPIter iter;
        bool hard_break;
    };

    template<typename CPIter, typename Sentinel>
    auto operator==(line_break_result<CPIter> result, Sentinel s)
        -> decltype(result.iter == s)
    {
        return result.iter == s;
    }

    template<typename CPIter>
    auto operator==(CPIter it, line_break_result<CPIter> result)
        -> decltype(it == result.iter)
    {
        return it == result.iter;
    }

    template<typename CPIter, typename Sentinel>
    auto operator!=(line_break_result<CPIter> result, Sentinel s)
        -> decltype(result.iter != s)
    {
        return result.iter != s;
    }

    template<typename CPIter, typename Sentinel>
    auto operator!=(CPIter it, line_break_result<CPIter> result)
        -> decltype(it != result.iter)
    {
        return it != result.iter;
    }

    /** A range of code points that delimit a pair of line break
        boundaries. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    struct line_break_cp_view : utf_view<format::utf32, I>
    {
        line_break_cp_view() : utf_view<format::utf32, I>(), hard_break_() {}
        line_break_cp_view(
            line_break_result<I> first, line_break_result<I> last) :
            utf_view<format::utf32, I>(first.iter, last.iter),
            hard_break_(last.hard_break)
        {}

        /** Returns true if the end of *this is a hard line break boundary. */
        bool hard_break() const { return hard_break_; }

    private:
        bool hard_break_;
    };

    /** A range of graphemes that delimit a pair of line break boundaries. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    struct line_break_grapheme_view : grapheme_view<I>
    {
        line_break_grapheme_view() : grapheme_view<I>(), hard_break_() {}
        line_break_grapheme_view(
            line_break_result<I> first, line_break_result<I> last) :
            grapheme_view<I>(first.iter, last.iter),
            hard_break_(last.hard_break)
        {}
        template<typename GraphemeIter>
        line_break_grapheme_view(
            line_break_result<GraphemeIter> first,
            line_break_result<GraphemeIter> last) :
            grapheme_view<I>(first.iter.base(), last.iter.base()),
            hard_break_(last.hard_break)
        {}

        /** Returns true if the end of *this is a hard line break boundary. */
        bool hard_break() const { return hard_break_; }

    private:
        bool hard_break_;
    };

    /** A tag type that can be passed to the `lines` view adaptor.  Pass the
        constant `boost::text::allowed_breaks` to get lines divided by allowed
        and hard line breaks, instead of only by hard line breaks. */
    struct allowed_breaks_t
    {};

#if defined(__cpp_inline_variables)
    inline constexpr allowed_breaks_t allowed_breaks;
#else
    namespace {
        constexpr allowed_breaks_t allowed_breaks;
    }
#endif

    namespace detail {
        // Note that whereas the other kinds of breaks have an 'Other', line
        // break has 'XX'.  However, due to Rule LB1, XX is replaced with AL,
        // so you'll see a lot of initializations from AL in this file.

        inline bool skippable(line_property prop)
        {
            return prop == line_property::CM || prop == line_property::ZWJ;
        }

        // Can represent the "X" in "X(CM|ZWJ)* -> X" in the LB9 rule.
        inline bool lb9_x(line_property prop)
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
        table_line_break(line_property lhs, line_property rhs)
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
            if (state.it != first && !detail::skippable(state.prev_prop) &&
                detail::lb9_x(state.prev_prop) &&
                detail::skippable(state.prop)) {
                auto temp_it = boost::text::find_if_not(
                    std::next(state.it), last, [](uint32_t cp) {
                        return detail::skippable(
                            boost::text::line_prop(cp));
                    });
                if (temp_it == last) {
                    state.it = temp_it;
                } else {
                    auto const temp_prop = boost::text::line_prop(*temp_it);
                    state.it = temp_it;
                    state.prop = temp_prop;
                    state.next_prop = line_property::AL;
                    if (std::next(state.it) != last) {
                        state.next_prop =
                            boost::text::line_prop(*std::next(state.it));
                    }
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
                auto const it = boost::text::find_if_not(
                    state.it, last, [](uint32_t cp) {
                        return boost::text::line_prop(cp) ==
                               line_property::SP;
                    });
                if (it == last)
                    return state;
                auto const temp_prop = boost::text::line_prop(*it);
                if (after(temp_prop)) {
                    state.it = it;
                    state.prop = temp_prop;
                    state.next_prop = line_property::AL;
                    if (std::next(state.it) != last) {
                        state.next_prop =
                            boost::text::line_prop(*std::next(state.it));
                    }
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
                auto const it = boost::text::find_if_not_backward(
                    first, state.it, [](uint32_t cp) {
                        auto const prop = boost::text::line_prop(cp);
                        return detail::skippable(prop) ||
                               prop == line_property::SP;
                    });
                if (it == state.it)
                    return state;
                auto const temp_prop = boost::text::line_prop(*it);
                if (before(temp_prop)) {
                    state.it = it;
                    state.it_points_to_prev = true;
                    state.prev_prop = temp_prop;
                    state.prev_prev_prop = line_property::AL;
                    if (state.it != first) {
                        state.prev_prev_prop =
                            boost::text::line_prop(*std::prev(state.it));
                    }
                }
            }
            return state;
        }

        inline bool hard_break_cp(uint32_t cp)
        {
            auto const prop = boost::text::line_prop(cp);
            return prop == line_property::BK || prop == line_property::CR ||
                   prop == line_property::LF || prop == line_property::NL;
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
            bool hard_breaks_only)
        {
            using result_t = line_break_result<CPIter>;

            if (it == first)
                return result_t{it, false};

            if (it == last && --it == first)
                return result_t{it, detail::hard_break_cp(*it)};

            detail::line_break_state<CPIter> state;

            state.it = it;

            state.prop = boost::text::line_prop(*state.it);

            // Special case: If state.prop is skippable, we need to skip
            // backward until we find a non-skippable, and if we're in one of
            // the space-skipping rules (LB14-LB17), back up to the start of
            // it.
            if (state.prop == line_property::SP ||
                detail::skippable(state.prop)) {
                auto space_or_skip = [](uint32_t cp) {
                    auto const prop = boost::text::line_prop(cp);
                    return prop == line_property::SP || detail::skippable(prop);
                };
                auto it_ = boost::text::find_if_not_backward(
                    first, it, space_or_skip);
                bool in_space_skipper = false;
                bool backed_up = false;
                if (it_ != it) {
                    auto const prop = boost::text::line_prop(*it_);
                    switch (prop) {
                    case line_property::OP: // LB14
                        in_space_skipper = true;
                        break;
                    case line_property::QU: { // LB15
                        auto it_2 = boost::text::find_if_not(
                            it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last && boost::text::line_prop(*it_2) ==
                                                line_property::OP;
                        break;
                    }
                    case line_property::CL: // LB16
                    case line_property::CP: {
                        auto it_2 = boost::text::find_if_not(
                            it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last && boost::text::line_prop(*it_2) ==
                                                line_property::NS;
                        break;
                    }
                    case line_property::B2: { // LB17
                        auto it_2 = boost::text::find_if_not(
                            it, last, space_or_skip);
                        in_space_skipper =
                            it_2 != last && boost::text::line_prop(*it_2) ==
                                                line_property::B2;
                        break;
                    }
                    default: break;
                    }

                    backed_up = in_space_skipper;
                    if (!in_space_skipper && detail::skippable(state.prop)) {
                        it_ = boost::text::find_if_not_backward(
                            first, it, [](uint32_t cp) {
                                return detail::skippable(
                                    boost::text::line_prop(cp));
                            });
                        backed_up = it_ != it;
                    }

                    if (backed_up) {
                        state.it = it_;
                        state.prop = boost::text::line_prop(*state.it);
                        state.next_prop =
                            boost::text::line_prop(*std::next(state.it));
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
                    return result_t{first, false};
            }

            state.prev_prev_prop = line_property::AL;
            if (std::prev(state.it) != first)
                state.prev_prev_prop =
                    boost::text::line_prop(*std::prev(state.it, 2));
            state.prev_prop = boost::text::line_prop(*std::prev(state.it));
            state.next_prop = line_property::AL;
            if (std::next(state.it) != last)
                state.next_prop =
                    boost::text::line_prop(*std::next(state.it));

            state.emoji_state = detail::line_break_emoji_state_t::none;

            auto skip = [](detail::line_break_state<CPIter> state,
                           CPIter first) {
                if (detail::skippable(state.prev_prop)) {
                    auto temp_it = boost::text::find_if_not_backward(
                        first, state.it, [](uint32_t cp) {
                            return detail::skippable(
                                boost::text::line_prop(cp));
                        });
                    if (temp_it == state.it)
                        return state;
                    auto temp_prev_prop = boost::text::line_prop(*temp_it);
                    // Don't skip over the skippables id they are immediately
                    // preceded by a hard-break property.
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
                                boost::text::line_prop(*std::prev(temp_it));
                        }
                    }
                }
                return state;
            };

            for (; state.it != first; state = detail::prev(state)) {
                if (std::prev(state.it) != first)
                    state.prev_prev_prop =
                        boost::text::line_prop(*std::prev(state.it, 2));
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
                            temp_state = detail::prev(temp_state);
                            if (temp_state.it != first &&
                                std::prev(temp_state.it) != first) {
                                temp_state.prev_prev_prop =
                                    boost::text::line_prop(
                                        *std::prev(temp_state.it, 2));
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
                    auto it = boost::text::find_if_not_backward(
                        first, state.it, [](uint32_t cp) {
                            return boost::text::line_prop(cp) ==
                                   line_property::SP;
                        });
                    if (it != state.it &&
                        boost::text::line_prop(*it) == line_property::ZW)
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
                    return result_t{state.it, false};

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
                    auto const prop = boost::text::line_prop(cp);
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
                    auto const new_state = detail::skip_backward_spaces_between(
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
                    auto const new_state = detail::skip_backward_spaces_between(
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
                    auto const new_state = detail::skip_backward_spaces_between(
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
                    auto const new_state = detail::skip_backward_spaces_between(
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
                    auto it = boost::text::find_if_not_backward(
                        first, state.it, after_nu);
                    if (it != state.it)
                        ++it;
                    if (it != state.it) {
                        if (boost::text::line_prop(*it) ==
                            line_property::NU) {
                            state.it = it;
                            state.prop = boost::text::line_prop(*state.it);
                            state.prev_prop = line_property::AL;
                            state.prev_prev_prop = line_property::AL;
                            if (state.it != first) {
                                state.prev_prop = boost::text::line_prop(
                                    *std::prev(state.it));
                                if (std::prev(state.it) != first) {
                                    state.prev_prev_prop =
                                        boost::text::line_prop(
                                            *std::prev(state.it, 2));
                                }
                            }

                            if (detail::table_line_break(
                                    state.prev_prop, state.prop))
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

            return result_t{first, false};
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
            bool break_overlong_lines)
        {
            using result_t = line_break_result<CPIter>;

            if (first == last)
                return result_t{first, false};

            line_break_state<CPIter> state;
            state.it = first;

            if (++state.it == last)
                return result_t{state.it, detail::hard_break_cp(*first)};

            state.prev_prev_prop = line_property::AL;
            state.prev_prop = boost::text::line_prop(*first);
            state.prop = boost::text::line_prop(*state.it);
            state.next_prop = line_property::AL;
            if (std::next(state.it) != last)
                state.next_prop =
                    boost::text::line_prop(*std::next(state.it));

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
                if (!result.hard_break)
                    result.hard_break =
                        detail::hard_break_cp(*std::prev(result.iter));

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
                        auto it = detail::prefix_lower_bound(
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
                            auto const range =
                                boost::text::grapheme(first, it, last);
                            if (range.begin() != it && range.end() != it) {
                                auto const grapheme_extent =
                                    cp_extent(first, range.end());
                                if (grapheme_extent == extent)
                                    it = range.end();
                            }
#endif

                        result = {it, false};
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
                auto const result =
                    result_t{it, detail::hard_break_cp(*std::prev(it))};
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

            for (; state.it != last; state = detail::next(state)) {
                if (std::next(state.it) != last)
                    state.next_prop =
                        boost::text::line_prop(*std::next(state.it));
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
                        return boost::text::line_prop(cp) ==
                               line_property::SP;
                    });
                    if (it == last)
                        return break_overlong(result_t{it, false});
                    auto const prop = boost::text::line_prop(*it);
                    if (!lb6(prop) && prop != line_property::ZW &&
                        break_here(it)) {
                        return *latest_result;
                    }
                }

                // LB8a
                if (state.prev_prop == line_property::ZWJ)
                    continue;

                // LB9
                // Puting this here means not having to do it explicitly
                // below between prop and next_prop (and transitively,
                // between prev_prop and prop).
                state = detail::skip_forward(state, first, last);
                if (state.it == last)
                    return break_overlong(result_t{state.it, false});

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
                    auto const prop = boost::text::line_prop(cp);
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
                        auto next_state = detail::next(state);
                        if (std::next(next_state.it) != last) {
                            next_state.next_prop = boost::text::line_prop(
                                *std::next(next_state.it));
                        } else {
                            next_state.next_prop = line_property::AL;
                        }

                        auto const new_state =
                            detail::skip_forward_spaces_between(
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
                            return break_overlong(
                                result_t{new_state.it, false});
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
                        auto it = boost::text::find_if_not(
                            state.it, last, after_nu);
                        state.it = --it;
                        state.prop = boost::text::line_prop(*state.it);
                        state.next_prop = line_property::AL;
                        if (std::next(state.it) != last)
                            state.next_prop = boost::text::line_prop(
                                *std::next(state.it));
                    }

                    continue;
                }

                // LB14
                {
                    auto const new_state = detail::skip_forward_spaces_between(
                        state,
                        last,
                        [](line_property prop) {
                            return prop == line_property::OP;
                        },
                        [](line_property prop) { return true; });
                    if (new_state.it != state.it) {
                        state = detail::prev(new_state);
                        continue;
                    }
                }

                // LB15
                {
                    auto const new_state = detail::skip_forward_spaces_between(
                        state,
                        last,
                        [](line_property prop) {
                            return prop == line_property::QU;
                        },
                        [](line_property prop) {
                            return prop == line_property::OP;
                        });
                    if (new_state.it == last)
                        return break_overlong(result_t{new_state.it, false});
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB16 is handled as part of LB13.
                {
                    auto const new_state = detail::skip_forward_spaces_between(
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
                        return break_overlong(result_t{new_state.it, false});
                    if (new_state.it != state.it) {
                        state = new_state;
                        continue;
                    }
                }

                // LB17
                {
                    auto const new_state = detail::skip_forward_spaces_between(
                        state,
                        last,
                        [](line_property prop) {
                            return prop == line_property::B2;
                        },
                        [](line_property prop) {
                            return prop == line_property::B2;
                        });
                    if (new_state.it == last)
                        return break_overlong(result_t{new_state.it, false});
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
                    auto it =
                        boost::text::find_if_not(state.it, last, after_nu);
                    state.it = --it;
                    state.prop = boost::text::line_prop(*state.it);
                    state.next_prop = line_property::AL;
                    if (std::next(state.it) != last) {
                        state.next_prop =
                            boost::text::line_prop(*std::next(state.it));
                    }
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

                if (detail::table_line_break(state.prev_prop, state.prop) &&
                    break_here(state.it)) {
                    return *latest_result;
                }
            }

            return break_overlong(result_t{state.it, false});
        }

        template<typename CPIter, typename Sentinel>
        CPIter prev_hard_line_break_impl(
            CPIter first, CPIter it, Sentinel last)
        {
            return detail::prev_line_break_impl(first, it, last, true).iter;
        }

        template<typename CPIter, typename Sentinel>
        bool
        at_hard_line_break_impl(CPIter first, CPIter it, Sentinel last)
        {
            if (it == last)
                return true;
            return detail::prev_line_break_impl(first, it, last, true).iter ==
                   it;
        }

        template<typename CPIter, typename Sentinel>
        line_break_result<CPIter> prev_allowed_line_break_impl(
            CPIter first, CPIter it, Sentinel last)
        {
            return detail::prev_line_break_impl(first, it, last, false);
        }

        template<typename CPIter, typename Sentinel>
        bool at_allowed_line_break_impl(
            CPIter first, CPIter it, Sentinel last)
        {
            if (it == last)
                return true;
            return detail::prev_line_break_impl(first, it, last, false).iter ==
                   it;
        }

        template<typename CPIter, typename Sentinel>
        CPIter next_hard_line_break_impl(CPIter first, Sentinel last)
        {
            no_op_cp_extent<CPIter, int> no_op;
            return detail::next_line_break_impl(
                       first, last, true, 0, no_op, false)
                .iter;
        }

        template<typename CPIter, typename Sentinel>
        line_break_result<CPIter>
        next_allowed_line_break_impl(CPIter first, Sentinel last)
        {
            no_op_cp_extent<CPIter, int> no_op;
            return detail::next_line_break_impl(
                first, last, false, 0, no_op, false);
        }

        template<typename CPIter, typename Sentinel>
        struct next_hard_line_break_callable
        {
            auto operator()(CPIter it, Sentinel last)
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return detail::next_hard_line_break_impl(it, last);
            }
        };

        inline void * align(
            std::size_t alignment,
            std::size_t size,
            void *& ptr,
            std::size_t & space)
        {
#if defined(__GNUC__) && __GNUC__ < 5
            void * retval = nullptr;
            if (size <= space) {
                char * p1 = static_cast<char *>(ptr);
                char * p2 = reinterpret_cast<char *>(
                    reinterpret_cast<size_t>(p1 + (alignment - 1)) &
                    -alignment);
                size_t d = static_cast<size_t>(p2 - p1);
                if (d <= space - size) {
                    retval = p2;
                    ptr = retval;
                    space -= d;
                }
            }
            return retval;
#else
            return std::align(alignment, size, ptr, space);
#endif
        }

        template<
            typename CPExtentFunc,
            bool trivial =
#if defined(__GNUC__) && __GNUC__ < 5
                has_trivial_copy<CPExtentFunc>::value
#else
                std::is_trivially_copy_constructible<CPExtentFunc>::value
#endif
            >
        struct optional_extent_func
        {
            optional_extent_func() : ptr_(nullptr) {}
            optional_extent_func(optional_extent_func const & other) :
                ptr_(nullptr)
            {
                if (other.ptr_)
                    ptr_ = new (aligned_ptr()) CPExtentFunc(*other);
            }
            optional_extent_func(optional_extent_func && other) : ptr_(nullptr)
            {
                if (other.ptr_)
                    ptr_ = new (aligned_ptr()) CPExtentFunc(std::move(*other));
            }

            optional_extent_func & operator=(optional_extent_func const & other)
            {
                destruct();
                if (other.ptr_)
                    ptr_ = new (aligned_ptr()) CPExtentFunc(*other);
                return *this;
            }
            optional_extent_func & operator=(optional_extent_func && other)
            {
                destruct();
                if (other.ptr_)
                    ptr_ = new (aligned_ptr()) CPExtentFunc(std::move(*other));
                return *this;
            }

            optional_extent_func(CPExtentFunc && f)
            {
                ptr_ = new (aligned_ptr()) CPExtentFunc(std::move(f));
            }
            optional_extent_func(CPExtentFunc const & f)
            {
                ptr_ = new (aligned_ptr()) CPExtentFunc(f);
            }

            ~optional_extent_func() { destruct(); }

            optional_extent_func & operator=(CPExtentFunc && f)
            {
                destruct();
                ptr_ = new (aligned_ptr()) CPExtentFunc(std::move(f));
            }
            optional_extent_func & operator=(CPExtentFunc const & f)
            {
                destruct();
                ptr_ = new (aligned_ptr()) CPExtentFunc(f);
            }

            explicit operator bool() const { return ptr_; }
            CPExtentFunc const & operator*() const { return *ptr_; }

            CPExtentFunc & operator*() { return *ptr_; }

        private:
            void * aligned_ptr()
            {
                void * ptr = buf_.data();
                std::size_t space = buf_.size();
                void * const retval = align(
                    alignof(CPExtentFunc), sizeof(CPExtentFunc), ptr, space);
                assert(retval);
                return retval;
            }
            void destruct()
            {
                if (ptr_)
                    ptr_->~CPExtentFunc();
            }

            std::array<char, sizeof(CPExtentFunc) + alignof(CPExtentFunc)> buf_;
            CPExtentFunc * ptr_;
        };

        template<typename CPExtentFunc>
        struct optional_extent_func<CPExtentFunc, true>
        {
            optional_extent_func() : ptr_(nullptr) {}
            optional_extent_func(optional_extent_func const & other) :
                ptr_(nullptr)
            {
                if (other.ptr_)
                    construct(*other);
            }
            optional_extent_func(CPExtentFunc const & f) { construct(f); }
            optional_extent_func(CPExtentFunc && f) { construct(f); }
            optional_extent_func & operator=(CPExtentFunc const & f)
            {
                construct(f);
                return *this;
            }
            optional_extent_func & operator=(CPExtentFunc && f)
            {
                construct(f);
                return *this;
            }

            template<typename T>
            optional_extent_func & operator=(CPExtentFunc const & f)
            {
                construct(f);
            }

            explicit operator bool() const { return ptr_; }
            CPExtentFunc const & operator*() const { return *ptr_; }

            CPExtentFunc & operator*() { return *ptr_; }

        private:
            void construct(CPExtentFunc f)
            {
                void * ptr = buf_.data();
                std::size_t space = buf_.size();
                void * const aligned_ptr = align(
                    alignof(CPExtentFunc), sizeof(CPExtentFunc), ptr, space);
                assert(aligned_ptr);

                ptr_ = new (aligned_ptr) CPExtentFunc(f);
            }

            std::array<char, sizeof(CPExtentFunc) + alignof(CPExtentFunc)> buf_;
            CPExtentFunc * ptr_;
        };

        template<typename Extent, typename CPExtentFunc>
        struct next_allowed_line_break_within_extent_callable
        {
            next_allowed_line_break_within_extent_callable() :
                extent_(), cp_extent_(), break_overlong_lines_(true)
            {}

            next_allowed_line_break_within_extent_callable(
                Extent extent,
                CPExtentFunc cp_extent,
                bool break_overlong_lines) :
                extent_(extent),
                cp_extent_(std::move(cp_extent)),
                break_overlong_lines_(break_overlong_lines)
            {}

            template<typename BreakResult, typename Sentinel>
            BreakResult
            operator()(BreakResult result, Sentinel last) const
            {
                return detail::next_line_break_impl(
                    result.iter,
                    last,
                    false,
                    extent_,
                    *cp_extent_,
                    break_overlong_lines_);
            }

            /** Moves the contained `CPExtentFunc` out of *this. */
            CPExtentFunc extent_func() &&
            {
                BOOST_ASSERT(cp_extent_);
                return std::move(*cp_extent_);
            }

        private:
            Extent extent_;
            optional_extent_func<CPExtentFunc> cp_extent_;
            bool break_overlong_lines_;
        };

        template<typename CPIter>
        struct prev_hard_line_break_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last)
                -> cp_iter_ret_t<CPIter, CPIter>
            {
                return detail::prev_line_break_impl(first, it, last, true).iter;
            }
        };

        template<typename CPRange, typename CPIter>
        auto prev_hard_line_break_cr_impl(CPRange && range, CPIter it)
            -> iterator_t<CPRange>
        {
            return detail::prev_hard_line_break_impl(
                detail::begin(range), it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto prev_hard_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
            -> iterator_t<GraphemeRange>
        {
            using cp_iter_t = decltype(range.begin().base());
            return {
                range.begin().base(),
                detail::prev_hard_line_break_impl(
                    range.begin().base(),
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base()),
                range.end().base()};
        }

        template<typename CPRange, typename CPIter>
        auto next_hard_line_break_cr_impl(CPRange && range, CPIter it)
            -> iterator_t<CPRange>
        {
            return detail::next_hard_line_break_impl(it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto next_hard_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
            -> iterator_t<GraphemeRange>
        {
            using cp_iter_t = decltype(range.begin().base());
            return {
                range.begin().base(),
                detail::next_hard_line_break_impl(
                    static_cast<cp_iter_t>(it.base()), range.end().base()),
                range.end().base()};
        }

        template<typename CPRange, typename CPIter>
        auto
        prev_allowed_line_break_cr_impl(CPRange && range, CPIter it)
            -> line_break_result<iterator_t<CPRange>>
        {
            return detail::prev_allowed_line_break_impl<iterator_t<CPRange>>(
                detail::begin(range), it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto prev_allowed_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
            -> line_break_result<iterator_t<GraphemeRange>>
        {
            using cp_iter_t = decltype(range.begin().base());
            auto const prev = detail::prev_allowed_line_break_impl(
                range.begin().base(),
                static_cast<cp_iter_t>(it.base()),
                range.end().base());
            return {
                {range.begin().base(), prev.iter, range.end().base()},
                prev.hard_break};
        }

        template<typename CPRange, typename CPIter>
        auto
        next_allowed_line_break_cr_impl(CPRange && range, CPIter it)
            -> line_break_result<iterator_t<CPRange>>
        {
            return detail::next_allowed_line_break_impl<iterator_t<CPRange>>(
                it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto next_allowed_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
            -> line_break_result<iterator_t<GraphemeRange>>
        {
            using cp_iter_t = decltype(range.begin().base());
            auto const next = detail::next_allowed_line_break_impl(
                static_cast<cp_iter_t>(it.base()), range.end().base());
            return {
                {range.begin().base(), next.iter, range.end().base()},
                next.hard_break};
        }

        template<typename CPRange, typename CPIter>
        bool at_hard_line_break_cr_impl(CPRange && range, CPIter it)
        {
            if (it == detail::end(range))
                return true;
            return detail::prev_hard_line_break_impl(
                       detail::begin(range), it, detail::end(range)) == it;
        }

        template<typename GraphemeRange, typename GraphemeIter>
        bool at_hard_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
        {
            if (it == detail::end(range))
                return true;
            using cp_iter_t = decltype(range.begin().base());
            cp_iter_t it_ = static_cast<cp_iter_t>(it.base());
            return detail::prev_hard_line_break_impl(
                       range.begin().base(), it_, range.end().base()) == it_;
        }

        template<typename CPRange, typename CPIter>
        bool at_allowed_line_break_cr_impl(CPRange && range, CPIter it)
        {
            if (it == detail::end(range))
                return true;
            return detail::prev_allowed_line_break_impl<iterator_t<CPRange>>(
                detail::begin(range), it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        bool at_allowed_line_break_gr_impl(
            GraphemeRange && range, GraphemeIter it)
        {
            if (it == detail::end(range))
                return true;
            using cp_iter_t = decltype(range.begin().base());
            cp_iter_t it_ = static_cast<cp_iter_t>(it.base());
            return detail::prev_allowed_line_break_impl(
                       range.begin().base(), it_, range.end().base()) == it_;
        }

        template<typename CPIter, typename Sentinel>
        utf_view<format::utf32, CPIter>
        line_impl(CPIter first, CPIter it, Sentinel last)
        {
            first = detail::prev_hard_line_break_impl(first, it, last);
            return utf_view<format::utf32, CPIter>{
                first, detail::next_hard_line_break_impl(first, last)};
        }

        template<typename CPRange, typename CPIter>
        auto line_cr_impl(CPRange && range, CPIter it)
            -> utf_view<format::utf32, iterator_t<CPRange>>
        {
            auto first = detail::prev_hard_line_break_impl<iterator_t<CPRange>>(
                detail::begin(range), it, detail::end(range));
            return utf_view<format::utf32, iterator_t<CPRange>>{
                first,
                detail::next_hard_line_break_impl(first, detail::end(range))};
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto line_gr_impl(GraphemeRange && range, GraphemeIter it)
            -> grapheme_view<decltype(range.begin().base())>
        {
            using cp_iter_t = decltype(range.begin().base());
            auto first = detail::prev_hard_line_break_impl(
                range.begin().base(),
                static_cast<cp_iter_t>(it.base()),
                range.end().base());
            return {
                range.begin().base(),
                first,
                detail::next_hard_line_break_impl(first, range.end().base()),
                range.end().base()};
        }

        template<
            typename CPIter,
            typename CPSentinel,
            typename NextFunc,
            typename CPRange>
        struct forward_allowed_break_iterator
            : stl_interfaces::proxy_iterator_interface<
                  forward_allowed_break_iterator<
                      CPIter,
                      CPSentinel,
                      NextFunc,
                      CPRange>,
                  std::forward_iterator_tag,
                  CPRange>
        {
            forward_allowed_break_iterator() = default;

            forward_allowed_break_iterator(
                CPIter first, CPSentinel last, NextFunc * next_func) :
                seg_({first, false}, {first, false}),
                last_(last),
                next_func_(next_func)
            {
                seg_.second = (*next_func_)(seg_.first, last_);
            }

            CPRange operator*() const
            {
                return CPRange{seg_.first, seg_.second};
            }

            forward_allowed_break_iterator & operator++()
            {
                auto const next_it = (*next_func_)(seg_.second, last_);
                seg_.first = seg_.second;
                seg_.second = next_it;
                return *this;
            }

            friend bool operator==(
                forward_allowed_break_iterator lhs,
                forward_allowed_break_iterator rhs)
            {
                return lhs.seg_ == rhs.seg_;
            }

            template<typename Sentinel>
            friend std::enable_if_t<
                !std::is_same<CPIter, CPSentinel>::value &&
                    std::is_same<Sentinel, CPSentinel>::value,
                bool>
            operator==(
                forward_allowed_break_iterator lhs, Sentinel rhs)
            {
                return lhs.seg_.first == rhs;
            }

            using base_type = stl_interfaces::proxy_iterator_interface<
                forward_allowed_break_iterator<
                    CPIter,
                    CPSentinel,
                    NextFunc,
                    CPRange>,
                std::forward_iterator_tag,
                CPRange>;
            using base_type::operator++;

        private:
            std::pair<line_break_result<CPIter>, line_break_result<CPIter>>
                seg_ = {};
            CPSentinel last_ = {};
            NextFunc * next_func_ = nullptr;
        };

        template<
            typename T,
            typename Iter,
            typename CPSentinel,
            typename NextFunc>
        struct make_fwd_allowed_break_iter_last
        {
            using type = CPSentinel;
            static auto call(Iter first, CPSentinel last, NextFunc * next_func)
            {
                return last;
            }
        };

        template<typename T, typename Iter, typename NextFunc>
        struct make_fwd_allowed_break_iter_last<T, Iter, Iter, NextFunc>
        {
            using type = T;
            static auto call(Iter first, Iter last, NextFunc * next_func)
            {
                return T(last, last, next_func);
            }
        };
    }

    /** Represents a forward-iterable view of non-overlapping code point
        subranges.  Each subrange represents the code points between one
        allowed line break and the next.  Each subrange is lazily produced; an
        output subrange is not produced until an iterator is dereferenced.
        Each element has a member function `bool hard_break()` that indicates
        whether the end of the subrange is at a hard line break, or just an
        allowed line break location.  If forward_line_break_view is
        constructed with a stateful `CPExtentFunc`, you can get it back by
        moving it out via `extent_func()`. */
    template<
        typename CPIter,
        typename CPSentinel,
        typename Extent,
        typename CPExtentFunc,
        typename Subrange = line_break_cp_view<CPIter>>
    struct forward_line_break_view
        : stl_interfaces::view_interface<forward_line_break_view<
              CPIter,
              CPSentinel,
              Extent,
              CPExtentFunc,
              Subrange>>
    {
        using iterator = detail::forward_allowed_break_iterator<
            CPIter,
            CPSentinel,
            detail::next_allowed_line_break_within_extent_callable<
                Extent,
                CPExtentFunc>,
            Subrange>;
        using sentinel = typename detail::make_fwd_allowed_break_iter_last<
            iterator,
            CPIter,
            CPSentinel,
            detail::next_allowed_line_break_within_extent_callable<
                Extent,
                CPExtentFunc>>::type;

        forward_line_break_view() {}
        forward_line_break_view(
            CPIter first,
            CPSentinel last,
            Extent max_extent,
            CPExtentFunc cp_extent,
            bool break_overlong_lines) :
            next_(max_extent, std::move(cp_extent), break_overlong_lines),
            first_(first, last, &next_)
        {
            last_ = detail::make_fwd_allowed_break_iter_last<
                iterator,
                CPIter,
                CPSentinel,
                detail::next_allowed_line_break_within_extent_callable<
                    Extent,
                    CPExtentFunc>>::call(first, last, &next_);
        }

        iterator begin() const { return first_; }
        sentinel end() const { return last_; }

        /** Moves the contained `CPExtentFunc` out of *this. */
        CPExtentFunc extent_func() &&
        {
            return std::move(next_).extent_func();
        }

    private:
        detail::
            next_allowed_line_break_within_extent_callable<Extent, CPExtentFunc>
                next_;
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

    namespace detail {
        template<
            typename CPIter,
            typename Sentinel,
            typename Extent,
            typename CPExtentFunc>
        forward_line_break_view<CPIter, Sentinel, Extent, CPExtentFunc>
        lines_impl(
            CPIter first,
            Sentinel last,
            Extent max_extent,
            CPExtentFunc cp_extent,
            bool break_overlong_lines = true)
        {
            return {
                first,
                last,
                max_extent,
                std::move(cp_extent),
                break_overlong_lines};
        }

        template<typename CPRange, typename Extent, typename CPExtentFunc>
        forward_line_break_view<
            iterator_t<CPRange>,
            sentinel_t<CPRange>,
            Extent,
            CPExtentFunc>
        lines_cr_impl(
            CPRange && range,
            Extent max_extent,
            CPExtentFunc cp_extent,
            bool break_overlong_lines = true)
        {
            return {
                detail::begin(range),
                detail::end(range),
                max_extent,
                std::move(cp_extent),
                break_overlong_lines};
        }

        template<typename GraphemeRange, typename Extent, typename CPExtentFunc>
        auto lines_gr_impl(
            GraphemeRange && range,
            Extent max_extent,
            CPExtentFunc cp_extent,
            bool break_overlong_lines = true)
            -> forward_line_break_view<
                gr_rng_cp_iter_t<GraphemeRange>,
                gr_rng_cp_sent_t<GraphemeRange>,
                Extent,
                CPExtentFunc,
                line_break_grapheme_view<gr_rng_cp_iter_t<GraphemeRange>>>
        {
            return {
                range.begin().base(),
                gr_rng_cp_last<GraphemeRange>::call(range),
                max_extent,
                std::move(cp_extent),
                break_overlong_lines};
        }

        template<typename CPIter, typename Sentinel>
        line_break_cp_view<CPIter>
        allowed_line_impl(CPIter first, CPIter it, Sentinel last)
        {
            auto const first_ =
                detail::prev_allowed_line_break_impl(first, it, last);
            return line_break_cp_view<CPIter>{
                first_,
                detail::next_allowed_line_break_impl(first_.iter, last)};
        }

        template<typename CPRange, typename CPIter>
        auto allowed_line_cr_impl(CPRange && range, CPIter it)
            -> line_break_cp_view<iterator_t<CPRange>>
        {
            auto const first =
                detail::prev_allowed_line_break_impl<iterator_t<CPRange>>(
                    detail::begin(range), it, detail::end(range));
            return line_break_cp_view<iterator_t<CPRange>>{
                first,
                detail::next_allowed_line_break_impl<iterator_t<CPRange>>(
                    first.iter, detail::end(range))};
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto
        allowed_line_gr_impl(GraphemeRange && range, GraphemeIter it)
            -> line_break_grapheme_view<decltype(range.begin().base())>
        {
            auto const first =
                detail::prev_allowed_line_break_gr_impl(range, it);
            return {
                first,
                detail::next_allowed_line_break_gr_impl(range, first.iter)};
        }

        template<typename CPIter, typename CPSentinel, typename Subrange>
        struct allowed_break_iterator
            : stl_interfaces::proxy_iterator_interface<
                  allowed_break_iterator<CPIter, CPSentinel, Subrange>,
                  std::bidirectional_iterator_tag,
                  Subrange>
        {
            allowed_break_iterator() = default;

            // begin
            allowed_break_iterator(CPIter first, CPSentinel last) :
                first_(first), seg_({first, false}, {first, false}), last_(last)
            {
                seg_.second = detail::next_allowed_line_break_impl(
                    seg_.second.iter, last_);
            }

            // end
            allowed_break_iterator(
                CPIter first, CPIter it, CPSentinel last) :
                first_(first), seg_({it, false}, {it, false}), last_(last)
            {}

            Subrange operator*() const
            {
                return Subrange{seg_.first, seg_.second};
            }

            allowed_break_iterator & operator++()
            {
                auto const next_it = detail::next_allowed_line_break_impl(
                    seg_.second.iter, last_);
                seg_.first = seg_.second;
                seg_.second = next_it;
                return *this;
            }

            allowed_break_iterator & operator--()
            {
                if (seg_.first == first_) {
                    seg_.second.iter = first_;
                    return *this;
                }

                bool const at_end = seg_.first == seg_.second;

                auto const prev_it = detail::prev_line_break_impl(
                    first_, std::prev(seg_.first.iter), last_, false);
                seg_.second = seg_.first;
                seg_.first = prev_it;

                if (at_end) {
                    seg_.second = detail::next_allowed_line_break_impl(
                        seg_.first.iter, seg_.second.iter);
                }

                return *this;
            }

            friend bool operator==(
                allowed_break_iterator lhs, allowed_break_iterator rhs)
            {
                return lhs.seg_ == rhs.seg_;
            }

            template<typename Sentinel>
            friend std::enable_if_t<
                !std::is_same<CPIter, CPSentinel>::value &&
                    std::is_same<Sentinel, CPSentinel>::value,
                bool>
            operator==(allowed_break_iterator lhs, Sentinel rhs)
            {
                return lhs.seg_.first == rhs;
            }

            using base_type = stl_interfaces::proxy_iterator_interface<
                allowed_break_iterator<CPIter, CPSentinel, Subrange>,
                std::bidirectional_iterator_tag,
                Subrange>;
            using base_type::operator++;
            using base_type::operator--;

        private:
            CPIter first_ = {};
            std::pair<line_break_result<CPIter>, line_break_result<CPIter>>
                seg_ = {};
            CPSentinel last_ = {};
        };

        template<typename T, typename Iter, typename CPSentinel>
        struct make_allowed_break_iter_last
        {
            using type = CPSentinel;
            static auto call(Iter first, CPSentinel last) { return last; }
        };

        template<typename T, typename Iter>
        struct make_allowed_break_iter_last<T, Iter, Iter>
        {
            using type = T;
            static auto call(Iter first, Iter last) {
                return T(first, last, last);
            }
        };
    }

    /** Represents a bidirectionally-iterable view of non-overlapping code
        point subranges.  Each subrange represents the code points between one
        allowed line break and the next.  Each subrange is lazily produced; an
        output subrange is not produced until an iterator is dereferenced.
        Each element has a member function `bool hard_break()` that indicates
        whether the end of the subrange is at a hard line break, or just an
        allowed line break location. */
    template<
        typename CPIter,
        typename CPSentinel,
        typename Subrange = line_break_cp_view<CPIter>>
    struct line_break_view : stl_interfaces::view_interface<
                                 line_break_view<CPIter, CPSentinel, Subrange>>
    {
        using iterator =
            detail::allowed_break_iterator<CPIter, CPSentinel, Subrange>;
        using sentinel = typename detail::
            make_allowed_break_iter_last<iterator, CPIter, CPSentinel>::type;

        line_break_view() {}
        line_break_view(CPIter first, CPSentinel last) :
            first_(first, last)
        {
            last_ = detail::make_allowed_break_iter_last<
                iterator,
                CPIter,
                CPSentinel>::call(first, last);
        }

        iterator begin() const { return first_; }
        sentinel end() const { return last_; }

    private:
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

    namespace detail {
        template<typename CPIter, typename Sentinel>
        line_break_view<CPIter, Sentinel>
        allowed_lines_impl(CPIter first, Sentinel last)
        {
            return {first, last};
        }

        template<typename CPRange>
        auto allowed_lines_cr_impl(CPRange && range)
            -> line_break_view<iterator_t<CPRange>, sentinel_t<CPRange>>
        {
            return {detail::begin(range), detail::end(range)};
        }

        template<typename GraphemeRange>
        auto allowed_lines_gr_impl(GraphemeRange && range)
            -> line_break_view<
                gr_rng_cp_iter_t<GraphemeRange>,
                gr_rng_cp_sent_t<GraphemeRange>,
                line_break_grapheme_view<gr_rng_cp_iter_t<GraphemeRange>>>
        {
            return {
                range.begin().base(),
                gr_rng_cp_last<GraphemeRange>::call(range)};
        }
    }
}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest hard line break at or before before `it`. If `it ==
        first`, that is returned.  Otherwise, the first code point of the line
        that `it` is within is returned (even if `it` is already at the first
        code point of a line).  A hard line break follows any code points with
        the property BK, CR (not followed by LF), LF, or NL. */
    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_hard_line_break(I first, I it, S last);

    /** Returns true iff `it` is at the beginning of a line (considering only
        hard line breaks), or `it == last`.  A hard line break follows any
        code points with the property BK, CR (not followed by LF), LF, or
        NL. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_hard_line_break(I first, I it, S last);

    /** Finds the nearest line break opportunity at or before before `it`.  If
        `it == first`, that is returned.  Otherwise, the first code point of
        the line that `it` is within is returned (even if `it` is already at
        the first code point of a line). */
    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_result<I> prev_allowed_line_break(I first, I it, S last);

    /** Returns true iff `it` is at the beginning of a line, or `it ==
        last`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_allowed_line_break(I first, I it, S last);

#else

    template<typename CPIter, typename Sentinel>
    auto prev_hard_line_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::prev_line_break_impl(first, it, last, true).iter;
    }

    template<typename CPIter, typename Sentinel>
    auto at_hard_line_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        if (it == last)
            return true;
        return detail::prev_line_break_impl(first, it, last, true).iter == it;
    }

    template<typename CPIter, typename Sentinel>
    auto
    prev_allowed_line_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<line_break_result<CPIter>, CPIter>
    {
        return detail::prev_line_break_impl(first, it, last, false);
    }

    template<typename CPIter, typename Sentinel>
    auto at_allowed_line_break(CPIter first, CPIter it, Sentinel last)
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        if (it == last)
            return true;
        return detail::prev_line_break_impl(first, it, last, false).iter == it;
    }

#endif

}}}


#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_hard_line_break(I first, I it, S last)
    {
        return detail::prev_line_break_impl(first, it, last, true).iter;
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_hard_line_break(I first, I it, S last)
    {
        if (it == last)
            return true;
        return detail::prev_line_break_impl(first, it, last, true).iter == it;
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_result<I> prev_allowed_line_break(I first, I it, S last)
    {
        return detail::prev_line_break_impl(first, it, last, false);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_allowed_line_break(I first, I it, S last)
    {
        if (it == last)
            return true;
        return detail::prev_line_break_impl(first, it, last, false).iter == it;
    }

}}}

#endif

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the next hard line break after `first`.  This will be the first
        code point after the current line, or `last` if no next line exists.
        A hard line break follows any code points with the property BK, CR
        (not followed by LF), LF, or NL.

        \pre `first` is at the beginning of a line. */
    template<code_point_iter I, std::sentinel_for<I> S>
    I next_hard_line_break(I first, S last);

    /** Finds the next line break opportunity after `first`. This will be the
        first code point after the current line, or `last` if no next line
        exists.

        \pre `first` is at the beginning of a line. */
    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_result<I> next_allowed_line_break(I first, S last);

    /** Finds the nearest hard line break at or before before `it`. If `it ==
        r.begin()`, that is returned.  Otherwise, the first code point of
        the line that `it` is within is returned (even if `it` is already at
        the first code point of a line).  A hard line break follows any code
        points with the property BK, CR (not followed by LF), LF, or NL. */
    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> prev_hard_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns a grapheme_iterator to the nearest hard line break at or
        before before `it`.  If `it == r.begin()`, that is returned.
        Otherwise, the first grapheme of the line that `it` is within is
        returned (even if `it` is already at the first grapheme of a line).  A
        hard line break follows any code points with the property BK, CR (not
        followed by LF), LF, or NL. */
    template<grapheme_range R>
    std::ranges::borrowed_iterator_t<R> prev_hard_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Finds the next hard line break after `it`.  This will be the first
        code point after the current line, or `r.end()` if no next line
        exists.  A hard line break follows any code points with the property
        BK, CR (not followed by LF), LF, or NL.

        \pre `it` is at the beginning of a line. */
    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> next_hard_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns a grapheme_iterator to the next hard line break after `it`.
        This will be the first grapheme after the current line, or
        `r.end()` if no next line exists.  A hard line break follows any
        code points with the property BK, CR (not followed by LF), LF, or NL.

        \pre `it` is at the beginning of a line. */
    template<grapheme_range R>
    std::ranges::borrowed_iterator_t<R> next_hard_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Finds the nearest line break opportunity at or before before `it`.  If
        `it == r.begin()`, that is returned.  Otherwise, the first code point
        of the line that `it` is within is returned (even if `it` is already
        at the first code point of a line.  Returns a `line_break_result`; in
        C++20 and later, if `std::ranges::borrowed_range<R>` is `false`, this
        function returns a `std::ranges::dangling` instead. */
    template<code_point_range R>
    detail::unspecified prev_allowed_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns a grapheme_iterator to the nearest line break opportunity at
        or before before `it`.  If `it == r.begin()`, that is returned.
        Otherwise, the first grapheme of the line that `it` is within is
        returned (even if `it` is already at the first grapheme of a line).
        Returns a `line_break_result`; in C++20 and later, if
        `std::ranges::borrowed_range<R>` is `false`, this function returns a
        `std::ranges::dangling` instead. */
    template<grapheme_range R>
    detail::unspecified prev_allowed_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Finds the next line break opportunity after `it`.  This will be the
        first code point after the current line, or `r.end()` if no next line
        exists.  Returns a `line_break_result`; in C++20 and later, if
        `std::ranges::borrowed_range<R>` is `false`, this function returns a
        `std::ranges::dangling` instead.

        \pre `it` is at the beginning of a line. */
    template<code_point_range R>
    detail::unspecified next_allowed_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns a grapheme_iterator to the next line break opportunity after
        `it`.  This will be the first grapheme after the current line, or
        `r.end()` if no next line exists.  Returns a `line_break_result`; in
        C++20 and later, if `std::ranges::borrowed_range<R>` is `false`, this
        function returns a `std::ranges::dangling` instead.

        \pre `it` is at the beginning of a line. */
    template<grapheme_range R>
    detail::unspecified next_allowed_line_break(
        R && r, std::ranges::iterator_t<R> it);

    /** Returns true iff `it` is at the beginning of a line (considering only
        hard line breaks), or `it == detail::end(range)`.  A hard line break
        follows any code points with the property BK, CR (not followed by LF),
        LF, or NL. */
    template<code_point_range R>
    bool at_hard_line_break(R && r, std::ranges::iterator_t<R> it);

    /** Returns true iff `it` is at the beginning of a line (considering only
        hard line breaks), or `it == detail::end(range)`.  A hard line break
        follows any code points with the property BK, CR (not followed by LF),
        LF, or NL. */
    template<grapheme_range R>
    bool at_hard_line_break(R && r, std::ranges::iterator_t<R> it);

    /** Returns true iff `it` is at the beginning of a line, or `it ==
        detail::end(range)`. */
    template<code_point_range R>
    bool at_allowed_line_break(R && r, std::ranges::iterator_t<R> it);

    /** Returns true iff `it` is at the beginning of a line, or `it ==
        detail::end(range)`. */
    template<grapheme_range R>
    bool at_allowed_line_break(R && r, std::ranges::iterator_t<R> it);

#else

    template<typename CPIter, typename Sentinel>
    auto next_hard_line_break(CPIter first, Sentinel last)
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::next_hard_line_break_impl(first, last);
    }

    template<typename CPIter, typename Sentinel>
    auto next_allowed_line_break(CPIter first, Sentinel last)
        -> detail::cp_iter_ret_t<line_break_result<CPIter>, CPIter>
    {
        return detail::next_allowed_line_break_impl(first, last);
    }

    template<typename CPRange, typename CPIter>
    auto prev_hard_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::prev_hard_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto prev_hard_line_break(
        GraphemeRange && range, GraphemeIter it)->detail::
        graph_rng_alg_ret_t<detail::iterator_t<GraphemeRange>, GraphemeRange>
    {
        return detail::prev_hard_line_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto next_hard_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::next_hard_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto next_hard_line_break(
        GraphemeRange && range, GraphemeIter it)->detail::
        graph_rng_alg_ret_t<detail::iterator_t<GraphemeRange>, GraphemeRange>
    {
        return detail::next_hard_line_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto prev_allowed_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<
            line_break_result<detail::iterator_t<CPRange>>,
            CPRange>
    {
        return detail::prev_allowed_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto prev_allowed_line_break(
        GraphemeRange && range, GraphemeIter it)
        ->detail::graph_rng_alg_ret_t<
            line_break_result<detail::iterator_t<GraphemeRange>>,
            GraphemeRange>
    {
        return detail::prev_allowed_line_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto next_allowed_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<
            line_break_result<detail::iterator_t<CPRange>>,
            CPRange>
    {
        return detail::next_allowed_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto next_allowed_line_break(
        GraphemeRange && range, GraphemeIter it)
        ->detail::graph_rng_alg_ret_t<
            line_break_result<detail::iterator_t<GraphemeRange>>,
            GraphemeRange>
    {
        return detail::next_allowed_line_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto at_hard_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return detail::at_hard_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto
    at_hard_line_break(GraphemeRange && range, GraphemeIter it)
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return detail::at_hard_line_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto at_allowed_line_break(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return detail::at_allowed_line_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto
    at_allowed_line_break(GraphemeRange && range, GraphemeIter it)
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return detail::at_allowed_line_break_gr_impl(range, it);
    }

#endif

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns the bounds of the line (using hard line breaks) that
        `it` lies within. */
    template<code_point_iter I, std::sentinel_for<I> S>
    utf_view<format::utf32, I> line(I first, I it, S last);

    /** Returns the bounds of the line (using hard line breaks) that `it` lies
        within.  Returns a `utf32_view`; in C++20 and later, if
        `std::ranges::borrowed_range<R>` is `false`, this function returns a
        `std::ranges::dangling` instead. */
    template<code_point_range R>
    detail::unspecified line(R && r, std::ranges::iterator_t<R> it);

    /** Returns grapheme range delimiting the bounds of the line (using hard
        line breaks) that `it` lies within.  Returns a `grapheme_view`; in
        C++20 and later, if `std::ranges::borrowed_range<R>` is `false`, this
        function returns a `std::ranges::dangling` instead. */
    template<grapheme_range R>
    detail::unspecified line(R && r, std::ranges::iterator_t<R> it);

    /** Returns the bounds of the smallest chunk of text that could be broken
        off into a line, searching from `it` in either direction. */
    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_cp_view<I> allowed_line(I first, I it, S last);

    /** Returns the bounds of the smallest chunk of text that could be broken
        off into a line, searching from `it` in either direction.  Returns a
        `line_break_cp_view`; in C++20 and later, if
        `std::ranges::borrowed_range<R>` is `false`, this function returns a
        `std::ranges::dangling` instead. */
    template<code_point_range R>
    detail::unspecified allowed_line(R && r, std::ranges::iterator_t<R> it);

    /** Returns a grapheme range delimiting the bounds of the line (using hard
        line breaks) that `it` lies within.  Returns a
        `line_break_grapheme_view`; in C++20 and later, if
        `std::ranges::borrowed_range<R>` is `false`, this function returns a
        `std::ranges::dangling` instead. */
    template<grapheme_range R>
    detail::unspecified allowed_line(R && r, std::ranges::iterator_t<R> it);

    /** Returns a view of the code point ranges delimiting lines (using hard
        line breaks) in `[first, last)`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    detail::unspecified lines(I first, S last);

    /** Returns a view of the code point ranges delimiting lines (using hard
        line breaks) in `r`.  The result is returned as a `borrowed_view_t` in
        C++20 and later. */
    template<code_point_range R>
    detail::unspecified lines(R && r);

    /** Returns a view of the grapheme ranges delimiting lines (using hard
        line breaks) in `r`.  The result is returned as a `borrowed_view_t` in
        C++20 and later. */
    template<grapheme_range R>
    detail::unspecified lines(R && r);

#else

    template<typename CPIter, typename Sentinel>
    utf_view<format::utf32, CPIter> line(CPIter first, CPIter it, Sentinel last)
    {
        return detail::line_impl(first, it, last);
    }

    template<typename CPRange, typename CPIter>
    auto line(CPRange && range, CPIter it)
        ->detail::cp_rng_alg_ret_t<
            utf_view<format::utf32, detail::iterator_t<CPRange>>,
            CPRange>
    {
        return detail::line_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto line(GraphemeRange && range, GraphemeIter it)
        -> detail::graph_rng_alg_ret_t<
            grapheme_view<decltype(range.begin().base())>,
            GraphemeRange>
    {
        return detail::line_gr_impl(range, it);
    }

#endif

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns a view of the code point ranges in `[first, last)` delimiting
        lines.  A line that does not end in a hard break will end in a allowed
        break that does not exceed `max_extent`, using the code point extents
        derived from `ExtentFunc`.  When a line has no allowed breaks before
        it would exceed `max_extent`, it will be broken only if
        `break_overlong_lines` is true.  If `break_overlong_lines` is false,
        such an unbreakable line will exceed `max_extent`. */
    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        typename Extent,
        line_break_cp_extent_func<I, Extent> ExtentFunc>
    detail::unspecified lines(
        I first,
        S last,
        Extent max_extent,
        ExtentFunc && cp_extent,
        bool break_overlong_lines = true);

    /** Returns a view of the code point ranges in `r` delimiting lines.
        A line that does not end in a hard break will end in a allowed break
        that does not exceed `max_extent`, using the code point extents
        derived from `ExtentFunc`.  When a line has no allowed breaks before
        it would exceed `max_extent`, it will be broken only if
        `break_overlong_lines` is true.  If `break_overlong_lines` is false,
        such an unbreakable line will exceed `max_extent`.  The result is
        returned as a `borrowed_view_t` in C++20 and later. */
    template<
        code_point_range R,
        typename Extent,
        line_break_cp_extent_func<std::ranges::iterator_t<R>, Extent>
            ExtentFunc>
    detail::unspecified lines(
        R && r,
        Extent max_extent,
        ExtentFunc cp_extent,
        bool break_overlong_lines = true);

    /** Returns a view of the grapheme ranges in `r` delimiting lines.  A
        line that does not end in a hard break will end in a allowed break
        that does not exceed `max_extent`, using the code point extents
        derived from `ExtentFunc`.  When a line has no allowed breaks before
        it would exceed `max_extent`, it will be broken only if
        `break_overlong_lines` is true.  If `break_overlong_lines` is false,
        such an unbreakable line will exceed `max_extent`.  The result is
        returned as a `borrowed_view_t` in C++20 and later. */
    template<
        grapheme_range R,
        typename Extent,
        line_break_cp_extent_func<code_point_iterator_t<R>, Extent> ExtentFunc>
    detail::unspecified lines(
        R && r,
        Extent max_extent,
        ExtentFunc cp_extent,
        bool break_overlong_lines = true);

    /** Returns a view of the code point ranges delimiting allowed lines in
        `[first, last)`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    detail::unspecified lines(I first, S last, allowed_breaks_t);

    /** Returns a view of the code point ranges delimiting allowed lines in
        `r`.  The result is returned as a `borrowed_view_t` in C++20 and
        later. */
    template<code_point_range R>
    detail::unspecified lines(R && r, allowed_breaks_t);

    /** Returns a view of the grapheme ranges delimiting allowed lines in `r`.
        The result is returned as a `borrowed_view_t` in C++20 and later. */
    template<grapheme_range R>
    detail::unspecified lines(R && r, allowed_breaks_t);

    /** Returns a view adaptor that can combined with a `code_point_range` or
        `grapheme_range` `r`, as in `r | lines(allowed_breaks)`. */
    detail::unspecified lines(allowed_breaks_t);

#else

    template<typename CPIter, typename Sentinel>
    line_break_cp_view<CPIter>
    allowed_line(CPIter first, CPIter it, Sentinel last)
    {
        return detail::allowed_line_impl(first, it, last);
    }

    template<typename CPRange, typename CPIter>
    auto allowed_line(CPRange && range, CPIter it)
        -> detail::cp_rng_alg_ret_t<
            line_break_cp_view<detail::iterator_t<CPRange>>,
            CPRange>
    {
        return detail::allowed_line_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto allowed_line(GraphemeRange && range, GraphemeIter it)
        ->detail::graph_rng_alg_ret_t<
            line_break_grapheme_view<decltype(range.begin().base())>,
            GraphemeRange>
    {
        return detail::allowed_line_gr_impl(range, it);
    }

    namespace dtl {
        struct lines_impl : range_adaptor_closure<lines_impl>
        {
            template<typename T>
            using does_arithmetic =
                decltype(-declval<T>() + declval<T>() / declval<T>() * declval<T>());

            template<
                typename CPIter,
                typename Sentinel,
                typename Enable = std::enable_if_t<
                    !is_detected<does_arithmetic, CPIter>::value>>
            auto operator()(CPIter first, Sentinel last) const
            {
                return detail::breaks_impl<
                    detail::prev_hard_line_break_callable,
                    detail::next_hard_line_break_callable>(first, last);
            }

            template<typename CPRange>
            auto operator()(CPRange && r) const
                -> detail::cp_rng_alg_ret_t<
                    decltype(detail::breaks_cr_impl<
                             detail::prev_hard_line_break_callable,
                             detail::next_hard_line_break_callable>(r)),
                    CPRange>
            {
                return detail::breaks_cr_impl<
                    detail::prev_hard_line_break_callable,
                    detail::next_hard_line_break_callable>(r);
            }

            template<typename GraphemeRange>
            auto operator()(GraphemeRange && r) const
                -> detail::graph_rng_alg_ret_t<
                    decltype(detail::breaks_gr_impl<
                             detail::prev_hard_line_break_callable,
                             detail::next_hard_line_break_callable>(r)),
                    GraphemeRange>
            {
                return detail::breaks_gr_impl<
                    detail::prev_hard_line_break_callable,
                    detail::next_hard_line_break_callable>(r);
            }

            template<
                typename CPIter,
                typename Sentinel,
                typename Extent,
                typename ExtentFunc>
            auto operator()(
                CPIter first,
                Sentinel last,
                Extent max_extent,
                ExtentFunc && cp_extent,
                bool break_overlong_lines = true) const
            {
                return detail::lines_impl(
                    first,
                    last,
                    max_extent,
                    std::move(cp_extent),
                    break_overlong_lines);
            }

            template<typename CPRange, typename Extent, typename ExtentFunc>
            auto operator()(
                CPRange && r,
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
                -> detail::cp_rng_alg_ret_t<
                    forward_line_break_view<
                        detail::iterator_t<CPRange>,
                        detail::sentinel_t<CPRange>,
                        Extent,
                        ExtentFunc>,
                    CPRange>

            {
                return detail::lines_cr_impl(
                    r, max_extent, std::move(cp_extent), break_overlong_lines);
            }

            template<
                typename GraphemeRange,
                typename Extent,
                typename ExtentFunc>
            auto operator()(
                GraphemeRange && r,
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
                -> detail::graph_rng_alg_ret_t<
                    forward_line_break_view<
                        detail::gr_rng_cp_iter_t<GraphemeRange>,
                        detail::gr_rng_cp_sent_t<GraphemeRange>,
                        Extent,
                        ExtentFunc,
                        line_break_grapheme_view<
                            detail::gr_rng_cp_iter_t<GraphemeRange>>>,
                    GraphemeRange>
            {
                return detail::lines_gr_impl(
                    r, max_extent, std::move(cp_extent), break_overlong_lines);
            }

            template<
                typename Extent,
                typename ExtentFunc,
                typename Enable = std::enable_if_t<
                    is_detected<does_arithmetic, Extent>::value>>
            auto operator()(
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
            {
                using closure_func_type = decltype(boost::text::bind_back(
                    *this,
                    std::move(max_extent),
                    std::move(cp_extent),
                    std::move(break_overlong_lines)));
                return closure<closure_func_type>(boost::text::bind_back(
                    *this,
                    std::move(max_extent),
                    std::move(cp_extent),
                    std::move(break_overlong_lines)));
            }

            template<typename CPIter, typename Sentinel>
            auto operator()(
                CPIter first, Sentinel last, allowed_breaks_t) const
            {
                return detail::allowed_lines_impl(first, last);
            }

            template<typename CPRange>
            auto operator()(CPRange && r, allowed_breaks_t) const
                -> detail::cp_rng_alg_ret_t<
                    line_break_view<
                        detail::iterator_t<CPRange>,
                        detail::sentinel_t<CPRange>>,
                    CPRange>
            {
                return detail::allowed_lines_cr_impl(r);
            }

            template<typename GraphemeRange>
            auto operator()(GraphemeRange && r, allowed_breaks_t) const
                -> detail::graph_rng_alg_ret_t<
                    line_break_view<
                        detail::gr_rng_cp_iter_t<GraphemeRange>,
                        detail::gr_rng_cp_sent_t<GraphemeRange>,
                        line_break_grapheme_view<
                            detail::gr_rng_cp_iter_t<GraphemeRange>>>,
                    GraphemeRange>
            {
                return detail::allowed_lines_gr_impl(r);
            }

            auto operator()(allowed_breaks_t ab) const
            {
                using closure_func_type =
                    decltype(boost::text::bind_back(*this, std::move(ab)));
                return closure<closure_func_type>(
                    boost::text::bind_back(*this, std::move(ab)));
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::lines_impl lines;
#else
    namespace {
        constexpr dtl::lines_impl lines;
    }
#endif

#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    //[ line_break_concepts
    template<typename T, typename I, typename Extent>
    concept line_break_cp_extent_func = std::invocable<T, I, I> &&
        std::convertible_to<std::invoke_result_t<T, I, I>, Extent>;

    template<typename T>
    concept text_extent = std::same_as<std::remove_cvref_t<T>, T> &&
        std::regular<T> && requires(T t) {
        {t += t} -> std::same_as<T &>;
        {t + t} -> std::convertible_to<T>;
        {t < t} -> std::convertible_to<bool>;
    };
    //]

    template<code_point_iter I, std::sentinel_for<I> S>
    I next_hard_line_break(I first, S last)
    {
        return detail::next_hard_line_break_impl(first, last);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_result<I> next_allowed_line_break(I first, S last)
    {
        return detail::next_allowed_line_break_impl(first, last);
    }

    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> prev_hard_line_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return detail::prev_hard_line_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::borrowed_iterator_t<R> prev_hard_line_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return detail::prev_hard_line_break_gr_impl(r, it);
    }

    template<code_point_range R>
    std::ranges::borrowed_iterator_t<R> next_hard_line_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return detail::next_hard_line_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::borrowed_iterator_t<R> next_hard_line_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        return detail::next_hard_line_break_gr_impl(r, it);
    }

    template<code_point_range R>
    line_break_result<std::ranges::iterator_t<R>> prev_allowed_line_break(
        R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::prev_allowed_line_break_cr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<grapheme_range R>
    auto prev_allowed_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::prev_allowed_line_break_gr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<code_point_range R>
    auto next_allowed_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::next_allowed_line_break_cr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<grapheme_range R>
    auto next_allowed_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::next_allowed_line_break_gr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<code_point_range R>
    bool at_hard_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        return detail::at_hard_line_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    bool at_hard_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        return detail::at_hard_line_break_gr_impl(r, it);
    }

    template<code_point_range R>
    bool at_allowed_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        return detail::at_allowed_line_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    bool at_allowed_line_break(R && r, std::ranges::iterator_t<R> it)
    {
        return detail::at_allowed_line_break_gr_impl(r, it);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    utf_view<format::utf32, I> line(I first, I it, S last)
    {
        return detail::line_impl(first, it, last);
    }

    template<code_point_range R>
    auto line(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::line_cr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<grapheme_range R>
    auto line(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::line_gr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    line_break_cp_view<I> allowed_line(I first, I it, S last)
    {
        return detail::allowed_line_impl(first, it, last);
    }

    template<code_point_range R>
    auto allowed_line(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::allowed_line_cr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    template<grapheme_range R>
    auto allowed_line(R && r, std::ranges::iterator_t<R> it)
    {
        if constexpr (std::ranges::borrowed_range<R>)
            return detail::allowed_line_gr_impl(r, it);
        else
            return std::ranges::dangling{};
    }

    namespace dtl {
        template<typename I, typename S>
        auto make_utf32_iter_for(I i, S s)
        {
            if constexpr (utf32_iter<I>) {
                return i;
            } else if constexpr (std::is_pointer_v<
                                     std::remove_reference_t<I>>) {
                return boost::text::as_utf32(i, null_sentinel).begin();
            } else if constexpr (std::ranges::range<I>) {
                return std::ranges::begin(i);
            } else {
                return std::ranges::begin(boost::text::as_utf32(i, s));
            }
        }

        template<typename I, typename S = I>
        using utf32_iter_for =
            decltype(make_utf32_iter_for(declval<I>(), declval<S>()));

        struct lines_impl : range_adaptor_closure<lines_impl>
        {
            template<utf_iter I, std::sentinel_for<I> S>
            auto operator()(I first, S last) const
            {
                if constexpr (utf32_iter<I>) {
                    return detail::breaks_impl<
                        detail::prev_hard_line_break_callable,
                        detail::next_hard_line_break_callable>(first, last);
                } else {
                    auto r = boost::text::as_utf32(first, last);
                    return detail::breaks_impl<
                        detail::prev_hard_line_break_callable,
                        detail::next_hard_line_break_callable>(
                        r.begin(), r.end());
                }
            }

            template<utf_range_like R>
            auto operator()(R && r) const
            {
                if constexpr (
                    !std::is_pointer_v<remove_reference_t<R>> &&
                    !std::ranges::borrowed_range<R>) {
                    return std::ranges::dangling{};
                } else if constexpr (std::is_pointer_v<remove_reference_t<R>>) {
                    return (*this)(r, null_sentinel);
                } else {
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                }
            }

            template<grapheme_range R>
            auto operator()(R && r) const
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::breaks_gr_impl<
                        detail::prev_hard_line_break_callable,
                        detail::next_hard_line_break_callable>(r);
                } else {
                    return std::ranges::dangling{};
                }
            }

            template<
                utf_iter I,
                std::sentinel_for<I> S,
                text_extent Extent,
                line_break_cp_extent_func<utf32_iter_for<I, S>, Extent>
                    ExtentFunc>
            auto operator()(
                I first,
                S last,
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
            {
                if constexpr (utf32_iter<I>) {
                    return detail::lines_impl(
                        first,
                        last,
                        max_extent,
                        std::move(cp_extent),
                        break_overlong_lines);
                } else {
                    auto r = boost::text::as_utf32(first, last);
                    return detail::lines_impl(
                        r.begin(),
                        r.end(),
                        max_extent,
                        std::move(cp_extent),
                        break_overlong_lines);
                }
            }

            template<
                utf_range_like R,
                text_extent Extent,
                line_break_cp_extent_func<utf32_iter_for<R>, Extent> ExtentFunc>
            auto operator()(
                R && r,
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::lines_cr_impl(
                        r,
                        max_extent,
                        std::move(cp_extent),
                        break_overlong_lines);
                } else {
                    return std::ranges::dangling{};
                }
            }

            template<
                grapheme_range R,
                text_extent Extent,
                line_break_cp_extent_func<code_point_iterator_t<R>, Extent>
                    ExtentFunc>
            auto operator()(
                R && r,
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::lines_gr_impl(
                        r,
                        max_extent,
                        std::move(cp_extent),
                        break_overlong_lines);
                } else {
                    return std::ranges::dangling{};
                }
            }

            template<text_extent Extent, typename ExtentFunc>
            auto operator()(
                Extent max_extent,
                ExtentFunc cp_extent,
                bool break_overlong_lines = true) const
            {
                return closure(boost::text::bind_back(
                    *this,
                    std::move(max_extent),
                    std::move(cp_extent),
                    std::move(break_overlong_lines)));
            }

            template<utf_iter I, std::sentinel_for<I> S>
            auto operator()(I first, S last, allowed_breaks_t) const
            {
                auto r = boost::text::as_utf32(first, last);
                return detail::allowed_lines_impl(r.begin(), r.end());
            }

            template<utf_range_like R>
            auto operator()(R && r, allowed_breaks_t ab) const
            {
                if constexpr (
                    !std::is_pointer_v<remove_reference_t<R>> &&
                    !std::ranges::borrowed_range<R>) {
                    return std::ranges::dangling{};
                } else if constexpr (std::is_pointer_v<remove_reference_t<R>>) {
                    return (*this)(r, null_sentinel);
                } else {
                    return (*this)(
                        std::ranges::begin(r), std::ranges::end(r), ab);
                }
            }

            template<grapheme_range R>
            auto operator()(R && r, allowed_breaks_t) const
            {
                if constexpr (std::ranges::borrowed_range<R>)
                    return detail::allowed_lines_gr_impl(r);
                else
                    return std::ranges::dangling{};
            }

            auto operator()(allowed_breaks_t ab) const
            {
                return closure(boost::text::bind_back(*this, std::move(ab)));
            }
        };
    }

    inline constexpr dtl::lines_impl lines;

}}}

namespace std::ranges {
    template<boost::text::code_point_iter I>
    inline constexpr bool
        enable_borrowed_range<boost::text::line_break_cp_view<I>> = true;

    template<boost::text::code_point_iter I>
    inline constexpr bool
        enable_borrowed_range<boost::text::line_break_grapheme_view<I>> = true;

    template<
        typename CPIter,
        typename CPSentinel,
        typename Extent,
        typename ExtentFunc,
        typename Subrange>
    inline constexpr bool
        enable_borrowed_range<boost::text::forward_line_break_view<
            CPIter,
            CPSentinel,
            Extent,
            ExtentFunc,
            Subrange>> = true;

    template<typename CPIter, typename CPSentinel, typename Subrange>
    inline constexpr bool enable_borrowed_range<
        boost::text::line_break_view<CPIter, CPSentinel, Subrange>> = true;
}

#endif

#endif
