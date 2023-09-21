// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_UTF_HPP
#define BOOST_TEXT_UTF_HPP

#include <boost/text/config.hpp>

#include <cstdint>
#include <type_traits>
#include <cstdint>


namespace boost { namespace text {

    /** The Unicode Transformation Formats. */
    enum class format { utf8 = 1, utf16 = 2, utf32 = 4 };

    namespace detail {
        template<typename T>
        constexpr format format_of()
        {
            if constexpr (std::same_as<T, char> || std::same_as<T, char8_t>) {
                return format::utf8;
            } else if (
                std::same_as<T, char16_t>
#ifdef _MSC_VER
                || std::same_as<T, wchar_t>
#endif
            ) {
                return format::utf16;
            } else {
                return format::utf32;
            }
        }
    }

}}

#endif
