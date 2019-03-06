#ifndef BOOST_TEXT_ALGORITHM_HPP
#define BOOST_TEXT_ALGORITHM_HPP

#include <boost/text/detail/sentinel_tag.hpp>

#include <cstddef>
#include <iterator>


namespace boost { namespace text {

    namespace detail {
        template<typename Iter>
        std::ptrdiff_t distance(Iter first, Iter last, non_sentinel_tag)
        {
            return std::distance(first, last);
        }

        template<typename Iter, typename Sentinel>
        std::ptrdiff_t distance(Iter first, Sentinel last, sentinel_tag)
        {
            std::ptrdiff_t retval = 0;
            while (first != last) {
                ++retval;
                ++first;
            }
            return retval;
        }
    }

    /** Range-friendly version of <code>std::distance()</code>, taking an
        iterator and a sentinel. */
    template<typename Iter, typename Sentinel>
    std::ptrdiff_t distance(Iter first, Sentinel last)
    {
        return detail::distance(
            first,
            last,
            typename std::conditional<
                std::is_same<Iter, Sentinel>::value,
                detail::non_sentinel_tag,
                detail::sentinel_tag>::type());
    }

    /** Range-friendly version of <code>std::find()</code>, taking an iterator
        and a sentinel. */
    template<typename BidiIter, typename Sentinel, typename T>
    BidiIter find(BidiIter first, Sentinel last, T const & x)
    {
        while (first != last) {
            if (*first == x)
                return first;
            ++first;
        }
        return first;
    }

    /** A range-friendly compliment to <code>std::find()</code>; returns an
        iterator to the first element not equal to <code>x</code>. */
    template<typename BidiIter, typename Sentinel, typename T>
    BidiIter find_not(BidiIter first, Sentinel last, T const & x)
    {
        while (first != last) {
            if (*first != x)
                return first;
            ++first;
        }
        return first;
    }

    /** Range-friendly version of <code>std::find_if()</code>, taking an
        iterator and a sentinel. */
    template<typename BidiIter, typename Sentinel, typename Pred>
    BidiIter find_if(BidiIter first, Sentinel last, Pred p)
    {
        while (first != last) {
            if (p(*first))
                return first;
            ++first;
        }
        return first;
    }

    /** Range-friendly version of <code>std::find_if_not()</code>, taking an
        iterator and a sentinel. */
    template<typename BidiIter, typename Sentinel, typename Pred>
    BidiIter find_if_not(BidiIter first, Sentinel last, Pred p)
    {
        while (first != last) {
            if (!p(*first))
                return first;
            ++first;
        }
        return first;
    }

    /** Analogue of <code>std::find()</code> that finds the last value in
        <code>[first, last)</code> equal to <code>x</code>. */
    template<typename BidiIter, typename T>
    BidiIter find_backward(BidiIter first, BidiIter last, T const & x)
    {
        auto it = last;
        while (it != first) {
            if (*--it == x)
                return it;
        }
        return last;
    }

    /** Analogue of <code>std::find()</code> that finds the last value in
        <code>[first, last)</code> not equal to <code>x</code>. */
    template<typename BidiIter, typename T>
    BidiIter find_not_backward(BidiIter first, BidiIter last, T const & x)
    {
        auto it = last;
        while (it != first) {
            if (*--it != x)
                return it;
        }
        return last;
    }

    /** Analogue of <code>std::find()</code> that finds the last value
        <code>v</code> in <code>[first, last)</code> for which
        <code>p(v)</code> is true. */
    template<typename BidiIter, typename Pred>
    BidiIter find_if_backward(BidiIter first, BidiIter last, Pred p)
    {
        auto it = last;
        while (it != first) {
            if (p(*--it))
                return it;
        }
        return last;
    }

    /** Analogue of <code>std::find()</code> that finds the last value
        <code>v</code> in <code>[first, last)</code> for which
        <code>p(v)</code> is false. */
    template<typename BidiIter, typename Pred>
    BidiIter find_if_not_backward(BidiIter first, BidiIter last, Pred p)
    {
        auto it = last;
        while (it != first) {
            if (!p(*--it))
                return it;
        }
        return last;
    }

    /** A utility range type returned by <code>foreach_subrange*()</code>. */
    template<typename Iter, typename Sentinel = Iter>
    struct foreach_subrange_range
    {
        using iterator = Iter;
        using sentinel = Sentinel;

        foreach_subrange_range() {}
        foreach_subrange_range(iterator first, sentinel last) :
            first_(first),
            last_(last)
        {}

        iterator begin() const noexcept { return first_; }
        sentinel end() const noexcept { return last_; }

    private:
        iterator first_;
        sentinel last_;
    };

    /** Calls <code>f(sub)</code> for each subrange <code>sub</code> in
        <code>[first, last)</code>.  A subrange is a contiguous subsequence of
        elements that each compares equal to the first element of the
        subsequence.  Subranges passed to <code>f</code> are
        non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename Func>
    Func foreach_subrange(FwdIter first, Sentinel last, Func f)
    {
        while (first != last) {
            auto const & x = *first;
            auto const next = find_not(first, last, x);
            if (first != next)
                f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** Calls <code>f(sub)</code> for each subrange <code>sub</code> in
        <code>[first, last)</code>.  A subrange is a contiguous subsequence of
        elements that for each element <code>e</code>, <code>proj(e)</code>
        each compares equal to <code>proj()</code> of the first element of the
        subsequence.  Subranges passed to <code>f</code> are
        non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename Func, typename Proj>
    Func foreach_subrange(FwdIter first, Sentinel last, Func f, Proj proj)
    {
        using value_type = typename std::iterator_traits<FwdIter>::value_type;
        while (first != last) {
            auto const & x = proj(*first);
            auto const next = find_if_not(
                first, last, [&x, proj](const value_type & element) {
                    return proj(element) == x;
                });
            if (first != next)
                f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** Calls <code>f(sub)</code> for each subrange <code>sub</code> in
        <code>[first, last)</code>.  A subrange is a contiguous subsequence of
        elements, each of which is equal to <code>x</code>.  Subranges passed
        to <code>f</code> are non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename T, typename Func>
    Func foreach_subrange_of(FwdIter first, Sentinel last, T const & x, Func f)
    {
        while (first != last) {
            first = find(first, last, x);
            auto const next = find_not(first, last, x);
            if (first != next)
                f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** Calls <code>f(sub)</code> for each subrange <code>sub</code> in
        <code>[first, last)</code>.  A subrange is a contiguous subsequence of
        elements <code>ei</code> for which <code>p(ei)</code> is true.
        Subranges passed to <code>f</code> are non-overlapping. */
    template<typename FwdIter, typename Sentinel, typename Pred, typename Func>
    Func foreach_subrange_if(FwdIter first, Sentinel last, Pred p, Func f)
    {
        while (first != last) {
            first = boost::text::find_if(first, last, p);
            auto const next = boost::text::find_if_not(first, last, p);
            if (first != next)
                f(foreach_subrange_range<FwdIter, Sentinel>(first, next));
            first = next;
        }
        return f;
    }

    /** Sentinel-friendly version of <code>std::all_of()</code>. */
    template<typename Iter, typename Sentinel, typename Pred>
    bool all_of(Iter first, Sentinel last, Pred p)
    {
        for (; first != last; ++first) {
            if (!p(*first))
                return false;
        }
        return true;
    }

}}

#endif
