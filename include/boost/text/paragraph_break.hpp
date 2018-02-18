#ifndef BOOST_TEXT_PARAGRAPH_BREAK_HPP
#define BOOST_TEXT_PARAGRAPH_BREAK_HPP


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
        while (it != first) {
            if (detail::paragraph_break(*--it)) {
                ++it;
                break;
            }
        }
        return it;
    }

    /** Finds the next paragraph break after <code>it</code>.  This will be
        the first code point after the current paragraph, or <code>last</code>
        if no next paragraph exists.

        \pre <code>it</code> is at the beginning of a paragraph. */
    template<typename CPIter>
    CPIter next_paragraph_break(CPIter first, CPIter it, CPIter last) noexcept
    {
        if (it == last)
            return last;
        while (++it != last) {
            if (detail::paragraph_break(*it)) {
                // Eat LF after CR.
                if (*it == 0xd && std::next(it) != last &&
                    *std::next(it) == 0xa) {
                    ++it;
                }
                return ++it;
            }
        }
        return it;
    }

    /** Returns the bounds of the paragraph that <code>it</code> lies
        within. */
    template<typename CPIter>
    cp_range<CPIter> paragraph(CPIter first, CPIter it, CPIter last) noexcept
    {
        cp_range<CPIter> retval{prev_paragraph_break(first, it, last)};
        retval.last = next_paragraph_break(first, retval.first, last);
        return retval;
    }

}}

#endif
