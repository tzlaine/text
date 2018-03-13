#ifndef BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP
#define BOOST_TEXT_LAZY_SEGMENT_RANGE_HPP

#include <boost/text/utility.hpp>


namespace boost { namespace text {

    namespace detail {
        template<typename CPIter, typename Sentinel, typename NextFunc>
        struct const_lazy_segment_iterator;

        template<typename CPIter>
        struct segment_arrow_proxy
        {
            cp_range<CPIter> * operator->() const noexcept { return &value_; }

        private:
            template<typename CPIter_, typename NextFunc>
            friend struct const_lazy_segment_iterator;

            segment_arrow_proxy(cp_range<CPIter> value) : value_(value) {}

            cp_range<CPIter> value_;
        };

        template<typename CPIter, typename Sentinel, typename NextFunc>
        struct const_lazy_segment_iterator
        {
        private:
            CPIter prev_;
            CPIter it_;
            Sentinel last_;

        public:
            using value_type = cp_range<CPIter>;
            using pointer = detail::segment_arrow_proxy<CPIter>;
            using reference = value_type;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            const_lazy_segment_iterator(CPIter it, Sentinel last) noexcept :
                prev_(it),
                it_(NextFunc{}(it, last)),
                last_(last)
            {}

            reference operator*() const noexcept
            {
                return value_type{prev_, it_};
            }

            pointer operator->() const noexcept { return pointer(**this); }

            const_lazy_segment_iterator & operator++() noexcept
            {
                auto const next_it = NextFunc{}(it_, last_);
                prev_ = it_;
                it_ = next_it;
                return *this;
            }
            const_lazy_segment_iterator operator++(int)noexcept
            {
                auto const retval = *this;
                ++*this;
                return retval;
            }

            friend bool operator==(
                const_lazy_segment_iterator lhs,
                const_lazy_segment_iterator rhs) noexcept
            {
                return lhs.it_ == rhs.last_;
            }
            friend bool operator!=(
                const_lazy_segment_iterator lhs,
                const_lazy_segment_iterator rhs) noexcept
            {
                return lhs.it_ != rhs.last_;
            }
        };
    }

    /** Represents a range of nonoverlapping ranges.  Each range represents
        some semantically significant segment, the semantics of which are
        controlled by the <code>Func</code> template parameter.  For instance,
        if <code>Func</code> is <code>next_paragraph_break</code>, the ranges
        produced by <code>lazy_segment_range</code> will be paragraphs.  Each
        range is lazily produced; an output range is not produced until the
        point at which one of the lazy range's iterators is dereferenced. */
    template<typename CPIter, typename Sentinel, typename Func>
    struct lazy_segment_range
    {
        using iterator =
            detail::const_lazy_segment_iterator<CPIter, Sentinel, Func>;

        lazy_segment_range() noexcept {}
        lazy_segment_range(iterator first, iterator last) noexcept :
            first_(first),
            last_(last)
        {}

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
    };

}}

#endif
