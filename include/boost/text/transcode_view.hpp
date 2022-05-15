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
#include <boost/text/detail/unpack.hpp>
#include <boost/text/view_adaptor.hpp>

#include <boost/stl_interfaces/view_interface.hpp>


namespace boost { namespace text {

    namespace detail {

        // UTF-8
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf8_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf8_tag, Iter, Sentinel>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_16_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_16_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_8_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_32_to_8_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf8_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf8_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_32_to_8_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_8_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf8_tag, decltype(f)>{f, l};
        }

        // UTF-16
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_8_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_8_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_16_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf16_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf16_range_(utf16_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf16_tag, Iter, Sentinel>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf16_range_(utf32_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_32_to_16_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf16_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf16_range_(utf32_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_32_to_16_iterator<Iter>(f_, f_, l_);
            auto l = utf_32_to_16_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf16_tag, decltype(f)>{f, l};
        }

        // UTF-32
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_8_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf8_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_8_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_8_to_32_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto
        make_utf32_range_(utf16_tag, Iter f_, Sentinel l) noexcept
        {
            auto f = utf_16_to_32_iterator<Iter, Sentinel>(f_, f_, l);
            return tagged_range<utf32_tag, decltype(f), Sentinel>{f, l};
        }
        template<typename Iter>
        constexpr auto make_utf32_range_(utf16_tag, Iter f_, Iter l_) noexcept
        {
            auto f = utf_16_to_32_iterator<Iter>(f_, f_, l_);
            auto l = utf_16_to_32_iterator<Iter>(f_, l_, l_);
            return tagged_range<utf32_tag, decltype(f)>{f, l};
        }
        template<typename Iter, typename Sentinel>
        constexpr auto make_utf32_range_(utf32_tag, Iter f, Sentinel l) noexcept
        {
            return tagged_range<utf32_tag, Iter, Sentinel>{f, l};
        }

        template<typename ResultType, typename Iterator, typename Sentinel>
        constexpr auto
        make_iter(Iterator first, Iterator it, Sentinel last) noexcept
            -> decltype(ResultType(first, it, last))
        {
            return ResultType(first, it, last);
        }
        template<typename ResultType>
        constexpr auto
        make_iter(ResultType first, ResultType it, ResultType last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Sentinel>
        constexpr auto
        make_iter(ResultType first, ResultType it, Sentinel last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
        template<typename ResultType, typename Iterator>
        constexpr auto
        make_iter(Iterator first, ResultType it, ResultType last) noexcept
            -> decltype(ResultType(it))
        {
            return it;
        }
    }

    /** A view over UTF-8 code units. */
#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS
    template<utf8_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct utf8_view : stl_interfaces::view_interface<utf8_view<I, S>>
    {
        using iterator = I;
        using sentinel = S;

        constexpr utf8_view() noexcept {}
        constexpr utf8_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf8_view lhs, utf8_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf8_view lhs, utf8_view rhs)
        {
            return !(lhs == rhs);
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, utf8_view v)
        {
            auto out = std::ostreambuf_iterator<char>(os);
            for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                *out = *it;
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, utf8_view v)
        {
            boost::text::transcode_utf_8_to_16(
                v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            return os;
        }
#endif

    private:
        using iterator_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .f_);
        using sentinel_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .l_);

        iterator_t first_;
        [[no_unique_address]] sentinel_t last_;
    };

    /** A view over UTF-16 code units. */
#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS
    template<utf16_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct utf16_view : stl_interfaces::view_interface<utf16_view<I, S>>
    {
        using iterator = I;
        using sentinel = S;

        constexpr utf16_view() noexcept {}
        constexpr utf16_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf16_view lhs, utf16_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf16_view lhs, utf16_view rhs)
        {
            return !(lhs == rhs);
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, utf16_view v)
        {
            boost::text::transcode_to_utf8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, utf16_view v)
        {
            auto out = std::ostreambuf_iterator<wchar_t>(os);
            for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                *out = *it;
            }
            return os;
        }
#endif

    private:
        using iterator_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .f_);
        using sentinel_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .l_);

        iterator_t first_;
        [[no_unique_address]] sentinel_t last_;
    };

    /** A view over UTF-32 code units. */
#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS
    template<utf32_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct utf32_view : stl_interfaces::view_interface<utf32_view<I, S>>
    {
        using iterator = I;
        using sentinel = S;

        constexpr utf32_view() noexcept {}
        constexpr utf32_view(iterator first, sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        constexpr iterator begin() const noexcept
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr sentinel end() const noexcept
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        friend constexpr bool operator==(utf32_view lhs, utf32_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(utf32_view lhs, utf32_view rhs)
        {
            return !(lhs == rhs);
        }

        /** Stream inserter; performs unformatted output, in UTF-8
            encoding. */
        friend std::ostream & operator<<(std::ostream & os, utf32_view v)
        {
            boost::text::transcode_to_utf8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs unformatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, utf32_view v)
        {
            boost::text::transcode_to_utf16(
                v.begin(), v.end(), std::ostreambuf_iterator<wchar_t>(os));
            return os;
        }
#endif

    private:
        using iterator_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .f_);
        using sentinel_t = decltype(detail::unpack_iterator_and_sentinel(
                                        std::declval<I>(), std::declval<S>())
                                        .l_);

        iterator_t first_;
        [[no_unique_address]] sentinel_t last_;
    };

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
            static constexpr auto call(Range && r) noexcept
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf8_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf8_impl : range_adaptor_closure<as_utf8_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf8_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const noexcept
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
            static constexpr auto call(Range && r) noexcept
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf16_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf16_impl : range_adaptor_closure<as_utf16_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf16_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const noexcept
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
            static constexpr auto call(Range && r) noexcept
                -> decltype(Impl{}(detail::begin(r), detail::end(r)))
            {
                return Impl{}(detail::begin(r), detail::end(r));
            }
        };

        template<typename Impl, typename Ptr>
        struct as_utf32_dispatch<Impl, Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(Impl{}(p, null_sentinel))
            {
                return Impl{}(p, null_sentinel);
            }
        };

        struct as_utf32_impl : range_adaptor_closure<as_utf32_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const noexcept
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

#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `utf8_view` over the data in `[first, last)`.  The view will
        transcode the data if necessary. */
    template<utf_iter I, std::sentinel_for<I> S>
    constexpr detail::unspecified as_utf8(I first, S last) noexcept;

    /** Returns a `utf8_view` over the data in `r`.  The view will transcode
        the data if necessary.  If `std::remove_reference_t<R>` is not a
        pointer, the result is returned as a `borrowed_view_t` (C++20 and
        later only). */
    template<utf_range_like R>
    constexpr detail::unspecified as_utf8(R && r) noexcept;

#endif

    namespace dtl {
        struct as_utf8_impl : range_adaptor_closure<as_utf8_impl>
        {
            template<utf_iter I, std::sentinel_for<I> S>
            constexpr auto operator()(I first, S last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf8_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<utf_range_like R>
            constexpr auto operator()(R && r) const noexcept
            {
                if constexpr (std::is_pointer_v<std::remove_reference_t<R>>)
                    return (*this)(r, null_sentinel);
                else if constexpr (std::ranges::borrowed_range<R>)
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                else
                    return std::ranges::dangling{};
            }
        };
    }

    inline constexpr dtl::as_utf8_impl as_utf8;

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `utf16_view` over the data in `[first, last)`.  The view
        will transcode the data if necessary. */
    template<utf_iter I, std::sentinel_for<I> S>
    constexpr detail::unspecified as_utf16(I first, S last) noexcept;

    /** Returns a `utf16_view` over the data in `r` the data if necessary.  If
        `std::remove_reference_t<R>` is not a pointer, the result is returned
        as a `borrowed_view_t` (C++20 and later only). */
    template<utf_range_like R>
    constexpr detail::unspecified as_utf16(R && r) noexcept;

#endif

    namespace dtl {
        struct as_utf16_impl : range_adaptor_closure<as_utf16_impl>
        {
            template<utf_iter I, std::sentinel_for<I> S>
            constexpr auto operator()(I first, S last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf16_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<utf_range_like R>
            constexpr auto operator()(R && r) const noexcept
            {
                if constexpr (std::is_pointer_v<std::remove_reference_t<R>>)
                    return (*this)(r, null_sentinel);
                else if constexpr (std::ranges::borrowed_range<R>)
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                else
                    return std::ranges::dangling{};
            }
        };
    }

    inline constexpr dtl::as_utf16_impl as_utf16;

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `utf32_view` over the data in `[first, last)`.  The view
         will transcode the data if necessary. */
    template<utf_iter I, std::sentinel_for<I> S>
    constexpr detail::unspecified as_utf32(I first, S last) noexcept;

    /** Returns a `utf32_view` over the data in `r`.  The view will transcode
        the data if necessary.  If `std::remove_reference_t<R>` is not a
        pointer, the result is returned as a `borrowed_view_t` (C++20 and
        later only). */
    template<utf_range_like R>
    constexpr detail::unspecified as_utf32(R && r) noexcept;

#endif

    namespace dtl {
        struct as_utf32_impl : range_adaptor_closure<as_utf32_impl>
        {
            template<utf_iter I, std::sentinel_for<I> S>
            constexpr auto operator()(I first, S last) const noexcept
            {
                auto unpacked =
                    detail::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    unpacked.tag_, unpacked.f_, unpacked.l_);
                return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
            }

            template<utf_range_like R>
            constexpr auto operator()(R && r) const noexcept
            {
                if constexpr (std::is_pointer_v<std::remove_reference_t<R>>)
                    return (*this)(r, null_sentinel);
                else if constexpr (std::ranges::borrowed_range<R>)
                    return (*this)(std::ranges::begin(r), std::ranges::end(r));
                else
                    return std::ranges::dangling{};
            }
        };
    }

    inline constexpr dtl::as_utf32_impl as_utf32;
}}}

namespace std::ranges {
    template<boost::text::utf8_iter I, std::sentinel_for<I> S>
    inline constexpr bool enable_borrowed_range<boost::text::utf8_view<I, S>> =
        true;

    template<boost::text::utf16_iter I, std::sentinel_for<I> S>
    inline constexpr bool enable_borrowed_range<boost::text::utf16_view<I, S>> =
        true;

    template<boost::text::utf32_iter I, std::sentinel_for<I> S>
    inline constexpr bool enable_borrowed_range<boost::text::utf32_view<I, S>> =
        true;
}

#endif

#endif
