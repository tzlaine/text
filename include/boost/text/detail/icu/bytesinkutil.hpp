// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

// bytesinkutil.h
// created: 2017sep14 Markus W. Scherer
#ifndef BYTESINKUTIL_H_
#define BYTESINKUTIL_H_

#include <boost/text/detail/icu/utf8.hpp>
#include <boost/text/detail/icu/utf16.hpp>

#include <boost/assert.hpp>


namespace boost { namespace text { inline namespace v1 { namespace detail { namespace icu {

    /**
     * \def uprv_lengthof
     * Convenience macro to determine the length of a fixed array at
     * compile-time.
     * @param array A fixed length array
     * @return The length of the array, in elements
     * @internal
     */
    template<typename T, std::size_t N>
    int32_t uprv_lengthof(T (&array)[N])
    {
        return (int32_t)(sizeof(array) / sizeof(array[0]));
    }

    template<typename Iter, typename Sentinel>
    std::ptrdiff_t dist(Iter first, Sentinel last)
    {
        std::ptrdiff_t retval = 0;
        for (; first != last; ++first) {
            ++retval;
        }
        return retval;
    }

    template<typename Iter>
    std::ptrdiff_t dist(Iter first, Iter last)
    {
        return std::distance(first, last);
    }

    namespace ByteSinkUtil {
        // See unicode/utf8.h U8_APPEND_UNSAFE().
        inline uint8_t getTwoByteLead(UChar32 c)
        {
            return (uint8_t)((c >> 6) | 0xc0);
        }
        inline uint8_t getTwoByteTrail(UChar32 c)
        {
            return (uint8_t)((c & 0x3f) | 0x80);
        }

        template<typename CharIter, typename UTF8Appender>
        void appendNonEmptyUnchanged(
            CharIter s, int32_t length, UTF8Appender & appender)
        {
            BOOST_ASSERT(length > 0);
            appender.append(s, s + length);
        }

        /** (length) bytes were mapped to valid (s16, s16Length). */
        template<typename UTF8Appender>
        UBool appendChange(
            int32_t length,
            const char16_t * s16,
            int32_t s16Length,
            UTF8Appender & appender)
        {
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
                char * buffer = scratch;
                if (detail::icu::u8_max_length < 1 ||
                    detail::icu::uprv_lengthof(scratch) < u8_max_length) {
                    capacity = 0;
                    buffer = nullptr;
                }
                capacity = detail::icu::uprv_lengthof(scratch);
                capacity -= u8_max_length - 1;
                int32_t j = 0;
                for (; i < s16Length && j < capacity;) {
                    UChar32 c;
                    detail::icu::u16_next_unsafe(s16, i, c);
                    detail::icu::u8_append_unsafe(buffer, j, c);
                }
                if (j > (INT32_MAX - s8Length)) {
                    return false;
                }
                appender.append(buffer, buffer + j);
                s8Length += j;
            }
            return true;
        }

        /** The bytes at [s, limit[ were mapped to valid (s16, s16Length). */
        template<typename CharIter, typename UTF8Appender>
        UBool appendChange(
            CharIter s,
            CharIter limit,
            const char16_t * s16,
            int32_t s16Length,
            UTF8Appender & appender)
        {
            if ((limit - s) > INT32_MAX) {
                return false;
            }
            return ByteSinkUtil::appendChange(
                (int32_t)(limit - s), s16, s16Length, appender);
        }

        /** (length) bytes were mapped/changed to valid code point c. */
        template<typename UTF8Appender>
        void appendCodePoint(int32_t length, UChar32 c, UTF8Appender & appender)
        {
            char s8[u8_max_length];
            int32_t s8Length = 0;
            detail::icu::u8_append_unsafe(s8, s8Length, c);
            appender.append(s8, s8 + s8Length);
        }

        /** The few bytes at [src, nextSrc[ were mapped/changed to valid code
         * point c. */
        template<typename CharIter, typename UTF8Appender>
        void appendCodePoint(
            CharIter src, CharIter nextSrc, UChar32 c, UTF8Appender & appender)
        {
            ByteSinkUtil::appendCodePoint(
                (int32_t)(nextSrc - src), c, appender);
        }

        /** Append the two-byte character (U+0080..U+07FF). */
        template<typename UTF8Appender>
        void appendTwoBytes(UChar32 c, UTF8Appender & appender)
        {
            BOOST_ASSERT(0x80 <= c && c <= 0x7ff); // 2-byte UTF-8
            char s8[2] = {(char)ByteSinkUtil::getTwoByteLead(c),
                          (char)ByteSinkUtil::getTwoByteTrail(c)};
            appender.append(s8, 2);
        }

        template<typename CharIter, typename UTF8Appender>
        UBool
        appendUnchanged(CharIter s, int32_t length, UTF8Appender & appender)
        {
            if (length > 0) {
                ByteSinkUtil::appendNonEmptyUnchanged(s, length, appender);
            }
            return true;
        }

        template<typename CharIter, typename Sentinel, typename UTF8Appender>
        UBool
        appendUnchanged(CharIter s, Sentinel limit, UTF8Appender & appender)
        {
            if (detail::icu::dist(s, limit) > INT32_MAX) {
                return false;
            }
            int32_t length = (int32_t)detail::icu::dist(s, limit);
            if (length > 0) {
                ByteSinkUtil::appendNonEmptyUnchanged(s, length, appender);
            }
            return true;
        }
    }

}}}}}

#endif
