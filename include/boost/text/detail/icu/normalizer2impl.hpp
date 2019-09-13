// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
*******************************************************************************
*
*   Copyright (C) 2009-2014, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  normalizer2impl.h
*   encoding:   UTF-8
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2009nov22
*   created by: Markus W. Scherer
*/

#ifndef NORMALIZER2IMPL_H_
#define NORMALIZER2IMPL_H_

#include "bytestream.hpp"
#include "bytesinkutil.hpp"
#include "machine.hpp"
#include "ucptrie.hpp"
#include "utypes.hpp"
#include "utf8.hpp"
#include "utf16.hpp"

#include <boost/assert.hpp>

#include <cstring>
#include <mutex>

#include <vector> // TODO


namespace boost { namespace text { namespace detail { namespace icu {

    // Copy/pasted from unorm2.h.
    /**
     * Constants for normalization modes.
     * For details about standard Unicode normalization forms
     * and about the algorithms which are also used with custom mapping tables
     * see http://www.unicode.org/unicode/reports/tr15/
     * @stable ICU 4.4
     */
    typedef enum {
        /**
         * Decomposition followed by composition.
         * Same as standard NFC when using an "nfc" instance.
         * Same as standard NFKC when using an "nfkc" instance.
         * For details about standard Unicode normalization forms
         * see http://www.unicode.org/unicode/reports/tr15/
         * @stable ICU 4.4
         */
        UNORM2_COMPOSE,
        /**
         * Map, and reorder canonically.
         * Same as standard NFD when using an "nfc" instance.
         * Same as standard NFKD when using an "nfkc" instance.
         * For details about standard Unicode normalization forms
         * see http://www.unicode.org/unicode/reports/tr15/
         * @stable ICU 4.4
         */
        UNORM2_DECOMPOSE,
        /**
         * "Fast C or D" form.
         * If a string is in this form, then further decomposition <i>without
         * reordering</i> would yield the same form as DECOMPOSE. Text in "Fast
         * C or D" form can be processed efficiently with data tables that are
         * "canonically closed", that is, that provide equivalent data for
         * equivalent text, without having to be fully normalized.
         * Not a standard Unicode normalization form.
         * Not a unique form: Different FCD strings can be canonically
         * equivalent. For details see http://www.unicode.org/notes/tn5/#FCD
         * @stable ICU 4.4
         */
        UNORM2_FCD,
        /**
         * Compose only contiguously.
         * Also known as "FCC" or "Fast C Contiguous".
         * The result will often but not always be in NFC.
         * The result will conform to FCD which is useful for processing.
         * Not a standard Unicode normalization form.
         * For details see http://www.unicode.org/notes/tn5/#FCC
         * @stable ICU 4.4
         */
        UNORM2_COMPOSE_CONTIGUOUS
    } UNormalization2Mode;

    /**
     * Result values for normalization quick check functions.
     * For details see
     * http://www.unicode.org/reports/tr15/#Detecting_Normalization_Forms
     * @stable ICU 2.0
     */
    typedef enum UNormalizationCheckResult {
        /**
         * The input string is not in the normalization form.
         * @stable ICU 2.0
         */
        UNORM_NO,
        /**
         * The input string is in the normalization form.
         * @stable ICU 2.0
         */
        UNORM_YES,
        /**
         * The input string may or may not be in the normalization form.
         * This value is only returned for composition forms like NFC and FCC,
         * when a backward-combining character is found for which the
         * surrounding text would have to be analyzed further.
         * @stable ICU 2.0
         */
        UNORM_MAYBE
    } UNormalizationCheckResult;

    // Copy/pasted from unorm.h.
    /**
     * Constants for normalization modes.
     * @deprecated ICU 56 Use unorm2.h instead.
     */
    typedef enum {
        /** No decomposition/composition. @deprecated ICU 56 Use unorm2.h
           instead. */
        UNORM_NONE = 1,
        /** Canonical decomposition. @deprecated ICU 56 Use unorm2.h instead. */
        UNORM_NFD = 2,
        /** Compatibility decomposition. @deprecated ICU 56 Use unorm2.h
           instead. */
        UNORM_NFKD = 3,
        /** Canonical decomposition followed by canonical composition.
           @deprecated ICU 56 Use unorm2.h instead. */
        UNORM_NFC = 4,
        /** Default normalization. @deprecated ICU 56 Use unorm2.h instead. */
        UNORM_DEFAULT = UNORM_NFC,
        /** Compatibility decomposition followed by canonical composition.
           @deprecated ICU 56 Use unorm2.h instead. */
        UNORM_NFKC = 5,
        /** "Fast C or D" form. @deprecated ICU 56 Use unorm2.h instead. */
        UNORM_FCD = 6,

        /** One more than the highest normalization mode constant. @deprecated
           ICU 56 Use unorm2.h instead. */
        UNORM_MODE_COUNT
    } UNormalizationMode;

    using UnicodeString = std::vector<UChar>; // TODO

    class Hangul
    {
    public:
        /* Korean Hangul and Jamo constants */
        enum {
            JAMO_L_BASE = 0x1100, /* "lead" jamo */
            JAMO_L_END = 0x1112,
            JAMO_V_BASE = 0x1161, /* "vowel" jamo */
            JAMO_V_END = 0x1175,
            JAMO_T_BASE = 0x11a7, /* "trail" jamo */
            JAMO_T_END = 0x11c2,

            HANGUL_BASE = 0xac00,
            HANGUL_END = 0xd7a3,

            JAMO_L_COUNT = 19,
            JAMO_V_COUNT = 21,
            JAMO_T_COUNT = 28,

            JAMO_VT_COUNT = JAMO_V_COUNT * JAMO_T_COUNT,

            HANGUL_COUNT = JAMO_L_COUNT * JAMO_V_COUNT * JAMO_T_COUNT,
            HANGUL_LIMIT = HANGUL_BASE + HANGUL_COUNT
        };

        static inline UBool isHangul(UChar32 c)
        {
            return HANGUL_BASE <= c && c < HANGUL_LIMIT;
        }
        static inline UBool isHangulLV(UChar32 c)
        {
            c -= HANGUL_BASE;
            return 0 <= c && c < HANGUL_COUNT && c % JAMO_T_COUNT == 0;
        }
        static inline UBool isJamoL(UChar32 c)
        {
            return (uint32_t)(c - JAMO_L_BASE) < JAMO_L_COUNT;
        }
        static inline UBool isJamoV(UChar32 c)
        {
            return (uint32_t)(c - JAMO_V_BASE) < JAMO_V_COUNT;
        }
        static inline UBool isJamoT(UChar32 c)
        {
            int32_t t = c - JAMO_T_BASE;
            return 0 < t && t < JAMO_T_COUNT; // not JAMO_T_BASE itself
        }
        static UBool isJamo(UChar32 c)
        {
            return JAMO_L_BASE <= c && c <= JAMO_T_END &&
                   (c <= JAMO_L_END || (JAMO_V_BASE <= c && c <= JAMO_V_END) ||
                    JAMO_T_BASE < c);
        }

        /**
         * Decomposes c, which must be a Hangul syllable, into buffer
         * and returns the length of the decomposition (2 or 3).
         */
        static inline int32_t decompose(UChar32 c, UChar buffer[3])
        {
            c -= HANGUL_BASE;
            UChar32 c2 = c % JAMO_T_COUNT;
            c /= JAMO_T_COUNT;
            buffer[0] = (UChar)(JAMO_L_BASE + c / JAMO_V_COUNT);
            buffer[1] = (UChar)(JAMO_V_BASE + c % JAMO_V_COUNT);
            if (c2 == 0) {
                return 2;
            } else {
                buffer[2] = (UChar)(JAMO_T_BASE + c2);
                return 3;
            }
        }

        /**
         * Decomposes c, which must be a Hangul syllable, into buffer.
         * This is the raw, not recursive, decomposition. Its length is
         * always 2.
         */
        static inline void getRawDecomposition(UChar32 c, UChar buffer[2])
        {
            UChar32 orig = c;
            c -= HANGUL_BASE;
            UChar32 c2 = c % JAMO_T_COUNT;
            if (c2 == 0) {
                c /= JAMO_T_COUNT;
                buffer[0] = (UChar)(JAMO_L_BASE + c / JAMO_V_COUNT);
                buffer[1] = (UChar)(JAMO_V_BASE + c % JAMO_V_COUNT);
            } else {
                buffer[0] = (UChar)(orig - c2); // LV syllable
                buffer[1] = (UChar)(JAMO_T_BASE + c2);
            }
        }

    private:
        Hangul(); // no instantiation
    };

    class Normalizer2Impl;

    class ReorderingBuffer
    {
    public:
        /** Constructs only; init() should be called. */
        ReorderingBuffer(const Normalizer2Impl & ni, UnicodeString & dest) :
            impl(ni),
            str(dest),
            start(nullptr),
            reorderStart(nullptr),
            limit(nullptr),
            remainingCapacity(0),
            lastCC(0)
        {}
        /** Constructs, removes the string contents, and initializes for a small
         * initial capacity. */
        ReorderingBuffer(
            const Normalizer2Impl & ni,
            UnicodeString & dest,
            UErrorCode & errorCode) :
            impl(ni),
            str(dest),
            start((str.resize(8), str.data())),
            reorderStart(start),
            limit(start),
            remainingCapacity(str.capacity()),
            lastCC(0)
        {
            if (start == nullptr && U_SUCCESS(errorCode)) {
                // getBuffer() already did str.setToBogus()
                errorCode = U_MEMORY_ALLOCATION_ERROR;
            }
        }
        ~ReorderingBuffer()
        {
            if (start != nullptr) {
                str.resize((int32_t)(limit - start));
            }
        }
        UBool init(int32_t destCapacity, UErrorCode & errorCode)
        {
            int32_t length = str.size();
            str.resize(destCapacity);
            start = str.data();
            if (start == nullptr) {
                // getBuffer() already did str.setToBogus()
                errorCode = U_MEMORY_ALLOCATION_ERROR;
                return FALSE;
            }
            limit = start + length;
            remainingCapacity = str.capacity() - length;
            reorderStart = start;
            if (start == limit) {
                lastCC = 0;
            } else {
                setIterator();
                lastCC = previousCC();
                // Set reorderStart after the last code point with cc<=1 if
                // there is one.
                if (lastCC > 1) {
                    while (previousCC() > 1) {
                    }
                }
                reorderStart = codePointLimit;
            }
            return TRUE;
        }

        UBool isEmpty() const { return start == limit; }
        int32_t length() const { return (int32_t)(limit - start); }
        UChar * getStart() { return start; }
        UChar * getLimit() { return limit; }
        uint8_t getLastCC() const { return lastCC; }

        UBool equals(const UChar * otherStart, const UChar * otherLimit) const
        {
            int32_t length = (int32_t)(limit - start);
            return length == (int32_t)(otherLimit - otherStart) &&
                   0 == memcmp(start, otherStart, length);
        }
        UBool
        equals(const uint8_t * otherStart, const uint8_t * otherLimit) const
        {
            BOOST_ASSERT(
                (otherLimit - otherStart) <= INT32_MAX); // ensured by caller
            int32_t length = (int32_t)(limit - start);
            int32_t otherLength = (int32_t)(otherLimit - otherStart);
            // For equal strings, UTF-8 is at least as long as UTF-16, and at
            // most three times as long.
            if (otherLength < length || (otherLength / 3) > length) {
                return FALSE;
            }
            // Compare valid strings from between normalization boundaries.
            // (Invalid sequences are normalization-inert.)
            for (int32_t i = 0, j = 0;;) {
                if (i >= length) {
                    return j >= otherLength;
                } else if (j >= otherLength) {
                    return FALSE;
                }
                // Not at the end of either string yet.
                UChar32 c, other;
                U16_NEXT_UNSAFE(start, i, c);
                U8_NEXT_UNSAFE(otherStart, j, other);
                if (c != other) {
                    return FALSE;
                }
            }
        }

        UBool append(UChar32 c, uint8_t cc, UErrorCode & errorCode)
        {
            return (c <= 0xffff) ? appendBMP((UChar)c, cc, errorCode)
                                 : appendSupplementary(c, cc, errorCode);
        }
        UBool append(
            const UChar * s,
            int32_t length,
            UBool isNFD,
            uint8_t leadCC,
            uint8_t trailCC,
            UErrorCode & errorCode);
        UBool appendBMP(UChar c, uint8_t cc, UErrorCode & errorCode)
        {
            if (remainingCapacity == 0 && !resize(1, errorCode)) {
                return FALSE;
            }
            if (lastCC <= cc || cc == 0) {
                *limit++ = c;
                lastCC = cc;
                if (cc <= 1) {
                    reorderStart = limit;
                }
            } else {
                insert(c, cc);
            }
            --remainingCapacity;
            return TRUE;
        }
        UBool appendZeroCC(UChar32 c, UErrorCode & errorCode)
        {
            int32_t cpLength = U16_LENGTH(c);
            if (remainingCapacity < cpLength && !resize(cpLength, errorCode)) {
                return FALSE;
            }
            remainingCapacity -= cpLength;
            if (cpLength == 1) {
                *limit++ = (UChar)c;
            } else {
                limit[0] = U16_LEAD(c);
                limit[1] = U16_TRAIL(c);
                limit += 2;
            }
            lastCC = 0;
            reorderStart = limit;
            return TRUE;
        }
        UBool appendZeroCC(
            const UChar * s, const UChar * sLimit, UErrorCode & errorCode)
        {
            if (s == sLimit) {
                return TRUE;
            }
            int32_t length = (int32_t)(sLimit - s);
            if (remainingCapacity < length && !resize(length, errorCode)) {
                return FALSE;
            }
            memcpy(limit, s, length);
            limit += length;
            remainingCapacity -= length;
            lastCC = 0;
            reorderStart = limit;
            return TRUE;
        }
        void remove()
        {
            reorderStart = limit = start;
            remainingCapacity = str.capacity();
            lastCC = 0;
        }
        void removeSuffix(int32_t suffixLength)
        {
            if (suffixLength < (limit - start)) {
                limit -= suffixLength;
                remainingCapacity += suffixLength;
            } else {
                limit = start;
                remainingCapacity = str.capacity();
            }
            lastCC = 0;
            reorderStart = limit;
        }
        void setReorderingLimit(UChar * newLimit)
        {
            remainingCapacity += (int32_t)(limit - newLimit);
            reorderStart = limit = newLimit;
            lastCC = 0;
        }
        void copyReorderableSuffixTo(UnicodeString & s) const
        {
            s.assign(
                reorderStart, reorderStart + (int32_t)(limit - reorderStart));
        }

    private:
        /*
         * TODO: Revisit whether it makes sense to track reorderStart.
         * It is set to after the last known character with cc<=1,
         * which stops previousCC() before it reads that character and looks up
         * its cc. previousCC() is normally only called from insert(). In other
         * words, reorderStart speeds up the insertion of a combining mark into
         * a multi-combining mark sequence where it does not belong at the end.
         * This might not be worth the trouble.
         * On the other hand, it's not a huge amount of trouble.
         *
         * We probably need it for UNORM_SIMPLE_APPEND.
         */

        UBool appendSupplementary(UChar32 c, uint8_t cc, UErrorCode & errorCode)
        {
            if (remainingCapacity < 2 && !resize(2, errorCode)) {
                return FALSE;
            }
            if (lastCC <= cc || cc == 0) {
                limit[0] = U16_LEAD(c);
                limit[1] = U16_TRAIL(c);
                limit += 2;
                lastCC = cc;
                if (cc <= 1) {
                    reorderStart = limit;
                }
            } else {
                insert(c, cc);
            }
            remainingCapacity -= 2;
            return TRUE;
        }
        // Inserts c somewhere before the last character.
        // Requires 0<cc<lastCC which implies reorderStart<limit.
        void insert(UChar32 c, uint8_t cc)
        {
            for (setIterator(), skipPrevious(); previousCC() > cc;) {
            }
            // insert c at codePointLimit, after the character with prevCC<=cc
            UChar * q = limit;
            UChar * r = limit += U16_LENGTH(c);
            do {
                *--r = *--q;
            } while (codePointLimit != q);
            writeCodePoint(q, c);
            if (cc <= 1) {
                reorderStart = r;
            }
        }
        static void writeCodePoint(UChar * p, UChar32 c)
        {
            if (c <= 0xffff) {
                *p = (UChar)c;
            } else {
                p[0] = U16_LEAD(c);
                p[1] = U16_TRAIL(c);
            }
        }
        UBool resize(int32_t appendLength, UErrorCode & errorCode)
        {
            int32_t reorderStartIndex = (int32_t)(reorderStart - start);
            int32_t length = (int32_t)(limit - start);
            str.resize(length);
            int32_t newCapacity = length + appendLength;
            int32_t doubleCapacity = 2 * str.capacity();
            if (newCapacity < doubleCapacity) {
                newCapacity = doubleCapacity;
            }
            if (newCapacity < 256) {
                newCapacity = 256;
            }
            str.resize(newCapacity);
            start = str.data();
            if (start == nullptr) {
                // getBuffer() already did str.setToBogus()
                errorCode = U_MEMORY_ALLOCATION_ERROR;
                return FALSE;
            }
            reorderStart = start + reorderStartIndex;
            limit = start + length;
            remainingCapacity = str.capacity() - length;
            return TRUE;
        }

        const Normalizer2Impl & impl;
        UnicodeString & str;
        UChar *start, *reorderStart, *limit;
        int32_t remainingCapacity;
        uint8_t lastCC;

        // private backward iterator
        void setIterator() { codePointStart = limit; }
        // Requires start<codePointStart.
        void skipPrevious()
        {
            codePointLimit = codePointStart;
            UChar c = *--codePointStart;
            if (U16_IS_TRAIL(c) && start < codePointStart &&
                U16_IS_LEAD(*(codePointStart - 1))) {
                --codePointStart;
            }
        }
        // Returns 0 if there is no previous character.
        uint8_t previousCC();

        UChar *codePointStart, *codePointLimit;
    };

    /**
     * Low-level implementation of the Unicode Normalization Algorithm.
     * For the data structure and details see the documentation at the end of
     * this normalizer2impl.h and in the design doc at
     * http://site.icu-project.org/design/normalization/custom
     */
    class Normalizer2Impl
    {
    public:
        Normalizer2Impl() : normTrie(nullptr) {}
        virtual ~Normalizer2Impl() {}

        void init(
            const int32_t * inIndexes,
            const UCPTrie * inTrie,
            const uint16_t * inExtraData,
            const uint8_t * inSmallFCD)
        {
            minDecompNoCP = static_cast<UChar>(inIndexes[IX_MIN_DECOMP_NO_CP]);
            minCompNoMaybeCP =
                static_cast<UChar>(inIndexes[IX_MIN_COMP_NO_MAYBE_CP]);
            minLcccCP = static_cast<UChar>(inIndexes[IX_MIN_LCCC_CP]);

            minYesNo = static_cast<uint16_t>(inIndexes[IX_MIN_YES_NO]);
            minYesNoMappingsOnly =
                static_cast<uint16_t>(inIndexes[IX_MIN_YES_NO_MAPPINGS_ONLY]);
            minNoNo = static_cast<uint16_t>(inIndexes[IX_MIN_NO_NO]);
            minNoNoCompBoundaryBefore = static_cast<uint16_t>(
                inIndexes[IX_MIN_NO_NO_COMP_BOUNDARY_BEFORE]);
            minNoNoCompNoMaybeCC =
                static_cast<uint16_t>(inIndexes[IX_MIN_NO_NO_COMP_NO_MAYBE_CC]);
            minNoNoEmpty = static_cast<uint16_t>(inIndexes[IX_MIN_NO_NO_EMPTY]);
            limitNoNo = static_cast<uint16_t>(inIndexes[IX_LIMIT_NO_NO]);
            minMaybeYes = static_cast<uint16_t>(inIndexes[IX_MIN_MAYBE_YES]);
            BOOST_ASSERT(
                (minMaybeYes & 7) == 0); // 8-aligned for noNoDelta bit fields
            centerNoNoDelta = (minMaybeYes >> DELTA_SHIFT) - MAX_DELTA - 1;

            normTrie = inTrie;

            maybeYesCompositions = inExtraData;
            extraData = maybeYesCompositions +
                        ((MIN_NORMAL_MAYBE_YES - minMaybeYes) >> OFFSET_SHIFT);

            smallFCD = inSmallFCD;
        }

        // The trie stores values for lead surrogate code *units*.
        // Surrogate code *points* are inert.
        uint16_t getNorm16(UChar32 c) const
        {
            return U_IS_LEAD(c) ? INERT
                                : UCPTRIE_FAST_GET(normTrie, UCPTRIE_16, c);
        }
        uint16_t getRawNorm16(UChar32 c) const
        {
            return UCPTRIE_FAST_GET(normTrie, UCPTRIE_16, c);
        }

        UNormalizationCheckResult getCompQuickCheck(uint16_t norm16) const
        {
            if (norm16 < minNoNo || MIN_YES_YES_WITH_CC <= norm16) {
                return UNORM_YES;
            } else if (minMaybeYes <= norm16) {
                return UNORM_MAYBE;
            } else {
                return UNORM_NO;
            }
        }
        UBool isAlgorithmicNoNo(uint16_t norm16) const
        {
            return limitNoNo <= norm16 && norm16 < minMaybeYes;
        }
        UBool isCompNo(uint16_t norm16) const
        {
            return minNoNo <= norm16 && norm16 < minMaybeYes;
        }
        UBool isDecompYes(uint16_t norm16) const
        {
            return norm16 < minYesNo || minMaybeYes <= norm16;
        }

        uint8_t getCC(uint16_t norm16) const
        {
            if (norm16 >= MIN_NORMAL_MAYBE_YES) {
                return getCCFromNormalYesOrMaybe(norm16);
            }
            if (norm16 < minNoNo || limitNoNo <= norm16) {
                return 0;
            }
            return getCCFromNoNo(norm16);
        }
        static uint8_t getCCFromNormalYesOrMaybe(uint16_t norm16)
        {
            return (uint8_t)(norm16 >> OFFSET_SHIFT);
        }
        static uint8_t getCCFromYesOrMaybe(uint16_t norm16)
        {
            return norm16 >= MIN_NORMAL_MAYBE_YES
                       ? getCCFromNormalYesOrMaybe(norm16)
                       : 0;
        }
        uint8_t getCCFromYesOrMaybeCP(UChar32 c) const
        {
            if (c < minCompNoMaybeCP) {
                return 0;
            }
            return getCCFromYesOrMaybe(getNorm16(c));
        }

        /**
         * Returns the FCD data for code point c.
         * @param c A Unicode code point.
         * @return The lccc(c) in bits 15..8 and tccc(c) in bits 7..0.
         */
        uint16_t getFCD16(UChar32 c) const
        {
            if (c < minDecompNoCP) {
                return 0;
            } else if (c <= 0xffff) {
                if (!singleLeadMightHaveNonZeroFCD16(c)) {
                    return 0;
                }
            }
            return getFCD16FromNormData(c);
        }

        /** Returns TRUE if the single-or-lead code unit c might have non-zero
         * FCD data. */
        UBool singleLeadMightHaveNonZeroFCD16(UChar32 lead) const
        {
            // 0<=lead<=0xffff
            uint8_t bits = smallFCD[lead >> 8];
            if (bits == 0) {
                return false;
            }
            return (UBool)((bits >> ((lead >> 5) & 7)) & 1);
        }
        /** Returns the FCD value from the regular normalization data. */
        uint16_t getFCD16FromNormData(UChar32 c) const
        {
            uint16_t norm16 = getNorm16(c);
            if (norm16 >= limitNoNo) {
                if (norm16 >= MIN_NORMAL_MAYBE_YES) {
                    // combining mark
                    norm16 = getCCFromNormalYesOrMaybe(norm16);
                    return norm16 | (norm16 << 8);
                } else if (norm16 >= minMaybeYes) {
                    return 0;
                } else { // isDecompNoAlgorithmic(norm16)
                    uint16_t deltaTrailCC = norm16 & DELTA_TCCC_MASK;
                    if (deltaTrailCC <= DELTA_TCCC_1) {
                        return deltaTrailCC >> OFFSET_SHIFT;
                    }
                    // Maps to an isCompYesAndZeroCC.
                    c = mapAlgorithmic(c, norm16);
                    norm16 = getRawNorm16(c);
                }
            }
            if (norm16 <= minYesNo || isHangulLVT(norm16)) {
                // no decomposition or Hangul syllable, all zeros
                return 0;
            }
            // c decomposes, get everything from the variable-length extra data
            const uint16_t * mapping = getMapping(norm16);
            uint16_t firstUnit = *mapping;
            norm16 = firstUnit >> 8; // tccc
            if (firstUnit & MAPPING_HAS_CCC_LCCC_WORD) {
                norm16 |= *(mapping - 1) & 0xff00; // lccc
            }
            return norm16;
        }

        enum {
            // Fixed norm16 values.
            MIN_YES_YES_WITH_CC = 0xfe02,
            JAMO_VT = 0xfe00,
            MIN_NORMAL_MAYBE_YES = 0xfc00,
            JAMO_L = 2, // offset=1 hasCompBoundaryAfter=FALSE
            INERT = 1,  // offset=0 hasCompBoundaryAfter=TRUE

            // norm16 bit 0 is comp-boundary-after.
            HAS_COMP_BOUNDARY_AFTER = 1,
            OFFSET_SHIFT = 1,

            // For algorithmic one-way mappings, norm16 bits 2..1 indicate the
            // tccc (0, 1, >1) for quick FCC boundary-after tests.
            DELTA_TCCC_0 = 0,
            DELTA_TCCC_1 = 2,
            DELTA_TCCC_GT_1 = 4,
            DELTA_TCCC_MASK = 6,
            DELTA_SHIFT = 3,

            MAX_DELTA = 0x40
        };

        enum {
            // Byte offsets from the start of the data, after the generic
            // header.
            IX_NORM_TRIE_OFFSET,
            IX_EXTRA_DATA_OFFSET,
            IX_SMALL_FCD_OFFSET,
            IX_RESERVED3_OFFSET,
            IX_RESERVED4_OFFSET,
            IX_RESERVED5_OFFSET,
            IX_RESERVED6_OFFSET,
            IX_TOTAL_SIZE,

            // Code point thresholds for quick check codes.
            IX_MIN_DECOMP_NO_CP,
            IX_MIN_COMP_NO_MAYBE_CP,

            // Norm16 value thresholds for quick check combinations and types of
            // extra data.

            /** Mappings & compositions in [minYesNo..minYesNoMappingsOnly[. */
            IX_MIN_YES_NO,
            /** Mappings are comp-normalized. */
            IX_MIN_NO_NO,
            IX_LIMIT_NO_NO,
            IX_MIN_MAYBE_YES,

            /** Mappings only in [minYesNoMappingsOnly..minNoNo[. */
            IX_MIN_YES_NO_MAPPINGS_ONLY,
            /** Mappings are not comp-normalized but have a comp boundary
               before. */
            IX_MIN_NO_NO_COMP_BOUNDARY_BEFORE,
            /** Mappings do not have a comp boundary before. */
            IX_MIN_NO_NO_COMP_NO_MAYBE_CC,
            /** Mappings to the empty string. */
            IX_MIN_NO_NO_EMPTY,

            IX_MIN_LCCC_CP,
            IX_RESERVED19,
            IX_COUNT
        };

        enum {
            MAPPING_HAS_CCC_LCCC_WORD = 0x80,
            MAPPING_HAS_RAW_MAPPING = 0x40,
            // unused bit 0x20,
            MAPPING_LENGTH_MASK = 0x1f
        };

        enum {
            COMP_1_LAST_TUPLE = 0x8000,
            COMP_1_TRIPLE = 1,
            COMP_1_TRAIL_LIMIT = 0x3400,
            COMP_1_TRAIL_MASK = 0x7ffe,
            COMP_1_TRAIL_SHIFT = 9, // 10-1 for the "triple" bit
            COMP_2_TRAIL_SHIFT = 6,
            COMP_2_TRAIL_MASK = 0xffc0
        };

        // higher-level functionality ------------------------------------------
        // ***

        // Dual functionality:
        // buffer!=nullptr: normalize
        // buffer==nullptr: isNormalized/spanQuickCheckYes
        const UChar * decompose(
            const UChar * src,
            const UChar * limit,
            ReorderingBuffer * buffer,
            UErrorCode & errorCode) const
        {
            UChar32 minNoCP = minDecompNoCP;
            if (limit == nullptr) {
                src = copyLowPrefixFromNulTerminated(
                    src, minNoCP, buffer, errorCode);
                if (U_FAILURE(errorCode)) {
                    return src;
                }
                limit = u_strchr(src, 0);
            }

            const UChar * prevSrc;
            UChar32 c = 0;
            uint16_t norm16 = 0;

            // only for quick check
            const UChar * prevBoundary = src;
            uint8_t prevCC = 0;

            for (;;) {
                // count code units below the minimum or with irrelevant data
                // for the quick check
                for (prevSrc = src; src != limit;) {
                    if ((c = *src) < minNoCP ||
                        isMostDecompYesAndZeroCC(
                            norm16 = UCPTRIE_FAST_BMP_GET(
                                normTrie, UCPTRIE_16, c))) {
                        ++src;
                    } else if (!U16_IS_LEAD(c)) {
                        break;
                    } else {
                        UChar c2;
                        if ((src + 1) != limit && U16_IS_TRAIL(c2 = src[1])) {
                            c = U16_GET_SUPPLEMENTARY(c, c2);
                            norm16 =
                                UCPTRIE_FAST_SUPP_GET(normTrie, UCPTRIE_16, c);
                            if (isMostDecompYesAndZeroCC(norm16)) {
                                src += 2;
                            } else {
                                break;
                            }
                        } else {
                            ++src; // unpaired lead surrogate: inert
                        }
                    }
                }
                // copy these code units all at once
                if (src != prevSrc) {
                    if (buffer != nullptr) {
                        if (!buffer->appendZeroCC(prevSrc, src, errorCode)) {
                            break;
                        }
                    } else {
                        prevCC = 0;
                        prevBoundary = src;
                    }
                }
                if (src == limit) {
                    break;
                }

                // Check one above-minimum, relevant code point.
                src += U16_LENGTH(c);
                if (buffer != nullptr) {
                    if (!decompose(c, norm16, *buffer, errorCode)) {
                        break;
                    }
                } else {
                    if (isDecompYes(norm16)) {
                        uint8_t cc = getCCFromYesOrMaybe(norm16);
                        if (prevCC <= cc || cc == 0) {
                            prevCC = cc;
                            if (cc <= 1) {
                                prevBoundary = src;
                            }
                            continue;
                        }
                    }
                    return prevBoundary; // "no" or cc out of order
                }
            }
            return src;
        }
        // Very similar to composeQuickCheck(): Make the same changes in both
        // places if relevant. doCompose: normalize !doCompose: isNormalized
        // (buffer must be empty and initialized)
        UBool compose(
            const UChar * src,
            const UChar * limit,
            UBool onlyContiguous,
            UBool doCompose,
            ReorderingBuffer & buffer,
            UErrorCode & errorCode) const
        {
            const UChar * prevBoundary = src;
            UChar32 minNoMaybeCP = minCompNoMaybeCP;
            if (limit == nullptr) {
                src = copyLowPrefixFromNulTerminated(
                    src,
                    minNoMaybeCP,
                    doCompose ? &buffer : nullptr,
                    errorCode);
                if (U_FAILURE(errorCode)) {
                    return FALSE;
                }
                limit = u_strchr(src, 0);
                if (prevBoundary != src) {
                    if (hasCompBoundaryAfter(*(src - 1), onlyContiguous)) {
                        prevBoundary = src;
                    } else {
                        buffer.removeSuffix(1);
                        prevBoundary = --src;
                    }
                }
            }

            for (;;) {
                // Fast path: Scan over a sequence of characters below the
                // minimum "no or maybe" code point, or with (compYes && ccc==0)
                // properties.
                const UChar * prevSrc;
                UChar32 c = 0;
                uint16_t norm16 = 0;
                for (;;) {
                    if (src == limit) {
                        if (prevBoundary != limit && doCompose) {
                            buffer.appendZeroCC(prevBoundary, limit, errorCode);
                        }
                        return TRUE;
                    }
                    if ((c = *src) < minNoMaybeCP ||
                        isCompYesAndZeroCC(
                            norm16 = UCPTRIE_FAST_BMP_GET(
                                normTrie, UCPTRIE_16, c))) {
                        ++src;
                    } else {
                        prevSrc = src++;
                        if (!U16_IS_LEAD(c)) {
                            break;
                        } else {
                            UChar c2;
                            if (src != limit && U16_IS_TRAIL(c2 = *src)) {
                                ++src;
                                c = U16_GET_SUPPLEMENTARY(c, c2);
                                norm16 = UCPTRIE_FAST_SUPP_GET(
                                    normTrie, UCPTRIE_16, c);
                                if (!isCompYesAndZeroCC(norm16)) {
                                    break;
                                }
                            }
                        }
                    }
                }
                // isCompYesAndZeroCC(norm16) is false, that is,
                // norm16>=minNoNo. The current character is either a "noNo"
                // (has a mapping) or a "maybeYes" (combines backward) or a
                // "yesYes" with ccc!=0. It is not a Hangul syllable or Jamo L
                // because those have "yes" properties.

                // Medium-fast path: Handle cases that do not require full
                // decomposition and recomposition.
                if (!isMaybeOrNonZeroCC(
                        norm16)) { // minNoNo <= norm16 < minMaybeYes
                    if (!doCompose) {
                        return FALSE;
                    }
                    // Fast path for mapping a character that is immediately
                    // surrounded by boundaries. In this case, we need not
                    // decompose around the current character.
                    if (isDecompNoAlgorithmic(norm16)) {
                        // Maps to a single isCompYesAndZeroCC character
                        // which also implies hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, onlyContiguous) ||
                            hasCompBoundaryBefore(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc, errorCode)) {
                                break;
                            }
                            if (!buffer.append(
                                    mapAlgorithmic(c, norm16), 0, errorCode)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 < minNoNoCompBoundaryBefore) {
                        // The mapping is comp-normalized which also implies
                        // hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, onlyContiguous) ||
                            hasCompBoundaryBefore(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc, errorCode)) {
                                break;
                            }
                            const UChar * mapping =
                                reinterpret_cast<const UChar *>(
                                    getMapping(norm16));
                            int32_t length = *mapping++ & MAPPING_LENGTH_MASK;
                            if (!buffer.appendZeroCC(
                                    mapping, mapping + length, errorCode)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 >= minNoNoEmpty) {
                        // The current character maps to nothing.
                        // Simply omit it from the output if there is a boundary
                        // before _or_ after it. The character itself implies no
                        // boundaries.
                        if (hasCompBoundaryBefore(src, limit) ||
                            hasCompBoundaryAfter(
                                prevBoundary, prevSrc, onlyContiguous)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc, errorCode)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    }
                    // Other "noNo" type, or need to examine more text around
                    // this character: Fall through to the slow path.
                } else if (isJamoVT(norm16) && prevBoundary != prevSrc) {
                    UChar prev = *(prevSrc - 1);
                    if (c < Hangul::JAMO_T_BASE) {
                        // The current character is a Jamo Vowel,
                        // compose with previous Jamo L and following Jamo T.
                        UChar l = (UChar)(prev - Hangul::JAMO_L_BASE);
                        if (l < Hangul::JAMO_L_COUNT) {
                            if (!doCompose) {
                                return FALSE;
                            }
                            int32_t t;
                            if (src != limit &&
                                0 < (t =
                                         ((int32_t)*src -
                                          Hangul::JAMO_T_BASE)) &&
                                t < Hangul::JAMO_T_COUNT) {
                                // The next character is a Jamo T.
                                ++src;
                            } else if (hasCompBoundaryBefore(src, limit)) {
                                // No Jamo T follows, not even via
                                // decomposition.
                                t = 0;
                            } else {
                                t = -1;
                            }
                            if (t >= 0) {
                                UChar32 syllable = Hangul::HANGUL_BASE +
                                                   (l * Hangul::JAMO_V_COUNT +
                                                    (c - Hangul::JAMO_V_BASE)) *
                                                       Hangul::JAMO_T_COUNT +
                                                   t;
                                --prevSrc; // Replace the Jamo L as well.
                                if (prevBoundary != prevSrc &&
                                    !buffer.appendZeroCC(
                                        prevBoundary, prevSrc, errorCode)) {
                                    break;
                                }
                                if (!buffer.appendBMP(
                                        (UChar)syllable, 0, errorCode)) {
                                    break;
                                }
                                prevBoundary = src;
                                continue;
                            }
                            // If we see L+V+x where x!=T then we drop to the
                            // slow path, decompose and recompose. This is to
                            // deal with NFKC finding normal L and V but a
                            // compatibility variant of a T.
                            // We need to either fully compose that combination
                            // here (which would complicate the code and may not
                            // work with strange custom data) or use the slow
                            // path.
                        }
                    } else if (Hangul::isHangulLV(prev)) {
                        // The current character is a Jamo Trailing consonant,
                        // compose with previous Hangul LV that does not contain
                        // a Jamo T.
                        if (!doCompose) {
                            return FALSE;
                        }
                        UChar32 syllable = prev + c - Hangul::JAMO_T_BASE;
                        --prevSrc; // Replace the Hangul LV as well.
                        if (prevBoundary != prevSrc &&
                            !buffer.appendZeroCC(
                                prevBoundary, prevSrc, errorCode)) {
                            break;
                        }
                        if (!buffer.appendBMP((UChar)syllable, 0, errorCode)) {
                            break;
                        }
                        prevBoundary = src;
                        continue;
                    }
                    // No matching context, or may need to decompose surrounding
                    // text first: Fall through to the slow path.
                } else if (norm16 > JAMO_VT) { // norm16 >= MIN_YES_YES_WITH_CC
                    // One or more combining marks that do not combine-back:
                    // Check for canonical order, copy unchanged if ok and
                    // if followed by a character with a boundary-before.
                    uint8_t cc = getCCFromNormalYesOrMaybe(norm16); // cc!=0
                    if (onlyContiguous /* FCC */ &&
                        getPreviousTrailCC(prevBoundary, prevSrc) > cc) {
                        // Fails FCD test, need to decompose and contiguously
                        // recompose.
                        if (!doCompose) {
                            return FALSE;
                        }
                    } else {
                        // If !onlyContiguous (not FCC), then we ignore the tccc
                        // of the previous character which passed the quick
                        // check "yes && ccc==0" test.
                        const UChar * nextSrc;
                        uint16_t n16;
                        for (;;) {
                            if (src == limit) {
                                if (doCompose) {
                                    buffer.appendZeroCC(
                                        prevBoundary, limit, errorCode);
                                }
                                return TRUE;
                            }
                            uint8_t prevCC = cc;
                            nextSrc = src;
                            UCPTRIE_FAST_U16_NEXT(
                                normTrie, UCPTRIE_16, nextSrc, limit, c, n16);
                            if (n16 >= MIN_YES_YES_WITH_CC) {
                                cc = getCCFromNormalYesOrMaybe(n16);
                                if (prevCC > cc) {
                                    if (!doCompose) {
                                        return FALSE;
                                    }
                                    break;
                                }
                            } else {
                                break;
                            }
                            src = nextSrc;
                        }
                        // src is after the last in-order combining mark.
                        // If there is a boundary here, then we continue with no
                        // change.
                        if (norm16HasCompBoundaryBefore(n16)) {
                            if (isCompYesAndZeroCC(n16)) {
                                src = nextSrc;
                            }
                            continue;
                        }
                        // Use the slow path. There is no boundary in [prevSrc,
                        // src[.
                    }
                }

                // Slow path: Find the nearest boundaries around the current
                // character, decompose and recompose.
                if (prevBoundary != prevSrc &&
                    !norm16HasCompBoundaryBefore(norm16)) {
                    const UChar * p = prevSrc;
                    UCPTRIE_FAST_U16_PREV(
                        normTrie, UCPTRIE_16, prevBoundary, p, c, norm16);
                    if (!norm16HasCompBoundaryAfter(norm16, onlyContiguous)) {
                        prevSrc = p;
                    }
                }
                if (doCompose && prevBoundary != prevSrc &&
                    !buffer.appendZeroCC(prevBoundary, prevSrc, errorCode)) {
                    break;
                }
                int32_t recomposeStartIndex = buffer.length();
                // We know there is not a boundary here.
                decomposeShort(
                    prevSrc,
                    src,
                    FALSE /* !stopAtCompBoundary */,
                    onlyContiguous,
                    buffer,
                    errorCode);
                // Decompose until the next boundary.
                src = decomposeShort(
                    src,
                    limit,
                    TRUE /* stopAtCompBoundary */,
                    onlyContiguous,
                    buffer,
                    errorCode);
                if (U_FAILURE(errorCode)) {
                    break;
                }
                if ((src - prevSrc) >
                    INT32_MAX) { // guard before buffer.equals()
                    errorCode = U_INDEX_OUTOFBOUNDS_ERROR;
                    return TRUE;
                }
                recompose(buffer, recomposeStartIndex, onlyContiguous);
                if (!doCompose) {
                    if (!buffer.equals(prevSrc, src)) {
                        return FALSE;
                    }
                    buffer.remove();
                }
                prevBoundary = src;
            }
            return TRUE;
        }

        /** sink==nullptr: isNormalized() */
        UBool composeUTF8(
            UBool onlyContiguous,
            const uint8_t * src,
            const uint8_t * limit,
            ByteSink * sink,
            UErrorCode & errorCode) const
        {
            BOOST_ASSERT(limit != nullptr);
            UnicodeString s16;
            uint8_t minNoMaybeLead = leadByteForCP(minCompNoMaybeCP);
            const uint8_t * prevBoundary = src;

            for (;;) {
                // Fast path: Scan over a sequence of characters below the
                // minimum "no or maybe" code point, or with (compYes && ccc==0)
                // properties.
                const uint8_t * prevSrc;
                uint16_t norm16 = 0;
                for (;;) {
                    if (src == limit) {
                        if (prevBoundary != limit && sink != nullptr) {
                            ByteSinkUtil::appendUnchanged(
                                prevBoundary, limit, *sink, errorCode);
                        }
                        return TRUE;
                    }
                    if (*src < minNoMaybeLead) {
                        ++src;
                    } else {
                        prevSrc = src;
                        UCPTRIE_FAST_U8_NEXT(
                            normTrie, UCPTRIE_16, src, limit, norm16);
                        if (!isCompYesAndZeroCC(norm16)) {
                            break;
                        }
                    }
                }
                // isCompYesAndZeroCC(norm16) is false, that is,
                // norm16>=minNoNo. The current character is either a "noNo"
                // (has a mapping) or a "maybeYes" (combines backward) or a
                // "yesYes" with ccc!=0. It is not a Hangul syllable or Jamo L
                // because those have "yes" properties.

                // Medium-fast path: Handle cases that do not require full
                // decomposition and recomposition.
                if (!isMaybeOrNonZeroCC(
                        norm16)) { // minNoNo <= norm16 < minMaybeYes
                    if (sink == nullptr) {
                        return FALSE;
                    }
                    // Fast path for mapping a character that is immediately
                    // surrounded by boundaries. In this case, we need not
                    // decompose around the current character.
                    if (isDecompNoAlgorithmic(norm16)) {
                        // Maps to a single isCompYesAndZeroCC character
                        // which also implies hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, onlyContiguous) ||
                            hasCompBoundaryBefore(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, *sink, errorCode)) {
                                break;
                            }
                            appendCodePointDelta(
                                prevSrc,
                                src,
                                getAlgorithmicDelta(norm16),
                                *sink);
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 < minNoNoCompBoundaryBefore) {
                        // The mapping is comp-normalized which also implies
                        // hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, onlyContiguous) ||
                            hasCompBoundaryBefore(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, *sink, errorCode)) {
                                break;
                            }
                            const uint16_t * mapping = getMapping(norm16);
                            int32_t length = *mapping++ & MAPPING_LENGTH_MASK;
                            if (!ByteSinkUtil::appendChange(
                                    prevSrc,
                                    src,
                                    (const UChar *)mapping,
                                    length,
                                    *sink,
                                    errorCode)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 >= minNoNoEmpty) {
                        // The current character maps to nothing.
                        // Simply omit it from the output if there is a boundary
                        // before _or_ after it. The character itself implies no
                        // boundaries.
                        if (hasCompBoundaryBefore(src, limit) ||
                            hasCompBoundaryAfter(
                                prevBoundary, prevSrc, onlyContiguous)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, *sink, errorCode)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    }
                    // Other "noNo" type, or need to examine more text around
                    // this character: Fall through to the slow path.
                } else if (isJamoVT(norm16)) {
                    // Jamo L: E1 84 80..92
                    // Jamo V: E1 85 A1..B5
                    // Jamo T: E1 86 A8..E1 87 82
                    BOOST_ASSERT((src - prevSrc) == 3 && *prevSrc == 0xe1);
                    UChar32 prev = previousHangulOrJamo(prevBoundary, prevSrc);
                    if (prevSrc[1] == 0x85) {
                        // The current character is a Jamo Vowel,
                        // compose with previous Jamo L and following Jamo T.
                        UChar32 l = prev - Hangul::JAMO_L_BASE;
                        if ((uint32_t)l < Hangul::JAMO_L_COUNT) {
                            if (sink == nullptr) {
                                return FALSE;
                            }
                            int32_t t = getJamoTMinusBase(src, limit);
                            if (t >= 0) {
                                // The next character is a Jamo T.
                                src += 3;
                            } else if (hasCompBoundaryBefore(src, limit)) {
                                // No Jamo T follows, not even via
                                // decomposition.
                                t = 0;
                            }
                            if (t >= 0) {
                                UChar32 syllable = Hangul::HANGUL_BASE +
                                                   (l * Hangul::JAMO_V_COUNT +
                                                    (prevSrc[2] - 0xa1)) *
                                                       Hangul::JAMO_T_COUNT +
                                                   t;
                                prevSrc -= 3; // Replace the Jamo L as well.
                                if (prevBoundary != prevSrc &&
                                    !ByteSinkUtil::appendUnchanged(
                                        prevBoundary,
                                        prevSrc,
                                        *sink,
                                        errorCode)) {
                                    break;
                                }
                                ByteSinkUtil::appendCodePoint(
                                    prevSrc, src, syllable, *sink);
                                prevBoundary = src;
                                continue;
                            }
                            // If we see L+V+x where x!=T then we drop to the
                            // slow path, decompose and recompose. This is to
                            // deal with NFKC finding normal L and V but a
                            // compatibility variant of a T.
                            // We need to either fully compose that combination
                            // here (which would complicate the code and may not
                            // work with strange custom data) or use the slow
                            // path.
                        }
                    } else if (Hangul::isHangulLV(prev)) {
                        // The current character is a Jamo Trailing consonant,
                        // compose with previous Hangul LV that does not contain
                        // a Jamo T.
                        if (sink == nullptr) {
                            return FALSE;
                        }
                        UChar32 syllable =
                            prev + getJamoTMinusBase(prevSrc, src);
                        prevSrc -= 3; // Replace the Hangul LV as well.
                        if (prevBoundary != prevSrc &&
                            !ByteSinkUtil::appendUnchanged(
                                prevBoundary, prevSrc, *sink, errorCode)) {
                            break;
                        }
                        ByteSinkUtil::appendCodePoint(
                            prevSrc, src, syllable, *sink);
                        prevBoundary = src;
                        continue;
                    }
                    // No matching context, or may need to decompose surrounding
                    // text first: Fall through to the slow path.
                } else if (norm16 > JAMO_VT) { // norm16 >= MIN_YES_YES_WITH_CC
                    // One or more combining marks that do not combine-back:
                    // Check for canonical order, copy unchanged if ok and
                    // if followed by a character with a boundary-before.
                    uint8_t cc = getCCFromNormalYesOrMaybe(norm16); // cc!=0
                    if (onlyContiguous /* FCC */ &&
                        getPreviousTrailCC(prevBoundary, prevSrc) > cc) {
                        // Fails FCD test, need to decompose and contiguously
                        // recompose.
                        if (sink == nullptr) {
                            return FALSE;
                        }
                    } else {
                        // If !onlyContiguous (not FCC), then we ignore the tccc
                        // of the previous character which passed the quick
                        // check "yes && ccc==0" test.
                        const uint8_t * nextSrc;
                        uint16_t n16;
                        for (;;) {
                            if (src == limit) {
                                if (sink != nullptr) {
                                    ByteSinkUtil::appendUnchanged(
                                        prevBoundary, limit, *sink, errorCode);
                                }
                                return TRUE;
                            }
                            uint8_t prevCC = cc;
                            nextSrc = src;
                            UCPTRIE_FAST_U8_NEXT(
                                normTrie, UCPTRIE_16, nextSrc, limit, n16);
                            if (n16 >= MIN_YES_YES_WITH_CC) {
                                cc = getCCFromNormalYesOrMaybe(n16);
                                if (prevCC > cc) {
                                    if (sink == nullptr) {
                                        return FALSE;
                                    }
                                    break;
                                }
                            } else {
                                break;
                            }
                            src = nextSrc;
                        }
                        // src is after the last in-order combining mark.
                        // If there is a boundary here, then we continue with no
                        // change.
                        if (norm16HasCompBoundaryBefore(n16)) {
                            if (isCompYesAndZeroCC(n16)) {
                                src = nextSrc;
                            }
                            continue;
                        }
                        // Use the slow path. There is no boundary in [prevSrc,
                        // src[.
                    }
                }

                // Slow path: Find the nearest boundaries around the current
                // character, decompose and recompose.
                if (prevBoundary != prevSrc &&
                    !norm16HasCompBoundaryBefore(norm16)) {
                    const uint8_t * p = prevSrc;
                    UCPTRIE_FAST_U8_PREV(
                        normTrie, UCPTRIE_16, prevBoundary, p, norm16);
                    if (!norm16HasCompBoundaryAfter(norm16, onlyContiguous)) {
                        prevSrc = p;
                    }
                }
                ReorderingBuffer buffer(*this, s16, errorCode);
                if (U_FAILURE(errorCode)) {
                    break;
                }
                // We know there is not a boundary here.
                decomposeShort(
                    prevSrc,
                    src,
                    FALSE /* !stopAtCompBoundary */,
                    onlyContiguous,
                    buffer,
                    errorCode);
                // Decompose until the next boundary.
                src = decomposeShort(
                    src,
                    limit,
                    TRUE /* stopAtCompBoundary */,
                    onlyContiguous,
                    buffer,
                    errorCode);
                if (U_FAILURE(errorCode)) {
                    break;
                }
                if ((src - prevSrc) >
                    INT32_MAX) { // guard before buffer.equals()
                    errorCode = U_INDEX_OUTOFBOUNDS_ERROR;
                    return TRUE;
                }
                recompose(buffer, 0, onlyContiguous);
                if (!buffer.equals(prevSrc, src)) {
                    if (sink == nullptr) {
                        return FALSE;
                    }
                    if (prevBoundary != prevSrc &&
                        !ByteSinkUtil::appendUnchanged(
                            prevBoundary, prevSrc, *sink, errorCode)) {
                        break;
                    }
                    if (!ByteSinkUtil::appendChange(
                            prevSrc,
                            src,
                            buffer.getStart(),
                            buffer.length(),
                            *sink,
                            errorCode)) {
                        break;
                    }
                    prevBoundary = src;
                }
            }
            return TRUE;
        }

        UBool hasCompBoundaryBefore(UChar32 c) const
        {
            return c < minCompNoMaybeCP ||
                   norm16HasCompBoundaryBefore(getNorm16(c));
        }
        UBool hasCompBoundaryAfter(UChar32 c, UBool onlyContiguous) const
        {
            return norm16HasCompBoundaryAfter(getNorm16(c), onlyContiguous);
        }
        UBool isCompInert(UChar32 c, UBool onlyContiguous) const
        {
            uint16_t norm16 = getNorm16(c);
            return isCompYesAndZeroCC(norm16) &&
                   (norm16 & HAS_COMP_BOUNDARY_AFTER) != 0 &&
                   (!onlyContiguous || isInert(norm16) ||
                    *getMapping(norm16) <= 0x1ff);
        }

    private:
        static UChar const * u_strchr(UChar const * s, int c)
        {
            BOOST_ASSERT(s);
            for (auto curr = *s; curr && curr != c;) {
                curr = *++s;
            }
            return s;
        }

        /**
         * UTF-8 lead byte for minNoMaybeCP.
         * Can be lower than the actual lead byte for c.
         * Typically U+0300 for NFC/NFD, U+00A0 for NFKC/NFKD, U+0041 for
         * NFKC_Casefold.
         */
        static uint8_t leadByteForCP(UChar32 c)
        {
            if (c <= 0x7f) {
                return (uint8_t)c;
            } else if (c <= 0x7ff) {
                return (uint8_t)(0xc0 + (c >> 6));
            } else {
                // Should not occur because ccc(U+0300)!=0.
                return 0xe0;
            }
        }

        /**
         * Returns the code point from one single well-formed UTF-8 byte
         * sequence between cpStart and cpLimit.
         *
         * Trie UTF-8 macros do not assemble whole code points (for efficiency).
         * When we do need the code point, we call this function.
         * We should not need it for normalization-inert data (norm16==0).
         * Illegal sequences yield the error value norm16==0 just like real
         * normalization-inert code points.
         */
        static UChar32
        codePointFromValidUTF8(const uint8_t * cpStart, const uint8_t * cpLimit)
        {
            // Similar to U8_NEXT_UNSAFE(s, i, c).
            BOOST_ASSERT(cpStart < cpLimit);
            uint8_t c = *cpStart;
            switch (cpLimit - cpStart) {
            case 1: return c;
            case 2: return ((c & 0x1f) << 6) | (cpStart[1] & 0x3f);
            case 3:
                // no need for (c&0xf) because the upper bits are truncated
                // after <<12 in the cast to (UChar)
                return (UChar)(
                    (c << 12) | ((cpStart[1] & 0x3f) << 6) |
                    (cpStart[2] & 0x3f));
            case 4:
                return ((c & 7) << 18) | ((cpStart[1] & 0x3f) << 12) |
                       ((cpStart[2] & 0x3f) << 6) | (cpStart[3] & 0x3f);
            default:
                BOOST_ASSERT(FALSE); // Should not occur.
                return U_SENTINEL;
            }
        }

        /**
         * Returns the last code point in [start, p[ if it is valid and in
         * U+1000..U+D7FF. Otherwise returns a negative value.
         */
        static UChar32
        previousHangulOrJamo(const uint8_t * start, const uint8_t * p)
        {
            if ((p - start) >= 3) {
                p -= 3;
                uint8_t l = *p;
                uint8_t t1, t2;
                if (0xe1 <= l && l <= 0xed &&
                    (t1 = (uint8_t)(p[1] - 0x80)) <= 0x3f &&
                    (t2 = (uint8_t)(p[2] - 0x80)) <= 0x3f &&
                    (l < 0xed || t1 <= 0x1f)) {
                    return ((l & 0xf) << 12) | (t1 << 6) | t2;
                }
            }
            return U_SENTINEL;
        }

        /**
         * Returns the offset from the Jamo T base if [src, limit[ starts with a
         * single Jamo T code point. Otherwise returns a negative value.
         */
        static int32_t
        getJamoTMinusBase(const uint8_t * src, const uint8_t * limit)
        {
            // Jamo T: E1 86 A8..E1 87 82
            if ((limit - src) >= 3 && *src == 0xe1) {
                if (src[1] == 0x86) {
                    uint8_t t = src[2];
                    // The first Jamo T is U+11A8 but JAMO_T_BASE is 11A7.
                    // Offset 0 does not correspond to any conjoining Jamo.
                    if (0xa8 <= t && t <= 0xbf) {
                        return t - 0xa7;
                    }
                } else if (src[1] == 0x87) {
                    uint8_t t = src[2];
                    if ((int8_t)t <= (int8_t)0x82u) {
                        return t - (0xa7 - 0x40);
                    }
                }
            }
            return -1;
        }

        static void appendCodePointDelta(
            const uint8_t * cpStart,
            const uint8_t * cpLimit,
            int32_t delta,
            ByteSink & sink)
        {
            char buffer[U8_MAX_LENGTH];
            int32_t length;
            int32_t cpLength = (int32_t)(cpLimit - cpStart);
            if (cpLength == 1) {
                // The builder makes ASCII map to ASCII.
                buffer[0] = (uint8_t)(*cpStart + delta);
                length = 1;
            } else {
                int32_t trail = *(cpLimit - 1) + delta;
                if (0x80 <= trail && trail <= 0xbf) {
                    // The delta only changes the last trail byte.
                    --cpLimit;
                    length = 0;
                    do {
                        buffer[length++] = *cpStart++;
                    } while (cpStart < cpLimit);
                    buffer[length++] = (uint8_t)trail;
                } else {
                    // Decode the code point, add the delta, re-encode.
                    UChar32 c =
                        codePointFromValidUTF8(cpStart, cpLimit) + delta;
                    length = 0;
                    U8_APPEND_UNSAFE(buffer, length, c);
                }
            }
            sink.Append(buffer, length);
        }

        UBool isMaybe(uint16_t norm16) const
        {
            return minMaybeYes <= norm16 && norm16 <= JAMO_VT;
        }
        UBool isMaybeOrNonZeroCC(uint16_t norm16) const
        {
            return norm16 >= minMaybeYes;
        }
        static UBool isInert(uint16_t norm16) { return norm16 == INERT; }
        static UBool isJamoL(uint16_t norm16) { return norm16 == JAMO_L; }
        static UBool isJamoVT(uint16_t norm16) { return norm16 == JAMO_VT; }
        uint16_t hangulLVT() const
        {
            return minYesNoMappingsOnly | HAS_COMP_BOUNDARY_AFTER;
        }
        UBool isHangulLV(uint16_t norm16) const { return norm16 == minYesNo; }
        UBool isHangulLVT(uint16_t norm16) const
        {
            return norm16 == hangulLVT();
        }
        UBool isCompYesAndZeroCC(uint16_t norm16) const
        {
            return norm16 < minNoNo;
        }
        // UBool isCompYes(uint16_t norm16) const {
        //     return norm16>=MIN_YES_YES_WITH_CC || norm16<minNoNo;
        // }
        // UBool isCompYesOrMaybe(uint16_t norm16) const {
        //     return norm16<minNoNo || minMaybeYes<=norm16;
        // }
        // UBool hasZeroCCFromDecompYes(uint16_t norm16) const {
        //     return norm16<=MIN_NORMAL_MAYBE_YES || norm16==JAMO_VT;
        // }
        UBool isDecompYesAndZeroCC(uint16_t norm16) const
        {
            return norm16 < minYesNo || norm16 == JAMO_VT ||
                   (minMaybeYes <= norm16 && norm16 <= MIN_NORMAL_MAYBE_YES);
        }
        /**
         * A little faster and simpler than isDecompYesAndZeroCC() but does not
         * include the MaybeYes which combine-forward and have ccc=0. (Standard
         * Unicode 10 normalization does not have such characters.)
         */
        UBool isMostDecompYesAndZeroCC(uint16_t norm16) const
        {
            return norm16 < minYesNo || norm16 == MIN_NORMAL_MAYBE_YES ||
                   norm16 == JAMO_VT;
        }
        UBool isDecompNoAlgorithmic(uint16_t norm16) const
        {
            return norm16 >= limitNoNo;
        }

        // For use with isCompYes().
        // Perhaps the compiler can combine the two tests for
        // MIN_YES_YES_WITH_CC. static uint8_t getCCFromYes(uint16_t norm16) {
        //     return norm16>=MIN_YES_YES_WITH_CC ?
        //     getCCFromNormalYesOrMaybe(norm16) : 0;
        // }
        uint8_t getCCFromNoNo(uint16_t norm16) const
        {
            const uint16_t * mapping = getMapping(norm16);
            if (*mapping & MAPPING_HAS_CCC_LCCC_WORD) {
                return (uint8_t) * (mapping - 1);
            } else {
                return 0;
            }
        }
        // requires that the [cpStart..cpLimit[ character passes
        // isCompYesAndZeroCC()
        uint8_t getTrailCCFromCompYesAndZeroCC(uint16_t norm16) const
        {
            if (norm16 <= minYesNo) {
                return 0; // yesYes and Hangul LV have ccc=tccc=0
            } else {
                // For Hangul LVT we harmlessly fetch a firstUnit with tccc=0
                // here.
                return (uint8_t)(*getMapping(norm16) >> 8); // tccc from yesNo
            }
        }
        uint8_t getPreviousTrailCC(const UChar * start, const UChar * p) const
        {
            if (start == p) {
                return 0;
            }
            int32_t i = (int32_t)(p - start);
            UChar32 c;
            U16_PREV(start, 0, i, c);
            return (uint8_t)getFCD16(c);
        }
        uint8_t
        getPreviousTrailCC(const uint8_t * start, const uint8_t * p) const
        {
            if (start == p) {
                return 0;
            }
            int32_t i = (int32_t)(p - start);
            UChar32 c;
            U8_PREV(start, 0, i, c);
            return (uint8_t)getFCD16(c);
        }

        // Requires algorithmic-NoNo.
        UChar32 mapAlgorithmic(UChar32 c, uint16_t norm16) const
        {
            return c + (norm16 >> DELTA_SHIFT) - centerNoNoDelta;
        }
        UChar32 getAlgorithmicDelta(uint16_t norm16) const
        {
            return (norm16 >> DELTA_SHIFT) - centerNoNoDelta;
        }

        // Requires minYesNo<norm16<limitNoNo.
        const uint16_t * getMapping(uint16_t norm16) const
        {
            return extraData + (norm16 >> OFFSET_SHIFT);
        }
        const uint16_t * getCompositionsListForDecompYes(uint16_t norm16) const
        {
            if (norm16 < JAMO_L || MIN_NORMAL_MAYBE_YES <= norm16) {
                return nullptr;
            } else if (norm16 < minMaybeYes) {
                return getMapping(
                    norm16); // for yesYes; if Jamo L: harmless empty list
            } else {
                return maybeYesCompositions + norm16 - minMaybeYes;
            }
        }
        const uint16_t * getCompositionsListForComposite(uint16_t norm16) const
        {
            // A composite has both mapping & compositions list.
            const uint16_t * list = getMapping(norm16);
            return list + // mapping pointer
                   1 +    // +1 to skip the first unit with the mapping length
                   (*list & MAPPING_LENGTH_MASK); // + mapping length
        }
        const uint16_t * getCompositionsListForMaybe(uint16_t norm16) const
        {
            // minMaybeYes<=norm16<MIN_NORMAL_MAYBE_YES
            return maybeYesCompositions +
                   ((norm16 - minMaybeYes) >> OFFSET_SHIFT);
        }
        /**
         * @param c code point must have compositions
         * @return compositions list pointer
         */
        const uint16_t * getCompositionsList(uint16_t norm16) const
        {
            return isDecompYes(norm16)
                       ? getCompositionsListForDecompYes(norm16)
                       : getCompositionsListForComposite(norm16);
        }

        const UChar * copyLowPrefixFromNulTerminated(
            const UChar * src,
            UChar32 minNeedDataCP,
            ReorderingBuffer * buffer,
            UErrorCode & errorCode) const
        {
            // Make some effort to support NUL-terminated strings reasonably.
            // Take the part of the fast quick check loop that does not look up
            // data and check the first part of the string.
            // After this prefix, determine the string length to simplify the
            // rest of the code.
            const UChar * prevSrc = src;
            UChar c;
            while ((c = *src++) < minNeedDataCP && c != 0) {
            }
            // Back out the last character for full processing.
            // Copy this prefix.
            if (--src != prevSrc) {
                if (buffer != nullptr) {
                    buffer->appendZeroCC(prevSrc, src, errorCode);
                }
            }
            return src;
        }
        // Decompose a short piece of text which is likely to contain characters
        // that fail the quick check loop and/or where the quick check loop's
        // overhead is unlikely to be amortized. Called by the compose() and
        // makeFCD() implementations.
        const UChar * decomposeShort(
            const UChar * src,
            const UChar * limit,
            UBool stopAtCompBoundary,
            UBool onlyContiguous,
            ReorderingBuffer & buffer,
            UErrorCode & errorCode) const
        {
            if (U_FAILURE(errorCode)) {
                return nullptr;
            }
            while (src < limit) {
                if (stopAtCompBoundary && *src < minCompNoMaybeCP) {
                    return src;
                }
                const UChar * prevSrc = src;
                UChar32 c;
                uint16_t norm16;
                UCPTRIE_FAST_U16_NEXT(
                    normTrie, UCPTRIE_16, src, limit, c, norm16);
                if (stopAtCompBoundary && norm16HasCompBoundaryBefore(norm16)) {
                    return prevSrc;
                }
                if (!decompose(c, norm16, buffer, errorCode)) {
                    return nullptr;
                }
                if (stopAtCompBoundary &&
                    norm16HasCompBoundaryAfter(norm16, onlyContiguous)) {
                    return src;
                }
            }
            return src;
        }
        UBool decompose(
            UChar32 c,
            uint16_t norm16,
            ReorderingBuffer & buffer,
            UErrorCode & errorCode) const
        {
            // get the decomposition and the lead and trail cc's
            if (norm16 >= limitNoNo) {
                if (isMaybeOrNonZeroCC(norm16)) {
                    return buffer.append(
                        c, getCCFromYesOrMaybe(norm16), errorCode);
                }
                // Maps to an isCompYesAndZeroCC.
                c = mapAlgorithmic(c, norm16);
                norm16 = getRawNorm16(c);
            }
            if (norm16 < minYesNo) {
                // c does not decompose
                return buffer.append(c, 0, errorCode);
            } else if (isHangulLV(norm16) || isHangulLVT(norm16)) {
                // Hangul syllable: decompose algorithmically
                UChar jamos[3];
                return buffer.appendZeroCC(
                    jamos, jamos + Hangul::decompose(c, jamos), errorCode);
            }
            // c decomposes, get everything from the variable-length extra data
            const uint16_t * mapping = getMapping(norm16);
            uint16_t firstUnit = *mapping;
            int32_t length = firstUnit & MAPPING_LENGTH_MASK;
            uint8_t leadCC, trailCC;
            trailCC = (uint8_t)(firstUnit >> 8);
            if (firstUnit & MAPPING_HAS_CCC_LCCC_WORD) {
                leadCC = (uint8_t)(*(mapping - 1) >> 8);
            } else {
                leadCC = 0;
            }
            return buffer.append(
                (const UChar *)mapping + 1,
                length,
                TRUE,
                leadCC,
                trailCC,
                errorCode);
        }

        const uint8_t * decomposeShort(
            const uint8_t * src,
            const uint8_t * limit,
            UBool stopAtCompBoundary,
            UBool onlyContiguous,
            ReorderingBuffer & buffer,
            UErrorCode & errorCode) const
        {
            if (U_FAILURE(errorCode)) {
                return nullptr;
            }
            while (src < limit) {
                const uint8_t * prevSrc = src;
                uint16_t norm16;
                UCPTRIE_FAST_U8_NEXT(normTrie, UCPTRIE_16, src, limit, norm16);
                // Get the decomposition and the lead and trail cc's.
                UChar32 c = U_SENTINEL;
                if (norm16 >= limitNoNo) {
                    if (isMaybeOrNonZeroCC(norm16)) {
                        // No boundaries around this character.
                        c = codePointFromValidUTF8(prevSrc, src);
                        if (!buffer.append(
                                c, getCCFromYesOrMaybe(norm16), errorCode)) {
                            return nullptr;
                        }
                        continue;
                    }
                    // Maps to an isCompYesAndZeroCC.
                    if (stopAtCompBoundary) {
                        return prevSrc;
                    }
                    c = codePointFromValidUTF8(prevSrc, src);
                    c = mapAlgorithmic(c, norm16);
                    norm16 = getRawNorm16(c);
                } else if (
                    stopAtCompBoundary && norm16 < minNoNoCompNoMaybeCC) {
                    return prevSrc;
                }
                // norm16!=INERT guarantees that [prevSrc, src[ is valid UTF-8.
                // We do not see invalid UTF-8 here because
                // its norm16==INERT is normalization-inert,
                // so it gets copied unchanged in the fast path,
                // and we stop the slow path where invalid UTF-8 begins.
                BOOST_ASSERT(norm16 != INERT);
                if (norm16 < minYesNo) {
                    if (c < 0) {
                        c = codePointFromValidUTF8(prevSrc, src);
                    }
                    // does not decompose
                    if (!buffer.append(c, 0, errorCode)) {
                        return nullptr;
                    }
                } else if (isHangulLV(norm16) || isHangulLVT(norm16)) {
                    // Hangul syllable: decompose algorithmically
                    if (c < 0) {
                        c = codePointFromValidUTF8(prevSrc, src);
                    }
                    char16_t jamos[3];
                    if (!buffer.appendZeroCC(
                            jamos,
                            jamos + Hangul::decompose(c, jamos),
                            errorCode)) {
                        return nullptr;
                    }
                } else {
                    // The character decomposes, get everything from the
                    // variable-length extra data.
                    const uint16_t * mapping = getMapping(norm16);
                    uint16_t firstUnit = *mapping;
                    int32_t length = firstUnit & MAPPING_LENGTH_MASK;
                    uint8_t trailCC = (uint8_t)(firstUnit >> 8);
                    uint8_t leadCC;
                    if (firstUnit & MAPPING_HAS_CCC_LCCC_WORD) {
                        leadCC = (uint8_t)(*(mapping - 1) >> 8);
                    } else {
                        leadCC = 0;
                    }
                    if (!buffer.append(
                            (const char16_t *)mapping + 1,
                            length,
                            TRUE,
                            leadCC,
                            trailCC,
                            errorCode)) {
                        return nullptr;
                    }
                }
                if (stopAtCompBoundary &&
                    norm16HasCompBoundaryAfter(norm16, onlyContiguous)) {
                    return src;
                }
            }
            return src;
        }

        /*
         * Finds the recomposition result for
         * a forward-combining "lead" character,
         * specified with a pointer to its compositions list,
         * and a backward-combining "trail" character.
         *
         * If the lead and trail characters combine, then this function returns
         * the following "compositeAndFwd" value:
         * Bits 21..1  composite character
         * Bit      0  set if the composite is a forward-combining starter
         * otherwise it returns -1.
         *
         * The compositions list has (trail, compositeAndFwd) pair entries,
         * encoded as either pairs or triples of 16-bit units.
         * The last entry has the high bit of its first unit set.
         *
         * The list is sorted by ascending trail characters (there are no
         * duplicates). A linear search is used.
         *
         * See normalizer2impl.h for a more detailed description
         * of the compositions list format.
         */
        static int32_t combine(const uint16_t * list, UChar32 trail)
        {
            uint16_t key1, firstUnit;
            if (trail < COMP_1_TRAIL_LIMIT) {
                // trail character is 0..33FF
                // result entry may have 2 or 3 units
                key1 = (uint16_t)(trail << 1);
                while (key1 > (firstUnit = *list)) {
                    list += 2 + (firstUnit & COMP_1_TRIPLE);
                }
                if (key1 == (firstUnit & COMP_1_TRAIL_MASK)) {
                    if (firstUnit & COMP_1_TRIPLE) {
                        return ((int32_t)list[1] << 16) | list[2];
                    } else {
                        return list[1];
                    }
                }
            } else {
                // trail character is 3400..10FFFF
                // result entry has 3 units
                key1 = (uint16_t)(
                    COMP_1_TRAIL_LIMIT +
                    (((trail >> COMP_1_TRAIL_SHIFT)) & ~COMP_1_TRIPLE));
                uint16_t key2 = (uint16_t)(trail << COMP_2_TRAIL_SHIFT);
                uint16_t secondUnit;
                for (;;) {
                    if (key1 > (firstUnit = *list)) {
                        list += 2 + (firstUnit & COMP_1_TRIPLE);
                    } else if (key1 == (firstUnit & COMP_1_TRAIL_MASK)) {
                        if (key2 > (secondUnit = list[1])) {
                            if (firstUnit & COMP_1_LAST_TUPLE) {
                                break;
                            } else {
                                list += 3;
                            }
                        } else if (key2 == (secondUnit & COMP_2_TRAIL_MASK)) {
                            return ((int32_t)(secondUnit & ~COMP_2_TRAIL_MASK)
                                    << 16) |
                                   list[2];
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
            return -1;
        }
        /*
         * Recomposes the buffer text starting at recomposeStartIndex
         * (which is in NFD - decomposed and canonically ordered),
         * and truncates the buffer contents.
         *
         * Note that recomposition never lengthens the text:
         * Any character consists of either one or two code units;
         * a composition may contain at most one more code unit than the
         * original starter, while the combining mark that is removed has at
         * least one code unit.
         */
        void recompose(
            ReorderingBuffer & buffer,
            int32_t recomposeStartIndex,
            UBool onlyContiguous) const
        {
            UChar * p = buffer.getStart() + recomposeStartIndex;
            UChar * limit = buffer.getLimit();
            if (p == limit) {
                return;
            }

            UChar *starter, *pRemove, *q, *r;
            const uint16_t * compositionsList;
            UChar32 c, compositeAndFwd;
            uint16_t norm16;
            uint8_t cc, prevCC;
            UBool starterIsSupplementary;

            // Some of the following variables are not used until we have a
            // forward-combining starter and are only initialized now to avoid
            // compiler warnings.
            compositionsList = nullptr; // used as indicator for whether we have
                                        // a forward-combining starter
            starter = nullptr;
            starterIsSupplementary = FALSE;
            prevCC = 0;

            for (;;) {
                UCPTRIE_FAST_U16_NEXT(
                    normTrie, UCPTRIE_16, p, limit, c, norm16);
                cc = getCCFromYesOrMaybe(norm16);
                if ( // this character combines backward and
                    isMaybe(norm16) &&
                    // we have seen a starter that combines forward and
                    compositionsList != nullptr &&
                    // the backward-combining character is not blocked
                    (prevCC < cc || prevCC == 0)) {
                    if (isJamoVT(norm16)) {
                        // c is a Jamo V/T, see if we can compose it with the
                        // previous character.
                        if (c < Hangul::JAMO_T_BASE) {
                            // c is a Jamo Vowel, compose with previous Jamo L
                            // and following Jamo T.
                            UChar prev =
                                (UChar)(*starter - Hangul::JAMO_L_BASE);
                            if (prev < Hangul::JAMO_L_COUNT) {
                                pRemove = p - 1;
                                UChar syllable = (UChar)(
                                    Hangul::HANGUL_BASE +
                                    (prev * Hangul::JAMO_V_COUNT +
                                     (c - Hangul::JAMO_V_BASE)) *
                                        Hangul::JAMO_T_COUNT);
                                UChar t;
                                if (p != limit &&
                                    (t = (UChar)(*p - Hangul::JAMO_T_BASE)) <
                                        Hangul::JAMO_T_COUNT) {
                                    ++p;
                                    syllable +=
                                        t; // The next character was a Jamo T.
                                }
                                *starter = syllable;
                                // remove the Jamo V/T
                                q = pRemove;
                                r = p;
                                while (r < limit) {
                                    *q++ = *r++;
                                }
                                limit = q;
                                p = pRemove;
                            }
                        }
                        /*
                         * No "else" for Jamo T:
                         * Since the input is in NFD, there are no Hangul LV
                         * syllables that a Jamo T could combine with. All Jamo
                         * Ts are combined above when handling Jamo Vs.
                         */
                        if (p == limit) {
                            break;
                        }
                        compositionsList = nullptr;
                        continue;
                    } else if (
                        (compositeAndFwd = combine(compositionsList, c)) >= 0) {
                        // The starter and the combining mark (c) do combine.
                        UChar32 composite = compositeAndFwd >> 1;

                        // Replace the starter with the composite, remove the
                        // combining mark.
                        pRemove = p - U16_LENGTH(c); // pRemove & p: start &
                                                     // limit of the combining
                                                     // mark
                        if (starterIsSupplementary) {
                            if (U_IS_SUPPLEMENTARY(composite)) {
                                // both are supplementary
                                starter[0] = U16_LEAD(composite);
                                starter[1] = U16_TRAIL(composite);
                            } else {
                                *starter = (UChar)composite;
                                // The composite is shorter than the starter,
                                // move the intermediate characters forward one.
                                starterIsSupplementary = FALSE;
                                q = starter + 1;
                                r = q + 1;
                                while (r < pRemove) {
                                    *q++ = *r++;
                                }
                                --pRemove;
                            }
                        } else if (U_IS_SUPPLEMENTARY(composite)) {
                            // The composite is longer than the starter,
                            // move the intermediate characters back one.
                            starterIsSupplementary = TRUE;
                            ++starter; // temporarily increment for the loop
                                       // boundary
                            q = pRemove;
                            r = ++pRemove;
                            while (starter < q) {
                                *--r = *--q;
                            }
                            *starter = U16_TRAIL(composite);
                            *--starter = U16_LEAD(
                                composite); // undo the temporary increment
                        } else {
                            // both are on the BMP
                            *starter = (UChar)composite;
                        }

                        /* remove the combining mark by moving the following
                         * text over it */
                        if (pRemove < p) {
                            q = pRemove;
                            r = p;
                            while (r < limit) {
                                *q++ = *r++;
                            }
                            limit = q;
                            p = pRemove;
                        }
                        // Keep prevCC because we removed the combining mark.

                        if (p == limit) {
                            break;
                        }
                        // Is the composite a starter that combines forward?
                        if (compositeAndFwd & 1) {
                            compositionsList = getCompositionsListForComposite(
                                getRawNorm16(composite));
                        } else {
                            compositionsList = nullptr;
                        }

                        // We combined; continue with looking for compositions.
                        continue;
                    }
                }

                // no combination this time
                prevCC = cc;
                if (p == limit) {
                    break;
                }

                // If c did not combine, then check if it is a starter.
                if (cc == 0) {
                    // Found a new starter.
                    if ((compositionsList = getCompositionsListForDecompYes(
                             norm16)) != nullptr) {
                        // It may combine with something, prepare for it.
                        if (U_IS_BMP(c)) {
                            starterIsSupplementary = FALSE;
                            starter = p - 1;
                        } else {
                            starterIsSupplementary = TRUE;
                            starter = p - 2;
                        }
                    }
                } else if (onlyContiguous) {
                    // FCC: no discontiguous compositions; any intervening
                    // character blocks.
                    compositionsList = nullptr;
                }
            }
            buffer.setReorderingLimit(limit);
        }

        UBool hasCompBoundaryBefore(UChar32 c, uint16_t norm16) const
        {
            return c < minCompNoMaybeCP || norm16HasCompBoundaryBefore(norm16);
        }
        UBool norm16HasCompBoundaryBefore(uint16_t norm16) const
        {
            return norm16 < minNoNoCompNoMaybeCC || isAlgorithmicNoNo(norm16);
        }
        UBool
        hasCompBoundaryBefore(const UChar * src, const UChar * limit) const
        {
            if (src == limit || *src < minCompNoMaybeCP) {
                return TRUE;
            }
            UChar32 c;
            uint16_t norm16;
            UCPTRIE_FAST_U16_NEXT(normTrie, UCPTRIE_16, src, limit, c, norm16);
            return norm16HasCompBoundaryBefore(norm16);
        }
        UBool
        hasCompBoundaryBefore(const uint8_t * src, const uint8_t * limit) const
        {
            if (src == limit) {
                return TRUE;
            }
            uint16_t norm16;
            UCPTRIE_FAST_U8_NEXT(normTrie, UCPTRIE_16, src, limit, norm16);
            return norm16HasCompBoundaryBefore(norm16);
        }
        UBool hasCompBoundaryAfter(
            const UChar * start, const UChar * p, UBool onlyContiguous) const
        {
            if (start == p) {
                return TRUE;
            }
            UChar32 c;
            uint16_t norm16;
            UCPTRIE_FAST_U16_PREV(normTrie, UCPTRIE_16, start, p, c, norm16);
            return norm16HasCompBoundaryAfter(norm16, onlyContiguous);
        }
        UBool hasCompBoundaryAfter(
            const uint8_t * start,
            const uint8_t * p,
            UBool onlyContiguous) const
        {
            if (start == p) {
                return TRUE;
            }
            uint16_t norm16;
            UCPTRIE_FAST_U8_PREV(normTrie, UCPTRIE_16, start, p, norm16);
            return norm16HasCompBoundaryAfter(norm16, onlyContiguous);
        }
        UBool
        norm16HasCompBoundaryAfter(uint16_t norm16, UBool onlyContiguous) const
        {
            return (norm16 & HAS_COMP_BOUNDARY_AFTER) != 0 &&
                   (!onlyContiguous || isTrailCC01ForCompBoundaryAfter(norm16));
        }
        /** For FCC: Given norm16 HAS_COMP_BOUNDARY_AFTER, does it have tccc<=1?
         */
        UBool isTrailCC01ForCompBoundaryAfter(uint16_t norm16) const
        {
            return isInert(norm16) ||
                   (isDecompNoAlgorithmic(norm16)
                        ? (norm16 & DELTA_TCCC_MASK) <= DELTA_TCCC_1
                        : *getMapping(norm16) <= 0x1ff);
        }

        // UVersionInfo dataVersion;

        // BMP code point thresholds for quick check loops looking at single
        // UTF-16 code units.
        UChar minDecompNoCP;
        UChar minCompNoMaybeCP;
        UChar minLcccCP;

        // Norm16 value thresholds for quick check combinations and types of
        // extra data.
        uint16_t minYesNo;
        uint16_t minYesNoMappingsOnly;
        uint16_t minNoNo;
        uint16_t minNoNoCompBoundaryBefore;
        uint16_t minNoNoCompNoMaybeCC;
        uint16_t minNoNoEmpty;
        uint16_t limitNoNo;
        uint16_t centerNoNoDelta;
        uint16_t minMaybeYes;

        const UCPTrie * normTrie;
        const uint16_t * maybeYesCompositions;
        const uint16_t * extraData; // mappings and/or compositions for yesYes,
                                    // yesNo & noNo characters
        const uint8_t * smallFCD; // [0x100] one bit per 32 BMP code points, set
                                  // if any FCD!=0
    };

    // implementations
    inline UBool ReorderingBuffer::append(
        const UChar * s,
        int32_t length,
        UBool isNFD,
        uint8_t leadCC,
        uint8_t trailCC,
        UErrorCode & errorCode)
    {
        if (length == 0) {
            return TRUE;
        }
        if (remainingCapacity < length && !resize(length, errorCode)) {
            return FALSE;
        }
        remainingCapacity -= length;
        if (lastCC <= leadCC || leadCC == 0) {
            if (trailCC <= 1) {
                reorderStart = limit + length;
            } else if (leadCC <= 1) {
                reorderStart = limit + 1; // Ok if not a code point boundary.
            }
            const UChar * sLimit = s + length;
            do {
                *limit++ = *s++;
            } while (s != sLimit);
            lastCC = trailCC;
        } else {
            int32_t i = 0;
            UChar32 c;
            U16_NEXT(s, i, length, c);
            insert(c, leadCC); // insert first code point
            while (i < length) {
                U16_NEXT(s, i, length, c);
                if (i < length) {
                    if (isNFD) {
                        leadCC = Normalizer2Impl::getCCFromYesOrMaybe(
                            impl.getRawNorm16(c));
                    } else {
                        leadCC = impl.getCC(impl.getNorm16(c));
                    }
                } else {
                    leadCC = trailCC;
                }
                append(c, leadCC, errorCode);
            }
        }
        return TRUE;
    }

    inline uint8_t ReorderingBuffer::previousCC()
    {
        codePointLimit = codePointStart;
        if (reorderStart >= codePointStart) {
            return 0;
        }
        UChar32 c = *--codePointStart;
        UChar c2;
        if (U16_IS_TRAIL(c) && start < codePointStart &&
            U16_IS_LEAD(c2 = *(codePointStart - 1))) {
            --codePointStart;
            c = U16_GET_SUPPLEMENTARY(c2, c);
        }
        return impl.getCCFromYesOrMaybeCP(c);
    }

}}}}

    /**
     * Format of Normalizer2 .nrm data files.
     * Format version 4.0.
     *
     * Normalizer2 .nrm data files provide data for the Unicode Normalization
     * algorithms. ICU ships with data files for standard Unicode Normalization
     * Forms NFC and NFD (nfc.nrm), NFKC and NFKD (nfkc.nrm) and NFKC_Casefold
     * (nfkc_cf.nrm). Custom (application-specific) data can be built into
     * additional .nrm files with the gennorm2 build tool. ICU ships with one
     * such file, uts46.nrm, for the implementation of UTS #46.
     *
     * Normalizer2.getInstance() causes a .nrm file to be loaded, unless it has
     * been cached already. Internally, Normalizer2Impl.load() reads the .nrm
     * file.
     *
     * A .nrm file begins with a standard ICU data file header
     * (DataHeader, see ucmndata.h and unicode/udata.h).
     * The UDataInfo.dataVersion field usually contains the Unicode version
     * for which the data was generated.
     *
     * After the header, the file contains the following parts.
     * Constants are defined as enum values of the Normalizer2Impl class.
     *
     * Many details of the data structures are described in the design doc
     * which is at http://site.icu-project.org/design/normalization/custom
     *
     * int32_t indexes[indexesLength]; --
     * indexesLength=indexes[IX_NORM_TRIE_OFFSET]/4;
     *
     *      The first eight indexes are byte offsets in ascending order.
     *      Each byte offset marks the start of the next part in the data file,
     *      and the end of the previous one.
     *      When two consecutive byte offsets are the same, then the
     * corresponding part is empty. Byte offsets are offsets from after the
     * header, that is, from the beginning of the indexes[]. Each part
     * starts at an offset with proper alignment for its data. If
     * necessary, the previous part may include padding bytes to achieve
     * this alignment.
     *
     *      minDecompNoCP=indexes[IX_MIN_DECOMP_NO_CP] is the lowest code point
     *      with a decomposition mapping, that is, with NF*D_QC=No.
     *      minCompNoMaybeCP=indexes[IX_MIN_COMP_NO_MAYBE_CP] is the lowest code
     * point with NF*C_QC=No (has a one-way mapping) or Maybe (combines
     * backward). minLcccCP=indexes[IX_MIN_LCCC_CP] (index 18, new in
     * formatVersion 3) is the lowest code point with lccc!=0.
     *
     *      The next eight indexes are thresholds of 16-bit trie values for
     * ranges of values indicating multiple normalization properties. They are
     * listed here in threshold order, not in the order they are stored in
     * the indexes. minYesNo=indexes[IX_MIN_YES_NO];
     *          minYesNoMappingsOnly=indexes[IX_MIN_YES_NO_MAPPINGS_ONLY];
     *          minNoNo=indexes[IX_MIN_NO_NO];
     *          minNoNoCompBoundaryBefore=indexes[IX_MIN_NO_NO_COMP_BOUNDARY_BEFORE];
     *          minNoNoCompNoMaybeCC=indexes[IX_MIN_NO_NO_COMP_NO_MAYBE_CC];
     *          minNoNoEmpty=indexes[IX_MIN_NO_NO_EMPTY];
     *          limitNoNo=indexes[IX_LIMIT_NO_NO];
     *          minMaybeYes=indexes[IX_MIN_MAYBE_YES];
     *      See the normTrie description below and the design doc for details.
     *
     * UCPTrie normTrie; -- see ucptrie_impl.h and ucptrie.h, same as Java
     * CodePointTrie
     *
     *      The trie holds the main normalization data. Each code point is
     * mapped to a 16-bit value. Rather than using independent bits in the value
     * (which would require more than 16 bits), information is extracted
     * primarily via range checks. Except, format version 3 uses bit 0 for
     * hasCompBoundaryAfter(). For example, a 16-bit value norm16 in the
     * range minYesNo<=norm16<minNoNo means that the character has
     * NF*C_QC=Yes and NF*D_QC=No properties, which means it has a two-way
     * (round-trip) decomposition mapping. Values in the range
     * 2<=norm16<limitNoNo are also directly indexes into the extraData
     *      pointing to mappings, compositions lists, or both.
     *      Value norm16==INERT (0 in versions 1 & 2, 1 in version 3)
     *      means that the character is normalization-inert, that is,
     *      it does not have a mapping, does not participate in composition, has
     * a zero canonical combining class, and forms a boundary where text before
     * it and after it can be normalized independently. For details about
     * how multiple properties are encoded in 16-bit values see the design
     * doc. Note that the encoding cannot express all combinations of the
     * properties involved; it only supports those combinations that are
     * allowed by the Unicode Normalization algorithms. Details are in the
     * design doc as well. The gennorm2 tool only builds .nrm files for
     * data that conforms to the limitations.
     *
     *      The trie has a value for each lead surrogate code unit representing
     * the "worst case" properties of the 1024 supplementary characters whose
     * UTF-16 form starts with the lead surrogate. If all of the 1024
     * supplementary characters are normalization-inert, then their lead
     * surrogate code unit has the trie value INERT. When the lead
     * surrogate unit's value exceeds the quick check minimum during
     * processing, the properties for the full supplementary code point need to
     * be looked up.
     *
     * uint16_t maybeYesCompositions[MIN_NORMAL_MAYBE_YES-minMaybeYes];
     * uint16_t extraData[];
     *
     *      There is only one byte offset for the end of these two arrays.
     *      The split between them is given by the constant and variable
     * mentioned above. In version 3, the difference must be shifted right by
     * OFFSET_SHIFT.
     *
     *      The maybeYesCompositions array contains compositions lists for
     * characters that combine both forward (as starters in composition pairs)
     *      and backward (as trailing characters in composition pairs).
     *      Such characters do not occur in Unicode 5.2 but are allowed by
     *      the Unicode Normalization algorithms.
     *      If there are no such characters, then
     * minMaybeYes==MIN_NORMAL_MAYBE_YES and the maybeYesCompositions array is
     * empty. If there are such characters, then minMaybeYes is subtracted
     * from their norm16 values to get the index into this array.
     *
     *      The extraData array contains compositions lists for "YesYes"
     * characters, followed by mappings and optional compositions lists for
     * "YesNo" characters, followed by only mappings for "NoNo" characters.
     *      (Referring to pairs of NFC/NFD quick check values.)
     *      The norm16 values of those characters are directly indexes into the
     * extraData array. In version 3, the norm16 values must be shifted right by
     * OFFSET_SHIFT for accessing extraData.
     *
     *      The data structures for compositions lists and mappings are
     * described in the design doc.
     *
     * uint8_t smallFCD[0x100]; -- new in format version 2
     *
     *      This is a bit set to help speed up FCD value lookups in the absence
     * of a full UTrie2 or other large data structure with the full FCD value
     * mapping.
     *
     *      Each smallFCD bit is set if any of the corresponding 32 BMP code
     * points has a non-zero FCD value (lccc!=0 or tccc!=0). Bit 0 of
     * smallFCD[0] is for U+0000..U+001F. Bit 7 of smallFCD[0xff] is for
     * U+FFE0..U+FFFF. A bit for 32 lead surrogates is set if any of the 32k
     * corresponding _supplementary_ code points has a non-zero FCD value.
     *
     *      This bit set is most useful for the large blocks of CJK characters
     * with FCD=0.
     *
     * Changes from format version 1 to format version 2
     * ---------------------------
     *
     * - Addition of data for raw (not recursively decomposed) mappings.
     *   + The MAPPING_NO_COMP_BOUNDARY_AFTER bit in the extraData is now also
     * set when the mapping is to an empty string or when the character
     * combines-forward. This subsumes the one actual use of the
     * MAPPING_PLUS_COMPOSITION_LIST bit which is then repurposed for the
     * MAPPING_HAS_RAW_MAPPING bit.
     *   + For details see the design doc.
     * - Addition of indexes[IX_MIN_YES_NO_MAPPINGS_ONLY] and separation of the
     * yesNo extraData into distinct ranges (combines-forward vs. not) so that a
     * range check can be used to find out if there is a compositions list.
     *   This is fully equivalent with formatVersion 1's
     * MAPPING_PLUS_COMPOSITION_LIST flag. It is needed for the new (in ICU 49)
     * composePair(), not for other normalization.
     * - Addition of the smallFCD[] bit set.
     *
     * Changes from format version 2 to format version 3 (ICU 60)
     * ------------------
     *
     * - norm16 bit 0 indicates hasCompBoundaryAfter(),
     *   except that for contiguous composition (FCC) the tccc must be checked
     * as well. Data indexes and ccc values are shifted left by one
     * (OFFSET_SHIFT). Thresholds like minNoNo are tested before shifting.
     *
     * - Algorithmic mapping deltas are shifted left by two more bits (total
     * DELTA_SHIFT), to make room for two bits (three values) indicating whether
     * the tccc is 0, 1, or greater. See DELTA_TCCC_MASK etc. This helps with
     * fetching tccc/FCD values and FCC hasCompBoundaryAfter(). minMaybeYes is
     * 8-aligned so that the DELTA_TCCC_MASK bits can be tested directly.
     *
     * - Algorithmic mappings are only used for mapping to "comp yes and ccc=0"
     * characters, and ASCII characters are mapped algorithmically only to other
     * ASCII characters. This helps with hasCompBoundaryBefore() and compose()
     * fast paths. It is never necessary any more to loop for algorithmic
     * mappings.
     *
     * - Addition of indexes[IX_MIN_NO_NO_COMP_BOUNDARY_BEFORE],
     *   indexes[IX_MIN_NO_NO_COMP_NO_MAYBE_CC], and
     * indexes[IX_MIN_NO_NO_EMPTY], and separation of the noNo extraData into
     * distinct ranges. With this, the noNo norm16 value indicates whether the
     * mapping is compose-normalized, not normalized but
     * hasCompBoundaryBefore(), not even that, or maps to an empty string.
     *   hasCompBoundaryBefore() can be determined solely from the norm16 value.
     *
     * - The norm16 value for Hangul LVT is now different from that for Hangul
     * LV, so that hasCompBoundaryAfter() need not check for the syllable type.
     *   For Hangul LV, minYesNo continues to be used (no comp-boundary-after).
     *   For Hangul LVT, minYesNoMappingsOnly|HAS_COMP_BOUNDARY_AFTER is used.
     *   The extraData units at these indexes are set to firstUnit=2 and
     * firstUnit=3, respectively, to simplify some code.
     *
     * - The extraData firstUnit bit 5 is no longer necessary
     *   (norm16 bit 0 used instead of firstUnit
     * MAPPING_NO_COMP_BOUNDARY_AFTER), is reserved again, and always set to 0.
     *
     * - Addition of indexes[IX_MIN_LCCC_CP], the first code point where
     * lccc!=0. This used to be hardcoded to U+0300, but in data like
     * NFKC_Casefold it is lower: U+00AD Soft Hyphen maps to an empty string,
     *   which is artificially assigned "worst case" values lccc=1 and tccc=255.
     *
     * - A mapping to an empty string has explicit lccc=1 and tccc=255 values.
     *
     * Changes from format version 3 to format version 4 (ICU 63)
     * ------------------
     *
     * Switched from UTrie2 to UCPTrie/CodePointTrie.
     *
     * The new trie no longer stores different values for surrogate code *units*
     * vs. surrogate code *points*. Lead surrogates still have values for
     * optimized UTF-16 string processing. When looking up code point
     * properties, the code now checks for lead surrogates and treats them as
     * inert.
     *
     * gennorm2 now has to reject mappings for surrogate code points.
     * UTS #46 maps unpaired surrogates to U+FFFD in code rather than via its
     * custom normalization data file.
     */

#endif
