// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
*******************************************************************************
*
*   Copyright (C) 1999-2015, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  utf8.h
*   encoding:   UTF-8
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999sep13
*   created by: Markus W. Scherer
*/

/**
 * \file
 * \brief C API: 8-bit Unicode handling macros
 *
 * This file defines macros to deal with 8-bit Unicode (UTF-8) code units
 * (bytes) and strings.
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

#ifndef UTF8_H_
#define UTF8_H_

#include "machine.hpp"
#include "utf.hpp"


namespace boost { namespace text { namespace detail { namespace icu {

    /**
     * Does this code unit (byte) encode a code point by itself (US-ASCII
     * 0..0x7f)?
     * @param c 8-bit code unit (byte)
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U8_IS_SINGLE(UChar32 c) { return (((c)&0x80) == 0); }

    /**
     * Is this code unit (byte) a UTF-8 lead byte? (0xC2..0xF4)
     * @param c 8-bit code unit (byte)
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U8_IS_LEAD(UChar32 c) { return ((uint8_t)((c)-0xc2) <= 0x32); }
    // 0x32=0xf4-0xc2

    /**
     * Is this code unit (byte) a UTF-8 trail byte? (0x80..0xBF)
     * @param c 8-bit code unit (byte)
     * @return TRUE or FALSE
     * @stable ICU 2.4
     */
    inline bool U8_IS_TRAIL(UChar32 c) { return ((int8_t)(c) < -0x40); }

    /**
     * Counts the trail bytes for a UTF-8 lead byte.
     * Returns 0 for 0..0xc1 as well as for 0xf5..0xff.
     * leadByte might be evaluated multiple times.
     *
     * This is internal since it is not meant to be called directly by external
     * clients; however it is called by public macros in this file and thus must
     * remain stable.
     *
     * @param leadByte The first byte of a UTF-8 sequence. Must be 0..0xff.
     * @internal
     */
    inline uint8_t U8_COUNT_TRAIL_BYTES(uint8_t leadByte)
    {
        return U8_IS_LEAD(leadByte) ? ((uint8_t)(leadByte) >= 0xe0) +
                                          ((uint8_t)(leadByte) >= 0xf0) + 1
                                    : 0;
    }

    /**
     * Counts the trail bytes for a UTF-8 lead byte of a valid UTF-8 sequence.
     * Returns 0 for 0..0xc1. Undefined for 0xf5..0xff.
     * leadByte might be evaluated multiple times.
     *
     * This is internal since it is not meant to be called directly by external
     * clients; however it is called by public macros in this file and thus must
     * remain stable.
     *
     * @param leadByte The first byte of a UTF-8 sequence. Must be 0..0xff.
     * @internal
     */
    inline uint8_t U8_COUNT_TRAIL_BYTES_UNSAFE(uint8_t leadByte)
    {
        return ((uint8_t)(leadByte) >= 0xc2) + ((uint8_t)(leadByte) >= 0xe0) +
               ((uint8_t)(leadByte) >= 0xf0);
    }

    /**
     * Mask a UTF-8 lead byte, leave only the lower bits that form part of the
     * code point value.
     *
     * This is internal since it is not meant to be called directly by external
     * clients; however it is called by public macros in this file and thus must
     * remain stable.
     * @internal
     */
    inline uint8_t U8_MASK_LEAD_BYTE(uint8_t leadByte, uint8_t countTrailBytes)
    {
        return ((leadByte) &= (1 << (6 - (countTrailBytes))) - 1);
    }

    /**
     * Internal bit vector for 3-byte UTF-8 validity check, for use in
     * U8_IS_VALID_LEAD3_AND_T1. Each bit indicates whether one lead byte +
     * first trail byte pair starts a valid sequence. Lead byte E0..EF bits 3..0
     * are used as byte index, first trail byte bits 7..5 are used as bit index
     * into that byte.
     * @see U8_IS_VALID_LEAD3_AND_T1
     * @internal
     */
    namespace {
        constexpr char const * U8_LEAD3_T1_BITS =
            "\x20\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x10\x30\x30";
    }

    /**
     * Internal 3-byte UTF-8 validity check.
     * Non-zero if lead byte E0..EF and first trail byte 00..FF start a valid
     * sequence.
     * @internal
     */
    inline bool U8_IS_VALID_LEAD3_AND_T1(uint8_t lead, uint8_t t1)
    {
        return (U8_LEAD3_T1_BITS[(lead)&0xf] & (1 << ((uint8_t)(t1) >> 5)));
    }

    /**
     * Internal bit vector for 4-byte UTF-8 validity check, for use in
     * U8_IS_VALID_LEAD4_AND_T1. Each bit indicates whether one lead byte +
     * first trail byte pair starts a valid sequence. First trail byte bits 7..4
     * are used as byte index, lead byte F0..F4 bits 2..0 are used as bit index
     * into that byte.
     * @see U8_IS_VALID_LEAD4_AND_T1
     * @internal
     */
    namespace {
        constexpr char const * U8_LEAD4_T1_BITS =
            "\x00\x00\x00\x00\x00\x00\x00\x00\x1E\x0F\x0F\x0F\x00\x00\x00\x00";
    }

    /**
     * Internal 4-byte UTF-8 validity check.
     * Non-zero if lead byte F0..F4 and first trail byte 00..FF start a valid
     * sequence.
     * @internal
     */
    inline bool U8_IS_VALID_LEAD4_AND_T1(uint8_t lead, uint8_t t1)
    {
        return (U8_LEAD4_T1_BITS[(uint8_t)(t1) >> 4] & (1 << ((lead)&7)));
    }

    /* single-code point definitions
     * -------------------------------------------- */

    /**
     * How many code units (bytes) are used for the UTF-8 encoding
     * of this Unicode code point?
     * @param c 32-bit code point
     * @return 1..4, or 0 if c is a surrogate or not a Unicode code point
     * @stable ICU 2.4
     */
    inline int U8_LENGTH(UChar32 c)
    {
        return (uint32_t)(c) <= 0x7f
                   ? 1
                   : ((uint32_t)(c) <= 0x7ff
                          ? 2
                          : ((uint32_t)(c) <= 0xd7ff
                                 ? 3
                                 : ((uint32_t)(c) <= 0xdfff ||
                                            (uint32_t)(c) > 0x10ffff
                                        ? 0
                                        : ((uint32_t)(c) <= 0xffff ? 3 : 4))));
    }


    /* definitions with forward iteration
     * --------------------------------------- */

    /**
     * Get a code point from a string at a code point boundary offset,
     * and advance the offset to the next code point boundary.
     * (Post-incrementing forward iteration.)
     * "Unsafe" macro, assumes well-formed UTF-8.
     *
     * The offset may point to the lead byte of a multi-byte sequence,
     * in which case the macro will read the whole sequence.
     * The result is undefined if the offset points to a trail byte
     * or an illegal UTF-8 sequence.
     *
     * @param s const uint8_t * string
     * @param i string offset
     * @param c output UChar32 variable
     * @see U8_NEXT
     * @stable ICU 2.4
     */
    inline void U8_NEXT_UNSAFE(const uint8_t * s, int i, UChar32 & c)
    {
        (c) = (uint8_t)(s)[(i)++];
        if (!U8_IS_SINGLE(c)) {
            if ((c) < 0xe0) {
                (c) = (((c)&0x1f) << 6) | ((s)[(i)++] & 0x3f);
            } else if ((c) < 0xf0) {
                /* no need for (c&0xf) because the upper bits are truncated
                 * after
                 * <<12 in the cast to (UChar) */
                (c) = (UChar)(
                    ((c) << 12) | (((s)[i] & 0x3f) << 6) |
                    ((s)[(i) + 1] & 0x3f));
                (i) += 2;
            } else {
                (c) = (((c)&7) << 18) | (((s)[i] & 0x3f) << 12) |
                      (((s)[(i) + 1] & 0x3f) << 6) | ((s)[(i) + 2] & 0x3f);
                (i) += 3;
            }
        }
    }

    /**
     * Append a code point to a string, overwriting 1 to 4 bytes.
     * The offset points to the current end of the string contents
     * and is advanced (post-increment).
     * "Unsafe" macro, assumes a valid code point and sufficient space in the
     * string. Otherwise, the result is undefined.
     *
     * @param s const uint8_t * string buffer
     * @param i string offset
     * @param c code point to append
     * @see U8_APPEND
     * @stable ICU 2.4
     */
    template<typename Char>
    void U8_APPEND_UNSAFE(Char * s, int i, UChar32 c)
    {
        uint32_t uc_ = (c);
        if (uc_ <= 0x7f) {
            (s)[(i)++] = (uint8_t)uc_;
        } else {
            if (uc_ <= 0x7ff) {
                (s)[(i)++] = (uint8_t)((uc_ >> 6) | 0xc0);
            } else {
                if (uc_ <= 0xffff) {
                    (s)[(i)++] = (uint8_t)((uc_ >> 12) | 0xe0);
                } else {
                    (s)[(i)++] = (uint8_t)((uc_ >> 18) | 0xf0);
                    (s)[(i)++] = (uint8_t)(((uc_ >> 12) & 0x3f) | 0x80);
                }
                (s)[(i)++] = (uint8_t)(((uc_ >> 6) & 0x3f) | 0x80);
            }
            (s)[(i)++] = (uint8_t)((uc_ & 0x3f) | 0x80);
        }
    }

    /**
     * Function for handling "previous code point" with error-checking.
     *
     * This is internal since it is not meant to be called directly by external
     * clients; however it is U_STABLE (not U_INTERNAL) since it is called by
     * public macros in this file and thus must remain stable, and should not be
     * hidden when other internal functions are hidden (otherwise public macros
     * would fail to compile).
     * @internal
     */
    inline UChar32 utf8_prevCharSafeBody(
        const uint8_t * s, int32_t start, int32_t * pi, UChar32 c, UBool strict)
    {
        auto errorValue = [](int32_t count, int8_t strict) -> UChar32 {
            const UChar32 utf8_errorValue[6] = {
                // Same values as UTF8_ERROR_VALUE_1, UTF8_ERROR_VALUE_2,
                // UTF_ERROR_VALUE, but without relying on the obsolete
                // unicode/utf_old.h.
                0x15,
                0x9f,
                0xffff,
                0x10ffff,
            };

            if (strict >= 0) {
                return utf8_errorValue[count];
            } else if (strict == -3) {
                return 0xfffd;
            } else {
                return U_SENTINEL;
            }
        };

        // *pi is the index of byte c.
        int32_t i = *pi;
        if (U8_IS_TRAIL(c) && i > start) {
            uint8_t b1 = s[--i];
            if (U8_IS_LEAD(b1)) {
                if (b1 < 0xe0) {
                    *pi = i;
                    return ((b1 - 0xc0) << 6) | (c & 0x3f);
                } else if (
                    b1 < 0xf0 ? U8_IS_VALID_LEAD3_AND_T1(b1, c)
                              : U8_IS_VALID_LEAD4_AND_T1(b1, c)) {
                    // Truncated 3- or 4-byte sequence.
                    *pi = i;
                    return errorValue(1, strict);
                }
            } else if (U8_IS_TRAIL(b1) && i > start) {
                // Extract the value bits from the last trail byte.
                c &= 0x3f;
                uint8_t b2 = s[--i];
                if (0xe0 <= b2 && b2 <= 0xf4) {
                    if (b2 < 0xf0) {
                        b2 &= 0xf;
                        if (strict != -2) {
                            if (U8_IS_VALID_LEAD3_AND_T1(b2, b1)) {
                                *pi = i;
                                c = (b2 << 12) | ((b1 & 0x3f) << 6) | c;
                                if (strict <= 0 || !U_IS_UNICODE_NONCHAR(c)) {
                                    return c;
                                } else {
                                    // strict: forbid non-characters like U+fffe
                                    return errorValue(2, strict);
                                }
                            }
                        } else {
                            // strict=-2 -> lenient: allow surrogates
                            b1 -= 0x80;
                            if ((b2 > 0 || b1 >= 0x20)) {
                                *pi = i;
                                return (b2 << 12) | (b1 << 6) | c;
                            }
                        }
                    } else if (U8_IS_VALID_LEAD4_AND_T1(b2, b1)) {
                        // Truncated 4-byte sequence.
                        *pi = i;
                        return errorValue(2, strict);
                    }
                } else if (U8_IS_TRAIL(b2) && i > start) {
                    uint8_t b3 = s[--i];
                    if (0xf0 <= b3 && b3 <= 0xf4) {
                        b3 &= 7;
                        if (U8_IS_VALID_LEAD4_AND_T1(b3, b2)) {
                            *pi = i;
                            c = (b3 << 18) | ((b2 & 0x3f) << 12) |
                                ((b1 & 0x3f) << 6) | c;
                            if (strict <= 0 || !U_IS_UNICODE_NONCHAR(c)) {
                                return c;
                            } else {
                                // strict: forbid non-characters like U+fffe
                                return errorValue(3, strict);
                            }
                        }
                    }
                }
            }
        }
        return errorValue(0, strict);
    }

    /**
     * Move the string offset from one code point boundary to the previous one
     * and get the code point between them.
     * (Pre-decrementing backward iteration.)
     * "Safe" macro, checks for illegal sequences and for string boundaries.
     *
     * The input offset may be the same as the string length.
     * If the offset is behind a multi-byte sequence, then the macro will read
     * the whole sequence.
     * If the offset is behind a lead byte, then that itself
     * will be returned as the code point.
     * If the offset is behind an illegal UTF-8 sequence, then c is set to a
     * negative value.
     *
     * @param s const uint8_t * string
     * @param start int32_t starting string offset (usually 0)
     * @param i int32_t string offset, must be start<i
     * @param c output UChar32 variable, set to <0 in case of an error
     * @see U8_PREV_UNSAFE
     * @stable ICU 2.4
     */
    inline void U8_PREV(const uint8_t * s, int start, int i, UChar32 & c)
    {
        (c) = (uint8_t)(s)[--(i)];
        if (!U8_IS_SINGLE(c)) {
            (c) = utf8_prevCharSafeBody((const uint8_t *)s, start, &(i), c, -1);
        }
    }

}}}}

#endif
