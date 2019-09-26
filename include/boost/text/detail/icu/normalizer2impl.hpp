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

#include <boost/text/transcode_iterator.hpp>
#include <boost/text/detail/icu/bytesinkutil.hpp>
#include <boost/text/detail/icu/machine.hpp>
#include <boost/text/detail/icu/ucptrie.hpp>
#include <boost/text/detail/icu/utf8.hpp>
#include <boost/text/detail/icu/utf16.hpp>

#include <boost/assert.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/algorithm/cxx14/equal.hpp>

#include <cstring>
#include <mutex>


namespace boost { namespace text { inline namespace v1 { namespace detail { namespace icu {

    // Copy/pasted from unorm2.h.
    /**
     * Constants for normalization modes.
     * For details about standard Unicode normalization forms
     * and about the algorithms which are also used with custom mapping
     * tables see http://www.unicode.org/unicode/reports/tr15/
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
        unorm2_compose,
        /**
         * Map, and reorder canonically.
         * Same as standard NFD when using an "nfc" instance.
         * Same as standard NFKD when using an "nfkc" instance.
         * For details about standard Unicode normalization forms
         * see http://www.unicode.org/unicode/reports/tr15/
         * @stable ICU 4.4
         */
        unorm2_decompose,
        /**
         * "Fast C or D" form.
         * If a string is in this form, then further decomposition
         * <i>without reordering</i> would yield the same form as DECOMPOSE.
         * Text in "Fast C or D" form can be processed efficiently with data
         * tables that are "canonically closed", that is, that provide
         * equivalent data for equivalent text, without having to be fully
         * normalized. Not a standard Unicode normalization form. Not a
         * unique form: Different FCD strings can be canonically equivalent.
         * For details see http://www.unicode.org/notes/tn5/#FCD
         * @stable ICU 4.4
         */
        unorm2_fcd,
        /**
         * Compose only contiguously.
         * Also known as "FCC" or "Fast C Contiguous".
         * The result will often but not always be in NFC.
         * The result will conform to FCD which is useful for processing.
         * Not a standard Unicode normalization form.
         * For details see http://www.unicode.org/notes/tn5/#FCC
         * @stable ICU 4.4
         */
        unorm2_compose_contiguous
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
        unorm_no,
        /**
         * The input string is in the normalization form.
         * @stable ICU 2.0
         */
        unorm_yes,
        /**
         * The input string may or may not be in the normalization form.
         * This value is only returned for composition forms like NFC and
         * FCC, when a backward-combining character is found for which the
         * surrounding text would have to be analyzed further.
         * @stable ICU 2.0
         */
        unorm_maybe
    } UNormalizationCheckResult;

    // Copy/pasted from unorm.h.
    /**
     * Constants for normalization modes.
     * @deprecated ICU 56 Use unorm2.h instead.
     */
    typedef enum {
        /** No decomposition/composition. @deprecated ICU 56 Use unorm2.h
           instead. */
        unorm_none = 1,
        /** Canonical decomposition. @deprecated ICU 56 Use unorm2.h
           instead. */
        unorm_nfd = 2,
        /** Compatibility decomposition. @deprecated ICU 56 Use unorm2.h
           instead. */
        unorm_nfkd = 3,
        /** Canonical decomposition followed by canonical composition.
           @deprecated ICU 56 Use unorm2.h instead. */
        unorm_nfc = 4,
        /** Default normalization. @deprecated ICU 56 Use unorm2.h instead.
         */
        unorm_default = unorm_nfc,
        /** Compatibility decomposition followed by canonical composition.
           @deprecated ICU 56 Use unorm2.h instead. */
        unorm_nfkc = 5,
        /** "Fast C or D" form. @deprecated ICU 56 Use unorm2.h instead. */
        unorm_fcd = 6,

        /** One more than the highest normalization mode constant.
           @deprecated ICU 56 Use unorm2.h instead. */
        unorm_mode_count
    } UNormalizationMode;

    namespace Hangul {
        /* Korean Hangul and Jamo constants */
        enum {
            jamo_l_base = 0x1100, /* "lead" jamo */
            jamo_l_end = 0x1112,
            jamo_v_base = 0x1161, /* "vowel" jamo */
            jamo_v_end = 0x1175,
            jamo_t_base = 0x11a7, /* "trail" jamo */
            jamo_t_end = 0x11c2,

            hangul_base = 0xac00,
            hangul_end = 0xd7a3,

            jamo_l_count = 19,
            jamo_v_count = 21,
            jamo_t_count = 28,

            jamo_vt_count = jamo_v_count * jamo_t_count,

            hangul_count = jamo_l_count * jamo_v_count * jamo_t_count,
            hangul_limit = hangul_base + hangul_count
        };

        inline UBool isHangul(UChar32 c)
        {
            return hangul_base <= c && c < hangul_limit;
        }
        inline UBool isHangulLV(UChar32 c)
        {
            c -= hangul_base;
            return 0 <= c && c < hangul_count && c % jamo_t_count == 0;
        }
        inline UBool isJamoL(UChar32 c)
        {
            return (uint32_t)(c - jamo_l_base) < jamo_l_count;
        }
        inline UBool isJamoV(UChar32 c)
        {
            return (uint32_t)(c - jamo_v_base) < jamo_v_count;
        }
        inline UBool isJamoT(UChar32 c)
        {
            int32_t t = c - jamo_t_base;
            return 0 < t && t < jamo_t_count; // not JAMO_T_BASE itself
        }
        inline UBool isJamo(UChar32 c)
        {
            return jamo_l_base <= c && c <= jamo_t_end &&
                   (c <= jamo_l_end ||
                    (jamo_v_base <= c && c <= jamo_v_end) ||
                    jamo_t_base < c);
        }

        /**
         * Decomposes c, which must be a Hangul syllable, into buffer
         * and returns the length of the decomposition (2 or 3).
         */
        inline int32_t decompose(UChar32 c, UChar buffer[3])
        {
            c -= hangul_base;
            UChar32 c2 = c % jamo_t_count;
            c /= jamo_t_count;
            buffer[0] = (UChar)(jamo_l_base + c / jamo_v_count);
            buffer[1] = (UChar)(jamo_v_base + c % jamo_v_count);
            if (c2 == 0) {
                return 2;
            } else {
                buffer[2] = (UChar)(jamo_t_base + c2);
                return 3;
            }
        }

        /**
         * Decomposes c, which must be a Hangul syllable, into buffer.
         * This is the raw, not recursive, decomposition. Its length is
         * always 2.
         */
        inline void getRawDecomposition(UChar32 c, UChar buffer[2])
        {
            UChar32 orig = c;
            c -= hangul_base;
            UChar32 c2 = c % jamo_t_count;
            if (c2 == 0) {
                c /= jamo_t_count;
                buffer[0] = (UChar)(jamo_l_base + c / jamo_v_count);
                buffer[1] = (UChar)(jamo_v_base + c % jamo_v_count);
            } else {
                buffer[0] = (UChar)(orig - c2); // LV syllable
                buffer[1] = (UChar)(jamo_t_base + c2);
            }
        }
    }

    // These appenders are used by ReorderingBuffer.
    template<typename String>
    struct utf16_appender
    {
        explicit utf16_appender(String & s) : s_(&s) {}

        template<typename Iter>
        _16_iter_ret_t<void, Iter> append(Iter utf16_first, Iter utf16_last)
        {
            s_->insert(s_->end(), utf16_first, utf16_last);
        }

    private:
        String * s_;
    };

    template<typename String>
    struct utf16_to_utf8_string_appender
    {
        explicit utf16_to_utf8_string_appender(String & s) : s_(&s) {}

        template<typename Iter>
        _16_iter_ret_t<void, Iter> append(Iter utf16_first, Iter utf16_last)
        {
            auto const dist = std::distance(utf16_first, utf16_last);
            auto const initial_size = s_->size();
            s_->resize(initial_size + dist * 3, typename String::value_type{});
            auto * s_first = &*s_->begin();
            auto * out = s_first + initial_size;
            out = boost::text::v1::transcode_utf_16_to_8(
                utf16_first, utf16_last, out);
            s_->resize(out - s_first, typename String::value_type{});
        }

        int out() const noexcept { return 0; }

    private:
        String * s_;
    };

    template<typename UTF32OutIter>
    struct utf16_to_utf32_appender
    {
        explicit utf16_to_utf32_appender(UTF32OutIter out) : out_(out) {}

        template<typename Iter>
        _16_iter_ret_t<void, Iter> append(Iter utf16_first, Iter utf16_last)
        {
            out_ = boost::text::v1::transcode_utf_16_to_32(
                utf16_first, utf16_last, out_);
        }

        UTF32OutIter out() const { return out_; }

    private:
        UTF32OutIter out_;
    };

    class Normalizer2Impl;

    template<typename UTF16Appender>
    class ReorderingBuffer;

    template<typename UTF16Appender>
    struct flush_disinhibitor
    {
        flush_disinhibitor(ReorderingBuffer<UTF16Appender> & buf) :
            buf_(buf)
        {}
        ~flush_disinhibitor() { buf_.inhibit_flushes = false; }

    private:
        ReorderingBuffer<UTF16Appender> & buf_;
    };

    template<typename UTF16Appender>
    class ReorderingBuffer
    {
        friend flush_disinhibitor<UTF16Appender>;

    public:
        ReorderingBuffer(
            const Normalizer2Impl & ni, UTF16Appender & appendr) :
            impl(ni),
            appender(appendr),
            buf(),
            reorderStart(begin()),
            limit(begin()),
            lastCC(0),
            inhibit_flushes(false)
        {}
        ~ReorderingBuffer() { flush(); }

        flush_disinhibitor<UTF16Appender> inhibit_flush()
        {
            inhibit_flushes = true;
            return flush_disinhibitor<UTF16Appender>(*this);
        }

        int size() const noexcept { return limit - begin(); }

        UChar const * begin() const { return buf.data(); }
        UChar const * end() const { return limit; }
        UChar * begin() { return buf.data(); }
        UChar * end() { return limit; }

        template<typename Iter>
        UBool equals_utf16(Iter otherStart, Iter otherLimit) const
        {
            return algorithm::equal(begin(), end(), otherStart, otherLimit);
        }
        template<typename CharIter>
        UBool equals_utf8(CharIter otherStart, CharIter otherLimit) const
        {
            auto const other_first =
                boost::text::v1::make_utf_8_to_16_iterator(
                    otherStart, otherStart, otherLimit);
            auto const other_last =
                boost::text::v1::make_utf_8_to_16_iterator(
                    otherStart, otherLimit, otherLimit);
            return algorithm::equal(
                begin(), end(), other_first, other_last);
        }

        UBool append(UChar32 c, uint8_t cc)
        {
            return (c <= 0xffff) ? appendBMP((UChar)c, cc)
                                 : appendSupplementary(c, cc);
        }
        UBool append(
            const UChar * s,
            int32_t length,
            UBool isNFD,
            uint8_t leadCC,
            uint8_t trailCC);
        UBool appendBMP(UChar c, uint8_t cc)
        {
            if (lastCC <= cc || cc == 0) {
                if (cc == 0 && !inhibit_flushes)
                    flush();
                *limit++ = c;
                lastCC = cc;
                if (cc <= 1) {
                    reorderStart = limit;
                }
            } else {
                insert(c, cc);
            }
            return true;
        }
        template<typename U16Iter>
        UBool appendZeroCC(U16Iter s, U16Iter sLimit)
        {
            BOOST_ASSERT(s != sLimit);
            if (!inhibit_flushes) {
                flush();
                auto second_to_last = std::prev(sLimit);
                if (boost::text::v1::low_surrogate(*second_to_last))
                    --second_to_last;
                appender.append(s, second_to_last);
                limit = std::copy(second_to_last, sLimit, limit);
            } else {
                limit = std::copy(s, sLimit, limit);
            }
            lastCC = 0;
            reorderStart = limit;
            return true;
        }
        void remove()
        {
            reorderStart = limit = begin();
            lastCC = 0;
        }
        void setReorderingLimit(UChar * newLimit)
        {
            reorderStart = limit = newLimit;
            lastCC = 0;
        }

    private:
        void flush()
        {
            appender.append(begin(), limit);
            remove();
        }

        /*
         * TODO: Revisit whether it makes sense to track reorderStart.
         * It is set to after the last known character with cc<=1,
         * which stops previousCC() before it reads that character and looks
         * up its cc. previousCC() is normally only called from insert(). In
         * other words, reorderStart speeds up the insertion of a combining
         * mark into a multi-combining mark sequence where it does not
         * belong at the end. This might not be worth the trouble. On the
         * other hand, it's not a huge amount of trouble.
         *
         * We probably need it for UNORM_SIMPLE_APPEND.
         */

        UBool appendSupplementary(UChar32 c, uint8_t cc)
        {
            if (lastCC <= cc || cc == 0) {
                if (cc == 0 && !inhibit_flushes)
                    flush();
                limit[0] = detail::icu::u16_lead(c);
                limit[1] = detail::icu::u16_trail(c);
                limit += 2;
                lastCC = cc;
                if (cc <= 1) {
                    reorderStart = limit;
                }
            } else {
                insert(c, cc);
            }
            return true;
        }
        // Inserts c somewhere before the last character.
        // Requires 0<cc<lastCC which implies reorderStart<limit.
        void insert(UChar32 c, uint8_t cc)
        {
            for (setIterator(), skipPrevious(); previousCC() > cc;) {
            }
            // insert c at codePointLimit, after the character with
            // prevCC<=cc
            UChar * q = limit;
            UChar * r = limit += detail::icu::u16_length(c);
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
                p[0] = detail::icu::u16_lead(c);
                p[1] = detail::icu::u16_trail(c);
            }
        }

        const Normalizer2Impl & impl;
        UTF16Appender & appender;
        std::array<UChar, 1024> buf;
        UChar *reorderStart, *limit;
        uint8_t lastCC;
        bool inhibit_flushes;

        // private backward iterator
        void setIterator() { codePointStart = limit; }
        // Requires start<codePointStart.
        void skipPrevious()
        {
            codePointLimit = codePointStart;
            UChar c = *--codePointStart;
            if (detail::icu::u16_is_trail(c) && begin() < codePointStart &&
                detail::icu::u16_is_lead(*(codePointStart - 1))) {
                --codePointStart;
            }
        }
        // Returns 0 if there is no previous character.
        uint8_t previousCC();

        UChar *codePointStart, *codePointLimit;
    };

    /**
     * Low-level implementation of the Unicode Normalization Algorithm.
     * For the data structure and details see the documentation at the end
     * of this normalizer2impl.h and in the design doc at
     * http://site.icu-project.org/design/normalization/custom
     */
    class Normalizer2Impl
    {
    public:
        Normalizer2Impl() : normTrie(nullptr) {}
        ~Normalizer2Impl() {}

        void init(
            const int32_t * inIndexes,
            const UCPTrie * inTrie,
            const uint16_t * inExtraData,
            const uint8_t * inSmallFCD)
        {
            minDecompNoCP =
                static_cast<UChar>(inIndexes[ix_min_decomp_no_cp]);
            minCompNoMaybeCP =
                static_cast<UChar>(inIndexes[ix_min_comp_no_maybe_cp]);
            minLcccCP = static_cast<UChar>(inIndexes[ix_min_lccc_cp]);

            minYesNo = static_cast<uint16_t>(inIndexes[ix_min_yes_no]);
            minYesNoMappingsOnly = static_cast<uint16_t>(
                inIndexes[ix_min_yes_no_mappings_only]);
            minNoNo = static_cast<uint16_t>(inIndexes[ix_min_no_no]);
            minNoNoCompBoundaryBefore = static_cast<uint16_t>(
                inIndexes[ix_min_no_no_comp_boundary_before]);
            minNoNoCompNoMaybeCC = static_cast<uint16_t>(
                inIndexes[ix_min_no_no_comp_no_maybe_cc]);
            minNoNoEmpty =
                static_cast<uint16_t>(inIndexes[ix_min_no_no_empty]);
            limitNoNo = static_cast<uint16_t>(inIndexes[ix_limit_no_no]);
            minMaybeYes =
                static_cast<uint16_t>(inIndexes[ix_min_maybe_yes]);
            BOOST_ASSERT(
                (minMaybeYes & 7) ==
                0); // 8-aligned for noNoDelta bit fields
            centerNoNoDelta = (minMaybeYes >> delta_shift) - max_delta - 1;

            normTrie = inTrie;

            maybeYesCompositions = inExtraData;
            extraData =
                maybeYesCompositions +
                ((min_normal_maybe_yes - minMaybeYes) >> offset_shift);

            smallFCD = inSmallFCD;
        }

        // The trie stores values for lead surrogate code *units*.
        // Surrogate code *points* are inert.
        uint16_t getNorm16(UChar32 c) const
        {
            return detail::icu::u_is_lead(c)
                       ? inert
                       : detail::icu::ucptrie_fast_get(
                             normTrie, ucptrie_16, c);
        }
        uint16_t getRawNorm16(UChar32 c) const
        {
            return detail::icu::ucptrie_fast_get(normTrie, ucptrie_16, c);
        }

        UNormalizationCheckResult getCompQuickCheck(uint16_t norm16) const
        {
            if (norm16 < minNoNo || min_yes_yes_with_cc <= norm16) {
                return unorm_yes;
            } else if (minMaybeYes <= norm16) {
                return unorm_maybe;
            } else {
                return unorm_no;
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
            if (norm16 >= min_normal_maybe_yes) {
                return getCCFromNormalYesOrMaybe(norm16);
            }
            if (norm16 < minNoNo || limitNoNo <= norm16) {
                return 0;
            }
            return getCCFromNoNo(norm16);
        }
        static uint8_t getCCFromNormalYesOrMaybe(uint16_t norm16)
        {
            return (uint8_t)(norm16 >> offset_shift);
        }
        static uint8_t getCCFromYesOrMaybe(uint16_t norm16)
        {
            return norm16 >= min_normal_maybe_yes
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

        /** Returns true if the single-or-lead code unit c might have
         * non-zero FCD data. */
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
                if (norm16 >= min_normal_maybe_yes) {
                    // combining mark
                    norm16 = getCCFromNormalYesOrMaybe(norm16);
                    return norm16 | (norm16 << 8);
                } else if (norm16 >= minMaybeYes) {
                    return 0;
                } else { // isDecompNoAlgorithmic(norm16)
                    uint16_t deltaTrailCC = norm16 & delta_tccc_mask;
                    if (deltaTrailCC <= delta_tccc_1) {
                        return deltaTrailCC >> offset_shift;
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
            // c decomposes, get everything from the variable-length extra
            // data
            const uint16_t * mapping = getMapping(norm16);
            uint16_t firstUnit = *mapping;
            norm16 = firstUnit >> 8; // tccc
            if (firstUnit & mapping_has_ccc_lccc_word) {
                norm16 |= *(mapping - 1) & 0xff00; // lccc
            }
            return norm16;
        }

        enum {
            // Fixed norm16 values.
            min_yes_yes_with_cc = 0xfe02,
            jamo_vt = 0xfe00,
            min_normal_maybe_yes = 0xfc00,
            jamo_l = 2, // offset=1 hasCompBoundaryAfter=false
            inert = 1,  // offset=0 hasCompBoundaryAfter=true

            // norm16 bit 0 is comp-boundary-after.
            has_comp_boundary_after = 1,
            offset_shift = 1,

            // For algorithmic one-way mappings, norm16 bits 2..1 indicate
            // the
            // tccc (0, 1, >1) for quick FCC boundary-after tests.
            delta_tccc_0 = 0,
            delta_tccc_1 = 2,
            delta_tccc_gt_1 = 4,
            delta_tccc_mask = 6,
            delta_shift = 3,

            max_delta = 0x40
        };

        enum {
            // Byte offsets from the start of the data, after the generic
            // header.
            ix_norm_trie_offset,
            ix_extra_data_offset,
            ix_small_fcd_offset,
            ix_reserved3_offset,
            ix_reserved4_offset,
            ix_reserved5_offset,
            ix_reserved6_offset,
            ix_total_size,

            // Code point thresholds for quick check codes.
            ix_min_decomp_no_cp,
            ix_min_comp_no_maybe_cp,

            // Norm16 value thresholds for quick check combinations and
            // types of
            // extra data.

            /** Mappings & compositions in [minYesNo..minYesNoMappingsOnly[.
             */
            ix_min_yes_no,
            /** Mappings are comp-normalized. */
            ix_min_no_no,
            ix_limit_no_no,
            ix_min_maybe_yes,

            /** Mappings only in [minYesNoMappingsOnly..minNoNo[. */
            ix_min_yes_no_mappings_only,
            /** Mappings are not comp-normalized but have a comp boundary
               before. */
            ix_min_no_no_comp_boundary_before,
            /** Mappings do not have a comp boundary before. */
            ix_min_no_no_comp_no_maybe_cc,
            /** Mappings to the empty string. */
            ix_min_no_no_empty,

            ix_min_lccc_cp,
            ix_reserved19,
            ix_count
        };

        enum {
            mapping_has_ccc_lccc_word = 0x80,
            mapping_has_raw_mapping = 0x40,
            // unused bit 0x20,
            mapping_length_mask = 0x1f
        };

        enum {
            comp_1_last_tuple = 0x8000,
            comp_1_triple = 1,
            comp_1_trail_limit = 0x3400,
            comp_1_trail_mask = 0x7ffe,
            comp_1_trail_shift = 9, // 10-1 for the "triple" bit
            comp_2_trail_shift = 6,
            comp_2_trail_mask = 0xffc0
        };

        // higher-level functionality
        // ------------------------------------------
        // ***

        // Dual functionality:
        // buffer!=nullptr: normalize
        // buffer==nullptr: isNormalized/spanQuickCheckYes
        template<
            bool WriteToOut,
            typename Iter,
            typename Sentinel,
            typename UTF16Appender>
        Iter decompose(
            Iter src,
            Sentinel limit,
            ReorderingBuffer<UTF16Appender> & buffer) const
        {
            UChar32 const minNoCP = minDecompNoCP;

            Iter prevSrc;
            UChar32 c = 0;
            uint16_t norm16 = 0;

            // only for quick check
            Iter prevBoundary = src;
            uint8_t prevCC = 0;

            for (;;) {
                // count code units below the minimum or with irrelevant
                // data for the quick check
                for (prevSrc = src; src != limit;) {
                    if ((c = *src) < minNoCP ||
                        isMostDecompYesAndZeroCC(
                            norm16 = detail::icu::ucptrie_fast_bmp_get(
                                normTrie, ucptrie_16, c))) {
                        ++src;
                    } else if (!detail::icu::u16_is_lead(c)) {
                        break;
                    } else {
                        UChar c2;
                        auto next = std::next(src);
                        if (next != limit &&
                            detail::icu::u16_is_trail(c2 = *next)) {
                            c = detail::icu::u16_get_supplementary(c, c2);
                            norm16 = detail::icu::ucptrie_fast_supp_get(
                                normTrie, ucptrie_16, c);
                            if (isMostDecompYesAndZeroCC(norm16)) {
                                ++src;
                                ++src;
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
                    if (WriteToOut) {
                        if (!buffer.appendZeroCC(prevSrc, src)) {
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
                std::advance(src, detail::icu::u16_length(c));
                if (WriteToOut) {
                    if (!decompose(c, norm16, buffer)) {
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
        // Very similar to composeQuickCheck(): Make the same changes in
        // both places if relevant. doCompose: normalize !doCompose:
        // isNormalized (buffer must be empty and initialized)
        template<
            bool OnlyContiguous,
            bool WriteToOut,
            typename Iter,
            typename Sentinel,
            typename UTF16Appender>
        UBool compose(
            Iter src,
            Sentinel limit,
            ReorderingBuffer<UTF16Appender> & buffer) const
        {
            Iter prevBoundary = src;
            UChar32 minNoMaybeCP = minCompNoMaybeCP;

            for (;;) {
                // Fast path: Scan over a sequence of characters below the
                // minimum "no or maybe" code point, or with (compYes &&
                // ccc==0) properties.
                Iter prevSrc;
                UChar32 c = 0;
                uint16_t norm16 = 0;
                for (;;) {
                    if (src == limit) {
                        if (prevBoundary != limit && WriteToOut) {
                            buffer.appendZeroCC(prevBoundary, limit);
                        }
                        return true;
                    }
                    if ((c = *src) < minNoMaybeCP ||
                        isCompYesAndZeroCC(
                            norm16 = detail::icu::ucptrie_fast_bmp_get(
                                normTrie, ucptrie_16, c))) {
                        ++src;
                    } else {
                        prevSrc = src++;
                        if (!detail::icu::u16_is_lead(c)) {
                            break;
                        } else {
                            UChar c2;
                            if (src != limit &&
                                detail::icu::u16_is_trail(c2 = *src)) {
                                ++src;
                                c = detail::icu::u16_get_supplementary(
                                    c, c2);
                                norm16 = detail::icu::ucptrie_fast_supp_get(
                                    normTrie, ucptrie_16, c);
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
                // "yesYes" with ccc!=0. It is not a Hangul syllable or Jamo
                // L because those have "yes" properties.

                // Medium-fast path: Handle cases that do not require full
                // decomposition and recomposition.
                if (!isMaybeOrNonZeroCC(
                        norm16)) { // minNoNo <= norm16 < minMaybeYes
                    if (!WriteToOut) {
                        return false;
                    }
                    // Fast path for mapping a character that is immediately
                    // surrounded by boundaries. In this case, we need not
                    // decompose around the current character.
                    if (isDecompNoAlgorithmic(norm16)) {
                        // Maps to a single isCompYesAndZeroCC character
                        // which also implies hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, OnlyContiguous) ||
                            hasCompBoundaryBefore_utf16(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc)) {
                                break;
                            }
                            if (!buffer.append(
                                    mapAlgorithmic(c, norm16), 0)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 < minNoNoCompBoundaryBefore) {
                        // The mapping is comp-normalized which also implies
                        // hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, OnlyContiguous) ||
                            hasCompBoundaryBefore_utf16(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc)) {
                                break;
                            }
                            const UChar * mapping =
                                reinterpret_cast<const UChar *>(
                                    getMapping(norm16));
                            int32_t length =
                                *mapping++ & mapping_length_mask;
                            if (!buffer.appendZeroCC(
                                    mapping, mapping + length)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 >= minNoNoEmpty) {
                        // The current character maps to nothing.
                        // Simply omit it from the output if there is a
                        // boundary before _or_ after it. The character
                        // itself implies no boundaries.
                        if (hasCompBoundaryBefore_utf16(src, limit) ||
                            hasCompBoundaryAfter_utf16(
                                prevBoundary, prevSrc, OnlyContiguous)) {
                            if (prevBoundary != prevSrc &&
                                !buffer.appendZeroCC(
                                    prevBoundary, prevSrc)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    }
                    // Other "noNo" type, or need to examine more text
                    // around this character: Fall through to the slow path.
                } else if (isJamoVT(norm16) && prevBoundary != prevSrc) {
                    UChar prev = *std::prev(prevSrc);
                    if (c < Hangul::jamo_t_base) {
                        // The current character is a Jamo Vowel,
                        // compose with previous Jamo L and following Jamo
                        // T.
                        UChar l = (UChar)(prev - Hangul::jamo_l_base);
                        if (l < Hangul::jamo_l_count) {
                            if (!WriteToOut) {
                                return false;
                            }
                            int32_t t;
                            if (src != limit &&
                                0 < (t =
                                         ((int32_t)*src -
                                          Hangul::jamo_t_base)) &&
                                t < Hangul::jamo_t_count) {
                                // The next character is a Jamo T.
                                ++src;
                            } else if (hasCompBoundaryBefore_utf16(
                                           src, limit)) {
                                // No Jamo T follows, not even via
                                // decomposition.
                                t = 0;
                            } else {
                                t = -1;
                            }
                            if (t >= 0) {
                                UChar32 syllable =
                                    Hangul::hangul_base +
                                    (l * Hangul::jamo_v_count +
                                     (c - Hangul::jamo_v_base)) *
                                        Hangul::jamo_t_count +
                                    t;
                                --prevSrc; // Replace the Jamo L as well.
                                if (prevBoundary != prevSrc &&
                                    !buffer.appendZeroCC(
                                        prevBoundary, prevSrc)) {
                                    break;
                                }
                                if (!buffer.appendBMP((UChar)syllable, 0)) {
                                    break;
                                }
                                prevBoundary = src;
                                continue;
                            }
                            // If we see L+V+x where x!=T then we drop to
                            // the slow path, decompose and recompose. This
                            // is to deal with NFKC finding normal L and V
                            // but a compatibility variant of a T. We need
                            // to either fully compose that combination here
                            // (which would complicate the code and may not
                            // work with strange custom data) or use the
                            // slow path.
                        }
                    } else if (Hangul::isHangulLV(prev)) {
                        // The current character is a Jamo Trailing
                        // consonant, compose with previous Hangul LV that
                        // does not contain a Jamo T.
                        if (!WriteToOut) {
                            return false;
                        }
                        UChar32 syllable = prev + c - Hangul::jamo_t_base;
                        --prevSrc; // Replace the Hangul LV as well.
                        if (prevBoundary != prevSrc &&
                            !buffer.appendZeroCC(prevBoundary, prevSrc)) {
                            break;
                        }
                        if (!buffer.appendBMP((UChar)syllable, 0)) {
                            break;
                        }
                        prevBoundary = src;
                        continue;
                    }
                    // No matching context, or may need to decompose
                    // surrounding text first: Fall through to the slow
                    // path.
                } else if (norm16 > jamo_vt) { // norm16 >=
                                               // MIN_YES_YES_WITH_CC
                    // One or more combining marks that do not combine-back:
                    // Check for canonical order, copy unchanged if ok and
                    // if followed by a character with a boundary-before.
                    uint8_t cc = getCCFromNormalYesOrMaybe(norm16); // cc!=0
                    if (OnlyContiguous /* FCC */ &&
                        getPreviousTrailCC_utf16(prevBoundary, prevSrc) >
                            cc) {
                        // Fails FCD test, need to decompose and
                        // contiguously recompose.
                        if (!WriteToOut) {
                            return false;
                        }
                    } else {
                        // If !OnlyContiguous (not FCC), then we ignore the
                        // tccc of the previous character which passed the
                        // quick check "yes && ccc==0" test.
                        Iter nextSrc;
                        uint16_t n16;
                        for (;;) {
                            if (src == limit) {
                                if (WriteToOut) {
                                    buffer.appendZeroCC(
                                        prevBoundary, limit);
                                }
                                return true;
                            }
                            uint8_t prevCC = cc;
                            nextSrc = src;
                            detail::icu::ucptrie_fast_u16_next(
                                normTrie,
                                ucptrie_16,
                                nextSrc,
                                limit,
                                c,
                                n16);
                            if (n16 >= min_yes_yes_with_cc) {
                                cc = getCCFromNormalYesOrMaybe(n16);
                                if (prevCC > cc) {
                                    if (!WriteToOut) {
                                        return false;
                                    }
                                    break;
                                }
                            } else {
                                break;
                            }
                            src = nextSrc;
                        }
                        // src is after the last in-order combining mark.
                        // If there is a boundary here, then we continue
                        // with no change.
                        if (norm16HasCompBoundaryBefore(n16)) {
                            if (isCompYesAndZeroCC(n16)) {
                                src = nextSrc;
                            }
                            continue;
                        }
                        // Use the slow path. There is no boundary in
                        // [prevSrc, src[.
                    }
                }

                // Slow path: Find the nearest boundaries around the current
                // character, decompose and recompose.
                if (prevBoundary != prevSrc &&
                    !norm16HasCompBoundaryBefore(norm16)) {
                    Iter p = prevSrc;
                    detail::icu::ucptrie_fast_u16_prev(
                        normTrie, ucptrie_16, prevBoundary, p, c, norm16);
                    if (!norm16HasCompBoundaryAfter(
                            norm16, OnlyContiguous)) {
                        prevSrc = p;
                    }
                }
                if (WriteToOut && prevBoundary != prevSrc &&
                    !buffer.appendZeroCC(prevBoundary, prevSrc)) {
                    break;
                }
                auto const no_flush = buffer.inhibit_flush();
                int32_t recomposeStartIndex = buffer.size();
                // We know there is not a boundary here.
                decomposeShort_utf16(
                    prevSrc,
                    src,
                    false /* !stopAtCompBoundary */,
                    OnlyContiguous,
                    buffer);
                // Decompose until the next boundary.
                src = decomposeShort_utf16(
                    src,
                    limit,
                    true /* stopAtCompBoundary */,
                    OnlyContiguous,
                    buffer);
                BOOST_ASSERT(std::distance(prevSrc, src) <= INT32_MAX);
                recompose(buffer, recomposeStartIndex, OnlyContiguous);
                if (!WriteToOut) {
                    if (!buffer.equals_utf16(prevSrc, src)) {
                        return false;
                    }
                    buffer.remove();
                }
                prevBoundary = src;
            }
            return true;
        }

        /** sink==nullptr: isNormalized() */
        template<
            bool OnlyContiguous,
            bool WriteToOut,
            typename CharIter,
            typename Sentinel,
            typename UTF8Appender>
        UBool composeUTF8(
            CharIter src, Sentinel limit, UTF8Appender & appender) const
        {
            container::small_vector<UChar, 1024> s16;
            uint8_t minNoMaybeLead = leadByteForCP(minCompNoMaybeCP);
            CharIter prevBoundary = src;

            for (;;) {
                // Fast path: Scan over a sequence of characters below the
                // minimum "no or maybe" code point, or with (compYes &&
                // ccc==0) properties.
                CharIter prevSrc;
                uint16_t norm16 = 0;
                for (;;) {
                    if (src == limit) {
                        if (prevBoundary != limit && WriteToOut) {
                            ByteSinkUtil::appendUnchanged(
                                prevBoundary, limit, appender);
                        }
                        return true;
                    }
                    if ((uint8_t)*src < minNoMaybeLead) {
                        ++src;
                    } else {
                        prevSrc = src;
                        detail::icu::ucptrie_fast_u8_next(
                            normTrie, ucptrie_16, src, limit, norm16);
                        if (!isCompYesAndZeroCC(norm16)) {
                            break;
                        }
                    }
                }
                // isCompYesAndZeroCC(norm16) is false, that is,
                // norm16>=minNoNo. The current character is either a "noNo"
                // (has a mapping) or a "maybeYes" (combines backward) or a
                // "yesYes" with ccc!=0. It is not a Hangul syllable or Jamo
                // L because those have "yes" properties.

                // Medium-fast path: Handle cases that do not require full
                // decomposition and recomposition.
                if (!isMaybeOrNonZeroCC(
                        norm16)) { // minNoNo <= norm16 < minMaybeYes
                    if (!WriteToOut) {
                        return false;
                    }
                    // Fast path for mapping a character that is immediately
                    // surrounded by boundaries. In this case, we need not
                    // decompose around the current character.
                    if (isDecompNoAlgorithmic(norm16)) {
                        // Maps to a single isCompYesAndZeroCC character
                        // which also implies hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, OnlyContiguous) ||
                            hasCompBoundaryBefore_utf8(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, appender)) {
                                break;
                            }
                            appendCodePointDelta(
                                prevSrc,
                                src,
                                getAlgorithmicDelta(norm16),
                                appender);
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 < minNoNoCompBoundaryBefore) {
                        // The mapping is comp-normalized which also implies
                        // hasCompBoundaryBefore.
                        if (norm16HasCompBoundaryAfter(
                                norm16, OnlyContiguous) ||
                            hasCompBoundaryBefore_utf8(src, limit)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, appender)) {
                                break;
                            }
                            const uint16_t * mapping = getMapping(norm16);
                            int32_t length =
                                *mapping++ & mapping_length_mask;
                            if (!ByteSinkUtil::appendChange(
                                    prevSrc,
                                    src,
                                    (const UChar *)mapping,
                                    length,
                                    appender)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    } else if (norm16 >= minNoNoEmpty) {
                        // The current character maps to nothing.
                        // Simply omit it from the output if there is a
                        // boundary before _or_ after it. The character
                        // itself implies no boundaries.
                        if (hasCompBoundaryBefore_utf8(src, limit) ||
                            hasCompBoundaryAfter_utf8(
                                prevBoundary, prevSrc, OnlyContiguous)) {
                            if (prevBoundary != prevSrc &&
                                !ByteSinkUtil::appendUnchanged(
                                    prevBoundary, prevSrc, appender)) {
                                break;
                            }
                            prevBoundary = src;
                            continue;
                        }
                    }
                    // Other "noNo" type, or need to examine more text
                    // around this character: Fall through to the slow path.
                } else if (isJamoVT(norm16)) {
                    // Jamo L: E1 84 80..92
                    // Jamo V: E1 85 A1..B5
                    // Jamo T: E1 86 A8..E1 87 82
                    BOOST_ASSERT(
                        (src - prevSrc) == 3 && (uint8_t)*prevSrc == 0xe1);
                    UChar32 prev =
                        previousHangulOrJamo(prevBoundary, prevSrc);
                    if ((uint8_t)prevSrc[1] == 0x85) {
                        // The current character is a Jamo Vowel,
                        // compose with previous Jamo L and following Jamo
                        // T.
                        UChar32 l = prev - Hangul::jamo_l_base;
                        if ((uint32_t)l < Hangul::jamo_l_count) {
                            if (!WriteToOut) {
                                return false;
                            }
                            int32_t t = getJamoTMinusBase(src, limit);
                            if (t >= 0) {
                                // The next character is a Jamo T.
                                src += 3;
                            } else if (hasCompBoundaryBefore_utf8(
                                           src, limit)) {
                                // No Jamo T follows, not even via
                                // decomposition.
                                t = 0;
                            }
                            if (t >= 0) {
                                UChar32 syllable =
                                    Hangul::hangul_base +
                                    (l * Hangul::jamo_v_count +
                                     ((uint8_t)prevSrc[2] - 0xa1)) *
                                        Hangul::jamo_t_count +
                                    t;
                                prevSrc -= 3; // Replace the Jamo L as well.
                                if (prevBoundary != prevSrc &&
                                    !ByteSinkUtil::appendUnchanged(
                                        prevBoundary, prevSrc, appender)) {
                                    break;
                                }
                                ByteSinkUtil::appendCodePoint(
                                    prevSrc, src, syllable, appender);
                                prevBoundary = src;
                                continue;
                            }
                            // If we see L+V+x where x!=T then we drop to
                            // the slow path, decompose and recompose. This
                            // is to deal with NFKC finding normal L and V
                            // but a compatibility variant of a T. We need
                            // to either fully compose that combination here
                            // (which would complicate the code and may not
                            // work with strange custom data) or use the
                            // slow path.
                        }
                    } else if (Hangul::isHangulLV(prev)) {
                        // The current character is a Jamo Trailing
                        // consonant, compose with previous Hangul LV that
                        // does not contain a Jamo T.
                        if (!WriteToOut) {
                            return false;
                        }
                        UChar32 syllable =
                            prev + getJamoTMinusBase(prevSrc, src);
                        prevSrc -= 3; // Replace the Hangul LV as well.
                        if (prevBoundary != prevSrc &&
                            !ByteSinkUtil::appendUnchanged(
                                prevBoundary, prevSrc, appender)) {
                            break;
                        }
                        ByteSinkUtil::appendCodePoint(
                            prevSrc, src, syllable, appender);
                        prevBoundary = src;
                        continue;
                    }
                    // No matching context, or may need to decompose
                    // surrounding text first: Fall through to the slow
                    // path.
                } else if (norm16 > jamo_vt) { // norm16 >=
                                               // MIN_YES_YES_WITH_CC
                    // One or more combining marks that do not combine-back:
                    // Check for canonical order, copy unchanged if ok and
                    // if followed by a character with a boundary-before.
                    uint8_t cc = getCCFromNormalYesOrMaybe(norm16); // cc!=0
                    if (OnlyContiguous /* FCC */ &&
                        getPreviousTrailCC_utf8(prevBoundary, prevSrc) >
                            cc) {
                        // Fails FCD test, need to decompose and
                        // contiguously recompose.
                        if (!WriteToOut) {
                            return false;
                        }
                    } else {
                        // If !OnlyContiguous (not FCC), then we ignore the
                        // tccc of the previous character which passed the
                        // quick check "yes && ccc==0" test.
                        CharIter nextSrc;
                        uint16_t n16;
                        for (;;) {
                            if (src == limit) {
                                if (WriteToOut) {
                                    ByteSinkUtil::appendUnchanged(
                                        prevBoundary, limit, appender);
                                }
                                return true;
                            }
                            uint8_t prevCC = cc;
                            nextSrc = src;
                            detail::icu::ucptrie_fast_u8_next(
                                normTrie, ucptrie_16, nextSrc, limit, n16);
                            if (n16 >= min_yes_yes_with_cc) {
                                cc = getCCFromNormalYesOrMaybe(n16);
                                if (prevCC > cc) {
                                    if (!WriteToOut) {
                                        return false;
                                    }
                                    break;
                                }
                            } else {
                                break;
                            }
                            src = nextSrc;
                        }
                        // src is after the last in-order combining mark.
                        // If there is a boundary here, then we continue
                        // with no change.
                        if (norm16HasCompBoundaryBefore(n16)) {
                            if (isCompYesAndZeroCC(n16)) {
                                src = nextSrc;
                            }
                            continue;
                        }
                        // Use the slow path. There is no boundary in
                        // [prevSrc, src[.
                    }
                }

                // Slow path: Find the nearest boundaries around the current
                // character, decompose and recompose.
                if (prevBoundary != prevSrc &&
                    !norm16HasCompBoundaryBefore(norm16)) {
                    CharIter p = prevSrc;
                    detail::icu::ucptrie_fast_u8_prev(
                        normTrie, ucptrie_16, prevBoundary, p, norm16);
                    if (!norm16HasCompBoundaryAfter(
                            norm16, OnlyContiguous)) {
                        prevSrc = p;
                    }
                }
                bool equals_utf8 = true;
                {
                    s16.clear();
                    utf16_appender<container::small_vector<UChar, 1024>>
                        buffer_appender(s16);
                    ReorderingBuffer<utf16_appender<
                        container::small_vector<UChar, 1024>>>
                        buffer(*this, buffer_appender);
                    auto const no_flush = buffer.inhibit_flush();
                    // We know there is not a boundary here.
                    decomposeShort_utf8(
                        prevSrc,
                        src,
                        false /* !stopAtCompBoundary */,
                        OnlyContiguous,
                        buffer);
                    // Decompose until the next boundary.
                    src = decomposeShort_utf8(
                        src,
                        limit,
                        true /* stopAtCompBoundary */,
                        OnlyContiguous,
                        buffer);
                    BOOST_ASSERT(src - prevSrc <= INT32_MAX);
                    recompose(buffer, 0, OnlyContiguous);
                    equals_utf8 = buffer.equals_utf8(prevSrc, src);
                }
                if (!equals_utf8) {
                    if (!WriteToOut) {
                        return false;
                    }
                    if (prevBoundary != prevSrc &&
                        !ByteSinkUtil::appendUnchanged(
                            prevBoundary, prevSrc, appender)) {
                        break;
                    }
                    if (!ByteSinkUtil::appendChange(
                            prevSrc,
                            src,
                            s16.data(),
                            s16.size(),
                            appender)) {
                        break;
                    }
                    prevBoundary = src;
                }
            }
            return true;
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
                   (norm16 & has_comp_boundary_after) != 0 &&
                   (!onlyContiguous || isInert(norm16) ||
                    *getMapping(norm16) <= 0x1ff);
        }

    private:
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
         * Trie UTF-8 macros do not assemble whole code points (for
         * efficiency). When we do need the code point, we call this
         * function. We should not need it for normalization-inert data
         * (norm16==0). Illegal sequences yield the error value norm16==0
         * just like real normalization-inert code points.
         */
        template<typename CharIter>
        static UChar32
        codePointFromValidUTF8(CharIter cpStart, CharIter cpLimit)
        {
            // Similar to U8_NEXT_UNSAFE(s, i, c).
            BOOST_ASSERT(cpStart < cpLimit);
            uint8_t c = *cpStart;
            switch (cpLimit - cpStart) {
            case 1: return c;
            case 2: return ((c & 0x1f) << 6) | ((uint8_t)cpStart[1] & 0x3f);
            case 3:
                // no need for (c&0xf) because the upper bits are truncated
                // after <<12 in the cast to (UChar)
                return (UChar)(
                    (c << 12) | (((uint8_t)cpStart[1] & 0x3f) << 6) |
                    ((uint8_t)cpStart[2] & 0x3f));
            case 4:
                return ((c & 7) << 18) |
                       (((uint8_t)cpStart[1] & 0x3f) << 12) |
                       (((uint8_t)cpStart[2] & 0x3f) << 6) |
                       ((uint8_t)cpStart[3] & 0x3f);
            default:
                BOOST_ASSERT(false); // Should not occur.
                return u_sentinel;
            }
        }

        /**
         * Returns the last code point in [start, p[ if it is valid and in
         * U+1000..U+D7FF. Otherwise returns a negative value.
         */
        template<typename CharIter>
        static UChar32 previousHangulOrJamo(CharIter start, CharIter p)
        {
            if ((p - start) >= 3) {
                p -= 3;
                uint8_t l = *p;
                uint8_t t1, t2;
                if (0xe1 <= l && l <= 0xed &&
                    (t1 = (uint8_t)((uint8_t)p[1] - 0x80)) <= 0x3f &&
                    (t2 = (uint8_t)((uint8_t)p[2] - 0x80)) <= 0x3f &&
                    (l < 0xed || t1 <= 0x1f)) {
                    return ((l & 0xf) << 12) | (t1 << 6) | t2;
                }
            }
            return u_sentinel;
        }

        /**
         * Returns the offset from the Jamo T base if [src, limit[ starts
         * with a single Jamo T code point. Otherwise returns a negative
         * value.
         */
        template<typename CharIter, typename Sentinel>
        static int32_t getJamoTMinusBase(CharIter src, Sentinel limit)
        {
            // Jamo T: E1 86 A8..E1 87 82
            if (detail::icu::dist(src, limit) >= 3 &&
                (uint8_t)*src == 0xe1) {
                if ((uint8_t)src[1] == 0x86) {
                    uint8_t t = src[2];
                    // The first Jamo T is U+11A8 but JAMO_T_BASE is 11A7.
                    // Offset 0 does not correspond to any conjoining Jamo.
                    if (0xa8 <= t && t <= 0xbf) {
                        return t - 0xa7;
                    }
                } else if ((uint8_t)src[1] == 0x87) {
                    uint8_t t = src[2];
                    if ((int8_t)t <= (int8_t)0x82u) {
                        return t - (0xa7 - 0x40);
                    }
                }
            }
            return -1;
        }

        template<typename CharIter, typename UTF8Appender>
        static void appendCodePointDelta(
            CharIter cpStart,
            CharIter cpLimit,
            int32_t delta,
            UTF8Appender & appender)
        {
            char buffer[u8_max_length];
            int32_t length;
            int32_t cpLength = (int32_t)(cpStart - cpLimit);
            if (cpLength == 1) {
                // The builder makes ASCII map to ASCII.
                buffer[0] = (uint8_t)((uint8_t)*cpStart + delta);
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
                    u8_append_unsafe(buffer, length, c);
                }
            }
            appender.append(buffer, buffer + length);
        }

        UBool isMaybe(uint16_t norm16) const
        {
            return minMaybeYes <= norm16 && norm16 <= jamo_vt;
        }
        UBool isMaybeOrNonZeroCC(uint16_t norm16) const
        {
            return norm16 >= minMaybeYes;
        }
        static UBool isInert(uint16_t norm16) { return norm16 == inert; }
        static UBool isJamoL(uint16_t norm16) { return norm16 == jamo_l; }
        static UBool isJamoVT(uint16_t norm16) { return norm16 == jamo_vt; }
        uint16_t hangulLVT() const
        {
            return minYesNoMappingsOnly | has_comp_boundary_after;
        }
        UBool isHangulLV(uint16_t norm16) const
        {
            return norm16 == minYesNo;
        }
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
            return norm16 < minYesNo || norm16 == jamo_vt ||
                   (minMaybeYes <= norm16 &&
                    norm16 <= min_normal_maybe_yes);
        }
        /**
         * A little faster and simpler than isDecompYesAndZeroCC() but does
         * not include the MaybeYes which combine-forward and have ccc=0.
         * (Standard Unicode 10 normalization does not have such
         * characters.)
         */
        UBool isMostDecompYesAndZeroCC(uint16_t norm16) const
        {
            return norm16 < minYesNo || norm16 == min_normal_maybe_yes ||
                   norm16 == jamo_vt;
        }
        UBool isDecompNoAlgorithmic(uint16_t norm16) const
        {
            return norm16 >= limitNoNo;
        }

        // For use with isCompYes().
        // Perhaps the compiler can combine the two tests for
        // MIN_YES_YES_WITH_CC. static uint8_t getCCFromYes(uint16_t norm16)
        // {
        //     return norm16>=MIN_YES_YES_WITH_CC ?
        //     getCCFromNormalYesOrMaybe(norm16) : 0;
        // }
        uint8_t getCCFromNoNo(uint16_t norm16) const
        {
            const uint16_t * mapping = getMapping(norm16);
            if (*mapping & mapping_has_ccc_lccc_word) {
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
                // For Hangul LVT we harmlessly fetch a firstUnit with
                // tccc=0 here.
                return (uint8_t)(
                    *getMapping(norm16) >> 8); // tccc from yesNo
            }
        }
        template<typename Iter>
        uint8_t getPreviousTrailCC_utf16(Iter start, Iter p) const
        {
            if (start == p) {
                return 0;
            }
            int32_t i = (int32_t)std::distance(start, p);
            UChar32 c;
            detail::icu::u16_prev(start, 0, i, c);
            return (uint8_t)getFCD16(c);
        }
        template<typename CharIter>
        uint8_t getPreviousTrailCC_utf8(CharIter start, CharIter p) const
        {
            if (start == p) {
                return 0;
            }
            int32_t i = (int32_t)(p - start);
            UChar32 c;
            detail::icu::u8_prev(start, 0, i, c);
            return (uint8_t)getFCD16(c);
        }

        // Requires algorithmic-NoNo.
        UChar32 mapAlgorithmic(UChar32 c, uint16_t norm16) const
        {
            return c + (norm16 >> delta_shift) - centerNoNoDelta;
        }
        UChar32 getAlgorithmicDelta(uint16_t norm16) const
        {
            return (norm16 >> delta_shift) - centerNoNoDelta;
        }

        // Requires minYesNo<norm16<limitNoNo.
        const uint16_t * getMapping(uint16_t norm16) const
        {
            return extraData + (norm16 >> offset_shift);
        }
        const uint16_t *
        getCompositionsListForDecompYes(uint16_t norm16) const
        {
            if (norm16 < jamo_l || min_normal_maybe_yes <= norm16) {
                return nullptr;
            } else if (norm16 < minMaybeYes) {
                return getMapping(
                    norm16); // for yesYes; if Jamo L: harmless empty list
            } else {
                return maybeYesCompositions + norm16 - minMaybeYes;
            }
        }
        const uint16_t *
        getCompositionsListForComposite(uint16_t norm16) const
        {
            // A composite has both mapping & compositions list.
            const uint16_t * list = getMapping(norm16);
            return list + // mapping pointer
                   1 + // +1 to skip the first unit with the mapping length
                   (*list & mapping_length_mask); // + mapping length
        }
        const uint16_t * getCompositionsListForMaybe(uint16_t norm16) const
        {
            // minMaybeYes<=norm16<MIN_NORMAL_MAYBE_YES
            return maybeYesCompositions +
                   ((norm16 - minMaybeYes) >> offset_shift);
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

        // Decompose a short piece of text which is likely to contain
        // characters that fail the quick check loop and/or where the quick
        // check loop's overhead is unlikely to be amortized. Called by the
        // compose() and makeFCD() implementations.
        template<typename Iter, typename Sentinel, typename UTF16Appender>
        Iter decomposeShort_utf16(
            Iter src,
            Sentinel limit,
            UBool stopAtCompBoundary,
            UBool onlyContiguous,
            ReorderingBuffer<UTF16Appender> & buffer) const
        {
            while (src != limit) {
                if (stopAtCompBoundary && *src < minCompNoMaybeCP) {
                    return src;
                }
                Iter prevSrc = src;
                UChar32 c;
                uint16_t norm16;
                detail::icu::ucptrie_fast_u16_next(
                    normTrie, ucptrie_16, src, limit, c, norm16);
                if (stopAtCompBoundary &&
                    norm16HasCompBoundaryBefore(norm16)) {
                    return prevSrc;
                }
                if (!decompose(c, norm16, buffer)) {
                    return src;
                }
                if (stopAtCompBoundary &&
                    norm16HasCompBoundaryAfter(norm16, onlyContiguous)) {
                    return src;
                }
            }
            return src;
        }
        template<typename UTF16Appender>
        UBool decompose(
            UChar32 c,
            uint16_t norm16,
            ReorderingBuffer<UTF16Appender> & buffer) const
        {
            // get the decomposition and the lead and trail cc's
            if (norm16 >= limitNoNo) {
                if (isMaybeOrNonZeroCC(norm16)) {
                    return buffer.append(c, getCCFromYesOrMaybe(norm16));
                }
                // Maps to an isCompYesAndZeroCC.
                c = mapAlgorithmic(c, norm16);
                norm16 = getRawNorm16(c);
            }
            if (norm16 < minYesNo) {
                // c does not decompose
                return buffer.append(c, 0);
            } else if (isHangulLV(norm16) || isHangulLVT(norm16)) {
                // Hangul syllable: decompose algorithmically
                UChar jamos[3];
                return buffer.appendZeroCC(
                    jamos, jamos + Hangul::decompose(c, jamos));
            }
            // c decomposes, get everything from the variable-length extra
            // data
            const uint16_t * mapping = getMapping(norm16);
            uint16_t firstUnit = *mapping;
            int32_t length = firstUnit & mapping_length_mask;
            uint8_t leadCC, trailCC;
            trailCC = (uint8_t)(firstUnit >> 8);
            if (firstUnit & mapping_has_ccc_lccc_word) {
                leadCC = (uint8_t)(*(mapping - 1) >> 8);
            } else {
                leadCC = 0;
            }
            return buffer.append(
                (const UChar *)mapping + 1, length, true, leadCC, trailCC);
        }

        template<
            typename CharIter,
            typename Sentinel,
            typename UTF16Appender>
        CharIter decomposeShort_utf8(
            CharIter src,
            Sentinel limit,
            UBool stopAtCompBoundary,
            UBool onlyContiguous,
            ReorderingBuffer<UTF16Appender> & buffer) const
        {
            while (src != limit) {
                CharIter prevSrc = src;
                uint16_t norm16;
                detail::icu::ucptrie_fast_u8_next(
                    normTrie, ucptrie_16, src, limit, norm16);
                // Get the decomposition and the lead and trail cc's.
                UChar32 c = u_sentinel;
                if (norm16 >= limitNoNo) {
                    if (isMaybeOrNonZeroCC(norm16)) {
                        // No boundaries around this character.
                        c = codePointFromValidUTF8(prevSrc, src);
                        if (!buffer.append(
                                c, getCCFromYesOrMaybe(norm16))) {
                            return src;
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
                // norm16!=INERT guarantees that [prevSrc, src[ is valid
                // UTF-8. We do not see invalid UTF-8 here because its
                // norm16==INERT is normalization-inert, so it gets copied
                // unchanged in the fast path, and we stop the slow path
                // where invalid UTF-8 begins.
                BOOST_ASSERT(norm16 != inert);
                if (norm16 < minYesNo) {
                    if (c < 0) {
                        c = codePointFromValidUTF8(prevSrc, src);
                    }
                    // does not decompose
                    if (!buffer.append(c, 0)) {
                        return src;
                    }
                } else if (isHangulLV(norm16) || isHangulLVT(norm16)) {
                    // Hangul syllable: decompose algorithmically
                    if (c < 0) {
                        c = codePointFromValidUTF8(prevSrc, src);
                    }
                    char16_t jamos[3];
                    if (!buffer.appendZeroCC(
                            jamos, jamos + Hangul::decompose(c, jamos))) {
                        return src;
                    }
                } else {
                    // The character decomposes, get everything from the
                    // variable-length extra data.
                    const uint16_t * mapping = getMapping(norm16);
                    uint16_t firstUnit = *mapping;
                    int32_t length = firstUnit & mapping_length_mask;
                    uint8_t trailCC = (uint8_t)(firstUnit >> 8);
                    uint8_t leadCC;
                    if (firstUnit & mapping_has_ccc_lccc_word) {
                        leadCC = (uint8_t)(*(mapping - 1) >> 8);
                    } else {
                        leadCC = 0;
                    }
                    if (!buffer.append(
                            (const char16_t *)mapping + 1,
                            length,
                            true,
                            leadCC,
                            trailCC)) {
                        return src;
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
         * If the lead and trail characters combine, then this function
         * returns the following "compositeAndFwd" value: Bits 21..1
         * composite character Bit      0  set if the composite is a
         * forward-combining starter otherwise it returns -1.
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
            if (trail < comp_1_trail_limit) {
                // trail character is 0..33FF
                // result entry may have 2 or 3 units
                key1 = (uint16_t)(trail << 1);
                while (key1 > (firstUnit = *list)) {
                    list += 2 + (firstUnit & comp_1_triple);
                }
                if (key1 == (firstUnit & comp_1_trail_mask)) {
                    if (firstUnit & comp_1_triple) {
                        return ((int32_t)list[1] << 16) | list[2];
                    } else {
                        return list[1];
                    }
                }
            } else {
                // trail character is 3400..10FFFF
                // result entry has 3 units
                key1 = (uint16_t)(
                    comp_1_trail_limit +
                    (((trail >> comp_1_trail_shift)) & ~comp_1_triple));
                uint16_t key2 = (uint16_t)(trail << comp_2_trail_shift);
                uint16_t secondUnit;
                for (;;) {
                    if (key1 > (firstUnit = *list)) {
                        list += 2 + (firstUnit & comp_1_triple);
                    } else if (key1 == (firstUnit & comp_1_trail_mask)) {
                        if (key2 > (secondUnit = list[1])) {
                            if (firstUnit & comp_1_last_tuple) {
                                break;
                            } else {
                                list += 3;
                            }
                        } else if (
                            key2 == (secondUnit & comp_2_trail_mask)) {
                            return ((int32_t)(
                                        secondUnit & ~comp_2_trail_mask)
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
        template<typename UTF16Appender>
        void recompose(
            ReorderingBuffer<UTF16Appender> & buffer,
            int32_t recomposeStartIndex,
            UBool onlyContiguous) const
        {
            UChar * p = buffer.begin() + recomposeStartIndex;
            UChar * limit = buffer.end();
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
            // forward-combining starter and are only initialized now to
            // avoid compiler warnings.
            compositionsList =
                nullptr; // used as indicator for whether we have
                         // a forward-combining starter
            starter = nullptr;
            starterIsSupplementary = false;
            prevCC = 0;

            for (;;) {
                detail::icu::ucptrie_fast_u16_next(
                    normTrie, ucptrie_16, p, limit, c, norm16);
                cc = getCCFromYesOrMaybe(norm16);
                if ( // this character combines backward and
                    isMaybe(norm16) &&
                    // we have seen a starter that combines forward and
                    compositionsList != nullptr &&
                    // the backward-combining character is not blocked
                    (prevCC < cc || prevCC == 0)) {
                    if (isJamoVT(norm16)) {
                        // c is a Jamo V/T, see if we can compose it with
                        // the previous character.
                        if (c < Hangul::jamo_t_base) {
                            // c is a Jamo Vowel, compose with previous Jamo
                            // L and following Jamo T.
                            UChar prev =
                                (UChar)(*starter - Hangul::jamo_l_base);
                            if (prev < Hangul::jamo_l_count) {
                                pRemove = p - 1;
                                UChar syllable = (UChar)(
                                    Hangul::hangul_base +
                                    (prev * Hangul::jamo_v_count +
                                     (c - Hangul::jamo_v_base)) *
                                        Hangul::jamo_t_count);
                                UChar t;
                                if (p != limit &&
                                    (t = (UChar)(
                                         *p - Hangul::jamo_t_base)) <
                                        Hangul::jamo_t_count) {
                                    ++p;
                                    syllable += t; // The next character was
                                                   // a Jamo T.
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
                         * syllables that a Jamo T could combine with. All
                         * Jamo Ts are combined above when handling Jamo Vs.
                         */
                        if (p == limit) {
                            break;
                        }
                        compositionsList = nullptr;
                        continue;
                    } else if (
                        (compositeAndFwd = combine(compositionsList, c)) >=
                        0) {
                        // The starter and the combining mark (c) do
                        // combine.
                        UChar32 composite = compositeAndFwd >> 1;

                        // Replace the starter with the composite, remove
                        // the combining mark.
                        pRemove = p - detail::icu::u16_length(
                                          c); // pRemove & p: start &
                                              // limit of the combining
                                              // mark
                        if (starterIsSupplementary) {
                            if (detail::icu::u_is_supplementary(
                                    composite)) {
                                // both are supplementary
                                starter[0] =
                                    detail::icu::u16_lead(composite);
                                starter[1] =
                                    detail::icu::u16_trail(composite);
                            } else {
                                *starter = (UChar)composite;
                                // The composite is shorter than the
                                // starter, move the intermediate characters
                                // forward one.
                                starterIsSupplementary = false;
                                q = starter + 1;
                                r = q + 1;
                                while (r < pRemove) {
                                    *q++ = *r++;
                                }
                                --pRemove;
                            }
                        } else if (detail::icu::u_is_supplementary(
                                       composite)) {
                            // The composite is longer than the starter,
                            // move the intermediate characters back one.
                            starterIsSupplementary = true;
                            ++starter; // temporarily increment for the loop
                                       // boundary
                            q = pRemove;
                            r = ++pRemove;
                            while (starter < q) {
                                *--r = *--q;
                            }
                            *starter = detail::icu::u16_trail(composite);
                            *--starter = detail::icu::u16_lead(
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
                        // Keep prevCC because we removed the combining
                        // mark.

                        if (p == limit) {
                            break;
                        }
                        // Is the composite a starter that combines forward?
                        if (compositeAndFwd & 1) {
                            compositionsList =
                                getCompositionsListForComposite(
                                    getRawNorm16(composite));
                        } else {
                            compositionsList = nullptr;
                        }

                        // We combined; continue with looking for
                        // compositions.
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
                        if (detail::icu::u_is_bmp(c)) {
                            starterIsSupplementary = false;
                            starter = p - 1;
                        } else {
                            starterIsSupplementary = true;
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
            return c < minCompNoMaybeCP ||
                   norm16HasCompBoundaryBefore(norm16);
        }
        UBool norm16HasCompBoundaryBefore(uint16_t norm16) const
        {
            return norm16 < minNoNoCompNoMaybeCC ||
                   isAlgorithmicNoNo(norm16);
        }
        template<typename Iter, typename Sentinel>
        UBool hasCompBoundaryBefore_utf16(Iter src, Sentinel limit) const
        {
            if (src == limit || *src < minCompNoMaybeCP) {
                return true;
            }
            UChar32 c;
            uint16_t norm16;
            detail::icu::ucptrie_fast_u16_next(
                normTrie, ucptrie_16, src, limit, c, norm16);
            return norm16HasCompBoundaryBefore(norm16);
        }
        template<typename CharIter, typename Sentinel>
        UBool hasCompBoundaryBefore_utf8(CharIter src, Sentinel limit) const
        {
            if (src == limit) {
                return true;
            }
            uint16_t norm16;
            detail::icu::ucptrie_fast_u8_next(
                normTrie, ucptrie_16, src, limit, norm16);
            return norm16HasCompBoundaryBefore(norm16);
        }
        template<typename Iter, typename Sentinel>
        UBool hasCompBoundaryAfter_utf16(
            Iter start, Sentinel p, UBool onlyContiguous) const
        {
            if (start == p) {
                return true;
            }
            UChar32 c = 0;
            uint16_t norm16;
            detail::icu::ucptrie_fast_u16_prev(
                normTrie, ucptrie_16, start, p, c, norm16);
            return norm16HasCompBoundaryAfter(norm16, onlyContiguous);
        }
        template<typename CharIter>
        UBool hasCompBoundaryAfter_utf8(
            CharIter start, CharIter p, UBool onlyContiguous) const
        {
            if (start == p) {
                return true;
            }
            uint16_t norm16;
            detail::icu::ucptrie_fast_u8_prev(
                normTrie, ucptrie_16, start, p, norm16);
            return norm16HasCompBoundaryAfter(norm16, onlyContiguous);
        }
        UBool norm16HasCompBoundaryAfter(
            uint16_t norm16, UBool onlyContiguous) const
        {
            return (norm16 & has_comp_boundary_after) != 0 &&
                   (!onlyContiguous ||
                    isTrailCC01ForCompBoundaryAfter(norm16));
        }
        /** For FCC: Given norm16 HAS_COMP_BOUNDARY_AFTER, does it have
         * tccc<=1?
         */
        UBool isTrailCC01ForCompBoundaryAfter(uint16_t norm16) const
        {
            return isInert(norm16) ||
                   (isDecompNoAlgorithmic(norm16)
                        ? (norm16 & delta_tccc_mask) <= delta_tccc_1
                        : *getMapping(norm16) <= 0x1ff);
        }

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
        const uint16_t *
            extraData; // mappings and/or compositions for yesYes,
                       // yesNo & noNo characters
        const uint8_t *
            smallFCD; // [0x100] one bit per 32 BMP code points, set
                      // if any FCD!=0
    };

    // implementations
    template<typename UTF16Appender>
    UBool ReorderingBuffer<UTF16Appender>::append(
        const UChar * s,
        int32_t length,
        UBool isNFD,
        uint8_t leadCC,
        uint8_t trailCC)
    {
        BOOST_ASSERT(0 < length);
        if (lastCC <= leadCC || leadCC == 0) {
            if (trailCC <= 1) {
                reorderStart = limit + length;
            } else if (leadCC <= 1) {
                reorderStart =
                    limit + 1; // Ok if not a code point boundary.
            }
            const UChar * sLimit = s + length;
            do {
                *limit++ = *s++;
            } while (s != sLimit);
            lastCC = trailCC;
        } else {
            int32_t i = 0;
            UChar32 c;
            detail::icu::u16_next(s, i, length, c);
            insert(c, leadCC); // insert first code point
            while (i < length) {
                detail::icu::u16_next(s, i, length, c);
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
                append(c, leadCC);
            }
        }
        return true;
    }

    template<typename UTF16Appender>
    uint8_t ReorderingBuffer<UTF16Appender>::previousCC()
    {
        codePointLimit = codePointStart;
        if (reorderStart >= codePointStart) {
            return 0;
        }
        UChar32 c = *--codePointStart;
        UChar c2;
        if (detail::icu::u16_is_trail(c) && begin() < codePointStart &&
            detail::icu::u16_is_lead(c2 = *(codePointStart - 1))) {
            --codePointStart;
            c = detail::icu::u16_get_supplementary(c2, c);
        }
        return impl.getCCFromYesOrMaybeCP(c);
    }

}}}}}

#endif
