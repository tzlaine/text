#ifndef BOOST_TEXT_ROPE_VIEW_HPP
#define BOOST_TEXT_ROPE_VIEW_HPP

#include <boost/text/detail/rope.hpp>


namespace boost { namespace text {

    struct rope;

    namespace detail {

        struct const_rope_iterator;
        struct const_reverse_rope_iterator;

        struct repeated_range
        {
            repeated_text_view::const_iterator first, last;
            repeated_text_view::const_iterator begin () const { return first; }
            repeated_text_view::const_iterator end () const { return last; }
        };

        inline std::ostream & operator<< (std::ostream & os, repeated_range rr)
        {
            for (char c : rr) {
                os << c;
            }
            return os;
        }

    }

    // TODO: Consider adding an implicit conversion ctor from text_view.
    struct rope_view
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        rope_view () noexcept : r_ (nullptr), lo_ (0), hi_ (0) {}

        rope_view (rope const & r) noexcept;
        rope_view (rope const & r, int lo, int hi);
        rope_view (rope const & r, int lo, int hi, utf8::unchecked_t) noexcept;

        rope_view (rope_view const & rhs) noexcept :
            r_ (rhs.r_), lo_ (rhs.lo_), hi_ (rhs.hi_)
        {}

        rope_view & operator= (rope_view const & rhs) noexcept
        {
            r_ = rhs.r_;
            lo_ = rhs.lo_;
            hi_ = rhs.hi_;
            return *this;
        }

        const_iterator begin () const noexcept;
        const_iterator end () const noexcept;

        const_reverse_iterator rbegin () const noexcept;
        const_reverse_iterator rend () const noexcept;

        bool empty () const noexcept
        { return lo_ == hi_; }

        size_type size () const noexcept
        { return hi_ - lo_; }

        char operator[] (int i) const noexcept;

        rope_view operator() (int lo, int hi) const
        {
            if (lo < 0)
                lo += size();
            if (hi < 0)
                hi += size();
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);
            return rope_view(r_, lo_ + lo, lo_ + hi);
        }

        rope_view operator() (int cut) const
        {
            int lo = 0;
            int hi = cut;
            if (cut < 0) {
                lo = cut + size();
                hi = size();
            }
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            return rope_view(r_, lo_ + lo, lo_ + hi);
        }

        constexpr size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        template <typename Fn>
        void foreach_segment (Fn && f) const;

        int compare (rope_view rhs) const noexcept;

        friend bool operator== (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) == 0; }

        friend bool operator!= (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) != 0; }

        friend bool operator< (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) < 0; }

        friend bool operator<= (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) <= 0; }

        friend bool operator> (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) > 0; }

        friend bool operator>= (rope_view lhs, rope_view rhs) noexcept
        { return lhs.compare(rhs) >= 0; }

        void swap (rope_view & rhs) noexcept
        {
            std::swap(r_, rhs.r_);
            std::swap(lo_, rhs.lo_);
            std::swap(hi_, rhs.hi_);
        }

    private:
        rope_view (rope const * r, int lo, int hi) :
            r_ (r), lo_ (lo), hi_ (hi)
        {}

        rope const * r_;
        int lo_;
        int hi_;

        friend struct rope;
    };

    inline rope operator+ (rope_view lhs, rope_view rhs);

} }

#include <boost/text/rope.hpp>

namespace boost { namespace text {

    inline rope_view::rope_view (rope const & r) noexcept :
        r_ (&r), lo_ (0), hi_ (r.size())
    {}

    inline rope_view::rope_view (rope const & r, int lo, int hi) :
        r_ (&r), lo_ (lo), hi_ (hi)
    {
        if (!utf8::starts_encoded(begin(), end()))
            throw std::invalid_argument("The start of the given string is not valid UTF-8.");
        if (!utf8::ends_encoded(begin(), end()))
            throw std::invalid_argument("The end of the given string is not valid UTF-8.");
    }

    inline rope_view::rope_view (rope const & r, int lo, int hi, utf8::unchecked_t) noexcept :
        r_ (&r), lo_ (lo), hi_ (hi)
    {}

    inline rope_view::const_iterator rope_view::begin () const noexcept
    { return const_iterator(r_, lo_); }
    inline rope_view::const_iterator rope_view::end () const noexcept
    { return const_iterator(r_, hi_); }

    inline rope_view::const_reverse_iterator rope_view::rbegin () const noexcept
    { return const_reverse_iterator(end() - 1); }
    inline rope_view::const_reverse_iterator rope_view::rend () const noexcept
    { return const_reverse_iterator(begin() - 1); }

    inline char rope_view::operator[] (int i) const noexcept
    {
        assert(lo_ + i < r_->size());
        return (*r_)[lo_ + i];
    }

    namespace detail {

        template <typename Fn>
        void apply_to_segment (
            detail::leaf_node_t const * leaf,
            std::ptrdiff_t lo,
            std::ptrdiff_t hi,
            Fn const & f
        ) {
            switch (leaf->which_) {
            case detail::node_t::which::t:
                f(leaf->as_text()(lo, hi));
                break;
            case detail::node_t::which::tv:
                f(leaf->as_text_view()(lo, hi));
                break;
            case detail::node_t::which::rtv:
                f(detail::repeated_range{
                    leaf->as_repeated_text_view().begin() + lo,
                    leaf->as_repeated_text_view().begin() + hi
                });
                break;
            case detail::node_t::which::ref:
                f(leaf->as_reference().ref_(lo, hi));
                break;
            default: assert(!"unhandled rope node case"); break;
            }
        }

    }

    template <typename Fn>
    void rope_view::foreach_segment (Fn && f) const
    {
        detail::found_leaf found_lo;
        detail::find_leaf(r_->ptr_, lo_, found_lo);

        detail::found_leaf found_hi;
        detail::find_leaf(r_->ptr_, hi_, found_hi);

        if (found_lo.leaf_->as_leaf() == found_hi.leaf_->as_leaf()) {
            detail::apply_to_segment(
                found_lo.leaf_->as_leaf(),
                found_lo.offset_,
                found_hi.offset_,
                f
            );
            return;
        }

        bool before_lo = true;
        detail::foreach_leaf(r_->ptr_, [&](detail::leaf_node_t const * leaf) {
            if (before_lo) {
                if (leaf == found_lo.leaf_->as_leaf()) {
                    auto const leaf_size = detail::size(leaf);
                    detail::apply_to_segment(leaf, found_lo.offset_, leaf_size, f);
                    before_lo = false;
                }
                return true; // continue
            }

            if (leaf == found_hi.leaf_->as_leaf()) {
                if (found_hi.offset_ != 0)
                    detail::apply_to_segment(leaf, 0, found_hi.offset_, f);
                return false; // break
            }

            auto const leaf_size = detail::size(leaf);
            detail::apply_to_segment(leaf, 0, leaf_size, f);

            return true;
        });
    }

    namespace detail {

        template <typename Iter>
        int mismatch_compare (rope_view rv, Iter rhs_first, Iter rhs_last)
        {
            if (rv.empty())
                return rhs_first == rhs_last ? 0 : -1;

            // TODO: This could probably be optimized quite a bit by using
            // rv.foreach_segment().
            auto const iters =
                algorithm::mismatch(rv.begin(), rv.end(), rhs_first, rhs_last);
            if (iters.first == rv.end()) {
                if (iters.second == rhs_last)
                    return 0;
                else
                    return -1;
            } else if (iters.second == rhs_last) {
                return 1;
            } else if (*iters.first == *iters.second) {
                return 0;
            } else if (*iters.first < *iters.second) {
                return -1;
            } else {
                return 1;
            }
        }

    }

    inline int rope_view::compare (rope_view rhs) const noexcept
    {
        if (empty())
            return rhs.empty() ? 0 : -1;

        // TODO: This could probably be optimized quite a bit by doing
        // something equivalent to mismatch, segment-wise.
        auto const iters =
            algorithm::mismatch(begin(), end(), rhs.begin(), rhs.end());
        if (iters.first == end()) {
            if (iters.second == rhs.end())
                return 0;
            else
                return -1;
        } else if (iters.second == rhs.end()) {
            return 1;
        } else if (*iters.first == *iters.second) {
            return 0;
        } else if (*iters.first < *iters.second) {
            return -1;
        } else {
            return 1;
        }
    }

    inline rope_view::iterator begin (rope_view rv) noexcept
    { return rv.begin(); }
    inline rope_view::iterator end (rope_view rv) noexcept
    { return rv.end(); }

    inline rope_view::reverse_iterator rbegin (rope_view rv) noexcept
    { return rv.rbegin(); }
    inline rope_view::reverse_iterator rend (rope_view rv) noexcept
    { return rv.rend(); }


    inline bool operator== (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) == 0; }

    inline bool operator!= (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) != 0; }

    inline bool operator< (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) < 0; }

    inline bool operator<= (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) <= 0; }

    inline bool operator> (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) > 0; }

    inline bool operator>= (rope_view lhs, text_view rhs) noexcept
    { return detail::mismatch_compare(lhs, rhs.begin(), rhs.end()) >= 0; }


    inline bool operator== (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) == 0; }

    inline bool operator!= (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) != 0; }

    inline bool operator< (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) > 0; }

    inline bool operator<= (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) >= 0; }

    inline bool operator> (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) < 0; }

    inline bool operator>= (text_view lhs, rope_view rhs) noexcept
    { return detail::mismatch_compare(rhs, lhs.begin(), lhs.end()) <= 0; }


    inline std::ostream & operator<< (std::ostream & os, rope_view rv)
    {
        if (os.good() && !rv.empty()) {
            detail::pad_width_before(os, rv.size());
            rv.foreach_segment(detail::segment_inserter{os});
            if (os.good())
                detail::pad_width_after(os, rv.size());
        }
        return os;
    }

    inline rope operator+ (rope_view lhs, rope_view rhs)
    {
        rope retval(lhs);
        return retval += rhs;
    }

} }

#endif
