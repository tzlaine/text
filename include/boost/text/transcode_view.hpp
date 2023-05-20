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

        template<typename I, typename S, format FromFormat, format ToFormat>
        struct transcoding_iterator;
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf8, format::utf16>
        {
            using type = utf_8_to_16_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf8, format::utf32>
        {
            using type = utf_8_to_32_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf16, format::utf8>
        {
            using type = utf_16_to_8_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf16, format::utf32>
        {
            using type = utf_16_to_32_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf32, format::utf8>
        {
            using type = utf_32_to_8_iterator<I, S>;
        };
        template<typename I, typename S>
        struct transcoding_iterator<I, S, format::utf32, format::utf16>
        {
            using type = utf_32_to_16_iterator<I, S>;
        };
        template<typename I, typename S, format FromFormat, format ToFormat>
        using transcoding_iterator_t =
            typename transcoding_iterator<I, S, FromFormat, ToFormat>::type;

        template<format Format, typename I, typename S>
        constexpr auto make_utf_view_iter(I first, S last)
        {
            constexpr format from_format =
                detail::format_of<std::iter_value_t<I>>();
            if constexpr (from_format == Format) {
                return first;
            } else {
                return transcoding_iterator_t<I, S, from_format, Format>{
                    first, first, last};
            }
        }
        template<format Format, typename I, typename S>
        constexpr auto make_utf_view_sent(I first, S last)
        {
            constexpr format from_format =
                detail::format_of<std::iter_value_t<I>>();
            if constexpr (from_format == Format || !std::is_same_v<I, S>) {
                return last;
            } else {
                return transcoding_iterator_t<I, S, from_format, Format>{
                    first, last, last};
            }
        }
    }

    template<format Format, typename V>
    struct utf_view : stl_interfaces::view_interface<utf_view<Format, V>>
    {
        using from_iterator = std::ranges::iterator_t<V>;
        using from_sentinel = std::ranges::sentinel_t<V>;

        using iterator = decltype(detail::make_utf_view_iter<Format>(
            std::declval<from_iterator>(), std::declval<from_sentinel>()));
        using sentinel = decltype(detail::make_utf_view_sent<Format>(
            std::declval<from_iterator>(), std::declval<from_sentinel>()));

        constexpr utf_view() {}
        constexpr utf_view(V base) : base_{base} {}

        constexpr V base() const & requires std::copy_constructible<V>
        {
            return base_;
        }
        constexpr V base() && { return std::move(base_); }

        constexpr iterator begin() const
        {
            return detail::make_utf_view_iter<Format>(
                std::ranges::begin(base_), std::ranges::end(base_));
        }
        constexpr sentinel end() const
        {
            return detail::make_utf_view_sent<Format>(
                std::ranges::begin(base_), std::ranges::end(base_));
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

    private:
        V base_;
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

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf8_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf8_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf8_impl : range_adaptor_closure<as_utf8_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked =
                    text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf8_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf8, decltype(subrange)>(subrange);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf8_dispatch<as_utf8_impl, Range &&>::call(
                    (Range &&) r))
            {
                return dtl::as_utf8_dispatch<as_utf8_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf8_impl as_utf8;
#else
    namespace {
        constexpr dtl::as_utf8_impl as_utf8;
    }
#endif

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf16_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf16_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf16_impl : range_adaptor_closure<as_utf16_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked = text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf16_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf16, decltype(subrange)>(subrange);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf16_dispatch<as_utf16_impl, Range &&>::
                                call((Range &&) r))
            {
                return dtl::as_utf16_dispatch<as_utf16_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf16_impl as_utf16;
#else
    namespace {
        constexpr dtl::as_utf16_impl as_utf16;
    }
#endif

    namespace dtl {
        template<
            typename Impl,
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_utf32_dispatch
        {
            static constexpr auto call(Range && r)
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf32_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p)
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf32_impl : range_adaptor_closure<as_utf32_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked =
                    text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                auto subrange = std::ranges::subrange(r.first, r.last);
                return utf_view<format::utf32, decltype(subrange)>(subrange);
           }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_utf32_dispatch<as_utf32_impl, Range &&>::
                                call((Range &&) r))
            {
                return dtl::as_utf32_dispatch<as_utf32_impl, Range &&>::call(
                    (Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_utf32_impl as_utf32;
#else
    namespace {
        constexpr dtl::as_utf32_impl as_utf32;
    }
#endif

}}}

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
