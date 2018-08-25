#ifndef BOOST_TEXT_WORD_BREAK_HPP
#define BOOST_TEXT_WORD_BREAK_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/lazy_segment_range.hpp>

#include <array>
#include <unordered_map>

#include <stdint.h>


namespace boost { namespace text {

    /** The word properties outlined in Unicode 10. */
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
        E_Base,
        E_Modifier,
        Glue_After_Zwj,
        E_Base_GAZ,
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

        BOOST_TEXT_DECL std::array<word_prop_interval, 21> const &
        make_word_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, word_property>
        make_word_prop_map();
    }

    /** Returns the word property associated with code point \a cp. */
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
            // clang-format off
// See chart at http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/WordBreakTest.html.
constexpr std::array<std::array<bool, 22>, 22> word_breaks = {{
//  Other CR LF NL Ktk AL ML MN MNL Num ENL RI HL DQ SQ E_Bse E_Mod GAZ EBG Fmt Extd ZWJ
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Other
    {{1,   1, 0, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  1,  1,   1}}, // CR
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  1,  1,   1}}, // LF
                                                                                     
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  1,  1,   1}}, // Newline
    {{1,   1, 1, 1, 0,  1, 1, 1, 1,  1,  0,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Katakana
    {{1,   1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // ALetter
                                                                                     
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // MidLetter
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // MidNum
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // MidNumLet
                                                                                     
    {{1,   1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Numeric
    {{1,   1, 1, 1, 0,  0, 1, 1, 1,  0,  0,  1, 0, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // ExtendNumLet
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  0, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // RI
                                                                                     
    {{1,   1, 1, 1, 1,  0, 1, 1, 1,  0,  0,  1, 0, 1, 0, 1,    1,    1,  1,  0,  0,   0}}, // Hebrew_Letter
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Double_Quote
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Single_Quote
                                                                                     
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    0,    1,  1,  0,  0,   0}}, // E_Base
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // E_Modifier
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Glue_After_Zwj
                                                                                     
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    0,    1,  1,  0,  0,   0}}, // EBG
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Format
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    1,  1,  0,  0,   0}}, // Extend
                                                                                     
    {{1,   1, 1, 1, 1,  1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 1,    1,    0,  0,  0,  0,   0}}, // ZWJ
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
            if (state.it != first && !skippable(state.caps[ph::prev].prop) &&
                skippable(state.caps[ph::curr].prop)) {
                auto temp_it =
                    find_if_not(state.it, last, [word_prop](uint32_t cp) {
                        return skippable(word_prop(cp));
                    });
                if (temp_it == last)
                    --temp_it;
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
            bool FIsWordPropFunc = std::
                is_same<detected_t<word_prop_func_, F>, word_property>::value>
        struct word_prop_func_ret
        {
        };

        template<typename T, typename F>
        struct word_prop_func_ret<T, F, true>
        {
            using type = T;
        };

        template<typename T, typename F>
        using word_prop_func_ret_t = typename word_prop_func_ret<T, F>::type;

        template<typename T, typename U>
        using comparable_ = decltype(std::declval<T>() == std::declval<U>());

        template<
            typename T,
            typename CPIter,
            typename Sentinel,
            bool FIsWordPropFunc = is_cp_iter<CPIter>::value &&
                is_detected<comparable_, CPIter, Sentinel>::value>
        struct cp_iter_sntl_ret
        {
        };

        template<typename T, typename CPIter, typename Sentinel>
        struct cp_iter_sntl_ret<T, CPIter, Sentinel, true>
        {
            using type = T;
        };

        template<typename T, typename CPIter, typename Sentinel>
        using cp_iter_sntl_ret_t =
            typename cp_iter_sntl_ret<T, CPIter, Sentinel>::type;
    }

    /** A callable type that returns the next word_property for the given code
        point cp.  This is the default used with the word breaking
        functions. */
    struct word_prop_callable
    {
        word_property operator()(uint32_t cp) const noexcept
        {
            return word_prop(cp);
        }
    };

    // TODO: Document WordPropFunc, including that it must be stateless.

    /* TODO: For docs on how to use WordPropFunc:
       Some or all of the following characters may be tailored to be in
       MidLetter, depending on the environment:

        U+002D ( - ) HYPHEN-MINUS
        U+055A ( ՚ ) ARMENIAN APOSTROPHE
        U+058A ( ֊ ) ARMENIAN HYPHEN
        U+0F0B ( ་ ) TIBETAN MARK INTERSYLLABIC TSHEG
        U+1806 ( ᠆ ) MONGOLIAN TODO SOFT HYPHEN
        U+2010 ( ‐ ) HYPHEN
        U+2011 ( ‑ ) NON-BREAKING HYPHEN
        U+201B ( ‛ ) SINGLE HIGH-REVERSED-9 QUOTATION MARK
        U+30A0 ( ゠ ) KATAKANA-HIRAGANA DOUBLE HYPHEN
        U+30FB ( ・ ) KATAKANA MIDDLE DOT
        U+FE63 ( ﹣ ) SMALL HYPHEN-MINUS
        U+FF0D ( － ) FULLWIDTH HYPHEN-MINUS

       For example, some writing systems use a hyphen character between
       syllables within a word. An example is the Iu Mien language written
       with the Thai script. Such words should behave as single words for the
       purpose of selection (“double-click”), indexing, and so forth, meaning
       that they should not word-break on the hyphen.

       Some or all of the following characters may be tailored to be in
       MidNum, depending on the environment, to allow for languages that use
       spaces as thousands separators, such as €1 234,56.

        U+0020 SPACE
        U+00A0 NO-BREAK SPACE
        U+2007 FIGURE SPACE
        U+2008 PUNCTUATION SPACE
        U+2009 THIN SPACE
        U+202F NARROW NO-BREAK SPACE
    */

    /** Finds the nearest word break at or before before <code>it</code>.  If
        <code>it == first</code>, that is returned.  Otherwise, the first code
        point of the word that <code>it</code> is within is returned (even if
        <code>it</code> is already at the first code point of a word). */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto prev_word_break(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
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
            state.it =
                find_if_not_backward(first, it, [word_prop](uint32_t cp) {
                    return detail::skippable(word_prop(cp));
                });
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
                temp_state = next(temp_state);
                temp_state =
                    detail::skip_forward(temp_state, first, last, word_prop);
                if (temp_state.it == last) {
                    state.caps[ph::next] = cp_and_word_prop();
                    state.caps[ph::next_next] = cp_and_word_prop();
                } else {
                    state.caps[ph::next] = temp_state.caps[ph::curr];
                    if (std::next(temp_state.it) != last) {
                        temp_state = next(temp_state);
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
                auto temp_it = find_if_not_backward(
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

        for (; state.it != first; state = prev(state)) {
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
                        temp_state = prev(temp_state);
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
                (state.caps[ph::curr].prop == word_property::Glue_After_Zwj ||
                 state.caps[ph::curr].prop == word_property::E_Base_GAZ)) {
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
                auto const temp_state = skip(prev(state), first);
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
                auto const temp_state = skip(prev(state), first);
                if (temp_state.caps[ph::prev].prop ==
                    word_property::Hebrew_Letter)
                    continue;
            }

            // WB11
            if (detail::mid_num(state.caps[ph::prev].prop) &&
                state.caps[ph::curr].prop == word_property::Numeric &&
                state.it != first) {
                auto const temp_state = skip(prev(state), first);
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

    /** Finds the next word break after <code>first</code>.  This will be the
        first code point after the current word, or <code>last</code> if no
        next word exists.

        \pre <code>first</code> is at the beginning of a word. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto next_word_break(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
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

        for (; state.it != last; state = next(state)) {
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
                (state.caps[ph::curr].prop == word_property::Glue_After_Zwj ||
                 state.caps[ph::curr].prop == word_property::E_Base_GAZ)) {
                continue;
            }

            // Puting this here means not having to do it explicitly below
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
                    detail::skip_forward(next(state), first, last, word_prop);
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
                    detail::skip_forward(next(state), first, last, word_prop);
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
                    detail::skip_forward(next(state), first, last, word_prop);
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

    /** Finds the nearest word break at or before before <code>it</code>.  If
        <code>it == range.begin()</code>, that is returned.  Otherwise, the
        first code point of the word that <code>it</code> is within is
        returned (even if <code>it</code> is already at the first code point
        of a word). */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto prev_word_break(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::iterator_t<CPRange>
    {
        return prev_word_break(
            std::begin(range), it, std::end(range), word_prop, cp_break);
    }

    /** Finds the next word break after <code>range.begin()</code>.  This will
        be the first code point after the current word, or
        <code>range.end()</code> if no next word exists.

        \pre <code>range.begin()</code> is at the beginning of a word. */
    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto next_word_break(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::iterator_t<CPRange>
    {
        return next_word_break(
            std::begin(range), std::end(range), word_prop, cp_break);
    }

    namespace detail {
        template<
            typename CPIter,
            typename Sentinel,
            typename WordPropFunc,
            typename CPBreakFunc>
        struct next_word_callable
        {
            auto operator()(CPIter it, Sentinel last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return next_word_break(it, last, word_prop_, cp_break_);
            }

            WordPropFunc word_prop_;
            CPBreakFunc cp_break_;
        };

        template<typename CPIter, typename WordPropFunc, typename CPBreakFunc>
        struct prev_word_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return prev_word_break(first, it, last, word_prop_, cp_break_);
            }

            WordPropFunc word_prop_;
            CPBreakFunc cp_break_;
        };
    }

    /** Returns the bounds of the word that <code>it</code> lies within. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    cp_range<CPIter> word(
        CPIter first,
        CPIter it,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
    {
        first = prev_word_break(first, it, last, word_prop, cp_break);
        return cp_range<CPIter>{
            first, next_word_break(first, last, word_prop, cp_break)};
    }

    /** Returns the bounds of the word that <code>it</code> lies within. */
    template<
        typename CPRange,
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto word(
        CPRange & range,
        CPIter it,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> cp_range<detail::iterator_t<CPRange>>
    {
        auto first = prev_word_break(
            std::begin(range), it, std::end(range), word_prop, cp_break);
        return cp_range<CPIter>{
            first,
            next_word_break(first, std::end(range), word_prop, cp_break)};
    }

    /** Returns a lazy range of the code point ranges delimiting words in
        <code>[first, last)</code>. */
    template<
        typename CPIter,
        typename Sentinel,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto words(
        CPIter first,
        Sentinel last,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::cp_iter_sntl_ret_t<
            lazy_segment_range<
                CPIter,
                Sentinel,
                detail::next_word_callable<
                    CPIter,
                    Sentinel,
                    WordPropFunc,
                    CPBreakFunc>,
                cp_range<CPIter>,
                detail::const_lazy_segment_iterator,
                false>,
            CPIter,
            Sentinel>
    {
        detail::next_word_callable<CPIter, Sentinel, WordPropFunc, CPBreakFunc>
            next{word_prop, cp_break};
        return {std::move(next), {first, last}, {last}};
    }

    /** Returns a lazy range of the code point ranges delimiting words in
        <code>range</code>. */
    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>,
                detail::next_word_callable<
                    detail::iterator_t<CPRange>,
                    detail::sentinel_t<CPRange>,
                    WordPropFunc,
                    CPBreakFunc>>,
            WordPropFunc>
    {
        detail::next_word_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>,
            WordPropFunc,
            CPBreakFunc>
            next{word_prop, cp_break};
        return {std::move(next),
                {std::begin(range), std::end(range)},
                {std::end(range)}};
    }

    /** Returns a lazy range of the code point ranges delimiting words in
        <code>[first, last)</code>, in reverse order. */
    template<
        typename CPIter,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto reverse_words(
        CPIter first,
        CPIter last,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::cp_iter_sntl_ret_t<
            lazy_segment_range<
                CPIter,
                CPIter,
                detail::prev_word_callable<CPIter, WordPropFunc, CPBreakFunc>,
                cp_range<CPIter>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            CPIter,
            CPIter>
    {
        detail::prev_word_callable<CPIter, WordPropFunc, CPBreakFunc> prev{
            word_prop, cp_break};
        return {std::move(prev), {first, last, last}, {first, first, last}};
    }

    /** Returns a lazy range of the code point ranges delimiting words in
        <code>range</code>. */
    template<
        typename CPRange,
        typename WordPropFunc = word_prop_callable,
        typename CPBreakFunc = detail::default_cp_break>
    auto reverse_words(
        CPRange & range,
        WordPropFunc word_prop = WordPropFunc{},
        CPBreakFunc cp_break = CPBreakFunc{}) noexcept
        -> detail::word_prop_func_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>,
                detail::prev_word_callable<
                    detail::iterator_t<CPRange>,
                    WordPropFunc,
                    CPBreakFunc>,
                cp_range<detail::iterator_t<CPRange>>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            WordPropFunc>
    {
        detail::prev_word_callable<
            detail::iterator_t<CPRange>,
            WordPropFunc,
            CPBreakFunc>
            prev{word_prop, cp_break};
        return {std::move(prev),
                {std::begin(range), std::end(range), std::end(range)},
                {std::begin(range), std::begin(range), std::end(range)}};
    }

}}

#endif
