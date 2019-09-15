// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

// ucpmap.h
// created: 2018sep03 Markus W. Scherer

#ifndef UCPMAP_H_
#define UCPMAP_H_


namespace boost { namespace text { namespace detail { namespace icu {

    /**
     * \file
     *
     * This file defines an abstract map from Unicode code points to integer
     * values.
     *
     * @see UCPMap
     * @see UCPTrie
     * @see UMutableCPTrie
     */

    /**
     * Abstract map from Unicode code points (U+0000..U+10FFFF) to integer
     * values.
     *
     * @see UCPTrie
     * @see UMutableCPTrie
     * @draft ICU 63
     */
    typedef struct UCPMap UCPMap;

    /**
     * Selectors for how ucpmap_getRange() etc. should report value ranges
     * overlapping with surrogates. Most users should use UCPMAP_RANGE_NORMAL.
     *
     * @see ucpmap_getRange
     * @see ucptrie_getRange
     * @see umutablecptrie_getRange
     * @draft ICU 63
     */
    enum UCPMapRangeOption {
        /**
         * ucpmap_getRange() enumerates all same-value ranges as stored in the
         * map. Most users should use this option.
         * @draft ICU 63
         */
        UCPMAP_RANGE_NORMAL,
        /**
         * ucpmap_getRange() enumerates all same-value ranges as stored in the
         * map, except that lead surrogates (U+D800..U+DBFF) are treated as
         * having the surrogateValue, which is passed to getRange() as a
         * separate parameter. The surrogateValue is not transformed via
         * filter(). See U_IS_LEAD(c).
         *
         * Most users should use UCPMAP_RANGE_NORMAL instead.
         *
         * This option is useful for maps that map surrogate code *units* to
         * special values optimized for UTF-16 string processing
         * or for special error behavior for unpaired surrogates,
         * but those values are not to be associated with the lead surrogate
         * code *points*.
         * @draft ICU 63
         */
        UCPMAP_RANGE_FIXED_LEAD_SURROGATES,
        /**
         * ucpmap_getRange() enumerates all same-value ranges as stored in the
         * map, except that all surrogates (U+D800..U+DFFF) are treated as
         * having the surrogateValue, which is passed to getRange() as a
         * separate parameter. The surrogateValue is not transformed via
         * filter(). See U_IS_SURROGATE(c).
         *
         * Most users should use UCPMAP_RANGE_NORMAL instead.
         *
         * This option is useful for maps that map surrogate code *units* to
         * special values optimized for UTF-16 string processing
         * or for special error behavior for unpaired surrogates,
         * but those values are not to be associated with the lead surrogate
         * code *points*.
         * @draft ICU 63
         */
        UCPMAP_RANGE_FIXED_ALL_SURROGATES
    };
    typedef enum UCPMapRangeOption UCPMapRangeOption;

    /**
     * Callback function type: Modifies a map value.
     * Optionally called by
     * ucpmap_getRange()/ucptrie_getRange()/umutablecptrie_getRange(). The
     * modified value will be returned by the getRange function.
     *
     * Can be used to ignore some of the value bits,
     * make a filter for one of several values,
     * return a value index computed from the map value, etc.
     *
     * @param context an opaque pointer, as passed into the getRange function
     * @param value a value from the map
     * @return the modified value
     * @draft ICU 63
     */
    typedef uint32_t UCPMapValueFilter(const void * context, uint32_t value);

}}}}

#endif
