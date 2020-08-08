// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_FWD_HPP
#define BOOST_TEXT_TEXT_FWD_HPP

#include <boost/text/normalize_fwd.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/transcode_iterator.hpp>


namespace boost { namespace text {

    template<nf Normalization, typename Char>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires utf8_code_unit<Char> || utf16_code_unit<Char>
#endif
    struct basic_text_view;
    // clang-format on

    template<
        nf Normalization,
        typename Char,
        typename String = std::basic_string<Char>>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires (utf8_code_unit<Char> || utf16_code_unit<Char>) &&
            std::is_same_v<Char, std::ranges::range_value_t<String>>
#endif
    struct basic_text;
    // clang-format on

    /** The specialization of `basic_text` that should be used in most
        situations. */
    using text = basic_text<nf::fcc, char>;
    // TODO: Change this to NFC.

    /** The specialization of `basic_text_view` that should be used in most
        situations. */
    using text_view = basic_text_view<nf::fcc, char>;
    // TODO: Change this to NFC.

    namespace detail {
        template<typename T, typename U, typename R>
        using enable_if_different =
            std::enable_if_t<!std::is_same<T, U>::value, R>;

        template<typename T, int Size = sizeof(T)>
        struct text_transcode_iterator;
        template<typename T>
        struct text_transcode_iterator<T, 1>
        {
            using type = utf_8_to_32_iterator<T *>;
        };
        template<typename T>
        struct text_transcode_iterator<T, 2>
        {
            using type = utf_16_to_32_iterator<T *>;
        };
        template<typename T>
        using text_transcode_iterator_t =
            typename text_transcode_iterator<T>::type;
    }

}}

#endif
