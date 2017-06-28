#ifndef BOOST_TEXT_ROPE_VIEW_HPP
#define BOOST_TEXT_ROPE_VIEW_HPP

#include <boost/text/detail/rope.hpp>


namespace boost { namespace text {

    struct rope;

    namespace detail {

        struct const_rope_view_iterator;
        struct const_reverse_rope_view_iterator;

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

    // TODO: Add substringing operators to repeated_text_view that makes a
    // rope_view.

    /** A reference to a substring of a rope, text, or repeated_text_view.
        The substring is assumed to be UTF-8 encoded, though it is possible to
        construct a rope_view which is not. */
    struct rope_view
    {
        using iterator = detail::const_rope_view_iterator;
        using const_iterator = detail::const_rope_view_iterator;
        using reverse_iterator = detail::const_reverse_rope_view_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_view_iterator;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        rope_view () noexcept : ref_ (rope_ref()), which_ (which::r) {}

        /** Constructs a rope_view covering the entire given rope.  The UTF-8
            encoding is not checked. */
        rope_view (rope const & r) noexcept;

        /** Constructs a substring of r, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.  The UTF-8 encoding is checked only at the beginning and
            end of the sequence, to prevent slicing of code points.  To fully
            check the encoding, use checked_encoding().

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= r.size()
            \pre 0 <= hi && lhi <= r.size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8.
            \post size() == r.size() && begin() == r.begin() + lo && end() ==
            r.begin() + hi */
        rope_view (rope const & r, int lo, int hi);

        /** Constructs a substring of r, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.  The UTF-8 encoding is not checked.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= r.size()
            \pre 0 <= hi && lhi <= r.size()
            \pre lo <= hi
            \post size() == r.size() && begin() == r.begin() + lo && end() ==
            r.begin() + hi */
        rope_view (rope const & r, int lo, int hi, utf8::unchecked_t) noexcept;

        /** Constructs a rope_view covering the entire given text_view.  The
            UTF-8 encoding is not checked. */
        rope_view (text_view tv) noexcept : ref_ (tv), which_ (which::tv) {}

        /** Constructs a rope_view covering the entire given
            repeated_text_view.  The UTF-8 encoding is not checked. */
        rope_view (repeated_text_view rtv) noexcept :
            ref_ (repeated_ref(rtv, 0, rtv.size())),
            which_ (which::rtv)
        {}

        /** Constructs a substring of rtv, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.  The UTF-8 encoding is checked only at the beginning and
            end of the sequence, to prevent slicing of code points.  To fully
            check the encoding, use checked_encoding().

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= r.size()
            \pre 0 <= hi && lhi <= r.size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8.
            \post size() == r.size() && begin() == r.begin() + lo && end() ==
            r.begin() + hi */
        rope_view (repeated_text_view rtv, int lo, int hi);

        /** Constructs a substring of rtv, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.  The UTF-8 encoding is not checked.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= r.size()
            \pre 0 <= hi && lhi <= r.size()
            \pre lo <= hi
            \post size() == r.size() && begin() == r.begin() + lo && end() ==
            r.begin() + hi */
        rope_view (repeated_text_view rtv, int lo, int hi, utf8::unchecked_t) noexcept :
            ref_ (repeated_ref(rtv, lo, hi)),
            which_ (which::rtv)
        {}

        const_iterator begin () const noexcept;
        const_iterator end () const noexcept;

        const_reverse_iterator rbegin () const noexcept;
        const_reverse_iterator rend () const noexcept;

        bool empty () const noexcept;
        size_type size () const noexcept;

        /** Returns the i-th char of *this (not a reference).

            \pre i < size() */
        char operator[] (int i) const noexcept;

        /** Returns a substring of *this, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope_view operator() (int lo, int hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
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
            return operator()(lo, hi);
        }

        /** Returns the maximum size a text_view can have. */
        size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        /** Visits each segment s of the underlying rope and calls f(s).  Each
            segment is a value whose type models a Char_iterator
            iterator-range.  Depending of the operation performed on each
            segment, this may be more efficient than iterating over [begin(),
            end()).

            \pre Fn is an Invocable accepting a single argument whose begin
            and end model Char_iterator. */
        template <typename Fn>
        void foreach_segment (Fn && f) const;

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare (rope_view rhs) const noexcept;

        /** Swaps *this with rhs. */
        void swap (rope_view & rhs) noexcept
        {
            std::swap(ref_, rhs.ref_);
            std::swap(which_, rhs.which_);
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum class which { r, tv, rtv };

        struct rope_ref
        {
            rope_ref () : r_ (nullptr), lo_ (0), hi_ (0) {}
            rope_ref (rope const * r, int lo, int hi) :
                r_ (r), lo_ (lo), hi_ (hi)
            {}

            rope const * r_;
            int lo_;
            int hi_;
        };

        struct repeated_ref
        {
            repeated_ref () : rtv_ (), lo_ (0), hi_ (0) {}
            repeated_ref (repeated_text_view rtv, int lo, int hi) :
                rtv_ (rtv), lo_ (lo), hi_ (hi)
            {}

            repeated_text_view rtv_;
            int lo_;
            int hi_;
        };

        union ref
        {
            ref (rope_ref r) : r_ (r) {}
            ref (text_view tv) : tv_ (tv) {}
            ref (repeated_ref rtv) : rtv_ (rtv) {}

            ref (ref const & rhs)
            { memcpy(this, &rhs, sizeof(*this)); }

            ref & operator= (ref const & rhs)
            {
                memcpy(this, &rhs, sizeof(*this));
                return *this;
            }

            rope_ref r_;
            text_view tv_;
            repeated_ref rtv_;
        };

        rope_view (rope const * r, int lo, int hi) :
            ref_ (rope_ref(r, lo, hi)),
            which_ (which::r)
        {}

        ref ref_;
        which which_;

        friend struct rope;
#endif

    };

    inline bool operator== (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) == 0; }

    inline bool operator!= (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) != 0; }

    inline bool operator< (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) < 0; }

    inline bool operator<= (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) <= 0; }

    inline bool operator> (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) > 0; }

    inline bool operator>= (rope_view lhs, rope_view rhs) noexcept
    { return lhs.compare(rhs) >= 0; }

    /** Creates a new rope containing the concatenation of lhs and rhs. */
    inline rope operator+ (rope_view lhs, rope_view rhs);

} }

#include <boost/text/rope.hpp>

namespace boost { namespace text {

    inline rope_view::rope_view (rope const & r) noexcept :
        ref_ (rope_ref(&r, 0, r.size())),
        which_ (which::r)
    {}

    inline rope_view::rope_view (rope const & r, int lo, int hi) :
        ref_ (rope_ref(&r, lo, hi)),
        which_ (which::r)
    {
        if (!utf8::starts_encoded(begin(), end()))
            throw std::invalid_argument("The start of the given string is not valid UTF-8.");
        if (!utf8::ends_encoded(begin(), end()))
            throw std::invalid_argument("The end of the given string is not valid UTF-8.");
    }

    inline rope_view::rope_view (rope const & r, int lo, int hi, utf8::unchecked_t) noexcept :
        ref_ (rope_ref(&r, lo, hi)),
        which_ (which::r)
    {}

    inline rope_view::rope_view (repeated_text_view rtv, int lo, int hi) :
        ref_ (repeated_ref(rtv, lo, hi)),
        which_ (which::rtv)
    {
        if (!utf8::starts_encoded(begin(), end()))
            throw std::invalid_argument("The start of the given string is not valid UTF-8.");
        if (!utf8::ends_encoded(begin(), end()))
            throw std::invalid_argument("The end of the given string is not valid UTF-8.");
    }

    inline rope_view::const_iterator rope_view::begin () const noexcept
    {
        switch (which_) {
        case which::r: return const_iterator(detail::const_rope_iterator(ref_.r_.r_, ref_.r_.lo_));
        case which::tv: return const_iterator(ref_.tv_.begin());
        case which::rtv: return const_iterator(ref_.rtv_.rtv_.begin() + ref_.rtv_.lo_);
        }
        return const_iterator(); // This should never execute.
    }

    inline rope_view::const_iterator rope_view::end () const noexcept
    {
        switch (which_) {
        case which::r: return const_iterator(detail::const_rope_iterator(ref_.r_.r_, ref_.r_.hi_));
        case which::tv: return const_iterator(ref_.tv_.end());
        case which::rtv: return const_iterator(ref_.rtv_.rtv_.begin() + ref_.rtv_.hi_);
        }
        return const_iterator(); // This should never execute.
    }

    inline rope_view::const_reverse_iterator rope_view::rbegin () const noexcept
    { return const_reverse_iterator(end() - 1); }
    inline rope_view::const_reverse_iterator rope_view::rend () const noexcept
    { return const_reverse_iterator(begin() - 1); }

    inline bool rope_view::empty () const noexcept
    { return begin() == end(); }

    inline rope_view::size_type rope_view::size () const noexcept
    { return end() - begin(); }

    inline char rope_view::operator[] (int i) const noexcept
    {
        assert(1 < size());
        return begin()[i];
    }

    inline rope_view rope_view::operator() (int lo, int hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);
        switch (which_) {
        case which::r: return rope_view(ref_.r_.r_, ref_.r_.lo_ + lo, ref_.r_.lo_ + hi);
        case which::tv: return rope_view(ref_.tv_(lo, hi));
        case which::rtv: return rope_view(ref_.rtv_.rtv_, ref_.rtv_.lo_ + lo, ref_.rtv_.lo_ + hi);
        }
        return *this; // This should never execute.
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
        if (which_ == which::tv) {
            f(ref_.tv_);
            return;
        }

        if (which_ == which::rtv) {
            f(detail::repeated_range{
                ref_.rtv_.rtv_.begin() + ref_.rtv_.lo_,
                ref_.rtv_.rtv_.begin() + ref_.rtv_.hi_
            });
            return;
        }

        rope_ref r_ref = ref_.r_;

        detail::found_leaf found_lo;
        detail::find_leaf(r_ref.r_->ptr_, r_ref.lo_, found_lo);

        detail::found_leaf found_hi;
        detail::find_leaf(r_ref.r_->ptr_, r_ref.hi_, found_hi);

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
        detail::foreach_leaf(r_ref.r_->ptr_, [&](detail::leaf_node_t const * leaf) {
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
        if (which_ == which::tv && rhs.which_ == which::tv)
            return ref_.tv_.compare(rhs.ref_.tv_);

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


    /** Stream inserter; performs formatted output. */
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
