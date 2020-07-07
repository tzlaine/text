// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_NORMALIZE_STRING_HPP
#define BOOST_TEXT_NORMALIZE_STRING_HPP

#include <boost/text/normalize.hpp>
#include <boost/text/string.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/icu/normalize.hpp>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {
        namespace {
            constexpr char const * ncstr = nullptr;
        }

#if 0
        // NFC/NFKC/FCC dispatch
        template<
            bool WriteToOut,     // false: check norm, true: normalize
            bool OnlyContiguous, // false: NFC, true: FCC
            typename CPIter,
            typename Sentinel,
            bool UTF8 = is_detected<utf8_range_expr, CPIter, Sentinel>::value>
        struct norm_nfc_append_impl
        {
            template<typename String>
            static void
            call(bool compatible, CPIter first, Sentinel last, String & s)
            {
                auto const r = boost::text::v1::as_utf16(first, last);
                detail::icu::utf16_to_utf8_string_appender<String> appender(s);
                (compatible ? detail::icu::nfkc_norm()
                            : detail::icu::nfc_norm())
                    .compose<OnlyContiguous, WriteToOut>(
                        r.begin(), r.end(), appender);
            }
        };

        template<
            bool WriteToOut,
            bool OnlyContiguous,
            typename CPIter,
            typename Sentinel>
        struct norm_nfc_append_impl<
            WriteToOut,
            OnlyContiguous,
            CPIter,
            Sentinel,
            true>
        {
            template<typename String>
            static void
            call(bool compatible, CPIter first, Sentinel last, String & s)
            {
                auto const r = boost::text::v1::as_utf8(first, last);
                detail::icu::utf8_string_appender<String> appender(s);
                (compatible ? detail::icu::nfkc_norm()
                            : detail::icu::nfc_norm())
                    .composeUTF8<OnlyContiguous, WriteToOut>(
                        r.begin(), r.end(), appender);
            }
        };
#endif

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename String,
            bool UTF8 = utf8_fast_path<CPIter, Sentinel>::value &&
                            Normalization != nf::d && Normalization != nf::kd>
        struct normalization_string_appender
        {
            using type = icu::utf16_to_utf8_string_appender<String>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename String>
        struct normalization_string_appender<
            Normalization,
            CPIter,
            Sentinel,
            String,
            true>
        {
            using type = icu::utf8_string_appender<String>;
        };

        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename String>
        using normalization_string_appender_t =
            typename normalization_string_appender<
                Normalization,
                CPIter,
                Sentinel,
                String>::type;
    }

#if 0
    /** Appends sequence `[first, last)` in normalization form NFD to `s`, in
        UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename String>
    inline auto
    normalize_to_nfd_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::icu::utf16_to_utf8_string_appender<String> appender(s);
        detail::norm_impl<nf::d, decltype(s.begin()), CPIter, Sentinel>(
            first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form NFD to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfd_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_to_nfd_append_utf8(
            std::begin(r), std::end(r), s);
    }

    /** Appends sequence `[first, last)` in normalization form NFKD to `s`, in
        UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename String>
    inline auto
    normalize_to_nfkd_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::icu::utf16_to_utf8_string_appender<String> appender(s);
        detail::norm_impl<nf::kd, decltype(s.begin()), CPIter, Sentinel>(
            first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form NFKD to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfkd_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_to_nfkd_append_utf8(
            std::begin(r), std::end(r), s);
    }

    /** Appends sequence `[first, last)` in normalization form NFC to `s`, in
        UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename String>
    inline auto
    normalize_to_nfc_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::icu::utf16_to_utf8_string_appender<String> appender(s);
        detail::norm_impl<nf::c, decltype(s.begin()), CPIter, Sentinel>(
            first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form NFC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfc_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_to_nfc_append_utf8(
            std::begin(r), std::end(r), s);
    }

    /** Appends sequence `[first, last)` in normalization form NFKC to `s`, in
        UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename String>
    inline auto
    normalize_to_nfkc_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::icu::utf16_to_utf8_string_appender<String> appender(s);
        detail::norm_impl<nf::kc, decltype(s.begin()), CPIter, Sentinel>(
            first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form NFKC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfkc_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_to_nfkc_append_utf8(
            std::begin(r), std::end(r), s);
    }

    /** Appends sequence `[first, last)` in normalization form FCC to `s`, in
        UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \see https://unicode.org/notes/tn5 */
    template<typename CPIter, typename Sentinel, typename String>
    inline auto
    normalize_to_fcc_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::icu::utf16_to_utf8_string_appender<String> appender(s);
        detail::norm_impl<nf::fcc, decltype(s.begin()), CPIter, Sentinel>(
            first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form FCC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_fcc_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_to_fcc_append_utf8(
            std::begin(r), std::end(r), s);
    }
#endif

    /** Appends sequence `[first, last)` in normalization form `Normalization`
        to `s`, in UTF-8 encoding.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<
        nf Normalization,
        typename CPIter,
        typename Sentinel,
        typename String>
    inline auto
    normalize_append_utf8(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(s.insert(s.end(), detail::ncstr, detail::ncstr), s),
            CPIter>
    {
        detail::normalization_string_appender_t<
            Normalization,
            CPIter,
            Sentinel,
            String>
            appender(s);
        detail::
            norm_impl<Normalization, decltype(s.begin()), CPIter, Sentinel>::
                call(first, last, appender);
        return s;
    }

    /** Appends sequence `r` in normalization form `Normalizaation` to `s`, in
        UTF-8 encoding. */
    template<nf Normalization, typename CPRange, typename String>
    inline String & normalize_append_utf8(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_append_utf8<Normalization>(
            std::begin(r), std::end(r), s);
    }

    /** Puts the contents of `s` in Unicode normalization form `Normalize`. */
    template<nf Normalization>
    void normalize(string & s)
    {
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<Normalization>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size() / 2 * 3);

        boost::text::v1::normalize_append_utf8<Normalization>(
            r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

#if 0
    /** Puts the contents of `s` in Unicode normalization form NFD.
        Normalization is not performed if `s` passes a normalization
        quick-check. */
    inline void normalize_to_nfd(string & s)
    {
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<nf::d>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size() / 2 * 3);

        boost::text::v1::normalize_append_utf8<nf::d>(r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of `s` in Unicode normalization form NFKD.
        Normalization is not performed if `s` passes a normalization
        quick-check. */
    inline void normalize_to_nfkd(string & s)
    {
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<nf::kd>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size() / 2 * 3);

        boost::text::v1::normalize_append_utf8<nf::kd>(
            r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of `s` in Unicode normalization form NFC.
        Normalization is not performed if `s` passes a normalization
        quick-check. */
    inline void normalize_to_nfc(string & s)
    {
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<nf::c>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(temp.size());

        boost::text::v1::normalize_append_utf8<nf::c>(r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of `s` in Unicode normalization form NFKC.
        Normalization is not performed if `s` passes a normalization
        quick-check. */
    inline void normalize_to_nfkc(string & s)
    {
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<nf::kc>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        boost::text::v1::normalize_append_utf8<nf::kc>(
            r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of `s` in normalization form FCC. */
    inline void normalize_to_fcc(string & s)
    {
        // http://www.unicode.org/notes/tn5/#FCC
        auto const r = as_utf32(s);

        string temp;
        temp.reserve(s.size());

        boost::text::v1::normalize_append_utf8<nf::fcc>(
            r.begin(), r.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }
#endif

}}}

#endif
