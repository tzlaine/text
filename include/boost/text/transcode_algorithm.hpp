#ifndef BOOST_TEXT_TRANSCODE_ALGORITHM_HPP
#define BOOST_TEXT_TRANSCODE_ALGORITHM_HPP

#include <boost/text/utf8.hpp>

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
        OutIter read_into_utf8_iter(uint32_t cp, OutIter out)
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
        OutIter read_into_utf16_iter(uint32_t cp, OutIter out)
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

        enum utf8_char_class : uint8_t {
            ill = 0,
            asc = 1,
            cr1 = 2,
            cr2 = 3,
            cr3 = 4,
            l2a = 5,
            l3a = 6,
            l3b = 7,
            l3c = 8,
            l4a = 9,
            l4b = 10,
            l4c = 11,
        };

        enum utf8_table_state : uint8_t {
            bgn = 0,
            end = bgn,
            err = 12,
            cs1 = 24,
            cs2 = 36,
            cs3 = 48,
            p3a = 60,
            p3b = 72,
            p4a = 84,
            p4b = 96,
        };

        struct first_cu
        {
            unsigned char initial_octet;
            utf8_table_state next;
        };

        constexpr first_cu first_cus[256] = {
            {0x00, bgn}, {0x01, bgn}, {0x02, bgn}, {0x03, bgn}, {0x04, bgn},
            {0x05, bgn}, {0x06, bgn}, {0x07, bgn}, {0x08, bgn}, {0x09, bgn},
            {0x0a, bgn}, {0x0b, bgn}, {0x0c, bgn}, {0x0d, bgn}, {0x0e, bgn},
            {0x0f, bgn}, {0x10, bgn}, {0x11, bgn}, {0x12, bgn}, {0x13, bgn},
            {0x14, bgn}, {0x15, bgn}, {0x16, bgn}, {0x17, bgn}, {0x18, bgn},
            {0x19, bgn}, {0x1a, bgn}, {0x1b, bgn}, {0x1c, bgn}, {0x1d, bgn},
            {0x1e, bgn}, {0x1f, bgn}, {0x20, bgn}, {0x21, bgn}, {0x22, bgn},
            {0x23, bgn}, {0x24, bgn}, {0x25, bgn}, {0x26, bgn}, {0x27, bgn},
            {0x28, bgn}, {0x29, bgn}, {0x2a, bgn}, {0x2b, bgn}, {0x2c, bgn},
            {0x2d, bgn}, {0x2e, bgn}, {0x2f, bgn}, {0x30, bgn}, {0x31, bgn},
            {0x32, bgn}, {0x33, bgn}, {0x34, bgn}, {0x35, bgn}, {0x36, bgn},
            {0x37, bgn}, {0x38, bgn}, {0x39, bgn}, {0x3a, bgn}, {0x3b, bgn},
            {0x3c, bgn}, {0x3d, bgn}, {0x3e, bgn}, {0x3f, bgn}, {0x40, bgn},
            {0x41, bgn}, {0x42, bgn}, {0x43, bgn}, {0x44, bgn}, {0x45, bgn},
            {0x46, bgn}, {0x47, bgn}, {0x48, bgn}, {0x49, bgn}, {0x4a, bgn},
            {0x4b, bgn}, {0x4c, bgn}, {0x4d, bgn}, {0x4e, bgn}, {0x4f, bgn},
            {0x50, bgn}, {0x51, bgn}, {0x52, bgn}, {0x53, bgn}, {0x54, bgn},
            {0x55, bgn}, {0x56, bgn}, {0x57, bgn}, {0x58, bgn}, {0x59, bgn},
            {0x5a, bgn}, {0x5b, bgn}, {0x5c, bgn}, {0x5d, bgn}, {0x5e, bgn},
            {0x5f, bgn}, {0x60, bgn}, {0x61, bgn}, {0x62, bgn}, {0x63, bgn},
            {0x64, bgn}, {0x65, bgn}, {0x66, bgn}, {0x67, bgn}, {0x68, bgn},
            {0x69, bgn}, {0x6a, bgn}, {0x6b, bgn}, {0x6c, bgn}, {0x6d, bgn},
            {0x6e, bgn}, {0x6f, bgn}, {0x70, bgn}, {0x71, bgn}, {0x72, bgn},
            {0x73, bgn}, {0x74, bgn}, {0x75, bgn}, {0x76, bgn}, {0x77, bgn},
            {0x78, bgn}, {0x79, bgn}, {0x7a, bgn}, {0x7b, bgn}, {0x7c, bgn},
            {0x7d, bgn}, {0x7e, bgn}, {0x7f, bgn}, {0x00, err}, {0x01, err},
            {0x02, err}, {0x03, err}, {0x04, err}, {0x05, err}, {0x06, err},
            {0x07, err}, {0x08, err}, {0x09, err}, {0x0a, err}, {0x0b, err},
            {0x0c, err}, {0x0d, err}, {0x0e, err}, {0x0f, err}, {0x10, err},
            {0x11, err}, {0x12, err}, {0x13, err}, {0x14, err}, {0x15, err},
            {0x16, err}, {0x17, err}, {0x18, err}, {0x19, err}, {0x1a, err},
            {0x1b, err}, {0x1c, err}, {0x1d, err}, {0x1e, err}, {0x1f, err},
            {0x20, err}, {0x21, err}, {0x22, err}, {0x23, err}, {0x24, err},
            {0x25, err}, {0x26, err}, {0x27, err}, {0x28, err}, {0x29, err},
            {0x2a, err}, {0x2b, err}, {0x2c, err}, {0x2d, err}, {0x2e, err},
            {0x2f, err}, {0x30, err}, {0x31, err}, {0x32, err}, {0x33, err},
            {0x34, err}, {0x35, err}, {0x36, err}, {0x37, err}, {0x38, err},
            {0x39, err}, {0x3a, err}, {0x3b, err}, {0x3c, err}, {0x3d, err},
            {0x3e, err}, {0x3f, err}, {0xc0, err}, {0xc1, err}, {0x02, cs1},
            {0x03, cs1}, {0x04, cs1}, {0x05, cs1}, {0x06, cs1}, {0x07, cs1},
            {0x08, cs1}, {0x09, cs1}, {0x0a, cs1}, {0x0b, cs1}, {0x0c, cs1},
            {0x0d, cs1}, {0x0e, cs1}, {0x0f, cs1}, {0x10, cs1}, {0x11, cs1},
            {0x12, cs1}, {0x13, cs1}, {0x14, cs1}, {0x15, cs1}, {0x16, cs1},
            {0x17, cs1}, {0x18, cs1}, {0x19, cs1}, {0x1a, cs1}, {0x1b, cs1},
            {0x1c, cs1}, {0x1d, cs1}, {0x1e, cs1}, {0x1f, cs1}, {0x00, p3a},
            {0x01, cs2}, {0x02, cs2}, {0x03, cs2}, {0x04, cs2}, {0x05, cs2},
            {0x06, cs2}, {0x07, cs2}, {0x08, cs2}, {0x09, cs2}, {0x0a, cs2},
            {0x0b, cs2}, {0x0c, cs2}, {0x0d, p3b}, {0x0e, cs2}, {0x0f, cs2},
            {0x00, p4a}, {0x01, cs3}, {0x02, cs3}, {0x03, cs3}, {0x04, p4b},
            {0xf5, err}, {0xf6, err}, {0xf7, err}, {0xf8, err}, {0xf9, err},
            {0xfa, err}, {0xfb, err}, {0xfc, err}, {0xfd, err}, {0xfe, err},
            {0xff, err},
        };

        constexpr utf8_char_class octet_classes[256] = {
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
            asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, cr1, cr1,
            cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1,
            cr1, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2,
            cr2, cr2, cr2, cr2, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3,
            cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3,
            cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, ill, ill, l2a,
            l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a,
            l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a,
            l2a, l2a, l2a, l3a, l3b, l3b, l3b, l3b, l3b, l3b, l3b, l3b, l3b,
            l3b, l3b, l3b, l3c, l3b, l3b, l4a, l4b, l4b, l4b, l4c, ill, ill,
            ill, ill, ill, ill, ill, ill, ill, ill, ill,
        };

        constexpr utf8_table_state transitions[108] = {
            err, end, err, err, err, cs1, p3a, cs2, p3b, p4a, cs3, p4b,
            err, err, err, err, err, err, err, err, err, err, err, err,
            err, err, end, end, end, err, err, err, err, err, err, err,
            err, err, cs1, cs1, cs1, err, err, err, err, err, err, err,
            err, err, cs2, cs2, cs2, err, err, err, err, err, err, err,
            err, err, err, err, cs1, err, err, err, err, err, err, err,
            err, err, cs1, cs1, err, err, err, err, err, err, err, err,
            err, err, err, cs2, cs2, err, err, err, err, err, err, err,
            err, err, cs2, err, err, err, err, err, err, err, err, err,
        };

        template<typename InputIter, typename Sentinel>
        uint32_t utf8_advance(InputIter & first, Sentinel last)
        {
            uint32_t retval = 0;

            first_cu const info = first_cus[(unsigned char)*first];
            ++first;

            retval = info.initial_octet;
            int state = info.next;

            while (state != bgn) {
                if (first != last) {
                    unsigned char const cu = *first;
                    retval = (retval << 6) | (cu & 0x3f);
                    utf8_char_class const class_ = octet_classes[cu];
                    state = transitions[state + class_];
                    if (state == err)
                        return utf8::replacement_character();
                    ++first;
                } else {
                    return utf8::replacement_character();
                }
            }

            return retval;
        }

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
                    auto const cp = detail::utf8_advance(first, last);
                    out = detail::read_into_utf16_iter(cp, out);
                }
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
            uint32_t v;
            constexpr int debruijn_bit_position[32] = {
                0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9,
            };
            return debruijn_bit_position
                [((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
        }
#endif

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
            std::random_access_iterator_tag)
        {
#if BOOST_TEXT_USE_SIMD
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
                    *out = detail::utf8_advance(first, last);
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
            while (first < (last - sizeof(__m128i))) {
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
            if (utf8::surrogate(hi)) {
                if (hi <= high_surrogate_max) {
                    ++first;
                    if (first == last) {
                        uint32_t const cp = utf8::replacement_character();
                        out = detail::read_into_utf8_iter(cp, out);
                        ++out;
                        return out;
                    }
                    uint32_t const lo = *first;
                    if (utf8::low_surrogate(lo)) {
                        uint32_t const cp = ((hi - high_surrogate_base) << 10) +
                                            (lo - low_surrogate_base);
                        out = detail::read_into_utf8_iter(cp, out);
                        continue;
                    }
                }
                out = detail::read_into_utf8_iter(
                    utf8::replacement_character(), out);
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
            if (utf8::surrogate(hi)) {
                if (hi <= high_surrogate_max) {
                    ++first;
                    if (first == last) {
                        *out = utf8::replacement_character();
                        ++out;
                        return out;
                    }
                    uint32_t const lo = *first;
                    if (utf8::low_surrogate(lo)) {
                        uint32_t const cp = ((hi - high_surrogate_base) << 10) +
                                            (lo - low_surrogate_base);
                        *out = cp;
                        ++out;
                        continue;
                    }
                }
                *out = utf8::replacement_character();
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
