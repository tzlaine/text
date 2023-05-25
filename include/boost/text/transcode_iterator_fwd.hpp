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

    namespace detail {
        template<
            typename RepackedIterator,
            typename I,
            typename S,
            typename Then>
        struct bidi_repacker;
    }
}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<
        format FromFormat,
        format ToFormat,
        code_unit_iter<FromFormat> I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    class utf_iterator;

    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_16_iterator =
        utf_iterator<format::utf8, format::utf16, I, S, ErrorHandler>;
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_8_iterator =
        utf_iterator<format::utf16, format::utf8, I, S, ErrorHandler>;


    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_32_iterator =
        utf_iterator<format::utf8, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_8_iterator =
        utf_iterator<format::utf32, format::utf8, I, S, ErrorHandler>;


    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_32_iterator =
        utf_iterator<format::utf16, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_16_iterator =
        utf_iterator<format::utf32, format::utf16, I, S, ErrorHandler>;

}}}

#endif
