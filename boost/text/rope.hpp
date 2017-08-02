#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/detail/rope.hpp>

#ifdef BOOST_TEXT_TESTING
#include <iostream>
#endif


namespace boost { namespace text {

    struct rope_view;
    struct rope;

    namespace detail {
        struct const_rope_iterator;
        struct const_reverse_rope_iterator;
    }

    /** A mutable sequence of char with copy-on-write semantics.  The sequence
        is assumed to be UTF-8 encoded, though it is possible to construct a
        sequence which is not. A rope is non-contiguous and is not
        null-terminated. */
    struct rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        rope () noexcept : ptr_ (nullptr) {}

        rope (rope const & rhs) = default;
        rope (rope && rhs) noexcept = default;

        /** Constructs a rope from a rope_view. */
        explicit rope (rope_view rv);

        /** Move-constructs a rope from a text. */
        explicit rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a rope from a sequence of char.

            The sequence's UTF-8 encoding is not checked.  To check the
            encoding, use a converting iterator.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template <typename Iter>
        rope (Iter first, Iter last);

#else

        template <typename Iter>
        rope (
            Iter first, Iter last,
            detail::char_iter_ret_t<void *, Iter> = 0
        ) : ptr_ ()
        { insert(0, first, last); }

#endif

        rope & operator= (rope const & rhs) = default;
        rope & operator= (rope && rhs) noexcept = default;

        /** Assignment from a rope_view. */
        rope & operator= (rope_view rv);

        /** Move-assignment from a text. */
        rope & operator= (text && t)
        {
            rope temp(std::move(t));
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

        /** Returns the i-th char of *this (not a reference).

            \pre 0 <= i && i < size() */
        char operator[] (size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        /** Returns a substring of *this as a rope_view, taken from the range
            of chars at offsets [lo, hi).  If either of lo or hi is a negative
            value x, x is taken to be an offset from the end, and so x +
            size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope_view operator() (int lo, int hi) const;

        /** Returns a substring of *this as a rope_view, taken from the first
            cut chars when cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope_view operator() (int cut) const;

        /** Returns the maximum size a rope can have. */
        size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        /** Returns a substring of *this as a new rope, taken from the range
            of chars at offsets [lo, hi).  If either of lo or hi is a negative
            value x, x is taken to be an offset from the end, and so x +
            size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope substr (size_type lo, size_type hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope substr (size_type cut) const;

        /** Visits each segment s of *this and calls f(s).  Each segment is a
            text_view or repeated_text_view.  Depending of the operation
            performed on each segment, this may be more efficient than
            iterating over [begin(), end()).

            \pre Fn is an Invocable accepting a single argument of any of the
            types listed above. */
        template <typename Fn>
        void foreach_segment (Fn && f) const
        {
            detail::foreach_leaf(ptr_, [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
                switch (leaf->which_) {
                case detail::which::t:
                    f(text_view(leaf->as_text()));
                    break;
                case detail::which::rtv:
                    f(leaf->as_repeated_text_view());
                    break;
                case detail::which::ref:
                    f(leaf->as_reference().ref_);
                    break;
                default: assert(!"unhandled rope node case"); break;
                }
                return true;
            });
        }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
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

        /** Returns true if *this and rhs contain the same root node pointer.
            This is useful when you want to check for equality between two
            ropes that are likely to have originated from the same initial
            rope, and may have since been mutated. */
        bool equal_root (rope rhs) const noexcept
        { return ptr_ == rhs.ptr_; }

        void clear ()
        { ptr_ = detail::node_ptr<detail::rope_tag>(); }

        /** Inserts the sequence of char from rv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, rope_view rv);

        /** Inserts the sequence of char from t into *this starting at offset
            at, by moving the contents of t.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), would_not_allocate); }

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        template <typename Iter>
        rope & insert (size_type at, Iter first, Iter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if insertion at position at would
            break UTF-8 encoding, and the inserted sequence's UTF-8 encoding
            is not checked.  To check the inserted sequence's encoding, use a
            converting iterator. */
        template <typename Iter>
        rope & insert (const_iterator at, Iter first, Iter last);

#else

        template <typename Iter>
        auto insert (size_type at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto insert (const_iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

#endif

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        rope & erase (rope_view rv);

        /** Erases the portion of *this delimited by [first, last).

            No check is made to determine whether erasing [first, last) breaks
            UTF-8 encoding.

            \pre first <= last */
        rope & erase (const_iterator first, const_iterator last);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, text && t);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template <typename Iter>
        rope & replace (rope_view old_substr, Iter first, Iter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if removing [old_first, old_last)
            would break UTF-8 encoding, and the inserted sequence's UTF-8
            encoding is not checked.  To check the inserted sequence's
            encoding, use a converting iterator.

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template <typename Iter>
        rope & replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last);

#else

        template <typename Iter>
        auto replace (rope_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last)
            -> detail::char_iter_ret_t<rope &, Iter>;

#endif

        /** Swaps *this with rhs. */
        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        /** Appends rv to *this. */
        rope & operator+= (rope_view rv);

        /** Appends r to *this, by moving its contents into *this. */
        rope & operator+= (rope && r)
        {
            detail::interior_node_t<detail::rope_tag> * new_root = nullptr;
            detail::node_ptr<detail::rope_tag> new_root_ptr(new_root = detail::new_interior_node<detail::rope_tag>());
            new_root->keys_.push_back(size());
            new_root->keys_.push_back(size() + r.size());
            new_root->children_.push_back(std::move(ptr_));
            new_root->children_.push_back(std::move(r.ptr_));
            ptr_ = std::move(new_root_ptr);
            return *this;
        }

        /** Appends t to *this, by moving its contents into *this. */
        rope & operator+= (text && t);

        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<< (std::ostream & os, rope r)
        {
            r.foreach_segment(detail::segment_inserter{os});
            return os;
        }

#ifdef BOOST_TEXT_TESTING
        friend void dump_tree (rope const & r)
        {
            if (r.empty())
                std::cout << "[EMPTY]\n";
            else
                detail::dump_tree(r.ptr_);
        }
#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

#ifdef BOOST_TEXT_TESTING
    public:
#endif
        explicit rope (detail::node_ptr<detail::rope_tag> const & node) : ptr_ (node) {}
#ifdef BOOST_TEXT_TESTING
    private:
#endif

        bool self_reference (rope_view rv) const;

        struct text_insertion
        {
            explicit operator bool () const
            { return text_ != nullptr; }

            text * text_;
            detail::found_leaf<detail::rope_tag> found_;
        };

        text_insertion mutable_insertion_leaf (size_type at, size_type size, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return text_insertion{nullptr};

            detail::found_leaf<detail::rope_tag> found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return text_insertion{nullptr};
            }

            if (1 < found.leaf_->get()->refs_)
                return text_insertion{nullptr};

            if (found.leaf_->as_leaf()->which_ == detail::which::t) {
                text & t = const_cast<text &>(found.leaf_->as_leaf()->as_text());
                auto const inserted_size = t.size() + size;
                if (inserted_size <= t.capacity() ||
                    (allocation_note == would_allocate && inserted_size <= detail::text_insert_max)) {
                    return text_insertion{&t, found};
                }
            }

            return text_insertion{nullptr};
        }

        void check_encoding_from (size_type at);

        template <typename T>
        rope & insert_impl (
            size_type at,
            T && t,
            allocation_note_t allocation_note
        ) {
            assert(0 <= at && at <= size());

            if (t.empty())
                return *this;

            check_encoding_from(at);

            if (text_insertion insertion = mutable_insertion_leaf(at, t.size(), allocation_note)) {
                auto const t_size = t.size();
                for (auto node : insertion.found_.path_) {
                    auto from = detail::find_child(node, at);
                    detail::bump_keys(const_cast<detail::interior_node_t<detail::rope_tag> *>(node), from, t_size);
                }
                insertion.text_->insert(insertion.found_.offset_, t);
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at,
                    detail::make_node(std::forward<T &&>(t)),
                    detail::check_encoding_breakage
                );
            }

            return *this;
        }

        detail::node_ptr<detail::rope_tag> ptr_;

        friend struct detail::const_rope_iterator;
        friend struct rope_view;

#endif

    };


    /** Forwards r when it is entirely UTF-8 encoded; throws otherwise.

        \throw std::invalid_argument when r is not UTF-8 encoded. */
    inline rope const & checked_encoding (rope const & r);

    /** Forwards r when it is entirely UTF-8 encoded; throws otherwise.

        \throw std::invalid_argument when r is not UTF-8 encoded. */
    inline rope && checked_encoding (rope && r);

} }

#include <boost/text/detail/rope_iterator.hpp>
#include <boost/text/rope_view.hpp>

namespace boost { namespace text {

#ifndef BOOST_TEXT_DOXYGEN

    inline rope::rope (rope_view rv) : ptr_ (nullptr)
    { insert(0, rv); }

    inline rope & rope::operator= (rope_view rv)
    {
        detail::node_ptr<detail::rope_tag> extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        rope temp(rv);
        swap(temp);
        return *this;
    }

    inline int rope::compare (rope rhs) const noexcept
    { return rope_view(*this).compare(rhs); }

    inline rope & rope::insert (size_type at, rope_view rv)
    {
        assert(0 <= at && at <= size());

        if (rv.empty())
            return *this;

        detail::node_ptr<detail::rope_tag> extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        if (rv.which_ == rope_view::which::tv) {
            text_view tv = rv.ref_.tv_;
            bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
            if (tv_null_terminated)
                tv = tv(0, -1);
            return insert_impl(at, tv, would_allocate);
        }

        if (rv.which_ == rope_view::which::rtv) {
            if (rv.ref_.rtv_.lo_ == 0 && rv.ref_.rtv_.hi_ == rv.ref_.rtv_.rtv_.size()) {
                repeated_text_view rtv = rv.ref_.rtv_.rtv_;
                bool const rtv_null_terminated =
                    !rtv.view().empty() && rtv.view().end()[-1] == '\0';
                if (rtv_null_terminated)
                    rtv = repeat(rtv.view()(0, -1), rtv.count());
                return insert_impl(at, rtv, would_not_allocate);
            }
            return insert(at, text(rv.begin(), rv.end()));
        }

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        check_encoding_from(at);

        rope_view::rope_ref rope_ref = rv.ref_.r_;

        detail::found_leaf<detail::rope_tag> found_lo;
        find_leaf(rope_ref.r_->ptr_, rope_ref.lo_, found_lo);
        detail::leaf_node_t<detail::rope_tag> const * const leaf_lo = found_lo.leaf_->as_leaf();

        // If the entire rope_view lies within a single segment, slice off
        // the appropriate part of that segment.
        if (found_lo.offset_ + rv.size() <= detail::size(leaf_lo)) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    found_lo.offset_ + rv.size(),
                    true,
                    detail::encoding_breakage_ok
                ),
                detail::check_encoding_breakage
            );
            return *this;
        }

        detail::found_leaf<detail::rope_tag> found_hi;
        find_leaf(rope_ref.r_->ptr_, rope_ref.hi_, found_hi);

        bool before_lo = true;
        detail::foreach_leaf(rope_ref.r_->ptr_, [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
            if (leaf == found_lo.leaf_->as_leaf()) {
                detail::node_ptr<detail::rope_tag> node;
                if (found_lo.offset_ != 0) {
                    node = slice_leaf(
                        *found_lo.leaf_,
                        found_lo.offset_,
                        detail::size(leaf),
                        true,
                        detail::encoding_breakage_ok
                    );
                } else {
                    node = detail::node_ptr<detail::rope_tag>(leaf);
                }
                auto const node_size = detail::size(node.get());
                ptr_ = detail::btree_insert(ptr_, at, std::move(node), detail::check_encoding_breakage);
                at += node_size;
                before_lo = false;
                return true; // continue
            }

            if (before_lo)
                return true; // continue

            if (leaf == found_hi.leaf_->as_leaf()) {
                if (found_hi.offset_ != 0) {
                    ptr_ = detail::btree_insert(
                        ptr_,
                        at,
                        slice_leaf(
                            *found_hi.leaf_,
                            0,
                            found_hi.offset_,
                            true,
                            detail::encoding_breakage_ok
                        ),
                        detail::check_encoding_breakage
                    );

                    at += found_hi.offset_;
                }

                return false; // break
            }

            ptr_ = detail::btree_insert(
                ptr_,
                at,
                detail::node_ptr<detail::rope_tag>(leaf),
                detail::check_encoding_breakage
            );
            at += detail::size(leaf);

            return true;
        });

        return *this;
    }

    template <typename Iter>
    auto rope::insert (size_type at, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(0 <= at && at <= size());

        if (first == last)
            return *this;

        check_encoding_from(at);

        ptr_ = detail::btree_insert(
            ptr_,
            at,
            detail::make_node(text(first, last)),
            detail::check_encoding_breakage
        );

        return *this;
    }

    template <typename Iter>
    auto rope::insert (const_iterator at, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(begin() <= at && at <= end());

        if (first == last)
            return *this;

        ptr_ = detail::btree_insert(
            ptr_,
            at - begin(),
            detail::make_node(text(first, last)),
            detail::encoding_breakage_ok
        );

        return *this;
    }

    inline rope & rope::erase (rope_view rv)
    {
        assert(self_reference(rv));

        rope_view::rope_ref rope_ref = rv.ref_.r_;

        assert(0 <= rope_ref.lo_ && rope_ref.lo_ <= size());
        assert(0 <= rope_ref.hi_ && rope_ref.hi_ <= size());
        assert(rope_ref.lo_ <= rope_ref.hi_);

        if (rope_ref.lo_ == rope_ref.hi_)
            return *this;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        ptr_ = btree_erase(ptr_, rope_ref.lo_, rope_ref.hi_, detail::check_encoding_breakage);

        return *this;
    }

    inline rope & rope::erase (const_iterator first, const_iterator last)
    {
        assert(first <= last);
        assert(begin() <= first && last <= end());

        if (first == last)
            return *this;

        auto const lo = first - begin();
        auto const hi = last - begin();
        ptr_ = btree_erase(ptr_, lo, hi, detail::encoding_breakage_ok);

        return *this;
    }

    inline rope & rope::replace (rope_view old_substr, rope_view rv)
    {
        assert(self_reference(old_substr));

        detail::node_ptr<detail::rope_tag> extra_ref;
        rope extra_rope;
        if (self_reference(rv)) {
            extra_ref = ptr_;
            extra_rope = rope(extra_ref);
            rope_view::rope_ref rope_ref = rv.ref_.r_;
            rv = rope_view(extra_rope, rope_ref.lo_, rope_ref.hi_);
        }

        return erase(old_substr).insert(old_substr.ref_.r_.lo_, rv);
    }

    inline rope & rope::replace (rope_view old_substr, text && t)
    { return erase(old_substr).insert(old_substr.ref_.r_.lo_, std::move(t)); }

    template <typename Iter>
    auto rope::replace (rope_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(self_reference(old_substr));
        assert(0 <= old_substr.size());
        const_iterator const old_first = old_substr.begin().as_rope_iter();
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    template <typename Iter>
    auto rope::replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(old_first <= old_last);
        assert(begin() <= old_first && old_last <= end());
        return erase(old_first, old_last).insert(old_first, new_first, new_last);
    }

    inline rope & rope::operator+= (rope_view rv)
    { return insert(size(), rv); }

    inline rope & rope::operator+= (text && t)
    { return insert(size(), std::move(t)); }

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

    inline rope rope::substr (size_type lo, size_type hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);

        if (lo == hi)
            return rope();

        auto const check_ends = (*this)(lo, hi);
        (void)check_ends;

        // If the entire substring falls within a single segment, slice
        // off the appropriate part of that segment.
        detail::found_leaf<detail::rope_tag> found;
        detail::find_leaf(ptr_, lo, found);
        if (found.offset_ + hi - lo <= detail::size(found.leaf_->get())) {
            return rope(
                slice_leaf(
                    *found.leaf_,
                    found.offset_,
                    found.offset_ + hi - lo,
                    true,
                    detail::check_encoding_breakage
                )
            );
        }

        // Take an extra ref to the root, which will force all a clone of
        // all the interior nodes.
        detail::node_ptr<detail::rope_tag> new_root = ptr_;

        if (hi != size())
            new_root = detail::btree_erase(new_root, hi, size(), detail::check_encoding_breakage);
        if (lo != 0)
            new_root = detail::btree_erase(new_root, 0, lo, detail::check_encoding_breakage);

        return rope(new_root);
    }

    inline rope rope::substr (size_type cut) const
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

    inline rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    inline rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    inline rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    inline rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

    inline bool rope::self_reference (rope_view rv) const
    { return rv.which_ == rope_view::which::r && rv.ref_.r_.r_ == this; }

    inline void rope::check_encoding_from (size_type at)
    {
        if (!utf8::starts_encoded(begin() + at, end()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");
    }

#endif

    /** Creates a new rope object that is the concatenation of r and r2. */
    inline rope operator+ (rope r, rope r2)
    { return r.insert(r.size(), std::move(r2)); }

    /** Creates a new rope object that is the concatenation of r and rv. */
    inline rope operator+ (rope r, rope_view rv)
    { return r.insert(r.size(), rv); }

    /** Creates a new rope object that is the concatenation of rv and r. */
    inline rope operator+ (rope_view rv, rope r)
    { return r.insert(0, rv); }

    /** Creates a new rope object that is the concatenation of r and t, by
        moving the contents of t into the result. */
    inline rope operator+ (rope r, text && t)
    { return r.insert(r.size(), std::move(t)); }

    /** Creates a new rope object that is the concatenation of t and r, by
        moving the contents of t into the result. */
    inline rope operator+ (text && t, rope r)
    { return r.insert(0, std::move(t)); }


    inline rope const & checked_encoding (rope const & r)
    {
        r.foreach_segment(detail::segment_encoding_checker{});
        return r;
    }

    inline rope && checked_encoding (rope && r)
    {
        r.foreach_segment(detail::segment_encoding_checker{});
        return std::move(r);
    }


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

    inline rope_view::rope_view (text const & r) noexcept :
        ref_ (text_view(r.begin(), r.size(), utf8::unchecked)),
        which_ (which::tv)
    {}

    inline rope_view::rope_view (text const & r, int lo, int hi) :
        ref_ (r(lo, hi)),
        which_ (which::tv)
    {}

    inline rope_view::rope_view (text const & r, int lo, int hi, utf8::unchecked_t) noexcept :
        ref_ (text_view(r.begin() + lo, hi - lo, utf8::unchecked)),
        which_ (which::tv)
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
            detail::leaf_node_t<detail::rope_tag> const * leaf,
            std::ptrdiff_t lo,
            std::ptrdiff_t hi,
            Fn const & f
        ) {
            switch (leaf->which_) {
            case detail::which::t:
                f(leaf->as_text()(lo, hi));
                break;
            case detail::which::rtv:
                f(detail::repeated_range{
                    leaf->as_repeated_text_view().begin() + lo,
                    leaf->as_repeated_text_view().begin() + hi
                });
                break;
            case detail::which::ref:
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

        if (!r_ref.r_)
            return;

        detail::found_leaf<detail::rope_tag> found_lo;
        detail::find_leaf(r_ref.r_->ptr_, r_ref.lo_, found_lo);

        detail::found_leaf<detail::rope_tag> found_hi;
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
        detail::foreach_leaf(r_ref.r_->ptr_, [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
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


    /** Stream inserter; performs unformatted output. */
    inline std::ostream & operator<< (std::ostream & os, rope_view rv)
    {
        rv.foreach_segment(detail::segment_inserter{os});
        return os;
    }

    inline rope operator+ (rope_view lhs, rope_view rhs)
    {
        rope retval(lhs);
        return retval += rhs;
    }

    inline text & text::operator+= (rope r)
    { return insert(size(), r.begin(), r.end()); }

    inline text & text::operator+= (rope_view rv)
    { return insert(size(), rv.begin(), rv.end()); }

    namespace detail {

#ifdef BOOST_TEXT_TESTING
        template <typename T>
        inline void dump_tree (node_ptr<T> const & root, int key, int indent)
        {
            std::cout << repeated_text_view("    ", indent)
                      << (root->leaf_ ? "LEAF" : "INTR")
                      << " @0x" << std::hex << root.get();
            if (key != -1)
                std::cout << " < " << std::dec << key;
            std::cout << " (" << root->refs_ << " refs)\n";
            if (!root->leaf_) {
                int i = 0;
                for (auto const & child : children(root)) {
                    dump_tree(child, keys(root)[i++], indent + 1);
                }
            }
        }
#endif

    }

} }

#endif
