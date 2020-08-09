// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_ROPE_FWD_HPP
#define BOOST_TEXT_ROPE_FWD_HPP

#include <boost/text/normalize_fwd.hpp>
#include <boost/text/concepts.hpp>


namespace boost { namespace text {

    template<
        nf Normalization,
        typename Char,
        typename String = std::basic_string<Char>>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires utf8_code_unit<Char> || utf16_code_unit<Char>
#endif
    struct basic_rope_view;
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
    struct basic_rope;
    // clang-format on

    /** The specialization of `basic_rope` that should be used in most
        situations. */
    using rope = basic_rope<nf::fcc, char>;
    // TODO: Change this to NFC.

    /** The specialization of `basic_rope_view` that should be used in most
        situations. */
    using rope_view = basic_rope_view<nf::fcc, char>;
    // TODO: Change this to NFC.

}}

#endif
