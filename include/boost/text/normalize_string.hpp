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


namespace boost { namespace text { namespace detail {
    template<
        nf Normalization,
        typename String,
        typename CPIter,
        typename Sentinel,
        bool UTF8Input = utf8_fast_path<CPIter, Sentinel>::value &&
                             Normalization != nf::d && Normalization != nf::kd,
        bool UTF8Output = sizeof(*std::declval<String>().begin()) == 1>
    struct normalization_string_appender
    {
        using type = icu::utf16_string_appender<String>;
    };

    template<
        nf Normalization,
        typename String,
        typename CPIter,
        typename Sentinel>
    struct normalization_string_appender<
        Normalization,
        String,
        CPIter,
        Sentinel,
        false,
        true>
    {
        using type = icu::utf16_to_utf8_string_appender<String>;
    };

    template<
        nf Normalization,
        typename String,
        typename CPIter,
        typename Sentinel>
    struct normalization_string_appender<
        Normalization,
        String,
        CPIter,
        Sentinel,
        true,
        true>
    {
        using type = icu::utf8_string_appender<String>;
    };

    template<
        nf Normalization,
        typename String,
        typename CPIter,
        typename Sentinel>
    using normalization_string_appender_t =
        typename normalization_string_appender<
            Normalization,
            String,
            CPIter,
            Sentinel>::type;
}}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    namespace detail {
        template<
            nf Normalization,
            typename CPIter,
            typename Sentinel,
            typename String>
        inline void
        normalize_append_impl_impl(CPIter first, Sentinel last, String & s)
        {
            detail::normalization_string_appender_t<
                Normalization,
                String,
                CPIter,
                Sentinel>
                appender(s);
            detail::norm_impl<
                Normalization,
                decltype(s.begin()),
                CPIter,
                Sentinel>::call(first, last, appender);
        }

        template<
            nf Normalization,
            typename String,
            bool IntegralElement =
                std::is_integral<typename std::remove_reference<decltype(
                    *std::declval<String>().begin())>::type>::value,
            int ElementSize = sizeof(*std::declval<String>().begin())>
        struct normalize_append_impl
        {};

        template<nf Normalization, typename String>
        struct normalize_append_impl<Normalization, String, true, 1>
        {
            template<typename CPIter, typename Sentinel>
            static void call(CPIter first, Sentinel last, String & s)
            {
                normalize_append_impl_impl<Normalization>(first, last, s);
            }
        };

        template<nf Normalization, typename String>
        struct normalize_append_impl<Normalization, String, true, 2>
        {
            template<typename CPIter, typename Sentinel>
            static void call(CPIter first, Sentinel last, String & s)
            {
                normalize_append_impl_impl<Normalization>(first, last, s);
            }
        };

        template<typename T>
        using has_capacity_reserve =
            decltype(std::declval<T>().reserve(std::declval<T>().capacity()));

        template<
            typename String,
            bool Enable = is_detected<has_capacity_reserve, String>::value>
        struct normalize_string_impl
        {
            static void prefix(String const & s, String & temp) {}
            static void suffix(String & s, String & temp)
            {
                s = std::move(temp);
            }
        };

        template<typename String>
        struct normalize_string_impl<String, true>
        {
            static void prefix(String const & s, String & temp)
            {
                temp.reserve(s.size() * 2);
            }
            static void suffix(String & s, String & temp)
            {
                if (temp.size() <= s.capacity()) {
                    s = temp;
                } else {
                    using std::swap;
                    swap(s, temp);
                }
            }
        };
    }

    /** Appends sequence `[first, last)` in normalization form `Normalization`
        to `s`.  The output is UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16
        otherwise.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<
        nf Normalization,
        typename CPIter,
        typename Sentinel,
        typename String>
    inline auto normalize_append(CPIter first, Sentinel last, String & s)
        -> detail::cp_iter_ret_t<
            decltype(detail::normalize_append_impl<Normalization, String>::call(
                first, last, s)),
            CPIter>
    {
        detail::normalize_append_impl<Normalization, String>::call(
            first, last, s);
    }

    /** Appends sequence `r` in normalization form `Normalization` to `s`.
        The output is UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16
        otherwise. */
    template<nf Normalization, typename CPRange, typename String>
    inline void normalize_append(CPRange const & r, String & s)
    {
        return boost::text::v1::normalize_append<Normalization>(
            std::begin(r), std::end(r), s);
    }

    /** Puts the contents of `s` in Unicode normalization form `Normalize`. */
    template<nf Normalization, typename String>
    void normalize(String & s)
    {
        // TODO: Remove!
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<Normalization>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        detail::normalize_string_impl<String> impl;

        String temp;
        impl.prefix(s, temp);

        boost::text::v1::normalize_append<Normalization>(
            r.begin(), r.end(), temp);

        impl.suffix(s, temp);
    }

}}}

#if defined(__cpp_lib_concepts)

#include <boost/text/concepts.hpp>

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    /** Appends `[first, last)` in normalization form `Normalization` to `s`.
        The output is UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16
        otherwise. */
    template<
        nf Normalization,
        code_point_iterator I,
        std::sentinel_for<I> S,
        utf_string String>
    inline void normalize_append(I first, S last, String & s)
    {
        detail::normalization_string_appender_t<Normalization, String, I, S>
            appender(s);
        detail::norm_impl<Normalization, decltype(s.begin()), I, S>::call(
            first, last, appender);
        return s;
    }

    /** Appends `r` in normalization form `Normalization` to `s`.  The output
        is UTF-8 if `sizeof(*s.begin()) == 1`, and UTF-16 otherwise. */
    template<nf Normalization, code_point_range R, utf_string String>
    inline void normalize_append(R const & r, String & s)
    {
        return boost::text::v1::normalize_append_utf8<Normalization>(
            std::begin(r), std::end(r), s);
    }

    namespace detail {
        template<typename T>
        concept reserve_capacity_range =
            // clang-format off
            std::ranges::range<T> && requires(T const tc, T t) {
            { tc.capacity() } -> std::integral;
            { tc.reserve(tc.capacity()) };
            // clang-format on
        };
    }

    /** Puts the contents of `s` into Unicode normalization form
        `Normalize`. */
    template<nf Normalization, utf_string String>
    void normalize(String & s)
    {
        // TODO: Remove!
        auto const r = as_utf32(s);
        if (detail::normalized_quick_check(r.begin(), r.end(), [](uint32_t cp) {
                return detail::quick_check_code_point<Normalization>(cp);
            }) == detail::quick_check::yes) {
            return;
        }

        String temp;
        if constexpr (detail::reserve_capacity_range<String>) {
            if constexpr (Normalization == nf::d || Normalization == nf::kd)
                temp.reserve(s.size() / 2 * 3);
            else
                temp.reserve(s.size());
        }

        boost::text::v1::normalize_append_utf8<Normalization>(
            r.begin(), r.end(), temp);

        if constexpr (detail::reserve_capacity_range<String>) {
            if (temp.size() <= s.capacity()) {
                s = temp;
                return;
            }
        }
        std::ranges::swap(s, temp);
    }

}}}

#endif

#endif
