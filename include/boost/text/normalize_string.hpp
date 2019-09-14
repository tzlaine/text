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

#if BOOST_TEXT_HAS_ICU
        template<typename Container>
        struct end_insert_sink : U_NAMESPACE_QUALIFIER ByteSink
        {
            explicit end_insert_sink(Container & c) : c_(&c) {}

            end_insert_sink(end_insert_sink const & other) : c_(other.c_) {}
            end_insert_sink & operator=(end_insert_sink const & other)
            {
                c_ = other.c_;
            }

            virtual void Append(char const * bytes, int32_t n) override
            {
                c_->insert(c_->end(), bytes, bytes + n);
            }

        private:
            Container * c_;
        };

        template<
            typename Iter,
            typename Sentinel,
            typename String,
            bool FastUTF8 = icu_utf8_in_fast_path<Iter, Sentinel>::value>
        struct icu_normalize_append
        {
            static void call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last,
                String & s)
            {
                auto out = utf_32_to_8_inserter(s, s.end());
                icu_normalize<Iter, Sentinel, decltype(out), false>::call(
                    norm, first, last, out);
            }
        };

        template<typename Iter, typename Sentinel, typename String>
        struct icu_normalize_append<Iter, Sentinel, String, true>
        {
            static void call(
                U_NAMESPACE_QUALIFIER Normalizer2 const & norm,
                Iter first,
                Sentinel last,
                String & s)
            {
                UErrorCode ec = U_ZERO_ERROR;
                end_insert_sink<String> sink(s);
                norm.normalizeUTF8(
                    0,
                    detail::make_string_piece(first, last),
                    sink,
                    nullptr,
                    ec);
                BOOST_ASSERT(U_SUCCESS(ec));
            }
        };
#else
        template<
            typename Iter,
            typename Sentinel,
            bool FastUTF8 = icu_utf8_in_fast_path<Iter, Sentinel>::value>
        struct dispatch_normalize_append
        {
            template<typename Utf16NormFunc, typename Utf8NormFunc>
            static void call(
                Iter first,
                Sentinel last,
                Utf16NormFunc && norm_func,
                Utf8NormFunc &&)
            {
                // TODO: Normalize UTF16 chunks.
                norm_func(first, last);
            }
        };

        template<typename Iter, typename Sentinel>
        struct dispatch_normalize_append<Iter, Sentinel, true>
        {
            template<typename Utf16NormFunc, typename Utf8NormFunc>
            static void call(
                Iter first,
                Sentinel last,
                Utf16NormFunc &&,
                Utf8NormFunc && norm_func)
            {
                auto const sv = make_string_view(first, last);
                norm_func(sv.begin(), sv.end());
            }
        };
#endif
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
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFDInstance(ec);
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
            [](uint32_t cp) { return detail::canonical_decompose(cp); });

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
        auto utf8_norm = [&s](char const * first, char const * last) {
            detail::icu::utf8_normalize_to_nfc_append(first, last, s);
        };
        detail::dispatch_normalize_append<CPIter, Sentinel>::call(
            first, last, utf16_norm, utf8_norm);
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
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getNFKCInstance(ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            detail::icu_normalize_append<CPIter, Sentinel, String>::call(
                *norm, first, last, s);
            return s;
        }
#endif
        detail::normalize_to_composed<false>(
            first,
            last,
            utf_32_to_8_inserter(s, s.end()),
            [](uint32_t cp) { return detail::compatible_decompose(cp); },
            [](uint32_t cp) {
                return detail::quick_check_nfkc_code_point(cp);
            });

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
#if BOOST_TEXT_HAS_ICU
        if (BOOST_TEXT_USE_ICU) {
            UErrorCode ec = U_ZERO_ERROR;
            U_NAMESPACE_QUALIFIER Normalizer2 const * const norm =
                U_NAMESPACE_QUALIFIER Normalizer2::getInstance(
                    nullptr, "nfc", UNORM2_COMPOSE_CONTIGUOUS, ec);
            BOOST_ASSERT(U_SUCCESS(ec));
            detail::icu_normalize_append<CPIter, Sentinel, String>::call(
                *norm, first, last, s);
            return s;
        }
#endif
        detail::normalize_to_composed<true>(
            first,
            last,
            utf_32_to_8_inserter(s, s.end()),
            [](uint32_t cp) { return detail::canonical_decompose(cp); },
            [](uint32_t cp) { return detail::quick_check_nfc_code_point(cp); });

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
