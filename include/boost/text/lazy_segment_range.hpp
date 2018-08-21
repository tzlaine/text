#ifndef BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP
#define BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP

#include <boost/text/utility.hpp>


namespace boost { namespace text {

    template<
        typename CPIter,
        typename Sentinel,
        typename NextFunc,
        typename CPRange = cp_range<CPIter>>
    struct lazy_segment_range;

    namespace detail {
        template<
            typename CPIter,
            typename Sentinel,
            typename NextFunc,
            typename CPRange>
        struct const_lazy_segment_iterator;

        template<typename CPIter, typename CPRange>
        struct segment_arrow_proxy
        {
            CPRange * operator->() const noexcept
            {
                return &value_;
            }

        private:
            template<
                typename CPIter_,
                typename Sentinel_,
                typename NextFunc_,
                typename CPRange_>
            friend struct const_lazy_segment_iterator;

            segment_arrow_proxy(CPRange value) : value_(value)
            {}

            CPRange value_;
        };

        template<
            typename CPIter,
            typename Sentinel,
            typename NextFunc,
            typename CPRange>
        struct const_lazy_segment_iterator
        {
        private:
            mutable NextFunc * next_func_;
            CPIter prev_;
            mutable CPIter it_;
            Sentinel last_;

            void set_next_func(NextFunc * next_func) const noexcept
            {
                next_func_ = next_func;
                it_ = (*next_func_)(prev_, last_);
            }

            template<
                typename CPIter2,
                typename Sentinel2,
                typename NextFunc2,
                typename CPRange2>
            friend struct ::boost::text::lazy_segment_range;

        public:
            using value_type = CPRange;
            using pointer = detail::segment_arrow_proxy<CPIter, CPRange>;
            using reference = value_type;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            const_lazy_segment_iterator() noexcept :
                next_func_(),
                prev_(),
                it_(),
                last_()
            {}

            const_lazy_segment_iterator(CPIter it, Sentinel last) noexcept :
                next_func_(),
                prev_(it),
                it_(),
                last_(last)
            {}

            const_lazy_segment_iterator(Sentinel last) noexcept :
                next_func_(),
                prev_(),
                it_(),
                last_(last)
            {}

            reference operator*() const noexcept
            {
                return value_type{prev_, it_};
            }

            pointer operator->() const noexcept { return pointer(**this); }

            const_lazy_segment_iterator & operator++() noexcept
            {
                auto const next_it = (*next_func_)(it_, last_);
                prev_ = it_;
                it_ = next_it;
                return *this;
            }

            friend bool operator==(
                const_lazy_segment_iterator lhs,
                const_lazy_segment_iterator rhs) noexcept
            {
                return lhs.prev_ == rhs.last_;
            }
            friend bool operator!=(
                const_lazy_segment_iterator lhs,
                const_lazy_segment_iterator rhs) noexcept
            {
                return !(lhs == rhs);
            }
        };
    }

    /** Represents a range of non-overlapping subranges.  Each subrange
        represents some semantically significant segment, the semantics of
        which are controlled by the NextFunc template parameter.  For
        instance, if NextFunc is next_paragraph_break, the subranges produced
        by lazy_segment_range will be paragraphs.  Each subrange is lazily
        produced; an output subrange is not produced until a lazy range
        iterator is dereferenced. */
    template<
        typename CPIter,
        typename Sentinel,
        typename NextFunc,
        typename CPRange>
    struct lazy_segment_range
    {
        using iterator = detail::
            const_lazy_segment_iterator<CPIter, Sentinel, NextFunc, CPRange>;

        lazy_segment_range() noexcept {}
        lazy_segment_range(
            NextFunc next_func, iterator first, iterator last) noexcept :
            next_func_(std::move(next_func)),
            first_(first),
            last_(last)
        {}

        iterator begin() const noexcept
        {
            first_.set_next_func(const_cast<NextFunc *>(&next_func_));
            return first_;
        }
        iterator end() const noexcept { return last_; }

        NextFunc && next_func() && noexcept { return std::move(next_func_); }

    private:
        NextFunc next_func_;
        iterator first_;
        iterator last_;
    };

}}

#endif
