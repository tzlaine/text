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

    // Using this custom template is quite a bit faster than using lambdas.
    // Unexpected.
    template<typename RepackedIterator, typename I, typename S, typename Then>
    struct repacker
    {
        auto operator()(I it) const
        {
            return then(RepackedIterator(first, it, last));
        }

        [[no_unique_address]] I first;
        [[no_unique_address]] S last;
        [[no_unique_address]] Then then;
    };

    namespace detail {
        template<typename I, typename S, typename Repack>
        constexpr auto
        unpack_iterator_and_sentinel_impl(I first, S last, Repack repack);

        // 8 -> 32
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_32_iterator<I> first,
            utf_8_to_32_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_32_iterator<I, S> first, S last, Repack repack);
        // 32 -> 8
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_8_iterator<I> first,
            utf_32_to_8_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_8_iterator<I, S> first, S last, Repack repack);
        // 16 -> 32
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_32_iterator<I> first,
            utf_16_to_32_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_32_iterator<I, S> first, S last, Repack repack);
        // 32 -> 16
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_16_iterator<I> first,
            utf_32_to_16_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_16_iterator<I, S> first, S last, Repack repack);
        // 8 -> 16
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_16_iterator<I> first,
            utf_8_to_16_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_16_iterator<I, S> first, S last, Repack repack);
        // 16 -> 8
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_8_iterator<I> first,
            utf_16_to_8_iterator<I> last,
            Repack repack);
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_8_iterator<I, S> first, S last, Repack repack);

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
        Repack repack;
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

        // 8 -> 32
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_32_iterator<I> first,
            utf_8_to_32_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_8_to_32_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_32_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_8_to_32_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }
        // 32 -> 8
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_8_iterator<I> first,
            utf_32_to_8_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_32_to_8_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_8_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_32_to_8_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }

        // 16 -> 32
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_32_iterator<I> first,
            utf_16_to_32_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_16_to_32_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_32_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_16_to_32_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }
        // 32 -> 16
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_16_iterator<I> first,
            utf_32_to_16_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_32_to_16_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_32_to_16_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_32_to_16_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }

        // 8 -> 16
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_16_iterator<I> first,
            utf_8_to_16_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_8_to_16_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_8_to_16_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_8_to_16_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }
        // 16 -> 8
        template<typename I, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_8_iterator<I> first,
            utf_16_to_8_iterator<I> last,
            Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last.base(),
                repacker<utf_16_to_8_iterator<I>, I, I, Repack>(
                    first.begin(), first.end(), repack));
        }
        template<typename I, typename S, typename Repack>
        constexpr auto unpack_iterator_and_sentinel_impl(
            utf_16_to_8_iterator<I, S> first, S last, Repack repack)
        {
            return detail::unpack_iterator_and_sentinel_impl(
                first.base(),
                last,
                repacker<utf_16_to_8_iterator<I, S>, I, S, Repack>(
                    first.begin(), first.end(), repack));
        }
}}}

#endif
