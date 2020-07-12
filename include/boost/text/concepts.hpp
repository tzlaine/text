// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_CONCEPTS_HPP
#define BOOST_TEXT_CONCEPTS_HPP

#if defined(__cpp_lib_concepts)

#include <boost/text/config.hpp>

#include <ranges>

namespace boost { namespace text { namespace v2 {

    namespace detail {
        template<typename T, int Bytes>
        concept u_x_iter =
            std::forward_iterator<T> &&
                std::is_convertible_v<std::iter_value_t<T>, char> &&
            sizeof(std::iter_value_t<T>) == Bytes;

        template<typename T, int Bytes>
        concept u_x_ptr = std::is_pointer_v<T> && u_x_iter<T, Bytes>;

        template<typename T, int Bytes>
        concept u_x_range = std::ranges::forward_range<T> &&
            u_x_iter<std::ranges::iterator_t<T>, Bytes>;

        template<typename T, int Bytes>
        concept contig_u_x_range = u_x_range<T, Bytes> &&
            std::contiguous_iterator<std::ranges::iterator_t<T>>;
    }

    template<typename T>
    concept u8_iter = detail::u_x_iter<T, 1>;
    template<typename T>
    concept u8_ptr = detail::u_x_ptr<T, 1>;
    template<typename T>
    concept u8_range = detail::u_x_range<T, 1>;
    template<typename T>
    concept contig_u8_range = detail::contig_u_x_range<T, 1>;

    template<typename T>
    concept u16_iter = detail::u_x_iter<T, 2>;
    template<typename T>
    concept u16_ptr = detail::u_x_ptr<T, 2>;
    template<typename T>
    concept u16_range = detail::u_x_range<T, 2>;
    template<typename T>
    concept contig_u16_range = detail::contig_u_x_range<T, 2>;

    template<typename T>
    concept u32_iter = detail::u_x_iter<T, 4>;
    template<typename T>
    concept u32_ptr = detail::u_x_ptr<T, 4>;
    template<typename T>
    concept u32_range = detail::u_x_range<T, 4>;
    template<typename T>
    concept contig_u32_range = detail::contig_u_x_range<T, 4>;

    namespace detail {
        template<typename T, int Bytes>
        concept cu_x = std::is_integral<T>::value &&
                           std::is_unsigned<T>::value &&
                       sizeof(T) == Bytes;
    }

    template<typename T>
    concept code_point = detail::cu_x<T, 4>;
    template<typename T>
    concept code_point_iterator =
        std::forward_iterator<T> && code_point<std::iter_value_t<T>>;
    template<typename T>
    concept code_point_range = std::ranges::forward_range<T> &&
        code_point_iterator<std::ranges::iterator_t<T>>;

    template<typename T>
    concept u32_code_unit = code_point<T>;

    template<typename T>
    concept u16_code_unit = detail::cu_x<T, 2>;

    template<typename T>
    concept u8_code_unit = detail::cu_x<T, 1>;

    // TODO: grapheme_iterator, grapheme_range, apply to grapeme_char_range

    template<typename T>
    concept grapheme_char_range =
        // clang-format off
        std::ranges::forward_range<T> && requires(T const t) {
        { t.begin().base() } -> code_point_iterator;
        { t.begin().base().base() } -> u8_iter;
        // clang-format on
    };

    namespace detail {
        template<typename T>
        using grapheme_bottom_iter_t =
            decltype(std::declval<T const>().begin().base().base());
    }

    template<typename T>
    concept contig_grapheme_char_range = grapheme_char_range<T> &&
        std::contiguous_iterator<detail::grapheme_bottom_iter_t<T>>;


    namespace detail {
        template<typename T>
        concept eraseable_sized_range =
            // clang-format off
            std::ranges::sized_range<T> && requires(T t) {
            { t.erase(t.begin(), t.end()) } -> std::same_as<std::ranges::iterator_t<T>>;
            // clang-format on
        };
    }

    template<typename T>
    concept utf8_string =
        // clang-format off
        detail::eraseable_sized_range<T> && requires(T t, char const * it) {
        { t.insert(t.end(), it, it) } -> std::same_as<std::ranges::iterator_t<T>>;
        // clang-format on
    };

    template<typename T>
    concept utf16_string =
        // clang-format off
        detail::eraseable_sized_range<T> && requires(T t, uint16_t const * it) {
        { t.insert(t.end(), it, it) } -> std::same_as<std::ranges::iterator_t<T>>;
        // clang-format on
    };

    template<typename T>
    concept utf_string = utf8_string<T> || utf16_string<T>;

}}}

#endif

#endif
