#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/detail/rope.hpp>


namespace boost { namespace text {

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

        explicit rope (text_view tv) : ptr_ (nullptr)
        { insert(0, tv); }

        explicit rope (repeated_text_view rtv) : ptr_ (nullptr)
        { insert(0, rtv); }

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
        rope & insert (size_type at, rope r)
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
        {
            bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
            if (tv_null_terminated)
                tv = tv(0, -1);
            return insert_impl(at, tv, false);
        }

        rope & insert (size_type at, repeated_text_view rtv)
        {
            bool const rtv_null_terminated =
                !rtv.view().empty() && rtv.view().end()[-1] == '\0';
            if (rtv_null_terminated)
                rtv = repeat(rtv.view()(0, -1), rtv.count());
            return insert_impl(at, rtv, false);
        }

        rope & erase (rope_view rv);

        rope & replace (rope_view rv, text const & t);
        rope & replace (rope_view rv, text && t);
        rope & replace (rope_view rv, text_view tv);
        rope & replace (rope_view rv, repeated_text_view rtv);

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        friend const_iterator begin (rope r) noexcept;
        friend const_iterator end (rope r) noexcept;

        friend const_reverse_iterator rbegin (rope r) noexcept;
        friend const_reverse_iterator rend (rope r) noexcept;

        friend std::ostream & operator<< (std::ostream & os, rope r)
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

} }

#include <boost/text/detail/rope_iterator.hpp>
#include <boost/text/rope_view.hpp>

namespace boost { namespace text {

    inline rope::rope (rope_view rv) : ptr_ (nullptr)
    { insert(0, rv); }

    inline rope & rope::operator= (rope_view rv)
    {
        rope temp(rv);
        swap(temp);
        return *this;
    }

    inline int rope::compare (rope rhs) const noexcept
    { return rope_view(*this).compare(rhs); }

    inline rope & rope::insert (size_type at, rope_view rv)
    {
        if (rv.empty())
            return *this;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

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

    inline rope & rope::erase (rope_view rv)
    {
        assert(0 <= rv.lo_ && rv.lo_ <= size());
        assert(0 <= rv.hi_ && rv.hi_ <= size());
        assert(rv.lo_ <= rv.hi_);

        if (rv.lo_ == rv.hi_)
            return *this;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        ptr_ = btree_erase(ptr_, rv.lo_, rv.hi_);

        return *this;
    }

    inline rope & rope::replace (rope_view rv, text const & t)
    { return erase(rv).insert(rv.lo_, t); }

    inline rope & rope::replace (rope_view rv, text && t)
    { return erase(rv).insert(rv.lo_, std::move(t)); }

    inline rope & rope::replace (rope_view rv, text_view tv)
    { return erase(rv).insert(rv.lo_, tv); }

    inline rope & rope::replace (rope_view rv, repeated_text_view rtv)
    { return erase(rv).insert(rv.lo_, rtv); }

    inline rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    inline rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    inline rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    inline rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    inline rope_view rope::operator() (int lo, int hi) const
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

    inline rope_view rope::operator() (int cut) const
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

    inline rope::const_iterator begin (rope r) noexcept
    { return r.begin(); }
    inline rope::const_iterator end (rope r) noexcept
    { return r.end(); }

    inline rope::const_reverse_iterator rbegin (rope r) noexcept
    { return r.rbegin(); }
    inline rope::const_reverse_iterator rend (rope r) noexcept
    { return r.rend(); }

} }

#endif
