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

    // TODO: This is a possible implementation, but it disables unpacking when
    // followed by an as_utfN adaptor.
#if 0
    template<format Format>
    struct as_charn_t_impl : range_adaptor_closure<as_charn_t_impl<Format>>
    {
        template<class R>
        requires (std::ranges::viewable_range<R> &&
                  std::integral<std::ranges::range_value_t<R>> &&
                  (int)ToFormat <= sizeof(std::ranges::range_value_t<R>)) ||
                 utf_pointer<std::remove_cvref_t<R>>
        [[nodiscard]] constexpr auto operator()(R && r) const
        {
            using char_type = dtl::format_to_type_t<ToFormat>;
            using T = std::remove_cvref_t<R>;
            if constexpr (detail::is_empty_view_v<T>) {
                return std::ranges::empty_view<dtl::format_to_type_t<Format>>{};
            } else if constexpr (std::is_pointer_v<T>) {
                return std::ranges::subrange(r, null_sentinel) |
                       std::views::transform(
                           [](auto c) { return static_cast<char_type>(c); });
            } else {
                return std::forward<R>(r) |
                       std::views::transform(
                           [](auto c) { return static_cast<char_type>(c); });
            }
        }
    };

    inline constexpr dtl::as_charn_t_impl<format::utf8> as_char8_t;
    inline constexpr dtl::as_charn_t_impl<format::utf16> as_char16_t;
    inline constexpr dtl::as_charn_t_impl<format::utf32> as_char32_t;
#endif

    namespace detail {
        template<class T>
        constexpr bool is_empty_view = false;
        template<class T>
        constexpr bool is_empty_view<std::ranges::empty_view<T>> = true;

        template<class I>
        constexpr auto iterator_to_tag()
        {
            if constexpr (std::contiguous_iterator<I>) {
                return std::contiguous_iterator_tag{};
            } else if constexpr (std::random_access_iterator<I>) {
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

        template<class I, class T>
        struct charn_projection_iterator
            : boost::stl_interfaces::proxy_iterator_interface<
                  charn_projection_iterator<I, T>, // TODO
                  iterator_to_tag_t<I>,
                  T>
        {
            constexpr charn_projection_iterator() = default;
            constexpr charn_projection_iterator(I it) : it_(std::move(it)) {}

            constexpr T operator*() const { return T(*it_); }

            friend constexpr bool operator==(charn_projection_iterator lhs, null_sentinel_t rhs)
                requires std::sentinel_for<null_sentinel_t, I>
            { return lhs.it_ == rhs; }

        private:
            friend boost::stl_interfaces::access;
            I & base_reference() noexcept { return it_; }
            I base_reference() const noexcept { return it_; }

            I it_;
        };
    }

    template<std::ranges::view V>
        requires std::convertible_to<std::ranges::range_value_t<V>, char8_t>
    class char8_view : public std::ranges::view_interface<char8_view<V>>
    {
        V base_ = V();

    public:
        using iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<V>, char8_t>;
        using const_iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<const V>, char8_t>;

        constexpr char8_view() requires std::default_initializable<V> = default;
        constexpr char8_view(V base) : base_{std::move(base)} {}

        constexpr V & base() & { return base_; }
        constexpr const V & base() const & { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() { return iterator{std::ranges::begin(base_)}; }
        constexpr auto end()
        {
            if constexpr (std::ranges::common_range<V>) {
                return iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
        constexpr auto begin() const { return const_iterator{std::ranges::begin(base_)}; }
        constexpr auto end() const
        {
            if constexpr (std::ranges::common_range<const V>) {
                return const_iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
    };

    template<std::ranges::view V>
        requires std::convertible_to<std::ranges::range_value_t<V>, char16_t>
    class char16_view : public std::ranges::view_interface<char16_view<V>>
    {
        V base_ = V();

    public:
        using iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<V>, char16_t>;
        using const_iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<const V>, char16_t>;

        constexpr char16_view() requires std::default_initializable<V> = default;
        constexpr char16_view(V base) : base_{std::move(base)} {}

        constexpr V & base() & { return base_; }
        constexpr const V & base() const & { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() { return iterator{std::ranges::begin(base_)}; }
        constexpr auto end()
        {
            if constexpr (std::ranges::common_range<V>) {
                return iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
        constexpr auto begin() const { return const_iterator{std::ranges::begin(base_)}; }
        constexpr auto end() const
        {
            if constexpr (std::ranges::common_range<const V>) {
                return const_iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
    };

    template<std::ranges::view V>
        requires std::convertible_to<std::ranges::range_value_t<V>, char32_t>
    class char32_view : public std::ranges::view_interface<char32_view<V>>
    {
        V base_ = V();

    public:
        using iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<V>, char32_t>;
        using const_iterator =
            detail::charn_projection_iterator<std::ranges::iterator_t<const V>, char32_t>;

        constexpr char32_view() requires std::default_initializable<V> = default;
        constexpr char32_view(V base) : base_{std::move(base)} {}

        constexpr V & base() & { return base_; }
        constexpr const V & base() const & { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto begin() { return iterator{std::ranges::begin(base_)}; }
        constexpr auto end()
        {
            if constexpr (std::ranges::common_range<V>) {
                return iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
        constexpr auto begin() const { return const_iterator{std::ranges::begin(base_)}; }
        constexpr auto end() const
        {
            if constexpr (std::ranges::common_range<const V>) {
                return const_iterator{std::ranges::end(base_)};
            } else {
                return std::ranges::end(base_);
            }
        }
    };

    template<class R>
    char8_view(R &&) -> char8_view<std::views::all_t<R>>;
    template<class R>
    char16_view(R &&) -> char16_view<std::views::all_t<R>>;
    template<class R>
    char32_view(R &&) -> char32_view<std::views::all_t<R>>;

    namespace detail {
        template<template<class> class View, format Format>
        struct as_charn_impl : range_adaptor_closure<as_charn_impl<View, Format>>
        {
            template<class R>
            requires (std::ranges::viewable_range<R> &&
                      std::convertible_to<std::ranges::range_value_t<R>, dtl::format_to_type_t<Format>>) ||
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
    }

    inline constexpr detail::as_charn_impl<char8_view, format::utf8> as_char8_t;
    inline constexpr detail::as_charn_impl<char16_view, format::utf16> as_char16_t;
    inline constexpr detail::as_charn_impl<char32_view, format::utf32> as_char32_t;

    // clang-format off
    template<std::ranges::range R>
      requires std::movable<R> // TODO && (!is-initializer-list<R>)
    class unpacking_owning_view : public std::ranges::view_interface<unpacking_owning_view<R>> {
      R r_ = R();

    public:
      constexpr unpacking_owning_view() requires std::default_initializable<R> = default;
      constexpr unpacking_owning_view(R&& r) : r_(std::move(r)) {}

      constexpr R& base() & noexcept { return r_; }
      constexpr const R& base() const & noexcept { return r_; }
      constexpr R&& base() && noexcept { return std::move(r_); }
      constexpr const R&& base() const && noexcept { return std::move(r_); }

      constexpr auto code_units() const noexcept {
        auto unpacked = boost::text::unpack_iterator_and_sentinel(std::ranges::begin(r_), std::ranges::end(r_));
        return std::ranges::subrange(unpacked.first, unpacked.last);
      }

      constexpr auto begin() const { return std::ranges::begin(r_); }
      constexpr auto end() const { return std::ranges::end(r_); }
    };
    // clang-format on

    namespace detail {
        template<class T>
        constexpr bool is_unpacking_owning_view = false;
        template<class R>
        constexpr bool is_unpacking_owning_view<unpacking_owning_view<R>> = true;
    }

    template<format Format, utf_range V>
        requires std::ranges::view<V>
    class utf_view : public std::ranges::view_interface<utf_view<Format, V>>
    {
        V base_ = V();

        template<format FromFormat, class I, class S>
        static constexpr auto make_begin(I first, S last)
        {
            if constexpr (std::bidirectional_iterator<I>) {
                return utf_iterator<FromFormat, Format, I, S>{
                    first, first, last};
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
                return utf_iterator<FromFormat, Format, I, S>{
                    first, last, last};
            } else {
                return utf_iterator<FromFormat, Format, I, S>{last, last};
            }
        }

    public:
        constexpr utf_view() requires std::default_initializable<V> = default;
        constexpr utf_view(V base) : base_{std::move(base)} {}

        constexpr V base() const & requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr auto code_units() const noexcept
            requires std::copy_constructible<V> || detail::is_unpacking_owning_view<V>
        {
            if constexpr (detail::is_unpacking_owning_view<V>) {
                return base_.code_units();
            } else {
                return base_;
            }
        }

        // TODO: If base_ is an as_charN_t view, then we need to use its
        // format as from_format, and we need to its *base*'s iterators.
        constexpr auto begin() const
        {
            constexpr format from_format =
                detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr (detail::is_unpacking_owning_view<V>) {
                return make_begin<from_format>(
                    std::ranges::begin(base_.code_units()),
                    std::ranges::end(base_.code_units()));
            } else {
                return make_begin<from_format>(
                    std::ranges::begin(base_), std::ranges::end(base_));
            }
        }
        constexpr auto end() const
        {
            constexpr format from_format =
                detail::format_of<std::ranges::range_value_t<V>>();
            if constexpr (detail::is_unpacking_owning_view<V>) {
                return make_end<from_format>(
                    std::ranges::begin(base_.code_units()),
                    std::ranges::end(base_.code_units()));
            } else {
                return make_end<from_format>(
                    std::ranges::begin(base_), std::ranges::end(base_));
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
    class utf8_view : public utf_view<format::utf8, V>
    {
    public:
        constexpr utf8_view() requires std::default_initializable<V> = default;
        constexpr utf8_view(V base) :
            utf_view<format::utf8, V>{std::move(base)}
        {}
    };
    template<utf_range V>
        requires std::ranges::view<V>
    class utf16_view : public utf_view<format::utf16, V>
    {
    public:
        constexpr utf16_view() requires std::default_initializable<V> = default;
        constexpr utf16_view(V base) :
            utf_view<format::utf16, V>{std::move(base)}
        {}
    };
    template<utf_range V>
        requires std::ranges::view<V>
    class utf32_view : public utf_view<format::utf32, V>
    {
    public:
        constexpr utf32_view() requires std::default_initializable<V> = default;
        constexpr utf32_view(V base) :
            utf_view<format::utf32, V>{std::move(base)}
        {}
    };

    template<class R>
    utf8_view(R &&) -> utf8_view<std::views::all_t<R>>;
    template<class R>
    utf16_view(R &&) -> utf16_view<std::views::all_t<R>>;
    template<class R>
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

        template<class T>
        constexpr bool is_char_n_view = false;
        template<class V>
        constexpr bool is_char_n_view<char8_view<V>> = true;
        template<class V>
        constexpr bool is_char_n_view<char16_view<V>> = true;
        template<class V>
        constexpr bool is_char_n_view<char32_view<V>> = true;

        template<class R>
        constexpr decltype(auto) unpack_range(R && r)
        {
            using T = std::remove_cvref_t<R>;
            if constexpr (std::ranges::forward_range<T>) {
                auto unpacked = boost::text::unpack_iterator_and_sentinel(
                    std::ranges::begin(r), std::ranges::end(r));
                if constexpr (std::is_bounded_array_v<T>) {
                    constexpr auto n = std::extent_v<T>;
                    if (n && !r[n - 1])
                        --unpacked.last;
                }
                if constexpr (std::ranges::borrowed_range<T> || std::is_lvalue_reference_v<R>) {
                    return std::ranges::subrange(unpacked.first, unpacked.last);
                } else if constexpr (
                    !std::same_as<decltype(unpacked.first), std::ranges::iterator_t<T>> ||
                    !std::same_as<decltype(unpacked.last), std::ranges::sentinel_t<T>>) {
                    return unpacking_owning_view(std::move(r));
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
                } else if constexpr (std::is_pointer_v<T>) {
                    return View(std::ranges::subrange(r, null_sentinel));
                } else {
                    return View(dtl::unpack_range(std::forward<R>(r)));
                }
            }
        };
    }

    inline constexpr dtl::as_utf_impl<utf8_view, format::utf8> as_utf8;
    inline constexpr dtl::as_utf_impl<utf16_view, format::utf16> as_utf16;
    inline constexpr dtl::as_utf_impl<utf32_view, format::utf32> as_utf32;

}}}

namespace std::ranges {
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::char8_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::char16_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::char32_view<V>> =
        enable_borrowed_range<V>;

    template<boost::text::format Format, class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf_view<Format, V>> =
        enable_borrowed_range<V>;

    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf8_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf16_view<V>> =
        enable_borrowed_range<V>;
    template<class V>
    inline constexpr bool enable_borrowed_range<boost::text::utf32_view<V>> =
        enable_borrowed_range<V>;
}

#endif

#endif
