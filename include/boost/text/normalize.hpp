#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/detail/normalization_data.hpp>
#include <boost/text/detail/icu/normalize.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {

        template<typename Iter, std::size_t Capacity>
        void order_canonically(
            Iter first,
            Iter last,
            container::static_vector<int, Capacity> & cccs) noexcept
        {
            BOOST_ASSERT(first != last);

            std::transform(first, last, cccs.begin(), ccc);

            --last;
            while (first != last) {
                auto it = first;
                auto new_last = first;
                auto ccc_it = cccs.begin();
                while (it != last) {
                    auto next = std::next(it);
                    auto ccc_next = std::next(ccc_it);
                    auto const ccc_a = *ccc_it;
                    auto const ccc_b = *ccc_next;
                    if (0 < ccc_b && ccc_b < ccc_a) {
                        std::iter_swap(it, next);
                        std::iter_swap(ccc_it, ccc_next);
                        new_last = it;
                    }
                    ++it;
                    ++ccc_it;
                }
                last = new_last;
            }
        }

        template<std::size_t Capacity, typename FlushFunc>
        bool flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc & flush)
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                detail::order_canonically(buffer.begin(), buffer.end(), cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<
            typename Iter,
            typename Sentinel,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename FlushFunc>
        bool normalize_to_decomposed_impl(
            Iter first,
            Sentinel last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            FlushFunc && flush)
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                if (!detail::ccc(decomp.storage_[0])) {
                    if (!detail::flush_buffer(buffer, flush))
                        return false;
                }
                buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                ++first;
            }
            if (!detail::flush_buffer(buffer, flush))
                return false;
            return true;
        }

        template<
            typename Iter,
            typename Sentinel,
            typename OutIter,
            typename DecomposeFunc>
        OutIter normalize_to_decomposed(
            Iter first, Sentinel last, OutIter out, DecomposeFunc && decompose)
        {
            container::static_vector<uint32_t, 64> buffer;
            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            detail::normalize_to_decomposed_impl(
                first,
                last,
                buffer,
                decompose,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(first, last, out);
                    return true;
                });

            return out;
        }

        inline constexpr bool hangul_l(uint32_t cp) noexcept
        {
            return 0x1100 <= cp && cp <= 0x1112;
        }
        inline constexpr bool hangul_v(uint32_t cp) noexcept
        {
            return 0x1161 <= cp && cp <= 0x1175;
        }
        inline constexpr bool hangul_t(uint32_t cp) noexcept
        {
            return 0x11a8 <= cp && cp <= 0x11c2;
        }

        template<bool DisallowDiscontiguous, std::size_t Capacity>
        void compose(
            container::static_vector<uint32_t, Capacity> & buffer,
            container::static_vector<int, Capacity> & cccs)
        {
            BOOST_ASSERT(buffer.size() == cccs.size());
            BOOST_ASSERT(2 <= buffer.size());

            auto starter_it = buffer.begin();
            auto it = std::next(buffer.begin());
            auto ccc_it = std::next(cccs.begin());
            while (it != buffer.end()) {
                // Hangul composition as described in Unicode 11.0 Section 3.12.
                auto const hangul_cp0 = *starter_it;
                auto const hangul_cp1 = *it;
                if (it == starter_it + 1 && detail::hangul_l(hangul_cp0) &&
                    detail::hangul_v(hangul_cp1)) {
                    auto const cp2_it = it + 1;
                    auto const hangul_cp2 =
                        cp2_it == buffer.end() ? 0 : *cp2_it;
                    if (detail::hangul_t(hangul_cp2)) {
                        *starter_it = detail::compose_hangul(
                            hangul_cp0, hangul_cp1, hangul_cp2);
                        buffer.erase(it, it + 2);
                        cccs.erase(ccc_it, ccc_it + 2);
                    } else {
                        *starter_it =
                            detail::compose_hangul(hangul_cp0, hangul_cp1);
                        buffer.erase(it, it + 1);
                        cccs.erase(ccc_it, ccc_it + 1);
                    }
                } else {
                    auto const prev_ccc = *std::prev(ccc_it);
                    auto const ccc = *ccc_it;
                    uint32_t composition = 0;
                    if ((it == starter_it + 1 ||
                         (!DisallowDiscontiguous &&
                          (prev_ccc != 0 && prev_ccc < ccc))) &&
                        (composition =
                             detail::compose_unblocked(*starter_it, *it))) {
                        *starter_it = composition;
                        buffer.erase(it);
                        cccs.erase(ccc_it);
                    } else {
                        ++it;
                        ++ccc_it;
                        if (it == buffer.end() &&
                            starter_it < buffer.end() - 2) {
                            ++starter_it;
                            it = std::next(starter_it);
                            ccc_it = cccs.begin() + (it - buffer.begin());
                        }
                    }
                }
            }
        }

        template<
            bool DisallowDiscontiguous,
            std::size_t Capacity,
            typename FlushFunc>
        bool compose_and_flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer,
            FlushFunc &&
                flush) noexcept(noexcept(flush(buffer.begin(), buffer.end())))
        {
            container::static_vector<int, Capacity> cccs(buffer.size());
            if (!buffer.empty())
                detail::order_canonically(buffer.begin(), buffer.end(), cccs);
            if (2 <= buffer.size())
                detail::compose<DisallowDiscontiguous>(buffer, cccs);
            if (!flush(buffer.begin(), buffer.end()))
                return false;
            buffer.clear();
            return true;
        }

        template<std::size_t Capacity>
        bool hangul_final_v(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return !buffer.empty() && detail::hangul_l(buffer.back()) &&
                   detail::hangul_v(cp);
        }

        template<std::size_t Capacity>
        bool hangul_final_t(
            container::static_vector<uint32_t, Capacity> & buffer,
            uint32_t cp) noexcept
        {
            return 2 <= buffer.size() &&
                   detail::hangul_l(buffer[buffer.size() - 2]) &&
                   detail::hangul_v(buffer.back()) && detail::hangul_t(cp);
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            typename Sentinel,
            std::size_t Capacity,
            typename DecomposeFunc,
            typename QuickCheckFunc,
            typename FlushFunc>
        bool normalize_to_composed_impl(
            Iter first,
            Sentinel last,
            container::static_vector<uint32_t, Capacity> & buffer,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_,
            FlushFunc && flush) noexcept
        {
            while (first != last) {
                auto const decomp = decompose(*first);
                auto const it = std::find_if(
                    decomp.begin(), decomp.end(), [&quick_check_](uint32_t cp) {
                        return !detail::ccc(cp) &&
                               quick_check_(cp) == quick_check::yes;
                    });
                if (it != decomp.end() && !detail::hangul_final_v(buffer, *it) &&
                    !detail::hangul_final_t(buffer, *it)) {
                    buffer.insert(buffer.end(), decomp.begin(), it);
                    if (!detail::compose_and_flush_buffer<
                            DisallowDiscontiguous>(buffer, flush)) {
                        return false;
                    }
                    buffer.insert(buffer.end(), it, decomp.end());
                } else {
                    buffer.insert(buffer.end(), decomp.begin(), decomp.end());
                }
                ++first;
            }
            if (!detail::compose_and_flush_buffer<DisallowDiscontiguous>(
                    buffer, flush)) {
                return false;
            }
            return true;
        }

        template<
            bool DisallowDiscontiguous,
            typename Iter,
            typename Sentinel,
            typename OutIter,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        OutIter normalize_to_composed(
            Iter first,
            Sentinel last,
            OutIter out,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_)
        {
            container::static_vector<uint32_t, 64> buffer;

            using buffer_iterator =
                container::static_vector<uint32_t, 64>::iterator;

            detail::normalize_to_composed_impl<DisallowDiscontiguous>(
                first,
                last,
                buffer,
                decompose,
                quick_check_,
                [&out](buffer_iterator first, buffer_iterator last) {
                    out = std::copy(first, last, out);
                    return true;
                });

            return out;
        }

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

        template<
            typename Iter,
            typename Sentinel,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        bool normalized_decomposed(
            Iter first,
            Sentinel last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                detail::normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return detail::normalize_to_decomposed_impl(
                    first,
                    last,
                    buffer,
                    decompose,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }

        template<
            typename Iter,
            typename Sentinel,
            typename DecomposeFunc,
            typename QuickCheckFunc>
        bool normalized_composed(
            Iter first,
            Sentinel last,
            DecomposeFunc && decompose,
            QuickCheckFunc && quick_check_) noexcept
        {
            auto const check =
                detail::normalized_quick_check(first, last, quick_check_);
            if (check == quick_check::maybe) {
                container::static_vector<uint32_t, 64> buffer;
                using buffer_iterator =
                    container::static_vector<uint32_t, 64>::iterator;
                return normalize_to_composed_impl<false>(
                    first,
                    last,
                    buffer,
                    decompose,
                    quick_check_,
                    [&first, last](
                        buffer_iterator buffer_first,
                        buffer_iterator buffer_last) {
                        while (first != last && buffer_first != buffer_last) {
                            if (*first++ != *buffer_first++)
                                return false;
                        }
                        return true;
                    });
            }
            return check == quick_check::yes;
        }


        template<typename CharIter, typename Sentinel = CharIter>
        struct utf8_range
        {
            CharIter begin() const noexcept { return first_; }
            Sentinel end() const noexcept { return last_; }

            CharIter first_;
            Sentinel last_;
        };

        template<typename Iter>
        typename std::enable_if<is_char_iter<Iter>::value, utf8_range<Iter>>::
            type
            make_utf8_range(Iter first, Iter last)
        {
            return utf8_range<Iter>{first, last};
        }
        template<typename Iter, typename Sentinel>
        typename std::enable_if<
            is_char_iter<Iter>::value,
            utf8_range<Iter, Sentinel>>::type
        make_utf8_range(Iter first, Sentinel last)
        {
            return utf8_range<Iter, Sentinel>{first, last};
        }
        template<typename Iter>
        auto make_utf8_range(
            utf_8_to_32_iterator<Iter> first, utf_8_to_32_iterator<Iter> last)
            -> utf8_range<decltype(first.base())>
        {
            return utf8_range<decltype(first.base())>{first.base(),
                                                      last.base()};
        }
        template<typename Iter, typename Sentinel>
        auto make_utf8_range(
            utf_8_to_32_iterator<Iter, Sentinel> first, Sentinel last)
            -> utf8_range<decltype(first.base()), Sentinel>
        {
            return utf8_range<decltype(first.base()), Sentinel>{first.base(),
                                                                last};
        }

        template<typename CPIter, typename Sentinel>
        using utf8_range_expr = decltype(detail::make_utf8_range(
            std::declval<CPIter>(), std::declval<Sentinel>()));

        template<typename CharIter, typename Sentinel = CharIter>
        struct utf16_range
        {
            CharIter begin() const noexcept { return first_; }
            Sentinel end() const noexcept { return last_; }

            CharIter first_;
            Sentinel last_;
        };

        template<typename Iter>
        typename std::enable_if<is_16_iter<Iter>::value, utf16_range<Iter>>::
            type
            make_utf16_range(Iter first, Iter last)
        {
            return utf16_range<Iter>{first, last};
        }
        template<typename Iter, typename Sentinel>
        typename std::enable_if<
            is_16_iter<Iter>::value,
            utf16_range<Iter, Sentinel>>::type
        make_utf16_range(Iter first, Sentinel last)
        {
            return utf16_range<Iter, Sentinel>{first, last};
        }
        template<typename Iter>
        auto make_utf16_range(
            utf_16_to_32_iterator<Iter> first, utf_16_to_32_iterator<Iter> last)
            -> utf16_range<decltype(first.base())>
        {
            return utf16_range<decltype(first.base())>{first.base(),
                                                       last.base()};
        }
        template<typename Iter, typename Sentinel>
        auto make_utf16_range(
            utf_16_to_32_iterator<Iter, Sentinel> first, Sentinel last)
            -> utf16_range<decltype(first.base()), Sentinel>
        {
            return utf16_range<decltype(first.base()), Sentinel>{first.base(),
                                                                 last};
        }

        template<typename CPIter, typename Sentinel>
        using utf16_range_expr = decltype(detail::make_utf16_range(
            std::declval<CPIter>(), std::declval<Sentinel>()));

        template<typename CPIter, typename Sentinel>
        struct utf32_to_utf16_range
        {
            using iterator = utf_32_to_16_iterator<CPIter, Sentinel>;

            utf32_to_utf16_range(CPIter first, Sentinel last) :
                first_(first, first, last),
                last_(last)
            {}

            iterator begin() const noexcept { return first_; }
            Sentinel end() const noexcept { return last_; }

            iterator first_;
            Sentinel last_;
        };

        template<typename CPIter>
        struct utf32_to_utf16_range<CPIter, CPIter>
        {
            using iterator = utf_32_to_16_iterator<CPIter>;

            utf32_to_utf16_range(CPIter first, CPIter last) :
                first_(first, first, last),
                last_(first, last, last)
            {}

            iterator begin() const noexcept { return first_; }
            iterator end() const noexcept { return last_; }

            iterator first_;
            iterator last_;
        };

        template<typename CPIter, typename Sentinel>
        typename std::enable_if<
            is_cp_iter<CPIter>::value,
            utf32_to_utf16_range<CPIter, Sentinel>>::type
        make_utf32_to_utf16_range(CPIter first, Sentinel last)
        {
            return utf32_to_utf16_range<CPIter, Sentinel>{first, last};
        }


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

        template<
            typename CPIter,
            typename Sentinel,
            bool UTF16 = is_detected<utf16_range_expr, CPIter, Sentinel>::value>
        struct make_norm_16_range_impl
        {
            static auto call(CPIter first, Sentinel last)
                -> decltype(detail::make_utf32_to_utf16_range(first, last))
            {
                return detail::make_utf32_to_utf16_range(first, last);
            }
        };
        template<typename CPIter, typename Sentinel>
        struct make_norm_16_range_impl<CPIter, Sentinel, true>
        {
            static auto call(CPIter first, Sentinel last)
                -> decltype(detail::make_utf16_range(first, last))
            {
                return detail::make_utf16_range(first, last);
            }
        };
        template<typename CPIter, typename Sentinel>
        auto make_norm_16_range(CPIter first, Sentinel last)
            -> decltype(detail::make_norm_16_range_impl<CPIter, Sentinel>::call(
                first, last))
        {
            return detail::make_norm_16_range_impl<CPIter, Sentinel>::call(
                first, last);
        }

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
            auto const r = detail::make_norm_16_range(first_, last_);
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
            bool UTF8 = is_detected<utf8_range_expr, CPIter, Sentinel>::value>
        struct norm_nfc_impl
        {
            static norm_result<OutIter>
            call(bool compatible, CPIter first, Sentinel last, OutIter out)
            {
                auto const r = detail::make_norm_16_range(first, last);
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
                auto const r = detail::make_utf8_range(first, last);
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
