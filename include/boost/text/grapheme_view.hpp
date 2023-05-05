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
    template<code_point_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct grapheme_view : stl_interfaces::view_interface<grapheme_view<I, S>>
    {
        using iterator = grapheme_iterator<I, S>;
        using sentinel = detail::gr_view_sentinel_t<I, S>;

        constexpr grapheme_view() : first_(), last_() {}

        /** Construct a grapheme view that covers all the graphemes in
            `[first, last)`. */
        constexpr grapheme_view(iterator first, sentinel last) :
            first_(first),
            last_(last)
        {}

        /** Construct a grapheme view that covers all the graphemes in
            `[first, last)`. */
        constexpr grapheme_view(I first, S last) :
            first_(first, first, last),
            last_(detail::make_iter<sentinel>(first, last, last))
        {}

        /** Construct a grapheme view that covers only the graphemes in
            `[view_first, view_last)`.

            \note You should prefer this constructor over the
            `grapheme_view(I, S)` constructor if you want to use `begin()` and
            `end()` to traverse parts of `[first, last)` outside of
            `[view_first, view_last)`. */
#if BOOST_TEXT_USE_CONCEPTS
        template<code_point_iter I2>
        // clang-format off
            requires std::constructible_from<iterator, I2, I2, I2> &&
                std::constructible_from<sentinel, I2, I2, I2>
#else
        template<
            typename I2 = I,
            typename S2 = S,
            typename Enable = std::enable_if_t<std::is_same<I2, S2>::value>>
#endif
        constexpr grapheme_view(
            I2 first, I2 view_first, I2 view_last, I2 last) :
            // clang-format on
            first_(first, view_first, last),
            last_(first, view_last, last)
        {}

        constexpr iterator begin() const { return first_; }
        constexpr sentinel end() const { return last_; }

        friend constexpr bool operator==(grapheme_view lhs, grapheme_view rhs)
        {
            return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
        }
        friend constexpr bool operator!=(grapheme_view lhs, grapheme_view rhs)
        {
            return !(lhs == rhs);
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

    private:
        iterator first_;
        [[no_unique_address]] sentinel last_;
    };

#if defined(__cpp_deduction_guides)
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S>
#else
    template<typename I, typename S>
#endif
    grapheme_view(grapheme_iterator<I, S>, grapheme_iterator<I, S>)
        -> grapheme_view<I, S>;

#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S>
#else
    template<typename I, typename S>
#endif
    grapheme_view(grapheme_iterator<I, S>, S) -> grapheme_view<I, S>;

#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S>
#else
    template<typename I, typename S>
#endif
    grapheme_view(I, S) -> grapheme_view<I, S>;
#endif

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `grapheme_view` over the data in `[first, last)`, transcoding
        the data if necessary. */
    template<typename Iter, typename Sentinel>
    constexpr auto as_graphemes(Iter first, Sentinel last);

    /** Returns a `grapheme_view` over the data in `r`, transcoding the data
        if necessary.  If `std::remove_reference_t<R>` is not a pointer, the
        result is returned as a `borrowed_view_t` (C++20 and later only). */
    template<typename Range>
    constexpr auto as_graphemes(Range && r);

#endif

    namespace dtl {
        template<
            typename Range,
            bool Pointer =
                detail::is_char_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_16_ptr_v<std::remove_reference_t<Range>> ||
                detail::is_cp_ptr_v<std::remove_reference_t<Range>>>
        struct as_graphemes_dispatch
        {
            static constexpr auto call(Range && r_)
            {
                auto r = boost::text::as_utf32(r_);
                return grapheme_view<decltype(r.begin()), decltype(r.end())>(
                    r.begin(), r.end());
            }
        };

        template<typename Ptr>
        struct as_graphemes_dispatch<Ptr, true>
        {
            static constexpr auto call(Ptr p)
            {
                auto r = boost::text::as_utf32(p);
                return grapheme_view<decltype(r.begin()), null_sentinel_t>(
                    r.begin(), null_sentinel);
            }
        };

        struct as_graphemes_impl : range_adaptor_closure<as_graphemes_impl>
        {
            template<typename Iter, typename Sentinel>
            constexpr auto operator()(Iter first, Sentinel last) const
            {
                auto unpacked =
                    boost::text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                return grapheme_view<decltype(r.first), decltype(r.last)>(
                    r.first, r.last);
            }

            template<typename Range>
            constexpr auto operator()(Range && r) const
                -> decltype(dtl::as_graphemes_dispatch<Range &&>::call(
                    (Range &&) r))
            {
                return dtl::as_graphemes_dispatch<Range &&>::call((Range &&) r);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::as_graphemes_impl as_graphemes;
#else
    namespace {
        constexpr dtl::as_graphemes_impl as_graphemes;
    }
#endif

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    namespace dtl {
        struct as_graphemes_impl : range_adaptor_closure<as_graphemes_impl>
        {
            template<utf_iter I, std::sentinel_for<I> S>
            constexpr auto operator()(I first, S last) const
            {
                auto unpacked =
                    boost::text::unpack_iterator_and_sentinel(first, last);
                auto r = detail::make_utf32_range_(
                    detail::tag_t<unpacked.format_tag>{},
                    unpacked.first,
                    unpacked.last);
                return grapheme_view<decltype(r.first), decltype(r.last)>(
                    r.first, r.last);
            }

            template<utf_range_like R>
            constexpr auto operator()(R && r) const
            {
                if constexpr (
                    !std::is_pointer_v<std::remove_reference_t<R>> &&
                    !std::ranges::borrowed_range<R>) {
                    return std::ranges::dangling{};
                } else {
                    auto intermediate = r | boost::text::as_utf32;
                    return grapheme_view<
                        std::ranges::iterator_t<decltype(intermediate)>,
                        std::ranges::sentinel_t<decltype(intermediate)>>(
                        intermediate.begin(), intermediate.end());
                }
            }
        };
    }

    inline constexpr dtl::as_graphemes_impl as_graphemes;

}}}

#endif

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<typename CPIter, typename Sentinel>
    inline constexpr bool
        enable_borrowed_range<boost::text::grapheme_view<CPIter, Sentinel>> =
            true;
}

#endif

#endif
