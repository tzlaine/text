// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_UNPACK_HPP
#define BOOST_TEXT_UNPACK_HPP

#include <boost/text/transcode_iterator_fwd.hpp>


namespace boost { namespace text {

    struct no_op_repacker
    {
        template<typename T>
        T operator()(T x) const
        {
            return x;
        }
    };

    namespace detail {
        // Using this custom template is quite a bit faster than using lambdas.
        // Unexpected.
        template<
            typename RepackedIterator,
            typename I,
            typename S,
            typename Then>
        struct bidi_repacker
        {
            bidi_repacker() = default;
            bidi_repacker(I first, S last, Then then) :
                first{first}, last{last}, then{then}
            {}

            auto operator()(I it) const
            {
                auto repacked = RepackedIterator(first, it, last);
                --repacked;
                repacked.buf_index_ = 0;
                return then(repacked);
            }

            I first;
            [[no_unique_address]] S last;
            [[no_unique_address]] Then then;
        };

        template<typename I, typename S, typename Repack>
        constexpr auto
        unpack_iterator_and_sentinel_impl(I first, S last, Repack repack);

        template<
            format FromFormat,
            format ToFormat,
            typename I,
            typename S,
            typename ErrorHandler,
            typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> first,
            utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> last,
            Repack repack);

        template<
            format FromFormat,
            format ToFormat,
            typename I,
            typename S,
            typename ErrorHandler,
            typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> first,
            S last,
            Repack repack);

        template<typename I, typename S, typename Repack>
        constexpr auto
        unpack_iterator_and_sentinel(I first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first, last, repack);
        }

        struct unpack_iterator_and_sentinel_cpo
        {
            template<
                utf_iter I,
                std::sentinel_for<I> S,
                typename Repack = no_op_repacker>
            constexpr auto
            operator()(I first, S last, Repack repack = Repack()) const
            {
                return unpack_iterator_and_sentinel(first, last, repack);
            }
        };
    }

    inline namespace cpo {
        inline constexpr detail::unpack_iterator_and_sentinel_cpo
            unpack_iterator_and_sentinel{};
    }

    template<
        format FormatTag,
        utf_iter I,
        std::sentinel_for<I> S,
        typename Repack>
    struct utf_tagged_range
    {
        static constexpr format format_tag = FormatTag;

        I first;
        [[no_unique_address]] S last;
        [[no_unique_address]] Repack repack;
    };

    namespace detail {
        template<typename I, typename S, typename Repack>
        constexpr auto
        unpack_iterator_and_sentinel_impl(I first, S last, Repack repack)
        {
            if constexpr (utf8_iter<I>) {
                return utf_tagged_range<format::utf8, I, S, Repack>{
                    first, last, repack};
            } else if constexpr (utf16_iter<I>) {
                return utf_tagged_range<format::utf16, I, S, Repack>{
                    first, last, repack};
            } else {
                return utf_tagged_range<format::utf32, I, S, Repack>{
                    first, last, repack};
            }
        }

    }
}}

#include <boost/text/transcode_iterator.hpp>

namespace boost { namespace text { namespace detail {

    template<format Format, typename I>
    auto back_up_one_cp(I first, I it)
    {
        if constexpr (Format == format::utf8) {
            it = detail::decrement(first, it);
        } else if constexpr (Format == format::utf16) {
            if (it == first)
                return it;
            if (boost::text::low_surrogate(*--it)) {
                if (it != first &&
                    boost::text::high_surrogate(*std::prev(it))) {
                    --it;
                }
            }
        } else {
            if (it != first)
                --it;
        }
        return it;
    }

    template<
        format FromFormat,
        format ToFormat,
        typename I,
        typename S,
        typename ErrorHandler,
        typename Repack>
    constexpr auto unpack_iterator_and_sentinel_impl(
        utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> first,
        utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> last,
        Repack repack)
    {
        using iterator = utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>;
        if constexpr (std::bidirectional_iterator<I>) {
            return boost::text::unpack_iterator_and_sentinel(
                detail::back_up_one_cp<FromFormat>(first.begin(), first.base()),
                last.base(),
                bidi_repacker<iterator, I, S, Repack>(
                    first.begin(), first.end(), repack));
        } else {
            return boost::text::unpack_iterator_and_sentinel(
                std::move(first).base(), last.base(), no_op_repacker{});
        }
    }

    template<
        format FromFormat,
        format ToFormat,
        typename I,
        typename S,
        typename ErrorHandler,
        typename Repack>
    constexpr auto unpack_iterator_and_sentinel_impl(
        utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler> first,
        S last,
        Repack repack)
    {
        using iterator = utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>;
        if constexpr (std::bidirectional_iterator<I>) {
            return boost::text::unpack_iterator_and_sentinel(
                detail::back_up_one_cp<FromFormat>(first.begin(), first.base()),
                last,
                bidi_repacker<iterator, I, S, Repack>(
                    first.begin(), first.end(), repack));
        } else {
            return boost::text::unpack_iterator_and_sentinel(
                std::move(first).base(), last, no_op_repacker{});
        }
    }

}}}

#endif
