// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_STRING_ALGORITHM_HPP
#define BOOST_TEXT_STRING_ALGORITHM_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/algorithm.hpp>

#include <algorithm>


namespace boost { namespace text { namespace detail {

    template<
        typename R,
        bool IsPtr = std::is_pointer<std::remove_reference_t<R>>::value>
    struct iterator_from
    {
        using type = iterator_t<R>;
    };
    template<typename R>
    struct iterator_from<R, true>
    {
        using type = std::remove_reference_t<R>;
    };
    template<typename R>
    using iterator_from_t = typename iterator_from<R>::type;

    template<typename T>
    using ptr_arr_value_t =
        std::remove_pointer_t<std::remove_extent_t<std::remove_reference_t<T>>>;

    template<typename I, typename S>
    auto remove_utf32_terminator(utf32_view<I, S> view)
    {
        return view;
    }
    template<typename I>
    auto remove_utf32_terminator(utf32_view<I> view)
    {
        if (!view.empty() && view.back() == 0)
            return utf32_view<I>(view.begin(), std::prev(view.end()));
        return view;
    }

    template<typename R>
    auto as_utf32_no_terminator(R & r)
    {
        auto retval = boost::text::as_utf32(r);
        return detail::remove_utf32_terminator(retval);
    }

    template<typename I>
    auto as_utf32_no_sentinel_or_terminator(I first, I last)
    {
        return detail::remove_utf32_terminator(
            boost::text::as_utf32(first, last));
    }

    template<typename I, typename S>
    auto as_utf32_no_sentinel_or_terminator(I first, S last)
    {
        auto it = first;
        while (it != last) {
            ++it;
        }
        return boost::text::as_utf32(first, it);
    }

    template<typename R, bool Pointer = std::is_pointer<R>::value>
    struct as_utf32_common_view_dispatch
    {
        static constexpr auto call(R & r) noexcept
            -> decltype(detail::as_utf32_no_sentinel_or_terminator(
                std::begin(r), std::end(r)))
        {
            return detail::as_utf32_no_sentinel_or_terminator(
                std::begin(r), std::end(r));
        }
    };

    template<typename Ptr>
    struct as_utf32_common_view_dispatch<Ptr, true>
    {
        using string_view_t = basic_string_view<std::remove_pointer_t<Ptr>>;
        static constexpr auto call(Ptr p) noexcept
            -> decltype(boost::text::as_utf32(string_view_t(p)))
        {
            return boost::text::as_utf32(string_view_t(p));
        }
    };

    template<typename R>
    auto as_utf32_common_view_no_terminator(R & r)
        -> decltype(as_utf32_common_view_dispatch<R>::call(r))
    {
        return as_utf32_common_view_dispatch<R>::call(r);
    }

}}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {
    // TODO
}}}

#if defined(BOOST_TEXT_DOXYGEN) || defined(__cpp_lib_concepts)

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    namespace dtl {
        template<typename I1, typename S1, typename I2, typename S2>
        std::ranges::subrange<I1> find(I1 first1, S1 last1, I2 first2, S2 last2)
        {
            return std::ranges::search(first1, last1, first2, last2);
        }

        template<typename I1, typename I2>
        std::ranges::subrange<I1>
        rfind(I1 first1, I1 last1, I2 first2, I2 last2)
        {
            auto const result = std::ranges::search(
                std::make_reverse_iterator(last1),
                std::make_reverse_iterator(first1),
                std::make_reverse_iterator(last2),
                std::make_reverse_iterator(first2));
            if (result.empty())
                return {last1, last1};
            return {result.end().base(), result.begin().base()};
        }

        template<typename I1, typename S1, typename I2, typename S2>
        I1 find_first_of(I1 first1, S1 last1, I2 first2, S2 last2)
        {
            return std::ranges::find_first_of(first1, last1, first2, last2);
        }

        template<typename I1, typename I2>
        I1 find_last_of(I1 first1, I1 last1, I2 first2, I2 last2)
        {
            auto const last1_ = std::make_reverse_iterator(first1);
            auto const result = std::ranges::find_first_of(
                std::make_reverse_iterator(last1),
                last1_,
                std::make_reverse_iterator(last2),
                std::make_reverse_iterator(first2));
            if (result == last1_)
                return last1;
            return std::prev(result.base());
        }

        template<typename I1, typename S1, typename I2, typename S2>
        I1 find_first_not_of(I1 first1, S1 last1, I2 first2, S2 last2)
        {
            return std::ranges::find_first_of(
                first1, last1, first2, last2, std::ranges::not_equal_to{});
        }

        template<typename I1, typename I2>
        I1 find_last_not_of(I1 first1, I1 last1, I2 first2, I2 last2)
        {
            auto const last1_ = std::make_reverse_iterator(first1);
            auto const result = std::ranges::find_first_of(
                std::make_reverse_iterator(last1),
                last1_,
                std::make_reverse_iterator(last2),
                std::make_reverse_iterator(first2),
                std::ranges::not_equal_to{});
            if (result == last1_)
                return last1;
            return std::prev(result.base());
        }

        template<typename I1, typename S1, typename I2, typename S2>
        bool starts_with(I1 first1, S1 last1, I2 first2, S2 last2)
        {
            return std::ranges::mismatch(first1, last1, first2, last2).in2 ==
                   last2;
        }

        template<typename I1, typename I2>
        bool ends_with(I1 first1, I1 last1, I2 first2, I2 last2)
        {
            auto const target = std::make_reverse_iterator(first2);
            return std::ranges::mismatch(
                       std::make_reverse_iterator(last1),
                       std::make_reverse_iterator(first1),
                       std::make_reverse_iterator(last2),
                       target)
                       .in2 == target;
        }

        template<typename I1, typename S1, typename I2, typename S2>
        bool contains(I1 first1, S1 last1, I2 first2, S2 last2)
        {
            return dtl::find(first1, last1, first2, last2).begin() != last1;
        }
    }

    // Code point iterator overloads.

    template<
        code_point_iter I1,
        std::sentinel_for<I1> S1,
        code_point_iter I2,
        std::sentinel_for<I2> S2>
    std::ranges::subrange<I1> find(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find(first1, last1, first2, last2);
    }

    template<code_point_iter I1, code_point_iter I2>
    std::ranges::subrange<I1> rfind(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::rfind(first1, last1, first2, last2);
    }

    template<
        code_point_iter I1,
        std::sentinel_for<I1> S1,
        code_point_iter I2,
        std::sentinel_for<I2> S2>
    I1 find_first_of(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find_first_of(first1, last1, first2, last2);
    }

    template<code_point_iter I1, code_point_iter I2>
    I1 find_last_of(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::find_last_of(first1, last1, first2, last2);
    }

    template<
        code_point_iter I1,
        std::sentinel_for<I1> S1,
        code_point_iter I2,
        std::sentinel_for<I2> S2>
    I1 find_first_not_of(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find_first_not_of(first1, last1, first2, last2);
    }

    template<code_point_iter I1, code_point_iter I2>
    I1 find_last_not_of(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::find_last_not_of(first1, last1, first2, last2);
    }

    template<
        code_point_iter I1,
        std::sentinel_for<I1> S1,
        code_point_iter I2,
        std::sentinel_for<I2> S2>
    bool starts_with(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::starts_with(first1, last1, first2, last2);
    }

    template<code_point_iter I1, code_point_iter I2>
    bool ends_with(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::ends_with(first1, last1, first2, last2);
    }

    template<
        code_point_iter I1,
        std::sentinel_for<I1> S1,
        code_point_iter I2,
        std::sentinel_for<I2> S2>
    bool contains(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::contains(first1, last1, first2, last2);
    }

    // Code point range overloads.

    template<utf_range_like R1, utf_range_like R2>
    auto find(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_no_terminator(r1);
        auto r2_ = detail::as_utf32_no_terminator(r2);
        return dtl::find(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    auto rfind(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_common_view_no_terminator(r1);
        auto r2_ = detail::as_utf32_common_view_no_terminator(r2);
        return dtl::rfind(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    auto find_first_of(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_no_terminator(r1);
        auto r2_ = detail::as_utf32_no_terminator(r2);
        return dtl::find_first_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    auto find_last_of(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_common_view_no_terminator(r1);
        auto r2_ = detail::as_utf32_common_view_no_terminator(r2);
        return dtl::find_last_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    auto find_first_not_of(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_no_terminator(r1);
        auto r2_ = detail::as_utf32_no_terminator(r2);
        return dtl::find_first_not_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    auto find_last_not_of(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_common_view_no_terminator(r1);
        auto r2_ = detail::as_utf32_common_view_no_terminator(r2);
        return dtl::find_last_not_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    bool starts_with(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_no_terminator(r1);
        auto r2_ = detail::as_utf32_no_terminator(r2);
        return dtl::starts_with(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    bool ends_with(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_common_view_no_terminator(r1);
        auto r2_ = detail::as_utf32_common_view_no_terminator(r2);
        return dtl::ends_with(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    template<utf_range_like R1, utf_range_like R2>
    bool contains(R1 && r1, R2 && r2)
    {
        auto r1_ = detail::as_utf32_no_terminator(r1);
        auto r2_ = detail::as_utf32_no_terminator(r2);
        return dtl::contains(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }

    // Grapheme iterator overloads.

    template<
        grapheme_iter I1,
        std::sentinel_for<I1> S1,
        grapheme_iter I2,
        std::sentinel_for<I2> S2>
    std::ranges::subrange<I1> find(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find(first1, last1, first2, last2);
    }

    template<grapheme_iter I1, grapheme_iter I2>
    std::ranges::subrange<I1> rfind(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::rfind(first1, last1, first2, last2);
    }

    template<
        grapheme_iter I1,
        std::sentinel_for<I1> S1,
        grapheme_iter I2,
        std::sentinel_for<I2> S2>
    I1 find_first_of(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find_first_of(first1, last1, first2, last2);
    }

    template<grapheme_iter I1, grapheme_iter I2>
    I1 find_last_of(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::find_last_of(first1, last1, first2, last2);
    }

    template<
        grapheme_iter I1,
        std::sentinel_for<I1> S1,
        grapheme_iter I2,
        std::sentinel_for<I2> S2>
    I1 find_first_not_of(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::find_first_not_of(first1, last1, first2, last2);
    }

    template<grapheme_iter I1, grapheme_iter I2>
    I1 find_last_not_of(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::find_last_not_of(first1, last1, first2, last2);
    }

    template<
        grapheme_iter I1,
        std::sentinel_for<I1> S1,
        grapheme_iter I2,
        std::sentinel_for<I2> S2>
    bool starts_with(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::starts_with(first1, last1, first2, last2);
    }

    template<grapheme_iter I1, grapheme_iter I2>
    bool ends_with(I1 first1, I1 last1, I2 first2, I2 last2)
    {
        return dtl::ends_with(first1, last1, first2, last2);
    }

    template<
        grapheme_iter I1,
        std::sentinel_for<I1> S1,
        grapheme_iter I2,
        std::sentinel_for<I2> S2>
    bool contains(I1 first1, S1 last1, I2 first2, S2 last2)
    {
        return dtl::contains(first1, last1, first2, last2);
    }

    // Grapheme range overloads.

    template<grapheme_range R1, grapheme_range R2>
    auto find(R1 && r1, R2 && r2)
    {
        return dtl::find(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto find(R1 && r1, R2 && r2)
    {
        auto r2_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r2));
        return dtl::find(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto find(R1 && r1, R2 && r2)
    {
        auto r1_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r1));
        return dtl::find(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    auto rfind(R1 && r1, R2 && r2)
    {
        return dtl::rfind(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto rfind(R1 && r1, R2 && r2)
    {
        auto r2_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r2));
        return dtl::rfind(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto rfind(R1 && r1, R2 && r2)
    {
        auto r1_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r1));
        return dtl::rfind(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    auto find_first_of(R1 && r1, R2 && r2)
    {
        return dtl::find_first_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto find_first_of(R1 && r1, R2 && r2)
    {
        auto r2_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r2));
        return dtl::find_first_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto find_first_of(R1 && r1, R2 && r2)
    {
        auto r1_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r1));
        return dtl::find_first_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    auto find_last_of(R1 && r1, R2 && r2)
    {
        return dtl::find_last_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto find_last_of(R1 && r1, R2 && r2)
    {
        auto r2_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r2));
        return dtl::find_last_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto find_last_of(R1 && r1, R2 && r2)
    {
        auto r1_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r1));
        return dtl::find_last_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    auto find_first_not_of(R1 && r1, R2 && r2)
    {
        return dtl::find_first_not_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto find_first_not_of(R1 && r1, R2 && r2)
    {
        auto r2_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r2));
        return dtl::find_first_not_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto ffind_first_not_of(R1 && r1, R2 && r2)
    {
        auto r1_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r1));
        return dtl::find_first_not_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    auto find_last_not_of(R1 && r1, R2 && r2)
    {
        return dtl::find_last_not_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    auto find_last_not_of(R1 && r1, R2 && r2)
    {
        auto r2_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r2));
        return dtl::find_last_not_of(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    auto find_last_not_of(R1 && r1, R2 && r2)
    {
        auto r1_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r1));
        return dtl::find_last_not_of(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    bool starts_with(R1 && r1, R2 && r2)
    {
        return dtl::starts_with(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    bool starts_with(R1 && r1, R2 && r2)
    {
        auto r2_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r2));
        return dtl::starts_with(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    bool starts_with(R1 && r1, R2 && r2)
    {
        auto r1_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r1));
        return dtl::starts_with(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    bool ends_with(R1 && r1, R2 && r2)
    {
        return dtl::ends_with(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    bool ends_with(R1 && r1, R2 && r2)
    {
        auto r2_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r2));
        return dtl::ends_with(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    bool ends_with(R1 && r1, R2 && r2)
    {
        auto r1_ = boost::text::as_graphemes(
            detail::as_utf32_common_view_no_terminator(r1));
        return dtl::ends_with(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

    template<grapheme_range R1, grapheme_range R2>
    bool contains(R1 && r1, R2 && r2)
    {
        return dtl::contains(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }
    template<grapheme_range R1, utf_range_like R2>
    bool contains(R1 && r1, R2 && r2)
    {
        auto r2_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r2));
        return dtl::contains(
            std::ranges::begin(r1),
            std::ranges::end(r1),
            std::ranges::begin(r2_),
            std::ranges::end(r2_));
    }
    template<utf_range_like R1, grapheme_range R2>
    bool contains(R1 && r1, R2 && r2)
    {
        auto r1_ =
            boost::text::as_graphemes(detail::as_utf32_no_terminator(r1));
        return dtl::contains(
            std::ranges::begin(r1_),
            std::ranges::end(r1_),
            std::ranges::begin(r2),
            std::ranges::end(r2));
    }

}}}

#endif

#endif
