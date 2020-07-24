// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_VIEW_HPP
#define BOOST_TEXT_TRANSCODE_VIEW_HPP

#include <boost/text/transcode_algorithm.hpp>
#if defined(__cpp_lib_concepts)
#include <boost/text/concepts.hpp>
#endif
#include <boost/text/detail/unpack.hpp>

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

    /** A view over UTF-8 code units.  The set of operations available depends
        on types `I` and `S`.  See [view.interface] in the C++20 or later
        standard for details. */
#if defined(__cpp_lib_concepts)
    template<u8_iter I, std::sentinel_for<I> S = I>
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

        friend std::ostream & operator<<(std::ostream & os, utf8_view v)
        {
            auto out = std::ostreambuf_iterator<char>(os);
            for (auto it = v.begin(); it != v.end(); ++it, ++out) {
                *out = *it;
            }
            return os;
        }
#if defined(_MSC_VER)
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
        sentinel_t last_;
    };

    /** A view over UTF-16 code units.  The set of operations available
        depends on types `I` and `S`.  See [view.interface] in the
        C++20 or later standard for details. */
#if defined(__cpp_lib_concepts)
    template<u16_iter I, std::sentinel_for<I> S = I>
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

        friend std::ostream & operator<<(std::ostream & os, utf16_view v)
        {
            boost::text::transcode_to_utf8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(_MSC_VER)
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
        sentinel_t last_;
    };

    /** A view over UTF-32 code units.  The set of operations available
        depends on types `I` and `S`.  See [view.interface] in the
        C++20 or later standard for details. */
#if defined(__cpp_lib_concepts)
    template<u32_iter I, std::sentinel_for<I> S = I>
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

        friend std::ostream & operator<<(std::ostream & os, utf32_view v)
        {
            boost::text::transcode_to_utf8(
                v.begin(), v.end(), std::ostreambuf_iterator<char>(os));
            return os;
        }
#if defined(_MSC_VER)
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
        sentinel_t last_;
    };

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

    /** Returns a `utf8_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf8(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace dtl {
        template<
            typename Range,
            bool Pointer = detail::char_ptr<Range>::value ||
                           detail::_16_ptr<Range>::value ||
                           detail::cp_ptr<Range>::value>
        struct as_utf8_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf8(std::begin(r), std::end(r)))
            {
                return as_utf8(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf8_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf8(p, null_sentinel{}))
            {
                return as_utf8(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf8_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf8(Range const & r) noexcept->decltype(
        dtl::as_utf8_dispatch<Range>::call(r))
    {
        return dtl::as_utf8_dispatch<Range>::call(r);
    }

    /** Returns a `utf16_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf16(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace dtl {
        template<
            typename Range,
            bool Pointer = detail::char_ptr<Range>::value ||
                           detail::_16_ptr<Range>::value ||
                           detail::cp_ptr<Range>::value>
        struct as_utf16_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf16(std::begin(r), std::end(r)))
            {
                return as_utf16(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf16_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf16(p, null_sentinel{}))
            {
                return as_utf16(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf16_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf16(Range const & r) noexcept->decltype(
        dtl::as_utf16_dispatch<Range>::call(r))
    {
        return dtl::as_utf16_dispatch<Range>::call(r);
    }

    /** Returns a `utf32_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_utf32(Iter first, Sentinel last) noexcept
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    namespace dtl {
        template<
            typename Range,
            bool Pointer = detail::char_ptr<Range>::value ||
                           detail::_16_ptr<Range>::value ||
                           detail::cp_ptr<Range>::value>
        struct as_utf32_dispatch
        {
            static constexpr auto call(Range const & r) noexcept
                -> decltype(as_utf32(std::begin(r), std::end(r)))
            {
                return as_utf32(std::begin(r), std::end(r));
            }
        };

        template<typename Ptr>
        struct as_utf32_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p) noexcept
                -> decltype(as_utf32(p, null_sentinel{}))
            {
                return as_utf32(p, null_sentinel{});
            }
        };
    }

    /** Returns a `utf32_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename Range>
    constexpr auto as_utf32(Range const & r) noexcept->decltype(
        dtl::as_utf32_dispatch<Range>::call(r))
    {
        return dtl::as_utf32_dispatch<Range>::call(r);
    }
}}}

#if defined(__cpp_lib_concepts)

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    /** Returns a `utf8_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename I, std::sentinel_for<I> S>
        // clang-format off
        requires u8_iter<I> || u16_iter<I> || u32_iter<I>
    constexpr auto as_utf8(I first, S last) noexcept
    // clang-format on
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf8_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf8_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename R>
        // clang-format off
        requires u8_range<R> || u16_range<R> || u32_range<R> ||
        u8_ptr<R> || u16_ptr<R> || u32_ptr<R>
    constexpr auto as_utf8(R const & r) noexcept
    // clang-format on
    {
        if constexpr (std::is_pointer_v<R>) {
            return text::as_utf8(r, null_sentinel{});
        } else {
            return text::as_utf8(std::ranges::begin(r), std::ranges::end(r));
        }
    }

    /** Returns a `utf16_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename I, std::sentinel_for<I> S>
        // clang-format off
        requires u8_iter<I> || u16_iter<I> || u32_iter<I>
    constexpr auto as_utf16(I first, S last) noexcept
    // clang-format on
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf16_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf16_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename R>
        // clang-format off
        requires u8_range<R> || u16_range<R> || u32_range<R> ||
        u8_ptr<R> || u16_ptr<R> || u32_ptr<R>
    constexpr auto as_utf16(R const & r) noexcept
    // clang-format on
    {
        if constexpr (std::is_pointer_v<R>) {
            return text::as_utf16(r, null_sentinel{});
        } else {
            return text::as_utf16(std::ranges::begin(r), std::ranges::end(r));
        }
    }

    /** Returns a `utf32_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename I, std::sentinel_for<I> S>
        // clang-format off
        requires u8_iter<I> || u16_iter<I> || u32_iter<I>
    constexpr auto as_utf32(I first, S last) noexcept
    // clang-format on
    {
        auto unpacked = detail::unpack_iterator_and_sentinel(first, last);
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        return utf32_view<decltype(r.f_), decltype(r.l_)>(r.f_, r.l_);
    }

    /** Returns a `utf32_view` over the data in `r`, transcoding the data if
        necessary. */
    template<typename R>
        // clang-format off
        requires u8_range<R> || u16_range<R> || u32_range<R> ||
        u8_ptr<R> || u16_ptr<R> || u32_ptr<R>
    constexpr auto as_utf32(R const & r) noexcept
    // clang-format on
    {
        if constexpr (std::is_pointer_v<R>) {
            return text::as_utf32(r, null_sentinel{});
        } else {
            return text::as_utf32(std::ranges::begin(r), std::ranges::end(r));
        }
    }
}}}

#endif

#endif
