// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_PARAGRAPH_BREAK_HPP
#define BOOST_TEXT_PARAGRAPH_BREAK_HPP

#include <boost/text/grapheme_view.hpp>
#include <boost/text/detail/breaks_impl.hpp>


namespace boost { namespace text { namespace detail {

    // The logic below is derived from Bidi_Class=Paragraph_Separator in
    // DerivedBidiClass.txt.
    inline bool paragraph_break(uint32_t cp)
    {
        return cp == 0xa ||                  // LF
               cp == 0xd ||                  // CR
               (0x1c <= cp && cp <= 0x1e) || // INFORMATION SEPARATORs
               cp == 0x85 ||                 // NEXT LINE (NEL)
               cp == 0x2029                  // PARAGRAPH SEPARATOR
            ;
    }

    template<typename CPIter, typename Sentinel>
    CPIter
    prev_paragraph_break_impl(CPIter first, CPIter it, Sentinel last) noexcept
    {
        // CRLF special case.
        if (it != first && it != last && *std::prev(it) == 0xd && *it == 0xa)
            --it;
        auto prev_it =
            boost::text::find_if_backward(first, it, detail::paragraph_break);
        if (prev_it == it)
            return first;
        return ++prev_it;
    }

    template<typename CPIter, typename Sentinel>
    CPIter next_paragraph_break_impl(CPIter first, Sentinel last) noexcept
    {
        if (first == last)
            return first;
        first = boost::text::find_if(
            std::next(first), last, detail::paragraph_break);
        if (first == last)
            return first;
        // Eat LF after CR.
        if (*first == 0xd && std::next(first) != last &&
            *std::next(first) == 0xa) {
            ++first;
        }
        return ++first;
    }

    template<typename CPIter, typename Sentinel>
    struct next_paragraph_callable
    {
        auto operator()(CPIter it, Sentinel last) const noexcept
            -> cp_iter_ret_t<CPIter, CPIter>
        {
            return detail::next_paragraph_break_impl(it, last);
        }
    };

    template<typename CPIter>
    struct prev_paragraph_callable
    {
        auto operator()(CPIter first, CPIter it, CPIter last) const noexcept
            -> cp_iter_ret_t<CPIter, CPIter>
        {
            return detail::prev_paragraph_break_impl(first, it, last);
        }
    };


    template<typename CPRange, typename CPIter>
    iterator_t<CPRange>
    prev_paragraph_break_cr_impl(CPRange && range, CPIter it) noexcept
    {
        return detail::prev_paragraph_break_impl(
            detail::begin(range), it, detail::end(range));
    }

    template<typename GraphemeRange, typename GraphemeIter>
    iterator_t<GraphemeRange> prev_paragraph_break_gr_impl(
        GraphemeRange && range, GraphemeIter it) noexcept
    {
        using cp_iter_t = decltype(range.begin().base());
        return {
            range.begin().base(),
            detail::prev_paragraph_break_impl(
                range.begin().base(),
                static_cast<cp_iter_t>(it.base()),
                range.end().base()),
            range.end().base()};
    }

    template<typename CPRange, typename CPIter>
    iterator_t<CPRange>
    next_paragraph_break_cr_impl(CPRange && range, CPIter it) noexcept
    {
        return detail::next_paragraph_break_impl(it, detail::end(range));
    }

    template<typename GraphemeRange, typename GraphemeIter>
    iterator_t<GraphemeRange> next_paragraph_break_gr_impl(
        GraphemeRange && range, GraphemeIter it) noexcept
    {
        using cp_iter_t = decltype(range.begin().base());
        return {
            range.begin().base(),
            detail::next_paragraph_break_impl(
                static_cast<cp_iter_t>(it.base()), range.end().base()),
            range.end().base()};
    }

    template<typename CPIter, typename Sentinel>
    bool
    at_paragraph_break_impl(CPIter first, CPIter it, Sentinel last) noexcept
    {
        if (it == last)
            return true;
        return detail::prev_paragraph_break_impl(first, it, last) == it;
    }

    template<typename CPRange, typename CPIter>
    bool at_paragraph_break_cr_impl(CPRange && range, CPIter it) noexcept
    {
        if (it == detail::end(range))
            return true;
        return detail::prev_paragraph_break_impl(
                   detail::begin(range), it, detail::end(range)) == it;
    }

    template<typename GraphemeRange, typename GraphemeIter>
    bool
    at_paragraph_break_gr_impl(GraphemeRange && range, GraphemeIter it) noexcept
    {
        if (it == detail::end(range))
            return true;
        using cp_iter_t = decltype(range.begin().base());
        cp_iter_t it_ = static_cast<cp_iter_t>(it.base());
        return detail::prev_paragraph_break_impl(
                   range.begin().base(), it_, range.end().base()) == it_;
    }

    template<typename CPIter, typename Sentinel>
    utf32_view<CPIter>
    paragraph_impl(CPIter first, CPIter it, Sentinel last) noexcept
    {
        first = detail::prev_paragraph_break_impl(first, it, last);
        return utf32_view<CPIter>{
            first, detail::next_paragraph_break_impl(first, last)};
    }

    template<typename CPRange, typename CPIter>
    utf32_view<iterator_t<CPRange>>
    paragraph_cr_impl(CPRange && range, CPIter it) noexcept
    {
        auto first = detail::prev_paragraph_break_impl(
            detail::begin(range), it, detail::end(range));
        return utf32_view<CPIter>{
            first, detail::next_paragraph_break_impl(first, detail::end(range))};
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto paragraph_gr_impl(GraphemeRange && range, GraphemeIter it) noexcept
        -> grapheme_view<decltype(range.begin().base())>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto first = detail::prev_paragraph_break_impl(
            range.begin().base(),
            static_cast<cp_iter_t>(it.base()),
            range.end().base());
        return {
            range.begin().base(),
            first,
            detail::next_paragraph_break_impl(first, range.end().base()),
            range.end().base()};
    }

}}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest paragraph break at or before before `it`.  If `it ==
        first`, that is returned.  Otherwise, the first code point of the
        paragraph that `it` is within is returned (even if `it` is already at
        the first code point of a paragraph).

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    CPIter
    prev_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept;

    /** Finds the next paragraph break after `first`.  This will be the first
        code point after the current paragraph, or `last` if no next paragraph
        exists.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept.

        \pre `first` is at the beginning of a paragraph. */
    template<typename CPIter, typename Sentinel>
    CPIter next_paragraph_break(CPIter first, Sentinel last) noexcept;

    /** Finds the nearest paragraph break at or before before `it`.  If `it ==
        range.begin()`, that is returned.  Otherwise, the first code point of
        the paragraph that `it` is within is returned (even if `it` is already
        at the first code point of a paragraph).

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept. */
    template<typename CPRange, typename CPIter>
    detail::unspecified
    prev_paragraph_break(CPRange && range, CPIter it) noexcept;

    /** Returns a grapheme_iterator to the nearest paragraph break at or
        before before `it`.  If `it == range.begin()`, that is returned.
        Otherwise, the first grapheme of the paragraph that `it` is within is
        returned (even if `it` is already at the first grapheme of a
        paragraph).

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    prev_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept;

    /** Finds the next paragraph break after `it`.  This will be the first
        code point after the current paragraph, or `range.end()` if no next
        paragraph exists.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept.

        \pre `it` is at the beginning of a paragraph. */
    template<typename CPRange, typename CPIter>
    detail::unspecified
    next_paragraph_break(CPRange && range, CPIter it) noexcept;

    /** Returns a grapheme_iterator to the next paragraph break after `it`.
        This will be the first grapheme after the current paragraph, or
        `range.end()` if no next paragraph exists.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept.

        \pre `it` is at the beginning of a paragraph. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    next_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept;

    /** Returns true iff `it` is at the beginning of a paragraph, or `it ==
        last`.

        This function only participates in overload resolution if `CPIter`
        models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    bool at_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept;

    /** Returns true iff `it` is at the beginning of a paragraph, or `it ==
        std::ranges::end(range)`.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept. */
    template<typename CPRange, typename CPIter>
    bool at_paragraph_break(CPRange && range, CPIter it) noexcept;

    /** Returns true iff `it` is at the beginning of a paragraph, or `it ==
        std::ranges::end(range)`.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept. */
    template<typename GraphemeRange, typename GraphemeIter>
    bool
    at_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept;

#else

    template<typename CPIter, typename Sentinel>
    auto prev_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::prev_paragraph_break_impl(first, it, last);
    }

    template<typename CPIter, typename Sentinel>
    auto next_paragraph_break(
        CPIter first,
        Sentinel last) noexcept->detail::cp_iter_ret_t<CPIter, CPIter>
    {
        return detail::next_paragraph_break_impl(first, last);
    }

    template<typename CPRange, typename CPIter>
    auto prev_paragraph_break(CPRange && range, CPIter it) noexcept
        ->detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::prev_paragraph_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto prev_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept
        ->detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        return detail::prev_paragraph_break_gr_impl(range, it);
    }

    template<typename CPRange, typename CPIter>
    auto next_paragraph_break(CPRange && range, CPIter it) noexcept
        ->detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return detail::next_paragraph_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto next_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept
        ->detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        return detail::next_paragraph_break_gr_impl(range, it);
    }

    template<typename CPIter, typename Sentinel>
    auto at_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept
        ->detail::cp_iter_ret_t<bool, CPIter>
    {
        return detail::at_paragraph_break_impl(first, it, last);
    }

    template<typename CPRange, typename CPIter>
    auto at_paragraph_break(
        CPRange && range,
        CPIter it) noexcept->detail::cp_rng_alg_ret_t<bool, CPRange>
    {
        return detail::at_paragraph_break_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto at_paragraph_break(GraphemeRange && range, GraphemeIter it) noexcept
        ->detail::graph_rng_alg_ret_t<bool, GraphemeRange>
    {
        return detail::at_paragraph_break_gr_impl(range, it);
    }

#endif

    /** Returns the bounds of the paragraph that `it` lies
        within. */
    template<typename CPIter, typename Sentinel>
    utf32_view<CPIter> paragraph(
        CPIter first, CPIter it, Sentinel last) noexcept
    {
        return detail::paragraph_impl(first, it, last);
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns the bounds of the paragraph that `it` lies within, as a
        utf32_view.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept. */
    template<typename CPRange, typename CPIter>
    detail::unspecified paragraph(CPRange && range, CPIter it) noexcept;

    /** Returns grapheme range delimiting the bounds of the paragraph that
        `it` lies within, as a grapheme_view.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    paragraph(GraphemeRange && range, GraphemeIter it) noexcept;

    /** Returns a view of the code point ranges delimiting paragraphs in
        `[first, last)`. */
    template<typename CPIter, typename Sentinel>
    detail::unspecified paragraphs(CPIter first, Sentinel last) noexcept;

    /** Returns a view of the code point ranges delimiting paragraphs in
        `range`.  The result is returned as a `borrowed_view_t` in C++20 and
        later.

        This function only participates in overload resolution if `CPRange`
        models the CPRange concept. */
    template<typename CPRange>
    detail::unspecified paragraphs(CPRange && range) noexcept;

    /** Returns a view of the grapheme ranges delimiting paragraphs in
        `range`.  The result is returned as a `borrowed_view_t` in C++20 and
        later.

        This function only participates in overload resolution if
        `GraphemeRange` models the GraphemeRange concept. */
    template<typename GraphemeRange>
    detail::unspecified paragraphs(GraphemeRange && range) noexcept;

#else

    template<typename CPRange, typename CPIter>
    auto paragraph(CPRange && range, CPIter it) noexcept -> detail::
        cp_rng_alg_ret_t<utf32_view<detail::iterator_t<CPRange>>, CPRange>
    {
        return detail::paragraph_cr_impl(range, it);
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto paragraph(GraphemeRange && range, GraphemeIter it) noexcept
        ->detail::graph_rng_alg_ret_t<
            grapheme_view<decltype(range.begin().base())>,
            GraphemeRange>
    {
        return detail::paragraph_gr_impl(range, it);
    }

    namespace dtl {
        struct paragraphs_impl : range_adaptor_closure<paragraphs_impl>
        {
            template<typename CPIter, typename Sentinel>
            auto operator()(CPIter first, Sentinel last) const noexcept
                -> decltype(detail::breaks_impl<
                            detail::prev_paragraph_callable,
                            detail::next_paragraph_callable>(first, last))
            {
                return detail::breaks_impl<
                    detail::prev_paragraph_callable,
                    detail::next_paragraph_callable>(first, last);
            }

            template<typename CPRange>
            auto operator()(CPRange && range) const noexcept
                -> detail::cp_rng_alg_ret_t<
                    decltype(detail::breaks_cr_impl<
                             detail::prev_paragraph_callable,
                             detail::next_paragraph_callable>(range)),
                    CPRange>
            {
                return detail::breaks_cr_impl<
                    detail::prev_paragraph_callable,
                    detail::next_paragraph_callable>(range);
            }

            template<typename GraphemeRange>
            auto operator()(GraphemeRange && range) const noexcept
                -> detail::graph_rng_alg_ret_t<
                    decltype(detail::breaks_gr_impl<
                             detail::prev_paragraph_callable,
                             detail::next_paragraph_callable>(range)),
                    GraphemeRange>
            {
                return detail::breaks_gr_impl<
                    detail::prev_paragraph_callable,
                    detail::next_paragraph_callable>(range);
            }
        };
    }

#if defined(__cpp_inline_variables)
    inline constexpr dtl::paragraphs_impl paragraphs;
#else
    namespace {
        constexpr dtl::paragraphs_impl paragraphs;
    }
#endif

#endif

}}}

#if defined(BOOST_TEXT_DOXYGEN) || BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<code_point_iter I, std::sentinel_for<I> S>
    I prev_paragraph_break(I first, I it, S last) noexcept
    {
        return detail::prev_paragraph_break_impl(first, it, last);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    I next_paragraph_break(I first, S last) noexcept
    {
        return detail::next_paragraph_break_impl(first, last);
    }

    template<code_point_range R>
    std::ranges::iterator_t<R> prev_paragraph_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::prev_paragraph_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::iterator_t<R> prev_paragraph_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::prev_paragraph_break_gr_impl(r, it);
    }

    template<code_point_range R>
    std::ranges::iterator_t<R> next_paragraph_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::next_paragraph_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    std::ranges::iterator_t<R> next_paragraph_break(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::next_paragraph_break_gr_impl(r, it);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    bool at_paragraph_break(I first, I it, S last) noexcept
    {
        return detail::at_paragraph_break_impl(first, it, last);
    }

    template<code_point_range R>
    bool at_paragraph_break(R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::at_paragraph_break_cr_impl(r, it);
    }

    template<grapheme_range R>
    bool at_paragraph_break(R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::at_paragraph_break_gr_impl(r, it);
    }

    template<code_point_iter I, std::sentinel_for<I> S>
    utf32_view<I> paragraph(I first, I it, S last) noexcept
    {
        return detail::paragraph_impl(first, it, last);
    }

    template<code_point_range R>
    utf32_view<std::ranges::iterator_t<R>> paragraph(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::paragraph_cr_impl(r, it);
    }

    template<grapheme_range R>
    grapheme_view<code_point_iterator_t<R>> paragraph(
        R && r, std::ranges::iterator_t<R> it) noexcept
    {
        return detail::paragraph_gr_impl(r, it);
    }

    namespace dtl {
        struct paragraphs_impl : range_adaptor_closure<paragraphs_impl>
        {
            template<code_point_iter I, std::sentinel_for<I> S>
            auto operator()(I first, S last) const noexcept
            {
                return detail::breaks_impl<
                    detail::prev_paragraph_callable,
                    detail::next_paragraph_callable>(first, last);
            }

            template<code_point_range R>
            auto operator()(R && r) const noexcept
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::breaks_cr_impl<
                        detail::prev_paragraph_callable,
                        detail::next_paragraph_callable>(r);
                } else {
                    return std::ranges::dangling{};
                }
            }

            template<grapheme_range R>
            auto operator()(R && r) const noexcept
            {
                if constexpr (std::ranges::borrowed_range<R>) {
                    return detail::breaks_gr_impl<
                        detail::prev_paragraph_callable,
                        detail::next_paragraph_callable>(r);
                } else {
                    return std::ranges::dangling{};
                }
            }
        };
    }

    inline constexpr dtl::paragraphs_impl paragraphs;

}}}

#endif

#endif
