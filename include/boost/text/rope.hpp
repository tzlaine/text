#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <iterator>


namespace boost { namespace text {

    namespace detail {
        struct const_rope_iterator;
    }

    struct rope_view;

    /** TODO. */
    struct rope
    {
        using iterator = grapheme_iterator<utf8::to_utf32_iterator<
            detail::const_rope_iterator,
            detail::const_rope_iterator>>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

        /** Default ctor. */
        rope() {}

        /** Constructs a rope from a null-terminated string. */
        rope(char const * c_str);

        /** Constructs a rope from a null-terminated string. */
        template<int N>
        rope(char (&c_str)[N]);

        /** Constructs a rope from a string_view. */
        explicit rope(string_view sv);

        /** Constructs a rope from a rope_view. */
        explicit rope(rope_view rv);

        /** Move-constructs a rope from a string. */
        explicit rope(string && s);

        /** Move-constructs a rope from a text. */
        explicit rope(text && t);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a rope from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        explicit rope(CharRange const & r);

        /** Constructs a rope from a sequence of char.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        rope(CharIter first, Iter Charlast);

#else

        template<typename CharRange>
        explicit rope(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0);

        template<typename CharIter>
        rope(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0);

#endif

        /** Assignment from a rope_view. */
        rope & operator=(rope_view rv);

        /** Assignment from a string_view. */
        rope & operator=(string_view sv);

        /** Move-assignment from a string. */
        rope & operator=(string && s);

        /** Move-assignment from a text. */
        rope & operator=(text && t);

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        rope & operator=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<rope &, CharRange>;

#endif

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator rend() const noexcept;

        const_reverse_iterator crbegin() const noexcept;
        const_reverse_iterator crend() const noexcept;

        /** Returns true if begin() == end(), false otherwise. */
        bool empty() const noexcept;

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        int storage_bytes() const noexcept;

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        int distance() const noexcept;

        /** Returns the maximum size a rope can have. */
        std::ptrdiff_t max_size() const noexcept { return PTRDIFF_MAX; }

        /** Visits each segment s of *this and calls f(s).  Each segment is a
            string_view or repeated_string_view.  Depending of the operation
            performed on each segment, this may be more efficient than
            iterating over [begin(), end()).

            \pre Fn is an Invocable accepting a single argument of any of the
            types listed above. */
        template<typename Fn>
        void foreach_segment(Fn && f) const;

        /** Returns true if *this and rhs contain the same root node pointer.
            This is useful when you want to check for equality between two
            unencoded_ropes that are likely to have originated from the same
            initial unencoded_rope, and may have since been mutated. */
        bool equal_root(rope rhs) const noexcept;

        /** Clear. */
        void clear() noexcept;

        /** Inserts the sequence of char from rv into *this starting at position
            at. */
        rope & insert(iterator at, rope_view rv);

        /** Inserts the sequence of char from sv into *this starting at position
            at. */
        rope & insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rsv into *this starting at
            position at. */
        rope & insert(iterator at, repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        rope & insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        rope & insert(iterator at, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<rope &, CharRange>;

        template<typename CharIter>
        auto insert(iterator at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<rope &, CharIter>;

#endif

        /** Erases the portion of *this delimited by rv.

            \pre !std::less(rv.begin().base().base(), begin().base().base()) &&
            !std::less(end().base().base(), rv.end().base().base()) */
        rope & erase(rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        rope & replace(rope_view old_substr, rope_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        rope & replace(rope_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        rope & replace(rope_view old_substr, repeated_string_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharRange>
        rope & replace(rope_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharIter>
        rope & replace(rope_view old_substr, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto replace(rope_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<rope &, CharRange>;

        template<typename CharIter>
        auto replace(rope_view old_substr, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<rope &, CharIter>;

#endif

        /** Swaps *this with rhs. */
        void swap(rope & rhs) noexcept;

        /** Removes and returns the underlying unencoded_rope from *this. */
        unencoded_rope extract() && noexcept;

        /** Replaces the underlying unencoded_rope in *this. */
        void replace(unencoded_rope && ur) noexcept;

        /** Appends c_str to *this. */
        rope & operator+=(char const * c_str);

        /** Appends rv to *this. */
        rope & operator+=(rope_view rv);

        /** Appends tv to *this. */
        rope & operator+=(string_view sv);

        /** Appends rtv to *this. */
        rope & operator+=(repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        rope & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<rope &, CharRange>;

#endif

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, rope const & r)
        {
            if (os.good()) {
                auto const size = r.distance();
                detail::pad_width_before(os, size);
                if (os.good())
                    r.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator make_iter(
            detail::const_rope_iterator first,
            detail::const_rope_iterator it,
            detail::const_rope_iterator last) noexcept;

        using mutable_utf32_iter = utf8::to_utf32_iterator<
            detail::const_rope_iterator,
            detail::const_rope_iterator>;

        mutable_utf32_iter prev_stable_cp(mutable_utf32_iter last) noexcept;
        mutable_utf32_iter next_stable_cp(mutable_utf32_iter first) noexcept;

        // https://www.unicode.org/reports/tr15/#Concatenation
        void normalize_subrange(int from_near_offset, int to_near_offset);

        unencoded_rope rope_;

        friend struct ::boost::text::rope_view;

#endif // Doxygen
    };

    inline rope::iterator begin(rope & t) noexcept { return t.begin(); }
    inline rope::iterator end(rope & t) noexcept { return t.end(); }
    inline rope::const_iterator begin(rope const & t) noexcept
    {
        return t.begin();
    }
    inline rope::const_iterator end(rope const & t) noexcept { return t.end(); }
    inline rope::const_iterator cbegin(rope const & t) noexcept
    {
        return t.cbegin();
    }
    inline rope::const_iterator cend(rope const & t) noexcept
    {
        return t.cend();
    }

    inline rope::reverse_iterator rbegin(rope & t) noexcept
    {
        return t.rbegin();
    }
    inline rope::reverse_iterator rend(rope & t) noexcept { return t.rend(); }
    inline rope::const_reverse_iterator rbegin(rope const & t) noexcept
    {
        return t.rbegin();
    }
    inline rope::const_reverse_iterator rend(rope const & t) noexcept
    {
        return t.rend();
    }
    inline rope::const_reverse_iterator crbegin(rope const & t) noexcept
    {
        return t.crbegin();
    }
    inline rope::const_reverse_iterator crend(rope const & t) noexcept
    {
        return t.crend();
    }

}}

#include <boost/text/text.hpp>
#include <boost/text/rope_view.hpp>
#include <boost/text/detail/rope_iterator.hpp>

namespace boost { namespace text {

    rope::rope(char const * c_str) : rope_(text(c_str).extract()) {}

    template<int N>
    rope::rope(char (&c_str)[N]) : rope_(text(string_view(c_str, N)).extract())
    {}

    rope::rope(rope_view rv) :
        rope_(rv.begin().base().base(), rv.end().base().base())
    {}

    rope::rope(string_view sv)
    {
        text t(sv);
        insert(begin(), std::move(t).extract());
    }

    rope::rope(string && s)
    {
        normalize_to_fcc(s);
        insert(begin(), std::move(s));
    }

    rope::rope(text && t) : rope_(std::move(t).extract()) {}

    template<typename CharRange>
    rope::rope(CharRange const & r, detail::rng_alg_ret_t<int *, CharRange>)
    {
        text t(r);
        insert(begin(), std::move(t).extract());
    }

    template<typename CharIter>
    rope::rope(
        CharIter first,
        CharIter last,
        detail::char_iter_ret_t<void *, CharIter>)
    {
        text t(first, last);
        insert(begin(), std::move(t).extract());
    }

    rope & rope::operator=(rope_view rv)
    {
        rope temp(rv);
        swap(temp);
        return *this;
    }

    rope & rope::operator=(string_view sv)
    {
        rope temp(sv);
        swap(temp);
        return *this;
    }

    rope & rope::operator=(string && s)
    {
        rope temp(std::move(s));
        swap(temp);
        return *this;
    }

    rope & rope::operator=(text && t)
    {
        rope temp(std::move(t));
        swap(temp);
        return *this;
    }

    template<typename CharRange>
    auto rope::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<rope &, CharRange>
    {
        rope temp(r);
        swap(temp);
        return *this;
    }

    rope::const_iterator rope::begin() const noexcept
    {
        return make_iter(rope_.begin(), rope_.begin(), rope_.end());
    }
    rope::const_iterator rope::end() const noexcept
    {
        return make_iter(rope_.begin(), rope_.end(), rope_.end());
    }

    rope::const_iterator rope::cbegin() const noexcept { return begin(); }
    rope::const_iterator rope::cend() const noexcept { return end(); }

    rope::const_reverse_iterator rope::rbegin() const noexcept
    {
        return reverse_iterator(end());
    }
    rope::const_reverse_iterator rope::rend() const noexcept
    {
        return reverse_iterator(begin());
    }

    rope::const_reverse_iterator rope::crbegin() const noexcept
    {
        return rbegin();
    }
    rope::const_reverse_iterator rope::crend() const noexcept { return rend(); }

    bool rope::empty() const noexcept { return rope_.empty(); }

    int rope::storage_bytes() const noexcept { return rope_.size(); }

    int rope::distance() const noexcept
    {
        return std::distance(begin(), end());
    }

    template<typename Fn>
    void rope::foreach_segment(Fn && f) const
    {
        rope_.foreach_segment(static_cast<Fn &&>(f));
    }

    bool rope::equal_root(rope rhs) const noexcept
    {
        return rope_.equal_root(rhs.rope_);
    }

    void rope::clear() noexcept { rope_.clear(); }

    rope & rope::insert(iterator at, rope_view rv)
    {
        // TODO
        return *this;
    }

    rope & rope::insert(iterator at, string_view sv)
    {
        // TODO
        return *this;
    }

    rope & rope::insert(iterator at, repeated_string_view rsv)
    {
        // TODO
        return *this;
    }

    template<typename CharRange>
    auto rope::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<rope &, CharRange>
    {
        // TODO
        return *this;
    }

    template<typename CharIter>
    auto rope::insert(iterator at, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<rope &, CharIter>
    {
        // TODO
        return *this;
    }

    rope & rope::erase(rope_view rv)
    {
        // TODO
        return *this;
    }

    rope & rope::replace(rope_view old_substr, rope_view new_substr)
    {
        // TODO
        return *this;
    }

    rope & rope::replace(rope_view old_substr, string_view new_substr)
    {
        // TODO
        return *this;
    }

    rope & rope::replace(rope_view old_substr, repeated_string_view new_substr)
    {
        // TODO
        return *this;
    }

    template<typename CharRange>
    auto rope::replace(rope_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<rope &, CharRange>
    {
        // TODO
        return *this;
    }

    template<typename CharIter>
    auto rope::replace(rope_view old_substr, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<rope &, CharIter>
    {
        // TODO
        return *this;
    }

    void rope::swap(rope & rhs) noexcept { rope_.swap(rhs.rope_); }

    unencoded_rope rope::extract() && noexcept { return std::move(rope_); }

    void rope::replace(unencoded_rope && ur) noexcept { rope_ = std::move(ur); }

    rope & rope::operator+=(char const * c_str)
    {
        // TODO
        return *this;
    }

    rope & rope::operator+=(rope_view rv)
    {
        // TODO
        return *this;
    }

    rope & rope::operator+=(string_view sv)
    {
        // TODO
        return *this;
    }

    rope & rope::operator+=(repeated_string_view rsv)
    {
        // TODO
        return *this;
    }

    template<typename CharRange>
    auto rope::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<rope &, CharRange>
    {
        // TODO
        return *this;
    }

    rope::iterator rope::make_iter(
        detail::const_rope_iterator first,
        detail::const_rope_iterator it,
        detail::const_rope_iterator last) noexcept
    {
        return iterator{utf8::to_utf32_iterator<
                            detail::const_rope_iterator,
                            detail::const_rope_iterator>{first, first, last},
                        utf8::to_utf32_iterator<
                            detail::const_rope_iterator,
                            detail::const_rope_iterator>{first, it, last},
                        utf8::to_utf32_iterator<
                            detail::const_rope_iterator,
                            detail::const_rope_iterator>{first, last, last}};
    }

}}

#endif
