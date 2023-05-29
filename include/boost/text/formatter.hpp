// Copyright (C) 2023 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_FORMATTER_HPP
#define BOOST_TEXT_FORMATTER_HPP

#include <boost/text/transcode_view.hpp>
#include <boost/text/estimated_width.hpp>

#include <format>


// Experimental implementation of formatter for utf_view.
// TODO: Replace with something portable!
#if defined(__GNUC__) && 13 <= __GNUC__

#include <boost/text/estimated_width.hpp>

namespace boost::text::detail {

    template<typename R, typename Out, typename CharT>
    Out format_aligned(
        R && r, Out out, std::__format::_Align align, int fill, CharT fill_char)
    {
        BOOST_ASSERT(align != std::__format::_Align_default);

        int left = 0;
        int right = 0;
        if (align == std::__format::_Align_centre) {
            left = fill / 2;
            right = left + (fill & 1);
        } else if (align == std::__format::_Align_right) {
            left = fill;
            right = 0;
        } else {
            left = 0;
            right = fill;
        }

        out = std::fill_n(out, left, fill_char);
        out = std::ranges::copy(r, out).out;
        out = std::fill_n(out, right, fill_char);

        return out;
    }

    template<typename CharT>
    concept formatter_char =
        std::same_as<CharT, char> || std::same_as<CharT, wchar_t>;
}

template<
    boost::text::format Format,
    class V,
    boost::text::detail::formatter_char CharT>
struct std::formatter<boost::text::utf_view<Format, V>, CharT>
{
    constexpr typename std::basic_format_parse_context<CharT>::iterator
    parse(std::basic_format_parse_context<CharT> & ctx)
    {
        auto first = ctx.begin();
        auto const last = ctx.end();
        std::__format::_Spec<CharT> spec = {};

        auto at_end = [&, this] {
            if (first == last || *first == '}') {
                spec_ = spec;
                return true;
            }
            return false;
        };

        if (at_end())
            return first;

        first = spec._M_parse_fill_and_align(first, last);
        if (at_end())
            return first;

        first = spec._M_parse_width(first, last, ctx);
        if (at_end())
            return first;

        first = spec._M_parse_precision(first, last, ctx);
        if (at_end())
            return first;

        if (*first == 's')
            ++first;
#if __cpp_lib_format_ranges
        else if (*first == '?') {
            spec._M_type = std::__format::_Pres_esc;
            ++first;
        }
#endif

        if (at_end())
            return first;

        BOOST_THROW_EXCEPTION(
            std::format_error("format error: failed to parse format-spec"));
    }

    template<typename Out>
    Out format(
        const boost::text::utf_view<Format, V> & view,
        std::basic_format_context<Out, CharT> & ctx) const
    {
        if (spec_._M_type == std::__format::_Pres_esc) {
            // TODO
        }

        if (spec_._M_width_kind == std::__format::_WP_none &&
            spec_._M_prec_kind == std::__format::_WP_none) {
            if constexpr (std::same_as<CharT, char>) {
                return std::ranges::copy(view | boost::text::as_utf8, ctx.out())
                    .out;
            } else {
                return std::ranges::copy(
                           view | boost::text::as_utf32, ctx.out())
                    .out;
            }
        }

        int const estimated_width = boost::text::estimated_width_of_graphemes(
            view | boost::text::as_utf32);
        int final_width = estimated_width;

        if (spec_._M_prec_kind != std::__format::_WP_none) {
            int const precision = spec_._M_get_precision(ctx);
            if (precision < estimated_width)
                final_width = precision;
        }

        int const width = spec_._M_get_width(ctx);
        int const fill = (std::max)(0, width - final_width);

        auto align = std::__format::_Align_left;
        if (spec_._M_align)
            align = spec_._M_align;

        if constexpr (std::same_as<CharT, char>) {
            return boost::text::detail::format_aligned(
                view | boost::text::as_utf8,
                ctx.out(),
                align,
                fill,
                spec_._M_fill);
        } else {
            return boost::text::detail::format_aligned(
                view | boost::text::as_utf32,
                ctx.out(),
                align,
                fill,
                spec_._M_fill);
        }
    }

#if __cpp_lib_format_ranges
    constexpr void set_debug_format() noexcept { spec_.type = escaped; }
#endif

private:
    std::__format::_Spec<CharT> spec_;
};

template<class V, class CharT>
struct std::formatter<boost::text::utf8_view<V>, CharT>
    : std::formatter<boost::text::utf_view<boost::text::format::utf8, V>, CharT>
{
    template<class FormatContext>
    auto format(const boost::text::utf8_view<V>& view, FormatContext & ctx) const
    {
        return std::formatter<
            boost::text::utf_view<boost::text::format::utf8, V>,
            CharT>::format(view, ctx);
    }
};

template<class V, class CharT>
struct std::formatter<boost::text::utf16_view<V>, CharT>
    : std::
          formatter<boost::text::utf_view<boost::text::format::utf16, V>, CharT>
{
    template<class FormatContext>
    auto format(const boost::text::utf16_view<V>& view, FormatContext & ctx) const
    {
        return std::formatter<
            boost::text::utf_view<boost::text::format::utf16, V>,
            CharT>::format(view, ctx);
    }
};

template<class V, class CharT>
struct std::formatter<boost::text::utf32_view<V>, CharT>
    : std::
          formatter<boost::text::utf_view<boost::text::format::utf32, V>, CharT>
{
    template<class FormatContext>
    auto format(const boost::text::utf32_view<V>& view, FormatContext & ctx) const
    {
        return std::formatter<
            boost::text::utf_view<boost::text::format::utf32, V>,
            CharT>::format(view, ctx);
    }
};

#endif

#endif
