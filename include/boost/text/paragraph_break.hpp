#ifndef BOOST_TEXT_PARAGRAPH_BREAK_HPP
#define BOOST_TEXT_PARAGRAPH_BREAK_HPP

#include <boost/text/grapheme_range.hpp>
#include <boost/text/lazy_segment_range.hpp>


namespace boost { namespace text {

    namespace detail {
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
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Finds the nearest paragraph break at or before before <code>it</code>.
        If <code>it == first</code>, that is returned.  Otherwise, the first
        code point of the paragraph that <code>it</code> is within is returned
        (even if <code>it</code> is already at the first code point of a
        paragraph).

        This function only participates in overload resolution if
        <code>CPIter</code> models the CPIter concept. */
    template<typename CPIter, typename Sentinel>
    CPIter
    prev_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept;

    /** Finds the next paragraph break after <code>first</code>.  This will be
        the first code point after the current paragraph, or <code>last</code>
        if no next paragraph exists.

        This function only participates in overload resolution if
        <code>CPIter</code> models the CPIter concept.

        \pre <code>first</code> is at the beginning of a paragraph. */
    template<typename CPIter, typename Sentinel>
    CPIter next_paragraph_break(CPIter first, Sentinel last) noexcept;

    /** Finds the nearest paragraph break at or before before <code>it</code>.
        If <code>it == range.begin()</code>, that is returned.  Otherwise, the
        first code point of the paragraph that <code>it</code> is within is
        returned (even if <code>it</code> is already at the first code point
        of a paragraph).

        This function only participates in overload resolution if
        <code>CPRange</code> models the CPRange concept. */
    template<typename CPRange, typename CPIter>
    detail::unspecified
    prev_paragraph_break(CPRange & range, CPIter it) noexcept;

    /** Returns a grapheme_iterator to the nearest paragraph break at or
        before before <code>it</code>.  If <code>it == range.begin()</code>,
        that is returned.  Otherwise, the first grapheme of the paragraph that
        <code>it</code> is within is returned (even if <code>it</code> is
        already at the first grapheme of a paragraph).

        This function only participates in overload resolution if
        <code>GraphemeRange</code> models the GraphemeRange concept. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    prev_paragraph_break(GraphemeRange const & range, GraphemeIter it) noexcept;

    /** Finds the next paragraph break after <code>it</code>.  This will be
        the first code point after the current paragraph, or
        <code>range.end()</code> if no next paragraph exists.

        This function only participates in overload resolution if
        <code>CPRange</code> models the CPRange concept.

        \pre <code>it</code> is at the beginning of a paragraph. */
    template<typename CPRange, typename CPIter>
    detail::unspecified
    next_paragraph_break(CPRange & range, CPIter it) noexcept;

    /** Returns a grapheme_iterator to the next paragraph break after
        <code>it</code>.  This will be the first grapheme after the current
        paragraph, or <code>range.end()</code> if no next paragraph exists.

        This function only participates in overload resolution if
        <code>GraphemeRange</code> models the GraphemeRange concept.

        \pre <code>it</code> is at the beginning of a paragraph. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    next_paragraph_break(GraphemeRange const & range, GraphemeIter it) noexcept;

#else

    template<typename CPIter, typename Sentinel>
    auto prev_paragraph_break(CPIter first, CPIter it, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        // CRLF special case.
        if (it != first && it != last && *std::prev(it) == 0xd && *it == 0xa)
            --it;
        auto prev_it = find_if_backward(first, it, detail::paragraph_break);
        if (prev_it == it)
            return first;
        return ++prev_it;
    }

    template<typename CPIter, typename Sentinel>
    auto next_paragraph_break(CPIter first, Sentinel last) noexcept
        -> detail::cp_iter_ret_t<CPIter, CPIter>
    {
        if (first == last)
            return first;
        first = find_if(std::next(first), last, detail::paragraph_break);
        if (first == last)
            return first;
        // Eat LF after CR.
        if (*first == 0xd && std::next(first) != last &&
            *std::next(first) == 0xa) {
            ++first;
        }
        return ++first;
    }

    template<typename CPRange, typename CPIter>
    auto prev_paragraph_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return prev_paragraph_break(std::begin(range), it, std::end(range));
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto
    prev_paragraph_break(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                prev_paragraph_break(
                    range.begin().base(),
                    static_cast<cp_iter_t>(it.base()),
                    range.end().base()),
                range.end().base()};
    }

    template<typename CPRange, typename CPIter>
    auto next_paragraph_break(CPRange & range, CPIter it) noexcept
        -> detail::cp_rng_alg_ret_t<detail::iterator_t<CPRange>, CPRange>
    {
        return next_paragraph_break(it, std::end(range));
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto
    next_paragraph_break(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            detail::iterator_t<GraphemeRange const>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        return {range.begin().base(),
                next_paragraph_break(
                    static_cast<cp_iter_t>(it.base()), range.end().base()),
                range.end().base()};
    }

#endif

    namespace detail {
        template<typename CPIter, typename Sentinel>
        struct next_paragraph_callable
        {
            auto operator()(CPIter it, Sentinel last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return next_paragraph_break(it, last);
            }
        };

        template<typename CPIter>
        struct prev_paragraph_callable
        {
            auto operator()(CPIter first, CPIter it, CPIter last) const noexcept
                -> detail::cp_iter_ret_t<CPIter, CPIter>
            {
                return prev_paragraph_break(first, it, last);
            }
        };
    }

    /** Returns the bounds of the paragraph that <code>it</code> lies
        within. */
    template<typename CPIter, typename Sentinel>
    cp_range<CPIter> paragraph(CPIter first, CPIter it, Sentinel last) noexcept
    {
        first = prev_paragraph_break(first, it, last);
        return cp_range<CPIter>{first, next_paragraph_break(first, last)};
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Returns the bounds of the paragraph that <code>it</code> lies
        within, as a cp_range.

        This function only participates in overload resolution if
        <code>CPRange</code> models the CPRange concept. */
    template<typename CPRange, typename CPIter>
    detail::unspecified paragraph(CPRange & range, CPIter it) noexcept;

    /** Returns grapheme range delimiting the bounds of the paragraph that
        <code>it</code> lies within, as a grapheme_range.

        This function only participates in overload resolution if
        <code>GraphemeRange</code> models the GraphemeRange concept. */
    template<typename GraphemeRange, typename GraphemeIter>
    detail::unspecified
    paragraph(GraphemeRange const & range, GraphemeIter it) noexcept;

    /** Returns a lazy range of the code point ranges delimiting paragraphs in
        <code>[first, last)</code>. */
    template<typename CPIter, typename Sentinel>
    detail::unspecified paragraphs(CPIter first, Sentinel last) noexcept;

    /** Returns a lazy range of the code point ranges delimiting paragraphs in
        <code>range</code>.

        This function only participates in overload resolution if
        <code>CPRange</code> models the CPRange concept. */
    template<typename CPRange>
    detail::unspecified paragraphs(CPRange & range) noexcept;

    /** Returns a lazy range of the grapheme ranges delimiting paragraphs in
        <code>range</code>.

        This function only participates in overload resolution if
        <code>GraphemeRange</code> models the GraphemeRange concept. */
    template<typename GraphemeRange>
    detail::unspecified paragraphs(GraphemeRange const & range) noexcept;

    /** Returns a lazy range of the code point ranges delimiting paragraphs in
        <code>[first, last)</code>, in reverse. */
    template<typename CPIter>
    detail::unspecified reversed_paragraphs(CPIter first, CPIter last) noexcept;

    /** Returns a lazy range of the code point ranges delimiting paragraphs in
        <code>range</code>, in reverse.

        This function only participates in overload resolution if
        <code>CPRange</code> models the CPRange concept. */
    template<typename CPRange>
    detail::unspecified reversed_paragraphs(CPRange & range) noexcept;

    /** Returns a lazy range of the grapheme ranges delimiting paragraphs in
        <code>range</code>, in reverse.

        This function only participates in overload resolution if
        <code>GraphemeRange</code> models the GraphemeRange concept. */
    template<typename GraphemeRange>
    detail::unspecified
    reversed_paragraphs(GraphemeRange const & range) noexcept;

#else

    template<typename CPRange, typename CPIter>
    auto paragraph(CPRange & range, CPIter it) noexcept -> detail::
        cp_rng_alg_ret_t<cp_range<detail::iterator_t<CPRange>>, CPRange>
    {
        auto first =
            prev_paragraph_break(std::begin(range), it, std::end(range));
        return cp_range<CPIter>{first,
                                next_paragraph_break(first, std::end(range))};
    }

    template<typename GraphemeRange, typename GraphemeIter>
    auto paragraph(GraphemeRange const & range, GraphemeIter it) noexcept
        -> detail::graph_rng_alg_ret_t<
            grapheme_range<decltype(range.begin().base())>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        auto first = prev_paragraph_break(
            range.begin().base(),
            static_cast<cp_iter_t>(it.base()),
            range.end().base());
        return {first, next_paragraph_break(first, range.end().base())};
    }

    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_paragraph_callable<CPIter, Sentinel>>
    paragraphs(CPIter first, Sentinel last) noexcept
    {
        detail::next_paragraph_callable<CPIter, Sentinel> next;
        return {std::move(next), {first, last}, {last}};
    }

    template<typename CPRange>
    auto paragraphs(CPRange & range) noexcept -> detail::cp_rng_alg_ret_t<
        lazy_segment_range<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>,
            detail::next_paragraph_callable<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>>>,
        CPRange>
    {
        detail::next_paragraph_callable<
            detail::iterator_t<CPRange>,
            detail::sentinel_t<CPRange>>
            next;
        return {std::move(next),
                {std::begin(range), std::end(range)},
                {std::end(range)}};
    }

    template<typename GraphemeRange>
    auto paragraphs(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::next_paragraph_callable<
                    decltype(range.begin().base()),
                    decltype(range.begin().base())>,
                grapheme_range<decltype(range.begin().base())>>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::next_paragraph_callable<cp_iter_t, cp_iter_t> next;
        return {std::move(next),
                {range.begin().base(), range.end().base()},
                {range.end().base()}};
    }

    template<typename CPIter>
    lazy_segment_range<
        CPIter,
        CPIter,
        detail::prev_paragraph_callable<CPIter>,
        cp_range<CPIter>,
        detail::const_reverse_lazy_segment_iterator,
        true>
    reversed_paragraphs(CPIter first, CPIter last) noexcept
    {
        detail::prev_paragraph_callable<CPIter> prev;
        return {std::move(prev), {first, last, last}, {first, first, last}};
    }

    template<typename CPRange>
    auto reversed_paragraphs(CPRange & range) noexcept
        -> detail::cp_rng_alg_ret_t<
            lazy_segment_range<
                detail::iterator_t<CPRange>,
                detail::sentinel_t<CPRange>,
                detail::prev_paragraph_callable<detail::iterator_t<CPRange>>,
                cp_range<detail::iterator_t<CPRange>>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            CPRange>
    {
        detail::prev_paragraph_callable<detail::iterator_t<CPRange>> prev;
        return {std::move(prev),
                {std::begin(range), std::end(range), std::end(range)},
                {std::begin(range), std::begin(range), std::end(range)}};
    }

    template<typename GraphemeRange>
    auto reversed_paragraphs(GraphemeRange const & range) noexcept
        -> detail::graph_rng_alg_ret_t<
            lazy_segment_range<
                decltype(range.begin().base()),
                decltype(range.begin().base()),
                detail::prev_paragraph_callable<decltype(range.begin().base())>,
                grapheme_range<decltype(range.begin().base())>,
                detail::const_reverse_lazy_segment_iterator,
                true>,
            GraphemeRange>
    {
        using cp_iter_t = decltype(range.begin().base());
        detail::prev_paragraph_callable<cp_iter_t> prev;
        return {
            std::move(prev),
            {range.begin().base(), range.end().base(), range.end().base()},
            {range.begin().base(), range.begin().base(), range.end().base()}};
    }

#endif

}}

#endif
