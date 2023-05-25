// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/dangling.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>

#include <format>


namespace boost { namespace text {

    namespace detail {

        // UTF-8
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf8>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf8,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf16>, Iter f_, Sentinel l)
        {
            auto f = utf_16_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(tag_t<format::utf16>, Iter f_, Iter l_)
        {
            auto f = utf_16_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_8_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(tag_t<format::utf32>, Iter f_, Sentinel l)
        {
            auto f = utf_32_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(tag_t<format::utf32>, Iter f_, Iter l_)
        {
            auto f = utf_32_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_8_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf8,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }

        // UTF-16
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(tag_t<format::utf8>, Iter f_, Sentinel l)
        {
            auto f = utf_8_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(tag_t<format::utf8>, Iter f_, Iter l_)
        {
            auto f = utf_8_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_16_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(tag_t<format::utf16>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf16,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf16_range_(tag_t<format::utf32>, Iter f_, Sentinel l)
        {
            auto f = utf_32_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(tag_t<format::utf32>, Iter f_, Iter l_)
        {
            auto f = utf_32_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_16_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf16,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }

        // UTF-32
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(tag_t<format::utf8>, Iter f_, Sentinel l)
        {
            auto f = utf_8_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(tag_t<format::utf8>, Iter f_, Iter l_)
        {
            auto f = utf_8_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_32_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf32_range_(tag_t<format::utf16>, Iter f_, Sentinel l)
        {
            auto f = utf_16_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                Sentinel,
                no_op_repacker>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(tag_t<format::utf16>, Iter f_, Iter l_)
        {
            auto f = utf_16_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_32_iterator<Iter>(f_, l_, l_);
            return utf_tagged_range<
                format::utf32,
                decltype(f),
                decltype(f),
                no_op_repacker>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(tag_t<format::utf32>, Iter f, Sentinel l)
        {
            return utf_tagged_range<
                format::utf32,
                Iter,
                Sentinel,
                no_op_repacker>{f, l};
        }

#if BOOST_TEXT_USE_CONCEPTS
        template<typename ResultType, typename I1, typename I2, typename S>
        static auto make_iter(I1 first, I2 it, S last)
        {
            if constexpr (requires { ResultType(first, it, last); }) {
                return ResultType{first, it, last};
            } else {
                return it;
            }
        }
#else
        template<typename ResultType, typename Iterator, typename Sentinel>
        constexpr auto
        make_iter(Iterator first, Iterator it, Sentinel last)
            -> decltype(ResultType(first, it, last))
        {
            return ResultType(first, it, last);
        }
        template<typename ResultType>
        constexpr auto
        make_iter(ResultType first, ResultType it, ResultType last)
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Sentinel>
        constexpr auto
        make_iter(ResultType first, ResultType it, Sentinel last)
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Iterator>
        constexpr auto
        make_iter(Iterator first, ResultType it, ResultType last)
            -> decltype(ResultType(it))
        {
            return it;
        }
#endif
    }

    template<format Format, typename V>
    class utf_view : public stl_interfaces::view_interface<utf_view<Format, V>>
    {
        V base_ = V();

    public:
        constexpr utf_view() {}
        constexpr utf_view(V base) : base_{base} {}

        constexpr V base() const & requires std::copy_constructible<V>
        {
            return base_;
        }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() const
        {
            constexpr format from_format =
                detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr (from_format == Format) {
                return std::ranges::begin(base_);
            } else {
                return utf_iterator<
                    from_format,
                    Format,
                    std::ranges::iterator_t<V>,
                    std::ranges::sentinel_t<V>>{
                    std::ranges::begin(base_),
                    std::ranges::begin(base_),
                    std::ranges::end(base_)};
            }
        }
        constexpr auto end() const
        {
            constexpr format from_format =
                detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr (
                from_format == Format || !std::ranges::common_range<V>) {
                return std::ranges::end(base_);
            } else {
                return utf_iterator<
                    from_format,
                    Format,
                    std::ranges::iterator_t<V>,
                    std::ranges::sentinel_t<V>>{
                    std::ranges::begin(base_),
                    std::ranges::end(base_),
                    std::ranges::end(base_)};
            }
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, utf_view v)
        {
            if constexpr (Format == format::utf8) {
                auto out = std::ostreambuf_iterator<char>(os);
                for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                    *out = *it;
                }
            } else {
                boost::text::transcode_to_utf8(
                    v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, utf_view v)
        {
            if constexpr (Format == format::utf16) {
                auto out = std::ostreambuf_iterator<wchar_t>(os);
                for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                    *out = *it;
                }
            } else {
                boost::text::transcode_to_utf16(
                    v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            }
            return os;
        }
#endif
    };

    template<utf_range V>
        requires std::ranges::view<V>
    struct utf8_view : utf_view<format::utf8, V>
    {
        constexpr utf8_view() = default;
        constexpr utf8_view(V base) :
            utf_view<format::utf8, V>{std::move(base)}
        {}
    };
    template<utf_range V>
        requires std::ranges::view<V>
    struct utf16_view : utf_view<format::utf16, V>
    {
        constexpr utf16_view() = default;
        constexpr utf16_view(V base) :
            utf_view<format::utf16, V>{std::move(base)}
        {}
    };
    template<utf_range V>
        requires std::ranges::view<V>
    struct utf32_view : utf_view<format::utf32, V>
    {
        constexpr utf32_view() = default;
        constexpr utf32_view(V base) :
            utf_view<format::utf32, V>{std::move(base)}
        {}
    };

    template<typename R>
    utf8_view(R &&) -> utf8_view<std::views::all_t<R>>;
    template<typename R>
    utf16_view(R &&) -> utf16_view<std::views::all_t<R>>;
    template<typename R>
    utf32_view(R &&) -> utf32_view<std::views::all_t<R>>;

}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** A view adaptor that produces a UTF-8 view of the given view. */
    constexpr detail::unspecified as_utf8;

    /** A view adaptor that produces a UTF-16 view of the given view. */
    constexpr detail::unspecified as_utf16;

    /** A view adaptor that produces a UTF-32 view of the given view. */
    constexpr detail::unspecified as_utf32;

#endif

    namespace dtl {
        template<typename T>
        struct is_empty_view : std::false_type
        {};
        template<typename T>
        struct is_empty_view<std::ranges::empty_view<T>> : std::true_type
        {};
        template<typename T>
        constexpr bool is_empty_view_v = is_empty_view<T>::value;

        template<typename R, template<typename> typename View>
        concept can_utf_view = requires { View(std::declval<R &>()); };

        template<template<typename> typename View, format Format>
        struct as_utf_impl : range_adaptor_closure<as_utf_impl<View, Format>>
        {
            template<typename R>
                requires(std::ranges::viewable_range<R> &&
                         can_utf_view<std::remove_cvref_t<R>, View>) ||
                        utf_pointer<std::remove_cvref_t<R>>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (dtl::is_empty_view_v<T>) {
                    return r;
                } else if constexpr (std::is_pointer_v<T>) {
                    auto unpacked = boost::text::unpack_iterator_and_sentinel(
                        r, null_sentinel);
                    return View(
                        std::ranges::subrange(unpacked.first, unpacked.last));
                } else {
                    auto unpacked = boost::text::unpack_iterator_and_sentinel(
                        std::ranges::begin(r), std::ranges::end(r));
                    return View(
                        std::ranges::subrange(unpacked.first, unpacked.last));
                }
            }
        };
    }

    inline constexpr dtl::as_utf_impl<utf8_view, format::utf8> as_utf8;
    inline constexpr dtl::as_utf_impl<utf16_view, format::utf16> as_utf16;
    inline constexpr dtl::as_utf_impl<utf32_view, format::utf32> as_utf32;
}}}

namespace std::ranges {
template<boost::text::format Format, typename V>
inline constexpr bool enable_borrowed_range<boost::text::utf_view<Format, V>> =
    enable_borrowed_range<V>;
}

#endif

#endif
