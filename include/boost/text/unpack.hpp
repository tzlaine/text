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
        template<class T>
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
            typename Then,
            bool Bidi>
        struct repacker
        {
            repacker() = default;
            repacker(I first, S last, Then then) requires Bidi : first{first},
                                                                 last{last},
                                                                 then{then}
            {}
            repacker(S last, Then then) requires (!Bidi) : last{last}, then{then}
            {}

            auto operator()(I it) const
            {
                if constexpr (Bidi) {
                    return then(RepackedIterator(*first, it, last));
                } else {
                    return then(RepackedIterator(it, last));
                }
            }

            std::optional<I> first;
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
            requires std::forward_iterator<I>
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
        class Repack>
    struct unpack_result
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
                return unpack_result<format::utf8, I, S, Repack>{
                    first, last, repack};
            } else if constexpr (utf16_iter<I>) {
                return unpack_result<format::utf16, I, S, Repack>{
                    first, last, repack};
            } else {
                return unpack_result<format::utf32, I, S, Repack>{
                    first, last, repack};
            }
        }

    }
}}

#include <boost/text/transcode_iterator.hpp>

namespace boost { namespace text { namespace detail {

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
                first.base(),
                last.base(),
                repacker<
                    iterator,
                    decltype(first.begin()),
                    decltype(first.end()),
                    Repack,
                    true>(first.begin(), first.end(), repack));
        } else {
            return boost::text::unpack_iterator_and_sentinel(
                first.base(),
                last.base(),
                repacker<iterator, int, decltype(first.end()), Repack, false>(
                    first.end(), repack));
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
                first.base(),
                last,
                repacker<
                    iterator,
                    decltype(first.begin()),
                    decltype(first.end()),
                    Repack,
                    true>(first.begin(), first.end(), repack));
        } else {
            return boost::text::unpack_iterator_and_sentinel(
                first.base(),
                last,
                repacker<iterator, int, S, Repack, false>(last, repack));
        }
    }

}}}

#endif
