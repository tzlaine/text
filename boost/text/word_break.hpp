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

    struct word_break_fsm
    {
        enum class state {
            use_table,
            ah_letter,     // WB7
            hebrew_letter, // WB7c
            numeric,       // WB11
            emoji_flag     // WB15, WB16
        };

        enum class result { use_table, break_, no_break, ignore };

        word_break_fsm() noexcept :
            state_(state::use_table),
            chars_ignored_(false)
        {}

        result update(
            word_prop_t prev_prop,
            word_prop_t prop,
            word_prop_t next_prop) noexcept
        {
            bool const ignored = chars_ignored_;
            chars_ignored_ = false;

            // WB3c, handled separately due to precedence before, and
            // interaction with, WB4.
            if (prev_prop == word_prop_t::ZWJ &&
                (prop == word_prop_t::Glue_After_Zwj ||
                 prop == word_prop_t::E_Base_GAZ)) {
                return ignored ? result::break_ : result::no_break;
            }

            // WB4: Except after line breaks, ignore (Extend | Format | ZWJ)*
            if (!linebreak(prev_prop) &&
                (prop == word_prop_t::Extend || prop == word_prop_t::Format ||
                 prop == word_prop_t::ZWJ)) {
                chars_ignored_ = true;
                return result::ignore;
            }

            // WB6
            if (ah_letter(prev_prop) && mid_ah(prop) &&
                /* This does not work because unbounded skipping is required
                   here for WB4 ignores. */
                ah_letter(next_prop)) {
                state_ = state::ah_letter;
                return result::no_break;
            }

            // WB7
            if (state_ == state::ah_letter) {
                if (ah_letter(prop)) {
                    state_ = state::use_table;
                    return result::no_break;
                } else {
                    state_ = state::use_table;
                }
            } else if (ah_letter(prev_prop) && mid_ah(prop)) {
                state_ = state::ah_letter;
            }

            // WB7c
            if (state_ == state::hebrew_letter) {
                if (prop == word_prop_t::Hebrew_Letter) {
                    state_ = state::use_table;
                    return result::no_break;
                } else {
                    state_ = state::use_table;
                }
            } else if (
                prev_prop == word_prop_t::Hebrew_Letter &&
                prop == word_prop_t::Double_Quote) {
                state_ = state::hebrew_letter;
            }

            // WB11
            if (state_ == state::numeric) {
                if (prop == word_prop_t::Numeric) {
                    state_ = state::use_table;
                    return result::no_break;
                } else {
                    state_ = state::use_table;
                }
            } else if (prev_prop == word_prop_t::Numeric && mid_num(prop)) {
                state_ = state::ah_letter;
            }

            // WB12
            if (prev_prop == word_prop_t::Numeric && mid_num(prop) &&
                next_prop == word_prop_t::Numeric) {
                return result::no_break;
            }

            if (state_ == state::emoji_flag) {
                if (prop == word_prop_t::Regional_Indicator) {
                    state_ = state::use_table;
                    return result::no_break;
                } else {
                    state_ = state::use_table;
                }
            } else if (prop == word_prop_t::Regional_Indicator) {
                state_ = state::emoji_flag;
            }

            return result::use_table;
        }

    private:
        static bool linebreak(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::CR ||
                   prop == word_prop_t::LF ||
                   prop == word_prop_t::Newline;
        }

        static bool ah_letter(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::ALetter ||
                   prop == word_prop_t::Hebrew_Letter;
        }

        // Corresonds to (MidLetter | MidNumLetQ) in WB6 and WB7
        static bool mid_ah(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::MidLetter ||
                   prop == word_prop_t::MidNumLet ||
                   prop == word_prop_t::Single_Quote;
        }

        // Corresonds to (MidNum | MidNumLetQ) in WB6 and WB7
        static bool mid_num(word_prop_t prop) noexcept
        {
            return prop == word_prop_t::MidNum ||
                   prop == word_prop_t::MidNumLet ||
                   prop == word_prop_t::Single_Quote;
        }

        state state_;
        bool chars_ignored_;
    };

    struct word_break_t
    {
        word_break_t() noexcept :
            break_(false),
            prev_prop_(word_prop_t::LF),
            prop_(word_prop_t::LF)
        {}
        word_break_t(
            bool b, word_prop_t pp, word_prop_t p, word_break_fsm f) noexcept :
            break_(b),
            prev_prop_(pp),
            prop_(p),
            fsm_(f)
        {}

        operator bool() const noexcept { return break_; }

        bool break_;
        word_prop_t prev_prop_; // The one just processed.
        word_prop_t prop_;      // The next to be processed.
        word_break_fsm fsm_;
    };

    /** Returns the word property associated with code point \a cp. */
    word_prop_t word_prop(uint32_t cp) noexcept;

    /** Returns true if and only if the table from the Unicode Character
        Database indicates a break between \a lhs and \a rhs. */
    inline word_break_t
    word_break(word_break_fsm fsm, word_prop_t prev_prop, word_prop_t prop, uint32_t cp) noexcept
    {
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

        auto const next_prop = word_prop(cp);
        auto const result = fsm.update(prev_prop, prop, next_prop);
        if (result == word_break_fsm::result::ignore) {
            return word_break_t(false, prev_prop, next_prop, fsm);
        } else if (result == word_break_fsm::result::break_) {
            return word_break_t(true, prop, next_prop, fsm);
        } else if (result == word_break_fsm::result::no_break) {
            return word_break_t(false, prop, next_prop, fsm);
        } else {
            auto const prev_prop_int = static_cast<int>(prev_prop);
            auto const prop_int = static_cast<int>(prop);
            return word_break_t(word_breaks[prev_prop_int][prop_int], prop, next_prop, fsm);
        }
    }

}}

#endif
