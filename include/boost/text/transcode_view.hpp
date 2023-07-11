// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>

#include <climits>
#include <format>


namespace boost { namespace text {

    namespace detail {
        template<class I>
        constexpr auto iterator_to_tag()
        {
            if constexpr (std::random_access_iterator<I>) {
                return std::random_access_iterator_tag{};
            } else if constexpr (std::bidirectional_iterator<I>) {
                return std::bidirectional_iterator_tag{};
            } else if constexpr (std::forward_iterator<I>) {
                return std::forward_iterator_tag{};
            } else {
                return std::input_iterator_tag{};
            }
        }
        template<class I>
        using iterator_to_tag_t = decltype(iterator_to_tag<I>());

        template<class Char>
        struct cast_to_charn {
            constexpr Char operator()(Char c) const { return c; }
        };
    }

    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> // TODO &&
                 // TODO @*can-reference*@<invoke_result_t<decltype(F)&, ranges::range_reference_t<V>>>
    class project_view : public std::ranges::view_interface<project_view<V, F>>
    {
        V base_ = V();

        template<bool Const>
        class iterator;
        template<bool Const>
        class sentinel;

    public:
        constexpr project_view() requires std::default_initializable<V> = default;
        constexpr explicit project_view(V base) : base_(std::move(base)) {}

        constexpr V& base() & { return base_; }
        constexpr const V& base() const& { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr iterator<false> begin() { return iterator<false>{std::ranges::begin(base_)}; }
        constexpr iterator<true> begin() const requires std::ranges::range<const V>
            { return iterator<true>{std::ranges::begin(base_)}; }

        constexpr sentinel<false> end() { return sentinel<false>{std::ranges::end(base_)}; }
        constexpr iterator<false> end() requires std::ranges::common_range<V> { return iterator<false>{std::ranges::end(base_)}; }
        constexpr sentinel<true> end() const requires std::ranges::range<const V> { return sentinel<true>{std::ranges::end(base_)}; }
        constexpr iterator<true> end() const requires std::ranges::common_range<const V>
            { return iterator<true>{std::ranges::end(base_)}; }

        constexpr auto size() requires std::ranges::sized_range<V> { return std::ranges::size(base_); }
        constexpr auto size() const requires std::ranges::sized_range<const V> { return std::ranges::size(base_); }
    };

    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> // TODO &&
                 // TODO @*can-reference*@<invoke_result_t<decltype(F)&, ranges::range_reference_t<V>>>
    template<bool Const>
    class project_view<V, F>::iterator
        : public boost::stl_interfaces::proxy_iterator_interface<
              iterator<Const>, // TODO
              detail::iterator_to_tag_t<std::ranges::iterator_t<detail::maybe_const<Const, V>>>,
              std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<V>>>
    {
        using iterator_type = std::ranges::iterator_t<detail::maybe_const<Const, V>>;
        using reference_type = std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<V>>;

        friend boost::stl_interfaces::access;
        iterator_type & base_reference() noexcept { return it_; }
        iterator_type base_reference() const { return it_; }

        iterator_type it_ = iterator_type();

        friend project_view<V, F>::sentinel<Const>;

    public:
        constexpr iterator() = default;
        constexpr iterator(iterator_type it) : it_(std::move(it)) {}

        constexpr reference_type operator*() const { return F(*it_); }
    };

    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> // TODO &&
                 // TODO @*can-reference*@<invoke_result_t<decltype(F)&, ranges::range_reference_t<V>>>
    template<bool Const>
    class project_view<V, F>::sentinel
    {
        using Base = detail::maybe_const<Const, V>;
        using sentinel_type = std::ranges::sentinel_t<Base>;

        sentinel_type end_ = sentinel_type();

    public:
        constexpr sentinel() = default;
        constexpr explicit sentinel(sentinel_type end) : end_(std::move(end)) {}
        constexpr sentinel(sentinel<!Const> i) requires Const
            && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>;

        constexpr sentinel_type base() const { return end_; }

        template<bool OtherConst>
            requires std::sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
        friend constexpr bool operator==(const iterator<OtherConst> & x,
                                         const sentinel & y)
            { return x.it_ == y.end_; }

        template<bool OtherConst>
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
        friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const iterator<OtherConst> & x, const sentinel & y)
            { return x.it_ - y.end_; }

        template<bool OtherConst>
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
        friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const sentinel & y, const iterator<OtherConst> & x)
            { return y.end_ - x.it_; }
    };

    template<class R, auto F>
    project_view(R &&) -> project_view<std::views::all_t<R>, F>;

    namespace detail {
        template<auto F>
            requires std::is_object_v<decltype(F)>
        struct project_impl : range_adaptor_closure<project_impl<F>>
        {
            template<class R>
                requires std::ranges::viewable_range<R> &&
                         std::ranges::input_range<R> &&
                         std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<R>> // TODO &&
                         // TODO @*can-reference*@<invoke_result_t<decltype(F)&, ranges::range_reference_t<R>>>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (std::is_rvalue_reference_v<R &&>) {
                    return project_view<std::views::all_t<T>, F>{std::forward<R>(r)};
                } else {
                    return project_view<T, F>{std::forward<R>(r)};
                }
            }
        };
    }

    template<auto F>
    constexpr detail::project_impl<F> project;

    template<class V>
    using char8_view = project_view<V, detail::cast_to_charn<char8_t>{}>;
    template<class V>
    using char16_view = project_view<V, detail::cast_to_charn<char16_t>{}>;
    template<class V>
    using char32_view = project_view<V, detail::cast_to_charn<char32_t>{}>;

    namespace detail {
        template<template<class> class View, format Format>
        struct as_charn_impl : range_adaptor_closure<as_charn_impl<View, Format>>
        {
            template<class R>
            requires (std::ranges::viewable_range<R> &&
                      std::ranges::input_range<R> &&
                      std::convertible_to<std::ranges::range_reference_t<R>, dtl::format_to_type_t<Format>>) ||
                     utf_pointer<std::remove_cvref_t<R>>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
                    return std::ranges::empty_view<dtl::format_to_type_t<Format>>{};
                } else if constexpr (std::is_pointer_v<T>) {
                    return View(std::ranges::subrange(r, null_sentinel));
                } else {
                    return View(std::forward<R>(r));
                }
            }
        };

        template<class T>
        constexpr bool is_charn_view = false;
        template<class V>
        constexpr bool is_charn_view<char8_view<V>> = true;
        template<class V>
        constexpr bool is_charn_view<char16_view<V>> = true;
        template<class V>
        constexpr bool is_charn_view<char32_view<V>> = true;
    }

    inline constexpr detail::as_charn_impl<char8_view, format::utf8> as_char8_t;
    inline constexpr detail::as_charn_impl<char16_view, format::utf16> as_char16_t;
    inline constexpr detail::as_charn_impl<char32_view, format::utf32> as_char32_t;

    // clang-format off
    template<utf_range V>
    requires std::ranges::view<V>
    class unpacking_view : public std::ranges::view_interface<unpacking_view<V>> {
      V base_ = V();

    public:
      constexpr unpacking_view() requires std::default_initializable<V> = default;
      constexpr unpacking_view(V base) : base_(std::move(base)) {}

      constexpr V base() const & requires std::copy_constructible<V> { return base_; }
      constexpr V base() && { return std::move(base_); }

      constexpr auto code_units() const noexcept {
        auto unpacked = boost::text::unpack_iterator_and_sentinel(std::ranges::begin(base_), std::ranges::end(base_));
        return std::ranges::subrange(unpacked.first, unpacked.last);
      }

      constexpr auto begin() { return std::ranges::begin(code_units()); }
      constexpr auto begin() const { return std::ranges::begin(code_units()); }

      constexpr auto end() { return std::ranges::end(code_units()); }
      constexpr auto end() const { return std::ranges::end(code_units()); }
    };

    template<class R>
    unpacking_view(R &&) -> unpacking_view<std::views::all_t<R>>;
    // clang-format on

    template<format Format, utf_range V>
        requires std::ranges::view<V>
    class utf_view : public std::ranges::view_interface<utf_view<Format, V>>
    {
        V base_ = V();

        template<format FromFormat, class I, class S>
        static constexpr auto make_begin(I first, S last)
        {
            if constexpr (std::bidirectional_iterator<I>) {
                return utf_iterator<FromFormat, Format, I, S>{first, first, last};
            } else {
                return utf_iterator<FromFormat, Format, I, S>{first, last};
            }
        }
        template<format FromFormat, class I, class S>
        static constexpr auto make_end(I first, S last)
        {
            if constexpr (!std::same_as<I, S>) {
                return last;
            } else if constexpr (std::bidirectional_iterator<I>) {
                return utf_iterator<FromFormat, Format, I, S>{first, last, last};
            } else {
                return utf_iterator<FromFormat, Format, I, S>{last, last};
            }
        }

    public:
        constexpr utf_view() requires std::default_initializable<V> = default;
        constexpr utf_view(V base) : base_{std::move(base)} {}

        constexpr V base() const & requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin()
        {
            constexpr format from_format = detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_begin<from_format>(std::ranges::begin(base_.base()), std::ranges::end(base_.base()));
            } else {
                return make_begin<from_format>(std::ranges::begin(base_), std::ranges::end(base_));
            }
        }
        constexpr auto begin() const
        {
            constexpr format from_format = detail::format_of<std::ranges::range_value_t<const V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_begin<from_format>(std::ranges::begin(base_.base()), std::ranges::end(base_.base()));
            } else {
                return make_begin<from_format>(std::ranges::begin(base_), std::ranges::end(base_));
            }
        }

        constexpr auto end()
        {
            constexpr format from_format = detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_end<from_format>(std::ranges::begin(base_.base()), std::ranges::end(base_.base()));
            } else {
                return make_end<from_format>(std::ranges::begin(base_), std::ranges::end(base_));
            }
        }
        constexpr auto end() const
        {
            constexpr format from_format = detail::format_of<std::ranges::range_value_t<const V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_end<from_format>(std::ranges::begin(base_.base()), std::ranges::end(base_.base()));
            } else {
                return make_end<from_format>(std::ranges::begin(base_), std::ranges::end(base_));
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


    template<format Format, class R>
    utf_view(R &&) -> utf_view<Format, std::views::all_t<R>>;

    template<class V>
    using utf8_view = utf_view<format::utf8, V>;
    template<class V>
    using utf16_view = utf_view<format::utf16, V>;
    template<class V>
    using utf32_view = utf_view<format::utf32, V>;

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
        template<class R, template<class> class View>
        concept can_utf_view = requires { View(std::declval<R>()); };

        template<class T>
        constexpr bool is_utf_view = false;
        template<class T>
        constexpr bool is_utf_view<utf8_view<T>> = true;
        template<class T>
        constexpr bool is_utf_view<utf16_view<T>> = true;
        template<class T>
        constexpr bool is_utf_view<utf32_view<T>> = true;
        template<format F, class T>
        constexpr bool is_utf_view<utf_view<F, T>> = true;

        template<class R>
        constexpr decltype(auto) unpack_range(R && r)
        {
            using T = std::remove_cvref_t<R>;
            if constexpr (std::ranges::forward_range<T>) {
                auto unpacked =
                    boost::text::unpack_iterator_and_sentinel(std::ranges::begin(r), std::ranges::end(r));
                if constexpr (std::is_bounded_array_v<T>) {
                    constexpr auto n = std::extent_v<T>;
                    if (n && !r[n - 1])
                        --unpacked.last;
                    return std::ranges::subrange(unpacked.first, unpacked.last);
                } else if constexpr (
                    !std::same_as<decltype(unpacked.first), std::ranges::iterator_t<R>> ||
                    !std::same_as<decltype(unpacked.last), std::ranges::sentinel_t<R>>) {
                    return unpacking_view(std::forward<R>(r));
                } else {
                    return std::forward<R>(r);
                }
            } else {
                return std::forward<R>(r);
            }
        }

        template<class R>
        using unpacked_range = decltype(dtl::unpack_range(std::declval<R>()));

        template<template<class> class View, format Format>
        struct as_utf_impl : range_adaptor_closure<as_utf_impl<View, Format>>
        {
            template<class R>
                requires is_utf_view<std::remove_cvref_t<R>> ||
                         (std::ranges::viewable_range<R> &&
                          can_utf_view<unpacked_range<R>, View>) ||
                         utf_pointer<std::remove_cvref_t<R>>
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = std::remove_cvref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
                    return std::ranges::empty_view<dtl::format_to_type_t<Format>>{};
                } else if constexpr (is_utf_view<T>) {
                    return View(std::forward<R>(r).base());
                } else if constexpr (detail::is_charn_view<T>) {
                    return View(std::forward<R>(r));
                } else if constexpr (std::is_pointer_v<T>) {
                    return View(std::ranges::subrange(r, null_sentinel));
                } else {
                    return View(dtl::unpack_range(std::forward<R>(r)));
                }
            }
        };

        template<class T>
        constexpr bool is_utf32_view = false;
        template<class V>
        constexpr bool is_utf32_view<utf_view<format::utf32, V>> = true;
    }

    inline constexpr dtl::as_utf_impl<utf8_view, format::utf8> as_utf8;
    inline constexpr dtl::as_utf_impl<utf16_view, format::utf16> as_utf16;
    inline constexpr dtl::as_utf_impl<utf32_view, format::utf32> as_utf32;

}}}

namespace std::ranges {
    template<class V, auto F>
    inline constexpr bool enable_borrowed_range<boost::text::project_view<V, F>> =
        enable_borrowed_range<V>;

    template<boost::text::format Format, class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf_view<Format, V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
