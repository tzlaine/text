#ifndef BOOST_TEXT_PARAGRAPH_BREAK_HPP
#define BOOST_TEXT_PARAGRAPH_BREAK_HPP

#include <boost/text/lazy_segment_range.hpp>


namespace boost { namespace text {

    namespace detail {
        // All the CPs below are derived from the CR, LF, and Sep CPs in
        // sentence_break.cpp.
        inline bool paragraph_break(uint32_t cp)
        {
            return cp == 0xa ||    // LF
                   cp == 0xd ||    // CR
                   cp == 0x85 ||   // Sep
                   cp == 0x2028 || // Sep
                   cp == 0x2029    // Sep
                ;
        }
    }

    /** Finds the nearest paragraph break at or before before <code>it</code>.
        If <code>it == first</code>, that is returned.  Otherwise, the first
        code point of the paragraph <code>it</code> is within is returned
        (even if <code>it</code> is already at the first code point of a
        paragraph. */
    template<typename CPIter>
    CPIter prev_paragraph_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        auto const initial_it = it;
        it = find_if_backward(first, it, detail::paragraph_break);
        if (it == initial_it)
            return first;
        return ++it;
    }

    /** Finds the next paragraph break after <code>it</code>.  This will be
        the first code point after the current paragraph, or <code>last</code>
        if no next paragraph exists.

        \pre <code>it</code> is at the beginning of a paragraph. */
    template<typename CPIter>
    CPIter next_paragraph_break(CPIter first, CPIter last) noexcept
    {
        if (first == last)
            return last;
        first = std::find_if(std::next(first), last, detail::paragraph_break);
        // Eat LF after CR.
        if (*first == 0xd && std::next(first) != last &&
            *std::next(first) == 0xa) {
            ++first;
        }
        return ++first;
    }

    namespace detail {
        template<typename CPIter, typename Sentinel>
        struct next_paragraph_callable
        {
            CPIter operator()(CPIter it, Sentinel last) noexcept
            {
                return next_paragraph_break(it, last);
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

    /** Returns a lazy range of the code point ranges delimiting paragraphs in
        <code>[first, last]</code>. */
    template<typename CPIter, typename Sentinel>
    lazy_segment_range<
        CPIter,
        Sentinel,
        detail::next_paragraph_callable<CPIter, Sentinel>>
    paragraphs(CPIter first, Sentinel last) noexcept
    {
        return {{first, last}, {last, last}};
    }

}}

#endif
