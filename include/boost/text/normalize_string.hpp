#ifndef BOOST_TEXT_NORMALIZE_STRING_HPP
#define BOOST_TEXT_NORMALIZE_STRING_HPP

#include <boost/text/normalize.hpp>
#include <boost/text/string.hpp>
#include <boost/text/detail/icu/normalize.hpp>


namespace boost { namespace text {

    namespace detail {
        namespace {
            constexpr char const * ncstr = nullptr;
        }

        struct utf8_norm_noop
        {
            template<typename CharIter, typename Sentinel>
            void operator()(CharIter first, Sentinel last)
            {}
        };

        template<typename String>
        struct utf8_norm_nfc
        {
            utf8_norm_nfc(String & s) : s_(&s) {}
            template<typename CharIter, typename Sentinel>
            void operator()(CharIter first, Sentinel last)
            {
                detail::icu::utf8_normalize_to_nfc_append(first, last, *s_);
            }
            String * s_;
        };

        template<typename String>
        struct utf8_norm_nfkc
        {
            utf8_norm_nfkc(String & s) : s_(&s) {}
            template<typename CharIter, typename Sentinel>
            void operator()(CharIter first, Sentinel last)
            {
                detail::icu::utf8_normalize_to_nfkc_append(first, last, *s_);
            }
            String * s_;
        };

        template<typename String>
        struct utf8_norm_fcc
        {
            utf8_norm_fcc(String & s) : s_(&s) {}
            template<typename CharIter, typename Sentinel>
            void operator()(CharIter first, Sentinel last)
            {
                detail::icu::utf8_normalize_to_fcc_append(first, last, *s_);
            }
            String * s_;
        };

        enum norm : bool {
            norm_utf8_fast_path = true,
            norm_no_utf8_fast_path = false,
            norm_compose = true,
            norm_decompose = false
        };

        template<
            typename Iter,
            typename Sentinel,
            bool Compose,
            bool FastUTF8 = icu_utf8_in_fast_path<Iter, Sentinel>::value>
        struct dispatch_normalize_append
        {
            template<typename Utf16NormFunc, typename Utf8ComposeFunc>
            static void call(
                Iter first,
                Sentinel last,
                Utf16NormFunc && norm_func,
                Utf8ComposeFunc &&)
            {
                // TODO: Compose CCC=0-delimited UTF16 chunks.
                norm_func(first, last);
            }
        };

        template<typename Iter, typename Sentinel>
        struct dispatch_normalize_append<
            Iter,
            Sentinel,
            norm_decompose,
            norm_no_utf8_fast_path>
        {
            template<typename Utf16NormFunc, typename Utf8ComposeFunc>
            static void call(
                Iter first,
                Sentinel last,
                Utf16NormFunc && norm_func,
                Utf8ComposeFunc &&)
            {
                norm_func(first, last);
            }
        };

        template<typename Iter, typename Sentinel>
        struct dispatch_normalize_append<
            Iter,
            Sentinel,
            norm_compose,
            norm_utf8_fast_path>
        {
            template<typename Utf16NormFunc, typename Utf8ComposeFunc>
            static void call(
                Iter first,
                Sentinel last,
                Utf16NormFunc &&,
                Utf8ComposeFunc && norm_func)
            {
                auto const r = make_utf8_range(first, last);
                norm_func(r.begin(), r.end());
            }
        };
    }

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
        auto utf16_norm = [&s](CPIter first, Sentinel last) {
            detail::icu::UnicodeString output;

            int const chunk_size = 512;
            detail::icu::UnicodeString input(chunk_size * 2);
            auto input_first = input.data();

            while (first != last) {
                auto chunk_last = std::next(first, chunk_size);
                auto input_last =
                    transcode_utf_32_to_16(first, chunk_last, input_first);
                first = chunk_last;
                auto const input_new_first =
                    detail::icu::utf16_normalize_to_nfd_append(
                        input.data(), input_last, output);
                input_first =
                    std::copy(input_new_first, input_last, input.data());

                transcode_utf_16_to_8(output, std::inserter(s, s.end()));
                output.clear();
            }
        };
        detail::dispatch_normalize_append<
            CPIter,
            Sentinel,
            detail::norm_decompose,
            false>::call(first, last, utf16_norm, detail::utf8_norm_noop{});

        return s;
    }

    /** Appends sequence `r` in normalization form NFD to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfd_append_utf8(CPRange const & r, String & s)
    {
        return normalize_to_nfd_append_utf8(std::begin(r), std::end(r), s);
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
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKDInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            detail::icu_normalize_append<CPIter, Sentinel, String>::call(
                *norm, first, last, s);
            return s;
        }
#endif
        detail::normalize_to_decomposed(
            first,
            last,
            utf_32_to_8_inserter(s, s.end()),
            [](uint32_t cp) { return detail::compatible_decompose(cp); });

        return s;
    }

    /** Appends sequence `r` in normalization form NFKD to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfkd_append_utf8(CPRange const & r, String & s)
    {
        return normalize_to_nfkd_append_utf8(std::begin(r), std::end(r), s);
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
        auto utf16_norm = [&s](CPIter first, Sentinel last) {
            normalize_to_nfc(first, last, utf_32_to_8_inserter(s, s.end()));
        };
        detail::
            dispatch_normalize_append<CPIter, Sentinel, detail::norm_compose>::
                call(first, last, utf16_norm, detail::utf8_norm_nfc<String>(s));
        return s;
    }

    /** Appends sequence `r` in normalization form NFC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfc_append_utf8(CPRange const & r, String & s)
    {
        return normalize_to_nfc_append_utf8(std::begin(r), std::end(r), s);
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
#if 0 // TODO: Badly broken!
        auto utf16_norm = [&s](CPIter first, Sentinel last) {
            normalize_to_nfkc(first, last, utf_32_to_8_inserter(s, s.end()));
        };
        detail::
            dispatch_normalize_append<CPIter, Sentinel, detail::norm_compose>::
                call(
                    first, last, utf16_norm, detail::utf8_norm_nfkc<String>(s));
#else
        detail::normalize_to_composed<false>(
            first,
            last,
            utf_32_to_8_inserter(s, s.end()),
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });
#endif
        return s;
    }

    /** Appends sequence `r` in normalization form NFKC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_nfkc_append_utf8(CPRange const & r, String & s)
    {
        return normalize_to_nfkc_append_utf8(std::begin(r), std::end(r), s);
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
        auto utf16_norm = [&s](CPIter first, Sentinel last) {
            normalize_to_fcc(first, last, utf_32_to_8_inserter(s, s.end()));
        };
        detail::
            dispatch_normalize_append<CPIter, Sentinel, detail::norm_compose>::
                call(first, last, utf16_norm, detail::utf8_norm_fcc<String>(s));
        return s;
    }

    /** Appends sequence `r` in normalization form FCC to `s`, in UTF-8
        encoding.

        \see https://unicode.org/notes/tn5 */
    template<typename CPRange, typename String>
    inline String & normalize_to_fcc_append_utf8(CPRange const & r, String & s)
    {
        return normalize_to_fcc_append_utf8(std::begin(r), std::end(r), s);
    }

    /** Puts the contents of `s` in Unicode normalization form NFD.
        Normalization is not performed if `s` passes a normalization
        quick-check. */
    inline void normalize_to_nfd(string & s)
    {
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size() / 2 * 3);

        normalize_to_nfd_append_utf8(as_utf32.begin(), as_utf32.end(), temp);

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
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkd_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size() / 2 * 3);

        normalize_to_nfkd_append_utf8(as_utf32.begin(), as_utf32.end(), temp);

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
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(temp.size());

        normalize_to_nfc_append_utf8(as_utf32.begin(), as_utf32.end(), temp);

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
        utf32_range as_utf32(s);
        if (detail::normalized_quick_check(
                as_utf32.begin(), as_utf32.end(), [](uint32_t cp) {
                    return detail::quick_check_nfkc_code_point(cp);
                }) == detail::quick_check::yes) {
            return;
        }

        string temp;
        temp.reserve(s.size());

        normalize_to_nfkc_append_utf8(as_utf32.begin(), as_utf32.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    /** Puts the contents of `s` in normalization form FCC. */
    inline void normalize_to_fcc(string & s)
    {
        // http://www.unicode.org/notes/tn5/#FCC
        utf32_range as_utf32(s);

        string temp;
        temp.reserve(s.size());

        normalize_to_fcc_append_utf8(as_utf32.begin(), as_utf32.end(), temp);

        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

}}

#endif
