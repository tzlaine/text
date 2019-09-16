// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
*******************************************************************************
*
*   Copyright (C) 1999-2012, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  utf16.h
*   encoding:   UTF-8
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999sep09
*   created by: Markus W. Scherer
*/

/**
 * \file
 * \brief C API: 16-bit Unicode handling macros
 *
 * This file defines macros to deal with 16-bit Unicode (UTF-16) code units and
 * strings.
 *
 * For more information see utf.h and the ICU User Guide Strings chapter
 * (http://userguide.icu-project.org/strings).
 *
 * <em>Usage:</em>
 * ICU coding guidelines for if() statements should be followed when using these
 * macros. Compound statements (curly braces {}) must be used  for
 * if-else-while... bodies and all macro statements should be terminated with
 * semicolon.
 */

#ifndef UTF16_H_
#define UTF16_H_

#include <boost/text/detail/icu/utf.hpp>


namespace boost { namespace text { namespace detail { namespace icu {

    /* single-code point definitions
     * -------------------------------------------- */

    /**
     * Does this code unit alone encode a code point (BMP, not a surrogate)?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U16_IS_SINGLE(UChar32 c) { return !U_IS_SURROGATE(c); }

    /**
     * Is this code unit a lead surrogate (U+d800..U+dbff)?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U16_IS_LEAD(UChar32 c) { return (((c)&0xfffffc00) == 0xd800); }

    /**
     * Is this code unit a trail surrogate (U+dc00..U+dfff)?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U16_IS_TRAIL(UChar32 c) { return (((c)&0xfffffc00) == 0xdc00); }

    /**
     * Is this code unit a surrogate (U+d800..U+dfff)?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U16_IS_SURROGATE(UChar32 c) { return U_IS_SURROGATE(c); }

    /**
     * Assuming c is a surrogate code point (U16_IS_SURROGATE(c)),
     * is it a lead surrogate?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U16_IS_SURROGATE_LEAD(UChar32 c) { return (((c)&0x400) == 0); }

    /**
     * Assuming c is a surrogate code point (U16_IS_SURROGATE(c)),
     * is it a trail surrogate?
     * @param c 16-bit code unit
     * @return TRUE or FALSE
     * @stable ICU 4.2
     */
    inline bool U16_IS_SURROGATE_TRAIL(UChar32 c) { return (((c)&0x400) != 0); }

    /**
     * Helper constant for U16_GET_SUPPLEMENTARY.
     * @internal
     */
    namespace {
        constexpr auto U16_SURROGATE_OFFSET =
            (0xd800 << 10UL) + 0xdc00 - 0x10000;
    }

    /**
     * Get a supplementary code point value (U+10000..U+10ffff)
     * from its lead and trail surrogates.
     * The result is undefined if the input values are not
     * lead and trail surrogates.
     *
     * @param lead lead surrogate (U+d800..U+dbff)
     * @param trail trail surrogate (U+dc00..U+dfff)
     * @return supplementary code point (U+10000..U+10ffff)
     * @stable ICU 2.4
     */
    inline UChar32 U16_GET_SUPPLEMENTARY(UChar lead, UChar trail)
    {
        return ((UChar32)(lead) << 10UL) +
               (UChar32)(trail)-U16_SURROGATE_OFFSET;
    }


    /**
     * Get the lead surrogate (0xd800..0xdbff) for a
     * supplementary code point (0x10000..0x10ffff).
     * @param supplementary 32-bit code point (U+10000..U+10ffff)
     * @return lead surrogate (U+d800..U+dbff) for supplementary
     * @stable ICU 2.4
     */
    inline UChar U16_LEAD(UChar32 supplementary)
    {
        return (UChar)(((supplementary) >> 10) + 0xd7c0);
    }

    /**
     * Get the trail surrogate (0xdc00..0xdfff) for a
     * supplementary code point (0x10000..0x10ffff).
     * @param supplementary 32-bit code point (U+10000..U+10ffff)
     * @return trail surrogate (U+dc00..U+dfff) for supplementary
     * @stable ICU 2.4
     */
    inline UChar U16_TRAIL(UChar32 supplementary)
    {
        return (UChar)(((supplementary)&0x3ff) | 0xdc00);
    }

    /**
     * How many 16-bit code units are used to encode this Unicode code point? (1
     * or 2) The result is not defined if c is not a Unicode code point
     * (U+0000..U+10ffff).
     * @param c 32-bit code point
     * @return 1 or 2
     * @stable ICU 2.4
     */
    inline int U16_LENGTH(UChar32 c)
    {
        return ((uint32_t)(c) <= 0xffff ? 1 : 2);
    }


    /* definitions with forward iteration
     * --------------------------------------- */

    /**
     * Get a code point from a string at a code point boundary offset,
     * and advance the offset to the next code point boundary.
     * (Post-incrementing forward iteration.)
     * "Unsafe" macro, assumes well-formed UTF-16.
     *
     * The offset may point to the lead surrogate unit
     * for a supplementary code point, in which case the macro will read
     * the following trail surrogate as well.
     * If the offset points to a trail surrogate, then that itself
     * will be returned as the code point.
     * The result is undefined if the offset points to a single, unpaired lead
     * surrogate.
     *
     * @param s const UChar * string
     * @param i string offset
     * @param c output UChar32 variable
     * @see U16_NEXT
     * @stable ICU 2.4
     */
    template<typename Iter>
    void U16_NEXT_UNSAFE(Iter s, int & i, UChar32 & c)
    {
        (c) = (s)[(i)++];
        if (U16_IS_LEAD(c)) {
            (c) = U16_GET_SUPPLEMENTARY((c), (s)[(i)++]);
        }
    }

    /**
     * Get a code point from a string at a code point boundary offset,
     * and advance the offset to the next code point boundary.
     * (Post-incrementing forward iteration.)
     * "Safe" macro, handles unpaired surrogates and checks for string
     * boundaries.
     *
     * The length can be negative for a NUL-terminated string.
     *
     * The offset may point to the lead surrogate unit
     * for a supplementary code point, in which case the macro will read
     * the following trail surrogate as well.
     * If the offset points to a trail surrogate or
     * to a single, unpaired lead surrogate, then c is set to that unpaired
     * surrogate.
     *
     * @param s const UChar * string
     * @param i string offset, must be i<length
     * @param length string length
     * @param c output UChar32 variable
     * @see U16_NEXT_UNSAFE
     * @stable ICU 2.4
     */
    template<typename Iter>
    void U16_NEXT(Iter s, int & i, int length, UChar32 & c)
    {
        (c) = (s)[(i)++];
        if (U16_IS_LEAD(c)) {
            uint16_t c2_;
            if ((i) != (length) && U16_IS_TRAIL(c2_ = (s)[(i)])) {
                ++(i);
                (c) = U16_GET_SUPPLEMENTARY((c), c2_);
            }
        }
    }

    /* definitions with backward iteration
     * -------------------------------------- */

    /**
     * Move the string offset from one code point boundary to the previous one
     * and get the code point between them.
     * (Pre-decrementing backward iteration.)
     * "Unsafe" macro, assumes well-formed UTF-16.
     *
     * The input offset may be the same as the string length.
     * If the offset is behind a trail surrogate unit
     * for a supplementary code point, then the macro will read
     * the preceding lead surrogate as well.
     * If the offset is behind a lead surrogate, then that itself
     * will be returned as the code point.
     * The result is undefined if the offset is behind a single, unpaired trail
     * surrogate.
     *
     * @param s const UChar * string
     * @param i string offset
     * @param c output UChar32 variable
     * @see U16_PREV
     * @stable ICU 2.4
     */
    template<typename Iter>
    void U16_PREV_UNSAFE(Iter s, int & i, UChar32 & c)
    {
        (c) = (s)[--(i)];
        if (U16_IS_TRAIL(c)) {
            (c) = U16_GET_SUPPLEMENTARY((s)[--(i)], (c));
        }
    }

    /**
     * Move the string offset from one code point boundary to the previous one
     * and get the code point between them.
     * (Pre-decrementing backward iteration.)
     * "Safe" macro, handles unpaired surrogates and checks for string
     * boundaries.
     *
     * The input offset may be the same as the string length.
     * If the offset is behind a trail surrogate unit
     * for a supplementary code point, then the macro will read
     * the preceding lead surrogate as well.
     * If the offset is behind a lead surrogate or behind a single, unpaired
     * trail surrogate, then c is set to that unpaired surrogate.
     *
     * @param s const UChar * string
     * @param start starting string offset (usually 0)
     * @param i string offset, must be start<i
     * @param c output UChar32 variable
     * @see U16_PREV_UNSAFE
     * @stable ICU 2.4
     */
    template<typename Iter>
    void U16_PREV(Iter s, int start, int & i, UChar32 & c)
    {
        (c) = *std::next(s, --i);
        if (U16_IS_TRAIL(c)) {
            uint16_t c2_;
            if ((i) > (start) && U16_IS_LEAD(c2_ = *std::next(s, i - 1))) {
                --(i);
                (c) = U16_GET_SUPPLEMENTARY(c2_, (c));
            }
        }
    }

}}}}

#endif
