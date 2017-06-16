#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/detail/rope.hpp>


namespace boost { namespace text {

    // TODO: Remove null terminators on insert, erase.
    // TODO: Header inclusion tests for rope.hpp and rope_view.hpp.

    struct rope_view;

    namespace detail {
        struct const_rope_iterator;
        struct const_reverse_rope_iterator;
    }

    struct rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        rope () noexcept : ptr_ (nullptr) {}

        rope (rope const & rhs) = default;
        rope (rope && rhs) noexcept = default;

        explicit rope (rope_view rv);
        explicit rope (text const & t) : ptr_ (detail::make_node(t)) {}
        explicit rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}
        explicit rope (text_view tv) : ptr_ (detail::make_node(tv)) {}
        explicit rope (repeated_text_view rtv) : ptr_ (detail::make_node(rtv)) {}

        rope & operator= (rope const & rhs) = default;
        rope & operator= (rope && rhs) noexcept = default;

        rope & operator= (text const & t)
        {
            rope temp(t);
            swap(temp);
            return *this;
        }

        rope & operator= (rope_view rv);

        rope & operator= (text && t)
        {
            rope temp(std::move(t));
            swap(temp);
            return *this;
        }

        rope & operator= (text_view tv)
        {
            rope temp(tv);
            swap(temp);
            return *this;
        }

        rope & operator= (repeated_text_view rtv)
        {
            rope temp(rtv);
            swap(temp);
            return *this;
        }

        const_iterator begin () const noexcept;
        const_iterator end () const noexcept;

        const_reverse_iterator rbegin () const noexcept;
        const_reverse_iterator rend () const noexcept;

        bool empty () const noexcept
        { return size() == 0; }

        size_type size () const noexcept
        { return detail::size(ptr_.get()); }

        char operator[] (size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        rope_view operator() (int lo, int hi) const;
        rope_view operator() (int cut) const;

        constexpr size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        rope substr (size_type lo, size_type hi) const
        {
            assert(ptr_);
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);

            if (lo == hi)
                return rope(detail::make_node(""));

            // If the entire substring falls within a single segment, slice
            // off the appropriate part of that segment.
            detail::found_leaf found;
            detail::find_leaf(ptr_, lo, found);
            if (found.offset_ + hi - lo <= detail::size(found.leaf_->get()))
                return rope(slice_leaf(*found.leaf_, found.offset_, found.offset_ + hi - lo, true));

            // Take an extra ref to the root, which will force all a clone of
            // all the interior nodes.
            detail::node_ptr new_root = ptr_;

            new_root = detail::btree_erase(new_root, hi, size());
            new_root = detail::btree_erase(new_root, 0, lo);

            return rope(new_root);
        }

        rope substr (size_type cut) const
        {
            int lo = 0;
            int hi = cut;
            if (cut < 0) {
                lo = cut + size();
                hi = size();
            }
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            return substr(lo, hi);
        }

        template <typename Fn>
        void foreach_segment (Fn && f) const
        {
            if (!ptr_)
                return;

            detail::found_leaf found;
            detail::find_leaf(ptr_, 0, found);
            detail::leaf_node_t const * leaf = found.leaf_->as_leaf();
            while (leaf) {
                switch (leaf->which_) {
                case detail::node_t::which::t:
                    f(leaf->as_text());
                    break;
                case detail::node_t::which::tv:
                    f(leaf->as_text_view());
                    break;
                case detail::node_t::which::rtv:
                    f(leaf->as_repeated_text_view());
                    break;
                case detail::node_t::which::ref:
                    f(leaf->as_reference().ref_);
                    break;
                default: assert(!"unhandled rope node case"); break;
                }
                leaf = leaf->next_;
            }
        }

        int compare (rope rhs) const noexcept;

        bool operator== (rope rhs) const noexcept
        { return compare(rhs) == 0; }

        bool operator!= (rope rhs) const noexcept
        { return compare(rhs) != 0; }

        bool operator< (rope rhs) const noexcept
        { return compare(rhs) < 0; }

        bool operator<= (rope rhs) const noexcept
        { return compare(rhs) <= 0; }

        bool operator> (rope rhs) const noexcept
        { return compare(rhs) > 0; }

        bool operator>= (rope rhs) const noexcept
        { return compare(rhs) >= 0; }

        void clear ()
        { ptr_ = detail::node_ptr(); }

        // TODO: Document that each insert(), erase(), and replace() overload
        // has only the basic guarantee.
        rope & insert (size_type at, rope const & r)
        {
            assert(0 <= at && at <= size());

            if (r.empty())
                return *this;

            detail::found_leaf found;
            find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t const * leaf = found.leaf_->as_leaf();

            while (leaf) {
                ptr_ = detail::btree_insert(ptr_, at, detail::node_ptr(leaf));
                at += detail::size(leaf);
                leaf = leaf->next_;
            }

            return *this;
        }

        rope & insert (size_type at, rope_view rv);

        rope & insert (size_type at, text const & t)
        { return insert_impl(at, t, true); }

        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), false); }

        rope & insert (size_type at, text_view tv)
        { return insert_impl(at, tv, false); }

        rope & insert (size_type at, repeated_text_view rtv)
        { return insert_impl(at, rtv, false); }

        rope & erase (rope_view rv);

        rope & replace (rope_view rv, text const & t);
        rope & replace (rope_view rv, text && t);
        rope & replace (rope_view rv, text_view tv);
        rope & replace (rope_view rv, repeated_text_view rtv);

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        friend const_iterator begin (rope const & r) noexcept;
        friend const_iterator end (rope const & r) noexcept;

        friend const_reverse_iterator rbegin (rope const & r) noexcept;
        friend const_reverse_iterator rend (rope const & r) noexcept;

        friend std::ostream & operator<< (std::ostream & os, rope const & r)
        {
            r.foreach_segment([&os](auto const & segment) { os << segment; });
            return os;
        }

    private:
        explicit rope (detail::node_ptr const & node) : ptr_ (node) {}

        text * mutable_insertion_leaf (size_type at, size_type size, bool insertion_would_allocate)
        {
            detail::found_leaf found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return nullptr;
            }

            if (found.leaf_->as_leaf()->which_ == detail::node_t::which::t) {
                text & t = const_cast<text &>(found.leaf_->as_leaf()->as_text());
                auto const inserted_size = t.size() + size;
                if (inserted_size <= t.capacity())
                    return &t;
                else if (insertion_would_allocate && inserted_size <= detail::text_insert_max)
                    return &t;
            }

            return nullptr;
        }

        template <typename T>
        rope & insert_impl (size_type at, T && t, bool insertion_would_allocate)
        {
            if (t.empty())
                return *this;

            if (text * leaf_t = mutable_insertion_leaf(at, t.size(), insertion_would_allocate))
                leaf_t->insert(leaf_t->size(), t);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(std::forward<T &&>(t)));

            return *this;
        }

        detail::node_ptr ptr_;

        friend struct detail::const_rope_iterator;
    };

    // TODO: Consider adding an implicit ctor from text_view.
    struct rope_view
    {
        using iterator = rope::iterator;
        using const_iterator = rope::const_iterator;
        using reverse_iterator = rope::reverse_iterator;
        using const_reverse_iterator = rope::const_reverse_iterator;

        using size_type = std::ptrdiff_t;

        rope_view () noexcept : r_ (nullptr), lo_ (0), hi_ (0) {}
        rope_view (rope const & r) noexcept : r_ (&r), lo_ (0), hi_ (r.size()) {}

        rope_view (rope const & r, int lo, int hi);

        rope_view (rope const & r, int lo, int hi, utf8::unchecked_t) noexcept :
            r_ (&r), lo_ (lo), hi_ (hi)
        {}

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

        char operator[] (int i) const noexcept
        {
            assert(lo_ + i < r_->size());
            return (*r_)[lo_ + i];
        }

        rope_view operator() (int lo, int hi) const
        {
            if (lo < 0)
                lo += size();
            if (hi < 0)
                hi += size();
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);
            return rope_view(*r_, lo_ + lo, lo_ + hi);
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
            return rope_view(*r_, lo_ + lo, lo_ + hi);
        }

        constexpr size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        // TODO: foreach_segment() ?

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

#if 0 // TODO
        friend std::ostream & operator<< (std::ostream & os, rope_view view)
        { return os.write(view.begin(), view.size()); }
#endif

    private:
        rope const * r_;
        int lo_;
        int hi_;

        friend struct rope;
    };

} }

#include <boost/text/detail/rope_iterator.hpp>

namespace boost { namespace text {

    // rope

    rope::rope (rope_view rv) : ptr_ (nullptr)
    { insert(0, rv); }

    rope & rope::operator= (rope_view rv)
    {
        rope temp(rv);
        swap(temp);
        return *this;
    }

    int rope::compare (rope rhs) const noexcept
    { return rope_view(*this).compare(rhs); }

    rope & rope::insert (size_type at, rope_view rv)
    {
        if (rv.empty())
            return *this;

        detail::found_leaf found_lo;
        find_leaf(rv.r_->ptr_, rv.lo_, found_lo);
        detail::leaf_node_t const * leaf = found_lo.leaf_->as_leaf();

        // If the entire rope_view lies within a single segment, slice off
        // the appropriate part of that segment.
        if (found_lo.offset_ + rv.size() <= detail::size(leaf)) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(*found_lo.leaf_, found_lo.offset_, found_lo.offset_ + rv.size(), true)
            );
            return *this;
        }

        {
            detail::node_ptr node;
            if (found_lo.offset_ != 0)
                node = slice_leaf(*found_lo.leaf_, found_lo.offset_, detail::size(leaf), true);
            else
                node = detail::node_ptr(leaf);
            ptr_ = detail::btree_insert(ptr_, at, node);
        }
        at += detail::size(leaf);
        leaf = leaf->next_;

        detail::found_leaf found_hi;
        find_leaf(rv.r_->ptr_, rv.hi_, found_hi);
        detail::leaf_node_t const * const leaf_hi = found_hi.leaf_->as_leaf();

        while (leaf != leaf_hi) {
            ptr_ = detail::btree_insert(ptr_, at, detail::node_ptr(leaf));
            at += detail::size(leaf);
            leaf = leaf->next_;
        }

        if (found_hi.offset_ != 0) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(*found_hi.leaf_, 0, found_hi.offset_, true)
            );
        }

        return *this;
    }

    rope & rope::erase (rope_view rv)
    {
        assert(0 <= rv.lo_ && rv.lo_ <= size());
        assert(0 <= rv.hi_ && rv.hi_ <= size());
        assert(rv.lo_ <= rv.hi_);

        if (rv.lo_ == rv.hi_)
            return *this;

        ptr_ = btree_erase(ptr_, rv.lo_, rv.hi_);

        return *this;
    }

    rope & rope::replace (rope_view rv, text const & t)
    { return erase(rv).insert(rv.lo_, t); }

    rope & rope::replace (rope_view rv, text && t)
    { return erase(rv).insert(rv.lo_, std::move(t)); }

    rope & rope::replace (rope_view rv, text_view tv)
    { return erase(rv).insert(rv.lo_, tv); }

    rope & rope::replace (rope_view rv, repeated_text_view rtv)
    { return erase(rv).insert(rv.lo_, rtv); }

    rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    rope_view rope::operator() (int lo, int hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);
        return rope_view(*this, lo, hi);
    }

    rope_view rope::operator() (int cut) const
    {
        int lo = 0;
        int hi = cut;
        if (cut < 0) {
            lo = cut + size();
            hi = size();
        }
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        return rope_view(*this, lo, hi);
    }

    rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }


    // rope_view

    inline rope_view::rope_view (rope const & r, int lo, int hi) :
        r_ (&r), lo_ (lo), hi_ (hi)
    {
        if (!utf8::starts_encoded(begin(), end()))
            throw std::invalid_argument("The start of the given string is not valid UTF-8.");
        if (!utf8::ends_encoded(begin(), end()))
            throw std::invalid_argument("The end of the given string is not valid UTF-8.");
    }

    inline rope_view::const_iterator rope_view::begin () const noexcept
    { return r_->begin() + lo_; }
    inline rope_view::const_iterator rope_view::end () const noexcept
    { return r_->begin() + hi_; }

    inline rope_view::const_reverse_iterator rope_view::rbegin () const noexcept
    { return const_reverse_iterator(r_->begin() + hi_ - 1); }
    inline rope_view::const_reverse_iterator rope_view::rend () const noexcept
    { return const_reverse_iterator(r_->begin() + lo_ - 1); }

    inline int rope_view::compare (rope_view rhs) const noexcept
    {
        // TODO: This could probably be optimized quite a bit by doing
        // something equivalent to mismatch, segment-wise.
        auto const iters = std::mismatch(begin(), end(), rhs.begin(), rhs.end());
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

    inline rope_view::iterator begin (rope_view v) noexcept
    { return v.begin(); }
    inline rope_view::iterator end (rope_view v) noexcept
    { return v.end(); }

    inline rope_view::reverse_iterator rbegin (rope_view v) noexcept
    { return v.rbegin(); }
    inline rope_view::reverse_iterator rend (rope_view v) noexcept
    { return v.rend(); }

} }

#endif
