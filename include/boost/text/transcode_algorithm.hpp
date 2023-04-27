// Copyright (C) 2018 Robert N. Steagall
// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_ALGORITHM_HPP
#define BOOST_TEXT_TRANSCODE_ALGORITHM_HPP

#include <boost/text/in_out_result.hpp>
#include <boost/text/transcode_iterator.hpp>
#if BOOST_TEXT_USE_CONCEPTS
#include <algorithm>
#endif

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

    // TODO: Should this live somewhere else?
    template<typename Range>
    struct utf_range_like_iterator
    {
        using type = decltype(std::declval<Range>().begin());
    };

    template<typename T>
    struct utf_range_like_iterator<T *>
    {
        using type = T *;
    };

    template<std::size_t N, typename T>
    struct utf_range_like_iterator<T[N]>
    {
        using type = T *;
    };

    template<std::size_t N, typename T>
    struct utf_range_like_iterator<T (&)[N]>
    {
        using type = T *;
    };

    template<typename Range>
    using utf_range_like_iterator_t =
        typename utf_range_like_iterator<Range>::type;

    /** An alias for `in_out_result` returned by algorithms that perform a
        transcoding copy. */
    template<typename Iter, typename OutIter>
    using transcode_result = in_out_result<Iter, OutIter>;

    namespace detail {
        template<typename OutIter>
        constexpr OutIter read_into_utf8_iter(uint32_t cp, OutIter out)
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
        constexpr OutIter read_into_utf16_iter(uint32_t cp, OutIter out)
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

        template<
            bool UseN,
            typename InputIter,
            typename Sentinel,
            typename OutIter>
        transcode_result<InputIter, OutIter> transcode_utf_8_to_16(
            InputIter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out,
            std::input_iterator_tag)
        {
            for (; first != last && (!UseN || n); --n) {
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
            return {first, out};
        }

        template<bool UseN, typename Iter, typename OutIter>
        transcode_result<Iter, OutIter> transcode_utf_8_to_16(
            Iter first,
            Iter last,
            std::ptrdiff_t n,
            OutIter out,
            std::random_access_iterator_tag)
        {
#if BOOST_TEXT_USE_SIMD
            while ((int)sizeof(__m128i) <= last - first &&
                   (!UseN || (int)sizeof(__m128i) <= n)) {
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
                    n -= incr;
                    finalize_sse_out(out, temp, incr);
                } else {
                    auto const cp = detail::advance(first, last);
                    out = detail::read_into_utf16_iter(cp, out);
                    --n;
                }
            }
#endif

            return transcode_utf_8_to_16<UseN>(
                first, last, n, out, std::input_iterator_tag{});
        }

        template<
            bool UseN,
            typename InputIter,
            typename Sentinel,
            typename OutIter>
        transcode_result<InputIter, OutIter> transcode_utf_8_to_32(
            InputIter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out,
            std::input_iterator_tag)
        {
            for (; first != last && (!UseN || n); --n) {
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
            return {first, out};
        }

        template<bool UseN, typename Iter, typename OutIter>
        transcode_result<Iter, OutIter> transcode_utf_8_to_32(
            Iter first,
            Iter last,
            std::ptrdiff_t n,
            OutIter out,
            std::random_access_iterator_tag)
        {
#if BOOST_TEXT_USE_SIMD
            while ((int)sizeof(__m128i) <= last - first &&
                   (!UseN || (int)sizeof(__m128i) <= n)) {
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
                    n -= incr;
                    finalize_sse_out(out, temp, incr);
                } else {
                    *out = detail::advance(first, last);
                    ++out;
                    --n;
                }
            }
#endif

            return transcode_utf_8_to_32<UseN>(
                first, last, n, out, std::input_iterator_tag{});
        }

        template<format Tag>
        struct tag_t
        {};

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_8(
            tag_t<format::utf8>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            for (; first != last && (!UseN || n); ++first, ++out) {
                *out = *first;
                --n;
            }
            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_16(
            tag_t<format::utf8>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            return detail::transcode_utf_8_to_16<UseN>(
                first,
                last,
                n,
                out,
                typename std::iterator_traits<Iter>::iterator_category{});
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_32(
            tag_t<format::utf8>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            return detail::transcode_utf_8_to_32<UseN>(
                first,
                last,
                n,
                out,
                typename std::iterator_traits<Iter>::iterator_category{});
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_8(
            tag_t<format::utf16>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            uint32_t const high_surrogate_max = 0xdbff;
            uint16_t const high_surrogate_base = 0xd7c0;
            uint16_t const low_surrogate_base = 0xdc00;

            for (; first != last && (!UseN || n); ++first, --n) {
                uint32_t const hi = *first;
                if (surrogate(hi)) {
                    if (hi <= high_surrogate_max) {
                        ++first;
                        if (first == last) {
                            uint32_t const cp = replacement_character();
                            out = detail::read_into_utf8_iter(cp, out);
                            ++out;
                            return {first, out};
                        }
                        uint32_t const lo = *first;
                        if (low_surrogate(lo)) {
                            uint32_t const cp =
                                ((hi - high_surrogate_base) << 10) +
                                (lo - low_surrogate_base);
                            out = detail::read_into_utf8_iter(cp, out);
                            continue;
                        }
                    }
                    out = detail::read_into_utf8_iter(
                        replacement_character(), out);
                } else {
                    out = detail::read_into_utf8_iter(hi, out);
                }
            }

            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_16(
            tag_t<format::utf16>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            for (; first != last && (!UseN || n); ++first, ++out, --n) {
                *out = *first;
            }
            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_32(
            tag_t<format::utf16>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            uint32_t const high_surrogate_max = 0xdbff;
            uint16_t const high_surrogate_base = 0xd7c0;
            uint16_t const low_surrogate_base = 0xdc00;

            for (; first != last && (!UseN || n); ++first, --n) {
                uint32_t const hi = *first;
                if (surrogate(hi)) {
                    if (hi <= high_surrogate_max) {
                        ++first;
                        if (first == last) {
                            *out = replacement_character();
                            ++out;
                            return {first, out};
                        }
                        uint32_t const lo = *first;
                        if (low_surrogate(lo)) {
                            uint32_t const cp =
                                ((hi - high_surrogate_base) << 10) +
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

            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_8(
            tag_t<format::utf32>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            for (; first != last && (!UseN || n); ++first, --n) {
                out = detail::read_into_utf8_iter(*first, out);
            }
            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_16(
            tag_t<format::utf32>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            for (; first != last && (!UseN || n); ++first, --n) {
                out = detail::read_into_utf16_iter(*first, out);
            }
            return {first, out};
        }

        template<bool UseN, typename Iter, typename Sentinel, typename OutIter>
        transcode_result<Iter, OutIter> transcode_to_32(
            tag_t<format::utf32>,
            Iter first,
            Sentinel last,
            std::ptrdiff_t n,
            OutIter out)
        {
            for (; first != last && (!UseN || n); ++first, ++out, --n) {
                *out = *first;
            }
            return {first, out};
        }
    }

#if 0
    /** Copies the code points in the range [first, last) to out, changing the
        encoding from UTF-8 to UTF-32.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    transcode_result<InputIter, OutIter> transcode_utf_8_to_32_take_n(
        InputIter first, Sentinel last, std::ptrdiff_t n, OutIter out)
    {
        auto const r = detail::unpack_iterator_and_sentinel(first, last);
        return detail::transcode_to_32<true>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, n, out);
    }

    /** Copies the first `n` code points in the range [first, last) to out,
        changing the encoding from UTF-8 to UTF-32.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    transcode_result<InputIter, OutIter> transcode_utf_8_to_32_take_n(
        InputIter first, Sentinel last, std::ptrdiff_t n, OutIter out)
    {
        auto const r = detail::unpack_iterator_and_sentinel(first, last);
        return detail::transcode_to_32<true>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, n, out);
    }

    /** Copies the first `n` code points in the range [first, last) to out,
        changing the encoding from UTF-8 to UTF-32.  */
    template<typename InputIter, typename Sentinel, typename OutIter>
    transcode_result<InputIter, OutIter>
    transcode_utf_8_to_32_take_n(Range && r, std::ptrdiff_t n, OutIter out)
    {
        return detail::transcode_utf_8_to_32_dispatch<true, Range, OutIter>::
            call(r, n, out)
                .out;
    }
#endif

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    // -> utf8

    /** Copies the code points in the range `[first, last)` to `out`, changing
        the encoding to UTF-8. */
    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<uint8_t> O>
    requires(
        utf16_code_unit<std::iter_value_t<I>> ||
        utf32_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf8(I first, S last, O out);

    /** Copies the code points in the range `[p, null_sentinel)` to `out`,
        changing the encoding to UTF-8.  */
    template<typename Ptr, std::output_iterator<uint8_t> O>
    requires(utf16_pointer<Ptr> || utf32_pointer<Ptr>)
    transcode_result<Ptr, O> transcode_to_utf8(Ptr p, O out);

    /** Copies the code points in the array `arr` to `out`, changing the
        encoding to UTF-8.  */
    template<std::size_t N, typename Char, std::output_iterator<uint8_t> O>
    requires (utf16_code_unit<Char> || utf32_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf8(Char (&arr)[N], O out);

    /** Copies the code points in the range `r` to `out`, changing the
        encoding to UTF-8.  */
    template<std::ranges::input_range R, std::output_iterator<uint8_t> O>
    requires (utf16_code_unit<std::ranges::range_value_t<R>> ||
              utf32_code_unit<std::ranges::range_value_t<R>>)
    transcode_result<std::ranges::borrowed_iterator_t<R>, O>
    transcode_to_utf8(R && r, O out);


    // -> utf16

    /** Copies the code points in the range `[first, last)` to `out`, changing
        the encoding to UTF-16. */
    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<char16_t> O>
    requires (utf8_code_unit<std::iter_value_t<I>> ||
              utf32_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf16(I first, S last, O out);

    /** Copies the code points in the range `[p, null_sentinel)` to `out`,
        changing the encoding to UTF-16.  */
    template<typename Ptr, std::output_iterator<char16_t> O>
    requires (utf8_pointer<Ptr> || utf32_pointer<Ptr>)
    transcode_result<Ptr, O> transcode_to_utf16(Ptr p, O out);

    /** Copies the code points in the array `arr` to `out`, changing the
        encoding to UTF-16.  */
    template<std::size_t N, typename Char, std::output_iterator<char16_t> O>
    requires (utf8_code_unit<Char> || utf32_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf16(Char (&arr)[N], O out);

    /** Copies the code points in the range `r` to `out`, changing the
        encoding to UTF-16.  */
    template<std::ranges::input_range R, std::output_iterator<cjar16_t> O>
    requires (utf8_code_unit<std::ranges::range_value_t<R>> ||
              utf32_code_unit<std::ranges::range_value_t<R>>)
    transcode_result<std::ranges::borrowed_iterator_t<R>, O>
    transcode_to_utf16(R && r, O out);


    // -> utf32

    /** Copies the code points in the range `[first, last)` to `out`, changing
        the encoding to UTF-32. */
    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<uint32_t> O>
    requires (utf8_code_unit<std::iter_value_t<I>> ||
              utf16_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf32(I first, S last, O out);

    /** Copies the code points in the range `[p, null_sentinel)` to `out`,
        changing the encoding to UTF-32.  */
    template<typename Ptr, std::output_iterator<uint32_t> O>
    requires (utf8_pointer<Ptr> || utf16_pointer<Ptr>)
    transcode_result<Ptr, O> transcode_to_utf32(Ptr p, O out);

    /** Copies the code points in the array `arr` to `out`, changing the
        encoding to UTF-32.  */
    template<std::size_t N, typename Char, std::output_iterator<uint32_t> O>
    requires (utf8_code_unit<Char> || utf16_code_unit<Char>)
    transcode_result<Char *, O> transcode_to_utf32(Char (&arr)[N], O out);

    /** Copies the code points in the range `r` to `out`, changing the
        encoding to UTF-32.  */
    template<std::ranges::input_range R, std::output_iterator<uint32_t> O>
    requires (utf8_code_unit<std::ranges::range_value_t<R>> ||
              utf16_code_unit<std::ranges::range_value_t<R>>)
    transcode_result<std::ranges::borrowed_iterator_t<R>, O>
    transcode_to_utf32(R && r, O out);

#endif

    namespace dtl {
        template<
            bool UseN,
            typename Range,
            typename OutIter,
            bool _16Ptr = detail::is_16_ptr_v<Range>,
            bool CPPtr = detail::is_cp_ptr_v<Range>>
        struct transcode_to_8_dispatch
        {
            static constexpr auto
            call(Range && r, std::ptrdiff_t n, OutIter out)
                -> transcode_result<decltype(detail::begin(r)), OutIter>
            {
                auto const u = text::unpack_iterator_and_sentinel(
                    detail::begin(r), detail::end(r));
                auto unpacked = detail::transcode_to_8<UseN>(
                    detail::tag_t<u.format_tag>{}, u.first, u.last, n, out);
                return {u.repack(unpacked.in), unpacked.out};
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_8_dispatch<UseN, Ptr, OutIter, true, false>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_8<UseN>(
                    detail::tag_t<format::utf16>{}, p, null_sentinel, n, out);
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_8_dispatch<UseN, Ptr, OutIter, false, true>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_8<UseN>(
                    detail::tag_t<format::utf32>{}, p, null_sentinel, n, out);
            }
        };

        template<
            bool UseN,
            typename Range,
            typename OutIter,
            bool CharPtr = detail::is_char_ptr_v<Range>,
            bool CPPtr = detail::is_cp_ptr_v<Range>>
        struct transcode_to_16_dispatch
        {
            static constexpr auto
            call(Range && r, std::ptrdiff_t n, OutIter out)
                -> transcode_result<decltype(detail::begin(r)), OutIter>
            {
                auto const u = text::unpack_iterator_and_sentinel(
                    detail::begin(r), detail::end(r));
                auto unpacked = detail::transcode_to_16<UseN>(
                    detail::tag_t<u.format_tag>{}, u.first, u.last, n, out);
                return {u.repack(unpacked.in), unpacked.out};
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_16_dispatch<UseN, Ptr, OutIter, true, false>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_16<UseN>(
                    detail::tag_t<format::utf8>{}, p, null_sentinel, n, out);
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_16_dispatch<UseN, Ptr, OutIter, false, true>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_16<UseN>(
                    detail::tag_t<format::utf32>{}, p, null_sentinel, n, out);
            }
        };

        template<
            bool UseN,
            typename Range,
            typename OutIter,
            bool CharPtr = detail::is_char_ptr_v<Range>,
            bool _16Ptr = detail::is_16_ptr_v<Range>>
        struct transcode_to_32_dispatch
        {
            static constexpr auto
            call(Range && r, std::ptrdiff_t n, OutIter out)
                -> transcode_result<decltype(detail::begin(r)), OutIter>
            {
                auto const u = text::unpack_iterator_and_sentinel(
                    detail::begin(r), detail::end(r));
                auto unpacked = detail::transcode_to_32<UseN>(
                    detail::tag_t<u.format_tag>{}, u.first, u.last, n, out);
                return {u.repack(unpacked.in), unpacked.out};
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_32_dispatch<UseN, Ptr, OutIter, true, false>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_32<UseN>(
                    detail::tag_t<format::utf8>{}, p, null_sentinel, n, out);
            }
        };

        template<bool UseN, typename Ptr, typename OutIter>
        struct transcode_to_32_dispatch<UseN, Ptr, OutIter, false, true>
        {
            static constexpr auto
            call(Ptr p, std::ptrdiff_t n, OutIter out)
            {
                return detail::transcode_to_32<UseN>(
                    detail::tag_t<format::utf16>{}, p, null_sentinel, n, out);
            }
        };
    }

    template<typename Iter, typename Sentinel, typename OutIter>
    transcode_result<Iter, OutIter> transcode_to_utf8(
        Iter first, Sentinel last, OutIter out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_8<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename Range, typename OutIter>
    transcode_result<utf_range_like_iterator_t<Range>, OutIter>
    transcode_to_utf8(Range && r, OutIter out)
    {
        return dtl::transcode_to_8_dispatch<false, Range, OutIter>::call(
            r, -1, out);
    }

    template<typename Iter, typename Sentinel, typename OutIter>
    transcode_result<Iter, OutIter> transcode_to_utf16(
        Iter first, Sentinel last, OutIter out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_16<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename Range, typename OutIter>
    transcode_result<utf_range_like_iterator_t<Range>, OutIter>
    transcode_to_utf16(Range && r, OutIter out)
    {
        return dtl::transcode_to_16_dispatch<false, Range, OutIter>::call(
            r, -1, out);
    }

    template<typename Iter, typename Sentinel, typename OutIter>
    transcode_result<Iter, OutIter> transcode_to_utf32(
        Iter first, Sentinel last, OutIter out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_32<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename Range, typename OutIter>
    transcode_result<utf_range_like_iterator_t<Range>, OutIter>
    transcode_to_utf32(Range && r, OutIter out)
    {
        return dtl::transcode_to_32_dispatch<false, Range, OutIter>::call(
            r, -1, out);
    }

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    // -> utf8

    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<uint8_t> O>
        requires(
            utf16_code_unit<std::iter_value_t<I>> ||
            utf32_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf8(I first, S last, O out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_8<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename R, std::output_iterator<uint32_t> O>
        requires(utf16_input_range_like<R> || utf32_input_range_like<R>)
    transcode_result<dtl::uc_result_iterator<R>, O> transcode_to_utf8(
        R && r, O out)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
            return text::transcode_to_utf8(r, null_sentinel, out);
        } else {
            return text::transcode_to_utf8(
                std::ranges::begin(r), std::ranges::end(r), out);
        }
    }


    // -> utf16

    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<char16_t> O>
        requires(
            utf8_code_unit<std::iter_value_t<I>> ||
            utf32_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf16(I first, S last, O out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_16<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename R, std::output_iterator<uint32_t> O>
        requires(utf8_input_range_like<R> || utf32_input_range_like<R>)
    transcode_result<dtl::uc_result_iterator<R>, O> transcode_to_utf16(
        R && r, O out)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
            return text::transcode_to_utf16(r, null_sentinel, out);
        } else {
            return text::transcode_to_utf16(
                std::ranges::begin(r), std::ranges::end(r), out);
        }
    }


    // -> utf32

    template<
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::output_iterator<uint32_t> O>
        requires(
            utf8_code_unit<std::iter_value_t<I>> ||
            utf16_code_unit<std::iter_value_t<I>>)
    transcode_result<I, O> transcode_to_utf32(I first, S last, O out)
    {
        auto const r = text::unpack_iterator_and_sentinel(first, last);
        auto unpacked = detail::transcode_to_32<false>(
            detail::tag_t<r.format_tag>{}, r.first, r.last, -1, out);
        return {r.repack(unpacked.in), unpacked.out};
    }

    template<typename R, std::output_iterator<uint32_t> O>
        requires(utf8_input_range_like<R> || utf16_input_range_like<R>)
    transcode_result<dtl::uc_result_iterator<R>, O> transcode_to_utf32(
        R && r, O out)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
            return text::transcode_to_utf32(r, null_sentinel, out);
        } else {
            return text::transcode_to_utf32(
                std::ranges::begin(r), std::ranges::end(r), out);
        }
    }

}}}

#endif

#endif
