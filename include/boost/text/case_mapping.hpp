// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_CASE_MAPPING_HPP
#define BOOST_TEXT_CASE_MAPPING_HPP

#include <boost/text/in_out_result.hpp>
#include <boost/text/word_break.hpp>
#include <boost/text/detail/case_constants.hpp>
#include <boost/text/detail/case_mapping_data.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/detail/lazy_segment_range.hpp>

#include <numeric>


namespace boost { namespace text {

    /** The list of languages handled specially in the case mapping
        functions. */
    enum class case_language : uint16_t {
        other,
        azerbaijani,
        lithuanian,
        turkish,
        dutch
    };

    /** A callable type that returns the next possible word break in `[first,
        last)`.  This is the default line break callable type used with the
        case mapping functions. */
    struct next_word_break_callable
    {
#ifdef BOOST_TEXT_DOXYGEN

        template<typename CPIter, typename Sentinel>
        CPIter operator()(CPIter it, Sentinel last) const;
#else

        template<typename CPIter, typename Sentinel>
        auto operator()(CPIter it, Sentinel last) const
            -> detail::cp_iter_ret_t<CPIter, CPIter>
        {
            return boost::text::next_word_break(it, last);
        }

#endif
    };

    /** An alias for `in_out_result` returned by algorithms that perform a
        case-mapping copy. */
    template<typename Iter, typename OutIter>
    using case_mapping_result = in_out_result<Iter, OutIter>;

    namespace detail {
        // Data and algorithm for handling Greek special-casing comes from
        // http://site.icu-project.org/design/case/greek-upper .
        enum greek_case_flags {
            has_vowel = 0x1000,
            has_ypogegrammeni = 0x2000,
            has_accent = 0x4000,
            has_dialytika = 0x8000,
            has_combining_dialytika = 0x10000,
            has_other_greek_diacritic = 0x20000
        };

        inline uint32_t greek_case_data(uint32_t cp)
        {
            // [U+0370, U+0400)
            constexpr static uint32_t _0370_0400[144] = {
                0x0370, // Ͱ
                0x0370, // ͱ
                0x0372, // Ͳ
                0x0372, // ͳ
                0,
                0,
                0x0376, // Ͷ
                0x0376, // ͷ
                0,
                0,
                0x037A, // ͺ
                0x03FD, // ͻ
                0x03FE, // ͼ
                0x03FF, // ͽ
                0,
                0x037F, // Ϳ
                0,
                0,
                0,
                0,
                0,
                0,
                0x0391 | has_vowel | has_accent, // Ά
                0,
                0x0395 | has_vowel | has_accent, // Έ
                0x0397 | has_vowel | has_accent, // Ή
                0x0399 | has_vowel | has_accent, // Ί
                0,
                0x039F | has_vowel | has_accent, // Ό
                0,
                0x03A5 | has_vowel | has_accent,                 // Ύ
                0x03A9 | has_vowel | has_accent,                 // Ώ
                0x0399 | has_vowel | has_accent | has_dialytika, // ΐ
                0x0391 | has_vowel,                              // Α
                0x0392,                                          // Β
                0x0393,                                          // Γ
                0x0394,                                          // Δ
                0x0395 | has_vowel,                              // Ε
                0x0396,                                          // Ζ
                0x0397 | has_vowel,                              // Η
                0x0398,                                          // Θ
                0x0399 | has_vowel,                              // Ι
                0x039A,                                          // Κ
                0x039B,                                          // Λ
                0x039C,                                          // Μ
                0x039D,                                          // Ν
                0x039E,                                          // Ξ
                0x039F | has_vowel,                              // Ο
                0x03A0,                                          // Π
                0x03A1,                                          // Ρ
                0,
                0x03A3,                                          // Σ
                0x03A4,                                          // Τ
                0x03A5 | has_vowel,                              // Υ
                0x03A6,                                          // Φ
                0x03A7,                                          // Χ
                0x03A8,                                          // Ψ
                0x03A9 | has_vowel,                              // Ω
                0x0399 | has_vowel | has_dialytika,              // Ϊ
                0x03A5 | has_vowel | has_dialytika,              // Ϋ
                0x0391 | has_vowel | has_accent,                 // ά
                0x0395 | has_vowel | has_accent,                 // έ
                0x0397 | has_vowel | has_accent,                 // ή
                0x0399 | has_vowel | has_accent,                 // ί
                0x03A5 | has_vowel | has_accent | has_dialytika, // ΰ
                0x0391 | has_vowel,                              // α
                0x0392,                                          // β
                0x0393,                                          // γ
                0x0394,                                          // δ
                0x0395 | has_vowel,                              // ε
                0x0396,                                          // ζ
                0x0397 | has_vowel,                              // η
                0x0398,                                          // θ
                0x0399 | has_vowel,                              // ι
                0x039A,                                          // κ
                0x039B,                                          // λ
                0x039C,                                          // μ
                0x039D,                                          // ν
                0x039E,                                          // ξ
                0x039F | has_vowel,                              // ο
                0x03A0,                                          // π
                0x03A1,                                          // ρ
                0x03A3,                                          // ς
                0x03A3,                                          // σ
                0x03A4,                                          // τ
                0x03A5 | has_vowel,                              // υ
                0x03A6,                                          // φ
                0x03A7,                                          // χ
                0x03A8,                                          // ψ
                0x03A9 | has_vowel,                              // ω
                0x0399 | has_vowel | has_dialytika,              // ϊ
                0x03A5 | has_vowel | has_dialytika,              // ϋ
                0x039F | has_vowel | has_accent,                 // ό
                0x03A5 | has_vowel | has_accent,                 // ύ
                0x03A9 | has_vowel | has_accent,                 // ώ
                0x03CF,                                          // Ϗ
                0x0392,                                          // ϐ
                0x0398,                                          // ϑ
                0x03D2,                                          // ϒ
                0x03D2 | has_accent,                             // ϓ
                0x03D2 | has_dialytika,                          // ϔ
                0x03A6,                                          // ϕ
                0x03A0,                                          // ϖ
                0x03CF,                                          // ϗ
                0x03D8,                                          // Ϙ
                0x03D8,                                          // ϙ
                0x03DA,                                          // Ϛ
                0x03DA,                                          // ϛ
                0x03DC,                                          // Ϝ
                0x03DC,                                          // ϝ
                0x03DE,                                          // Ϟ
                0x03DE,                                          // ϟ
                0x03E0,                                          // Ϡ
                0x03E0,                                          // ϡ
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0x039A,             // ϰ
                0x03A1,             // ϱ
                0x03F9,             // ϲ
                0x037F,             // ϳ
                0x03F4,             // ϴ
                0x0395 | has_vowel, // ϵ
                0,
                0x03F7, // Ϸ
                0x03F7, // ϸ
                0x03F9, // Ϲ
                0x03FA, // Ϻ
                0x03FA, // ϻ
                0x03FC, // ϼ
                0x03FD, // Ͻ
                0x03FE, // Ͼ
                0x03FF, // Ͽ
            };

            // [U+1F00, U+2000)
            constexpr static uint32_t _1f00_2000[256] = {
                0x0391 | has_vowel,              // ἀ
                0x0391 | has_vowel,              // ἁ
                0x0391 | has_vowel | has_accent, // ἂ
                0x0391 | has_vowel | has_accent, // ἃ
                0x0391 | has_vowel | has_accent, // ἄ
                0x0391 | has_vowel | has_accent, // ἅ
                0x0391 | has_vowel | has_accent, // ἆ
                0x0391 | has_vowel | has_accent, // ἇ
                0x0391 | has_vowel,              // Ἀ
                0x0391 | has_vowel,              // Ἁ
                0x0391 | has_vowel | has_accent, // Ἂ
                0x0391 | has_vowel | has_accent, // Ἃ
                0x0391 | has_vowel | has_accent, // Ἄ
                0x0391 | has_vowel | has_accent, // Ἅ
                0x0391 | has_vowel | has_accent, // Ἆ
                0x0391 | has_vowel | has_accent, // Ἇ
                0x0395 | has_vowel,              // ἐ
                0x0395 | has_vowel,              // ἑ
                0x0395 | has_vowel | has_accent, // ἒ
                0x0395 | has_vowel | has_accent, // ἓ
                0x0395 | has_vowel | has_accent, // ἔ
                0x0395 | has_vowel | has_accent, // ἕ
                0,
                0,
                0x0395 | has_vowel,              // Ἐ
                0x0395 | has_vowel,              // Ἑ
                0x0395 | has_vowel | has_accent, // Ἒ
                0x0395 | has_vowel | has_accent, // Ἓ
                0x0395 | has_vowel | has_accent, // Ἔ
                0x0395 | has_vowel | has_accent, // Ἕ
                0,
                0,
                0x0397 | has_vowel,              // ἠ
                0x0397 | has_vowel,              // ἡ
                0x0397 | has_vowel | has_accent, // ἢ
                0x0397 | has_vowel | has_accent, // ἣ
                0x0397 | has_vowel | has_accent, // ἤ
                0x0397 | has_vowel | has_accent, // ἥ
                0x0397 | has_vowel | has_accent, // ἦ
                0x0397 | has_vowel | has_accent, // ἧ
                0x0397 | has_vowel,              // Ἠ
                0x0397 | has_vowel,              // Ἡ
                0x0397 | has_vowel | has_accent, // Ἢ
                0x0397 | has_vowel | has_accent, // Ἣ
                0x0397 | has_vowel | has_accent, // Ἤ
                0x0397 | has_vowel | has_accent, // Ἥ
                0x0397 | has_vowel | has_accent, // Ἦ
                0x0397 | has_vowel | has_accent, // Ἧ
                0x0399 | has_vowel,              // ἰ
                0x0399 | has_vowel,              // ἱ
                0x0399 | has_vowel | has_accent, // ἲ
                0x0399 | has_vowel | has_accent, // ἳ
                0x0399 | has_vowel | has_accent, // ἴ
                0x0399 | has_vowel | has_accent, // ἵ
                0x0399 | has_vowel | has_accent, // ἶ
                0x0399 | has_vowel | has_accent, // ἷ
                0x0399 | has_vowel,              // Ἰ
                0x0399 | has_vowel,              // Ἱ
                0x0399 | has_vowel | has_accent, // Ἲ
                0x0399 | has_vowel | has_accent, // Ἳ
                0x0399 | has_vowel | has_accent, // Ἴ
                0x0399 | has_vowel | has_accent, // Ἵ
                0x0399 | has_vowel | has_accent, // Ἶ
                0x0399 | has_vowel | has_accent, // Ἷ
                0x039F | has_vowel,              // ὀ
                0x039F | has_vowel,              // ὁ
                0x039F | has_vowel | has_accent, // ὂ
                0x039F | has_vowel | has_accent, // ὃ
                0x039F | has_vowel | has_accent, // ὄ
                0x039F | has_vowel | has_accent, // ὅ
                0,
                0,
                0x039F | has_vowel,              // Ὀ
                0x039F | has_vowel,              // Ὁ
                0x039F | has_vowel | has_accent, // Ὂ
                0x039F | has_vowel | has_accent, // Ὃ
                0x039F | has_vowel | has_accent, // Ὄ
                0x039F | has_vowel | has_accent, // Ὅ
                0,
                0,
                0x03A5 | has_vowel,              // ὐ
                0x03A5 | has_vowel,              // ὑ
                0x03A5 | has_vowel | has_accent, // ὒ
                0x03A5 | has_vowel | has_accent, // ὓ
                0x03A5 | has_vowel | has_accent, // ὔ
                0x03A5 | has_vowel | has_accent, // ὕ
                0x03A5 | has_vowel | has_accent, // ὖ
                0x03A5 | has_vowel | has_accent, // ὗ
                0,
                0x03A5 | has_vowel, // Ὑ
                0,
                0x03A5 | has_vowel | has_accent, // Ὓ
                0,
                0x03A5 | has_vowel | has_accent, // Ὕ
                0,
                0x03A5 | has_vowel | has_accent, // Ὗ
                0x03A9 | has_vowel,              // ὠ
                0x03A9 | has_vowel,              // ὡ
                0x03A9 | has_vowel | has_accent, // ὢ
                0x03A9 | has_vowel | has_accent, // ὣ
                0x03A9 | has_vowel | has_accent, // ὤ
                0x03A9 | has_vowel | has_accent, // ὥ
                0x03A9 | has_vowel | has_accent, // ὦ
                0x03A9 | has_vowel | has_accent, // ὧ
                0x03A9 | has_vowel,              // Ὠ
                0x03A9 | has_vowel,              // Ὡ
                0x03A9 | has_vowel | has_accent, // Ὢ
                0x03A9 | has_vowel | has_accent, // Ὣ
                0x03A9 | has_vowel | has_accent, // Ὤ
                0x03A9 | has_vowel | has_accent, // Ὥ
                0x03A9 | has_vowel | has_accent, // Ὦ
                0x03A9 | has_vowel | has_accent, // Ὧ
                0x0391 | has_vowel | has_accent, // ὰ
                0x0391 | has_vowel | has_accent, // ά
                0x0395 | has_vowel | has_accent, // ὲ
                0x0395 | has_vowel | has_accent, // έ
                0x0397 | has_vowel | has_accent, // ὴ
                0x0397 | has_vowel | has_accent, // ή
                0x0399 | has_vowel | has_accent, // ὶ
                0x0399 | has_vowel | has_accent, // ί
                0x039F | has_vowel | has_accent, // ὸ
                0x039F | has_vowel | has_accent, // ό
                0x03A5 | has_vowel | has_accent, // ὺ
                0x03A5 | has_vowel | has_accent, // ύ
                0x03A9 | has_vowel | has_accent, // ὼ
                0x03A9 | has_vowel | has_accent, // ώ
                0,
                0,
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾀ
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾁ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾂ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾃ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾄ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾅ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾆ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾇ
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾈ
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾉ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾊ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾋ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾌ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾍ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾎ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾏ
                0x0397 | has_vowel | has_ypogegrammeni,              // ᾐ
                0x0397 | has_vowel | has_ypogegrammeni,              // ᾑ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾒ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾓ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾔ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾕ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾖ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾗ
                0x0397 | has_vowel | has_ypogegrammeni,              // ᾘ
                0x0397 | has_vowel | has_ypogegrammeni,              // ᾙ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾚ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾛ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾜ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾝ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾞ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ᾟ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ᾠ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ᾡ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾢ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾣ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾤ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾥ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾦ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾧ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ᾨ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ᾩ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾪ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾫ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾬ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾭ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾮ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ᾯ
                0x0391 | has_vowel,                                  // ᾰ
                0x0391 | has_vowel,                                  // ᾱ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾲ
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾳ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾴ
                0,
                0x0391 | has_vowel | has_accent,                     // ᾶ
                0x0391 | has_vowel | has_ypogegrammeni | has_accent, // ᾷ
                0x0391 | has_vowel,                                  // Ᾰ
                0x0391 | has_vowel,                                  // Ᾱ
                0x0391 | has_vowel | has_accent,                     // Ὰ
                0x0391 | has_vowel | has_accent,                     // Ά
                0x0391 | has_vowel | has_ypogegrammeni,              // ᾼ
                0,
                0x0399 | has_vowel, // ι
                0,
                0,
                0,
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ῂ
                0x0397 | has_vowel | has_ypogegrammeni,              // ῃ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ῄ
                0,
                0x0397 | has_vowel | has_accent,                     // ῆ
                0x0397 | has_vowel | has_ypogegrammeni | has_accent, // ῇ
                0x0395 | has_vowel | has_accent,                     // Ὲ
                0x0395 | has_vowel | has_accent,                     // Έ
                0x0397 | has_vowel | has_accent,                     // Ὴ
                0x0397 | has_vowel | has_accent,                     // Ή
                0x0397 | has_vowel | has_ypogegrammeni,              // ῌ
                0,
                0,
                0,
                0x0399 | has_vowel,                              // ῐ
                0x0399 | has_vowel,                              // ῑ
                0x0399 | has_vowel | has_accent | has_dialytika, // ῒ
                0x0399 | has_vowel | has_accent | has_dialytika, // ΐ
                0,
                0,
                0x0399 | has_vowel | has_accent,                 // ῖ
                0x0399 | has_vowel | has_accent | has_dialytika, // ῗ
                0x0399 | has_vowel,                              // Ῐ
                0x0399 | has_vowel,                              // Ῑ
                0x0399 | has_vowel | has_accent,                 // Ὶ
                0x0399 | has_vowel | has_accent,                 // Ί
                0,
                0,
                0,
                0,
                0x03A5 | has_vowel,                              // ῠ
                0x03A5 | has_vowel,                              // ῡ
                0x03A5 | has_vowel | has_accent | has_dialytika, // ῢ
                0x03A5 | has_vowel | has_accent | has_dialytika, // ΰ
                0x03A1,                                          // ῤ
                0x03A1,                                          // ῥ
                0x03A5 | has_vowel | has_accent,                 // ῦ
                0x03A5 | has_vowel | has_accent | has_dialytika, // ῧ
                0x03A5 | has_vowel,                              // Ῠ
                0x03A5 | has_vowel,                              // Ῡ
                0x03A5 | has_vowel | has_accent,                 // Ὺ
                0x03A5 | has_vowel | has_accent,                 // Ύ
                0x03A1,                                          // Ῥ
                0,
                0,
                0,
                0,
                0,
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ῲ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ῳ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ῴ
                0,
                0x03A9 | has_vowel | has_accent,                     // ῶ
                0x03A9 | has_vowel | has_ypogegrammeni | has_accent, // ῷ
                0x039F | has_vowel | has_accent,                     // Ὸ
                0x039F | has_vowel | has_accent,                     // Ό
                0x03A9 | has_vowel | has_accent,                     // Ὼ
                0x03A9 | has_vowel | has_accent,                     // Ώ
                0x03A9 | has_vowel | has_ypogegrammeni,              // ῼ
                0,
                0,
                0,
            };

            if (cp < 0x370 || (0x400 <= cp && cp < 0x1f00) ||
                (0x2000 <= cp && cp != 0x2126)) {
                return 0;
            } else if (cp < 0x400) {
                return _0370_0400[cp - 0x370];
            } else if (cp < 0x2000) {
                return _1f00_2000[cp - 0x1f00];
            } else {
                return 0x03A9 | has_vowel; // Ω
            }
        }

        inline uint32_t greek_diacritic_data(uint32_t cp)
        {
            switch (cp) {
            case 0x0300:
            case 0x0301:
            case 0x0302:
            case 0x0303:
            case 0x0311:
            case 0x0342: return has_accent;
            case 0x0308: return has_combining_dialytika;
            case 0x0344: return has_combining_dialytika | has_accent;
            case 0x0345: return has_ypogegrammeni;
            case 0x0304:
            case 0x0306:
            case 0x0313:
            case 0x0314:
            case 0x0343: return has_other_greek_diacritic;
            default: return 0;
            }
            return 0;
        }

        inline uint16_t lang_to_condition(case_language lang)
        {
            switch (lang) {
            case case_language::azerbaijani:
                return (uint16_t)detail::case_condition::az;
            case case_language::lithuanian:
                return (uint16_t)detail::case_condition::lt;
            case case_language::turkish:
                return (uint16_t)detail::case_condition::tr;
            default: return 0;
            }
            return 0;
        }

        inline bool ccc_230_0(uint32_t cp)
        {
            auto const ccc_ = detail::ccc(cp);
            return ccc_ == 230 || ccc_ == 0;
        }

        enum class title_state_t {
            none,        // Not doing title-casing.
            before,      // The first CP.
            right_after, // The second CP.
            after        // Any subsequent CP.
        };

        enum class map_case_mode { lower, title, upper };
        enum class final_sigma_state_t { none, after, before_after };

        template<typename CPIter, typename Sentinel, typename OutIter>
        case_mapping_result<CPIter, OutIter> map_case(
            CPIter first,
            CPIter it,
            Sentinel last,
            OutIter out,
            case_language lang,
            case_map_t const & map,
            map_case_mode mode)
        {
            uint16_t const lang_conditions = lang_to_condition(lang);

            title_state_t title_state = mode == map_case_mode::title
                                            ? title_state_t::before
                                            : title_state_t::none;

            bool const special_dutch_case = lang == case_language::dutch &&
                                            it != last &&
                                            (*it == 'i' || *it == 'I');

            auto after_vowel_with_accent = [first](CPIter it) {
                if (it == first)
                    return false;
                return (detail::greek_case_data(*detail::prev(it)) &
                        (has_vowel | has_accent | has_dialytika)) ==
                       (has_vowel | has_accent);
            };

            // Returns after, before_after, or none if only the
            // after-condition, both conditions, or neither condition is
            // found.
            auto final_sigma = [first,
                                last](CPIter it, CPIter condition_first) {
                auto find_it = boost::text::find_if(
                    condition_first, last, [](uint32_t cp) {
                        return !detail::case_ignorable(cp);
                    });
                if (find_it != last && detail::cased(*find_it))
                    return final_sigma_state_t::none;

                find_it =
                    boost::text::find_if_backward(first, it, [](uint32_t cp) {
                        return !detail::case_ignorable(cp);
                    });
                if (find_it != it && detail::cased(*find_it))
                    return final_sigma_state_t::before_after;

                return final_sigma_state_t::after;
            };

            auto advance_state = [&title_state]() {
                if (title_state == title_state_t::right_after)
                    title_state = title_state_t::after;
                if (title_state == title_state_t::before)
                    title_state = title_state_t::right_after;
            };

            for (; it != last; advance_state()) {
                if (special_dutch_case &&
                    title_state == title_state_t::right_after &&
                    (*it == 'j' || *it == 'J')) {
                    *out = uint32_t('J');
                    ++out;
                    ++it;
                    continue;
                }

                auto const condition_first = detail::next(it);

                uint32_t greek_data = mode == map_case_mode::upper
                                          ? detail::greek_case_data(*it)
                                          : 0;
                if (greek_data) {
                    uint32_t upper_masked = greek_data & 0x3ff;
                    if ((greek_data & has_vowel) &&
                        (upper_masked == 0x399 || upper_masked == 0x3a5) &&
                        after_vowel_with_accent(it)) {
                        greek_data |= has_dialytika;
                    }

                    int ypogegrammeni = greek_data & has_ypogegrammeni;
                    auto next = condition_first;
                    for (; next != last; ++next) {
                        uint32_t const diacritic_data =
                            detail::greek_diacritic_data(*next);
                        if (!diacritic_data)
                            break;
                        greek_data |= diacritic_data;
                        if (diacritic_data == 0x0345) // has_ypogegrammeni
                            ++ypogegrammeni;
                    }

                    uint32_t const has_either_dialytika =
                        has_dialytika | has_combining_dialytika;

                    bool add_tonos = false;
                    if (upper_masked == 0x0397 && (greek_data & has_accent) &&
                        !ypogegrammeni &&
                        final_sigma(it, condition_first) ==
                            final_sigma_state_t::after) {
                        add_tonos = true;
                    } else if (greek_data & has_dialytika) {
                        if (upper_masked == 0x0399) {
                            upper_masked = 0x03aa;
                            greek_data &= ~has_either_dialytika;
                        } else if (upper_masked == 0x03a5) {
                            upper_masked = 0x03ab;
                            greek_data &= ~has_either_dialytika;
                        }
                    }

                    *out = upper_masked;
                    ++out;
                    if (greek_data & has_either_dialytika) {
                        *out = 0x0308;
                        ++out;
                    }
                    if (greek_data & add_tonos) {
                        *out = 0x0301;
                        ++out;
                    }
                    out = std::fill_n(out, ypogegrammeni, 0x0399);

                    it = next;
                    continue;
                }

                auto const map_it = title_state_t::right_after <= title_state
                                        ? to_lower_map().find(*it)
                                        : map.find(*it);
                auto const map_last = title_state_t::right_after <= title_state
                                          ? to_lower_map().end()
                                          : map.end();

                uint16_t conditions = lang_conditions;

                if (map_it != map_last) {
                    case_elements const elements = map_it->second;
                    auto const elem_first =
                        elements.begin(case_mapping_to_ptr());
                    auto const elem_last = elements.end(case_mapping_to_ptr());

                    auto const all_conditions = std::accumulate(
                        elem_first,
                        elem_last,
                        uint16_t(0),
                        [](uint16_t conds, case_mapping_to to) {
                            return conds | to.conditions_;
                        });

                    if ((all_conditions &
                         (uint16_t)case_condition::Final_Sigma) &&
                        final_sigma(it, condition_first) ==
                            final_sigma_state_t::before_after) {
                        conditions |= (uint16_t)case_condition::Final_Sigma;
                    }

                    if (all_conditions &
                        (uint16_t)case_condition::After_Soft_Dotted) {
                        auto find_it = boost::text::find_if_backward(
                            first, it, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != it) {
                            if (detail::soft_dotted(*find_it)) {
                                conditions |=
                                    (uint16_t)case_condition::After_Soft_Dotted;
                            }
                        }
                    }

                    if (all_conditions & (uint16_t)case_condition::More_Above) {
                        if (condition_first != last &&
                            detail::ccc(*condition_first) == 230) {
                            conditions |= (uint16_t)case_condition::More_Above;
                        } else {
                            auto find_it = boost::text::find_if(
                                condition_first, last, [](uint32_t cp) {
                                    return ccc_230_0(cp);
                                });
                            if (find_it != last) {
                                if (detail::ccc(*find_it) == 230) {
                                    conditions |=
                                        (uint16_t)case_condition::More_Above;
                                }
                            }
                        }
                    }

                    if (all_conditions &
                        (uint16_t)case_condition::Not_Before_Dot) {
                        bool before = false;
                        auto find_it = boost::text::find_if(
                            condition_first, last, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != last) {
                            if (*find_it == 0x0307)
                                before = true;
                        }
                        if (!before) {
                            conditions |=
                                (uint16_t)case_condition::Not_Before_Dot;
                        }
                    }

                    if (all_conditions & (uint16_t)case_condition::After_I) {
                        auto find_it = boost::text::find_if_backward(
                            first, it, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != it) {
                            if (*find_it == 0x0049)
                                conditions |= (uint16_t)case_condition::After_I;
                        }
                    }

                    bool met_conditions = false;
                    for (auto elem_it = elem_first; elem_it != elem_last;
                         ++elem_it) {
                        // This condition only works (that is, it correctly
                        // prefers matches with conditions) because the data
                        // generation puts the condition=0 elements at the
                        // end.
                        if ((elem_it->conditions_ & conditions) ==
                            elem_it->conditions_) {
                            auto const cp_first =
                                case_cps_ptr() + elem_it->first_;
                            auto const cp_last =
                                case_cps_ptr() + elem_it->last_;
                            out = std::copy(cp_first, cp_last, out);
                            it = condition_first;
                            met_conditions = true;
                            break;
                        }
                    }
                    if (met_conditions)
                        continue;
                }

                *out = *it;
                ++out;
                ++it;
            }

            return {it, out};
        }
    }
}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns true if no code point in `[first, last)` would change in a
        call to to_lower(), and false otherwise. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool is_lower(I first, S last);

    /** Returns true if no code point in `r` would change in a call to
        to_lower(), and false otherwise. */
    template<code_point_range R>
    bool is_lower(R && r);

    /** Returns true if no grapheme in `r` would change in a call to
        to_lower(), and false otherwise. */
    template<grapheme_range R>
    bool is_lower(R && r);

    /** Writes the code point sequence comprising the lower-case form of
        `[first, last)` to `out`, using language-specific handling as
        indicated by `lang`.  The normalization of the result is undefined. */
    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires std::indirectly_copyable<I, O>
    case_mapping_result<I, O>
    to_lower(I first, S last, O out, case_language lang = case_language::other);

    /** Writes the code point sequence comprising the lower-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined. */
    template<code_point_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_lower(R && r, O out, case_language lang = case_language::other);

    /** Writes the code point sequence comprising the lower-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined.  Returns a
        `case_mapping_result`.  Note that the type of the result's `.in`
        member may not be `std::ranges::iterator_t<R>`, since the result may
        be a new iterator that results from reconstituting unpacked iterators
        out of the input.  The type of `.in` will model `grapheme_iter`.  The
        `.in` member of the result is a `borrowed_iterator_t` in C++20 and
        later. */
    template<grapheme_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<code_point_iterator_t<R>, O>
    detail::unspecified
    to_lower(R && r, O out, case_language lang = case_language::other);

    /** Returns true if no code point in `[first, last)` would change in a
        call to to_title(), and false otherwise. */
    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        I first,
        S last,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Returns true if no code point in `r` would change in a call to
        to_title(), and false otherwise. */
    template<
        code_point_range R,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        R && r,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Returns true if no grapheme in `r` would change in a call to
        to_title(), and false otherwise. */
    template<
        grapheme_range R,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        R && r,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Writes the code point sequence comprising the title-case form of
        `[first, last)` to `out`, using language-specific handling as
        indicated by `lang`.  The normalization of the result is undefined. */
    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires std::indirectly_copyable<I, O>
    case_mapping_result<I, O>
    to_title(
        I first,
        S last,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Writes the code point sequence comprising the title-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined. */
    template<
        code_point_range R,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires std::indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_title(
        R && r,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Writes the code point sequence comprising the title-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined.  Returns a
        `case_mapping_result`.  Note that the type of the result's `.in`
        member may not be `std::ranges::iterator_t<R>`, since the result may
        be a new iterator that results from reconstituting unpacked iterators
        out of the input.  The type of `.in` will model `grapheme_iter`.  The
        `.in` member of the result is a `borrowed_iterator_t` in C++20 and
        later. */
    template<
        grapheme_range R,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires std::indirectly_copyable<code_point_iterator_t<R>, O>
    detail::unspecified to_title(
        R && r,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{});

    /** Returns true if no code point in `[first, last)` would change in a
        call to to_upper(), and false otherwise. */
    template<code_point_iter I, std::sentinel_for<I> S>
    bool is_upper(I first, S last);

    /** Returns true if no code point in `r` would change in a call to
        to_upper(), and false otherwise. */
    template<code_point_range R>
    bool is_upper(R && r);

    /** Returns true if no grapheme in `r` would change in a call to
        to_upper(), and false otherwise. */
    template<grapheme_range R>
    bool is_upper(R && r);

    /** Writes code point sequence comprising the the upper-case form of
        `[first, last)` to `out`, using language-specific handling as
        indicated by `lang`.  The normalization of the result is undefined. */
    template<code_point_iter I, std::sentinel_for<I> S, std::weakly_incrementable O>
    requires std::indirectly_copyable<I, O>
    case_mapping_result<I, O>
    to_upper(I first, S last, O out, case_language lang = case_language::other);

    /** Writes code point sequence comprising the the upper-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined. */
    template<code_point_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_upper(R && r, O out, case_language lang = case_language::other);

    /** Writes the code point sequence comprising the upper-case form of `r`
        to `out`, using language-specific handling as indicated by `lang`.
        The normalization of the result is undefined.  Returns a
        `case_mapping_result`.  Note that the type of the result's `.in`
        member may not be `std::ranges::iterator_t<R>`, since the result may
        be a new iterator that results from reconstituting unpacked iterators
        out of the input.  The type of `.in` will model `grapheme_iter`.  The
        `.in` member of the result is a `borrowed_iterator_t` in C++20 and
        later. */
    template<grapheme_range R, std::weakly_incrementable O>
    requires std::indirectly_copyable<code_point_iterator_t<R>, O>
    detail::unspecified
    to_upper(R && r, O out, case_language lang = case_language::other);

#else

    template<typename CPIter, typename Sentinel>
    bool is_lower(CPIter first, Sentinel last)
    {
        return boost::text::all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_lowered(cp);
        });
    }

    template<typename CPRange>
    auto is_lower(CPRange && range)
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return v1::is_lower(detail::begin(range), detail::end(range));
    }

    template<typename GraphemeRange>
    auto is_lower(GraphemeRange && range)
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return v1::is_lower(range.begin().base(), range.end().base());
    }

    template<typename CPIter, typename Sentinel, typename OutIter>
    auto to_lower(
        CPIter first,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other)
    {
        return map_case(
            first,
            first,
            last,
            out,
            lang,
            detail::to_lower_map(),
            detail::map_case_mode::lower);
    }

    template<typename CPRange, typename OutIter>
    auto to_lower(
        CPRange && range,
        OutIter out,
        case_language lang = case_language::other)
        ->detail::cp_rng_alg_ret_t<
            case_mapping_result<detail::iterator_t<CPRange>, OutIter>,
            CPRange>
    {
        return v1::to_lower(
            detail::begin(range), detail::end(range), out, lang);
    }

    template<
        typename GraphemeRange,
        typename OutIter,
        typename Enable = detail::graph_rng_alg_ret_t<void, GraphemeRange>>
    auto to_lower(
        GraphemeRange && range,
        OutIter out,
        case_language lang = case_language::other)
    {
        auto result =
            v1::to_lower(range.begin().base(), range.end().base(), out, lang);
        grapheme_iterator<
            decltype(range.begin().base()),
            decltype(range.end().base())>
            in(range.begin().base(), result.in, range.end().base());
        return case_mapping_result<decltype(in), OutIter>{
            in, std::move(result.out)};
    }

    template<
        typename CPIter,
        typename Sentinel,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        CPIter first,
        Sentinel last,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        NextWordBreakFunc next;
        detail::lazy_segment_range<CPIter, Sentinel, NextWordBreakFunc> words{
            std::move(next), {first, last}, {last}};
        for (auto r : words) {
            auto it = r.begin();
            if (detail::changes_when_titled(*it))
                return false;
            if (!is_lower(++it, r.end()))
                return false;
        }
        return true;
    }

    template<
        typename CPRange,
        typename NextWordBreakFunc = next_word_break_callable>
    auto is_title(
        CPRange && range,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return v1::is_title(
            detail::begin(range), detail::end(range), next_word_break);
    }

    template<
        typename GraphemeRange,
        typename NextWordBreakFunc = next_word_break_callable>
    auto is_title(
        GraphemeRange && range,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return v1::is_title(
            range.begin().base(), range.end().base(), next_word_break);
    }

    template<
        typename CPIter,
        typename Sentinel,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable>
    auto to_title(
        CPIter first,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        NextWordBreakFunc next;
        detail::lazy_segment_range<CPIter, Sentinel, NextWordBreakFunc> words{
            std::move(next), {first, last}, {last}};

        CPIter in = first;
        for (auto r : words) {
            in = r.end();
            out = map_case(
                first,
                r.begin(),
                r.end(),
                out,
                lang,
                detail::to_title_map(),
                detail::map_case_mode::title).out;
        }

        return case_mapping_result<CPIter, OutIter>{in, std::move(out)};
    }

    template<
        typename CPRange,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable>
    auto to_title(
        CPRange && range,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
        ->detail::cp_rng_alg_ret_t<
            case_mapping_result<detail::iterator_t<CPRange>, OutIter>,
            CPRange>
    {
        return v1::to_title(
            detail::begin(range),
            detail::end(range),
            out,
            lang,
            next_word_break);
    }

    template<
        typename GraphemeRange,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable,
        typename Enable = detail::graph_rng_alg_ret_t<void, GraphemeRange>>
    auto to_title(
        GraphemeRange && range,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        auto result = v1::to_title(
            range.begin().base(),
            range.end().base(),
            out,
            lang,
            next_word_break);
        grapheme_iterator<
            decltype(range.begin().base()),
            decltype(range.end().base())>
            in(range.begin().base(), result.in, range.end().base());
        return case_mapping_result<decltype(in), OutIter>{
            in, std::move(result.out)};
    }

    template<typename CPIter, typename Sentinel>
    bool is_upper(CPIter first, Sentinel last)
    {
        return boost::text::all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_uppered(cp);
        });
    }

    template<typename CPRange>
    auto is_upper(CPRange && range)
        -> detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return v1::is_upper(detail::begin(range), detail::end(range));
    }

    template<typename GraphemeRange>
    auto is_upper(GraphemeRange && range)
        -> detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return v1::is_upper(range.begin().base(), range.end().base());
    }

    template<typename CPIter, typename Sentinel, typename OutIter>
    auto to_upper(
        CPIter first,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other)
    {
        return detail::map_case(
            first,
            first,
            last,
            out,
            lang,
            detail::to_upper_map(),
            detail::map_case_mode::upper);
    }

    template<typename CPRange, typename OutIter>
    auto to_upper(
        CPRange && range,
        OutIter out,
        case_language lang = case_language::other)
        ->detail::cp_rng_alg_ret_t<
            case_mapping_result<detail::iterator_t<CPRange>, OutIter>,
            CPRange>
    {
        return v1::to_upper(
            detail::begin(range), detail::end(range), out, lang);
    }

    template<
        typename GraphemeRange,
        typename OutIter,
        typename Enable = detail::graph_rng_alg_ret_t<void, GraphemeRange>>
    auto to_upper(
        GraphemeRange && range,
        OutIter out,
        case_language lang = case_language::other)
    {
        auto result =
            v1::to_upper(range.begin().base(), range.end().base(), out, lang);
        grapheme_iterator<
            decltype(range.begin().base()),
            decltype(range.end().base())>
            in(range.begin().base(), result.in, range.end().base());
        return case_mapping_result<decltype(in), OutIter>{
            in, std::move(result.out)};
    }

#endif

}}}


#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<code_point_iter I, std::sentinel_for<I> S>
    bool is_lower(I first, S last)
    {
        return boost::text::all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_lowered(cp);
        });
    }

    template<code_point_range R>
    bool is_lower(R && r)
    {
        return v2::is_lower(detail::begin(r), detail::end(r));
    }

    template<grapheme_range R>
    bool is_lower(R && r)
    {
        return v2::is_lower(r.begin().base(), r.end().base());
    }

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires indirectly_copyable<I, O> case_mapping_result<I, O>
    to_lower(
        I first, S last, O out, case_language lang = case_language::other)
    {
        return map_case(
            first,
            first,
            last,
            out,
            lang,
            detail::to_lower_map(),
            detail::map_case_mode::lower);
    }

    template<code_point_range R, std::weakly_incrementable O>
    requires indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_lower(R && r, O out, case_language lang = case_language::other)
    {
        return v2::to_lower(detail::begin(r), detail::end(r), out, lang);
    }

    template<grapheme_range R, std::weakly_incrementable O>
    requires indirectly_copyable<code_point_iterator_t<R>, O>
    auto to_lower(R && r, O out, case_language lang = case_language::other)
    {
        auto [in_, out_] =
            v2::to_lower(r.begin().base(), r.end().base(), out, lang);
        if constexpr (std::ranges::borrowed_range<R>) {
            auto in = grapheme_iterator(r.begin().base(), in_, r.end().base());
            return case_mapping_result<decltype(in), O>{
                std::move(in), std::move(out_)};
        } else {
            return case_mapping_result<std::ranges::dangling, O>{
                std::ranges::dangling{}, std::move(out_)};
        }
    }

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        I first,
        S last,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        NextWordBreakFunc next;
        detail::lazy_segment_range<I, S, NextWordBreakFunc> words{
            std::move(next), {first, last}, {last}};
        for (auto r : words) {
            auto it = r.begin();
            if (detail::changes_when_titled(*it))
                return false;
            if (!is_lower(++it, r.end()))
                return false;
        }
        return true;
    }

    template<
        code_point_range R,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        R && r, NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        return v2::is_title(detail::begin(r), detail::end(r), next_word_break);
    }

    template<
        grapheme_range R,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        R && r, NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        return v2::is_title(r.begin().base(), r.end().base(), next_word_break);
    }

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires indirectly_copyable<I, O>
    case_mapping_result<I, O>
    to_title(
        I first,
        S last,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        NextWordBreakFunc next;
        detail::lazy_segment_range<I, S, NextWordBreakFunc> words{
            std::move(next), {first, last}, {last}};

        I in = first;
        for (auto r : words) {
            in = r.end();
            out = map_case(
                first,
                r.begin(),
                r.end(),
                out,
                lang,
                detail::to_title_map(),
                detail::map_case_mode::title).out;
        }

        return {in, out};
    }

    template<
        code_point_range R,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_title(
        R && r,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        return v2::to_title(
            detail::begin(r), detail::end(r), out, lang, next_word_break);
    }

    template<
        grapheme_range R,
        std::weakly_incrementable O,
        typename NextWordBreakFunc = next_word_break_callable>
    requires indirectly_copyable<code_point_iterator_t<R>, O>
    auto to_title(
        R && r,
        O out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{})
    {
        auto [in_, out_] = v2::to_title(
            r.begin().base(), r.end().base(), out, lang, next_word_break);
        if constexpr (std::ranges::borrowed_range<R>) {
            auto in = grapheme_iterator(r.begin().base(), in_, r.end().base());
            return case_mapping_result<decltype(in), O>{
                std::move(in), std::move(out_)};
        } else {
            return case_mapping_result<std::ranges::dangling, O>{
                std::ranges::dangling{}, std::move(out_)};
        }
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool is_upper(I first, S last)
    {
        return boost::text::all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_uppered(cp);
        });
    }

    template<code_point_range R>
    bool is_upper(R && r)
    {
        return v2::is_upper(detail::begin(r), detail::end(r));
    }

    template<grapheme_range R>
    bool is_upper(R && r)
    {
        return v2::is_upper(r.begin().base(), r.end().base());
    }

    template<
        code_point_iter I,
        std::sentinel_for<I> S,
        std::weakly_incrementable O>
    requires indirectly_copyable<I, O>
    case_mapping_result<I, O>
    to_upper(I first, S last, O out, case_language lang = case_language::other)
    {
        return detail::map_case(
            first,
            first,
            last,
            out,
            lang,
            detail::to_upper_map(),
            detail::map_case_mode::upper);
    }

    template<code_point_range R, std::weakly_incrementable O>
    requires indirectly_copyable<std::ranges::iterator_t<R>, O>
    case_mapping_result<std::ranges::borrowed_iterator_t<R>, O>
    to_upper(R && r, O out, case_language lang = case_language::other)
    {
        return v2::to_upper(detail::begin(r), detail::end(r), out, lang);
    }

    template<grapheme_range R, std::weakly_incrementable O>
    requires indirectly_copyable<code_point_iterator_t<R>, O>
    auto to_upper(R && r, O out, case_language lang = case_language::other)
    {
        auto [in_, out_] =
            v2::to_upper(r.begin().base(), r.end().base(), out, lang);
        if constexpr (std::ranges::borrowed_range<R>) {
            auto in = grapheme_iterator(r.begin().base(), in_, r.end().base());
            return case_mapping_result<decltype(in), O>{
                std::move(in), std::move(out_)};
        } else {
            return case_mapping_result<std::ranges::dangling, O>{
                std::ranges::dangling{}, std::move(out_)};
        }
    }
}}}

#endif

#endif
