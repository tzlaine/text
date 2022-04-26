// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DETAIL_UNPACK_HPP
#define BOOST_TEXT_DETAIL_UNPACK_HPP

#include <boost/text/transcode_iterator.hpp>


namespace boost { namespace text { namespace detail {

    struct no_op_repack
    {
        template<typename T>
        T operator()(T x) const
        {
            return x;
        }
    };

    // Using this custom template is quite a bit faster than using lambdas.
    // Unexpected.
    template<typename Iterator, typename I, typename S, typename Then>
    struct repacker_t
    {
        auto operator()(I it) const
        {
            return then_(Iterator(first_, it, last_));
        }

        [[no_unique_address]] I first_;
        [[no_unique_address]] S last_;
        [[no_unique_address]] Then then_;
    };
    template<typename Iterator, typename I, typename S, typename Then>
    auto repacker(I first, S last, Then then)
    {
        return repacker_t<Iterator, I, S, Then>{first, last, then};
    }

    struct utf8_tag
    {};
    struct utf16_tag
    {};
    struct utf32_tag
    {};

    template<
        typename Tag,
        typename Iter,
        typename Sentinel = Iter,
        typename Repack = no_op_repack>
    struct tagged_range
    {
        Iter f_;
        Sentinel l_;
        Tag tag_;
        Repack repack_;
    };

    template<typename Tag, typename Iter, typename Sentinel, typename Repack>
    auto make_tagged_range(Tag tag, Iter f, Sentinel l, Repack repack)
    {
        return tagged_range<Tag, Iter, Sentinel, Repack>{f, l, tag, repack};
    }

    template<
        typename Iter,
        typename Sentinel,
        typename Repack,
        bool UTF8 = is_char_iter<Iter>::value,
        bool UTF16 = is_16_iter<Iter>::value,
        bool UTF32 = is_cp_iter<Iter>::value>
    struct unpack_iterator_and_sentinel_impl
    {};

    template<typename Iter, typename Sentinel, typename Repack>
    struct unpack_iterator_and_sentinel_impl<
        Iter,
        Sentinel,
        Repack,
        true,
        false,
        false>
    {
        static constexpr auto
        call(Iter first, Sentinel last, Repack repack) noexcept
        {
            return detail::make_tagged_range(utf8_tag{}, first, last, repack);
        }
    };
    template<typename Iter, typename Sentinel, typename Repack>
    struct unpack_iterator_and_sentinel_impl<
        Iter,
        Sentinel,
        Repack,
        false,
        true,
        false>
    {
        static constexpr auto
        call(Iter first, Sentinel last, Repack repack) noexcept
        {
            return detail::make_tagged_range(utf16_tag{}, first, last, repack);
        }
    };
    template<typename Iter, typename Sentinel, typename Repack>
    struct unpack_iterator_and_sentinel_impl<
        Iter,
        Sentinel,
        Repack,
        false,
        false,
        true>
    {
        static constexpr auto
        call(Iter first, Sentinel last, Repack repack) noexcept
        {
            return detail::make_tagged_range(utf32_tag{}, first, last, repack);
        }
    };

    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        Iter first, Sentinel last, Repack repack = no_op_repack{}) noexcept
        -> decltype(unpack_iterator_and_sentinel_impl<
                    std::remove_cv_t<Iter>,
                    std::remove_cv_t<Sentinel>,
                    Repack>::call(first, last, repack))
    {
        using iterator = std::remove_cv_t<Iter>;
        using sentinel = std::remove_cv_t<Sentinel>;
        return detail::
            unpack_iterator_and_sentinel_impl<iterator, sentinel, Repack>::call(
                first, last, repack);
    }

    // 8 -> 32
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_32_iterator<Iter> first,
        utf_8_to_32_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_32_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;
    // 32 -> 8
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_8_iterator<Iter> first,
        utf_32_to_8_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_8_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;
    // 16 -> 32
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_32_iterator<Iter> first,
        utf_16_to_32_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_32_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;
    // 32 -> 16
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_16_iterator<Iter> first,
        utf_32_to_16_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_16_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;
    // 8 -> 16
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_16_iterator<Iter> first,
        utf_8_to_16_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_16_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;
    // 16 -> 8
    template<typename Iter, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_8_iterator<Iter> first,
        utf_16_to_8_iterator<Iter> last,
        Repack repack = no_op_repack{}) noexcept;
    template<typename Iter, typename Sentinel, typename Repack = no_op_repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_8_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack = no_op_repack{}) noexcept;

    // 8 -> 32
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_32_iterator<Iter> first,
        utf_8_to_32_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_8_to_32_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_32_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_8_to_32_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }
    // 32 -> 8
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_8_iterator<Iter> first,
        utf_32_to_8_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_32_to_8_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_8_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_32_to_8_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }

    // 16 -> 32
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_32_iterator<Iter> first,
        utf_16_to_32_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_16_to_32_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_32_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_16_to_32_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }
    // 32 -> 16
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_16_iterator<Iter> first,
        utf_32_to_16_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_32_to_16_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_32_to_16_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_32_to_16_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }

    // 8 -> 16
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_16_iterator<Iter> first,
        utf_8_to_16_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_8_to_16_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_8_to_16_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_8_to_16_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }
    // 16 -> 8
    template<typename Iter, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_8_iterator<Iter> first,
        utf_16_to_8_iterator<Iter> last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last.base(),
            detail::repacker<utf_16_to_8_iterator<Iter>>(
                first.begin(), first.end(), repack));
    }
    template<typename Iter, typename Sentinel, typename Repack>
    constexpr auto unpack_iterator_and_sentinel(
        utf_16_to_8_iterator<Iter, Sentinel> first,
        Sentinel last,
        Repack repack) noexcept
    {
        return detail::unpack_iterator_and_sentinel(
            first.base(),
            last,
            detail::repacker<utf_16_to_8_iterator<Iter, Sentinel>>(
                first.begin(), first.end(), repack));
    }

}}}

#endif
