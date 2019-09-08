/*
Copyright (c) 2018 Robert N. Steagall and KEWB Computing
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

 1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimers.
 2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimers in the documentation
and/or other materials provided with the distribution.
 3. Neither the names of the copyright holder, nor the names of its contributors
may be used to endorse or promote products derived from this Software without
specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/
// The copyright notice above applies to the tables and enums used in the
// table-based UTF-8 conversion code, and in the SSE2-specific optimization.
#ifndef BOOST_TEXT_TRANSCODE_ALGORITHM_HPP
#define BOOST_TEXT_TRANSCODE_ALGORITHM_HPP

#include <boost/text/transcode_iterator.hpp>

#include <boost/config.hpp>
#include <boost/predef/hardware/simd.h>


#if !defined(BOOST_TEXT_USE_SIMD) || BOOST_TEXT_USE_SIMD == 1
#if BOOST_HW_SIMD_X86_SSE2_VERSION <= BOOST_HW_SIMD_X86
#if defined(BOOST_GCC) || defined(BOOST_CLANG)
#include <x86intrin.h>
#define BOOST_TEXT_USE_SIMD 1
#elif defined(BOOST_MSVC)
#include <intrin.h>
#define BOOST_TEXT_USE_SIMD 1
#else
#define BOOST_TEXT_USE_SIMD 0
#endif
#endif
#endif


namespace boost { namespace text {

    namespace detail {
        template<typename OutIter>
        BOOST_TEXT_CXX14_CONSTEXPR OutIter
        read_into_utf8_iter(uint32_t cp, OutIter out)
        {
            if (cp < 0x80) {
                *out = static_cast<char>(cp);
                ++out;
            } else if (cp < 0x800) {
                *out = static_cast<char>(0xC0 + (cp >> 6));
                ++out;
                *out = static_cast<char>(0x80 + (cp & 0x3f));
                ++out;
            } else if (cp < 0x10000) {
                *out = static_cast<char>(0xe0 + (cp >> 12));
                ++out;
                *out = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                ++out;
                *out = static_cast<char>(0x80 + (cp & 0x3f));
                ++out;
            } else {
                *out = static_cast<char>(0xf0 + (cp >> 18));
                ++out;
                *out = static_cast<char>(0x80 + ((cp >> 12) & 0x3f));
                ++out;
                *out = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                ++out;
                *out = static_cast<char>(0x80 + (cp & 0x3f));
                ++out;
            }
            return out;
        }

        template<typename OutIter>
        BOOST_TEXT_CXX14_CONSTEXPR OutIter
        read_into_utf16_iter(uint32_t cp, OutIter out)
        {
            uint16_t const high_surrogate_base = 0xd7c0;
            uint16_t const low_surrogate_base = 0xdc00;

            if (cp < 0x10000) {
                *out = static_cast<uint16_t>(cp);
                ++out;
            } else {
                *out = static_cast<uint16_t>(cp >> 10) + high_surrogate_base;
                ++out;
                *out = static_cast<uint16_t>(cp & 0x3ff) + low_surrogate_base;
                ++out;
            }
            return out;
        }

#if BOOST_TEXT_USE_SIMD
        template<typename Iter>
        __m128i load_chars_for_sse(Iter it) noexcept
        {
            std::array<char, 16> chars;
            std::copy(it, it + 16, chars.begin());
            return _mm_loadu_si128((__m128i const *)chars.data());
        }

        inline __m128i load_chars_for_sse(char const * it) noexcept
        {
            return _mm_loadu_si128((__m128i const *)it);
        }

#if defined(BOOST_GCC) || defined(BOOST_CLANG)
        inline int trailing_zeros(uint32_t x) noexcept
        {
            return __builtin_ctz(x);
        }
#elif defined(BOOST_MSVC)
        inline int trailing_zeros(uint32_t x) noexcept
        {
            unsigned long retval;
            _BitScanForward(&retval, (unsigned long)x);
            return (int)retval;
        }
#else
        // Table taken from
        // https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
        inline int trailing_zeros(uint32_t x) noexcept
        {
            constexpr int debruijn_bit_position[32] = {
                0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9,
            };
            return debruijn_bit_position
                [((uint32_t)((x & -x) * 0x077CB531U)) >> 27];
        }
#endif

        // TODO: Overload based on contiguous iterators in C++20 and later.
        template<typename OutIter>
        int32_t * sse_out_ptr(OutIter out, std::array<int32_t, 16> & tmp)
        {
            return tmp.data();
        }

        inline int32_t *
        sse_out_ptr(int32_t * out, std::array<int32_t, 16> & tmp)
        {
            return out;
        }

        inline int32_t *
        sse_out_ptr(uint32_t * out, std::array<int32_t, 16> & tmp)
        {
            return (int32_t *)out;
        }

        template<typename OutIter>
        void finalize_sse_out(
            OutIter & out, std::array<int32_t, 16> const & tmp, int incr)
        {
            out = std::copy(tmp.begin(), tmp.begin() + incr, out);
        }

        inline void finalize_sse_out(
            int32_t *& out, std::array<int32_t, 16> const & tmp, int incr)
        {
            out += incr;
        }

        inline void finalize_sse_out(
            uint32_t *& out, std::array<int32_t, 16> const & tmp, int incr)
        {
            out += incr;
        }
#endif

        template<typename InputIter, typename Sentinel, typename OutIter>
        OutIter transcode_utf_8_to_16(
            InputIter first,
            Sentinel last,
            OutIter out,
            std::input_iterator_tag)
        {
            while (first != last) {
                unsigned char const c = *first;
                if (c < 0x80) {
                    *out = *first;
                    ++first;
                    ++out;
                } else {
                    auto const cp = detail::advance(first, last);
                    out = detail::read_into_utf16_iter(cp, out);
                }
            }
            return out;
        }

        template<typename Iter, typename OutIter>
        OutIter transcode_utf_8_to_16(
            Iter first, Iter last, OutIter out, std::random_access_iterator_tag)
        {
#if BOOST_TEXT_USE_SIMD
            while ((int)sizeof(__m128i) <= last - first) {
                if ((unsigned char)*first < 0x80) {
                    __m128i chunk = load_chars_for_sse(first);
                    int32_t mask = _mm_movemask_epi8(chunk);
                    __m128i half = _mm_unpacklo_epi8(chunk, _mm_set1_epi8(0));
                    std::array<int32_t, 16> temp;
                    int32_t * out_ptr = sse_out_ptr(out, temp);
                    _mm_storeu_si128((__m128i *)out_ptr, half);
                    half = _mm_unpackhi_epi8(chunk, _mm_set1_epi8(0));
                    _mm_storeu_si128((__m128i *)(out_ptr + 8), half);

                    int const incr = mask == 0 ? 16 : trailing_zeros(mask);
                    first += incr;
                    finalize_sse_out(out, temp, incr);
                } else {
                    auto const cp = detail::advance(first, last);
                    out = detail::read_into_utf16_iter(cp, out);
                }
            }
#endif

            return transcode_utf_8_to_16(
                first, last, out, std::input_iterator_tag{});
        }

        template<typename InputIter, typename Sentinel, typename OutIter>
        OutIter transcode_utf_8_to_32(
            InputIter first,
            Sentinel last,
            OutIter out,
            std::input_iterator_tag)
        {
            while (first != last) {
                unsigned char const c = *first;
                if (c < 0x80) {
                    *out = *first;
                    ++first;
                    ++out;
                } else {
                    *out = detail::advance(first, last);
                    ++out;
                }
            }
            return out;
        }

        template<typename Iter, typename OutIter>
        OutIter transcode_utf_8_to_32(
            Iter first, Iter last, OutIter out, std::random_access_iterator_tag)
        {
#if BOOST_TEXT_USE_SIMD
            while ((int)sizeof(__m128i) <= last - first) {
                if ((unsigned char)*first < 0x80) {
                    __m128i zero = _mm_set1_epi8(0);
                    __m128i chunk = load_chars_for_sse(first);
                    int32_t const mask = _mm_movemask_epi8(chunk);

                    __m128i half = _mm_unpacklo_epi8(chunk, zero);
                    __m128i qrtr = _mm_unpacklo_epi16(half, zero);
                    std::array<int32_t, 16> temp;
                    int32_t * out_ptr = sse_out_ptr(out, temp);
                    _mm_storeu_si128((__m128i *)out_ptr, qrtr);
                    qrtr = _mm_unpackhi_epi16(half, zero);
                    _mm_storeu_si128((__m128i *)(out_ptr + 4), qrtr);

                    half = _mm_unpackhi_epi8(chunk, zero);
                    qrtr = _mm_unpacklo_epi16(half, zero);
                    _mm_storeu_si128((__m128i *)(out_ptr + 8), qrtr);
                    qrtr = _mm_unpackhi_epi16(half, zero);
                    _mm_storeu_si128((__m128i *)(out_ptr + 12), qrtr);

                    int const incr = mask == 0 ? 16 : trailing_zeros(mask);
                    first += incr;
                    finalize_sse_out(out, temp, incr);
                } else {
                    *out = detail::advance(first, last);
                    ++out;
                }
            }
#endif

            return transcode_utf_8_to_32(
                first, last, out, std::input_iterator_tag{});
        }
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-8 to UTF-16.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_8_to_16(InputIter first, Sentinel last, OutIter out)
    {
        return detail::transcode_utf_8_to_16(
            first,
            last,
            out,
            typename std::iterator_traits<InputIter>::iterator_category{});
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-8 to UTF-16.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_8_to_16(Range const & r, OutIter out)
    {
        return transcode_utf_8_to_16(r.begin(), r.end(), out);
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-8 to UTF-32.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_8_to_32(InputIter first, Sentinel last, OutIter out)
    {
        return detail::transcode_utf_8_to_32(
            first,
            last,
            out,
            typename std::iterator_traits<InputIter>::iterator_category{});
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-8 to UTF-32.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_8_to_32(Range const & r, OutIter out)
    {
        return transcode_utf_8_to_32(r.begin(), r.end(), out);
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-16 to UTF-8.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_16_to_8(InputIter first, Sentinel last, OutIter out)
    {
        uint32_t const high_surrogate_max = 0xdbff;
        uint16_t const high_surrogate_base = 0xd7c0;
        uint16_t const low_surrogate_base = 0xdc00;

        for (; first != last; ++first) {
            uint32_t const hi = *first;
            if (surrogate(hi)) {
                if (hi <= high_surrogate_max) {
                    ++first;
                    if (first == last) {
                        uint32_t const cp = replacement_character();
                        out = detail::read_into_utf8_iter(cp, out);
                        ++out;
                        return out;
                    }
                    uint32_t const lo = *first;
                    if (low_surrogate(lo)) {
                        uint32_t const cp = ((hi - high_surrogate_base) << 10) +
                                            (lo - low_surrogate_base);
                        out = detail::read_into_utf8_iter(cp, out);
                        continue;
                    }
                }
                out = detail::read_into_utf8_iter(replacement_character(), out);
            } else {
                out = detail::read_into_utf8_iter(hi, out);
            }
        }

        return out;
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-16 to UTF-8.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_16_to_8(Range const & r, OutIter out)
    {
        return transcode_utf_16_to_8(r.begin(), r.end(), out);
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-16 to UTF-32.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_16_to_32(InputIter first, Sentinel last, OutIter out)
    {
        uint32_t const high_surrogate_max = 0xdbff;
        uint16_t const high_surrogate_base = 0xd7c0;
        uint16_t const low_surrogate_base = 0xdc00;

        for (; first != last; ++first) {
            uint32_t const hi = *first;
            if (surrogate(hi)) {
                if (hi <= high_surrogate_max) {
                    ++first;
                    if (first == last) {
                        *out = replacement_character();
                        ++out;
                        return out;
                    }
                    uint32_t const lo = *first;
                    if (low_surrogate(lo)) {
                        uint32_t const cp = ((hi - high_surrogate_base) << 10) +
                                            (lo - low_surrogate_base);
                        *out = cp;
                        ++out;
                        continue;
                    }
                }
                *out = replacement_character();
                ++out;
            } else {
                *out = hi;
                ++out;
            }
        }

        return out;
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-16 to UTF-32.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_16_to_32(Range const & r, OutIter out)
    {
        return transcode_utf_16_to_32(r.begin(), r.end(), out);
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-32 to UTF-8.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_32_to_8(InputIter first, Sentinel last, OutIter out)
    {
        for (; first != last; ++first) {
            out = detail::read_into_utf8_iter(*first, out);
        }
        return out;
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-32 to UTF-8.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_32_to_8(Range const & r, OutIter out)
    {
        return transcode_utf_32_to_8(r.begin(), r.end(), out);
    }

    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-32 to UTF-16.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    OutIter transcode_utf_32_to_16(InputIter first, Sentinel last, OutIter out)
    {
        for (; first != last; ++first) {
            out = detail::read_into_utf16_iter(*first, out);
        }
        return out;
    }

    /** Copies the code points in the range r to out, changing the encoding
        from UTF-32 to UTF-16.  */
    template<typename Range, typename OutIter>
    OutIter transcode_utf_32_to_16(Range const & r, OutIter out)
    {
        return transcode_utf_32_to_16(r.begin(), r.end(), out);
    }

}}

#endif
