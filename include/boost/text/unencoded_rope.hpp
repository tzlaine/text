#ifndef BOOST_TEXT_UNENCODED_ROPE_HPP
#define BOOST_TEXT_UNENCODED_ROPE_HPP

#include <boost/text/detail/rope.hpp>

#ifdef BOOST_TEXT_TESTING
#    include <iostream>
#endif


namespace boost { namespace text {

    struct unencoded_rope_view;
    struct unencoded_rope;

    namespace detail {
        struct const_rope_iterator;
        struct const_reverse_rope_iterator;
    }

    /** A mutable sequence of char with copy-on-write semantics.  An
        unencoded_rope is non-contiguous and is not null-terminated. */
    struct unencoded_rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;
        using value_type = char;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        unencoded_rope() noexcept : ptr_(nullptr) {}

        unencoded_rope(unencoded_rope const & rhs) = default;
        unencoded_rope(unencoded_rope && rhs) noexcept = default;

        /** Constructs an unencoded_rope from a null-terminated string. */
        explicit unencoded_rope(char const * c_str);

        /** Constructs an unencoded_rope from an unencoded_rope_view. */
        explicit unencoded_rope(unencoded_rope_view rv);

        /** Move-constructs an unencoded_rope from a string. */
        explicit unencoded_rope(string && t) :
            ptr_(detail::make_node(std::move(t)))
        {}

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs an unencoded_rope from a sequence of char.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template<typename Iter>
        unencoded_rope(Iter first, Iter last);

#else

        template<typename Iter>
        unencoded_rope(
            Iter first, Iter last, detail::char_iter_ret_t<void *, Iter> = 0) :
            ptr_()
        {
            insert(0, first, last);
        }

#endif

        unencoded_rope & operator=(unencoded_rope const & rhs) = default;
        unencoded_rope & operator=(unencoded_rope && rhs) noexcept = default;

        /** Assignment from an unencoded_rope_view. */
        unencoded_rope & operator=(unencoded_rope_view rv);

        /** Move-assignment from a string. */
        unencoded_rope & operator=(string && t)
        {
            unencoded_rope temp(std::move(t));
            swap(temp);
            return *this;
        }

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator rend() const noexcept;

        bool empty() const noexcept { return size() == 0; }

        size_type size() const noexcept { return detail::size(ptr_.get()); }

        /** Returns the i-th char of *this (not a reference).

            \pre 0 <= i && i < size() */
        char operator[](size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        /** Returns a substring of *this as an unencoded_rope_view, taken from
            the range of chars at offsets [lo, hi).  If either of lo or hi is a
            negative value x, x is taken to be an offset from the end, and so x
            + size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi */
        unencoded_rope_view operator()(int lo, int hi) const;

        /** Returns a substring of *this as an unencoded_rope_view, taken from
            the first cut chars when cut => 0, or the last -cut chars when cut <
            0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope_view operator()(int cut) const;

        /** Returns the maximum size an unencoded_rope can have. */
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        /** Returns a substring of *this as a new unencoded_rope, taken from the
            range of chars at offsets [lo, hi).  If either of lo or hi is a
            negative value x, x is taken to be an offset from the end, and so x
            + size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi */
        unencoded_rope substr(size_type lo, size_type hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope substr(size_type cut) const;

        /** Visits each segment s of *this and calls f(s).  Each segment is a
            string_view or repeated_string_view.  Depending of the operation
            performed on each segment, this may be more efficient than
            iterating over [begin(), end()).

            \pre Fn is an Invocable accepting a single argument of any of the
            types listed above. */
        template<typename Fn>
        void foreach_segment(Fn && f) const
        {
            detail::foreach_leaf(
                ptr_, [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
                    switch (leaf->which_) {
                    case detail::which::t:
                        f(string_view(leaf->as_string()));
                        break;
                    case detail::which::rtv:
                        f(leaf->as_repeated_string_view());
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
        int compare(unencoded_rope rhs) const noexcept;

        bool operator==(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) == 0;
        }

        bool operator!=(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) != 0;
        }

        bool operator<(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) < 0;
        }

        bool operator<=(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) <= 0;
        }

        bool operator>(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) > 0;
        }

        bool operator>=(unencoded_rope rhs) const noexcept
        {
            return compare(rhs) >= 0;
        }

        /** Returns true if *this and rhs contain the same root node pointer.
            This is useful when you want to check for equality between two
            unencoded_ropes that are likely to have originated from the same
            initial unencoded_rope, and may have since been mutated. */
        bool equal_root(unencoded_rope rhs) const noexcept
        {
            return ptr_ == rhs.ptr_;
        }

        void clear() { ptr_ = detail::node_ptr<detail::rope_tag>(); }

        /** Inserts the null-terminated string into *this starting at offset
            at. */
        unencoded_rope & insert(size_type at, char const * c_str);

        /** Inserts the sequence of char from rv into *this starting at offset
            at. */
        unencoded_rope & insert(size_type at, unencoded_rope_view rv);

        /** Inserts the sequence of char from t into *this starting at offset
            at, by moving the contents of t. */
        unencoded_rope & insert(size_type at, string && t)
        {
            return insert_impl(at, std::move(t), would_not_allocate);
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template<typename Iter>
        unencoded_rope & insert(size_type at, Iter first, Iter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template<typename Iter>
        unencoded_rope & insert(const_iterator at, Iter first, Iter last);

#else

        template<typename Iter>
        auto insert(size_type at, Iter first, Iter last)
            -> detail::char_iter_ret_t<unencoded_rope &, Iter>;

        template<typename Iter>
        auto insert(const_iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<unencoded_rope &, Iter>;

#endif

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        unencoded_rope & erase(unencoded_rope_view rv);

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        unencoded_rope & erase(const_iterator first, const_iterator last);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope &
        replace(unencoded_rope_view old_substr, unencoded_rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope & replace(unencoded_rope_view old_substr, string && t);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter>
        unencoded_rope &
        replace(unencoded_rope_view old_substr, Iter first, Iter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter>
        unencoded_rope & replace(
            const_iterator old_first,
            const_iterator old_last,
            Iter new_first,
            Iter new_last);

#else

        template<typename Iter>
        auto replace(unencoded_rope_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<unencoded_rope &, Iter>;

        template<typename Iter>
        auto replace(
            const_iterator old_first,
            const_iterator old_last,
            Iter new_first,
            Iter new_last) -> detail::char_iter_ret_t<unencoded_rope &, Iter>;

#endif

        /** Swaps *this with rhs. */
        void swap(unencoded_rope & rhs) { ptr_.swap(rhs.ptr_); }

        /** Appends rv to *this. */
        unencoded_rope & operator+=(unencoded_rope_view rv);

        /** Appends r to *this, by moving its contents into *this. */
        unencoded_rope & operator+=(unencoded_rope && r)
        {
            detail::interior_node_t<detail::rope_tag> * new_root = nullptr;
            detail::node_ptr<detail::rope_tag> new_root_ptr(
                new_root = detail::new_interior_node<detail::rope_tag>());
            new_root->keys_.push_back(size());
            new_root->keys_.push_back(size() + r.size());
            new_root->children_.push_back(std::move(ptr_));
            new_root->children_.push_back(std::move(r.ptr_));
            ptr_ = std::move(new_root_ptr);
            return *this;
        }

        /** Appends t to *this, by moving its contents into *this. */
        unencoded_rope & operator+=(string && t);

        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<<(std::ostream & os, unencoded_rope r)
        {
            r.foreach_segment(detail::segment_inserter{os});
            return os;
        }

#ifdef BOOST_TEXT_TESTING
        friend void dump_tree(unencoded_rope const & r)
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

#    ifdef BOOST_TEXT_TESTING
    public:
#    endif
        explicit unencoded_rope(
            detail::node_ptr<detail::rope_tag> const & node) :
            ptr_(node)
        {}
#    ifdef BOOST_TEXT_TESTING
    private:
#    endif

        bool self_reference(unencoded_rope_view rv) const;

        struct string_insertion
        {
            explicit operator bool() const { return string_ != nullptr; }

            string * string_;
            detail::found_leaf<detail::rope_tag> found_;
        };

        string_insertion mutable_insertion_leaf(
            size_type at, size_type size, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return string_insertion{nullptr};

            detail::found_leaf<detail::rope_tag> found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return string_insertion{nullptr};
            }

            if (1 < found.leaf_->get()->refs_)
                return string_insertion{nullptr};

            if (found.leaf_->as_leaf()->which_ == detail::which::t) {
                string & t =
                    const_cast<string &>(found.leaf_->as_leaf()->as_string());
                auto const inserted_size = t.size() + size;
                if (size < 0 && t.size() < found.offset_ + -size)
                    return string_insertion{nullptr};
                if ((0 < inserted_size && inserted_size <= t.capacity()) ||
                    (allocation_note == would_allocate &&
                     inserted_size <= detail::string_insert_max)) {
                    return string_insertion{&t, found};
                }
            }

            return string_insertion{nullptr};
        }

        template<typename T>
        unencoded_rope &
        insert_impl(size_type at, T && t, allocation_note_t allocation_note)
        {
            assert(0 <= at && at <= size());

            if (t.empty())
                return *this;

            if (string_insertion insertion =
                    mutable_insertion_leaf(at, t.size(), allocation_note)) {
                auto const t_size = t.size();
                bump_along_path_to_leaf(ptr_, at, t_size);
                insertion.string_->insert(insertion.found_.offset_, t);
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at,
                    detail::make_node(std::forward<T>(t)),
                    detail::encoding_breakage_ok);
            }

            return *this;
        }

        detail::node_ptr<detail::rope_tag> ptr_;

        friend struct detail::const_rope_iterator;
        friend struct unencoded_rope_view;

#endif
    };

}}

#include <boost/text/detail/rope_iterator.hpp>
#include <boost/text/unencoded_rope_view.hpp>

namespace boost { namespace text {

#ifndef BOOST_TEXT_DOXYGEN

    inline unencoded_rope::unencoded_rope(char const * c_str) : ptr_(nullptr)
    {
        insert(0, unencoded_rope_view(c_str));
    }

    inline unencoded_rope::unencoded_rope(unencoded_rope_view rv) :
        ptr_(nullptr)
    {
        insert(0, rv);
    }

    inline unencoded_rope & unencoded_rope::operator=(unencoded_rope_view rv)
    {
        detail::node_ptr<detail::rope_tag> extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        unencoded_rope temp(rv);
        swap(temp);
        return *this;
    }

    inline int unencoded_rope::compare(unencoded_rope rhs) const noexcept
    {
        return unencoded_rope_view(*this).compare(rhs);
    }

    inline unencoded_rope &
    unencoded_rope::insert(size_type at, char const * c_str)
    {
        return insert(at, unencoded_rope_view(c_str));
    }

    inline unencoded_rope &
    unencoded_rope::insert(size_type at, unencoded_rope_view rv)
    {
        assert(0 <= at && at <= size());

        if (rv.empty())
            return *this;

        detail::node_ptr<detail::rope_tag> extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        if (rv.which_ == unencoded_rope_view::which::tv) {
            string_view tv = rv.ref_.tv_;
            bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
            if (tv_null_terminated)
                tv = tv(0, -1);
            return insert_impl(at, tv, would_allocate);
        }

        if (rv.which_ == unencoded_rope_view::which::rtv) {
            if (rv.ref_.rtv_.lo_ == 0 &&
                rv.ref_.rtv_.hi_ == rv.ref_.rtv_.rtv_.size()) {
                repeated_string_view rtv = rv.ref_.rtv_.rtv_;
                bool const rtv_null_terminated =
                    !rtv.view().empty() && rtv.view().end()[-1] == '\0';
                if (rtv_null_terminated)
                    rtv = repeat(rtv.view()(0, -1), rtv.count());
                return insert_impl(at, rtv, would_not_allocate);
            }
            return insert(at, string(rv.begin(), rv.end()));
        }

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        unencoded_rope_view::rope_ref rope_ref = rv.ref_.r_;

        detail::found_leaf<detail::rope_tag> found_lo;
        find_leaf(rope_ref.r_->ptr_, rope_ref.lo_, found_lo);
        detail::leaf_node_t<detail::rope_tag> const * const leaf_lo =
            found_lo.leaf_->as_leaf();

        // If the entire unencoded_rope_view lies within a single segment, slice
        // off the appropriate part of that segment.
        if (found_lo.offset_ + rv.size() <= detail::size(leaf_lo)) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    found_lo.offset_ + rv.size(),
                    true,
                    detail::encoding_breakage_ok),
                detail::encoding_breakage_ok);
            return *this;
        }

        detail::found_leaf<detail::rope_tag> found_hi;
        find_leaf(rope_ref.r_->ptr_, rope_ref.hi_, found_hi);

        bool before_lo = true;
        detail::foreach_leaf(
            rope_ref.r_->ptr_,
            [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
                if (leaf == found_lo.leaf_->as_leaf()) {
                    detail::node_ptr<detail::rope_tag> node;
                    if (found_lo.offset_ != 0) {
                        node = slice_leaf(
                            *found_lo.leaf_,
                            found_lo.offset_,
                            detail::size(leaf),
                            true,
                            detail::encoding_breakage_ok);
                    } else {
                        node = detail::node_ptr<detail::rope_tag>(leaf);
                    }
                    auto const node_size = detail::size(node.get());
                    ptr_ = detail::btree_insert(
                        ptr_,
                        at,
                        std::move(node),
                        detail::encoding_breakage_ok);
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
                                detail::encoding_breakage_ok),
                            detail::encoding_breakage_ok);

                        at += found_hi.offset_;
                    }

                    return false; // break
                }

                ptr_ = detail::btree_insert(
                    ptr_,
                    at,
                    detail::node_ptr<detail::rope_tag>(leaf),
                    detail::encoding_breakage_ok);
                at += detail::size(leaf);

                return true;
            });

        return *this;
    }

    template<typename Iter>
    auto unencoded_rope::insert(size_type at, Iter first, Iter last)
        -> detail::char_iter_ret_t<unencoded_rope &, Iter>
    {
        assert(0 <= at && at <= size());

        if (first == last)
            return *this;

        ptr_ = detail::btree_insert(
            ptr_,
            at,
            detail::make_node(string(first, last)),
            detail::encoding_breakage_ok);

        return *this;
    }

    template<typename Iter>
    auto unencoded_rope::insert(const_iterator at, Iter first, Iter last)
        -> detail::char_iter_ret_t<unencoded_rope &, Iter>
    {
        assert(begin() <= at && at <= end());

        if (first == last)
            return *this;

        ptr_ = detail::btree_insert(
            ptr_,
            at - begin(),
            detail::make_node(string(first, last)),
            detail::encoding_breakage_ok);

        return *this;
    }

    inline unencoded_rope & unencoded_rope::erase(unencoded_rope_view rv)
    {
        assert(self_reference(rv));

        unencoded_rope_view::rope_ref rope_ref = rv.ref_.r_;

        assert(0 <= rope_ref.lo_ && rope_ref.lo_ <= size());
        assert(0 <= rope_ref.hi_ && rope_ref.hi_ <= size());
        assert(rope_ref.lo_ <= rope_ref.hi_);

        if (rope_ref.lo_ == rope_ref.hi_)
            return *this;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        if (string_insertion insertion = mutable_insertion_leaf(
                rope_ref.lo_, -rv.size(), would_not_allocate)) {
            auto const rv_size = rv.size();
            bump_along_path_to_leaf(ptr_, rope_ref.lo_, -rv_size);
            insertion.string_->erase((*insertion.string_)(
                insertion.found_.offset_, insertion.found_.offset_ + rv_size));
        } else {
            ptr_ = btree_erase(
                ptr_, rope_ref.lo_, rope_ref.hi_, detail::encoding_breakage_ok);
        }

        return *this;
    }

    inline unencoded_rope &
    unencoded_rope::erase(const_iterator first, const_iterator last)
    {
        assert(first <= last);
        assert(begin() <= first && last <= end());

        if (first == last)
            return *this;

        auto const lo = first - begin();
        auto const hi = last - begin();
        if (string_insertion insertion =
                mutable_insertion_leaf(lo, -(hi - lo), would_not_allocate)) {
            auto const size = hi - lo;
            bump_along_path_to_leaf(ptr_, lo, -size);
            insertion.string_->erase((*insertion.string_)(
                insertion.found_.offset_, insertion.found_.offset_ + size));
        } else {
            ptr_ = btree_erase(ptr_, lo, hi, detail::encoding_breakage_ok);
        }

        return *this;
    }

    inline unencoded_rope & unencoded_rope::replace(
        unencoded_rope_view old_substr, unencoded_rope_view rv)
    {
        assert(self_reference(old_substr));

        detail::node_ptr<detail::rope_tag> extra_ref;
        unencoded_rope extra_rope;
        if (self_reference(rv)) {
            extra_ref = ptr_;
            extra_rope = unencoded_rope(extra_ref);
            unencoded_rope_view::rope_ref rope_ref = rv.ref_.r_;
            rv = unencoded_rope_view(extra_rope, rope_ref.lo_, rope_ref.hi_);
        }

        return erase(old_substr).insert(old_substr.ref_.r_.lo_, rv);
    }

    inline unencoded_rope &
    unencoded_rope::replace(unencoded_rope_view old_substr, string && t)
    {
        return erase(old_substr).insert(old_substr.ref_.r_.lo_, std::move(t));
    }

    template<typename Iter>
    auto unencoded_rope::replace(
        unencoded_rope_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<unencoded_rope &, Iter>
    {
        assert(self_reference(old_substr));
        assert(0 <= old_substr.size());
        const_iterator const old_first = old_substr.begin().as_rope_iter();
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    template<typename Iter>
    auto unencoded_rope::replace(
        const_iterator old_first,
        const_iterator old_last,
        Iter new_first,
        Iter new_last) -> detail::char_iter_ret_t<unencoded_rope &, Iter>
    {
        assert(old_first <= old_last);
        assert(begin() <= old_first && old_last <= end());
        return erase(old_first, old_last)
            .insert(old_first, new_first, new_last);
    }

    inline unencoded_rope & unencoded_rope::operator+=(unencoded_rope_view rv)
    {
        return insert(size(), rv);
    }

    inline unencoded_rope & unencoded_rope::operator+=(string && t)
    {
        return insert(size(), std::move(t));
    }

    inline unencoded_rope::const_iterator unencoded_rope::begin() const noexcept
    {
        return const_iterator(*this, 0);
    }
    inline unencoded_rope::const_iterator unencoded_rope::end() const noexcept
    {
        return const_iterator(*this, size());
    }

    inline unencoded_rope::const_reverse_iterator unencoded_rope::rbegin() const
        noexcept
    {
        return const_reverse_iterator(const_iterator(*this, size() - 1));
    }
    inline unencoded_rope::const_reverse_iterator unencoded_rope::rend() const
        noexcept
    {
        return const_reverse_iterator(const_iterator(*this, -1));
    }

    inline unencoded_rope_view unencoded_rope::operator()(int lo, int hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);
        return unencoded_rope_view(*this, lo, hi);
    }

    inline unencoded_rope_view unencoded_rope::operator()(int cut) const
    {
        int lo = 0;
        int hi = cut;
        if (cut < 0) {
            lo = cut + size();
            hi = size();
        }
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        return unencoded_rope_view(*this, lo, hi);
    }

    inline unencoded_rope
    unencoded_rope::substr(size_type lo, size_type hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);

        if (lo == hi)
            return unencoded_rope();

        // If the entire substring falls within a single segment, slice
        // off the appropriate part of that segment.
        detail::found_leaf<detail::rope_tag> found;
        detail::find_leaf(ptr_, lo, found);
        if (found.offset_ + hi - lo <= detail::size(found.leaf_->get())) {
            return unencoded_rope(slice_leaf(
                *found.leaf_,
                found.offset_,
                found.offset_ + hi - lo,
                true,
                detail::encoding_breakage_ok));
        }

        // Take an extra ref to the root, which will force all a clone of
        // all the interior nodes.
        detail::node_ptr<detail::rope_tag> new_root = ptr_;

        if (hi != size())
            new_root = detail::btree_erase(
                new_root, hi, size(), detail::encoding_breakage_ok);
        if (lo != 0)
            new_root = detail::btree_erase(
                new_root, 0, lo, detail::encoding_breakage_ok);

        return unencoded_rope(new_root);
    }

    inline unencoded_rope unencoded_rope::substr(size_type cut) const
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

    inline unencoded_rope::const_iterator
    begin(unencoded_rope const & r) noexcept
    {
        return r.begin();
    }
    inline unencoded_rope::const_iterator end(unencoded_rope const & r) noexcept
    {
        return r.end();
    }

    inline unencoded_rope::const_reverse_iterator
    rbegin(unencoded_rope const & r) noexcept
    {
        return r.rbegin();
    }
    inline unencoded_rope::const_reverse_iterator
    rend(unencoded_rope const & r) noexcept
    {
        return r.rend();
    }

    inline bool unencoded_rope::self_reference(unencoded_rope_view rv) const
    {
        return rv.which_ == unencoded_rope_view::which::r &&
               rv.ref_.r_.r_ == this;
    }

#endif

    /** Creates a new unencoded_rope object that is the concatenation of r and
        r2. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope r2)
    {
        return r.insert(r.size(), std::move(r2));
    }

    /** Creates a new unencoded_rope object that is the concatenation of r and
        rv. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope_view rv)
    {
        return r.insert(r.size(), rv);
    }

    /** Creates a new unencoded_rope object that is the concatenation of rv and
        r. */
    inline unencoded_rope operator+(unencoded_rope_view rv, unencoded_rope r)
    {
        return r.insert(0, rv);
    }

    /** Creates a new unencoded_rope object that is the concatenation of r and
        t, by moving the contents of t into the result. */
    inline unencoded_rope operator+(unencoded_rope r, string && t)
    {
        return r.insert(r.size(), std::move(t));
    }

    /** Creates a new unencoded_rope object that is the concatenation of t and
        r, by moving the contents of t into the result. */
    inline unencoded_rope operator+(string && t, unencoded_rope r)
    {
        return r.insert(0, std::move(t));
    }


    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r) noexcept :
        ref_(rope_ref(&r, 0, r.size())),
        which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r, int lo, int hi) :
        ref_(rope_ref(&r, lo, hi)),
        which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(string const & r) noexcept :
        ref_(string_view(r.begin(), r.size())),
        which_(which::tv)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        string const & r, int lo, int hi) :
        ref_(r(lo, hi)),
        which_(which::tv)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        repeated_string_view rtv, int lo, int hi) :
        ref_(repeated_ref(rtv, lo, hi)),
        which_(which::rtv)
    {}

    inline unencoded_rope_view::const_iterator
    unencoded_rope_view::begin() const noexcept
    {
        switch (which_) {
        case which::r:
            return const_iterator(
                detail::const_rope_iterator(ref_.r_.r_, ref_.r_.lo_));
        case which::tv: return const_iterator(ref_.tv_.begin());
        case which::rtv:
            return const_iterator(ref_.rtv_.rtv_.begin() + ref_.rtv_.lo_);
        }
        return const_iterator(); // This should never execute.
    }

    inline unencoded_rope_view::const_iterator unencoded_rope_view::end() const
        noexcept
    {
        switch (which_) {
        case which::r:
            return const_iterator(
                detail::const_rope_iterator(ref_.r_.r_, ref_.r_.hi_));
        case which::tv: return const_iterator(ref_.tv_.end());
        case which::rtv:
            return const_iterator(ref_.rtv_.rtv_.begin() + ref_.rtv_.hi_);
        }
        return const_iterator(); // This should never execute.
    }

    inline unencoded_rope_view::const_reverse_iterator
    unencoded_rope_view::rbegin() const noexcept
    {
        return const_reverse_iterator(end() - 1);
    }
    inline unencoded_rope_view::const_reverse_iterator
    unencoded_rope_view::rend() const noexcept
    {
        return const_reverse_iterator(begin() - 1);
    }

    inline bool unencoded_rope_view::empty() const noexcept
    {
        return begin() == end();
    }

    inline unencoded_rope_view::size_type unencoded_rope_view::size() const
        noexcept
    {
        return end() - begin();
    }

    inline char unencoded_rope_view::operator[](int i) const noexcept
    {
        assert(1 < size());
        return begin()[i];
    }

    inline unencoded_rope_view unencoded_rope_view::
    operator()(int lo, int hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);
        switch (which_) {
        case which::r:
            return unencoded_rope_view(
                ref_.r_.r_, ref_.r_.lo_ + lo, ref_.r_.lo_ + hi);
        case which::tv: return unencoded_rope_view(ref_.tv_(lo, hi));
        case which::rtv:
            return unencoded_rope_view(
                ref_.rtv_.rtv_, ref_.rtv_.lo_ + lo, ref_.rtv_.lo_ + hi);
        }
        return *this; // This should never execute.
    }

    namespace detail {

        template<typename Fn>
        void apply_to_segment(
            detail::leaf_node_t<detail::rope_tag> const * leaf,
            std::ptrdiff_t lo,
            std::ptrdiff_t hi,
            Fn const & f)
        {
            switch (leaf->which_) {
            case detail::which::t: f(leaf->as_string()(lo, hi)); break;
            case detail::which::rtv:
                f(detail::repeated_range{
                    leaf->as_repeated_string_view().begin() + lo,
                    leaf->as_repeated_string_view().begin() + hi});
                break;
            case detail::which::ref:
                f(leaf->as_reference().ref_(lo, hi));
                break;
            default: assert(!"unhandled rope node case"); break;
            }
        }
    }

    template<typename Fn>
    void unencoded_rope_view::foreach_segment(Fn && f) const
    {
        if (which_ == which::tv) {
            f(ref_.tv_);
            return;
        }

        if (which_ == which::rtv) {
            f(detail::repeated_range{ref_.rtv_.rtv_.begin() + ref_.rtv_.lo_,
                                     ref_.rtv_.rtv_.begin() + ref_.rtv_.hi_});
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
                f);
            return;
        }

        bool before_lo = true;
        detail::foreach_leaf(
            r_ref.r_->ptr_,
            [&](detail::leaf_node_t<detail::rope_tag> const * leaf) {
                if (before_lo) {
                    if (leaf == found_lo.leaf_->as_leaf()) {
                        auto const leaf_size = detail::size(leaf);
                        detail::apply_to_segment(
                            leaf, found_lo.offset_, leaf_size, f);
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

        template<typename Iter>
        int
        mismatch_compare(unencoded_rope_view rv, Iter rhs_first, Iter rhs_last)
        {
            if (rv.empty())
                return rhs_first == rhs_last ? 0 : -1;

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

    inline int unencoded_rope_view::compare(unencoded_rope_view rhs) const
        noexcept
    {
        if (which_ == which::tv && rhs.which_ == which::tv)
            return ref_.tv_.compare(rhs.ref_.tv_);

        if (empty())
            return rhs.empty() ? 0 : -1;

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

    inline unencoded_rope_view::iterator begin(unencoded_rope_view rv) noexcept
    {
        return rv.begin();
    }
    inline unencoded_rope_view::iterator end(unencoded_rope_view rv) noexcept
    {
        return rv.end();
    }

    inline unencoded_rope_view::reverse_iterator
    rbegin(unencoded_rope_view rv) noexcept
    {
        return rv.rbegin();
    }
    inline unencoded_rope_view::reverse_iterator
    rend(unencoded_rope_view rv) noexcept
    {
        return rv.rend();
    }


    /** Stream inserter; performs unformatted output. */
    inline std::ostream & operator<<(std::ostream & os, unencoded_rope_view rv)
    {
        rv.foreach_segment(detail::segment_inserter{os});
        return os;
    }

    inline unencoded_rope
    operator+(unencoded_rope_view lhs, unencoded_rope_view rhs)
    {
        unencoded_rope retval(lhs);
        return retval += rhs;
    }

    inline string & string::operator+=(unencoded_rope r)
    {
        return insert(size(), r.begin(), r.end());
    }

    inline string & string::operator+=(unencoded_rope_view rv)
    {
        return insert(size(), rv.begin(), rv.end());
    }

    namespace detail {

#ifdef BOOST_TEXT_TESTING
        template<typename T>
        inline void dump_tree(node_ptr<T> const & root, int key, int indent)
        {
            std::cout << repeated_string_view("    ", indent)
                      << (root->leaf_ ? "LEAF" : "INTR") << " @0x" << std::hex
                      << root.get();
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

}}

#endif
