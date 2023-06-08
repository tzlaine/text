// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_CONCEPTS_HPP
#define BOOST_TEXT_CONCEPTS_HPP

#include <boost/text/config.hpp>
#include <boost/text/utf.hpp>

#if BOOST_TEXT_USE_CONCEPTS

#include <ranges>
#include <string_view>


namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    //[ concepts_concepts

#define BOOST_TEXT_CODE_UNIT_CONCEPT_OPTION_2 1

#if _MSC_VER
    inline constexpr format wchar_t_format = format::utf32;
#else
    inline constexpr format wchar_t_format = format::utf32;
#endif

    template<typename T, format F>
    concept code_unit = (std::same_as<T, char8_t> && F == format::utf8) ||
                        (std::same_as<T, char16_t> && F == format::utf16) ||
                        (std::same_as<T, char32_t> && F == format::utf32)
#if BOOST_TEXT_CODE_UNIT_CONCEPT_OPTION_2
                        || (std::same_as<T, char> && F == format::utf8) ||
                        (std::same_as<T, wchar_t> && F == wchar_t_format)
#endif
        ;

    template<typename T>
    concept utf8_code_unit = code_unit<T, format::utf8>;

    template<typename T>
    concept utf16_code_unit = code_unit<T, format::utf16>;

    template<typename T>
    concept utf32_code_unit = code_unit<T, format::utf32>;

    template<typename T, format F>
    concept code_unit_iter =
        std::input_iterator<T> && code_unit<std::iter_value_t<T>, F>;

    template<typename T>
    concept utf_code_unit =
        utf8_code_unit<T> || utf16_code_unit<T> || utf32_code_unit<T>;


    template<typename T, format F>
    concept code_unit_pointer =
        std::is_pointer_v<T> && code_unit<std::iter_value_t<T>, F>;

    template<typename T, format F>
    concept code_unit_range = std::ranges::input_range<T> &&
        code_unit<std::ranges::range_value_t<T>, F>;

    template<typename T, format F>
    concept contiguous_code_unit_range = std::ranges::contiguous_range<T> &&
        code_unit<std::ranges::range_value_t<T>, F>;

    template<typename T>
    concept utf8_iter = code_unit_iter<T, format::utf8>;
    template<typename T>
    concept utf8_pointer = code_unit_pointer<T, format::utf8>;
    template<typename T>
    concept utf8_range = code_unit_range<T, format::utf8>;
    template<typename T>
    concept contiguous_utf8_range = contiguous_code_unit_range<T, format::utf8>;

    template<typename T>
    concept utf16_iter = code_unit_iter<T, format::utf16>;
    template<typename T>
    concept utf16_pointer = code_unit_pointer<T, format::utf16>;
    template<typename T>
    concept utf16_range = code_unit_range<T, format::utf16>;
    template<typename T>
    concept contiguous_utf16_range =
        contiguous_code_unit_range<T, format::utf16>;

    template<typename T>
    concept utf32_iter = code_unit_iter<T, format::utf32>;
    template<typename T>
    concept utf32_pointer = code_unit_pointer<T, format::utf32>;
    template<typename T>
    concept utf32_range = code_unit_range<T, format::utf32>;
    template<typename T>
    concept contiguous_utf32_range =
        contiguous_code_unit_range<T, format::utf32>;

    template<typename T>
    concept code_point = utf32_code_unit<T>;
    template<typename T>
    concept code_point_iter = utf32_iter<T>;
    template<typename T>
    concept code_point_range = utf32_range<T>;

    template<typename T>
    concept utf_iter = utf8_iter<T> || utf16_iter<T> || utf32_iter<T>;
    template<typename T>
    concept utf_pointer =
        utf8_pointer<T> || utf16_pointer<T> || utf32_pointer<T>;
    template<typename T>
    concept utf_range = utf8_range<T> || utf16_range<T> || utf32_range<T>;


    template<typename T>
    concept grapheme_iter =
        // clang-format off
        std::input_iterator<T> &&
        code_point_range<std::iter_reference_t<T>> &&
        requires(T t) {
        { t.base() } -> code_point_iter;
        // clang-format on
    };

    template<typename T>
    concept grapheme_range = std::ranges::input_range<T> &&
        grapheme_iter<std::ranges::iterator_t<T>>;

    template<typename R>
    using code_point_iterator_t = decltype(std::declval<R>().begin().base());

    template<typename R>
    using code_point_sentinel_t = decltype(std::declval<R>().end().base());

    template<typename T, format F>
    concept grapheme_iter_code_unit =
        // clang-format off
        grapheme_iter<T> &&
        requires(T t) {
        { t.base().base() } -> code_unit_iter<F>;
        // clang-format on
    };

    template<typename T, format F>
    concept grapheme_range_code_unit = grapheme_range<T> &&
        grapheme_iter_code_unit<std::ranges::iterator_t<T>, F>;


    namespace dtl {
        template<typename T, class CodeUnit>
        concept eraseable_insertable_sized_bidi_range =
            // clang-format off
            std::ranges::sized_range<T> &&
            std::ranges::input_range<T> &&
            requires(T t, CodeUnit const * it) {
                { t.erase(t.begin(), t.end()) } ->
                    std::same_as<std::ranges::iterator_t<T>>;
                { t.insert(t.end(), it, it) } ->
                    std::same_as<std::ranges::iterator_t<T>>;
            };
        // clang-format on
    }

    template<typename T>
    concept utf8_string =
        // clang-format off
        utf8_code_unit<std::ranges::range_value_t<T>> &&
        dtl::eraseable_insertable_sized_bidi_range<
            T, std::ranges::range_value_t<T>>;
        // clang-format on

    template<typename T>
    concept utf16_string =
        // clang-format off
        utf16_code_unit<std::ranges::range_value_t<T>> &&
        dtl::eraseable_insertable_sized_bidi_range<
            T, std::ranges::range_value_t<T>>;
        // clang-format on

    template<typename T>
    concept utf_string = utf8_string<T> || utf16_string<T>;

    template<typename T>
    // clang-format off
        concept transcoding_error_handler = requires(T t, std::string_view msg) {
        { t(msg) } -> std::same_as<char32_t>;
        // clang-format on
    };

    template<typename T>
    // clang-format off
    concept utf_range_like =
        utf_range<std::remove_reference_t<T>> ||
        utf_pointer<std::remove_reference_t<T>>;
    // clang-format on

    template<typename T>
    concept utf8_input_range_like =
        (std::ranges::input_range<std::remove_reference_t<T>> &&
         utf8_code_unit<std::iter_value_t<T>>) ||
        utf8_pointer<std::remove_reference_t<T>>;
    template<typename T>
    concept utf16_input_range_like =
        (std::ranges::input_range<std::remove_reference_t<T>> &&
         utf16_code_unit<std::iter_value_t<T>>) ||
        utf16_pointer<std::remove_reference_t<T>>;
    template<typename T>
    concept utf32_input_range_like =
        (std::ranges::input_range<std::remove_reference_t<T>> &&
         utf32_code_unit<std::iter_value_t<T>>) ||
        utf32_pointer<std::remove_reference_t<T>>;

    template<typename T>
    concept utf_input_range_like =
        utf8_input_range_like<T> || utf16_input_range_like<T> ||
        utf32_input_range_like<T>;

    //]

    // Clang 13 defines __cpp_lib_concepts but not std::indirectly copyable.
#if defined(__clang_major__) && __clang_major__ == 13
    template<typename In, typename Out>
    // clang-format off
    concept indirectly_copyable =
        std::indirectly_readable<In> &&
        std::indirectly_writable<Out, std::iter_reference_t<In>>;
    // clang-format on
#else
    template<typename In, typename Out>
    concept indirectly_copyable = std::indirectly_copyable<In, Out>;
#endif

}}}

#endif

#endif
