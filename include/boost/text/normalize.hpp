// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/detail/icu/normalize.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {

        template<typename CPIter, typename Sentinel>
        using utf8_range_expr = is_char_iter<decltype(
            detail::unpack_iterator_and_sentinel(
                std::declval<CPIter>(), std::declval<Sentinel>())
                .f_)>;

        template<typename CPIter, typename Sentinel>
        using utf8_fast_path =
            detected_or<std::false_type, utf8_range_expr, CPIter, Sentinel>;

        template<typename OutIter>
        struct norm_result
        {
            OutIter out_;
            bool normalized_;
        };

        template<
            nf Normalization,
            typename OutIter, // will be bool for the check-only case
            typename CPIter,
            typename Sentinel,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value,
            bool Composition =
                Normalization != nf::d && Normalization != nf::kd>
        struct norm_impl
        {
            // Primary template does decomposition.
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first_, Sentinel last_, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = boost::text::v1::as_utf16(first_, last_);
                auto first = r.begin();
                auto const last = r.end();

                int const chunk_size = 512;
                std::array<detail::icu::UChar, chunk_size> input;
                auto input_first = input.data();

                while (first != last) {
                    int n = 0;
                    auto input_last = input_first;
                    for (; first != last && n < chunk_size - 1;
                         ++first, ++input_last, ++n) {
                        *input_last = *first;
                    }
                    if (high_surrogate(*std::prev(input_last)) &&
                        first != last) {
                        *input_last++ = *first;
                        ++first;
                    }
                    detail::icu::ReorderingBuffer<Appender> buffer(
                        Normalization == nf::kd ? detail::icu::nfkc_norm()
                                                : detail::icu::nfc_norm(),
                        appender);
                    auto const input_new_first =
                        Normalization == nf::kd
                            ? detail::icu::nfkc_norm().decompose<do_writes>(
                                  input.data(), input_last, buffer)
                            : detail::icu::nfc_norm().decompose<do_writes>(
                                  input.data(), input_last, buffer);
                    if (!do_writes && input_new_first != input_last)
                        return norm_result<OutIter>{appender.out(), false};
                    input_first =
                        std::copy(input_new_first, input_last, input.data());
                }

                return norm_result<OutIter>{appender.out(), true};
            }
        };


        template<
            nf Normalization,
            typename OutIter,
            typename CPIter,
            typename Sentinel,
            bool UTF8>
        struct norm_impl<Normalization, OutIter, CPIter, Sentinel, UTF8, true>
        {
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first, Sentinel last, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = boost::text::v1::as_utf16(first, last);
                detail::icu::ReorderingBuffer<Appender> reorder_buffer(
                    (Normalization == nf::kc ? detail::icu::nfkc_norm()
                                             : detail::icu::nfc_norm()),
                    appender);
                auto const normalized =
                    (Normalization == nf::kc ? detail::icu::nfkc_norm()
                                             : detail::icu::nfc_norm())
                        .compose<Normalization == nf::fcc, do_writes>(
                            r.begin(), r.end(), reorder_buffer);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };

        template<
            nf Normalization,
            typename OutIter,
            typename CPIter,
            typename Sentinel>
        struct norm_impl<Normalization, OutIter, CPIter, Sentinel, true, true>
        {
            template<typename Appender>
            static norm_result<OutIter>
            call(CPIter first, Sentinel last, Appender appender)
            {
                constexpr bool do_writes = !std::is_same<OutIter, bool>::value;

                auto const r = boost::text::v1::as_utf8(first, last);
                auto const normalized =
                    (Normalization == nf::kc ? detail::icu::nfkc_norm()
                                             : detail::icu::nfc_norm())
                        .composeUTF8<Normalization == nf::fcc, do_writes>(
                            r.begin(), r.end(), appender);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value &&
                            Normalization != nf::d && Normalization != nf::kd>
        struct normalization_appender
        {
            using type = icu::utf16_to_utf32_appender<OutIter>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter>
        struct normalization_appender<
            Normalization,
            CPIter,
            Sentinel,
            OutIter,
            true>
        {
            using type = icu::utf8_to_utf32_appender<OutIter>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename OutIter>
        using normalization_appender_t = typename normalization_appender<
            Normalization,
            CPIter,
            Sentinel,
            OutIter>::type;

        template<typename OutIter>
        inline OutIter
        stream_safe_cp(int & nonstarters, uint32_t cp, OutIter out)
        {
            auto decomp = compatible_decompose(cp);
            uint32_t const degenerate_decomposition[1] = {cp};
            auto const decomposition_first =
                decomp.empty() ? std::begin(degenerate_decomposition)
                               : decomp.first_;
            auto const decomposition_last =
                decomp.empty() ? std::end(degenerate_decomposition)
                               : decomp.last_;
            auto const starter_first = std::find_if(
                decomposition_first, decomposition_last, [](auto cp) {
                    return detail::ccc(cp) == 0;
                });
            int const initial_nonstarters = starter_first - decomposition_first;
            if (30 < nonstarters + initial_nonstarters) {
                *out = 0x034f; // U+034F COMBINING GRAPHEME JOINER (CGJ)
                ++out;
                nonstarters = 0;
            }
            *out = cp;
            ++out;
            if (starter_first == decomposition_last) {
                nonstarters += decomposition_last - decomposition_first;
            } else {
                auto const starter_last_minus_one = find_if_backward(
                    starter_first, decomposition_last, [](auto cp) {
                        return detail::ccc(cp) == 0;
                    });
                auto const nonstarter_first =
                    starter_last_minus_one +
                    (starter_last_minus_one == decomposition_last ? 0 : 1);
                nonstarters += decomposition_last - nonstarter_first;
            }
            return out;
        }
    }

    /** Writes sequence `[first, last)` to `out`, ensuring Stream-Safe Text
        Format.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto stream_safe_copy(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        int nonstarters = 0;
        for (; first != last; ++first) {
            auto const cp = *first;
            out = detail::stream_safe_cp(nonstarters, cp, out);
        }
        return out;
    }

    /** Writes sequence `[first, last)` to `out`, ensuring Stream-Safe Text
        Format.

        \see https://unicode.org/reports/tr15/#Stream_Safe_Text_Format */
    template<typename CPRange, typename OutIter>
    inline OutIter stream_safe_copy(CPRange const & r, OutIter out)
    {
        return boost::text::v1::stream_safe_copy(
            std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form
        `Normalization` to `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<nf Normalization, typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        detail::
            normalization_appender_t<Normalization, CPIter, Sentinel, OutIter>
                appender(out);
        return detail::norm_impl<Normalization, OutIter, CPIter, Sentinel>::
            call(first, last, appender)
                .out_;
    }

    /** Writes sequence `r` in Unicode normalization form `Normalization` to
        `out`.

        \see https://unicode.org/notes/tn5 */
    template<nf Normalization, typename CPRange, typename OutIter>
    inline OutIter normalize(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize<Normalization>(
            std::begin(r), std::end(r), out);
    }

    /** Returns true iff the given sequence of code points is in Unicode
        normalization form `Normalization`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<nf Normalization, typename CPIter, typename Sentinel>
    auto normalized(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        detail::icu::null_appender appender;
        return detail::norm_impl<Normalization, bool, CPIter, Sentinel>::call(
                   first, last, appender)
            .normalized_;
    }

    /** Returns true iff the given sequence of code points is in Unicode
        normalization form `Normalization`. */
    template<nf Normalization, typename CPRange>
    bool normalized(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized<Normalization>(
            std::begin(r), std::end(r));
    }

}}}

#endif
