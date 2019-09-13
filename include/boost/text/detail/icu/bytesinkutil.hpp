// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

// bytesinkutil.h
// created: 2017sep14 Markus W. Scherer
#ifndef BYTESINKUTIL_H_
#define BYTESINKUTIL_H_

#include "utypes.hpp"
#include "utf8.hpp"
#include "utf16.hpp"

#include <boost/assert.hpp>


namespace boost { namespace text { namespace detail { namespace icu {

    /**
     * \def UPRV_LENGTHOF
     * Convenience macro to determine the length of a fixed array at
     * compile-time.
     * @param array A fixed length array
     * @return The length of the array, in elements
     * @internal
     */
    template<typename T, std::size_t N>
    int32_t UPRV_LENGTHOF(T (&array)[N])
    {
        return (int32_t)(sizeof(array) / sizeof(array[0]));
    }

    class ByteSinkUtil
    {
    public:
        ByteSinkUtil() = delete; // all static

        /** (length) bytes were mapped to valid (s16, s16Length). */
        template<typename UTF8Appender>
        static UBool appendChange(
            int32_t length,
            const char16_t * s16,
            int32_t s16Length,
            UTF8Appender & appender,
            UErrorCode & errorCode)
        {
            if (U_FAILURE(errorCode)) {
                return FALSE;
            }
            char scratch[200];
            int32_t s8Length = 0;
            for (int32_t i = 0; i < s16Length;) {
                int32_t capacity;
                int32_t desiredCapacity = s16Length - i;
                if (desiredCapacity < (INT32_MAX / 3)) {
                    desiredCapacity *=
                        3; // max 3 UTF-8 bytes per UTF-16 code unit
                } else if (desiredCapacity < (INT32_MAX / 2)) {
                    desiredCapacity *= 2;
                } else {
                    desiredCapacity = INT32_MAX;
                }
#if 1
                char * buffer = scratch;
                if (U8_MAX_LENGTH < 1 ||
                    UPRV_LENGTHOF(scratch) < U8_MAX_LENGTH) {
                    capacity = 0;
                    buffer = nullptr;
                }
                capacity = UPRV_LENGTHOF(scratch);
#else
                char * buffer = sink.GetAppendBuffer(
                    U8_MAX_LENGTH,
                    desiredCapacity,
                    scratch,
                    UPRV_LENGTHOF(scratch),
                    &capacity);
#endif
                capacity -= U8_MAX_LENGTH - 1;
                int32_t j = 0;
                for (; i < s16Length && j < capacity;) {
                    UChar32 c;
                    U16_NEXT_UNSAFE(s16, i, c);
                    U8_APPEND_UNSAFE(buffer, j, c);
                }
                if (j > (INT32_MAX - s8Length)) {
                    errorCode = U_INDEX_OUTOFBOUNDS_ERROR;
                    return FALSE;
                }
                appender.append(buffer, j);
                s8Length += j;
            }
            return TRUE;
        }


        /** The bytes at [s, limit[ were mapped to valid (s16, s16Length). */
        template<typename UTF8Appender>
        static UBool appendChange(
            const uint8_t * s,
            const uint8_t * limit,
            const char16_t * s16,
            int32_t s16Length,
            UTF8Appender & appender,
            UErrorCode & errorCode)
        {
            if (U_FAILURE(errorCode)) {
                return FALSE;
            }
            if ((limit - s) > INT32_MAX) {
                errorCode = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            return appendChange(
                (int32_t)(limit - s), s16, s16Length, appender, errorCode);
        }

        /** (length) bytes were mapped/changed to valid code point c. */
        template<typename UTF8Appender>
        static void
        appendCodePoint(int32_t length, UChar32 c, UTF8Appender & appender)
        {
            char s8[U8_MAX_LENGTH];
            int32_t s8Length = 0;
            U8_APPEND_UNSAFE(s8, s8Length, c);
            appender.append(s8, s8Length);
        }

        /** The few bytes at [src, nextSrc[ were mapped/changed to valid code
         * point c. */
        template<typename UTF8Appender>
        static inline void appendCodePoint(
            const uint8_t * src,
            const uint8_t * nextSrc,
            UChar32 c,
            UTF8Appender & appender)
        {
            appendCodePoint((int32_t)(nextSrc - src), c, appender);
        }

        /** Append the two-byte character (U+0080..U+07FF). */
        template<typename UTF8Appender>
        static void appendTwoBytes(UChar32 c, UTF8Appender & appender)
        {
            BOOST_ASSERT(0x80 <= c && c <= 0x7ff); // 2-byte UTF-8
            char s8[2] = {(char)getTwoByteLead(c), (char)getTwoByteTrail(c)};
            appender.append(s8, 2);
        }

        template<typename UTF8Appender>
        static UBool appendUnchanged(
            const uint8_t * s,
            int32_t length,
            UTF8Appender & appender,
            UErrorCode & errorCode)
        {
            if (U_FAILURE(errorCode)) {
                return FALSE;
            }
            if (length > 0) {
                appendNonEmptyUnchanged(s, length, appender);
            }
            return TRUE;
        }

        template<typename UTF8Appender>
        static UBool appendUnchanged(
            const uint8_t * s,
            const uint8_t * limit,
            UTF8Appender & appender,
            UErrorCode & errorCode)
        {
            if (U_FAILURE(errorCode)) {
                return FALSE;
            }
            if ((limit - s) > INT32_MAX) {
                errorCode = U_INDEX_OUTOFBOUNDS_ERROR;
                return FALSE;
            }
            int32_t length = (int32_t)(limit - s);
            if (length > 0) {
                appendNonEmptyUnchanged(s, length, appender);
            }
            return TRUE;
        }

    private:
        // See unicode/utf8.h U8_APPEND_UNSAFE().
        static uint8_t getTwoByteLead(UChar32 c)
        {
            return (uint8_t)((c >> 6) | 0xc0);
        }
        static uint8_t getTwoByteTrail(UChar32 c)
        {
            return (uint8_t)((c & 0x3f) | 0x80);
        }

        template<typename UTF8Appender>
        static void appendNonEmptyUnchanged(
            const uint8_t * s, int32_t length, UTF8Appender & appender)
        {
            BOOST_ASSERT(length > 0);
            auto const first = reinterpret_cast<const char *>(s);
            appender.append(first, length);
        }
    };

}}}}

#endif
