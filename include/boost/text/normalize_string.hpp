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

}}}

#endif
