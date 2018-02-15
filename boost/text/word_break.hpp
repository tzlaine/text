#ifndef BOOST_TEXT_WORD_BREAK_HPP
#define BOOST_TEXT_WORD_BREAK_HPP

#include <array>

#include <stdint.h>

namespace boost { namespace text {

    /** The word properties outlined in Unicode 10. */
    enum class word_prop_t {
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

    /** Returns the word property associated with code point \a cp. */
    word_prop_t word_prop(uint32_t cp) noexcept;

    namespace detail {
        inline bool skippable(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::Extend || prop == word_prop_t::Format ||
                   prop == word_prop_t::ZWJ;
        }

        inline bool linebreak(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::CR || prop == word_prop_t::LF ||
                   prop == word_prop_t::Newline;
        }

        inline bool ah_letter(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::ALetter ||
                   prop == word_prop_t::Hebrew_Letter;
        }

        // Corresonds to (MidLetter | MidNumLetQ) in WB6 and WB7
        inline bool mid_ah(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::MidLetter ||
                   prop == word_prop_t::MidNumLet ||
                   prop == word_prop_t::Single_Quote;
        }

        // Corresonds to (MidNum | MidNumLetQ) in WB11 and WB12
        inline bool mid_num(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::MidNum ||
                   prop == word_prop_t::MidNumLet ||
                   prop == word_prop_t::Single_Quote;
        }

        // Used in WB15, WB16
        enum class word_break_emoji_state_t {
            none,
            first_emoji, // Indicates that prop points to an odd-count emoji.
            second_emoji // Indicates that prop points to an even-count emoji.
        };

        template<typename CPIter>
        struct word_break_state
        {
            CPIter it;
            bool it_points_to_prev = false;

            word_prop_t prev_prev_prop;
            word_prop_t prev_prop;
            word_prop_t prop;
            word_prop_t next_prop;
            word_prop_t next_next_prop;

            word_break_emoji_state_t emoji_state;
        };

        template<typename CPIter>
        word_break_state<CPIter> next(word_break_state<CPIter> state)
        {
            ++state.it;
            state.prev_prev_prop = state.prev_prop;
            state.prev_prop = state.prop;
            state.prop = state.next_prop;
            state.next_prop = state.next_next_prop;
            return state;
        }

        template<typename CPIter>
        word_break_state<CPIter> prev(word_break_state<CPIter> state)
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

        inline bool table_word_break(word_prop_t lhs, word_prop_t rhs)
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
        template<typename CPIter>
        word_break_state<CPIter>
        skip_forward(word_break_state<CPIter> state, CPIter last)
        {
            if (skippable(state.prop)) {
                auto temp_it = state.it;
                while (std::next(temp_it) != last) {
                    auto temp_next_prop = word_prop(*++temp_it);
                    if (!skippable(temp_next_prop))
                        break;
                }
                if (temp_it == last) {
                    state.it = last;
                } else {
                    auto const temp_prop = word_prop(*temp_it);
                    if (!linebreak(temp_prop)) {
                        state.it = temp_it;
                        state.prop = temp_prop;
                        state.next_prop = word_prop_t::Other;
                        state.next_next_prop = word_prop_t::Other;
                        if (std::next(state.it) != last) {
                            state.next_prop = word_prop(*std::next(state.it));
                            if (std::next(state.it, 2) != last) {
                                state.next_next_prop =
                                    word_prop(*std::next(state.it, 2));
                            }
                        }
                    }
                }
            }
            return state;
        }

    }

    // TODO: Sentinels!  Also, audit elsewhere for places that can use them.

    // TODO: Consolidate symmetrical break blocking rules like WB6,WB7 by
    // advancing past them once they are detected, as an optimization.

    /** Finds the nearest word break at or before before <code>it</code>.  If
        <code>it == first</code>, that is returned.  Otherwise, the first code
        point of the word <code>it</code> is within is returned (even if
        <coe>it</code> is already at the first code point of a word. */
    template<typename CPIter>
    inline CPIter
    prev_word_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        if (it == first)
            return it;

        if (it == last && --it == first)
            return it;

        detail::word_break_state<CPIter> state;

        state.it = it;

        // b word_break.hpp:243
        // b word_break.hpp:301
        // b word_break_30.cpp:40

        state.prev_prev_prop = word_prop_t::Other;
        if (std::prev(state.it) != first)
            state.prev_prev_prop = word_prop(*std::prev(state.it, 2));
        state.prev_prop = word_prop(*std::prev(state.it));
        state.prop = word_prop(*state.it);
        state.next_prop = word_prop_t::Other;
        state.next_next_prop = word_prop_t::Other;
        if (std::next(state.it) != last) {
            state.next_prop = word_prop(*std::next(state.it));
            if (std::next(state.it, 2) != last)
                state.next_next_prop = word_prop(*std::next(state.it, 2));
        }

        // Since 'it' may be anywhere within the word in which it sits, we
        // need to look forward to make sure that next_prop and next_next_prop
        // don't point to skippables.
        {
            if (std::next(state.it) != last) {
                auto temp_state = state;
                temp_state = next(temp_state);
                temp_state = detail::skip_forward(temp_state, last);
                if (temp_state.it == last) {
                    state.next_prop = word_prop_t::Other;
                    state.next_next_prop = word_prop_t::Other;
                } else {
                    state.next_prop = temp_state.prop;
                    if (std::next(temp_state.it) != last) {
                        temp_state = next(temp_state);
                        temp_state = detail::skip_forward(temp_state, last);
                        if (temp_state.it == last)
                            state.next_next_prop = word_prop_t::Other;
                        else
                            state.next_next_prop = temp_state.prop;
                    }
                }
            }
        }

        state.emoji_state = detail::word_break_emoji_state_t::none;

        // WB4: Except after line breaks, ignore/skip (Extend | Format |
        // ZWJ)*
        auto skip = [](detail::word_break_state<CPIter> state, CPIter first) {
            if (detail::skippable(state.prev_prop)) {
                auto temp_it = state.it;
                auto temp_prev_prop = word_prop(*--temp_it);
                while (temp_it != first && detail::skippable(temp_prev_prop)) {
                    temp_prev_prop = word_prop(*--temp_it);
                }
                if (!detail::linebreak(temp_prev_prop)) {
                    state.it = temp_it;
                    state.it_points_to_prev = true;
                    state.prev_prop = temp_prev_prop;
                    auto temp_prev_prev_prop = word_prop(*std::prev(temp_it));
                    state.prev_prev_prop = temp_prev_prev_prop;
                }
            }
            return state;
        };

        for (; state.it != first; state = prev(state)) {
            if (std::prev(state.it) != first)
                state.prev_prev_prop = word_prop(*std::prev(state.it, 2));
            else
                state.prev_prev_prop = word_prop_t::Other;

            // When we see an RI, back up to the first RI so we can see what
            // emoji state we're supposed to be in here.
            if (state.emoji_state == detail::word_break_emoji_state_t::none &&
                state.prop == word_prop_t::Regional_Indicator) {
                auto temp_state = state;
                int ris_before = 0;
                while (temp_state.it != first) {
                    temp_state = skip(temp_state, first);
                    if (temp_state.it == first)
                        break;
                    if (temp_state.prev_prop ==
                        word_prop_t::Regional_Indicator) {
                        temp_state = prev(temp_state);
                        if (std::prev(temp_state.it) != first)
                            temp_state.prev_prev_prop =
                                word_prop(*std::prev(temp_state.it, 2));
                        else
                            temp_state.prev_prev_prop = word_prop_t::Other;
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
            if (state.prev_prop == word_prop_t::CR &&
                state.prop == word_prop_t::LF) {
                continue;
            }

            // WB3a
            if (state.prev_prop == word_prop_t::CR ||
                state.prev_prop == word_prop_t::LF ||
                state.prev_prop == word_prop_t::Newline) {
                return state.it;
            }

            // WB3b
            if (state.prop == word_prop_t::CR ||
                state.prop == word_prop_t::LF ||
                state.prop == word_prop_t::Newline) {
                return state.it;
            }

            // WB3c
            if (state.prev_prop == word_prop_t::ZWJ &&
                (state.prop == word_prop_t::Glue_After_Zwj ||
                 state.prop == word_prop_t::E_Base_GAZ)) {
                continue;
            }

            // If we end up breaking here, we wnt the break to show up after
            // the skip, so that the skippable CPs go with the CP before them.
            // This is to maintain symmetry with next_word_break().
            auto after_skip_it = state.it;

            // Puting this here means not having to do it explicitly below
            // between prev_prop and prop (and transitively, between prop and
            // next_prop).
            state = skip(state, first);

            // WB6
            if (detail::ah_letter(state.prev_prop) &&
                detail::mid_ah(state.prop) &&
                detail::ah_letter(state.next_prop)) {
                continue;
            }

            // WB7
            if (detail::mid_ah(state.prev_prop) &&
                detail::ah_letter(state.prop) && state.it != first) {
                auto const temp_state = skip(prev(state), first);
                if (detail::ah_letter(temp_state.prev_prop))
                    continue;
            }

            // WB7b
            if (state.prev_prop == word_prop_t::Hebrew_Letter &&
                state.prop == word_prop_t::Double_Quote &&
                state.next_prop == word_prop_t::Hebrew_Letter) {
                continue;
            }

            // WB7c
            if (state.prev_prop == word_prop_t::Double_Quote &&
                state.prop == word_prop_t::Hebrew_Letter && state.it != first) {
                auto const temp_state = skip(prev(state), first);
                if (temp_state.prev_prop == word_prop_t::Hebrew_Letter)
                    continue;
            }

            // WB11
            if (detail::mid_num(state.prev_prop) &&
                state.prop == word_prop_t::Numeric && state.it != first) {
                auto const temp_state = skip(prev(state), first);
                if (temp_state.prev_prop == word_prop_t::Numeric)
                    continue;
            }

            // WB12
            if (state.prev_prop == word_prop_t::Numeric &&
                detail::mid_num(state.prop) &&
                state.next_prop == word_prop_t::Numeric) {
                continue;
            }

            if (state.emoji_state ==
                detail::word_break_emoji_state_t::first_emoji) {
                if (state.prev_prop == word_prop_t::Regional_Indicator) {
                    state.emoji_state =
                        detail::word_break_emoji_state_t::second_emoji;
                } else {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                }
            } else if (
                state.emoji_state ==
                    detail::word_break_emoji_state_t::second_emoji &&
                state.prev_prop == word_prop_t::Regional_Indicator) {
                state.emoji_state =
                    detail::word_break_emoji_state_t::first_emoji;
                continue;
            }

            if (detail::table_word_break(state.prev_prop, state.prop))
                return after_skip_it;
        }

        return first;
    }

    /** Finds the next word break after <code>it</code>.  This will be the
        first code point after the current word, or <code>last</code> if no
        next word exists.

        \pre <code>it</code> is at the beginning of a word.*/
    template<typename CPIter>
    inline CPIter next_word_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        if (it == last)
            return last;

        if (++it == last)
            return last;

        detail::word_break_state<CPIter> state;

        state.it = it;

        state.prev_prev_prop = word_prop_t::Other;
        state.prev_prop = word_prop_t::Other;
        if (state.it != first) {
            state.prev_prop = word_prop(*std::prev(state.it));
            if (std::prev(state.it) != first)
                state.prev_prev_prop = word_prop(*std::prev(state.it, 2));
        }
        state.prop = word_prop(*state.it);
        state.next_prop = word_prop_t::Other;
        state.next_next_prop = word_prop_t::Other;
        if (std::next(state.it) != last) {
            state.next_prop = word_prop(*std::next(state.it));
            if (std::next(state.it, 2) != last)
                state.next_next_prop = word_prop(*std::next(state.it, 2));
        }

        state.emoji_state = state.prev_prop == word_prop_t::Regional_Indicator
                                ? detail::word_break_emoji_state_t::first_emoji
                                : detail::word_break_emoji_state_t::none;

        for (; state.it != last; state = next(state)) {
            if (std::next(state.it) != last && std::next(state.it, 2) != last)
                state.next_next_prop = word_prop(*std::next(state.it, 2));
            else
                state.next_next_prop = word_prop_t::Other;

            // WB3
            if (state.prev_prop == word_prop_t::CR &&
                state.prop == word_prop_t::LF) {
                continue;
            }

            // WB3a
            if (state.prev_prop == word_prop_t::CR ||
                state.prev_prop == word_prop_t::LF ||
                state.prev_prop == word_prop_t::Newline) {
                return state.it;
            }

            // WB3b
            if (state.prop == word_prop_t::CR ||
                state.prop == word_prop_t::LF ||
                state.prop == word_prop_t::Newline) {
                return state.it;
            }

            // WB3c
            if (state.prev_prop == word_prop_t::ZWJ &&
                (state.prop == word_prop_t::Glue_After_Zwj ||
                 state.prop == word_prop_t::E_Base_GAZ)) {
                continue;
            }

            // Puting this here means not having to do it explicitly below
            // between prop and next_prop (and transitively, between prev_prop
            // and prop).
            state = detail::skip_forward(state, last);
            if (state.it == last)
                return last;

            // WB6
            if (detail::ah_letter(state.prev_prop) &&
                detail::mid_ah(state.prop) && std::next(state.it) != last) {
                auto const temp_state = detail::skip_forward(next(state), last);
                if (temp_state.it == last)
                    return last;
                if (detail::ah_letter(temp_state.prop))
                    continue;
            }

            // WB7
            if (detail::ah_letter(state.prev_prev_prop) &&
                detail::mid_ah(state.prev_prop) &&
                detail::ah_letter(state.prop)) {
                continue;
            }

            // WB7b
            if (state.prev_prop == word_prop_t::Hebrew_Letter &&
                state.prop == word_prop_t::Double_Quote &&
                std::next(state.it) != last) {
                auto const temp_state = detail::skip_forward(next(state), last);
                if (temp_state.it == last)
                    return last;
                if (temp_state.prop == word_prop_t::Hebrew_Letter)
                    continue;
            }

            // WB7c
            if (state.prev_prev_prop == word_prop_t::Hebrew_Letter &&
                state.prev_prop == word_prop_t::Double_Quote &&
                state.prop == word_prop_t::Hebrew_Letter) {
                continue;
            }

            // WB11
            if (state.prev_prev_prop == word_prop_t::Numeric &&
                detail::mid_num(state.prev_prop) &&
                state.prop == word_prop_t::Numeric) {
                continue;
            }

            // WB12
            if (state.prev_prop == word_prop_t::Numeric &&
                detail::mid_num(state.prop) && std::next(state.it) != last) {
                auto const temp_state = detail::skip_forward(next(state), last);
                if (temp_state.it == last)
                    return last;
                if (temp_state.prop == word_prop_t::Numeric)
                    continue;
            }

            if (state.emoji_state ==
                detail::word_break_emoji_state_t::first_emoji) {
                if (state.prop == word_prop_t::Regional_Indicator) {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                    continue;
                } else {
                    state.emoji_state = detail::word_break_emoji_state_t::none;
                }
            } else if (state.prop == word_prop_t::Regional_Indicator) {
                state.emoji_state =
                    detail::word_break_emoji_state_t::first_emoji;
            }

            if (detail::table_word_break(state.prev_prop, state.prop))
                return state.it;
        }
        return last;
    }

}}

#endif
