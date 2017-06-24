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

        template <
            typename T,
            typename R1,
            bool R1IsCharRange = is_char_range<R1, text, text_view>{}
        >
        struct rope_rng_ret {};

        template <typename T, typename R1>
        struct rope_rng_ret<T, R1, true>
        { using type = T; };

        template <typename T, typename R1>
        using rope_rng_ret_t = typename rope_rng_ret<T, R1>::type;

        struct segment_inserter
        {
            template <typename Segment>
            void operator() (Segment const & s) const
            {
                if (os_.good())
                    os_ << s;
            }

            std::ostream & os_;
        };

        template <typename Segment>
        bool encoded (Segment const & segment)
        { return utf8::encoded(segment.begin(), segment.end()); }

        inline bool encoded (repeated_text_view rtv)
        { return utf8::encoded(rtv.view().begin(), rtv.view().end()); }

        struct segment_encoding_checker
        {
            template <typename Segment>
            void operator() (Segment const & s) const
            {
                if (!encoded(s))
                    throw std::invalid_argument("Invalid UTF-8 encoding");
            }
        };

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

        template <typename CharRange>
        explicit rope (
            CharRange const & r,
            detail::rope_rng_ret_t<int *, CharRange> = 0
        ) : ptr_ ()
        { insert(0, r); }

        explicit rope (text_view tv) : ptr_ (nullptr)
        { insert(0, tv); }

        explicit rope (repeated_text_view rtv) : ptr_ (nullptr)
        { insert(0, rtv); }

        template <typename Iter>
        rope (
            Iter first, Iter last,
            detail::char_iter_ret_t<void *, Iter> = 0
        ) : ptr_ ()
        { insert(0, first, last); }

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

        template <typename CharRange>
        auto operator= (CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>
        { return *this = text_view(&*r.begin(), r.end() - r.begin()); }

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
            detail::foreach_leaf(ptr_, [&](detail::leaf_node_t const * leaf) {
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
                return true;
            });
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

        template <typename CharRange>
        auto insert (size_type at, CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>
        { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

        // TODO: Document that each insert(), erase(), and replace() overload
        // has only the basic guarantee.
        rope & insert (size_type at, rope_view rv);

        rope & insert (size_type at, text const & t)
        { return insert_impl(at, t, would_allocate); }

        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), would_not_allocate); }

        rope & insert (size_type at, text_view tv)
        {
            bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
            if (tv_null_terminated)
                tv = tv(0, -1);
            return insert_impl(at, tv, would_not_allocate);
        }

        rope & insert (size_type at, repeated_text_view rtv)
        {
            bool const rtv_null_terminated =
                !rtv.view().empty() && rtv.view().end()[-1] == '\0';
            if (rtv_null_terminated)
                rtv = repeat(rtv.view()(0, -1), rtv.count());
            return insert_impl(at, rtv, would_not_allocate);
        }

        // TODO: Document that the inserted/replaced sequence need not be
        // UTF-8 encoded, since direct use of iterators is the unsafe
        // interface.  (To once again make it safe, use one of the converting
        // iterators.)
        template <typename Iter>
        auto insert (size_type at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto insert (const_iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        rope & erase (rope_view rv);
        rope & erase (const_iterator first, const_iterator last);

        template <typename CharRange>
        auto replace (rope_view old_substr, CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>;

        rope & replace (rope_view old_substr, rope_view rv);
        rope & replace (rope_view old_substr, text const & t);
        rope & replace (rope_view old_substr, text && t);
        rope & replace (rope_view old_substr, text_view tv);
        rope & replace (rope_view old_substr, repeated_text_view rtv);

        template <typename Iter>
        auto replace (rope_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        inline rope & operator+= (rope_view rv);
        inline rope & operator+= (text const & t);
        inline rope & operator+= (text && t);
        inline rope & operator+= (text_view tv);
        inline rope & operator+= (repeated_text_view rtv);

        template <typename CharRange>
        auto operator+= (CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>;

        friend std::ostream & operator<< (std::ostream & os, rope r)
        {
            if (os.good()) {
                detail::pad_width_before(os, r.size());
                r.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, r.size());
            }
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

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

        explicit rope (detail::node_ptr const & node) : ptr_ (node) {}

        bool self_reference (rope_view rv) const;

        struct text_insertion
        {
            explicit operator bool () const
            { return text_ != nullptr; }

            text * text_;
            std::ptrdiff_t offset_;
        };

        text_insertion mutable_insertion_leaf (size_type at, size_type size, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return text_insertion{nullptr};

            detail::found_leaf found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return text_insertion{nullptr};
            }

            if (found.leaf_->as_leaf()->which_ == detail::node_t::which::t) {
                text & t = const_cast<text &>(found.leaf_->as_leaf()->as_text());
                auto const inserted_size = t.size() + size;
                if (inserted_size <= t.capacity())
                    return text_insertion{&t, found.offset_};
                else if (allocation_note == would_allocate && inserted_size <= detail::text_insert_max)
                    return text_insertion{&t, found.offset_};
            }

            return text_insertion{nullptr};
        }

        template <typename T>
        rope & insert_impl (
            size_type at,
            T && t,
            allocation_note_t allocation_note,
            detail::encoding_note_t encoding_note = detail::check_encoding_breakage
        ) {
            if (t.empty())
                return *this;

            if (text_insertion insertion = mutable_insertion_leaf(at, t.size(), allocation_note)) {
                if (encoding_note == detail::encoding_breakage_ok)
                    insertion.text_->insert(insertion.text_->begin() + insertion.offset_, t.begin(), t.end());
                else
                    insertion.text_->insert(insertion.offset_, t);
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at,
                    detail::make_node(std::forward<T &&>(t)),
                    encoding_note
                );
            }

            return *this;
        }

        detail::node_ptr ptr_;

        friend struct detail::const_rope_iterator;
        friend struct rope_view;
    };


    inline rope const & checked_encoding (rope const & r);
    inline rope && checked_encoding (rope && r);

} }

#include <boost/text/detail/rope_iterator.hpp>
#include <boost/text/rope_view.hpp>

namespace boost { namespace text {

    inline rope::rope (rope_view rv) : ptr_ (nullptr)
    { insert(0, rv); }

    inline rope & rope::operator= (rope_view rv)
    {
        detail::node_ptr extra_ref;
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
        if (rv.empty())
            return *this;

        detail::node_ptr extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        detail::found_leaf found_lo;
        find_leaf(rv.r_->ptr_, rv.lo_, found_lo);
        detail::leaf_node_t const * const leaf_lo = found_lo.leaf_->as_leaf();

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
                    detail::check_encoding_breakage
                )
            );
            return *this;
        }

        {
            detail::node_ptr node;
            if (found_lo.offset_ != 0) {
                node = slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    detail::size(leaf_lo),
                    true,
                    detail::check_encoding_breakage
                );
            } else {
                node = detail::node_ptr(leaf_lo);
            }
            ptr_ = detail::btree_insert(ptr_, at, std::move(node));
        }
        at += detail::size(leaf_lo);

        detail::found_leaf found_hi;
        find_leaf(rv.r_->ptr_, rv.hi_, found_hi);
        detail::leaf_node_t const * const leaf_hi = found_hi.leaf_->as_leaf();

        detail::foreach_leaf(ptr_, [&](detail::leaf_node_t const * leaf) {
            if (leaf == leaf_lo)
                return true; // continue
            if (leaf == leaf_hi)
                return false; // break
            ptr_ = detail::btree_insert(ptr_, at, detail::node_ptr(leaf));
            at += detail::size(leaf);
            return true;
        });

        if (found_hi.offset_ != 0) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_hi.leaf_,
                    0,
                    found_hi.offset_,
                    true,
                    detail::check_encoding_breakage
                )
            );
        }

        return *this;
    }

    template <typename Iter>
    auto rope::insert (size_type at, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(0 <= at && at <= size());

        if (first == last)
            return *this;

        if (!utf8::starts_encoded(begin() + at, end()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        ptr_ = detail::btree_insert(ptr_, at, detail::make_node(text(first, last)));

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

    template <typename CharRange>
    auto rope::replace (rope_view old_substr, CharRange const & r)
        -> detail::rope_rng_ret_t<rope &, CharRange>
    { return replace(old_substr, text_view(&*r.begin(), r.end() - r.begin())); }

    inline rope & rope::replace (rope_view old_substr, rope_view rv)
    {
        detail::node_ptr extra_ref;
        rope extra_rope;
        if (self_reference(rv)) {
            extra_ref = ptr_;
            extra_rope = rope(extra_ref);
            rv = rope_view(extra_rope, rv.lo_, rv.hi_);
        }

        return erase(old_substr).insert(old_substr.lo_, rv);
    }

    inline rope & rope::replace (rope_view old_substr, text const & t)
    { return erase(old_substr).insert(old_substr.lo_, t); }

    inline rope & rope::replace (rope_view old_substr, text && t)
    { return erase(old_substr).insert(old_substr.lo_, std::move(t)); }

    inline rope & rope::replace (rope_view old_substr, text_view tv)
    { return erase(old_substr).insert(old_substr.lo_, tv); }

    inline rope & rope::replace (rope_view old_substr, repeated_text_view rtv)
    { return erase(old_substr).insert(old_substr.lo_, rtv); }

    template <typename Iter>
    auto rope::replace (rope_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(self_reference(old_substr));
        assert(0 <= old_substr.size());
        const_iterator const old_first = old_substr.begin();
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

    inline rope & rope::operator+= (text const & t)
    { return insert(size(), t); }

    inline rope & rope::operator+= (text && t)
    { return insert(size(), std::move(t)); }

    inline rope & rope::operator+= (text_view tv)
    { return insert(size(), tv); }

    inline rope & rope::operator+= (repeated_text_view rtv)
    { return insert(size(), rtv); }

    template <typename CharRange>
    auto rope::operator+= (CharRange const & r)
        -> detail::rope_rng_ret_t<rope &, CharRange>
    { return insert(size(), text_view(&*r.begin(), r.end() - r.begin())); }

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

    inline rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    inline rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    inline rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    inline rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

    inline bool rope::self_reference (rope_view rv) const
    { return rv.r_ == this; }


    inline rope operator+ (rope r, rope_view rv)
    { return r.insert(r.size(), rv); }

    inline rope operator+ (rope r, text const & t)
    { return r.insert(r.size(), t); }

    inline rope operator+ (rope r, text && t)
    { return r.insert(r.size(), std::move(t)); }

    inline rope operator+ (rope r, text_view tv)
    { return r.insert(r.size(), tv); }

    inline rope operator+ (rope r, repeated_text_view rtv)
    { return r.insert(r.size(), rtv); }

    template <typename CharRange>
    auto operator+ (rope r, CharRange const & range)
        -> detail::rope_rng_ret_t<rope, CharRange>
    { return r.insert(r.size(), text_view(&*range.begin(), range.end() - range.begin())); }


    inline rope operator+ (rope_view rv, rope r)
    { return r.insert(0, rv); }

    inline rope operator+ (text const & t, rope r)
    { return r.insert(0, t); }

    inline rope operator+ (text && t, rope r)
    { return r.insert(0, std::move(t)); }

    inline rope operator+ (text_view tv, rope r)
    { return r.insert(0, tv); }

    inline rope operator+ (repeated_text_view rtv, rope r)
    { return r.insert(0, rtv); }

    template <typename CharRange>
    auto operator+ (CharRange const & range, rope r)
        -> detail::rope_rng_ret_t<rope, CharRange>
    { return r.insert(0, text_view(&*range.begin(), range.end() - range.begin())); }

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


    namespace detail {

#ifdef BOOST_TEXT_TESTING
        inline void dump_tree (node_ptr const & root, int key, int indent)
        {
            std::cout << repeated_text_view("    ", indent)
                      << (root->leaf_ ? "LEAF" : "INTR")
                      << " @0x" << std::hex << root.get();
            if (key != -1)
                std::cout << " < " << key;
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
