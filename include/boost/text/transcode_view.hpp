// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/subrange.hpp>
#include <boost/text/transcode_algorithm.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>

#include <climits>


namespace boost { namespace text {

    namespace detail {
        template<class I>
        constexpr auto iterator_to_tag()
        {
#if BOOST_TEXT_USE_CONCEPTS
            if constexpr (std::random_access_iterator<I>) {
                return std::random_access_iterator_tag{};
            } else if constexpr (std::bidirectional_iterator<I>) {
                return std::bidirectional_iterator_tag{};
            } else if constexpr (std::forward_iterator<I>) {
#else
            if constexpr (detail::random_access_iterator_v<I>) {
                return std::random_access_iterator_tag{};
            } else if constexpr (detail::bidirectional_iterator_v<I>) {
                return std::bidirectional_iterator_tag{};
            } else if constexpr (detail::forward_iterator_v<I>) {
#endif
                return std::forward_iterator_tag{};
            } else {
                return std::input_iterator_tag{};
            }
        }
        template<class I>
        using iterator_to_tag_t = decltype(iterator_to_tag<I>());

#if BOOST_TEXT_USE_CONCEPTS
        template<class T>
        using with_reference = T &;
        template<typename T>
        concept can_reference = requires { typename with_reference<T>; };
#endif

#if BOOST_TEXT_USE_CONCEPTS
        template<class Char>
        struct cast_to_charn {
            constexpr Char operator()(Char c) const { return c; }
        };
#else
        struct cast_to_char8;
        struct cast_to_char16;
        struct cast_to_char32;
        template<typename Tag, typename Arg>
        auto function_for_tag(Arg arg)
        {
#if defined(__cpp_char8_t)
            if constexpr (std::is_same_v<Tag, cast_to_char8>) {
                return (char8_t)arg;
            } else
#endif
                if constexpr (std::is_same_v<Tag, cast_to_char16>) {
                return (char16_t)arg;
            } else if constexpr (std::is_same_v<Tag, cast_to_char32>) {
                return (char32_t)arg;
            }
        }
#endif
    }

#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> &&
                 detail::can_reference<std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<V>>>
#else
    template<typename V, typename F> // F is a tag type in c++17
#endif
    class project_view : public stl_interfaces::view_interface<project_view<V, F>>
    {
        V base_ = V();

        template<bool Const>
        class iterator;
        template<bool Const>
        class sentinel;

    public:
        constexpr project_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr explicit project_view(V base) : base_(std::move(base)) {}

        constexpr V& base() & { return base_; }
        constexpr const V& base() const& { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr iterator<false> begin() { return iterator<false>{detail::begin(base_)}; }
        constexpr iterator<true> begin() const
#if BOOST_TEXT_USE_CONCEPTS
            requires std::ranges::range<const V>
#endif
        { return iterator<true>{detail::begin(base_)}; }

        constexpr sentinel<false> end() { return sentinel<false>{detail::end(base_)}; }
#if BOOST_TEXT_USE_CONCEPTS
        constexpr iterator<false> end() requires std::ranges::common_range<V>
            { return iterator<false>{detail::end(base_)}; }
#endif
        constexpr sentinel<true> end() const
#if BOOST_TEXT_USE_CONCEPTS
            requires std::ranges::range<const V>
        { return sentinel<true>{detail::end(base_)}; }
        constexpr iterator<true> end() const
            requires std::ranges::common_range<const V>
#endif
        { return iterator<true>{detail::end(base_)}; }

#if BOOST_TEXT_USE_CONCEPTS
        constexpr auto size() requires std::ranges::sized_range<V> { return std::ranges::size(base_); }
        constexpr auto size() const requires std::ranges::sized_range<const V> { return std::ranges::size(base_); }
#endif
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> &&
                 detail::can_reference<std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<V>>>
#else
    template<typename V, typename F>
#endif
    template<bool Const>
    class project_view<V, F>::iterator
        : public boost::stl_interfaces::proxy_iterator_interface<
              iterator<Const>, // TODO
              detail::iterator_to_tag_t<detail::iterator_t<detail::maybe_const<Const, V>>>,
#if BOOST_TEXT_USE_CONCEPTS
              std::invoke_result_t<decltype(F)&, detail::range_reference_t<V>>
#else
              decltype(detail::function_for_tag<F>(0))
#endif
        >
    {
        using iterator_type = detail::iterator_t<detail::maybe_const<Const, V>>;
        using sentinel_type = detail::sentinel_t<detail::maybe_const<Const, V>>;
        using reference_type =
#if BOOST_TEXT_USE_CONCEPTS
            std::invoke_result_t<decltype(F) &, detail::range_reference_t<V>>
#else
            decltype(detail::function_for_tag<F>(0))
#endif
            ;
        using sentinel = project_view<V, F>::sentinel<Const>;

        friend boost::stl_interfaces::access;
        iterator_type & base_reference() noexcept { return it_; }
        iterator_type base_reference() const { return it_; }

        iterator_type it_ = iterator_type();

        friend project_view<V, F>::sentinel<Const>;

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr bool operator==(const iterator<OtherConst> & x,
                                         const sentinel & y);

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr detail::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const iterator<OtherConst> & x, const sentinel & y);

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr detail::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const sentinel & y, const iterator<OtherConst> & x);

    public:
        constexpr iterator() = default;
        constexpr iterator(iterator_type it) : it_(std::move(it)) {}

#if BOOST_TEXT_USE_CONCEPTS
        constexpr reference_type operator*() const { return F(*it_); }
#else
        constexpr reference_type operator*() const
        {
            return detail::function_for_tag<F>(*it_);
        }
#endif
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V, auto F>
        requires std::ranges::view<V> &&
                 std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<V>> &&
                 detail::can_reference<std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<V>>>
#else
    template<typename V, typename F>
#endif
    template<bool Const>
    class project_view<V, F>::sentinel
    {
        using Base = detail::maybe_const<Const, V>;
        using sentinel_type = detail::sentinel_t<Base>;

        sentinel_type end_ = sentinel_type();

    public:
        constexpr sentinel() = default;
        constexpr explicit sentinel(sentinel_type end) : end_(std::move(end)) {}
#if !BOOST_TEXT_USE_CONCEPTS
        template<bool Enable = Const, class = std::enable_if_t<Enable>>
#endif
        constexpr sentinel(sentinel<!Const> i)
#if BOOST_TEXT_USE_CONCEPTS
            requires Const &&
            std::convertible_to<detail::sentinel_t<V>, detail::sentinel_t<Base>>
#endif
            : end_(std::move(i.end_))
        {}

        constexpr sentinel_type base() const { return end_; }

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr bool operator==(const iterator<OtherConst> & x,
                                         const sentinel & y)
            { return x.it_ == y.end_; }

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr detail::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const iterator<OtherConst> & x, const sentinel & y)
            { return x.it_ - y.end_; }

        template<bool OtherConst>
#if BOOST_TEXT_USE_CONCEPTS
            requires std::sized_sentinel_for<sentinel_type, std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
#endif
        friend constexpr detail::range_difference_t<detail::maybe_const<OtherConst, V>>
        operator-(const sentinel & y, const iterator<OtherConst> & x)
            { return y.end_ - x.it_; }
    };

#if BOOST_TEXT_USE_ALIAS_CTAD
    template<class R, auto F>
    project_view(R &&) -> project_view<std::views::all_t<R>, F>;
#endif

    namespace detail {
#if BOOST_TEXT_USE_CONCEPTS
        template<auto F>
#else
        template<typename F>
#endif
        struct project_impl : range_adaptor_closure<project_impl<F>>
        {
            template<class R>
            using project_view_type = project_view<R, F>;

#if BOOST_TEXT_USE_CONCEPTS
            template<class R>
                requires std::ranges::viewable_range<R> &&
                         std::ranges::input_range<R> &&
                         std::regular_invocable<decltype(F)&, std::ranges::range_reference_t<R>> &&
                         detail::can_reference<std::invoke_result_t<decltype(F)&, std::ranges::range_reference_t<R>>>
#else
            template<class R>
#endif
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
#if BOOST_TEXT_USE_ALIAS_CTAD
                return project_view_type(std::forward<R>(r));
#else
                return project_view_type<R>(std::forward<R>(r));
#endif
            }
        };
    }

#if BOOST_TEXT_USE_CONCEPTS
    template<auto F>
#else
    template<typename F>
#endif
    constexpr detail::project_impl<F> project;

#if BOOST_TEXT_USE_ALIAS_CTAD

    template<class V>
    using char8_view = project_view<V, detail::cast_to_charn<char8_t>{}>;
    template<class V>
    using char16_view = project_view<V, detail::cast_to_charn<char16_t>{}>;
    template<class V>
    using char32_view = project_view<V, detail::cast_to_charn<char32_t>{}>;

#else

#if defined(__cpp_char8_t)
#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V>
        requires std::ranges::view<V> && std::convertible_to<std::ranges::range_reference_t<V>, char8_t>
    class char8_view : public project_view<V, detail::cast_to_charn<char8_type>{}>
#else
    template<typename V>
    class char8_view : public project_view<V, detail::cast_to_char8>
#endif
    {
    public:
        constexpr char8_view() requires std::default_initializable<V> = default;
        constexpr char8_view(V base) :
#if BOOST_TEXT_USE_CONCEPTS
            project_view<V, detail::cast_to_charn<char8_t>{}>{std::move(base)}
#else
            project_view<V, detail::cast_to_char8>{std::move(base)}
#endif
        {}
    };
#endif
#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V>
        requires std::ranges::view<V> && std::convertible_to<std::ranges::range_reference_t<V>, char16_t>
    class char16_view : public project_view<V, detail::cast_to_charn<char16_t>{}>
#else
    template<typename V>
    class char16_view : public project_view<V, detail::cast_to_char16>
#endif
    {
    public:
        constexpr char16_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr char16_view(V base) :
#if BOOST_TEXT_USE_CONCEPTS
            project_view<V, detail::cast_to_charn<char16_t>{}>{std::move(base)}
#else
            project_view<V, detail::cast_to_char16>{std::move(base)}
#endif
        {}
    };
#if BOOST_TEXT_USE_CONCEPTS
    template<std::ranges::input_range V>
        requires std::ranges::view<V> && std::convertible_to<std::ranges::range_reference_t<V>, char32_t>
    class char32_view : public project_view<V, detail::cast_to_charn<char32_t>{}>
#else
    template<typename V>
    class char32_view : public project_view<V, detail::cast_to_char32>
#endif
    {
    public:
        constexpr char32_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr char32_view(V base) :
#if BOOST_TEXT_USE_CONCEPTS
            project_view<V, detail::cast_to_charn<char32_t>{}>{std::move(base)}
#else
            project_view<V, detail::cast_to_char32>{std::move(base)}
#endif
        {}
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<class R>
    char8_view(R &&) -> char8_view<std::views::all_t<R>>;
    template<class R>
    char16_view(R &&) -> char16_view<std::views::all_t<R>>;
    template<class R>
    char32_view(R &&) -> char32_view<std::views::all_t<R>>;
#endif

#endif

    namespace detail {
        template<template<class> class View, format Format>
        struct as_charn_impl : range_adaptor_closure<as_charn_impl<View, Format>>
        {
#if BOOST_TEXT_USE_CONCEPTS
            template<class R>
            requires (std::ranges::viewable_range<R> &&
                      std::ranges::input_range<R> &&
                      std::convertible_to<std::ranges::range_reference_t<R>, format_to_type_t<Format>>) ||
                     utf_pointer<std::remove_cvref_t<R>>
#else
            template<class R>
#endif
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = remove_cv_ref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
#if BOOST_TEXT_USE_CONCEPTS
                    return std::ranges::empty_view<format_to_type_t<Format>>{};
#else
                    return 42; // Never gonna happen.
#endif
                } else if constexpr (std::is_pointer_v<T>) {
#if BOOST_TEXT_USE_CONCEPTS
                    return View(std::ranges::subrange(r, null_sentinel));
#else
                    return View(subrange{r, null_sentinel});
#endif
                } else {
                    return View(std::forward<R>(r));
                }
            }
        };

        template<class T>
        constexpr bool is_charn_view = false;
#if BOOST_TEXT_USE_CONCEPTS
        template<class V>
        constexpr bool is_charn_view<char8_view<V>> = true;
#endif
        template<class V>
        constexpr bool is_charn_view<char16_view<V>> = true;
        template<class V>
        constexpr bool is_charn_view<char32_view<V>> = true;
    }

#if defined(__cpp_char8_t)
    inline constexpr detail::as_charn_impl<char8_view, format::utf8> as_char8_t;
#endif
    inline constexpr detail::as_charn_impl<char16_view, format::utf16> as_char16_t;
    inline constexpr detail::as_charn_impl<char32_view, format::utf32> as_char32_t;

    // clang-format off
#if BOOST_TEXT_USE_CONCEPTS
    template<utf_range V>
    requires std::ranges::view<V> && std::ranges::forward_range<V>
#else
    template<typename V>
#endif
    class unpacking_view : public stl_interfaces::view_interface<unpacking_view<V>> {
      V base_ = V();

    public:
      constexpr unpacking_view()
#if BOOST_TEXT_USE_CONCEPTS
          requires std::default_initializable<V>
#endif
      = default;
      constexpr unpacking_view(V base) : base_(std::move(base)) {}

      constexpr V base() const &
#if BOOST_TEXT_USE_CONCEPTS
          requires std::copy_constructible<V>
#endif
      { return base_; }
      constexpr V base() && { return std::move(base_); }

      constexpr auto code_units() const noexcept {
        auto unpacked = boost::text::unpack_iterator_and_sentinel(detail::begin(base_), detail::end(base_));
#if BOOST_TEXT_USE_CONCEPTS
        return std::ranges::subrange(unpacked.first, unpacked.last);
#else
        return subrange{unpacked.first, unpacked.last};
#endif
      }

      constexpr auto begin() { return code_units().begin(); }
      constexpr auto begin() const { return code_units().begin(); }

      constexpr auto end() { return code_units().end(); }
      constexpr auto end() const { return code_units().end(); }
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<class R>
    unpacking_view(R &&) -> unpacking_view<std::views::all_t<R>>;
#endif
    // clang-format on

#if BOOST_TEXT_USE_CONCEPTS
    template<format Format, utf_range V>
        requires std::ranges::view<V>
#else
    template<format Format, typename V/* TODO,
                                        typename Enable = std::enable_if_t<detail::utf_range_v<V>>*/>
#endif
    class utf_view : public stl_interfaces::view_interface<utf_view<Format, V>>
    {
        V base_ = V();

        template<format FromFormat, class I, class S>
        static constexpr auto make_begin(I first, S last)
        {
            if constexpr (detail::bidirectional_iterator_v<I>) {
                return utf_iterator<FromFormat, Format, I, S>{first, first, last};
            } else {
                return utf_iterator<FromFormat, Format, I, S>{first, last};
            }
        }
        template<format FromFormat, class I, class S>
        static constexpr auto make_end(I first, S last)
        {
            if constexpr (!std::is_same_v<I, S>) {
                return last;
            } else if constexpr (detail::bidirectional_iterator_v<I>) {
                return utf_iterator<FromFormat, Format, I, S>{first, last, last};
            } else {
                return utf_iterator<FromFormat, Format, I, S>{last, last};
            }
        }

    public:
        constexpr utf_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr utf_view(V base) : base_{std::move(base)} {}

        constexpr V base() const &
#if BOOST_TEXT_USE_CONCEPTS
            requires std::copy_constructible<V>
#endif
        { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin()
        {
            constexpr format from_format = detail::format_of<detail::range_value_t<V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_begin<from_format>(detail::begin(base_.base()), detail::end(base_.base()));
            } else {
                return make_begin<from_format>(detail::begin(base_), detail::end(base_));
            }
        }
        constexpr auto begin() const
        {
            constexpr format from_format = detail::format_of<detail::range_value_t<const V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_begin<from_format>(detail::begin(base_.base()), detail::end(base_.base()));
            } else {
                return make_begin<from_format>(detail::begin(base_), detail::end(base_));
            }
        }

        constexpr auto end()
        {
            constexpr format from_format = detail::format_of<detail::range_value_t<V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_end<from_format>(detail::begin(base_.base()), detail::end(base_.base()));
            } else {
                return make_end<from_format>(detail::begin(base_), detail::end(base_));
            }
        }
        constexpr auto end() const
        {
            constexpr format from_format = detail::format_of<detail::range_value_t<const V>>();
            if constexpr(detail::is_charn_view<V>) {
                return make_end<from_format>(detail::begin(base_.base()), detail::end(base_.base()));
            } else {
                return make_end<from_format>(detail::begin(base_), detail::end(base_));
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


#if BOOST_TEXT_USE_ALIAS_CTAD

    template<format Format, class R>
    utf_view(R &&) -> utf_view<Format, std::views::all_t<R>>;

    template<class V>
    using utf8_view = utf_view<format::utf8, V>;
    template<class V>
    using utf16_view = utf_view<format::utf16, V>;
    template<class V>
    using utf32_view = utf_view<format::utf32, V>;

#else

#if BOOST_TEXT_USE_CONCEPTS
    template<utf_range V>
        requires std::ranges::view<V>
#else
    template<typename V>
#endif
    class utf8_view : public utf_view<format::utf8, V>
    {
    public:
        constexpr utf8_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr utf8_view(V base) :
            utf_view<format::utf8, V>{std::move(base)}
        {}
    };
#if BOOST_TEXT_USE_CONCEPTS
    template<utf_range V>
        requires std::ranges::view<V>
#else
    template<typename V>
#endif
    class utf16_view : public utf_view<format::utf16, V>
    {
    public:
        constexpr utf16_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr utf16_view(V base) :
            utf_view<format::utf16, V>{std::move(base)}
        {}
    };
#if BOOST_TEXT_USE_CONCEPTS
    template<utf_range V>
        requires std::ranges::view<V>
#else
    template<typename V>
#endif
    class utf32_view : public utf_view<format::utf32, V>
    {
    public:
        constexpr utf32_view()
#if BOOST_TEXT_USE_CONCEPTS
            requires std::default_initializable<V>
#endif
        = default;
        constexpr utf32_view(V base) :
            utf_view<format::utf32, V>{std::move(base)}
        {}
    };

#if BOOST_TEXT_USE_CONCEPTS
    template<class R>
    utf8_view(R &&) -> utf8_view<std::views::all_t<R>>;
    template<class R>
    utf16_view(R &&) -> utf16_view<std::views::all_t<R>>;
    template<class R>
    utf32_view(R &&) -> utf32_view<std::views::all_t<R>>;
#endif

#endif

#if defined(BOOST_TEXT_DOXYGEN)

    /** A view adaptor that produces a UTF-8 view of the given view. */
    constexpr detail::unspecified as_utf8;

    /** A view adaptor that produces a UTF-16 view of the given view. */
    constexpr detail::unspecified as_utf16;

    /** A view adaptor that produces a UTF-32 view of the given view. */
    constexpr detail::unspecified as_utf32;

#endif

    namespace detail {
#if BOOST_TEXT_USE_CONCEPTS
        template<class R, template<class> class View>
        concept can_utf_view = requires { View(std::declval<R>()); };
#else
        template<class R, class View>
        using can_utf_view_expr = decltype(View(std::declval<R>()));
        template<class R, template<class> class View>
        constexpr bool can_utf_view =
            is_detected_v<can_utf_view_expr, R, View<R>>;
#endif

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

        template<typename T>
        constexpr bool is_bounded_array_v = false;
        template<typename T, int N>
        constexpr bool is_bounded_array_v<T[N]> = true;

        template<class R>
        constexpr decltype(auto) unpack_range(R && r)
        {
            using T = detail::remove_cv_ref_t<R>;
            if constexpr (forward_range_v<T>) {
                auto unpacked =
                    boost::text::unpack_iterator_and_sentinel(detail::begin(r), detail::end(r));
                if constexpr (is_bounded_array_v<T>) {
                    constexpr auto n = std::extent_v<T>;
                    if (n && !r[n - 1])
                        --unpacked.last;
#if BOOST_TEXT_USE_CONCEPTS
                    return std::ranges::subrange(unpacked.first, unpacked.last);
#else
                    return subrange{unpacked.first, unpacked.last};
#endif
                } else if constexpr (
                    !std::is_same_v<decltype(unpacked.first), iterator_t<R>> ||
                    !std::is_same_v<decltype(unpacked.last), sentinel_t<R>>) {
                    return unpacking_view(std::forward<R>(r));
                } else {
                    return std::forward<R>(r);
                }
            } else {
                return std::forward<R>(r);
            }
        }

        template<class R>
        using unpacked_range = decltype(detail::unpack_range(std::declval<R>()));

        template<template<class> class View, format Format>
        struct as_utf_impl : range_adaptor_closure<as_utf_impl<View, Format>>
        {
#if BOOST_TEXT_USE_CONCEPTS
            template<class R>
                requires is_utf_view<std::remove_cvref_t<R>> ||
                         (std::ranges::viewable_range<R> &&
                          can_utf_view<unpacked_range<R>, View>) ||
                         utf_pointer<std::remove_cvref_t<R>>
#else
            template<typename R>
#endif
            [[nodiscard]] constexpr auto operator()(R && r) const
            {
                using T = detail::remove_cv_ref_t<R>;
                if constexpr (detail::is_empty_view<T>) {
#if BOOST_TEXT_USE_CONCEPTS
                    return std::ranges::empty_view<format_to_type_t<Format>>{};
#else
                    return 42; // Never gonna happen.
#endif
                } else if constexpr (is_utf_view<T>) {
                    return View(std::forward<R>(r).base());
                } else if constexpr (detail::is_charn_view<T>) {
                    return View(std::forward<R>(r));
                } else if constexpr (std::is_pointer_v<T>) {
#if BOOST_TEXT_USE_CONCEPTS
                    return View(std::ranges::subrange(r, null_sentinel));
#else
                    return View(subrange{r, null_sentinel});
#endif
                } else {
                    return View(detail::unpack_range(std::forward<R>(r)));
                }
            }
        };

        template<class T>
        constexpr bool is_utf32_view = false;
        template<class V>
        constexpr bool is_utf32_view<utf_view<format::utf32, V>> = true;
    }

    inline constexpr detail::as_utf_impl<utf8_view, format::utf8> as_utf8;
    inline constexpr detail::as_utf_impl<utf16_view, format::utf16> as_utf16;
    inline constexpr detail::as_utf_impl<utf32_view, format::utf32> as_utf32;

}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<class V, auto F>
    inline constexpr bool enable_borrowed_range<boost::text::project_view<V, F>> =
        enable_borrowed_range<V>;

    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::unpacking_view<V>> =
        enable_borrowed_range<V>;

    template<boost::text::format Format, class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf_view<Format, V>> =
        enable_borrowed_range<V>;

#if !BOOST_TEXT_USE_ALIAS_CTAD
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf8_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf16_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf32_view<V>> =
        enable_borrowed_range<V>;
#endif
}

#endif

#endif
