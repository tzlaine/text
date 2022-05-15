// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_SENTENCE_BREAK_HPP
#define BOOST_TEXT_SENTENCE_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/detail/breaks_impl.hpp>

#include <boost/assert.hpp>

#include <array>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text {

    /** The sentence properties defined by Unicode. */
    enum class sentence_property {
        Other,
        CR,
        LF,
        Sep,
        Sp,
        Lower,
        Upper,
        OLetter,
        Numeric,
        ATerm,
        STerm,
        Close,
        SContinue,
        Format,
        Extend
    };

    namespace detail {
        struct sentence_prop_interval
        {
            uint32_t lo_;
            uint32_t hi_;
            sentence_property prop_;
        };

        inline bool operator<(
            sentence_prop_interval lhs, sentence_prop_interval rhs) noexcept
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<sentence_prop_interval, 28> const &
        make_sentence_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, sentence_property>
        make_sentence_prop_map();
    }

    /** Returns the sentence property associated with code point `cp`. */
    inline sentence_property sentence_prop(uint32_t cp) noexcept
    {
        static auto const map = detail::make_sentence_prop_map();
        static auto const intervals = detail::make_sentence_prop_intervals();

        auto const it = map.find(cp);
        if (it == map.end()) {
            auto const it2 = std::lower_bound(
                intervals.begin(),
                intervals.end(),
                detail::sentence_prop_interval{cp, cp + 1});
            if (it2 == intervals.end() || cp < it2->lo_ || it2->hi_ <= cp)
                return sentence_property::Other;
            return it2->prop_;
        }
        return it->second;
    }

    namespace detail {
        inline bool skippable(sentence_property prop) noexcept
        {
            return prop == sentence_property::Extend ||
                   prop == sentence_property::Format;
        }

        inline bool para_sep(sentence_property prop) noexcept
        {
            return prop == sentence_property::Sep ||
                   prop == sentence_property::CR ||
                   prop == sentence_property::LF;
        }

        inline bool sa_term(sentence_property prop) noexcept
        {
            return prop == sentence_property::STerm ||
                   prop == sentence_property::ATerm;
        }

        inline bool sb8_not(sentence_property prop) noexcept
        {
            return prop != sentence_property::OLetter &&
                   prop != sentence_property::Upper &&
                   prop != sentence_property::Lower &&
                   !detail::para_sep(prop) && !detail::sa_term(prop);
        }

        template<typename CPIter, typename Func>
        bool before_close_sp(CPIter it_, CPIter first, bool skip_sps, Func func)
        {
            stl_interfaces::reverse_iterator<CPIter> it(std::next(it_));
            stl_interfaces::reverse_iterator<CPIter> last(first);
            it = std::find_if_not(std::next(it), last, [](uint32_t cp) {
                return detail::skippable(boost::text::sentence_prop(cp));
            });
            if (skip_sps) {
                while (it != last && boost::text::sentence_prop(*it) ==
                                         sentence_property::Sp) {
                    it = std::find_if_not(std::next(it), last, [](uint32_t cp) {
                        return detail::skippable(
                            boost::text::sentence_prop(cp));
                    });
                }
            }
            while (it != last && boost::text::sentence_prop(*it) ==
                                     sentence_property::Close) {
                it = std::find_if_not(std::next(it), last, [](uint32_t cp) {
                    return detail::skippable(boost::text::sentence_prop(cp));
                });
            }
            if (it == last)
                return false;
            return func(boost::text::sentence_prop(*it));
        }

        template<typename CPIter>
        struct sentence_break_state
        {
            CPIter it;
            bool it_points_to_prev = false;

            sentence_property prev_prev_prop;
            sentence_property prev_prop;
            sentence_property prop;
            sentence_property next_prop;
            sentence_property next_next_prop;
        };

        template<typename CPIter>
        sentence_break_state<CPIter> next(sentence_break_state<CPIter> state)
        {
            ++state.it;
            state.prev_prev_prop = state.prev_prop;
            state.prev_prop = state.prop;
            state.prop = state.next_prop;
            state.next_prop = state.next_next_prop;
            return state;
        }

        template<typename CPIter>
        sentence_break_state<CPIter> prev(sentence_break_state<CPIter> state)
        {
            if (!state.it_points_to_prev)
                --state.it;
            state.it_points_to_prev = false;
            state.next_next_prop = state.next_prop;
            state.next_prop = state.prop;
            state.prop = state.prev_prop;
            state.prev_prop = state.prev_prev_prop;
            return state;
        }

        // SB5: Except after line breaks, ignore/skip (Extend | Format)*
        template<typename CPIter, typename Sentinel>
        sentence_break_state<CPIter> skip_forward(
            sentence_break_state<CPIter> state, CPIter first, Sentinel last)
        {
            if (state.it != first && !detail::skippable(state.prev_prop) &&
                detail::skippable(state.prop)) {
                auto temp_it = boost::text::find_if_not(
                    std::next(state.it), last, [](uint32_t cp) {
                        return detail::skippable(
                            boost::text::sentence_prop(cp));
                    });
                if (temp_it == last) {
                    state.it = temp_it;
                } else {
                    auto const temp_prop = boost::text::sentence_prop(*temp_it);
                    state.it = temp_it;
                    state.prop = temp_prop;
                    state.next_prop = sentence_property::Other;
                    state.next_next_prop = sentence_property::Other;
                    if (std::next(state.it) != last) {
                        state.next_prop =
                            boost::text::sentence_prop(*std::next(state.it));
                        if (std::next(state.it, 2) != last) {
                            state.next_next_prop = boost::text::sentence_prop(
                                *std::next(state.it, 2));
                        }
                    }
                }
            }
            return state;
        }

        inline bool table_sentence_break(
            sentence_property lhs, sentence_property rhs) noexcept
        {
            // clang-format off
// See chart at http://www.unicode.org/Public/10.0.0/ucd/auxiliary/SentenceBreakTest.html
constexpr std::array<std::array<bool, 15>, 15> sentence_breaks = {{
//  Other CR LF Sep Sp Lwr Upr OLet Num ATrm STrm Cls SCont Fmt Ext
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Other
    {{1,   1, 0, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}}, // CR
    {{1,   1, 1, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}}, // LF

    {{1,   1, 1, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}}, // Sep
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Sp
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Lower

    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Upper
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // OLetter
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Number

    {{1,   0, 0, 0,  0, 0,  1,  1,   0,  0,   0,   0,  0,    0,  0}}, // ATerm
    {{1,   0, 0, 0,  0, 1,  1,  1,   1,  0,   0,   0,  0,    0,  0}}, // STerm
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Close

    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // SContinue
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Format
    {{0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}}, // Extend
}};
            // clang-format on
            auto const lhs_int = static_cast<int>(lhs);
            auto const rhs_int = static_cast<int>(rhs);
            return sentence_breaks[lhs_int][rhs_int];
        }

        template<typename CPIter, typename Sentinel>
        CPIter prev_sentence_break_impl(
            CPIter first, CPIter it, Sentinel last) noexcept
        {
            if (it == first)
                return it;

            if (it == last && --it == first)
                return it;

            detail::sentence_break_state<CPIter> state;

            state.it = it;

            state.prop = boost::text::sentence_prop(*state.it);

            // Special case: If state.prop is skippable, we need to skip
            // backward until we find a non-skippable.
            if (detail::skippable(state.prop)) {
                auto const prev = boost::text::find_if_not_backward(
                    first, state.it, [](uint32_t cp) {
                        return detail::skippable(
                            boost::text::sentence_prop(cp));
                    });
                if (prev != state.it) {
                    state.it = prev;
                    state.next_prop =
                        boost::text::sentence_prop(*std::next(state.it));
                    state.prop = boost::text::sentence_prop(*state.it);

                    // If we end up on a non-skippable that should break before
                    // the skippable(s) we just moved over, break on the last
                    // skippable.
                    if (!detail::skippable(state.prop) &&
                        detail::table_sentence_break(
                            state.prop, state.next_prop)) {
                        return ++state.it;
                    }
                    if (state.it == first)
                        return first;
                }
            }

            state.prev_prev_prop = sentence_property::Other;
            if (std::prev(state.it) != first)
                state.prev_prev_prop =
                    boost::text::sentence_prop(*std::prev(state.it, 2));
            state.prev_prop = boost::text::sentence_prop(*std::prev(state.it));
            state.next_prop = sentence_property::Other;
            state.next_next_prop = sentence_property::Other;
            if (std::next(state.it) != last) {
                state.next_prop =
                    boost::text::sentence_prop(*std::next(state.it));
                if (std::next(state.it, 2) != last)
                    state.next_next_prop =
                        boost::text::sentence_prop(*std::next(state.it, 2));
            }

            // Since 'it' may be anywhere within the sentence in which it sits,
            // we need to look forward to make sure that next_prop and
            // next_next_prop don't point to skippables.
            {
                if (std::next(state.it) != last) {
                    auto temp_state = state;
                    temp_state = detail::next(temp_state);
                    temp_state = detail::skip_forward(temp_state, first, last);
                    if (temp_state.it == last) {
                        state.next_prop = sentence_property::Other;
                        state.next_next_prop = sentence_property::Other;
                    } else {
                        state.next_prop = temp_state.prop;
                        if (std::next(temp_state.it) != last) {
                            temp_state = detail::next(temp_state);
                            temp_state =
                                detail::skip_forward(temp_state, first, last);
                            if (temp_state.it == last)
                                state.next_next_prop = sentence_property::Other;
                            else
                                state.next_next_prop = temp_state.prop;
                        }
                    }
                }
            }

            // SB5: Except after ParaSep, ignore/skip (Extend | Format)*
            auto skip = [](detail::sentence_break_state<CPIter> state,
                           CPIter first) {
                if (detail::skippable(state.prev_prop)) {
                    auto temp_it = boost::text::find_if_not_backward(
                        first, state.it, [](uint32_t cp) {
                            return detail::skippable(
                                boost::text::sentence_prop(cp));
                        });

                    if (temp_it == state.it)
                        return state;
                    auto temp_prev_prop = boost::text::sentence_prop(*temp_it);
                    if (!detail::para_sep(temp_prev_prop)) {
                        state.it = temp_it;
                        state.it_points_to_prev = true;
                        state.prev_prop = temp_prev_prop;
                        if (temp_it == first)
                            state.prev_prev_prop = sentence_property::Other;
                        else
                            state.prev_prev_prop =
                                boost::text::sentence_prop(*std::prev(temp_it));
                    }
                }
                return state;
            };

            for (; state.it != first; state = detail::prev(state)) {
                if (std::prev(state.it) != first)
                    state.prev_prev_prop =
                        boost::text::sentence_prop(*std::prev(state.it, 2));
                else
                    state.prev_prev_prop = sentence_property::Other;

                // SB3
                if (state.prev_prop == sentence_property::CR &&
                    state.prop == sentence_property::LF) {
                    continue;
                }

                // SB4
                if (detail::para_sep(state.prev_prop))
                    return state.it;

                // Puting this here means not having to do it explicitly below
                // between prop and next_prop (and transitively, between
                // prev_prop and prop).
                auto const after_skip_it = state.it;
                state = skip(state, first);
                if (state.it == last)
                    return state.it;

                // SB6
                if (state.prev_prop == sentence_property::ATerm &&
                    state.prop == sentence_property::Numeric) {
                    continue;
                }

                // SB7
                if (state.prev_prop == sentence_property::ATerm &&
                    state.prop == sentence_property::Upper) {
                    auto const temp_state = skip(detail::prev(state), first);
                    if (temp_state.prev_prop == sentence_property::Upper ||
                        temp_state.prev_prop == sentence_property::Lower)
                        continue;
                }

                // SB8
                if ((state.prev_prop == sentence_property::ATerm ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (detail::sb8_not(state.prop) ||
                     state.prop == sentence_property::Lower)) {
                    bool const aterm = detail::before_close_sp(
                        after_skip_it, first, true, [](sentence_property prop) {
                            return prop == sentence_property::ATerm;
                        });
                    if (aterm) {
                        auto it = after_skip_it;
                        while (
                            it != last &&
                            detail::sb8_not(boost::text::sentence_prop(*it))) {
                            it = boost::text::find_if_not(
                                std::next(it), last, [](uint32_t cp) {
                                    return detail::skippable(
                                        boost::text::sentence_prop(cp));
                                });
                        }
                        if (it != last && boost::text::sentence_prop(*it) ==
                                              sentence_property::Lower) {
                            continue;
                        }
                    }
                }

                // SB8a
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (state.prop == sentence_property::SContinue ||
                     detail::sa_term(state.prop))) {
                    if (detail::before_close_sp(
                            after_skip_it,
                            first,
                            true,
                            [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB9
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close) &&
                    (state.prop == sentence_property::Close ||
                     state.prop == sentence_property::Sp ||
                     detail::para_sep(state.prop))) {
                    if (detail::before_close_sp(
                            after_skip_it,
                            first,
                            false,
                            [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB10
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (state.prop == sentence_property::Sp ||
                     detail::para_sep(state.prop))) {
                    if (detail::before_close_sp(
                            after_skip_it,
                            first,
                            true,
                            [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB11
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp ||
                     detail::para_sep(state.prev_prop)) &&
                    !detail::skippable(state.prop)) {
                    auto it = after_skip_it;
                    if (detail::para_sep(state.prev_prop))
                        --it;
                    if (it != first &&
                        detail::before_close_sp(
                            it, first, true, [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        return after_skip_it;
                    }
                }
            }

            return first;
        }

        template<typename CPIter, typename Sentinel>
        CPIter next_sentence_break_impl(CPIter first, Sentinel last) noexcept
        {
            if (first == last)
                return first;

            detail::sentence_break_state<CPIter> state;
            state.it = first;

            if (++state.it == last)
                return state.it;

            state.prev_prev_prop = sentence_property::Other;
            state.prev_prop = boost::text::sentence_prop(*first);
            state.prop = boost::text::sentence_prop(*state.it);
            state.next_prop = sentence_property::Other;
            state.next_next_prop = sentence_property::Other;
            if (std::next(state.it) != last) {
                state.next_prop =
                    boost::text::sentence_prop(*std::next(state.it));
                if (std::next(state.it, 2) != last)
                    state.next_next_prop =
                        boost::text::sentence_prop(*std::next(state.it, 2));
            }

            for (; state.it != last; state = detail::next(state)) {
                if (std::next(state.it) != last &&
                    std::next(state.it, 2) != last)
                    state.next_next_prop =
                        boost::text::sentence_prop(*std::next(state.it, 2));
                else
                    state.next_next_prop = sentence_property::Other;

                // SB3
                if (state.prev_prop == sentence_property::CR &&
                    state.prop == sentence_property::LF) {
                    continue;
                }

                // SB4
                if (detail::para_sep(state.prev_prop))
                    return state.it;

                // Puting this here means not having to do it explicitly below
                // between prop and next_prop (and transitively, between
                // prev_prop and prop).
                state = detail::skip_forward(state, first, last);
                if (state.it == last)
                    return state.it;

                // SB6
                if (state.prev_prop == sentence_property::ATerm &&
                    state.prop == sentence_property::Numeric) {
                    continue;
                }

                // SB7
                if ((state.prev_prev_prop == sentence_property::Upper ||
                     state.prev_prev_prop == sentence_property::Lower) &&
                    state.prev_prop == sentence_property::ATerm &&
                    state.prop == sentence_property::Upper) {
                    continue;
                }

                // SB8
                if ((state.prev_prop == sentence_property::ATerm ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (detail::sb8_not(state.prop) ||
                     state.prop == sentence_property::Lower)) {
                    bool const aterm = detail::before_close_sp(
                        state.it, first, true, [](sentence_property prop) {
                            return prop == sentence_property::ATerm;
                        });
                    if (aterm) {
                        auto it = state.it;
                        while (
                            it != last &&
                            detail::sb8_not(boost::text::sentence_prop(*it))) {
                            it = boost::text::find_if_not(
                                std::next(it), last, [](uint32_t cp) {
                                    return detail::skippable(
                                        boost::text::sentence_prop(cp));
                                });
                        }
                        if (it != last && boost::text::sentence_prop(*it) ==
                                              sentence_property::Lower) {
                            continue;
                        }
                    }
                }

                // SB8a
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (state.prop == sentence_property::SContinue ||
                     detail::sa_term(state.prop))) {
                    if (detail::before_close_sp(
                            state.it, first, true, [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB9
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close) &&
                    (state.prop == sentence_property::Close ||
                     state.prop == sentence_property::Sp ||
                     detail::para_sep(state.prop))) {
                    if (detail::before_close_sp(
                            state.it, first, false, [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB10
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp) &&
                    (state.prop == sentence_property::Sp ||
                     detail::para_sep(state.prop))) {
                    if (detail::before_close_sp(
                            state.it, first, true, [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        continue;
                    }
                }

                // SB11
                if ((detail::sa_term(state.prev_prop) ||
                     state.prev_prop == sentence_property::Close ||
                     state.prev_prop == sentence_property::Sp ||
                     detail::para_sep(state.prev_prop)) &&
                    !detail::skippable(state.prop)) {
                    auto it = state.it;
                    if (detail::para_sep(state.prev_prop))
                        --it;
                    if (it != first &&
                        detail::before_close_sp(
                            it, first, true, [](sentence_property prop) {
                                return detail::sa_term(prop);
                            })) {
                        return state.it;
                    }
                }
            }
            return state.it;
        }

        template<typename CPIter, typename Sentinel>
        struct next_sentence_callable
        {
            auto operator()(CPIter it, Sentinel last) noexcept
                -> cp_iter_ret_t<CPIter, CPIter>
            {
                return detail::next_sentence_break_impl(it, last);
            }
        };

        template<typename CPIter>
        struct prev_sentence_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last) noexcept
                -> cp_iter_ret_t<CPIter, CPIter>
            {
                return detail::prev_sentence_break_impl(first, it, last);
            }
        };

        template<typename CPRange, typename CPIter>
        iterator_t<CPRange>
        prev_sentence_break_cp_impl(CPRange && range, CPIter it) noexcept
        {
            return detail::prev_sentence_break_impl(
                detail::begin(range), it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        iterator_t<GraphemeRange> prev_sentence_break_gr_impl(
            GraphemeRange && range, GraphemeIter it) noexcept
        {
            using cp_iter_t = decltype(range.begin().base());
            return {
                range.begin().base(),
                detail::prev_sentence_break_impl(
                    range.begin().base(),
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base()),
                range.end().base()};
        }

        template<typename CPRange, typename CPIter>
        iterator_t<CPRange>
        next_sentence_break_cp_impl(CPRange && range, CPIter it) noexcept
        {
            return detail::next_sentence_break_impl(it, detail::end(range));
        }

        template<typename GraphemeRange, typename GraphemeIter>
        iterator_t<GraphemeRange> next_sentence_break_gr_impl(
            GraphemeRange && range, GraphemeIter it) noexcept
        {
            using cp_iter_t = decltype(range.begin().base());
            return {
                range.begin().base(),
                detail::next_sentence_break_impl(
                    static_cast<cp_iter_t>(it.base()), range.end().base()),
                range.end().base()};
        }

        template<typename CPIter, typename Sentinel>
        bool
        at_sentence_break_impl(CPIter first, CPIter it, Sentinel last) noexcept
        {
            if (it == last)
                return true;
            return detail::prev_sentence_break_impl(first, it, last) == it;
        }

        template<typename CPRange, typename CPIter>
        bool at_sentence_break_cp_impl(CPRange && range, CPIter it) noexcept
        {
            if (it == detail::end(range))
                return true;
            return detail::prev_sentence_break_impl(
                       detail::begin(range), it, detail::end(range)) == it;
        }

        template<typename GraphemeRange, typename GraphemeIter>
        bool at_sentence_break_gr_impl(
            GraphemeRange && range, GraphemeIter it) noexcept
        {
            if (it == detail::end(range))
                return true;
            using cp_iter_t = decltype(range.begin().base());
            cp_iter_t it_ = static_cast<cp_iter_t>(it.base());
            return detail::prev_sentence_break_impl(
                       range.begin().base(), it_, range.end().base()) == it_;
        }

        template<typename CPIter, typename Sentinel>
        utf32_view<CPIter>
        sentence_impl(CPIter first, CPIter it, Sentinel last) noexcept
        {
            first = detail::prev_sentence_break_impl(first, it, last);
            return utf32_view<CPIter>{
                first, detail::next_sentence_break_impl(first, last)};
        }

        template<typename CPRange, typename CPIter>
        utf32_view<iterator_t<CPRange>>
        sentence_cr_impl(CPRange && range, CPIter it) noexcept
        {
            auto first = detail::prev_sentence_break_impl(
                detail::begin(range), it, detail::end(range));
            return utf32_view<iterator_t<CPRange>>{
                first,
                detail::next_sentence_break_impl(first, detail::end(range))};
        }

        template<typename GraphemeRange, typename GraphemeIter>
        auto sentence_gr_impl(GraphemeRange && range, GraphemeIter it) noexcept
            -> grapheme_view<decltype(range.begin().base())>
        {
            using cp_iter_t = decltype(range.begin().base());
            auto first = detail::prev_sentence_break_impl(
                range.begin().base(),
                static_cast<cp_iter_t>(it.base()),
                range.end().base());
            return {
                range.begin().base(),
                first,
                detail::next_sentence_break_impl(first, range.end().base()),
                range.end().base()};
        }
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest sentence break at or before before `it`.  If `it ==
        first`, that is returned.  Otherwise, the first code point of the
        sentence that `it` is within is returned (even if `it` is already at
        the first code point of a sentence). */
    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_sentence_break(I first, I it, S last) noexcept;

    /** Finds the next sentence break after `first`.  This will be the first
        code point after the current sentence, or `last` if no next sentence
        exists.

        \pre `first` is at the beginning of a sentence. */
    template<code_point_iter I, std::sentinel_for<I> S>
    I next_sentence_break(I first, S last) noexcept;

    /** Finds the nearest sentence break at or before before `it`.  If `it ==
        r.begin()`, that is returned.  Otherwise, the first code point of
        the sentence that `it` is within is returned (even if `it` is already
        at the first code point of a sentence). */
    template<code_point_range R>
    std::ranges::iterator_t<R> prev_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns a grapheme_iterator to the nearest sentence break at or before
        before `it`.  If `it == r.begin()`, that is returned.  Otherwise, the
        first grapheme of the sentence that `it` is within is returned (even
        if `it` is already at the first grapheme of a sentence). */
    template<grapheme_range R>
    std::ranges::iterator_t<R> prev_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Finds the next sentence break after `it`.  This will be the first code
        point after the current sentence, or `r.end()` if no next sentence
        exists.

        \pre `it` is at the beginning of a sentence. */
    template<code_point_range R>
    std::ranges::iterator_t<R> next_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns a grapheme_iterator to the next sentence break after `it`.
        This will be the first grapheme after the current sentence, or
        `r.end()` if no next sentence exists.

        \pre `it` is at the beginning of a sentence. */
    template<grapheme_range R>
    std::ranges::iterator_t<R> next_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns true iff `it` is at the beginning of a sentence, or `it ==
        last`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_sentence_break(I first, I it, S last) noexcept;

    /** Returns true iff `it` is at the beginning of a sentence, or `it ==
        std::ranges::end(r)`. */
    template<code_point_range R>
    bool at_sentence_break(R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns true iff `it` is at the beginning of a sentence, or `it ==
        std::ranges::end(r)`. */
    template<grapheme_range R>
    bool at_sentence_break(R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns the bounds of the sentence that `it` lies within. */
    template<code_point_iter I, std::sentinel_for<I> S>
    utf32_view<I> sentence(I first, I it, S last) noexcept;

    /** Returns the bounds of the sentence that `it` lies within, as a
        utf32_view. */
    template<code_point_range R>
    utf32_view<std::ranges::iterator_t<R>> sentence(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns grapheme range delimiting the bounds of the sentence that `it`
        lies within, as a grapheme_view. */
    template<grapheme_range R>
    grapheme_view<code_point_iterator_t<R>> sentence(
        R && r, std::ranges::iterator_t<R> it) noexcept;

    /** Returns a view of the code point ranges delimiting sentences in
        `[first, last)`. */
    template<code_point_iter I, std::sentinel_for<I> S>
    detail::unspecified sentences(I first, S last) noexcept;

    /** Returns a view of the code point ranges delimiting sentences in `r`.
        The result is returned as a `borrowed_view_t` in C++20 and later. */
    template<code_point_range R>
    detail::unspecified sentences(R && r) noexcept;

    /** Returns a view of the grapheme ranges delimiting sentences in `r`.
        The result is returned as a `borrowed_view_t` in C++20 and later. */
    template<grapheme_range R>
    detail::unspecified sentences(R && r) noexcept;

#else

    template<typename CPIter, typename Sentinel>
    auto prev_sentence_break(CPIter first, CPIter it, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::prev_sentence_break_impl(first, it, last);
    }

    template<typename CPIter, typename Sentinel>
    auto next_sentence_break(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::next_sentence_break_impl(first, last);
    }

    template<typename CPRange, typename CPIter>
    auto prev_sentence_break(CPRange && range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::prev_sentence_break_cp_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto prev_sentence_break(
        GraphemeRange && range, GraphemeIter it) noexcept->detail::
        graph_rng_alg_ret_t<detail::iterator_t<GraphemeRange>, GraphemeRange>
    {
        return detail::prev_sentence_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto next_sentence_break(CPRange && range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::next_sentence_break_cp_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto next_sentence_break(
        GraphemeRange && range, GraphemeIter it) noexcept->detail::
        graph_rng_alg_ret_t<detail::iterator_t<GraphemeRange>, GraphemeRange>
    {
        return detail::next_sentence_break_gr_impl(range, it);
    }

    template<typename CPIter, typename Sentinel>
    auto at_sentence_break(CPIter first, CPIter it, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::at_sentence_break_impl(first, it, last);
    }

    template<typename CPRange, typename CPIter>
    auto at_sentence_break(CPRange && range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return detail::at_sentence_break_cp_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto
    at_sentence_break(GraphemeRange && range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return detail::at_sentence_break_gr_impl(range, it);
    }

    template<typename CPIter, typename Sentinel>
    utf32_view<CPIter> sentence(CPIter first, CPIter it, Sentinel last) noexcept
    {
        return detail::sentence_impl(first, it, last);
    }

    template<typename CPRange, typename CPIter>
    auto sentence(CPRange && range, CPIter it) noexcept -> detail::
        cp_rng_alg_ret_t<utf32_view<detail::iterator_t<CPRange>>, CPRange>
    {
        return detail::sentence_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto sentence(GraphemeRange && range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            grapheme_view<decltype(range.begin().base())>,
            GraphemeRange>
    {
        return detail::sentence_gr_impl(range, it);
    }

    namespace dtl {
        struct sentences_impl : range_adaptor_closure<sentences_impl>
        {
            template<typename CPIter, typename Sentinel>
            auto operator()(CPIter first, Sentinel last) const noexcept
                -> decltype(detail::breaks_impl<
                            detail::prev_sentence_callable,
                            detail::next_sentence_callable>(first, last))
            {
                return detail::breaks_impl<
                    detail::prev_sentence_callable,
                    detail::next_sentence_callable>(first, last);
            }

            template<typename CPRange>
            auto operator()(CPRange && range) const noexcept
                -> detail::cp_rng_alg_ret_t<
                    decltype(detail::breaks_cr_impl<
                             detail::prev_sentence_callable,
                             detail::next_sentence_callable>(range)),
                    CPRange>
            {
                return detail::breaks_cr_impl<
                    detail::prev_sentence_callable,
                    detail::next_sentence_callable>(range);
            }

            template<typename GraphemeRange>
            auto operator()(GraphemeRange && range) const noexcept
                -> detail::graph_rng_alg_ret_t<
                    decltype(detail::breaks_gr_impl<
                             detail::prev_sentence_callable,
                             detail::next_sentence_callable>(range)),
                    GraphemeRange>
            {
                return detail::breaks_gr_impl<
                    detail::prev_sentence_callable,
                    detail::next_sentence_callable>(range);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::sentences_impl sentences;
#else
    namespace {
        constexpr dtl::sentences_impl sentences;
    }
#endif

#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_sentence_break(I first, I it, S last) noexcept
    {
        return detail::prev_sentence_break_impl(first, it, last);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    I next_sentence_break(I first, S last) noexcept
    {
        return detail::next_sentence_break_impl(first, last);
    }

    template<code_point_range R>
    std::ranges::iterator_t<R> prev_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::prev_sentence_break_cp_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::iterator_t<R> prev_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::prev_sentence_break_gr_impl(r, it);
    }

    template<code_point_range R>
    std::ranges::iterator_t<R> next_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::next_sentence_break_cp_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::iterator_t<R> next_sentence_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::next_sentence_break_gr_impl(r, it);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_sentence_break(I first, I it, S last) noexcept
    {
        return detail::at_sentence_break_impl(first, it, last);
    }

    template<code_point_range R>
    bool at_sentence_break(R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::at_sentence_break_cp_impl(r, it);
    }

    template<grapheme_range R>
    bool at_sentence_break(R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::at_sentence_break_gr_impl(r, it);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    utf32_view<I> sentence(I first, I it, S last) noexcept
    {
        first = boost::text::prev_sentence_break(first, it, last);
        return utf32_view<I>{
            first, boost::text::next_sentence_break(first, last)};
    }

    template<code_point_range R>
    utf32_view<std::ranges::iterator_t<R>> sentence(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::sentence_cr_impl(r, it);
    }

    template<grapheme_range R>
    grapheme_view<code_point_iterator_t<R>> sentence(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::sentence_gr_impl(r, it);
    }

    namespace dtl {
        struct sentences_impl : range_adaptor_closure<sentences_impl>
        {
            template<code_point_iter I, std::sentinel_for<I> S>
            auto operator()(I first, S last) const noexcept
            {
                return detail::breaks_impl<
                    detail::prev_sentence_callable,
                    detail::next_sentence_callable>(first, last);
            }

            template<code_point_range R>
            auto operator()(R && r) const noexcept
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::breaks_cr_impl<
                        detail::prev_sentence_callable,
                        detail::next_sentence_callable>(r);
                } else {
                    return std::ranges::dangling{};
                }
            }

            template<grapheme_range R>
            auto operator()(R && r) const noexcept
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::breaks_gr_impl<
                        detail::prev_sentence_callable,
                        detail::next_sentence_callable>(r);
                } else {
                    return std::ranges::dangling{};
                }
            }
        };
    }

    inline constexpr dtl::sentences_impl sentences;

}}}

#endif

#endif
