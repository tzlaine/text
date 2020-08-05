// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_FWD_HPP
#define BOOST_TEXT_TEXT_FWD_HPP

#include <boost/text/normalize_fwd.hpp>
#include <boost/text/concepts.hpp>


namespace boost { namespace text {

    template<nf Normalization, typename T>
#if defined(__cpp_lib_concepts)
    // clang-format off
        requires u8_code_unit<T> // TODO: Support for UTF-16
#endif
    struct basic_text_view;
    // clang-format on

    template<nf Normalization, typename String>
#if defined(__cpp_lib_concepts)
    // clang-format off
        requires u8_code_unit<std::ranges::range_value_t<String>> // TODO: Support for UTF-16
#endif
    struct basic_text;
    // clang-format on

    /** The specialization of `basic_text` that should be used in most
        situations. */
    using text = basic_text<nf::fcc, std::string>;
    // TODO: Change this to NFD.

    /** The specialization of `basic_text` that should be used in most
        situations. */
    using text_view = basic_text_view<nf::fcc, char>;
    // TODO: Change this to NFD.

}}

#endif
