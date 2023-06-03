// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_GRAPHEME_VIEW_HPP
#define BOOST_TEXT_GRAPHEME_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {
        template<typename CPIter, typename Sentinel>
        using gr_view_sentinel_t = std::conditional_t<
            std::is_same<CPIter, Sentinel>::value,
            grapheme_iterator<CPIter, Sentinel>,
            Sentinel>;
    }

    /** A view over graphemes that occur in an underlying sequence of code
        points. */
    template<std::ranges::view V>
    // clang-format off
        requires utf32_range<V>
    class grapheme_view : public std::ranges::view_interface<grapheme_view<V>>
    // clang-format on
    {
        V base_ = V();

    public:
        constexpr grapheme_view() requires std::default_initializable<V> = default;
        constexpr grapheme_view(V base) : base_{std::move(base)} {}

        constexpr V base() const & requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() const {
            return grapheme_iterator(
                std::ranges::begin(base_),
                std::ranges::begin(base_),
                std::ranges::end(base_));
        }
        constexpr auto end() const
        {
            if constexpr (std::ranges::common_range<V>) {
                return grapheme_iterator(
                    std::ranges::begin(base_),
                    std::ranges::end(base_),
                    std::ranges::end(base_));
            } else {
                return std::ranges::end(base_);
            }
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, grapheme_view v)
        {
            boost::text::transcode_to_utf8(
                v.begin().base(),
                v.end().base(),
                std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, grapheme_view v)
        {
            boost::text::transcode_to_utf16(
                v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            return os;
        }
#endif
    };

    template<class R>
    grapheme_view(R &&) -> grapheme_view<std::views::all_t<R>>;
}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    namespace dtl {
        template<class R>
        concept can_grapheme_view = requires { grapheme_view(std::declval<R &>()); };

        template<class R>
        constexpr decltype(auto) unpack_range_to_utf32(R && r)
        {
            using T = std::remove_cvref_t<R>;
            if constexpr (utf32_range<T>) {
                return std::forward<R>(r);
            } else if constexpr (std::ranges::forward_range<T>) {
                auto unpacked = boost::text::unpack_iterator_and_sentinel(
                    std::ranges::begin(r), std::ranges::end(r));
                return std::ranges::subrange(unpacked.first, unpacked.last) |
                       as_utf32;
            } else {
                return std::forward<R>(r) | as_utf32;
            }
        }

        template<class R>
        using unpacked_range_utf32 =
            decltype(dtl::unpack_range_to_utf32(std::declval<R>()));

        struct as_graphemes_impl : range_adaptor_closure<as_graphemes_impl>
        {
            template<class R>
                requires(std::ranges::viewable_range<R> &&
                         can_grapheme_view<unpacked_range_utf32<R>>) ||
                        utf_pointer<std::remove_cvref_t<R>>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (dtl::is_empty_view<T>) {
                    return r;
                } else if constexpr (std::is_pointer_v<T>) {
                    return grapheme_view(
                        std::ranges::subrange(r, null_sentinel) | as_utf32);
                } else {
                    return grapheme_view(
                        dtl::unpack_range_to_utf32(std::forward<R>(r)));
                }
            }
        };
    }

    inline constexpr dtl::as_graphemes_impl as_graphemes;

}}}

#endif

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::grapheme_view<V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
