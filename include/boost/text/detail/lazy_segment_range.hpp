// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP
#define BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP

#include <boost/text/transcode_view.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>


namespace boost { namespace text { namespace detail {

    template<
        typename CPIter,
        typename Sentinel,
        typename NextFunc,
        typename CPRange>
    struct const_lazy_segment_iterator
        : stl_interfaces::proxy_iterator_interface<
              const_lazy_segment_iterator<CPIter, Sentinel, NextFunc, CPRange>,
              std::forward_iterator_tag,
              CPRange>
    {
    private:
        NextFunc * next_func_;
        CPIter prev_;
        CPIter it_;
        [[no_unique_address]] Sentinel last_;

    public:
        const_lazy_segment_iterator() :
            next_func_(), prev_(), it_(), last_()
        {}

        const_lazy_segment_iterator(CPIter it, Sentinel last) :
            next_func_(), prev_(it), it_(), last_(last)
        {}

        const_lazy_segment_iterator(Sentinel last) :
            next_func_(), prev_(), it_(), last_(last)
        {}

        CPRange operator*() const
        {
            return CPRange{std::ranges::subrange(prev_, it_)};
        }

        const_lazy_segment_iterator & operator++()
        {
            auto const next_it = (*next_func_)(it_, last_);
            prev_ = it_;
            it_ = next_it;
            return *this;
        }

        void set_next_func(NextFunc * next_func)
        {
            next_func_ = next_func;
            it_ = (*next_func_)(prev_, last_);
        }

        friend bool operator==(
            const_lazy_segment_iterator lhs,
            const_lazy_segment_iterator rhs)
        {
            return lhs.prev_ == rhs.last_;
        }

        using base_type = stl_interfaces::proxy_iterator_interface<
            const_lazy_segment_iterator<CPIter, Sentinel, NextFunc, CPRange>,
            std::forward_iterator_tag,
            CPRange>;
        using base_type::operator++;
    };

    template<
        typename CPIter,
        typename Ignored,
        typename PrevFunc,
        typename CPRange>
    struct const_reverse_lazy_segment_iterator
        : stl_interfaces::proxy_iterator_interface<
              const_reverse_lazy_segment_iterator<
                  CPIter,
                  Ignored,
                  PrevFunc,
                  CPRange>,
              std::forward_iterator_tag,
              CPRange>
    {
    private:
        PrevFunc * prev_func_;
        CPIter first_;
        CPIter it_;
        CPIter next_;

    public:
        const_reverse_lazy_segment_iterator() :
            prev_func_(), first_(), it_(), next_()
        {}

        const_reverse_lazy_segment_iterator(
            CPIter first, CPIter it, CPIter last) :
            prev_func_(), first_(first), it_(it), next_(last)
        {}

        CPRange operator*() const { return CPRange{it_, next_}; }

        const_reverse_lazy_segment_iterator & operator++()
        {
            if (it_ == first_) {
                next_ = first_;
                return *this;
            }
            auto const prev_it = (*prev_func_)(first_, std::prev(it_), next_);
            next_ = it_;
            it_ = prev_it;
            return *this;
        }

        void set_next_func(PrevFunc * prev_func)
        {
            prev_func_ = prev_func;
            ++*this;
        }

        friend bool operator==(
            const_reverse_lazy_segment_iterator lhs,
            const_reverse_lazy_segment_iterator rhs)
        {
            return lhs.next_ == rhs.first_;
        }

        using base_type = stl_interfaces::proxy_iterator_interface<
            const_reverse_lazy_segment_iterator<
                CPIter,
                Ignored,
                PrevFunc,
                CPRange>,
            std::forward_iterator_tag,
            CPRange>;
        using base_type::operator++;
    };

    template<
        typename CPIter,
        typename Sentinel,
        typename NextFunc,
        typename CPRange = std::ranges::subrange<CPIter>,
        template<class, class, class, class> class IteratorTemplate =
            detail::const_lazy_segment_iterator,
        bool Reverse = false>
    struct lazy_segment_range
    {
        using iterator = IteratorTemplate<CPIter, Sentinel, NextFunc, CPRange>;

        lazy_segment_range() {}
        lazy_segment_range(
            NextFunc next_func, iterator first, iterator last) :
            next_func_(std::move(next_func)), first_(first), last_(last)
        {}

        iterator begin() const
        {
            const_cast<iterator &>(first_).set_next_func(
                const_cast<NextFunc *>(&next_func_));
            return first_;
        }
        iterator end() const { return last_; }

        /** Moves the contained `NextFunc` out of *this. */
        NextFunc && next_func() && { return std::move(next_func_); }

    private:
        NextFunc next_func_;
        iterator first_;
        iterator last_;
    };

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<
        typename CPIter,
        typename Sentinel,
        typename NextFunc,
        typename CPRange,
        template<class, class, class, class>
        class IteratorTemplate>
    inline constexpr bool
        enable_borrowed_range<boost::text::detail::lazy_segment_range<
            CPIter,
            Sentinel,
            NextFunc,
            CPRange,
            IteratorTemplate>> = true;
}

#endif

#endif
