#ifndef BOOST_TEXT_SENTENCE_BREAK_HPP
#define BOOST_TEXT_SENTENCE_BREAK_HPP

#include <array>

#include <cassert>
#include <stdint.h>


namespace boost { namespace text {

    /** The sentence properties outlined in Unicode 10. */
    enum class sentence_prop_t {
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

    /** Returns true if and only if the table from the Unicode Character
        Database indicates a break between \a lhs and \a rhs. */
    inline bool
    sentence_table_break(sentence_prop_t lhs, sentence_prop_t rhs) noexcept
    {
        // clang-format off
// See chart at http://www.unicode.org/Public/10.0.0/ucd/auxiliary/SentenceBreakTest.html
constexpr std::array<std::array<bool, 15>, 15> sentence_breaks = {{
//  Other CR LF Sep Sp Lwr Upr OLet Num ATrm STrm Cls SCont Fmt Ext
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Other
    {1,   1, 0, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}, // CR
    {1,   1, 1, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}, // LF

    {1,   1, 1, 1,  1, 1,  1,  1,   1,  1,   1,   1,  1,    1,  1}, // Sep
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Sp
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Lower

    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Upper
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // OLetter
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Number

    {1,   0, 0, 0,  0, 0,  1,  1,   0,  0,   0,   0,  0,    0,  0}, // ATerm
    {1,   0, 0, 0,  0, 1,  1,  1,   1,  0,   0,   0,  0,    0,  0}, // STerm
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Close

    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // SContinue
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Format
    {0,   0, 0, 0,  0, 0,  0,  0,   0,  0,   0,   0,  0,    0,  0}, // Extend
}};
        // clang-format on
        auto const lhs_int = static_cast<int>(lhs);
        auto const rhs_int = static_cast<int>(rhs);
        return sentence_breaks[lhs_int][rhs_int];
    }

}}

#endif
