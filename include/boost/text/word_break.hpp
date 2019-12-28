#ifndef BOOST_TEXT_WORD_BREAK_HPP
#define BOOST_TEXT_WORD_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/lazy_segment_range.hpp>

#include <array>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text { inline namespace v1 {

    /** The word properties defined by Unicode. */
    enum class word_property {
        Other,
        CR,
        LF,
        Newline,
        Katakana,
        ALetter,
        MidLetter,
        MidNum,
        MidNumLet,
        Numeric,
        ExtendNumLet,
        Regional_Indicator,
        Hebrew_Letter,
        Double_Quote,
        Single_Quote,
        ExtPict,
        WSegSpace,
        Format,
        Extend,
        ZWJ
    };

    namespace detail {
        struct word_prop_interval
        {
            uint32_t lo_;
            uint32_t hi_;
            word_property prop_;
        };

        inline bool
        operator<(word_prop_interval lhs, word_prop_interval rhs) noexcept
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<word_prop_interval, 24> const &
        make_word_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, word_property>
        make_word_prop_map();
    }

    /** Returns the word property associated with code point `cp`. */
    inline word_property word_prop(uint32_t cp) noexcept
    {
        static auto const map = detail::make_word_prop_map();
        static auto const intervals = detail::make_word_prop_intervals();

        auto const it = map.find(cp);
        if (it == map.end()) {
            auto const it2 = std::lower_bound(
                intervals.begin(),
                intervals.end(),
                detail::word_prop_interval{cp, cp + 1});
            if (it2 == intervals.end() || cp < it2->lo_ || it2->hi_ <= cp)
                return word_property::Other;
            return it2->prop_;
        }
        return it->second;
    }

    namespace detail {
        struct default_cp_break
        {
            bool
            operator()(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) const
                noexcept
            {
                return false;
            }
        };

        inline bool skippable(word_property prop) noexcept
        {
            return prop == word_property::Extend ||
                   prop == word_property::Format || prop == word_property::ZWJ;
        }

        inline bool linebreak(word_property prop) noexcept
        {
            return prop == word_property::CR || prop == word_property::LF ||
                   prop == word_property::Newline;
        }

        inline bool ah_letter(word_property prop) noexcept
        {
            return prop == word_property::ALetter ||
                   prop == word_property::Hebrew_Letter;
        }

        // Corresonds to (MidLetter | MidNumLetQ) in WB6 and WB7
        inline bool mid_ah(word_property prop) noexcept
        {
            return prop == word_property::MidLetter ||
                   prop == word_property::MidNumLet ||
                   prop == word_property::Single_Quote;
        }

        // Corresonds to (MidNum | MidNumLetQ) in WB11 and WB12
        inline bool mid_num(word_property prop) noexcept
        {
            return prop == word_property::MidNum ||
                   prop == word_property::MidNumLet ||
                   prop == word_property::Single_Quote;
        }

        // Used in WB15, WB16
        enum class word_break_emoji_state_t {
            none,
            first_emoji, // Indicates that prop points to an odd-count emoji.
            second_emoji // Indicates that prop points to an even-count emoji.
        };

        struct ph
        {
            enum { prev_prev, prev, curr, next, next_next };
        };

        struct cp_and_word_prop
        {
            cp_and_word_prop() {}

            template<typename WordPropFunc>
            cp_and_word_prop(uint32_t c, WordPropFunc word_prop) :
                cp(c),
                prop(word_prop(c))
            {}

            uint32_t cp = 0;
            word_property prop = word_property::Other;
        };

        template<typename CPIter>
        struct word_break_state
        {
            word_break_state() {}

            CPIter it;
            bool it_points_to_prev = false;

            std::array<cp_and_word_prop, 5> caps;

            word_break_emoji_state_t emoji_state;
        };

        template<typename CPIter>
        word_break_state<CPIter> next(word_break_state<CPIter> state)
        {
            ++state.it;
            std::copy(
                state.caps.begin() + 1, state.caps.end(), state.caps.begin());
            return state;
        }

        template<typename CPIter>
        word_break_state<CPIter> prev(word_break_state<CPIter> state)
        {
            if (!state.it_points_to_prev)
                --state.it;
            state.it_points_to_prev = false;
            std::copy_backward(
                state.caps.begin(), state.caps.end() - 1, state.caps.end());
            return state;
        }

        inline bool table_word_break(word_property lhs, word_property rhs)
        {
            // Note that WSegSpace.WSegSpace was changed to '1' since that
            // case is handled in the word break FSM.

            // clang-format off
// See chart at http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/WordBreakTest.html.
constexpr std::array<std::array<bool, 20>, 20> word_breaks = {{
// Other CR LF NL Ktk AL ML MN MNL Num ENL RI HL DQ SQ EP WSSp Fmt Extd ZWJ
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Other
    {{1, 1, 0, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   1,  1,   1}}, // CR
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   1,  1,   1}}, // LF
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   1,  1,   1}}, // Newline
    {{1, 1, 1, 1, 0,  1, 1, 1, 1,  1,  0,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Katakana
    {{1, 1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1, 1,   0,  0,   0}}, // ALetter
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // MidLetter
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // MidNum
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // MidNumLet
    {{1, 1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1, 1,   0,  0,   0}}, // Numeric
    {{1, 1, 1, 1, 0,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1, 1,   0,  0,   0}}, // ExtendNumLet
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  0, 1, 1, 1, 1, 1,   0,  0,   0}}, // RI
    {{1, 1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 0, 1, 1,   0,  0,   0}}, // Hebrew_Letter
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Double_Quote
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Single_Quote
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // ExtPict
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // WSegSpace
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Format
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1, 1,   0,  0,   0}}, // Extend
    {{1, 1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 0, 1,   0,  0,   0}}, // ZWJ
}};
            // clang-format on
            auto const lhs_int = static_cast<int>(lhs);
            auto const rhs_int = static_cast<int>(rhs);
            return word_breaks[lhs_int][rhs_int];
        }

        // WB4: Except after line breaks, ignore/skip (Extend | Format |
        // ZWJ)*
        template<typename CPIter, typename Sentinel, typename WordPropFunc>
        word_break_state<CPIter> skip_forward(
            word_break_state<CPIter> state,
            CPIter first,
            Sentinel last,
            WordPropFunc word_prop)
        {
            if (state.it != first && !detail::skippable(state.caps[ph::prev].prop) &&
                detail::skippable(state.caps[ph::curr].prop)) {
                auto last_prop = word_property::Other;
                auto temp_it = boost::text::v1::find_if_not(
                    state.it, last, [word_prop, &last_prop](uint32_t cp) {
                        last_prop = word_prop(cp);
                        return detail::skippable(last_prop);
                    });
                if (temp_it == last) {
                    --temp_it;
                } else if (last_prop == word_property::ExtPict) {
                    auto const next_to_last_prop =
                        word_prop(*std::prev(temp_it));
                    if (next_to_last_prop == word_property::ZWJ)
                        --temp_it;
                }
                state.it = temp_it;
                state.caps[ph::curr] = cp_and_word_prop(*temp_it, word_prop);
                state.caps[ph::next] = cp_and_word_prop();
                state.caps[ph::next_next] = cp_and_word_prop();
                if (std::next(state.it) != last) {
                    state.caps[ph::next] =
                        cp_and_word_prop(*std::next(state.it), word_prop);
                    if (std::next(state.it, 2) != last) {
                        state.caps[ph::next_next] = cp_and_word_prop(
                            *std::next(state.it, 2), word_prop);
                    }
                }
            }
            return state;
        }

        template<typename T>
        using word_prop_func_ =
            decltype(std::declval<T>()(std::declval<uint32_t>()));

        template<
            typename T,
            typename F,
            typename R,
            bool RIsCPRange = is_cp_iter<iterator_t<R>>::value,
            bool FIsWordPropFunc = std::
                is_same<detected_t<word_prop_func_, F>, word_property>::value>
        struct word_prop_func_ret
        {
        };

        template<typename T, typename F, typename R>
        struct word_prop_func_ret<T, F, R, true, true>
        {
            using type = T;
        };

        template<typename T, typename F, typename R = utf32_view<uint32_t *>>
        using word_prop_func_ret_t = typename word_prop_func_ret<T, F, R>::type;

        template<
            typename T,
            typename F,
            typename R,
            bool RIsCPRange = is_grapheme_char_range<R>::value,
            bool FIsWordPropFunc = std::
                is_same<detected_t<word_prop_func_, F>, word_property>::value>
        struct graph_word_prop_func_ret
        {
        };

        template<typename T, typename F, typename R>
        struct graph_word_prop_func_ret<T, F, R, true, true>
        {
            using type = T;
        };

        template<typename T, typename F, typename R>
        using graph_word_prop_func_ret_t =
            typename graph_word_prop_func_ret<T, F, R>::type;
    }

    /** A callable type that returns the next word_property for the given code
        point `cp`.  This is the default used with the word breaking
        functions. */
    struct word_prop_callable
    {
        word_property operator()(uint32_t cp) const noexcept
        {
            return boost::text::v1::word_prop(cp);
        }
    };

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest word break at or before before `it`.  If `it ==
        first`, that is returned.  Otherwise, the first code point of the word
        that `it` is within is returned (even if `it` is already at the first
        code point of a word).

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    CPIter prev_word_break(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Finds the next word break after `first`.  This will be the first code
        point after the current word, or `last` if no next word exists.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \pre `first` is at the beginning of a word. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    CPIter next_word_break(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Finds the nearest word break at or before before `it`.  If `it ==
        range.begin()`, that is returned.  Otherwise, the first code point of
        the word that `it` is within is returned (even if `it` is already at
        the first code point of a word).

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept. */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined prev_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a grapheme_iterator to the nearest word break at or before
        before `it`.  If `it == range.begin()`, that is returned.  Otherwise,
        the first grapheme of the word that `it` is within is returned (even
        if `it` is already at the first grapheme of a word).

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept. */
    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined prev_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Finds the next word break after `it`.  This will be the first code
        point after the current word, or `range.end()` if no next word exists.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept.

        \pre `it` is at the beginning of a word. */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined next_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{});

    /** Returns a grapheme_iterator to the next word break after `it`.  This
        will be the first grapheme after the current word, or `range.end()` if
        no next word exists.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept.

        \pre `it` is at the beginning of a word. */
    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined next_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns true iff `it` is at the beginning of a word, or `it == last`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    bool at_word_break(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns true iff `it` is at the beginning of a word, or `it ==
        std::end(range)`.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept. */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    bool at_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns true iff `it` is at the beginning of a word, or `it ==
        std::end(range)`.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept. */
    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    bool at_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

#else

    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto prev_word_break(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        using detail::ph;
        using detail::cp_and_word_prop;

        if (it == first)
            return it;

        if (it == last && --it == first)
            return it;

        detail::word_break_state<CPIter> state;

        state.it = it;

        state.caps[ph::curr] = cp_and_word_prop(*state.it, word_prop);

        // Since cp_break is evaluated unconditionally before the other rules,
        // we need to do all this her before the special-casing below.
        if (it != first) {
            state.caps[ph::prev] =
                cp_and_word_prop(*std::prev(state.it), word_prop);
            if (std::prev(state.it) != first) {
                state.caps[ph::prev_prev] =
                    cp_and_word_prop(*std::prev(state.it, 2), word_prop);
            }
        }
        if (std::next(state.it) != last) {
            state.caps[ph::next] =
                cp_and_word_prop(*std::next(state.it), word_prop);
            if (std::next(state.it, 2) != last) {
                state.caps[ph::next_next] =
                    cp_and_word_prop(*std::next(state.it, 2), word_prop);
            }
        }
        if (cp_break(
                state.caps[ph::prev_prev].cp,
                state.caps[ph::prev].cp,
                state.caps[ph::curr].cp,
                state.caps[ph::next].cp,
                state.caps[ph::next_next].cp)) {
            return state.it;
        }

        // Special case: If state.caps[ph::curr].prop is skippable, we need to
        // skip backward until we find a non-skippable.
        if (detail::skippable(state.caps[ph::curr].prop)) {
            auto const prev = boost::text::v1::find_if_not_backward(
                first, it, [word_prop](uint32_t cp) {
                    return detail::skippable(word_prop(cp));
                });
            if (prev != it) {
                state.it = prev;
                state.caps[ph::next] =
                    cp_and_word_prop(*std::next(state.it), word_prop);
                state.caps[ph::curr] = cp_and_word_prop(*state.it, word_prop);

                // If we end up on a non-skippable that should break before the
                // skippable(s) we just moved over, break on the last skippable.
                if (!detail::skippable(state.caps[ph::curr].prop) &&
                    detail::table_word_break(
                        state.caps[ph::curr].prop, state.caps[ph::next].prop)) {
                    return ++state.it;
                }
                if (state.it == first)
                    return first;
            }
        }

        state.caps[ph::prev_prev] = cp_and_word_prop();
        if (std::prev(state.it) != first) {
            state.caps[ph::prev_prev] =
                cp_and_word_prop(*std::prev(state.it, 2), word_prop);
        }
        state.caps[ph::prev] =
            cp_and_word_prop(*std::prev(state.it), word_prop);
        state.caps[ph::next] = cp_and_word_prop();
        state.caps[ph::next_next] = cp_and_word_prop();
        if (std::next(state.it) != last) {
            state.caps[ph::next] =
                cp_and_word_prop(*std::next(state.it), word_prop);
            if (std::next(state.it, 2) != last) {
                state.caps[ph::next_next] =
                    cp_and_word_prop(*std::next(state.it, 2), word_prop);
            }
        }

        // Since 'it' may be anywhere within the word in which it sits, we
        // need to look forward to make sure that next_prop and next_next_prop
        // don't point to skippables.
        {
            if (std::next(state.it) != last) {
                auto temp_state = state;
                temp_state = detail::next(temp_state);
                temp_state =
                    detail::skip_forward(temp_state, first, last, word_prop);
                if (temp_state.it == last) {
                    state.caps[ph::next] = cp_and_word_prop();
                    state.caps[ph::next_next] = cp_and_word_prop();
                } else {
                    state.caps[ph::next] = temp_state.caps[ph::curr];
                    if (std::next(temp_state.it) != last) {
                        temp_state = detail::next(temp_state);
                        temp_state = detail::skip_forward(
                            temp_state, first, last, word_prop);
                        if (temp_state.it == last) {
                            state.caps[ph::next_next] = cp_and_word_prop();
                        } else {
                            state.caps[ph::next_next] =
                                temp_state.caps[ph::curr];
                        }
                    }
                }
            }
        }

        state.emoji_state = detail::word_break_emoji_state_t::none;

        // WB4: Except after line breaks, ignore/skip (Extend | Format |
        // ZWJ)*
        auto skip = [word_prop](
                        detail::word_break_state<CPIter> state, CPIter first) {
            if (detail::skippable(state.caps[ph::prev].prop)) {
                auto temp_it = boost::text::v1::find_if_not_backward(
                    first, state.it, [word_prop](uint32_t cp) {
                        return detail::skippable(word_prop(cp));
                    });
                if (temp_it == state.it)
                    return state;
                auto temp_prev_cap = cp_and_word_prop(*temp_it, word_prop);
                if (!detail::linebreak(temp_prev_cap.prop)) {
                    state.it = temp_it;
                    state.it_points_to_prev = true;
                    state.caps[ph::prev] = temp_prev_cap;
                    if (temp_it == first) {
                        state.caps[ph::prev_prev] = cp_and_word_prop();
                    } else {
                        state.caps[ph::prev_prev] =
                            cp_and_word_prop(*std::prev(temp_it), word_prop);
                    }
                }
            }
            return state;
        };

        for (; state.it != first; state = detail::prev(state)) {
            if (std::prev(state.it) != first) {
                state.caps[ph::prev_prev] =
                    cp_and_word_prop(*std::prev(state.it, 2), word_prop);
            } else {
                state.caps[ph::prev_prev] = cp_and_word_prop();
            }

            // Check cp_break before anything else.
            if (cp_break(
                    state.caps[ph::prev_prev].cp,
                    state.caps[ph::prev].cp,
                    state.caps[ph::curr].cp,
                    state.caps[ph::next].cp,
                    state.caps[ph::next_next].cp)) {
                return state.it;
            }

            // When we see an RI, back up to the first RI so we can see what
            // emoji state we're supposed to be in here.
            if (state.emoji_state == detail::word_break_emoji_state_t::none &&
                state.caps[ph::curr].prop ==
                    word_property::Regional_Indicator) {
                auto temp_state = state;
                int ris_before = 0;
                while (temp_state.it != first) {
                    temp_state = skip(temp_state, first);
                    if (temp_state.it == first) {
                        if (temp_state.caps[ph::prev].prop ==
                            word_property::Regional_Indicator) {
                            ++ris_before;
                        }
                        break;
                    }
                    if (temp_state.caps[ph::prev].prop ==
                        word_property::Regional_Indicator) {
                        temp_state = detail::prev(temp_state);
                        if (temp_state.it != first &&
                            std::prev(temp_state.it) != first) {
                            temp_state.caps[ph::prev_prev] = cp_and_word_prop(
                                *std::prev(temp_state.it, 2), word_prop);
                        } else {
                            temp_state.caps[ph::prev_prev] = cp_and_word_prop();
                        }
                        ++ris_before;
                    } else {
                        break;
                    }
                }
                state.emoji_state =
                    (ris_before % 2 == 0)
                        ? detail::word_break_emoji_state_t::first_emoji
                        : detail::word_break_emoji_state_t::second_emoji;
            }

            // WB3
            if (state.caps[ph::prev].prop == word_property::CR &&
                state.caps[ph::curr].prop == word_property::LF) {
                continue;
            }

            // WB3a
            if (state.caps[ph::prev].prop == word_property::CR ||
                state.caps[ph::prev].prop == word_property::LF ||
                state.caps[ph::prev].prop == word_property::Newline) {
                return state.it;
            }

            // WB3b
            if (state.caps[ph::curr].prop == word_property::CR ||
                state.caps[ph::curr].prop == word_property::LF ||
                state.caps[ph::curr].prop == word_property::Newline) {
                return state.it;
            }

            // WB3c
            if (state.caps[ph::prev].prop == word_property::ZWJ &&
                state.caps[ph::curr].prop == word_property::ExtPict) {
                continue;
            }

            // WB3d
            if (state.caps[ph::prev].prop == word_property::WSegSpace &&
                state.caps[ph::curr].prop == word_property::WSegSpace) {
                continue;
            }

            // If we end up breaking durign this iteration, we want the break
            // to show up after the skip, so that the skippable CPs go with
            // the CP before them.  This is to maintain symmetry with
            // next_word_break().
            auto after_skip_it = state.it;

            // Puting this here means not having to do it explicitly below
            // between prev_prop and prop (and transitively, between prop and
            // next_prop).
            state = skip(state, first);

            // WB6
            if (detail::ah_letter(state.caps[ph::prev].prop) &&
                detail::mid_ah(state.caps[ph::curr].prop) &&
                detail::ah_letter(state.caps[ph::next].prop)) {
                continue;
            }

            // WB7
            if (detail::mid_ah(state.caps[ph::prev].prop) &&
                detail::ah_letter(state.caps[ph::curr].prop) &&
                state.it != first) {
                auto const temp_state = skip(detail::prev(state), first);
                if (detail::ah_letter(temp_state.caps[ph::prev].prop))
                    continue;
            }

            // WB7b
            if (state.caps[ph::prev].prop == word_property::Hebrew_Letter &&
                state.caps[ph::curr].prop == word_property::Double_Quote &&
                state.caps[ph::next].prop == word_property::Hebrew_Letter) {
                continue;
            }

            // WB7c
            if (state.caps[ph::prev].prop == word_property::Double_Quote &&
                state.caps[ph::curr].prop == word_property::Hebrew_Letter &&
                state.it != first) {
                auto const temp_state = skip(detail::prev(state), first);
                if (temp_state.caps[ph::prev].prop ==
                    word_property::Hebrew_Letter)
                    continue;
            }

            // WB11
            if (detail::mid_num(state.caps[ph::prev].prop) &&
                state.caps[ph::curr].prop == word_property::Numeric &&
                state.it != first) {
                auto const temp_state = skip(detail::prev(state), first);
                if (temp_state.caps[ph::prev].prop == word_property::Numeric)
                    continue;
            }

            // WB12
            if (state.caps[ph::prev].prop == word_property::Numeric &&
                detail::mid_num(state.caps[ph::curr].prop) &&
                state.caps[ph::next].prop == word_property::Numeric) {
                continue;
            }

            if (state.emoji_state ==
                detail::word_break_emoji_state_t::first_emoji) {
                if (state.caps[ph::prev].prop ==
                    word_property::Regional_Indicator) {
                    state.emoji_state =
                        detail::word_break_emoji_state_t::second_emoji;
                    return after_skip_it;
                } else {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                }
            } else if (
                state.emoji_state ==
                    detail::word_break_emoji_state_t::second_emoji &&
                state.caps[ph::prev].prop ==
                    word_property::Regional_Indicator) {
                state.emoji_state =
                    detail::word_break_emoji_state_t::first_emoji;
                continue;
            }

            if (detail::table_word_break(
                    state.caps[ph::prev].prop, state.caps[ph::curr].prop))
                return after_skip_it;
        }

        return first;
    }

    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto next_word_break(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        using detail::ph;
        using detail::cp_and_word_prop;

        if (first == last)
            return first;

        detail::word_break_state<CPIter> state;
        state.it = first;

        if (++state.it == last)
            return state.it;

        state.caps[ph::prev_prev] = cp_and_word_prop();
        state.caps[ph::prev] =
            cp_and_word_prop(*std::prev(state.it), word_prop);
        state.caps[ph::curr] = cp_and_word_prop(*state.it, word_prop);
        state.caps[ph::next] = cp_and_word_prop();
        state.caps[ph::next_next] = cp_and_word_prop();
        if (std::next(state.it) != last) {
            state.caps[ph::next] =
                cp_and_word_prop(*std::next(state.it), word_prop);
            if (std::next(state.it, 2) != last) {
                state.caps[ph::next_next] =
                    cp_and_word_prop(*std::next(state.it, 2), word_prop);
            }
        }

        state.emoji_state =
            state.caps[ph::prev].prop == word_property::Regional_Indicator
                ? detail::word_break_emoji_state_t::first_emoji
                : detail::word_break_emoji_state_t::none;

        for (; state.it != last; state = detail::next(state)) {
            if (std::next(state.it) != last && std::next(state.it, 2) != last) {
                state.caps[ph::next_next] =
                    cp_and_word_prop(*std::next(state.it, 2), word_prop);
            } else {
                state.caps[ph::next_next] = cp_and_word_prop();
            }

            // Check cp_break before anything else.
            if (cp_break(
                    state.caps[ph::prev_prev].cp,
                    state.caps[ph::prev].cp,
                    state.caps[ph::curr].cp,
                    state.caps[ph::next].cp,
                    state.caps[ph::next_next].cp)) {
                return state.it;
            }

            // WB3
            if (state.caps[ph::prev].prop == word_property::CR &&
                state.caps[ph::curr].prop == word_property::LF) {
                continue;
            }

            // WB3a
            if (state.caps[ph::prev].prop == word_property::CR ||
                state.caps[ph::prev].prop == word_property::LF ||
                state.caps[ph::prev].prop == word_property::Newline) {
                return state.it;
            }

            // WB3b
            if (state.caps[ph::curr].prop == word_property::CR ||
                state.caps[ph::curr].prop == word_property::LF ||
                state.caps[ph::curr].prop == word_property::Newline) {
                return state.it;
            }

            // WB3c
            if (state.caps[ph::prev].prop == word_property::ZWJ &&
                state.caps[ph::curr].prop == word_property::ExtPict) {
                continue;
            }

            // WB3d
            if (state.caps[ph::prev].prop == word_property::WSegSpace &&
                state.caps[ph::curr].prop == word_property::WSegSpace) {
                continue;
            }

            // Putting this here means not having to do it explicitly below
            // between prop and next_prop (and transitively, between prev_prop
            // and prop).
            state = detail::skip_forward(state, first, last, word_prop);
            if (state.it == last)
                return state.it;

            // WB6
            if (detail::ah_letter(state.caps[ph::prev].prop) &&
                detail::mid_ah(state.caps[ph::curr].prop) &&
                std::next(state.it) != last) {
                auto const temp_state =
                    detail::skip_forward(detail::next(state), first, last, word_prop);
                if (temp_state.it == last)
                    return temp_state.it;
                if (detail::ah_letter(temp_state.caps[ph::curr].prop))
                    continue;
            }

            // WB7
            if (detail::ah_letter(state.caps[ph::prev_prev].prop) &&
                detail::mid_ah(state.caps[ph::prev].prop) &&
                detail::ah_letter(state.caps[ph::curr].prop)) {
                continue;
            }

            // WB7b
            if (state.caps[ph::prev].prop == word_property::Hebrew_Letter &&
                state.caps[ph::curr].prop == word_property::Double_Quote &&
                std::next(state.it) != last) {
                auto const temp_state =
                    detail::skip_forward(detail::next(state), first, last, word_prop);
                if (temp_state.it == last)
                    return temp_state.it;
                if (temp_state.caps[ph::curr].prop ==
                    word_property::Hebrew_Letter)
                    continue;
            }

            // WB7c
            if (state.caps[ph::prev_prev].prop ==
                    word_property::Hebrew_Letter &&
                state.caps[ph::prev].prop == word_property::Double_Quote &&
                state.caps[ph::curr].prop == word_property::Hebrew_Letter) {
                continue;
            }

            // WB11
            if (state.caps[ph::prev_prev].prop == word_property::Numeric &&
                detail::mid_num(state.caps[ph::prev].prop) &&
                state.caps[ph::curr].prop == word_property::Numeric) {
                continue;
            }

            // WB12
            if (state.caps[ph::prev].prop == word_property::Numeric &&
                detail::mid_num(state.caps[ph::curr].prop) &&
                std::next(state.it) != last) {
                auto const temp_state =
                    detail::skip_forward(detail::next(state), first, last, word_prop);
                if (temp_state.it == last)
                    return temp_state.it;
                if (temp_state.caps[ph::curr].prop == word_property::Numeric)
                    continue;
            }

            if (state.emoji_state ==
                detail::word_break_emoji_state_t::first_emoji) {
                if (state.caps[ph::curr].prop ==
                    word_property::Regional_Indicator) {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                    continue;
                } else {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                }
            } else if (
                state.caps[ph::curr].prop ==
                word_property::Regional_Indicator) {
                state.emoji_state =
                    detail::word_break_emoji_state_t::first_emoji;
                return state.it;
            }

            if (detail::table_word_break(
                    state.caps[ph::prev].prop, state.caps[ph::curr].prop))
                return state.it;
        }
        return state.it;
    }

    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto prev_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept -> detail::
        word_prop_func_ret_t<detail::iterator_t<CPRange>, WordPropFunc, CPRange>
    {
        return boost::text::v1::prev_word_break(
            std::begin(range), it, std::end(range), word_prop, cp_break);
    }

    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto prev_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<
            detail::iterator_t<GraphemeRange const>,
            WordPropFunc,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                boost::text::v1::prev_word_break(
                    range.begin().base(),
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base(),
                    word_prop,
                    cp_break),
                range.end().base()};
    }

    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto next_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept -> detail::
        word_prop_func_ret_t<detail::iterator_t<CPRange>, WordPropFunc, CPRange>
    {
        return boost::text::v1::next_word_break(
            it, std::end(range), word_prop, cp_break);
    }

    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto next_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<
            detail::iterator_t<GraphemeRange const>,
            WordPropFunc,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                boost::text::v1::next_word_break(
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base(),
                    word_prop,
                    cp_break),
                range.end().base()};
    }

    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto at_word_break(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        if (it == last)
            return true;
        return prev_word_break(first, it, last, word_prop, cp_break) == it;
    }

    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto at_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<bool, WordPropFunc, CPRange>
    {
        if (it == std::end(range))
            return true;
        return prev_word_break(
                   std::begin(range),
                   it,
                   std::end(range),
                   word_prop,
                   cp_break) == it;
    }

    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto at_word_break(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<bool, WordPropFunc, GraphemeRange>
    {
        if (it == std::end(range))
            return true;
        using cp_iter_t = decltype(range.begin().base());
        cp_iter_t it_ = static_cast<cp_iter_t>(it.base());
        return prev_word_break(
                   range.begin().base(),
                   it_,
                   range.end().base(),
                   word_prop,
                   cp_break) == it_;
    }

#endif

    namespace detail {
        template<
            typename CPIter,
            typename Sentinel,
            typename WordPropFunc,
            typename CPWordBreakFunc>
        struct next_word_callable
        {
            auto operator()(CPIter it, Sentinel last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return boost::text::v1::next_word_break(
                    it, last, word_prop_, cp_break_);
            }

            WordPropFunc word_prop_;
            CPWordBreakFunc cp_break_;
        };

        template<
            typename CPIter,
            typename WordPropFunc,
            typename CPWordBreakFunc>
        struct prev_word_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return boost::text::v1::prev_word_break(
                    first, it, last, word_prop_, cp_break_);
            }

            WordPropFunc word_prop_;
            CPWordBreakFunc cp_break_;
        };
    }

    /** Returns the bounds of the word that `it` lies within. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    utf32_view<CPIter> word(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
    {
        first = boost::text::v1::prev_word_break(
            first, it, last, word_prop, cp_break);
        return utf32_view<CPIter>{
            first,
            boost::text::v1::next_word_break(first, last, word_prop, cp_break)};
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns the bounds of the word that `it` lies within, as a utf32_view.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept. */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined word(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns grapheme range delimiting the bounds of the word that `it`
        lies within, as a grapheme_view.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept. */
    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined word(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the code point ranges delimiting words in
        `[first, last)`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept and CPIter is equality comparable with
        Sentinel. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined words(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the code point ranges delimiting words in
        `range`.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept. */
    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the grapheme ranges delimiting words in
        `range`.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept. */
    template<
        typename GraphemeRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined words(
        GraphemeRange const & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the code point ranges delimiting words in
        `[first, last)`, in reverse.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept and CPIter is equality comparable with
        Sentinel. */
    template<
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined reversed_words(
        CPIter first,
        CPIter last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the code point ranges delimiting words in
        `range`, in reverse.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept and `WordPropFunc` models the WordPropFunc
        concept. */
    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined reversed_words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

    /** Returns a lazy range of the grapheme ranges delimiting words in
        `range`, in reverse.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept and `WordPropFunc`
        models the WordPropFunc concept. */
    template<
        typename GraphemeRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::undefined>
    detail::undefined reversed_words(
        GraphemeRange const & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept;

#else

    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto word(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<
            utf32_view<detail::iterator_t<CPRange>>,
            WordPropFunc,
            CPRange>
    {
        auto first = boost::text::v1::prev_word_break(
            std::begin(range), it, std::end(range), word_prop, cp_break);
        return utf32_view<detail::iterator_t<CPRange>>{
            first,
            boost::text::v1::next_word_break(
                first, std::end(range), word_prop, cp_break)};
    }

    template<
        typename GraphemeRange,
        typename GraphemeIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto word(
        GraphemeRange const & range,
        GraphemeIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<
            grapheme_view<decltype(range.begin().base())>,
            WordPropFunc,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto first = boost::text::v1::prev_word_break(
            range.begin().base(),
            static_cast<cp_iter_t>(it.base()),
            range.end().base(),
            word_prop,
            cp_break);
        return {range.begin().base(),
                first,
                boost::text::v1::next_word_break(
                    first, range.end().base(), word_prop, cp_break),
                range.end().base()};
    }

    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto words(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::cp_iter_sntl_ret_t<
            lazy_segment_range<
                CPIter,
                Sentinel,
                detail::next_word_callable<
                    CPIter,
                    Sentinel,
                    WordPropFunc,
                    CPWordBreakFunc>,
                utf32_view<CPIter>,
                detail::const_lazy_segment_iterator,
                false>,
            CPIter,
            Sentinel>
    {
        detail::
            next_word_callable<CPIter, Sentinel, WordPropFunc, CPWordBreakFunc>
                next{word_prop, cp_break};
        return {std::move(next), {first, last}, {last}};
    }

    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>,
                detail::next_word_callable<
                    detail::iterator_t<CPRange>,
                    detail::sentinel_t<CPRange>,
                    WordPropFunc,
                    CPWordBreakFunc>>,
            WordPropFunc,
            CPRange>
    {
        detail::next_word_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>,
            WordPropFunc,
            CPWordBreakFunc>
            next{word_prop, cp_break};
        return {std::move(next),
                {std::begin(range), std::end(range)},
                {std::end(range)}};
    }

    template<
        typename GraphemeRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto words(
        GraphemeRange const & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::next_word_callable<
                    decltype(range.begin().base()),
                    decltype(range.begin().base()),
                    WordPropFunc,
                    CPWordBreakFunc>,
                grapheme_view<decltype(range.begin().base())>>,
            WordPropFunc,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::next_word_callable<
            cp_iter_t,
            cp_iter_t,
            WordPropFunc,
            CPWordBreakFunc>
            next{word_prop, cp_break};
        return {std::move(next),
                {range.begin().base(), range.end().base()},
                {range.end().base()}};
    }

    template<
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto reversed_words(
        CPIter first,
        CPIter last,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::cp_iter_sntl_ret_t<
            lazy_segment_range<
                CPIter,
                CPIter,
                detail::
                    prev_word_callable<CPIter, WordPropFunc, CPWordBreakFunc>,
                utf32_view<CPIter>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            CPIter,
            CPIter>
    {
        detail::prev_word_callable<CPIter, WordPropFunc, CPWordBreakFunc> prev{
            word_prop, cp_break};
        return {std::move(prev), {first, last, last}, {first, first, last}};
    }

    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto reversed_words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>,
                detail::prev_word_callable<
                    detail::iterator_t<CPRange>,
                    WordPropFunc,
                    CPWordBreakFunc>,
                utf32_view<detail::iterator_t<CPRange>>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            WordPropFunc,
            CPRange>
    {
        detail::prev_word_callable<
            detail::iterator_t<CPRange>,
            WordPropFunc,
            CPWordBreakFunc>
            prev{word_prop, cp_break};
        return {std::move(prev),
                {std::begin(range), std::end(range), std::end(range)},
                {std::begin(range), std::begin(range), std::end(range)}};
    }

    template<
        typename GraphemeRange,
        typename WordPropFunc = word_prop_callable,
        typename CPWordBreakFunc = detail::default_cp_break>
    auto reversed_words(
        GraphemeRange const & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPWordBreakFunc cp_break = CPWordBreakFunc{}) noexcept
        -> detail::graph_word_prop_func_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::prev_word_callable<
                    decltype(range.begin().base()),
                    WordPropFunc,
                    CPWordBreakFunc>,
                grapheme_view<decltype(range.begin().base())>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            WordPropFunc,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::prev_word_callable<cp_iter_t, WordPropFunc, CPWordBreakFunc>
            prev{word_prop, cp_break};
        return {
            std::move(prev),
            {range.begin().base(), range.end().base(), range.end().base()},
            {range.begin().base(), range.begin().base(), range.end().base()}};
    }

#endif

}}}

#endif
