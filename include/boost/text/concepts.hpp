// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_CONCEPTS_HPP
#define BOOST_TEXT_CONCEPTS_HPP

#include <boost/text/config.hpp>

#if defined(BOOST_TEXT_DOXYGEN) || defined(__cpp_lib_concepts)

#include <ranges>


namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    namespace dtl {
        template<typename T, int Bytes>
        concept cu_x = std::is_integral<T>::value && sizeof(T) == Bytes;
    }

    template<typename T>
    concept u8_code_unit = dtl::cu_x<T, 1>;

    template<typename T>
    concept u16_code_unit = dtl::cu_x<T, 2>;

    template<typename T>
    concept u32_code_unit = dtl::cu_x<T, 4>;

    namespace dtl {
        template<typename T, int Bytes>
        concept u_x_iter =
            std::bidirectional_iterator<T> && cu_x<std::iter_value_t<T>, Bytes>;

        template<typename T, int Bytes>
        concept u_x_ptr =
            std::is_pointer_v<T> && cu_x<std::iter_value_t<T>, Bytes>;

        template<typename T, int Bytes>
        concept u_x_range = std::ranges::bidirectional_range<T> &&
            cu_x<std::ranges::range_value_t<T>, Bytes>;

        template<typename T, int Bytes>
        concept contig_u_x_range = std::ranges::contiguous_range<T> &&
            cu_x<std::ranges::range_value_t<T>, Bytes>;
    }

    template<typename T>
    concept u8_iter = dtl::u_x_iter<T, 1>;
    template<typename T>
    concept u8_ptr = dtl::u_x_ptr<T, 1>;
    template<typename T>
    concept u8_range = dtl::u_x_range<T, 1>;
    template<typename T>
    concept contig_u8_range = dtl::contig_u_x_range<T, 1>;

    template<typename T>
    concept u16_iter = dtl::u_x_iter<T, 2>;
    template<typename T>
    concept u16_ptr = dtl::u_x_ptr<T, 2>;
    template<typename T>
    concept u16_range = dtl::u_x_range<T, 2>;
    template<typename T>
    concept contig_u16_range = dtl::contig_u_x_range<T, 2>;

    template<typename T>
    concept u32_iter = dtl::u_x_iter<T, 4>;
    template<typename T>
    concept u32_ptr = dtl::u_x_ptr<T, 4>;
    template<typename T>
    concept u32_range = dtl::u_x_range<T, 4>;
    template<typename T>
    concept contig_u32_range = dtl::contig_u_x_range<T, 4>;

    template<typename T>
    concept code_point = u32_code_unit<T>;
    template<typename T>
    concept code_point_iterator = u32_iter<T>;
    template<typename T>
    concept code_point_range = u32_range<T>;


    template<typename T>
    concept grapheme_iter =
        // clang-format off
        std::bidirectional_iterator<T> &&
        code_point_range<std::iter_reference_t<T>> &&
        requires(T t) {
        { t.base() } -> code_point_iterator;
        // clang-format on
    };

    template<typename T>
    concept grapheme_range = std::ranges::bidirectional_range<T> &&
        grapheme_iter<std::ranges::iterator_t<T>>;

    template<typename T>
    concept grapheme_char_iter =
        // clang-format off
        grapheme_iter<T> &&
        requires(T t) {
        { t.base().base() } -> u8_iter;
        // clang-format on
    };

    template<typename T>
    concept grapheme_char_range = std::ranges::bidirectional_range<T> &&
        grapheme_char_iter<std::ranges::iterator_t<T>>;

    namespace dtl {
        template<typename T>
        using grapheme_bottom_iter_t =
            decltype(std::declval<T>().begin().base().base());
    }

    template<typename T>
    concept contig_grapheme_char_range = grapheme_char_range<T> &&
        std::contiguous_iterator<dtl::grapheme_bottom_iter_t<T>>;


    namespace dtl {
        template<typename T>
        concept eraseable_sized_bidi_range =
            // clang-format off
            std::ranges::sized_range<T> &&
            std::ranges::bidirectional_range<T> && requires(T t) {
            { t.erase(t.begin(), t.end()) } ->
                std::same_as<std::ranges::iterator_t<T>>;
            // clang-format on
        };
    }

    template<typename T>
    concept utf8_string =
        // clang-format off
        dtl::eraseable_sized_bidi_range<T> &&
        u8_code_unit<std::ranges::range_value_t<T>> &&
        requires(T t, char const * it) {
        { t.insert(t.end(), it, it) } ->
            std::same_as<std::ranges::iterator_t<T>>;
        // clang-format on
    };

    template<typename T>
    concept utf16_string =
        // clang-format off
        dtl::eraseable_sized_bidi_range<T> &&
        u16_code_unit<std::ranges::range_value_t<T>> &&
        requires(T t, uint16_t const * it) {
        { t.insert(t.end(), it, it) } ->
            std::same_as<std::ranges::iterator_t<T>>;
        // clang-format on
    };

    template<typename T>
    concept utf_string = utf8_string<T> || utf16_string<T>;

    template<typename T>
    // clang-format off
    concept transcoding_error_handler = requires(T t) {
        { t("") } -> code_point;
        // clang-format on
    };

}}}

#endif

#endif
