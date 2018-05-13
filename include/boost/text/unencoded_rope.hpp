#ifndef BOOST_TEXT_UNENCODED_ROPE_HPP
#define BOOST_TEXT_UNENCODED_ROPE_HPP

#include <boost/text/detail/rope.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#ifdef BOOST_TEXT_TESTING
#include <iostream>
#endif


namespace boost { namespace text {

    struct unencoded_rope_view;

    namespace detail {
        struct const_rope_iterator;
        struct const_reverse_rope_iterator;
    }

    /** A mutable sequence of char with copy-on-write semantics.  An
        unencoded_rope is non-contiguous and is not null-terminated. */
    struct unencoded_rope
    {
        using value_type = char;
        using size_type = std::ptrdiff_t;
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        unencoded_rope() noexcept : ptr_(nullptr) {}

        unencoded_rope(unencoded_rope const & rhs) = default;
        unencoded_rope(unencoded_rope && rhs) noexcept = default;

        /** Constructs an unencoded_rope from a null-terminated string. */
        unencoded_rope(char const * c_str);

        /** Constructs an unencoded_rope from an unencoded_rope_view. */
        explicit unencoded_rope(unencoded_rope_view rv);

        /** Move-constructs an unencoded_rope from a string. */
        explicit unencoded_rope(string && s) :
            ptr_(detail::make_node(std::move(s)))
        {}

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a unencoded_rope from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        explicit unencoded_rope(CharRange const & r);

        /** Constructs an unencoded_rope from a sequence of char.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        unencoded_rope(CharIter first, CharIter last);

        /** Constructs a unencoded_rope from a range of graphemes over an
            underlying range of char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit unencoded_rope(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit unencoded_rope(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            ptr_()
        {
            insert(0, r);
        }

        template<typename CharIter>
        unencoded_rope(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0) :
            ptr_()
        {
            insert(0, first, last);
        }

        template<typename GraphemeRange>
        explicit unencoded_rope(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0)
        {
            using std::begin;
            using std::end;
            insert(0, begin(r).base().base(), end(r).base().base());
        }

#endif

        unencoded_rope & operator=(unencoded_rope const & rhs) = default;
        unencoded_rope & operator=(unencoded_rope && rhs) noexcept = default;

        /** Assignment from an unencoded_rope_view. */
        unencoded_rope & operator=(unencoded_rope_view rv);

        /** Move-assignment from a string. */
        unencoded_rope & operator=(string && s)
        {
            unencoded_rope temp(std::move(s));
            swap(temp);
            return *this;
        }

        /** Assignment from a null-terminated string. */
        unencoded_rope & operator=(char const * c_str);

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        unencoded_rope & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        unencoded_rope & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope &, CharRange>
        {
            unencoded_rope temp(r);
            swap(temp);
            return *this;
        }

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<unencoded_rope &, GraphemeRange>
        {
            unencoded_rope temp(r);
            swap(temp);
            return *this;
        }

#endif

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator rend() const noexcept;

        const_reverse_iterator crbegin() const noexcept;
        const_reverse_iterator crend() const noexcept;

        bool empty() const noexcept { return size() == 0; }

        size_type size() const noexcept { return detail::size(ptr_.get()); }

        /** Returns the char (not a reference) of *this at index i, or the
            char at index -i when i < 0.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        char operator[](size_type i) const noexcept
        {
            assert(ptr_);
            if (i < 0)
                i += size();
            assert(0 <= i && i < size());
            detail::found_char found;
            find_char(ptr_, i, found);
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
        unencoded_rope_view operator()(size_type lo, size_type hi) const;

        /** Returns a substring of *this as an unencoded_rope_view, taken from
            the first cut chars when cut => 0, or the last -cut chars when cut <
            0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope_view operator()(size_type cut) const;

        /** Returns the maximum size an unencoded_rope can have. */
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

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

        /** Inserts the null-terminated string into *this starting at position
            at. */
        const_iterator insert(const_iterator at, char const * c_str);

        /** Inserts the sequence of char from rv into *this starting at offset
            at. */
        unencoded_rope & insert(size_type at, unencoded_rope_view rv);

        /** Inserts the sequence of char from rv into *this starting at
            position at. */
        const_iterator insert(const_iterator at, unencoded_rope_view rv);

        /** Inserts the sequence of char from t into *this starting at offset
            at, by moving the contents of t. */
        unencoded_rope & insert(size_type at, string && s)
        {
            return insert_impl(at, std::move(s), would_not_allocate);
        }

        /** Inserts the sequence of char from t into *this starting at
            position at, by moving the contents of t. */
        const_iterator insert(const_iterator at, string && s);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char sequence r into *this starting at offset at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        auto insert(size_type at, CharRange const & r);

        /** Inserts the char sequence r into *this starting at position at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        auto insert(const_iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        unencoded_rope & insert(size_type at, CharIter first, CharIter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        unencoded_rope &
        insert(const_iterator at, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto insert(size_type at, CharRange const & r) -> detail::rng_alg_ret_t<
            unencoded_rope &,
            CharRange,
            string_view,
            repeated_string_view>;

        template<typename CharRange>
        auto insert(const_iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<
                const_iterator,
                CharRange,
                string_view,
                repeated_string_view>;

        template<typename CharIter>
        auto insert(size_type at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<unencoded_rope &, CharIter>;

        template<typename CharIter>
        auto insert(const_iterator at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<const_iterator, CharIter>;

#endif

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        unencoded_rope & erase(unencoded_rope_view rv);

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        const_iterator erase(const_iterator first, const_iterator last);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from c_str.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope &
        replace(unencoded_rope_view old_substr, char const * c_str);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope &
        replace(unencoded_rope_view old_substr, unencoded_rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope & replace(unencoded_rope_view old_substr, string && s);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence r.

            This function only participates in overload resolution if CharRange
            models the CharRange concept.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename CharRange>
        auto replace(unencoded_rope_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename CharIter>
        unencoded_rope &
        replace(unencoded_rope_view old_substr, CharIter first, CharIter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename CharIter>
        unencoded_rope & replace(
            const_iterator old_first,
            const_iterator old_last,
            CharIter new_first,
            CharIter new_last);

#else

        template<typename CharRange>
        auto replace(unencoded_rope_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<
                unencoded_rope &,
                CharRange,
                string_view,
                repeated_string_view>;

        template<typename CharIter>
        auto
        replace(unencoded_rope_view old_substr, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<unencoded_rope &, CharIter>;

        template<typename CharIter>
        auto replace(
            const_iterator old_first,
            const_iterator old_last,
            CharIter new_first,
            CharIter new_last)
            -> detail::char_iter_ret_t<unencoded_rope &, CharIter>;

#endif

        /** Swaps *this with rhs. */
        void swap(unencoded_rope & rhs) { ptr_.swap(rhs.ptr_); }

        /** Appends c_str to *this. */
        unencoded_rope & operator+=(char const * c_str);

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
        unencoded_rope & operator+=(string && s);


#ifdef BOOST_TEXT_DOXYGEN

        /** Append r to *this.

            This function only participates in overload resolution if CharRange
            models the CharRange concept. */
        template<typename CharRange>
        unencoded_rope & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope, CharRange, string>
        {
            using std::begin;
            using std::end;
            return insert(size(), begin(r), end(r));
        }

#endif

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

#ifdef BOOST_TEXT_TESTING
    public:
#endif
        explicit unencoded_rope(
            detail::node_ptr<detail::rope_tag> const & node) :
            ptr_(node)
        {}
#ifdef BOOST_TEXT_TESTING
    private:
#endif

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
                    ptr_, at, detail::make_node(std::forward<T>(t)));
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

#ifndef BOOST_TEXT_DOXYGEN

namespace boost { namespace text {

    inline unencoded_rope::unencoded_rope(char const * c_str) : ptr_(nullptr)
    {
        insert(0, unencoded_rope_view(c_str));
    }

    inline unencoded_rope::unencoded_rope(unencoded_rope_view rv) :
        ptr_(nullptr)
    {
        insert(0, rv);
    }

    inline int unencoded_rope::compare(unencoded_rope rhs) const noexcept
    {
        return unencoded_rope_view(*this).compare(rhs);
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

    inline unencoded_rope & unencoded_rope::operator=(char const * c_str)
    {
        unencoded_rope temp(c_str);
        swap(temp);
        return *this;
    }

    inline unencoded_rope &
    unencoded_rope::insert(size_type at, char const * c_str)
    {
        return insert(at, unencoded_rope_view(c_str));
    }

    inline unencoded_rope::const_iterator
    unencoded_rope::insert(const_iterator at, char const * c_str)
    {
        auto const offset = at - begin();
        insert(at - begin(), unencoded_rope_view(c_str));
        return begin() + offset;
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

        // If the entire unencoded_rope_view lies within a single segment,
        // slice off the appropriate part of that segment.
        if (found_lo.offset_ + rv.size() <= detail::size(leaf_lo)) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    found_lo.offset_ + rv.size(),
                    true));
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
                            true);
                    } else {
                        node = detail::node_ptr<detail::rope_tag>(leaf);
                    }
                    auto const node_size = detail::size(node.get());
                    ptr_ = detail::btree_insert(ptr_, at, std::move(node));
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
                                *found_hi.leaf_, 0, found_hi.offset_, true));

                        at += found_hi.offset_;
                    }

                    return false; // break
                }

                ptr_ = detail::btree_insert(
                    ptr_, at, detail::node_ptr<detail::rope_tag>(leaf));
                at += detail::size(leaf);

                return true;
            });

        return *this;
    }

    inline unencoded_rope::const_iterator
    unencoded_rope::insert(const_iterator at, unencoded_rope_view rv)
    {
        auto const offset = at - begin();
        insert(at - begin(), rv);
        return begin() + offset;
    }

    inline unencoded_rope::const_iterator
    unencoded_rope::insert(const_iterator at, string && s)
    {
        auto const offset = at - begin();
        insert_impl(at - begin(), std::move(s), would_not_allocate);
        return begin() + offset;
    }

    template<typename CharRange>
    auto unencoded_rope::insert(size_type at, CharRange const & r)
        -> detail::rng_alg_ret_t<
            unencoded_rope &,
            CharRange,
            string_view,
            repeated_string_view>
    {
        using std::begin;
        using std::end;
        insert(at, begin(r), end(r));
        return *this;
    }

    template<typename CharRange>
    auto unencoded_rope::insert(const_iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<
            const_iterator,
            CharRange,
            string_view,
            repeated_string_view>
    {
        using std::begin;
        using std::end;
        auto const offset = at - this->begin();
        insert(at, begin(r), end(r));
        return this->begin() + offset;
    }

    template<typename CharIter>
    auto unencoded_rope::insert(size_type at, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<unencoded_rope &, CharIter>
    {
        assert(0 <= at && at <= size());

        if (first == last)
            return *this;

        ptr_ = detail::btree_insert(
            ptr_, at, detail::make_node(string(first, last)));

        return *this;
    }

    template<typename CharIter>
    auto
    unencoded_rope::insert(const_iterator at, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<const_iterator, CharIter>
    {
        assert(begin() <= at && at <= end());

        if (first == last)
            return at;

        auto const offset = at - begin();
        ptr_ = detail::btree_insert(
            ptr_, at - begin(), detail::make_node(string(first, last)));
        return begin() + offset;
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
            ptr_ = btree_erase(ptr_, rope_ref.lo_, rope_ref.hi_);
        }

        return *this;
    }

    inline unencoded_rope::const_iterator
    unencoded_rope::erase(const_iterator first, const_iterator last)
    {
        assert(first <= last);
        assert(begin() <= first && last <= end());

        if (first == last)
            return first;

        auto const offset = first - begin();

        auto const lo = first - begin();
        auto const hi = last - begin();
        if (string_insertion insertion =
                mutable_insertion_leaf(lo, -(hi - lo), would_not_allocate)) {
            auto const size = hi - lo;
            bump_along_path_to_leaf(ptr_, lo, -size);
            insertion.string_->erase((*insertion.string_)(
                insertion.found_.offset_, insertion.found_.offset_ + size));
        } else {
            ptr_ = btree_erase(ptr_, lo, hi);
        }

        return begin() + offset;
    }

    inline unencoded_rope &
    unencoded_rope::replace(unencoded_rope_view old_substr, char const * c_str)
    {
        return replace(old_substr, unencoded_rope_view(c_str));
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
    unencoded_rope::replace(unencoded_rope_view old_substr, string && s)
    {
        return erase(old_substr).insert(old_substr.ref_.r_.lo_, std::move(s));
    }

    template<typename CharRange>
    auto
    unencoded_rope::replace(unencoded_rope_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<
            unencoded_rope &,
            CharRange,
            string_view,
            repeated_string_view>
    {
        using std::begin;
        using std::end;
        return replace(old_substr, begin(r), end(r));
    }

    template<typename CharIter>
    auto unencoded_rope::replace(
        unencoded_rope_view old_substr, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<unencoded_rope &, CharIter>
    {
        assert(self_reference(old_substr));
        assert(0 <= old_substr.size());
        const_iterator const old_first = old_substr.begin().as_rope_iter();
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    template<typename CharIter>
    auto unencoded_rope::replace(
        const_iterator old_first,
        const_iterator old_last,
        CharIter new_first,
        CharIter new_last)
        -> detail::char_iter_ret_t<unencoded_rope &, CharIter>
    {
        assert(old_first <= old_last);
        assert(begin() <= old_first && old_last <= end());
        auto const it = erase(old_first, old_last);
        insert(it, new_first, new_last);
        return *this;
    }

    inline unencoded_rope & unencoded_rope::operator+=(char const * c_str)
    {
        return insert(size(), unencoded_rope_view(c_str));
    }

    inline unencoded_rope & unencoded_rope::operator+=(unencoded_rope_view rv)
    {
        return insert(size(), rv);
    }

    inline unencoded_rope & unencoded_rope::operator+=(string && s)
    {
        return insert(size(), std::move(s));
    }

    inline unencoded_rope::const_iterator unencoded_rope::begin() const noexcept
    {
        return const_iterator(*this, 0);
    }
    inline unencoded_rope::const_iterator unencoded_rope::end() const noexcept
    {
        return const_iterator(*this, size());
    }

    inline unencoded_rope::const_iterator unencoded_rope::cbegin() const
        noexcept
    {
        return begin();
    }
    inline unencoded_rope::const_iterator unencoded_rope::cend() const noexcept
    {
        return end();
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

    inline unencoded_rope::const_reverse_iterator
    unencoded_rope::crbegin() const noexcept
    {
        return rbegin();
    }
    inline unencoded_rope::const_reverse_iterator unencoded_rope::crend() const
        noexcept
    {
        return rend();
    }

    inline unencoded_rope_view unencoded_rope::
    operator()(size_type lo, size_type hi) const
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

    inline unencoded_rope_view unencoded_rope::operator()(size_type cut) const
    {
        size_type lo = 0;
        size_type hi = cut;
        if (cut < 0) {
            lo = cut + size();
            hi = size();
        }
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        return unencoded_rope_view(*this, lo, hi);
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

    inline unencoded_rope::const_iterator
    cbegin(unencoded_rope const & r) noexcept
    {
        return r.cbegin();
    }
    inline unencoded_rope::const_iterator
    cend(unencoded_rope const & r) noexcept
    {
        return r.cend();
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

    inline unencoded_rope::const_reverse_iterator
    crbegin(unencoded_rope const & r) noexcept
    {
        return r.crbegin();
    }
    inline unencoded_rope::const_reverse_iterator
    crend(unencoded_rope const & r) noexcept
    {
        return r.crend();
    }

    inline bool unencoded_rope::self_reference(unencoded_rope_view rv) const
    {
        return rv.which_ == unencoded_rope_view::which::r &&
               rv.ref_.r_.r_ == this;
    }

}}

#endif

namespace boost { namespace text {

    inline bool
    operator==(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return algorithm::equal(
            lhs, lhs + std::strlen(lhs), rhs.begin(), rhs.end());
    }
    inline bool
    operator==(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool
    operator!=(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool
    operator!=(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return rhs != lhs;
    }

    inline bool operator<(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return detail::generalized_compare(
                   lhs, lhs + std::strlen(lhs), rhs.begin(), rhs.end()) < 0;
    }
    inline bool operator<(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return detail::generalized_compare(
                   lhs.begin(), lhs.end(), rhs, rhs + std::strlen(rhs)) < 0;
    }

    inline bool
    operator<=(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return lhs < rhs || lhs == rhs;
    }
    inline bool
    operator<=(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return lhs < rhs || lhs == rhs;
    }

    inline bool operator>(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return rhs < lhs;
    }
    inline bool operator>(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return rhs < lhs;
    }

    inline bool
    operator>=(char const * lhs, unencoded_rope const & rhs) noexcept
    {
        return rhs <= lhs;
    }
    inline bool
    operator>=(unencoded_rope const & lhs, char const * rhs) noexcept
    {
        return rhs <= lhs;
    }

    template<typename CharRange>
    auto operator==(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        using std::begin;
        using std::end;
        return algorithm::equal(begin(lhs), end(lhs), rhs.begin(), rhs.end());
    }
    template<typename CharRange>
    auto operator==(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        using std::begin;
        using std::end;
        return algorithm::equal(lhs.begin(), lhs.end(), begin(rhs), end(rhs));
    }

    template<typename CharRange>
    auto operator!=(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return !(lhs == rhs);
    }
    template<typename CharRange>
    auto operator!=(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return rhs != lhs;
    }

    template<typename CharRange>
    auto operator<(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        using std::begin;
        using std::end;
        return detail::generalized_compare(
                   begin(lhs), end(lhs), rhs.begin(), rhs.end()) < 0;
    }
    template<typename CharRange>
    auto operator<(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        using std::begin;
        using std::end;
        return detail::generalized_compare(
                   lhs.begin(), lhs.end(), begin(rhs), end(rhs)) < 0;
    }

    template<typename CharRange>
    auto operator<=(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return lhs < rhs || lhs == rhs;
    }
    template<typename CharRange>
    auto operator<=(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return lhs < rhs || lhs == rhs;
    }

    template<typename CharRange>
    auto operator>(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return rhs < lhs;
    }
    template<typename CharRange>
    auto operator>(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return rhs < lhs;
    }

    template<typename CharRange>
    auto operator>=(CharRange const & lhs, unencoded_rope const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return rhs <= lhs;
    }
    template<typename CharRange>
    auto operator>=(unencoded_rope const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return rhs <= lhs;
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
        and c_str. */
    inline unencoded_rope operator+(unencoded_rope r, char const * c_str)
    {
        return r += string_view(c_str);
    }

    /** Creates a new unencoded_rope object that is the concatenation of c_str
        and r. */
    inline unencoded_rope operator+(char const * c_str, unencoded_rope r)
    {
        return r.insert(0, c_str);
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
        and r2. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope r2)
    {
        return r += r2;
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
       and rv. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope_view rv)
    {
        return r += rv;
    }

    /** Creates a new unencoded_rope object that is the concatenation of rv
       and r. */
    inline unencoded_rope operator+(unencoded_rope_view rv, unencoded_rope r)
    {
        return r.insert(0, rv);
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
        and t, by moving the contents of t into the result. */
    inline unencoded_rope operator+(unencoded_rope r, string && s)
    {
        return r += std::move(s);
    }

    /** Creates a new unencoded_rope object that is the concatenation of t
        and r, by moving the contents of t into the result. */
    inline unencoded_rope operator+(string && s, unencoded_rope r)
    {
        return r.insert(0, std::move(s));
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new unencoded_rope object that is the concatenation of ur
        and r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept. */
    template<typename CharRange>
    unencoded_rope & operator+(unencoded_rope ur, CharRange const & r);

    /** Creates a new unencoded_rope object that is the concatenation of r
        and ur.

        This function only participates in overload resolution if CharRange
        models the CharRange concept. */
    template<typename CharRange>
    unencoded_rope & operator+(CharRange const & r, unencoded_rope const & ur);

#else

    template<typename CharRange>
    auto operator+(unencoded_rope ur, CharRange const & r)
        -> detail::rng_alg_ret_t<unencoded_rope, CharRange, string>
    {
        return ur += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, unencoded_rope ur)
        -> detail::rng_alg_ret_t<unencoded_rope, CharRange, string>
    {
        using std::begin;
        using std::end;
        return ur.insert(0, begin(r), end(r));
    }

#endif


    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r) noexcept :
        ref_(rope_ref(&r, 0, r.size())),
        which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r, size_type lo, size_type hi) :
        ref_(rope_ref(&r, lo, hi)),
        which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(string const & s) noexcept :
        ref_(string_view(s.begin(), s.size())),
        which_(which::tv)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        string const & s, int lo, int hi) :
        ref_(s(lo, hi)),
        which_(which::tv)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        repeated_string_view rsv, size_type lo, size_type hi) :
        ref_(repeated_ref(rsv, lo, hi)),
        which_(which::rtv)
    {}

    template<typename ContigCharRange>
    unencoded_rope_view::unencoded_rope_view(
        ContigCharRange const & r,
        detail::contig_rng_alg_ret_t<int *, ContigCharRange>) :
        ref_(rope_ref())
    {
        using std::begin;
        using std::end;
        if (begin(r) == end(r)) {
            *this = unencoded_rope_view();
        } else {
            *this =
                unencoded_rope_view(string_view(&*begin(r), end(r) - begin(r)));
        }
    }

    template<typename ContigGraphemeRange>
    unencoded_rope_view::unencoded_rope_view(
        ContigGraphemeRange const & r,
        detail::contig_graph_rng_alg_ret_t<int *, ContigGraphemeRange>) :
        ref_(rope_ref())
    {
        using std::begin;
        using std::end;
        if (begin(r) == end(r)) {
            *this = unencoded_rope_view();
        } else {
            *this = unencoded_rope_view(string_view(
                &*begin(r).base().base(),
                end(r).base().base() - begin(r).base().base()));
        }
    }

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

    inline unencoded_rope_view::const_iterator
    unencoded_rope_view::cbegin() const noexcept
    {
        return begin();
    }
    inline unencoded_rope_view::const_iterator unencoded_rope_view::cend() const
        noexcept
    {
        return end();
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

    inline unencoded_rope_view::const_reverse_iterator
    unencoded_rope_view::crbegin() const noexcept
    {
        return rbegin();
    }
    inline unencoded_rope_view::const_reverse_iterator
    unencoded_rope_view::crend() const noexcept
    {
        return rend();
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

    inline char unencoded_rope_view::operator[](size_type i) const noexcept
    {
        if (i < 0)
            i += size();
        assert(0 <= i && i < size());
        return begin()[i];
    }

    inline unencoded_rope_view unencoded_rope_view::
    operator()(size_type lo, size_type hi) const
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

    inline unencoded_rope_view::iterator begin(unencoded_rope_view rv) noexcept
    {
        return rv.begin();
    }
    inline unencoded_rope_view::iterator end(unencoded_rope_view rv) noexcept
    {
        return rv.end();
    }

    inline unencoded_rope_view::iterator cbegin(unencoded_rope_view rv) noexcept
    {
        return rv.cbegin();
    }
    inline unencoded_rope_view::iterator cend(unencoded_rope_view rv) noexcept
    {
        return rv.cend();
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

    inline unencoded_rope_view::reverse_iterator
    crbegin(unencoded_rope_view rv) noexcept
    {
        return rv.crbegin();
    }
    inline unencoded_rope_view::reverse_iterator
    crend(unencoded_rope_view rv) noexcept
    {
        return rv.crend();
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


    /** Stream inserter; performs unformatted output. */
    inline std::ostream & operator<<(std::ostream & os, unencoded_rope_view rv)
    {
        rv.foreach_segment(detail::segment_inserter{os});
        return os;
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
