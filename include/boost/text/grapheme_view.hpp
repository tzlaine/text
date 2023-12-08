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
#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::view V>
    // clang-format off
        requires utf32_range<V>
#else
    template<typename V>
#endif
    class grapheme_view : public stl_interfaces::view_interface<grapheme_view<V>>
    // clang-format on
    {
        V base_ = V();

    public:
        constexpr grapheme_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr grapheme_view(V base) : base_{std::move(base)} {}

        constexpr V base() const &
#if BOOST_TEXT_USE_CONCEPTS
            requires std::copy_constructible<V>
#endif
        { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() const {
            return grapheme_iterator(
                detail::begin(base_),
                detail::begin(base_),
                detail::end(base_));
        }
        constexpr auto end() const
        {
            if constexpr (detail::common_range_v<V>) {
                return grapheme_iterator(
                    detail::begin(base_),
                    detail::end(base_),
                    detail::end(base_));
            } else {
                return detail::end(base_);
            }
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, grapheme_view v)
        {
            std::ostreambuf_iterator<char> it(os);
            for (auto c : v.base_ | as_utf8) {
                *it = c;
                ++it;
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, grapheme_view v)
        {
            std::ostreambuf_iterator<char> it(os);
            for (auto c : v.base_ | as_utf16) {
                *it = c;
                ++it;
            }
            return os;
        }
#endif
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<class R>
    grapheme_view(R &&) -> grapheme_view<std::views::all_t<R>>;
#endif

    namespace detail {
#if BOOST_TEXT_USE_CONCEPTS
        template<class R>
        concept can_grapheme_view = requires { grapheme_view(std::declval<R &>()); };
#endif

        template<class R>
        constexpr decltype(auto) unpack_range_to_utf32(R && r)
        {
            using T = remove_cv_ref_t<R>;
            if constexpr (utf32_range_v<T>) {
                return std::forward<R>(r);
            } else if constexpr (forward_range_v<T>) {
                auto unpacked = boost::text::unpack_iterator_and_sentinel(
                    detail::begin(r), detail::end(r));
#if BOOST_TEXT_USE_CONCEPTS
                return BOOST_TEXT_SUBRANGE(unpacked.first, unpacked.last) |
                       as_utf32;
#else
                return subrange{unpacked.first, unpacked.last} | as_utf32;
#endif
            } else {
                return std::forward<R>(r) | as_utf32;
            }
        }

        template<class R>
        using unpacked_range_utf32 =
            decltype(detail::unpack_range_to_utf32(std::declval<R>()));

        struct as_graphemes_impl : range_adaptor_closure<as_graphemes_impl>
        {
            template<class R>
#if BOOST_TEXT_USE_CONCEPTS
                requires(std::ranges::viewable_range<R> &&
                         can_grapheme_view<unpacked_range_utf32<R>>) ||
                        utf_pointer<std::remove_cvref_t<R>>
#endif
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = remove_cv_ref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
                    return r;
                } else if constexpr (std::is_pointer_v<T>) {
                    return grapheme_view(
#if BOOST_TEXT_USE_CONCEPTS
                        BOOST_TEXT_SUBRANGE(r, null_sentinel)
#else
                        subrange{r, null_sentinel}
#endif
                        | as_utf32);
                } else {
                    return grapheme_view(
                        detail::unpack_range_to_utf32(std::forward<R>(r)));
                }
            }
        };
    }

    inline constexpr detail::as_graphemes_impl as_graphemes;

}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::grapheme_view<V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
