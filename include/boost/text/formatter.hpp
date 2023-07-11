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


namespace boost::text::detail {
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
  template<class ParseContext>
    constexpr typename ParseContext::iterator
      parse(ParseContext& ctx) {
        return underlying_.parse(ctx);
      }

  template<class FormatContext>
    typename FormatContext::iterator
      format(const boost::text::utf_view<Format, V>& view, FormatContext& ctx) const {
        basic_string<CharT> str;
        if constexpr (std::same_as<CharT, char>) {
          auto v = view | boost::text::as_utf8;
          str.insert(str.begin(), v.begin(), v.end());
        } else {
#if defined(_MSC_VER)
          auto v = view | boost::text::as_utf16;
#else
          auto v = view | boost::text::as_utf32;
#endif
          str.insert(str.begin(), v.begin(), v.end());
        }
        return underlying_.format(str, ctx);
      }

  constexpr void set_debug_format() noexcept {
    underlying_.set_debug_format();
  }

private:
  std::formatter<std::basic_string<CharT>, CharT> underlying_;
};

#endif
