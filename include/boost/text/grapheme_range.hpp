#ifndef BOOST_TEXT_GRAPHEME_RANGE_HPP
#define BOOST_TEXT_GRAPHEME_RANGE_HPP

#include <boost/text/grapheme_iterator.hpp>


namespace boost { namespace text {

    /** A range of graphemes. */
    template<typename CPIter>
    struct grapheme_range
    {
        using iterator = grapheme_iterator<CPIter>;

        grapheme_range() : first_(), last_() {}

        grapheme_range(CPIter first, CPIter last) :
            first_(first, first, last),
            last_(first, last, last)
        {}

        bool empty() const noexcept { return first_ == last_; }

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
    };

}}

#endif
