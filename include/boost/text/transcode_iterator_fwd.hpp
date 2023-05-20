// Copyright (C) 2023 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_ITERATOR_FWD_HPP
#define BOOST_TEXT_TRANSCODE_ITERATOR_FWD_HPP

#include <boost/text/concepts.hpp>


namespace boost { namespace text {

    struct use_replacement_character;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_8_to_32_iterator;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_32_to_8_iterator;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename Sentinel = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_8_to_16_iterator;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_16_to_8_iterator;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_16_to_32_iterator;

#if BOOST_TEXT_USE_CONCEPTS
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
#else
    template<
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    struct utf_32_to_16_iterator;

    template<
        format FromFormat,
        format ToFormat,
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    class transcoding_iterator;

}}

#endif
