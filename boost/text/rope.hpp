#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/detail/rope.hpp>


namespace boost { namespace text {

    // TODO: Encoding breakage checks.
    // TODO: Remove null terminators on insert, erase.

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

        rope () : ptr_ (nullptr) {}

        rope (text const & t) : ptr_ (detail::make_node(t)) {}
        rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}
        rope (text_view tv) : ptr_ (detail::make_node(tv)) {}
        rope (repeated_text_view rtv) : ptr_ (detail::make_node(rtv)) {}

        rope & operator= (text const & t)
        {
            rope temp(t);
            swap(temp);
            return *this;
        }

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
            if (found.offset_ + hi - lo <= detail::size(found.leaf_->get())) {
                detail::node_ptr extra_ref(*found.leaf_);
                return rope(slice_leaf(*found.leaf_, found.offset_, found.offset_ + hi - lo, true));
            }

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

        friend const_iterator begin (rope const & r) noexcept;
        friend const_iterator end (rope const & r) noexcept;

        friend const_reverse_iterator rbegin (rope const & r) noexcept;
        friend const_reverse_iterator rend (rope const & r) noexcept;

        friend std::ostream & operator<< (std::ostream & os, rope const & r)
        {
            r.foreach_segment([&os](auto const & segment) { os << segment; });
            return os;
        }

        void clear ()
        { ptr_ = detail::node_ptr(); }

        rope & insert (size_type at, rope const & r)
        {
            if (r.empty())
                return *this;

            detail::found_leaf found;
            find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t const * leaf = found.leaf_->as_leaf();

            if (!ptr_) {
                ptr_ = detail::node_ptr(leaf);
                leaf = leaf->next_;
            }

            while (leaf) {
                ptr_ = detail::btree_insert(ptr_, at, detail::node_ptr(leaf));
                leaf = leaf->next_;
            }

            return *this;
        }

        rope & insert (size_type at, text const & t)
        { return insert_impl(at, t, true); }

        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), false); }

        rope & insert (size_type at, text_view tv)
        { return insert_impl(at, tv, false); }

        rope & insert (size_type at, repeated_text_view rtv)
        { return insert_impl(at, rtv, false); }

        rope & erase (size_type lo, size_type hi)
        {
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);

            if (lo == hi)
                return *this;

            ptr_ = btree_erase(ptr_, lo, hi);

            return *this;
        }

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

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

            if (!ptr_)
                ptr_ = detail::make_node(std::forward<T &&>(t));
            else if (text * leaf_t = mutable_insertion_leaf(at, t.size(), insertion_would_allocate))
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

namespace boost { namespace text {

    int rope::compare (rope rhs) const noexcept
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

    rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

} }

#endif
