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

        enum class state_t {
            use_table,
            emoji_flag     // WB15, WB16
        };
    }

    template<typename CPIter>
    inline CPIter next_word_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        if (it == last)
            return last;

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

        auto prev_prev_prop = word_prop_t::Other;
        if (it != first)
            prev_prev_prop = word_prop(*std::prev(it));
        auto prev_prop = word_prop(*it);
        auto prop = word_prop(*++it);

        auto state = detail::state_t::use_table;

        // Back up to the first RI so we can see what state we're supposed to
        // be in here.
        if (prop == word_prop_t::Regional_Indicator) {
            auto ri_it = it;
            int ris_before = 0;
            while (ri_it != first) {
                if (word_prop(--ri_it) == word_prop_t::Regional_Indicator)
                    ++ris_before;
                else
                    break;
            }
            if (ris_before % 2 == 1)
                state = detail::state_t::emoji_flag;
        }

        for (; it != last; prev_prev_prop = prev_prop,
                           prev_prop = prop,
                           prop = word_prop(*++it)) {
            // WB3
            if (prev_prop == word_prop_t::CR && prop == word_prop_t::LF)
                continue;

            // WB3a
            if (prev_prop == word_prop_t::CR || prev_prop == word_prop_t::LF ||
                prev_prop == word_prop_t::Newline) {
                return it;
            }

            // WB3b
            if (prop == word_prop_t::CR || prop == word_prop_t::LF ||
                prop == word_prop_t::Newline) {
                return it;
            }

            // WB3c
            if (prev_prop == word_prop_t::ZWJ &&
                (prop == word_prop_t::Glue_After_Zwj ||
                 prop == word_prop_t::E_Base_GAZ)) {
                continue;
            }

            // WB4: Except after line breaks, ignore/skip (Extend | Format |
            // ZWJ)*
            if (!linebreak(prev_prop)) {
                while (it != last && (prop == word_prop_t::Extend ||
                                      prop == word_prop_t::Format ||
                                      prop == word_prop_t::ZWJ)) {
                    prop = word_prop(*++it);
                }
                if (it == last)
                    return last;
            }

            // WB6
            if (std::next(it) != last) {
                auto next_prop = word_prop(*std::next(it));
                if (detail::ah_letter(prev_prop) && detail::mid_ah(prop) &&
                    detail::ah_letter(next_prop)) {
                    continue;
                }
            }

            // WB7
            if (detail::ah_letter(prev_prev_prop) &&
                detail::mid_ah(prev_prop) && detail::ah_letter(prop)) {
                continue;
            }

            // WB7c
            if (prev_prev_prop == word_prop_t::Hebrew_Letter &&
                prev_prop == word_prop_t::Double_Quote &&
                prop == word_prop_t::Hebrew_Letter) {
                continue;
            }

            // WB11
            if (prev_prev_prop == word_prop_t::Numeric &&
                detail::mid_num(prev_prop) && prop == word_prop_t::Numeric) {
                continue;
            }

            // WB12
            if (std::next(it) != last) {
                auto next_prop = word_prop(*std::next(it));
                if (prev_prop == word_prop_t::Numeric &&
                    detail::mid_num(prop) &&
                    next_prop == word_prop_t::Numeric) {
                    continue;
                }
            }

            if (state == detail::state_t::emoji_flag) {
                if (prop == word_prop_t::Regional_Indicator) {
                    state = detail::state_t::use_table;
                    continue;
                } else {
                    state = detail::state_t::use_table;
                }
            } else if (prop == word_prop_t::Regional_Indicator) {
                state = detail::state_t::emoji_flag;
            }

            if (state == detail::state_t::use_table) {
                auto const prev_prop_int = static_cast<int>(prev_prop);
                auto const prop_int = static_cast<int>(prop);
                if (word_breaks[prev_prop_int][prop_int])
                    return it;
            }
        }
    }

}}

#endif
