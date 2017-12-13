#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/grapheme_iterator.hpp>

#include <iterator>


namespace boost { namespace text {

    /* TODO: Make sure we support operations on:
        char const *          checked     (=)string_view
        string_view           checked     (=)string_view
        repeated_string_view  checked     
        string                checked     (=)string_view, except sink params
        unencoded_rope        checked     (=)unencoded_rope_view
        unencoded_rope_view   checked     (=)unencoded_rope_view
        text_view             unchecked   (=)text_view
        text                  unchecked   (=)text_view
        rope                  unchecked   
    */

    /** TODO */
    struct text
    {
        using iterator = grapheme_iterator<utf8::to_utf32_iterator<char *>>;
        using const_iterator =
            grapheme_iterator<utf8::to_utf32_iterator<char const *>>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /** Constructs a text from a string. */
        explicit text(string t);

        /** Constructs a text from a text_view. */
        explicit text(text_view tv);

        /** Constructs a text from a string_view. */
        explicit text(string_view tv);

        /** Constructs a text from a repeated_string_view. */
        explicit text(repeated_string_view tv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a text from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        explicit text(CharRange const & r);

        /** Constructs a text from a sequence of char.

            The sequence's UTF-8 encoding is not checked.  To check the
            encoding, use a converting iterator.

            This function only participates in overload resolution if CharIter
            models the Char_iterator concept. */
        template<typename CharIter>
        text(CharIter first, Iter Charlast);

#else

        template<typename CharRange>
        explicit text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            str_()
        {
            str_.insert(0, r);
        }

        template<typename CharIter>
        text(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0) :
            str_()
        {
            str_.insert(0, first, last);
        }

#endif

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        text & operator=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        /** Assignment from a string. */
        text & operator=(string const & t);

        /** Assignment from a string. */
        text & operator=(string && t);

        /** Assignment from a text_view. */
        text & operator=(text_view tv);

        /** Assignment from a string_view. */
        text & operator=(string_view tv);

        /** Assignment from a repeated_string_view. */
        text & operator=(repeated_string_view tv);

        iterator begin() noexcept { return make_iter(str_.begin()); }
        iterator end() noexcept { return make_iter(str_.end()); }

        const_iterator begin() const noexcept { make_iter(str_.begin()); }
        const_iterator end() const noexcept { make_iter(str_.end()); }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept
        {
            return make_reverse_iter(end() - 1);
        }
        reverse_iterator rend() noexcept
        {
            return make_reverse_iter(begin() - 1);
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return make_reverse_iter(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return make_reverse_iter(begin());
        }

        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        /** Returns true if size() == 0, false otherwise. */
        bool empty() const noexcept { return str_.empty(); }

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        int size() const noexcept { return str_.size(); }

        /** Returns the number of bytes of storage currently in use by
            *this. */
        int capacity() const noexcept { return str_.capacity(); }

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        int count_graphemes() const noexcept
        {
            returns std::distance(begin(), end());
        }

#if 0 // TODO: Do this in terms of iterators, or just drop it?
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
        text_view operator()(int lo, int hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        text_view operator()(int cut) const;
#endif

        /** Returns the maximum size a text can have. */
        int max_size() const noexcept { return INT_MAX / 2; }

#if 0 // TODO: Requires normalization and collation.
        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(text_view rhs) const noexcept;

        bool operator==(text_view rhs) const noexcept;
        bool operator!=(text_view rhs) const noexcept;
        bool operator<(text_view rhs) const noexcept;
        bool operator<=(text_view rhs) const noexcept;
        bool operator>(text_view rhs) const noexcept;
        bool operator>=(text_view rhs) const noexcept;
#endif

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit an
            empty string */
        void clear() noexcept { str_.clear(); }

        // TODO: Add reverse_iterator overloads too?
        /** Inserts the sequence of char from tv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        text & insert(iterator at, text_view tv);

        // TODO: Why isn't this redundant with the CharRange overload?  This
        // applies to other places as well.

        /** Inserts the sequence of char from tv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        text & insert(iterator at, string_view tv);

        /** Inserts the sequence of char from rtv into *this starting at
            offset at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        text & insert(iterator at, repeated_string_view rtv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at offset at.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding, or if the ends of the range are not valid
            UTF-8. */
        template<typename CharRange>
        text & insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if CharIter
            models the Char_iterator concept.

            No check is made to determine if insertion at position at would
            break UTF-8 encoding, and the inserted sequence's UTF-8 encoding
            is not checked.  To check the inserted sequence's encoding, use a
            converting iterator. */
        template<typename CharIter>
        text & insert(iterator at, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename CharIter>
        auto insert(iterator at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<text &, CharIter>
        {
            return str_.insert(at.base(), first, last);
        }

#endif

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin(), begin()) && !std::less(end(),
            tv.end()) */
        text & erase(text_view tv) noexcept;

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text &
        replace(text_view old_substr, text_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text &
        replace(text_view old_substr, repeated_string_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8.
            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharRange>
        text &
        replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharIter>
        text &
        replace(text_view old_substr, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto replace(text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename CharIter>
        auto
        replace(text_view old_substr, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<text &, CharIter>;

#endif

        /** Reserves storage enough for a string of at least new_size
            bytes.

            \post capacity() >= new_size + 1 */
        void reserve(int new_size) { str_.resize(); }

        /** Reduces storage used by *this to just the amount necessary to
            contain size() chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit() { str_.shrink_to_fit(); }

        /** Swaps *this with rhs. */
        void swap(text & rhs) noexcept { str_.swap(rhs.str_); }

        // TODO: Why isn't this redundant with the string_view overload?
        /** Appends c_str to *this. */
        text & operator+=(char const * c_str);

        /** Appends tv to *this. */
        text & operator+=(string const & t); // TODO: String overloads are needed elsewhere.

        /** Appends tv to *this. */
        text & operator+=(text_view tv);

        /** Appends tv to *this. */
        text & operator+=(string_view tv);

        /** Appends rtv to *this. */
        text & operator+=(repeated_string_view rtv);

        /** Appends r to *this. */
        text & operator+=(rope r);

        /** Appends r to *this. */
        text & operator+=(rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        text & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        // TODO: Do formatted output.
        /** Stream inserter; performs unformatted output. */
        friend std::ostream &
        operator<<(std::ostream & os, text const & t)
        {
            return os.write(t.begin(), t.size());
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        iterator make_iter(char * ptr)
        {
            return iterator{utf8::to_utf32_iterator<char *>{ptr}};
        }

        iterator make_iter(char const * ptr)
        {
            return iterator{utf8::to_utf32_iterator<char const *>{ptr}};
        }

        template<typename Iter>
        std::reverse_iterator<Iter> make_reverse_iter(Iter it)
        {
            return std::reverse_iterator<Iter>{it};
        }

        string str_;

#endif // Doxygen
    };

    inline text::iterator begin(text & t) noexcept { return t.begin(); }
    inline text::iterator end(text & t) noexcept { return t.end(); }
    inline text::const_iterator begin(text const & t) noexcept
    {
        return t.begin();
    }
    inline text::const_iterator end(text const & t) noexcept { return t.end(); }
    inline text::const_iterator cbegin(text const & t) noexcept
    {
        return t.cbegin();
    }
    inline text::const_iterator cend(text const & t) noexcept
    {
        return t.cend();
    }

    inline text::reverse_iterator rbegin(text & t) noexcept
    {
        return t.rbegin();
    }
    inline text::reverse_iterator rend(text & t) noexcept { return t.rend(); }
    inline text::const_reverse_iterator rbegin(text const & t) noexcept
    {
        return t.rbegin();
    }
    inline text::const_reverse_iterator rend(text const & t) noexcept
    {
        return t.rend();
    }
    inline text::const_reverse_iterator crbegin(text const & t) noexcept
    {
        return t.crbegin();
    }
    inline text::const_reverse_iterator crend(text const & t) noexcept
    {
        return t.crend();
    }

}}

#include <boost/text/repeated_text_view.hpp>

namespace boost { namespace text {

    namespace literals {

        /** Creates a text from a char string literal.

            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        inline text operator"" _gt(char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            assert(utf8::encoded(str, str + len));
            return text(str, str + len);
        }

        /** Creates a text from a char16_t string literal.

            \throw std::invalid_argument if the string is not valid UTF-16. */
        inline text operator"" _gt(char16_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 2);
            return text(
                utf8::from_utf16_iterator<char16_t const *>(str),
                utf8::from_utf16_iterator<char16_t const *>(str + len));
        }

        /** Creates a text from a char32_t string literal.

            \throw std::invalid_argument if the string is not valid UTF-32. */
        inline text operator"" _gt(char32_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 4);
            return text(
                utf8::from_utf32_iterator<char32_t const *>(str),
                utf8::from_utf32_iterator<char32_t const *>(str + len));
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline text::text(text_view tv) : str_()
    {
        str_.insert(str_.begin(), tv.begin().base(), tv.end().base());
    }

    inline text::text(string_view tv) :
        str_()
    {
        str_.insert(str_.begin(), tv.begin(), tv.end());
    }

    inline text::text(repeated_string_view rtv) :
        str_()
    {
        str_.insert(str_.begin(), rtv.begin(), rtv.end());
    }

    template<typename CharRange>
    auto text::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return *this = string_view(r); // TODO
    }

    inline text & text::operator=(text_view tv)
    {
        clear();
        str_.insert(str_.begin(), tv.begin().base(), tv.end().base());
        return *this;
    }

    inline text & text::operator=(string_view tv)
    {
        // TODO: Encoding checking?
        str_ = tv;
        return *this;
    }

    inline text & text::operator=(repeated_string_view rtv)
    {
        str_ = rtv;
        return *this;
    }

#if 0 // TODO
    inline text_view text::operator()(int lo, int hi) const
    {
        return text_view(*this)(lo, hi);
    }

    inline text_view text::operator()(int cut) const
    {
        return text_view(*this)(cut);
    }
#endif

#if 0 // TODO
    inline int text::compare(text_view rhs) const noexcept
    {
        return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end());
    }

    inline bool text::operator==(text_view rhs) const noexcept
    {
        return compare(rhs) == 0;
    }

    inline bool text::operator!=(text_view rhs) const noexcept
    {
        return compare(rhs) != 0;
    }

    inline bool text::operator<(text_view rhs) const noexcept
    {
        return compare(rhs) < 0;
    }

    inline bool text::operator<=(text_view rhs) const noexcept
    {
        return compare(rhs) <= 0;
    }

    inline bool text::operator>(text_view rhs) const noexcept
    {
        return compare(rhs) > 0;
    }

    inline bool text::operator>=(text_view rhs) const noexcept
    {
        return compare(rhs) >= 0;
    }
#endif

    template<typename CharRange>
    auto text::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return insert(at, text_view(r));
    }

    inline text &
    text::insert(iterator at, text_view tv)
    {
        str_.insert(at.base(), tv.begin().base(), tv.end().base());
        return *this;
    }

    inline text & text::insert(iterator at, string_view tv)
    {
        str_.insert(at.base(), tv);
        return *this;
    }

    inline text &
    text::insert(iterator at, repeated_string_view rtv)
    {
        str_.insert(at.base(), rtv);
        return *this;
    }

    inline text & text::erase(text_view tv) noexcept
    {
        str_.erase(tv.begin().base(), tv.end().base());
        return *this;
    }

    template<typename CharRange>
    auto
    text::replace(text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return replace(
            old_substr.begin().base(), old_substr.end().base(), string_view(r));
    }

    inline text & text::replace(
        text_view old_substr, text_view new_substr)
    {
        // TODO
        return *this;
    }

    inline text &
    text::replace(text_view old_substr, string_view new_substr)
    {
        // TODO
        return *this;
    }

    inline text & text::replace(
        text_view old_substr, repeated_string_view new_substr)
    {
        // TODO
        return *this;
    }

    template<typename Iter>
    auto
    text::replace(text_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<text &, Iter>
    {
        // TODO
        return *this;
    }

    inline text & text::operator+=(char const * c_str)
    {
        str_ += c_str;
        return *this;
    }

    inline text & text::operator+=(text_view tv)
    {
        return str_.insert(size(), tv); // TODO
    }

    inline text & text::operator+=(string_view tv)
    {
        str_ += tv;
        return *this;
    }

    inline text & text::operator+=(repeated_string_view rtv)
    {
        str_ += rtv;
        return *this;
    }

    template<typename CharRange>
    auto text::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return insert(size(), text_view(r));
    }

    inline bool text::self_reference(text_view tv) const
    {
        using less_t = std::less<char const *>;
        less_t less;
        return !less(tv.begin(), begin()) && !less(end(), tv.end());
    }

#endif // Doxygen

    inline bool operator==(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) == 0;
    }

    inline bool operator!=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) != 0;
    }

    inline bool operator<(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) < 0;
    }

    inline bool operator<=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) <= 0;
    }

    inline bool operator>(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) > 0;
    }

    inline bool operator>=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) >= 0;
    }


    /** Creates a new text object that is the concatenation of t and t2. */
    inline text operator+(text t, text const & t2) { return t += t2; }

    /** Creates a new text object that is the concatenation of t and tv. */
    inline text operator+(text t, text_view tv) { return t += tv; }

    /** Creates a new text object that is the concatenation of tv and t. */
    inline text operator+(text_view tv, text const & t)
    {
        return (text() += tv) += t;
    }

    /** Creates a new text object that is the concatenation of t and rtv. */
    inline text operator+(text t, repeated_string_view rtv) { return t += rtv; }

    /** Creates a new text object that is the concatenation of rtv and t. */
    inline text operator+(repeated_string_view rtv, text const & t)
    {
        return (text() += rtv) += t;
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new text object that is the concatenation of t and r.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(text t, CharRange const & r);

    /** Creates a new text object that is the concatenation of r and t.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(CharRange const & r, text const & t);

#else

    template<typename CharRange>
    auto operator+(text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return t += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return (text() += r) += t;
    }

#endif

}}

#endif
