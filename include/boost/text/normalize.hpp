#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/detail/icu/normalize.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {

#if 0
        // NOTE: The logic in
        // http://www.unicode.org/reports/tr15/tr15-45.html#Detecting_Normalization_Forms
        // seems to indicate that if a supplementary code point is encountered
        // in normalized_quick_check(), then we should proceed as normal for
        // this iteration, but then do a double increment of the loop control
        // variable.  That looks wrong, so I'm leaving that out for now.
        bool supplemantary(uint32_t cp)
        {
            return 0x10000 <= cp && cp <= 0x10FFFF;
        }
#endif

        template<typename Iter, typename Sentinel, typename QuickCheckFunc>
        quick_check normalized_quick_check(
            Iter first, Sentinel last, QuickCheckFunc && quick_check_) noexcept
        {
            quick_check retval = quick_check::yes;
            int prev_ccc = 0;
            while (first != last) {
                auto const cp = *first;
#if 0
                // See note above.
                if (supplemantary(cp))
                    ++first;
#endif
                auto const check = quick_check_(cp);
                if (check == quick_check::no)
                    return quick_check::no;
                if (check == quick_check::maybe)
                    retval = quick_check::maybe;
                auto const ccc_ = detail::ccc(cp);
                if (ccc_ && ccc_ < prev_ccc)
                    return quick_check::no;
                prev_ccc = ccc_;
                ++first;
            }
            return retval;
        }


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

        // The "dispatch" logic here and below selects the proper underlying
        // implementation, which may operate on UTF-8 or UTF-16.

        enum norm_dispatch_constants {
            norm_nfc = false,
            norm_nfkc = true,
            norm_nfd = false,
            norm_nfkd = true,
            norm_check = false,
            norm_normalize = true,
            norm_fcc = true
        };

        // NFD/NFKD dispatch
        template<
            bool WriteToOut, // false: check norm, true: normalize
            typename OutIter,
            typename CPIter,
            typename Sentinel,
            typename Appender>
        static norm_result<OutIter> norm_nfd_impl(
            bool compatible, CPIter first_, Sentinel last_, Appender & appender)
        {
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
                if (high_surrogate(*std::prev(input_last)) && first != last) {
                    *input_last++ = *first;
                    ++first;
                }
                detail::icu::ReorderingBuffer<Appender> buffer(
                    compatible ? detail::icu::nfkc_norm()
                               : detail::icu::nfc_norm(),
                    appender);
                auto const input_new_first =
                    compatible ? detail::icu::nfkc_norm().decompose<WriteToOut>(
                                     input.data(), input_last, buffer)
                               : detail::icu::nfc_norm().decompose<WriteToOut>(
                                     input.data(), input_last, buffer);
                if (!WriteToOut && input_new_first != input_last)
                    return norm_result<OutIter>{appender.out(), false};
                input_first =
                    std::copy(input_new_first, input_last, input.data());
            }

            return norm_result<OutIter>{appender.out(), true};
        }


        template<bool WriteToOut, typename OutIter>
        using nfc_appender_t = typename std::conditional<
            WriteToOut,
            detail::icu::utf16_to_utf32_appender<OutIter>,
            detail::icu::null_appender>::type;

        // NFC/NFKC/FCC dispatch
        template<
            bool WriteToOut,     // false: check norm, true: normalize
            bool OnlyContiguous, // false: NFC, true: FCC
            typename OutIter,
            typename CPIter,
            typename Sentinel,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value>
        struct norm_nfc_impl
        {
            static norm_result<OutIter>
            call(bool compatible, CPIter first, Sentinel last, OutIter out)
            {
                auto const r = boost::text::v1::as_utf16(first, last);
                using appender_type = nfc_appender_t<WriteToOut, OutIter>;
                appender_type appender(out);
                detail::icu::ReorderingBuffer<appender_type> buffer(
                    (compatible ? detail::icu::nfkc_norm()
                                : detail::icu::nfc_norm()),
                    appender);
                auto const normalized =
                    (compatible ? detail::icu::nfkc_norm()
                                : detail::icu::nfc_norm())
                        .compose<OnlyContiguous, WriteToOut>(
                            r.begin(), r.end(), buffer);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };

        template<
            bool WriteToOut,
            bool OnlyContiguous,
            typename OutIter,
            typename CPIter,
            typename Sentinel>
        struct norm_nfc_impl<
            WriteToOut,
            OnlyContiguous,
            OutIter,
            CPIter,
            Sentinel,
            true>
        {
            static norm_result<OutIter>
            call(bool compatible, CPIter first, Sentinel last, OutIter out)
            {
                auto const r = boost::text::v1::as_utf8(first, last);
                typename std::conditional<
                    WriteToOut,
                    detail::icu::utf8_to_utf32_appender<OutIter>,
                    detail::icu::null_appender>::type appender(out);
                auto const normalized =
                    (compatible ? detail::icu::nfkc_norm()
                                : detail::icu::nfc_norm())
                        .composeUTF8<OnlyContiguous, WriteToOut>(
                            r.begin(), r.end(), appender);
                return norm_result<OutIter>{appender.out(), (bool)normalized};
            }
        };
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFD to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfd(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        detail::icu::utf16_to_utf32_appender<OutIter> appender(out);
        return detail::norm_nfd_impl<detail::norm_normalize, OutIter>(
                   detail::norm_nfd, first, last, appender)
            .out_;
    }

    /** Writes sequence `r` in Unicode normalization form NFD to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfd(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize_to_nfd(
            std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFKD to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfkd(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        detail::icu::utf16_to_utf32_appender<OutIter> appender(out);
        return detail::norm_nfd_impl<detail::norm_normalize, OutIter>(
                   detail::norm_nfkd, first, last, appender)
            .out_;
    }

    /** Writes sequence `r` in Unicode normalization form NFKD to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfkd(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize_to_nfkd(
            std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFC to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_normalize,
                   detail::norm_nfc,
                   OutIter,
                   CPIter,
                   Sentinel>::call(detail::norm_nfc, first, last, out)
            .out_;
    }

    /** Writes sequence `r` in Unicode normalization form NFC to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfc(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize_to_nfc(
            std::begin(r), std::end(r), out);
    }

    /** Writes sequence `[first, last)` in Unicode normalization form NFKC to
        `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_nfkc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_normalize,
                   detail::norm_nfc,
                   OutIter,
                   CPIter,
                   Sentinel>::call(detail::norm_nfkc, first, last, out)
            .out_;
    }

    /** Writes sequence `r` in Unicode normalization form NFKC to `out`. */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_nfkc(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize_to_nfkc(
            std::begin(r), std::end(r), out);
    }

    /** Returns true iff the given sequence of code points is normalized NFD.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfd(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        detail::icu::null_appender ignored;
        return detail::norm_nfd_impl<detail::norm_check, bool>(
                   detail::norm_nfd, first, last, ignored)
            .normalized_;
    }

    /** Returns true iff the given range of code points is normalized NFD. */
    template<typename CPRange>
    bool normalized_nfd(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized_nfd(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFKD.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfkd(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        detail::icu::null_appender ignored;
        return detail::norm_nfd_impl<detail::norm_check, bool>(
                   detail::norm_nfkd, first, last, ignored)
            .normalized_;
    }

    /** Returns true iff the given range of code points is normalized NFKD. */
    template<typename CPRange>
    bool normalized_nfkd(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized_nfkd(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFC.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfc(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_check,
                   detail::norm_nfc,
                   bool,
                   CPIter,
                   Sentinel>::call(detail::norm_nfc, first, last, bool())
            .normalized_;
    }

    /** Returns true iff the given range of code points is normalized NFC. */
    template<typename CPRange>
    bool normalized_nfc(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized_nfc(std::begin(r), std::end(r));
    }

    /** Returns true iff the given sequence of code points is normalized NFKC.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    auto normalized_nfkc(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_check,
                   detail::norm_nfc,
                   bool,
                   CPIter,
                   Sentinel>::call(detail::norm_nfkc, first, last, bool())
            .normalized_;
    }

    /** Returns true iff the given range of code points is normalized NFKC. */
    template<typename CPRange>
    bool normalized_nfkc(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized_nfkc(std::begin(r), std::end(r));
    }

    /** Writes sequence `[first, last)` in normalization form FCC to `out`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename OutIter>
    inline auto normalize_to_fcc(CPIter first, Sentinel last, OutIter out)
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_normalize,
                   detail::norm_fcc,
                   OutIter,
                   CPIter,
                   Sentinel>::call(detail::norm_nfc, first, last, out)
            .out_;
    }

    /** Writes sequence `r` in normalization form FCC to `out`

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename OutIter>
    inline OutIter normalize_to_fcc(CPRange const & r, OutIter out)
    {
        return boost::text::v1::normalize_to_fcc(
            std::begin(r), std::end(r), out);
    }

    /** Returns true iff the given sequence of code points is normalized FCC.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel>
    auto normalized_fcc(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::norm_nfc_impl<
                   detail::norm_check,
                   detail::norm_fcc,
                   bool,
                   CPIter,
                   Sentinel>::call(detail::norm_nfc, first, last, bool())
            .normalized_;
    }

    /** Returns true iff the given range of code points is normalized FCC. */
    template<typename CPRange>
    bool normalized_fcc(CPRange const & r) noexcept
    {
        return boost::text::v1::normalized_fcc(std::begin(r), std::end(r));
    }

}}}

#endif
